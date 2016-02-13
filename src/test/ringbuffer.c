#include "ringbuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>

#define TRUE  1
#define FALSE 0

/* Test normal ring buffer creation */
char test_create(uint32_t bufsize)
{
    int success = TRUE;
    printf("Ring buffer creation for %d items...\n",bufsize);
    ringbuffer_t *rb = rb_create(bufsize);
    if (rb == NULL)
    {
        printf("Received NULL pointer instead of created ring buffer.\n");
        success = FALSE;
    }
    else
    {
        if (rb->size != bufsize)
        {
            printf("Ring buffer allocation of %d items created buffer with size %d instead.\n", bufsize, rb->size);
            success = FALSE;
        }
        if (rb->start == NULL)
        {
            printf("Ring buffer failed to allocate memory space for %d items.\n", bufsize);
            success = FALSE;
        }
        if (rb->read != 0)
        {
            printf("Ring buffer initial read index set to %d instead of 0.\n", rb->write);
            success = FALSE;
        }
        if (rb->write != 0)
        {
            printf("Ring buffer initial write index set to %d instead of 0.\n", rb->write);
            success = FALSE;
        }
        /* No guarantee this will work though, it hasn't been tested yet! */
        rb_destroy(rb);
    }
    if (success)
        printf("SUCCESS\n");
    else
        printf("FAILURE\n");
    return success;
}

char test_create_zero()
{
    printf("Verifying that it is not possible to create an invalid ring buffer of zero size...\n");
    ringbuffer_t *rb = rb_create(0);
    if (rb != NULL)
    {
        printf("Invalid ring buffer was created\n");
        /* Note: At this point you have a memory leak in the test. */
        printf("FAILURE\n");
        return FALSE;
    }
    else
    {
        printf("SUCCESS\n");
        return TRUE;
    }
}

char test_create_nomem()
{
    printf("Testing correct handling of Out of Memory situation during create...\n");
    char success = TRUE;
    const uint32_t extramem = 1024;
    /* Get memory usage */
    struct rusage ru;
    if (getrusage(RUSAGE_SELF, &ru) != 0)
    {
        printf("Failed to get memory usage!\n");
        return FALSE;
    }
    else
    {
        /* Restrict memory usage */
        struct rlimit rl;
        /* ru_maxrss is in kiB, RLIMIT_AS is set in bytes. We want to leave 1kiB extra space.*/
        rl.rlim_cur = (ru.ru_maxrss + 1) * extramem;
        rl.rlim_max = RLIM_INFINITY;
        if (setrlimit(RLIMIT_AS, &rl) != 0)
        {
            printf("Failed to set memory limit!\n");
            return FALSE;
        }
        ringbuffer_t *rb = rb_create(1);
        if (rb == NULL)
        {
            printf("Failed to create small ring buffer\n");
            success = FALSE;
        }
        else
        {
            /* No guarantee this works, hasn't been tested yet. */
            rb_destroy(rb);
        }
        rb = rb_create(1024*1024);
        if (rb != NULL)
        {
            printf("Created ring buffer beyond allowed memory limits\n");
            /* No guarantee this works, hasn't been tested yet. */
            rb_destroy(rb);
            success = FALSE;
        }
        /* Remove memory limits */
        rl.rlim_cur = RLIM_INFINITY;
        if (setrlimit(RLIMIT_AS, &rl) != 0)
        {
            printf("Failed to remove memory limit.\n");
            return FALSE;
        }
    }
    if (success)
        printf("SUCCESS\n");
    else
        printf("FAILURE\n");
    return success;
}

