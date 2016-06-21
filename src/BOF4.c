#include <stdio.h>
#include <stdlib.h>
#include <BOF4.h>

/*#include <asm/ptrace.h>*/
/*#include <asm/processor.h>*/


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
 //   register int ecx asm("lr");
//    printf("LR value in vuln() function before calling the gets() is %p \n", ecx);
    char buff[10];
    gets(buff);
/*struct pt_regs *regs = task_pt_regs(current);*/
/*asm("movl %%lr, %0;" : "=r" (value) : );*/
    register int ecx2 asm("lr");
    printf("LR value in vuln() function after calling gets is %p \n", ecx2);
}
int main(int argc, char **argv){
//    register int ecx3 asm("lr");
//    printf("LR value before calling vuln() in main() is %p \n", ecx3);
//    rb_init_writer();
//    printf("The value of ring buffer read pointer is %p\n", rb_writer_read);
    vuln(argv[1]);
    register int ecx2 asm("lr");
    printf("LR value after calling vuln() in main() is %p \n", ecx2);
    printf("Normal Execution, No Buffer Overflow Occurred.\n");
    return 0;}


