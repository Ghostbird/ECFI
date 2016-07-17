#include <stdio.h>       /* fprintf(), stderr */
#include <stdlib.h>      /* exit() and macros */
#include <signal.h>      /* sigtimedwait() */
#include <time.h>        /* timer related functions */
#include <unistd.h>      /* usleep() */
#include "ringbuffer.h"
int main()
{
    printf("Injection-test: Starting injection test program.\n");
    /* Create array to hold reads. */
    regval_t data[WRITE_DATACOUNT];
    /* Attach to ringbuffer. */
    ringbuffer_t *rb = rb_attach(3);
    printf("Injection-test: Attached to ringbuffer.\n");
    if (rb == NULL)
    {
        fprintf(stderr, "Injection-test: Failed to attach to ringbuffer. Abort.\n");
        exit(EXIT_FAILURE);
    }
#if 0
    /* Declare space for timer. */
    timer_t tim = NULL;
    /* Create timer NULL sigevent creates a default one, which is good.*/
    if ( timer_create(CLOCK_REALTIME, NULL, &tim) == -1)
    {
        fprintf(stderr, "Injection-test: Failed to create timer. Abort.\n");
        exit(EXIT_FAILURE);
    }
    printf("Injection-test: Created timer.\n");
    fflush(NULL);
    /* Start timer
       - interval 1,000,000 nanoseconds. (1kHz)
       - start after 1 second.
       - wait timeout 30 seconds.*/
    const struct itimerspec its = { .it_interval = {0,1000000}, .it_value = {1,0} };
    const struct timespec wait = {3,0};
    if ( timer_settime(tim, 0, &its, NULL) == -1)
    {
        fprintf(stderr, "Injection-test: Failed to start timer. Abort.\n");
        exit(EXIT_FAILURE);
    }
    /* Create sigmask that matches only SIGALRM */
    sigset_t sigalarm;
    sigemptyset(&sigalarm);
    printf("Injection-test: Starting 1kHz timer in 1 second.\n");
    fflush(NULL);
    sigaddset(&sigalarm, SIGALRM);
#endif
    for (uint32_t i = 0; i < rb->size * 5; i++)
    {
#if 0
        printf("Injection-test: Waiting for sigalarm\n");
        sigtimedwait(&sigalarm, NULL, &wait );
        printf("Injection-test: Woke up!\n");
#else
        usleep(1000);
#endif
        for (uint32_t j = 0; j < WRITE_DATACOUNT; j++)
        {
            data[j] = i * j;
        }
        rb_write(rb, data);
    }
    printf("Injection-test: Finished.\n");
    fflush(NULL);
}
