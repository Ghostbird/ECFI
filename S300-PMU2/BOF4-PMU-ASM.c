// BEFORE RUNNING THIS CODE PLEASE INSERT THE CRASH1 Kernel Module to Enable PMU Access.

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
//	register int ecx asm("lr");

//	printf("LR value is %p", ecx);

	printf("Hi");
//	vuln(argv[1]);
        unsigned int output;

        // Read current event counter
        asm volatile ("MRC p15,   0,    %0,  c9  ,   c13 ,   2\t\n": "=r"(output));
    	printf("Event count	0: %ul\n", output);
//	register int ecx asm("lr");
//	printf("LR value is %p", ecx);
//	do_reset =1;
	printf("Normal Execution, No Buffer Overflow Occurred.\n");
   return 0;
}
