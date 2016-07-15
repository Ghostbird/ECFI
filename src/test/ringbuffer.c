#include "ringbuffer.h"
#include "errors.h"
#include <stdio.h> /* printf(), fprintf() */
#include <stdlib.h> /* malloc(), exit() and macros*/
#include <unistd.h> /* fork() */
#include <sys/resource.h> /* getrusage(), setrlimit() */
#include <sys/wait.h> /* waitpid() */
#include <sys/mman.h> /* shm_open() and mmap() */
#include <sys/stat.h> /* Mode constraints for shm_open() */
#include <fcntl.h>    /* Flag values for O_ constants for shm_open() */
#include <string.h>   /* memcmp() */

#define TRUE  1
#define FALSE 0

#define BUFNAME "testbuffer"
#define EXPECTED_FD 3

/* Test normal ring buffer creation */
char test_create(uint32_t bufsize)
{
    /* Flush to prevent stdout and stderr to desynchronise between test cases. */
    fflush(NULL);
    int success = TRUE;
    printf("Ring buffer creation for %d items... ",bufsize);
    ringbuffer_info_t *rb_info = rb_create(bufsize, BUFNAME);
    if (rb_info == NULL)
    {
        fprintf(stderr, "Received NULL pointer instead of created ring buffer.\n");
        success = FALSE;
    }
    else
    {
        ringbuffer_t *rb = rb_info->rb;
        if (strncmp(rb_info->name, BUFNAME, strlen(BUFNAME) + 1) != 0)
        {
            fprintf(stderr, "Name not set correctly. Expected %s, but got %s.\n", BUFNAME, rb_info->name);
            success = FALSE;
        }
        if (rb_info->fd != EXPECTED_FD)
        {
            fprintf(stderr, "Got fd: %i instead of fd: %i", rb_info->fd, EXPECTED_FD);
            success = FALSE;
        }
        if (rb == NULL)
        {
            fprintf(stderr, "Got NULL pointer instead of created ring buffer struct.\n");
            success = FALSE;
        }
        else
        {
            if (rb->size != bufsize)
            {
                fprintf(stderr, "Ring buffer allocation of %d items created buffer with size %d instead.\n", bufsize, rb->size);
                success = FALSE;
            }
            if (rb->read != 0)
            {
                fprintf(stderr,"Ring buffer initial read index set to %d instead of 0.\n", rb->write);
                success = FALSE;
            }
            if (rb->write != 0)
            {
                fprintf(stderr,"Ring buffer initial write index set to %d instead of 0.\n", rb->write);
                success = FALSE;
            }
        }
        /* No guarantee this will work though, it hasn't been tested yet! */
        rb_destroy(rb_info);
    }
    if (success)
        printf("SUCCESS\n");
    else
        printf("FAILURE\n");
    return success;
}

