#include <stdio.h>
#include <stdlib.h>
#include <ringbuffer.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>

typedef int (*compare_cb)(int a, int b);


static long perf_event_open(struct perf_event_attr *hw_event, pid_t pid, int cpu, int group_fd, unsigned long flags)
	{
		int ret;

		ret = syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
	return ret;
}

void cpukicker(){
int k;
int l=1;
int c=7;
int d=100;
int f=1000;
int h=10000;
int p;
int g;
p =l +c;
p=c+d;
p=d+100;
p=p+c+d+f+h;
g=p*10;
for( k = 1; k < 100; k = k + 1 ){
	p=c+p;
	}
dummya(1,2);
return 0;
}



void secretfunction(){
    register int ecx2 asm("lr");
    printf("Final LR value in secret function is %p \n", ecx2);
    puts("Secret Function to execute system command...\n");
    system("setterm –term linux –foreground red -clear");
    system("cat /etc/passwd");
    puts("Tango Down...");
    exit(0);
}
void vuln(char *arg)
{
    char buff[10];
    gets(buff);
    register int ecx2 asm("lr");
    printf("LR value in vuln() function after calling gets is %p \n", ecx2);
}

int dummya(int z, int b){
struct perf_event_attr pe;
long long count;
int fd;
int a;
for( a = 1; a < 1000001; a = a + 1 ){
	memset(&pe, 0, sizeof(struct perf_event_attr));
	pe.type = PERF_TYPE_HARDWARE;
	pe.size = sizeof(struct perf_event_attr);
//pe.config = PERF_COUNT_HW_BRANCH_INSTRUCTIONS;
	pe.config = PERF_COUNT_HW_CPU_CYCLES;
	pe.disabled = 1;
	pe.exclude_kernel = 1;
	pe.exclude_hv = 1;
	fd = perf_event_open(&pe, 0, -1, -1, 0);
	if (fd == -1) {
		fprintf(stderr, "Error opening leader %llx\n", pe.config);
		exit(EXIT_FAILURE);
	       }

	ioctl(fd, PERF_EVENT_IOC_RESET, 0);
	ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);
//From here the counter starts.
	asm("PUSH    {R5, R6, R8}");
	asm("LDR     r5, =[rb_writer_buffer]");
	asm("LDR     r5, [r5]");
	asm("LDR     r8, =1234");
	asm("SUB     r6, r5, #8");
	asm("LDR     r6, [r6]");
	asm("ADD     r5, r6");
	asm("MOV     r6,LR");
	asm("STMIA   r5!, {r8,r6}");
	asm("LDR     r6, =[rb_writer_end]");
	asm("LDR     r6, [r6]");
	asm("SUB     r8, r6, #1020");
	asm("TEQ     r6, r5");
	asm("MOVEQ   r5, r8");
	asm("LDR     r6, =[rb_writer_read]");
	asm("LDR     r6, [r6]");
	asm("ADD     r8,r6, r8");
	asm("TEQ     r5, r8");
	asm("SUBEQ   r8, r8, r6");
	asm("ADDEQ   r8, r8, #20");
	asm("SUBEQ   r8, r5, r8");
	asm("LDREQ   r6, =[rb_writer_read]");
	asm("LDREQ   r6, [r6]");
	asm("STREQ   r8, [r6]");
	asm("LDR     r6, =[rb_writer_write]");
	asm("LDR     r6, [r6]");
	asm("MOV     r8, r6");
	asm("ADD     r8, #8");
	asm("SUB     r5, r5, r8");
	asm("STR     r5, [r6]");
	asm("POP     {R5, R6, R8}");

//Disabling Counter
	ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);

	read(fd, &count, sizeof(long long));
	printf("%lld\n", count);
	cpukicker();
	close(fd);
}
return 5;
}

int dummyb(int z, int b){
return 5;
}

func1(compare_cb fp){

	return fp(5, 6);
	dummyb(2,3);
}

int main(int argc, char **argv){
    rb_init_writer();
    printf("The value of ring buffer read pointer is %p \n", rb_writer_read);
    printf("The value of ring buffer write pointer is %p \n", rb_writer_write);
    printf("The value of ring buffer pointer is %p \n", rb_writer_buffer);
    printf("The value of ring buffer end is %p \n", rb_writer_end);
    func1(dummya);
    register int ecx3 asm("lr");
    printf("LR value before calling vuln() in main() is %p \n", ecx3);
    vuln(argv[1]);
    register int ecx2 asm("lr");
    printf("LR value after calling vuln() in main() is %p \n", ecx2);
    printf("Normal Execution, No Buffer Overflow Occurred.\n");
    return 0;}


