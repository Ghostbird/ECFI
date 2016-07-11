#!/usr/bin/env python

"""
DummyModbusSVC client (master)

Simple dummy modbus TCP client performing continuous read/modify/write operation to 2 registers calculating fibonnaci mod 0x10000.
"""

from sys import argv
from time import sleep
from pymodbus.client.sync import ModbusTcpClient

def mod_fibonacci(fn1, fn2, m):	
	return ((fn1 + fn2) % m)

def read_regs(client):
	rr = client.read_holding_registers(0, 2)
	assert (rr.function_code < 0x80), "[-] Exception function code returned during read_regs"
	return rr.registers

def write_regs(client, a, b):
	rq = client.write_registers(0, [a, b])
	assert (rq.function_code < 0x80), "[-] Exception function code returned during write_regs"
	return

client = ModbusTcpClient('192.168.0.102', port=1502)

# Fibonnaci modulus
fib_mod = 0x10000

# Initialization
if(len(argv) > 1):
	fib_start = int(argv[1])
else:
	fib_start = 1

write_regs(client, fib_start, fib_start)

while(True):
	fn2, fn1 = read_regs(client)
	print ("%d" % fn2)
	fn = mod_fibonacci(fn1, fn2, fib_mod)
	write_regs(client, fn1, fn)
	sleep(0.1)