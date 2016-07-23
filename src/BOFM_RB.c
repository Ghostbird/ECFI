#include <stdio.h>
#include <stdlib.h>
#include <ringbuffer.h>
typedef int (*compare_cb)(int a, int b);

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

int dummya(int a, int b){
return 5;
}

int dummyb(int a, int b){
return 5;
}

void func1(compare_cb fp){

	return fp(5, 6);

}

int main(int argc, char **argv){
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


