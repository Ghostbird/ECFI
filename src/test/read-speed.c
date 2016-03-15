#include "ringbuffer.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFCOUNT 1024
#define BUFSIZE  255

int main()
{
    printf("Reading %i buffer entries %i times.\n", BUFSIZE, BUFCOUNT);
    ringbuffer_info_t *rb_info = rb_create(BUFSIZE, "speedtestbuffer");
    if (rb_info == NULL)
    {
        fprintf(stderr, "Could not create ringbuffer. Abort.\n");
        exit(EXIT_FAILURE);
    }
    ringbuffer_t *rb = rb_info->rb;
    /* This is a hack. It invalidates writing, but it also allows us to read unimpeded. */
    rb->write = rb->size + WRITE_DATACOUNT;
    uint32_t data[] = {0};
    struct timespec starttime;
    struct timespec endtime;
    if (clock_gettime(CLOCK_REALTIME, &starttime) < 0)
    {
        fprintf(stderr,"Could not read start clock. Abort.\n");
        rb_destroy(rb_info);
        exit(EXIT_FAILURE);
    }
    for (uint32_t i = 0; i < BUFSIZE * BUFCOUNT; i++)
    {
        rb_read(rb, data, WRITE_DATACOUNT);
    }
    if (clock_gettime(CLOCK_REALTIME, &endtime) < 0)
    {
        fprintf(stderr,"Could not read final clock. Abort.\n");
        rb_destroy(rb_info);
        exit(EXIT_FAILURE);
    }
    printf("It took %li seconds and %li nanoseconds to read %i buffer entries %i times.\n", endtime.tv_sec - starttime.tv_sec, endtime.tv_nsec - starttime.tv_nsec, BUFSIZE, BUFCOUNT);
    rb_destroy(rb_info);
}