/* Requires that creation of the ring buffer has been tested successfully */
char test_destroy()
{
    printf("Destruction testing...\n");
    char success = TRUE;
    ringbuffer_t *curr_rb = NULL;
    ringbuffer_t *prev_rb = NULL;
    regval_t *curr_rv = NULL;
    regval_t *prev_rv = NULL;
    for (uint32_t i = 1; i < 1024; i++)
    {
        curr_rb = rb_create(1024 * i);
        if (curr_rb == NULL)
        {
            printf("Create failed during destruction test.\n");
            return FALSE;
        }
        curr_rv = curr_rb->start;
        if (curr_rv == NULL)
        {
            printf("Initialisation of buffer failed during destruction test.\n");
            return FALSE;
        }
        if (prev_rb != NULL)
        {
            /* We test that subsequent creation, destruction then re-creation use the same memory location every time.
            If not, the destruction has failed to free some memory */
            if (curr_rb != prev_rb)
            {
                printf("Incomplete destruction for buffer with i=%d identified by ringbuffer pointer.\n", i);
                success = FALSE;
            }
            if (curr_rv != prev_rv)
            {
                printf("Incomplete destruction for buffer with i=%d identified by ringbuffer->start pointer.\n", i);
                success = FALSE;
            }
            rb_destroy(prev_rb);
            prev_rb = curr_rb;
            prev_rv = curr_rv;
        }
    }
    /* Clean up the last ring buffer. */
    rb_destroy(curr_rb);
    if (success)
        printf("SUCCESS\n");
    else
        printf("FAILURE\n");
    return success;
}

char test_read()
{
    printf("Testing read functionality...\n");
    char success = TRUE;
    ringbuffer_t *rb = rb_create(1024);
    /* Fill the buffer manually with test values */
    for (uint32_t i = 0; i < rb->size; i++)
    {
        rb->start[i] = (regval_t)i;
    }
    if (rb_read(NULL,NULL,0) != NULL)
    {
        printf("Possible to read from NULL buffer\n");
        success = FALSE;
    }
    if (rb_read(rb,NULL,0) != NULL)
    {
        printf("Possible to read into NULL array\n");
        success = FALSE;
    }
    uint32_t datacount = 11;
    regval_t *data = malloc(datacount * sizeof(regval_t));
    if (data == NULL)
    {
        printf("Failed to allocate test memory, aborting!\n");
        return FALSE;
    }
    if (rb_read(rb,data,0) == NULL)
    {
        printf("Failed to read zero data into valid array\n");
        success = FALSE;
    }
    if (rb_read(rb,data,datacount) != NULL)
    {
        printf("Read beyond write pointer\n");
        success = FALSE;
    }
    rb->write = datacount - 1;
    if (rb_read(rb,data,datacount) != NULL)
    {
        printf("Partial read beyond write pointer\n");
        success = FALSE;
    }
    rb->write = datacount;
    if (rb_read(rb,data,datacount) == NULL)
    {
        printf("Failed to read exactly to write pointer\n");
        success = FALSE;
    }
    for (uint32_t i = 0; i < datacount; i++)
    {
        if (data[i] != i)
        {
            printf("Incorrect values found expected %d, got %d instead\n", i, data[i]);
            success = FALSE;
        }
    }
    if (rb->read != datacount)
    {
        printf("Detected failure to update read index");
        success = FALSE;
    }
    rb->write = 0;
    if (rb_read(rb,data,datacount) == NULL)
    {
        printf("Failed to read while ahead of write pointer");
        success = FALSE;
    }
    for (uint32_t i = 0; i < datacount; i++)
    {
        if (data[i] != i + datacount)
        {
            printf("Incorrect values found expected %d, got %d instead", i + datacount, data[i]);
            success = FALSE;
        }
    }
    if (rb->read != datacount * 2)
    {
        printf("Detected failure to update read index a second time\n");
        success = FALSE;
    }
    rb->read = 0;
    free(data);
    datacount = rb->size;
    data = malloc(datacount * sizeof(regval_t));
    if (data == NULL)
    {
        printf("Failed to allocate of test memory, aborting! (2)\n");
        return FALSE;
    }
    if (rb_read(rb, data, rb->size) != NULL)
    {
        printf("Incorrectly read entire buffer of old data\n");
        success = FALSE;
    }
    if (success)
        printf("SUCCESS\n");
    else
        printf("FAILURE\n");
    return success;
}

int main(void)
{
    return !(test_create(1) && test_create(1024) && test_create(1024*1024) && test_create_zero() && test_create_nomem() && test_destroy() && test_read());
}