char test_create_zero()
{
    /* Flush to prevent stdout and stderr to desynchronise between test cases. */
    fflush(NULL);
    printf("Verifying that it is not possible to create an invalid ring buffer of zero size... ");
    ringbuffer_info_t *rb_info = rb_create(0, BUFNAME);
    if (rb_info != NULL)
    {
        fprintf(stderr,"Invalid ring buffer was created\n");
        /* Try to destroy the invalid buffer. May fail. */
        rb_destroy(rb_info);
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

char test_create_noname()
{
    /* Flush to prevent stdout and stderr to desynchronise between test cases. */
    fflush(NULL);
    printf("Verifying that it is not possible to create an invalid ring buffer without a name... ");
    ringbuffer_info_t *rb_info = rb_create(1, "");
    if (rb_info != NULL)
    {
        fprintf(stderr,"Invalid ring buffer was created\n");
        /* Try to destroy the invalid buffer. May fail. */
        rb_destroy(rb_info);
        /* Note: At this point you may have a memory leak in the test. */
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
    /* Flush to prevent stdout and stderr to desynchronise between test cases. */
    fflush(NULL);
    /* Memory that can still be allocated.*/
    printf("Testing correct handling of Out of Memory situation during create... ");
    char success = TRUE;
    /* Get memory usage */
    struct rusage ru;
    if (getrusage(RUSAGE_SELF, &ru) != 0)
    {
        fprintf(stderr, "Failed to get memory usage!\n");
        return FALSE;
    }
    else
    {
        /* Restrict memory usage to current usage.*/
        struct rlimit rl;
        /* ru_maxrss is in kiB, RLIMIT_AS is set in bytes. We want to leave 1kiB extra space.*/
        rl.rlim_cur = (ru.ru_maxrss * 1024);
        rl.rlim_max = RLIM_INFINITY;
        if (setrlimit(RLIMIT_AS, &rl) != 0)
        {
            fprintf(stderr, "Failed to set memory limit!\n");
            return FALSE;
        }
        /* Now try to create a ring buffer */
        ringbuffer_info_t *rb_info = rb_create(1, BUFNAME);
        if (rb_info != NULL)
        {
            fprintf(stderr, "Created ring buffer beyond allowed memory limits\n");
            /* No guarantee this works, hasn't been tested yet. */
            rb_destroy(rb_info);
            success = FALSE;
        }
        /* Remove memory limits */
        rl.rlim_cur = RLIM_INFINITY;
        if (setrlimit(RLIMIT_AS, &rl) != 0)
        {
            fprintf(stderr, "Failed to remove memory limit.\n");
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
    /* Flush to prevent stdout and stderr to desynchronise between test cases. */
    fflush(NULL);
    printf("Destruction testing... ");
    /* Flush again because this test takes a while and the buffer will only flush after the loop. */
    fflush(NULL);
    char success = TRUE;
    for (uint32_t i = 1; i < 1024; i++)
    {
        /* Create buffer */
        ringbuffer_info_t *rb_info = rb_create(i, BUFNAME);
        if (rb_info == NULL)
        {
            fprintf(stderr, "Create failed during destruction test %d.\n", i);
            success = FALSE;
            break;
        }
        /* Destroy buffer. If destruction fails,
        subsequent creates will fail.*/
        rb_destroy(rb_info);
    }
    if (success)
        printf("SUCCESS\n");
    else
        printf("FAILURE\n");
    return success;
}

char test_read()
{
    /* Flush to prevent stdout and stderr to desynchronise between test cases. */
    fflush(NULL);
    printf("Testing read functionality... ");
    char success = TRUE;
    ringbuffer_info_t *rb_info = rb_create(1024, BUFNAME);
    ringbuffer_t *rb = rb_info->rb;
    /* Fill the buffer manually with test values */
    for (uint32_t i = 0; i < rb->size; i++)
    {
        ((regval_t*)(rb + 1))[i] = (regval_t)i;
    }
    if (rb_read(NULL,NULL,0) != NULL)
    {
        fprintf(stderr, "Possible to read from NULL buffer\n");
        success = FALSE;
    }
    if (rb_read(rb,NULL,0) != NULL)
    {
        fprintf(stderr, "Possible to read into NULL array\n");
        success = FALSE;
    }
    uint32_t datacount = 11;
    regval_t *data = malloc(datacount * sizeof(regval_t));
    if (data == NULL)
    {
        fprintf(stderr, "Failed to allocate test memory, aborting!\n");
        rb_destroy(rb_info);
        return FALSE;
    }
    if (rb_read(rb,data,0) == NULL)
    {
        fprintf(stderr, "Failed to read zero data into valid array\n");
        success = FALSE;
    }
    if (rb_read(rb,data,datacount) != NULL)
    {
        fprintf(stderr, "Read beyond write pointer\n");
        success = FALSE;
    }
    rb->write = datacount - 1;
    if (rb_read(rb,data,datacount) != NULL)
    {
        fprintf(stderr, "Partial read beyond write pointer\n");
        success = FALSE;
    }
    rb->write = datacount;
    if (rb_read(rb,data,datacount) == NULL)
    {
        fprintf(stderr, "Failed to read exactly to write pointer\n");
        success = FALSE;
    }
    for (uint32_t i = 0; i < datacount; i++)
    {
        if (data[i] != i)
        {
            fprintf(stderr, "Incorrect values found expected %d, got %d instead\n", i, data[i]);
            success = FALSE;
        }
    }
    if (rb->read != datacount)
    {
        fprintf(stderr, "Detected failure to update read index");
        success = FALSE;
    }
    rb->write = 0;
    if (rb_read(rb,data,datacount) == NULL)
    {
        fprintf(stderr, "Failed to read while ahead of write pointer");
        success = FALSE;
    }
    for (uint32_t i = 0; i < datacount; i++)
    {
        if (data[i] != i + datacount)
        {
            fprintf(stderr, "Incorrect values found expected %d, got %d instead", i + datacount, data[i]);
            success = FALSE;
        }
    }
    if (rb->read != datacount * 2)
    {
        fprintf(stderr, "Detected failure to update read index a second time\n");
        success = FALSE;
    }
    rb->read = 0;
    free(data);
    datacount = rb->size;
    data = malloc(datacount * sizeof(regval_t));
    if (data == NULL)
    {
        fprintf(stderr, "Failed to allocate of test memory, aborting! (2)\n");
        rb_destroy(rb_info);
        return FALSE;
    }
    if (rb_read(rb, data, rb->size) != NULL)
    {
        fprintf(stderr, "Incorrectly read entire buffer of old data\n");
        success = FALSE;
    }
    free(data);
    rb_destroy(rb_info);
    if (success)
        printf("SUCCESS\n");
    else
        printf("FAILURE\n");
    return success;
}

char test_write()
{
    /* Flush to prevent stdout and stderr to desynchronise between test cases. */
    const regval_t testdata[WRITE_DATACOUNT] = {5, 12, 1337, 8430, 123};
    fflush(NULL);
    const uint32_t bufsize = 1024;
    char success = TRUE;
    printf("Testing the ability to write to the ring buffer... ");
    /* Create ringbuffer. */
    ringbuffer_info_t *rb_info = rb_create(bufsize, BUFNAME);
    ringbuffer_t *rb = rb_info->rb;
    /* Flush here. Otherwise the buffered nature of stdout/stderr can cause double writes.
       This is because fork(3) copies the output buffers to the child process. */
    fflush(NULL);
    /* Spawn child process */
    pid_t pid = fork();
    if (pid == -1)
    {
        fprintf(stderr, "Failed to spawn child process. Aborting.\n");
        return FALSE;
    }
    else if (pid == 0) /* Child process */
    {
        /* Open existing shared memory object. */
        int shmfd = shm_open(BUFNAME, (O_RDWR), (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP));
        if (shmfd == -1)
        {
            fprintf(stderr,"child: Failed to open shared memory object. Aborting.\n");
            exit(EXIT_FAILURE);
        }
        /* Map shared memory object into process memory. */
        ringbuffer_t *rb_child = (ringbuffer_t *)mmap(NULL, RB_MEMSIZE(bufsize), (PROT_READ | PROT_WRITE), MAP_SHARED, shmfd, 0);
        if (rb_child == (ringbuffer_t*) -1)
        {
            fprintf(stderr,"child: Failed to map shared object into memory. Aborting.\n");
            exit(EXIT_FAILURE);
        }
        /* Get write index of ringbuffer */
        uint32_t iwrite = rb->write;
        /* Write to ringbuffer */
        rb_write(testdata, rb_child);
        /* Quick check to see whether write did anything. */
        if (iwrite == rb->write)
        {
            fprintf(stderr, "child: Write seems to have failed.\n");
        }
        /* Unmap and unlink shared memory object.
        Note: Since the object is still linked to the parent process,
          it is not destroyed system-wide. */
        if ((shm_unlink(BUFNAME) == -1) | (munmap(rb_child, RB_MEMSIZE(bufsize)) == -1))
        {
            fprintf(stderr, "child: Detach from ringbuffer failed.\n");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }
    else /* Parent process */
    {
        /* Variable to store child exit status. */
        int childstatus;
        /* Wait for child to exit. */
        waitpid(pid, &childstatus, 0);
        /* Check child exit status. */
        if (WIFEXITED(childstatus) && WEXITSTATUS(childstatus) == EXIT_SUCCESS)
        {
            /* Create array for actual read data data */
            regval_t actual_data[WRITE_DATACOUNT];
            if (rb_read(rb, actual_data, WRITE_DATACOUNT) == NULL)
            {
                fprintf(stderr, "Read from ringbuffer failed.\n");
                success = FALSE;
            }
            /* Check whether read matches the expectation. */
            else if (memcmp((void*)testdata, (void*)actual_data, WRITE_DATACOUNT * sizeof(regval_t)) != 0)
            {
                /* Print details of the error if read does not match expectation. */
                fprintf(stderr, "Data read from ringbuffer is:\n0x");
                for (uint32_t i = 0; i < WRITE_DATACOUNT; i++)
                    fprintf(stderr, "%08x", actual_data[i]);
                fprintf(stderr, "\nData expected in ringbuffer was:\n0x");
                for (uint32_t i = 0; i < WRITE_DATACOUNT; i++)
                    fprintf(stderr, "%08x", testdata[i]);
                fprintf(stderr, "\n");
                success = FALSE;
            }
        }
        else
        {
            fprintf(stderr, "Child process failed. Check its output.\n");
            success = FALSE;
        }
    }
    rb_destroy(rb_info);
    if (success)
        printf("SUCCESS\n");
    else
        printf("FAILURE\n");
    return success;
}

int main(void)
{
    return !(test_create(1) && test_destroy() && test_create(1024) && test_create(1024*1024) && test_create_zero() && test_create_noname() /*&& test_create_nomem()*/ && test_read() && test_write());
}
