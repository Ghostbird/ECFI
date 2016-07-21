# S300-PMU2: Accessing ARM SoCs Performance Monitoring Unit(PMU) - User/Kernel Mode

S300-PMU2 is a sophisticated Russian surface-to-air missle to protect an airspace from cruise and ballistic missles.  
We name this component of the ECFI S300-PMU2, since it protects the ring buffer of RTOS from malicious overwrites. It can also calculate the Branches, Returns and CPU cycle.

##Using PMU for calculating CPU cycles of injectioncode

We can use S300-PMU2 to calculate the CPU cycle for our injection code. To do so we use perf syscall. 

Here is the sample C code we use. 
``` 
int Anyfunction(){
//This is my function you need to change it for yourself
//preparing perf
// Please include unistd.h, string.h, sys/ioctl.h, linux/perf_event.h, asm/unistd.h in the program.
struct perf_event_attr pe;
long long count;
int fd;
        memset(&pe, 0, sizeof(struct perf_event_attr));
        pe.type = PERF_TYPE_HARDWARE;
        pe.size = sizeof(struct perf_event_attr);
        //here we choosed the CPU cycle you can change it.
        // Look at http://man7.org/linux/man-pages/man2/perf_event_open.2.html
        pe.config = PERF_COUNT_HW_CPU_CYCLES;
        pe.disabled = 1;
        pe.exclude_kernel = 1;
        pe.exclude_hv = 1;
        fd = perf_event_open(&pe, 0, -1, -1, 0);
        if (fd == -1) {
                fprintf(stderr, "Error opening leader %llx\n", pe.config);
                exit(EXIT_FAILURE);
               }
        //resetting any previous counter and enabling counter
        ioctl(fd, PERF_EVENT_IOC_RESET, 0);
        ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);
//From here the counter starts.
        asm("Your ASM Codes");
        asm("Your ASM Codes");
        asm("Your ASM Codes");
        asm("Your ASM Codes");
        asm("Your ASM Codes");
        asm("Your ASM Codes");
        asm("Your ASM Codes");
        asm("Your ASM Codes");
//Disabling Cycle Counter
        ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
        read(fd, &count, sizeof(long long));
        printf("%lld\n", count);
        close(fd);
return 0;
}

```
## Using PMU for Ring Buffer Protection
To protect the ring buffer we can count number of branches or returns or even cpu cycles taken place and if there is any significant change raise an alert. To do so we can either use either perf or using our own assembly code. In this section we use our own code.
Before we use the PMU we need to enable user mode access to the PMU. To do it we write a Loadable Kernel Module as follow:

``` 
/* Module source file 'crash1.c'. */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

arm_write(unsigned long val)
{
        // enabling both read and write
        asm volatile("mrc p15, 0, %0, c9, c14, 0" :: "r"(1));
        asm volatile("mcr p15, 0, %0, c9, c14, 0" :: "r"(1));

}

static int loader(void)
{
     unsigned long value = 1;
     printk(KERN_INFO "Enabling PMU usermode.\n");
     arm_write(value);
     return 0;
}

static void unloader(void)
{
     printk(KERN_INFO "crash1 module being unloaded.\n");
}

module_init(loader);
module_exit(unloader);

MODULE_AUTHOR("Ali Abbasi, Gijsbert ter Horst");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Enabling PMU access from user mode");
```

We then make a Makefile as follow:

``` 
ifeq ($(KERNELRELEASE),)

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

.PHONY: build clean

build:
        $(MAKE) -Wall -C $(KERNELDIR) M=$(PWD) modules

clean:
        rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c
else

$(info Building with KERNELRELEASE = ${KERNELRELEASE})
obj-m :=    crash1.o

endif
```

We then compile the code with make and insert the module to the kernel with `insmod crash1.ko`. Once the module loads, the user mode access will be enabled. We then use PMU directly. Here is the inline assembly code we used for monitoring the PMU in BOF example:

``` 
#include <stdio.h>
#include <asm/ptrace.h>
/*#include <asm/processor.h>*/
int aliabs;
aliabs=1;
void secretfunction(){

//	register int ecx asm("lr");
//	printf("LR value is %p", ecx);

	puts("Secret Function to execute system command...\n");

	system("setterm –term linux –foreground red -clear");
	system("cat /etc/passwd");

	puts("Tango Down...");
      exit(0);
}

void vuln(char *arg){

//	register int ecx asm("lr");
//	printf("LR value is %p", ecx);

	char buff[10];
	char buf[90];
	gets(buff);
	printf("A\n");
	gets(buf);

/*struct pt_regs *regs = task_pt_regs(current);*/
/*asm("movl %%lr, %0;" : "=r" (value) : );*/
//	register int ecx asm("lr");
//	printf("LR value is %p", ecx);
}

int main(int argc, char **argv){
	int enable_divider =1;
	int do_reset=1;
        int value = 1;

        // peform reset:
        if (do_reset) {
                value |= 2;     // reset all counters to zero.
                value |= 4;     // reset cycle counter to zero.
        }

        if (enable_divider)
                value |= 8;     // enable "by 64" divider for CCNT.

        value |= 16;
        // program the performance-counter control-register with mask constructed above
        asm volatile ("MCR p15, 0, %0, c9, c12, 0\t\n" :: "r"(value));
        // enable all counters:
        asm volatile ("MCR p15, 0, %0, c9, c12, 1\t\n" :: "r"(0x8000000f));

        // clear overflows:
        asm volatile ("MCR p15, 0, %0, c9, c12, 3\t\n" :: "r"(0x80000001));

        // Select individual counter (0)
        asm volatile ("MCR p15,   0,    %0,  c9  ,   c12 ,   5\t\n":: "r"(0x00));

        // Write event (0x11 = Cycle count)
        asm volatile ("MCR p15,   0,    %0,  c9  ,   c13 ,   1\t\n":: "r"(0xD));


	printf("Hi");
        unsigned int output;

        // Read current event counter
        asm volatile ("MRC p15,   0,    %0,  c9  ,   c13 ,   2\t\n": "=r"(output));
    	printf("Event count	0: %ul\n", output);
	printf("Normal Execution, No Buffer Overflow Occurred.\n");
   return 0;
}

```
