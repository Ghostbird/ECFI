#include <stdio.h>  /* fprintf() */
#include <stdlib.h> /* exit() and macros */
#include <signal.h> /* sigtimedwait() */
#include <time.h>   /* timer related functions */
#include "ringbuffer.h"
int main()
{
    /* Create array to hold reads. */
    regval_t data[WRITE_DATACOUNT];
    /* Attach to ringbuffer. */
    ringbuffer_t *rb = rb_attach(3);
    /* Declare space for timer. */
    timer_t tim = NULL;
    /* Create timer tim. NULL sigevent creates a default one, which is good.*/
    if ( timer_create(CLOCK_REALTIME, NULL, tim) == -1)
    {
        fprintf(stderr, "Failed to create timer. Aborting.");
        exit(EXIT_FAILURE);
    }
    /* Start timer
       - interval 1,000,000 nanoseconds. (1kHz)
       - start after 1 second.
    */
    const struct itimerspec its = {{1,0},{0,1000000}};
    if ( timer_settime(tim, 0, &its, NULL) == -1)
    {
        fprintf(stderr, "Failed to start timer. Aborting.");
        exit(EXIT_FAILURE);
    }
    /* Create sigmask that matches only SIGALRM */
    sigset_t sigalarm;
    sigemptyset(&sigalarm);
    sigaddset(&sigalarm, SIGALRM);
    for (uint32_t i; i < rb->size * 5; i++)
    {
        /* Wait for sigalarm with 1 second timeout.
           Combined with the timer, this makes the loop run at 1kHz */
        printf("Waiting for SIGALRM");
        fflush(NULL);
        sigtimedwait(&sigalarm, NULL, &(its.it_value));
        for (uint32_t j; j < WRITE_DATACOUNT; j++)
        {
            data[j] = i * j;
        }
        rb_write(data, rb);
    }
}
