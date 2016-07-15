#include <ringbuffer.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "USAGE: %s <ringbuffer file>\n", argv[0]);
        exit(1);
    }
    int fd = open(argv[1], O_RDWR);
    if (fd < 0)
    {
        fprintf(stderr, "Failed to open file %s\n", argv[1]);
        exit(1);
    }
    ringbuffer_t* rb = rb_attach(fd);
    if (rb == NULL)
    {
        fprintf(stderr, "Failed to attach to ringbuffer on fd %i\n", fd);
        exit(1);
    }
    regval_t data[WRITE_DATACOUNT];
    regval_t* result;
    result = rb_read(rb, data, WRITE_DATACOUNT);
    if (result == NULL)
    {
        fprintf(stderr, "Failed to read from ringbuffer\n");
        exit(1);
    }
    printf("Read: 0x");
    for (int i; i < WRITE_DATACOUNT; i++)
    {
        printf("%04x ",data[i]);
    }
    printf("\n");
}
