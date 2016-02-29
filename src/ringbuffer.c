#include "ringbuffer.h"
#include "errors.h"
#include <sys/mman.h>    /* shm_open(), shm_unlink() and mmap() */
#include <sys/stat.h>    /* Mode constraints for shm_open() */
#include <fcntl.h>       /* Flag values for O_ constants for shm_open() */
#include <unistd.h>      /* ftruncate() and close() */
#include <errno.h>       /* errno */
#include <stdio.h>       /* fprintf() and stderr */
#include <string.h>      /* memcpy() */

ringbuffer_t *rb_create(uint32_t bufsize, const char *bufname)
{
    /* Bail out in case of idiotic request */
    if (bufsize == 0) return NULL;

    ringbuffer_t *rb = NULL;
    /* Open shared memory object. */
    int shmfd = shm_open(bufname, (O_RDWR | O_CREAT | O_EXCL), (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP));
    if (shmfd == -1)
    {
        fprintf(stderr,"rb_create(): Failed to open shared memory object. ");
        /* Print error message based on errno. */
        shm_error_msg(errno);
        fprintf(stderr,"Returning NULL immediately.\n");
        return NULL;
    }

    /* Truncate file descriptor for shared memory to the desired length.
    NOTE: ftruncate is not C99, and requires the compiler argument -D_XOPEN_SOURCE=500 */
    if (ftruncate(shmfd, RB_MEMSIZE(bufsize)) == -1)
    {
        /* Handle errors during ftruncate */
        fprintf(stderr,"rb_create(): Failed to truncate shared memory object to length %lu. ", RB_MEMSIZE(bufsize));
        /* Print error messages based on errno. */
        ftrunc_error_msg(errno);
    }
    else
    {
        /* Try to mmap the shared memory file descriptor.
        Posibilities for optimisation are:
        - MAP_ANONYMOUS (not backed to file)
        - MAP_UNINITIALISED (not zeroed before use).
        - MAP_STACK (map on stack) */
        rb = (ringbuffer_t *)mmap(NULL, RB_MEMSIZE(bufsize), (PROT_READ | PROT_WRITE), MAP_SHARED, shmfd, 0);
    	/* Check whether memory allocation succeeded. */
        if (rb == (ringbuffer_t *)-1)
    	{
            fprintf(stderr,"rb_create(): Failed to map memory of shared object. ");
            /* Print error message based on errno. */
            mmap_error_msg(errno);
            /* Reset rb as a signal to the cleanup below. */
            rb = NULL;
    	}
    	else
        {
            /* Fill the struct with data */
            rb->size = bufsize;
            /* Calculate the start position of the buffer to lie directly after the “administrative” ringbuffer struct in memory. */
            rb->start = (regval_t *)(rb + sizeof(ringbuffer_t));
            rb->read = 0;
            rb->write = 0;
        }
    }

    /* Unlink shared memory object if something failed (rb == NULL). */
    if (rb == NULL && shm_unlink(bufname) == -1)
    {
        fprintf(stderr, "rb_create(): Failed to unlink shared memory object when trying to gracefully handle error.\n");
        /* Print error messages based on errno. */
        shm_error_msg(errno);
        fprintf(stderr, "Program state may now be ill-defined.");
    }

    /* Always close fd, since we con't need it after mmap. See man shm_open(3). */
    if (close(shmfd) == -1)
    {
        fprintf(stderr,"rb_create(): Failed to close file descriptor to shared memory object");
        /* Print error message based on errno. */
        close_error_msg(errno);
        fprintf(stderr,"Program state may now be ill-defined./n");
    }
    /* Return the pointer to the ringbuffer struct. */
    return rb;
}

void rb_destroy(ringbuffer_t *rbptr, const char *bufname)
{
    /* Unmap the memory allocated to the buffer. */
    if (munmap(rbptr, rbptr->size + sizeof(ringbuffer_t)) == -1)
    {
        fprintf(stderr, "rb_destroy(): Failed to unmap memory. MEMORY LEAK!\n");
        /* Print error message based on errno. */
        mmap_error_msg(errno);
    }
    /* Unlink shared memory object. */
    if (shm_unlink(bufname) == -1)
    {
        fprintf(stderr, "rb_destroy(): Failed to unlink shared memory object. Next attempt to shm_open(%s) will fail!\n", bufname);
        /* Print error message based on errno. */
        shm_error_msg(errno);
    }
}

regval_t *rb_read(ringbuffer_t *rbptr, regval_t *data, uint32_t count)
{
    /* Bail out on stupid input */
    if ((rbptr == NULL) || (data == NULL))
        return NULL;
    /* The read is only valid if it does not read beyond the write pointer */
    /* Upcast to avoid uint wrapping when rbptr->read - rbptr->write < 0 */
    int64_t upcast_write = (int64_t)rbptr->write;
    /* Upcast to avoid uint wrapping when rbptr->read + count > UINT32_MAX */
    int64_t upcast_read  = (int64_t)rbptr->read;
    if ((uint32_t)(MOD((upcast_write - upcast_read), rbptr->size)) >= count)
    {
        /* Copy register values one by one.
        Use remainder operation to wrap linear memory space.
        Can most likely be optimised. */
        for (uint32_t i = 0; i < count; i++)
        {
            data[i] = rbptr->start[(uint32_t)((upcast_read + i) % rbptr->size)];
        }
        /* Update read index. */
        rbptr->read = (uint32_t)((upcast_read + count) % rbptr->size);
    }
    else
    {
        return NULL;
    }
    return data;
}

void rb_write(const regval_t data[8], ringbuffer_t *rbptr)
{
    /* Copy arguments to ringbuffer.*/
    memcpy((void*) &(rbptr->start[rbptr->write]), (void*) data, WRITE_DATACOUNT * sizeof(regval_t));
    /* Update write index. */
    rbptr->write += WRITE_DATACOUNT;
    /* Fix write index if it's beyond the size of the buffer. */
    if (rbptr->write == rbptr->size)
	rbptr->write -= rbptr->size;
    /* Fix read index if it is an overwrite.
    It will point to one write operation in advance of the write index. */
    if (rbptr->write == rbptr->read)
	rbptr->read += WRITE_DATACOUNT;
    return;
}

