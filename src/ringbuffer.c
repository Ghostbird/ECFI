#include "ringbuffer.h"
#include "errors.h"
#include <sys/mman.h>    /* shm_open(), shm_unlink(), mmap(), protection and flags macros */
#include <sys/stat.h>    /* Mode constraints for shm_open() */
#include <fcntl.h>       /* Flag values for O_ constants for shm_open() */
#include <unistd.h>      /* ftruncate() and close() */
#include <errno.h>       /* errno */
#include <stdio.h>       /* fprintf() and stderr */
#include <stdlib.h>      /* malloc() and free() */
#include <string.h>      /* memcpy() */
#include <inttypes.h>    /* Platform independent printf format specifier macros */

ringbuffer_info_t *rb_create(uint32_t bufsize, const char *bufname)
{
    /* Bail out in case of idiotic request */
    if (bufsize == 0) return NULL;

    ringbuffer_t *rb = NULL;
    ringbuffer_info_t *rb_info = NULL;

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
        fprintf(stderr,"rb_create(): Failed to truncate shared memory object to length %" PRIu32 ".\n", RB_MEMSIZE(bufsize));
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
        rb = (ringbuffer_t *)mmap(NULL, RB_MEMSIZE(bufsize), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
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
            if (mlock(rb, RB_MEMSIZE(bufsize)) == -1)
            {
                fprintf(stderr, "rb_create(): Failed to lock buffer pages into memory. Performance may be suboptimal.\n");
                mlock_error_msg(errno);
            }
            /* Fill the struct with data */
            rb->size = bufsize;
            rb->read = 0;
            rb->write = 0;
            rb_info = malloc(sizeof(ringbuffer_info_t));
            /* Write data only if rb_info allocation succeeded.
            Cleanup in case of error is done later. */
            if (rb_info != NULL)
            {
                /* Calculate size of bufname including terminating null character. */
                size_t namesize = strlen(bufname) + 1;
                rb_info->name = malloc(namesize);
                /* Check whether memory allocation for buffer name succeeded.*/
                if (rb_info->name != NULL)
                {
                    /* Do strncpy, discard result. */
                    strncpy(rb_info->name, bufname, namesize);
                    /* Verify copied string. */
                    if (strncmp(rb_info->name, bufname, namesize) == 0)
                    {
                        rb_info->rb = rb;
                        rb_info->fd = shmfd;
                    }
                    else
                    {
                        /* Failed to copy string. Try to clean up. */
                        free(rb_info->name);
                        /* Setting rb_info->name to NULL will signal cleanup further down in the function. */
                        rb_info->name = NULL;
                    }
                }
            }
        }
    }
    if (rb_info != NULL && rb_info->name == NULL)
    {
        /* Something went wrong. Try to clean up */
        free(rb_info);
        /* Setting rb_info = NULL to trigger further cleanup. */
        rb_info = NULL;
    }
    if (rb != NULL && rb_info == NULL)
    {
        /* Something went wrong. Try to clean up. */
        if (munmap(rb, RB_MEMSIZE(bufsize)) == -1)
        {
            fprintf(stderr, "rb_create(): Failed to unmap memory when trying to gracefully handle error.\n");
            /* Print error messages based on errno. */
            mmap_error_msg(errno);
            fprintf(stderr, "Program state may now be ill-defined.");
        }
        /* Set rb NULL to trigger further clean-up. */
        rb = NULL;
    }
    if (rb == NULL)
    {
        /* Something went wrong. Try to clean up. */
        /* Close shared memory object file descriptor. */
        if (close(shmfd) == -1)
        {
            fprintf(stderr, "rb_create(): Failed to close file descriptor when trying to gracefully handle error.\n");
            /* Print error messages based on errno. */
            close_error_msg(errno);
            fprintf(stderr, "Program state may now be ill-defined.");
        }
        /* Unlink shared memory. This should trigger file deletion, since we already closed it. */
        if (shm_unlink(bufname) == -1)
        {
            fprintf(stderr, "rb_create(): Failed to unlink shared memory object when trying to gracefully handle error.\n");
            /* Print error messages based on errno. */
            shm_error_msg(errno);
            fprintf(stderr, "Program state may now be ill-defined.");
        }
    }
    /* Return the pointer to the ringbuffer_info struct. */
    return rb_info;
}

void rb_destroy(ringbuffer_info_t *rb_info)
{
    /* Close file descriptor to ringbuffer object. */
    if (close(rb_info->fd) == -1)
    {
        fprintf(stderr, "rb_destroy(): Failed close fd %i.\n", rb_info->fd);
        /* Print error message based on errno. */
        close_error_msg(errno);
    }
    /* Unlink from shared memory object. */
    if (shm_unlink(rb_info->name) == -1)
    {
        fprintf(stderr, "rb_destroy(): Failed to unlink shared memory object. Next attempt to shm_open(%s) may fail!\n", rb_info->name);
        /* Print error message based on errno. */
        shm_error_msg(errno);
    }
    /* Unmap the memory allocated to the buffer. */
    if (munmap(rb_info->rb, RB_MEMSIZE(rb_info->rb->size)) == -1)
    {
        fprintf(stderr, "rb_destroy(): Failed to unmap memory. MEMORY LEAK!\n");
        /* Print error message based on errno. */
        mmap_error_msg(errno);
    }
    /* Unlink shared memory object. */
    /* Free memory allocated for the buffer name. */
    free(rb_info->name);
    /* Free memory allocated buffer information itself. */
    free(rb_info);
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
    /* Count is in objects of size sizeof(regval_t) whereas the calculation is in bytes */
    if ((uint32_t)(MOD((upcast_write - upcast_read), rbptr->size)) >= count * sizeof(regval_t))
    {
        /* Copy register values one by one.
        Use remainder operation to wrap linear memory space.
        Can most likely be optimised. */
        for (uint32_t i = 0; i < count; i++)
        {
            /* (rbptr + 1) is a pointer to the memory directly behind that used by the struct rbptr.
               That is where the real buffer begins. Cast it to regval_t*, to index the buffer.
               Divide upcast_read and rbptr->size by sizeof(regval_t) to go from byte addressing to regval_t addressing.
               This should be safe, since those values must always be a multiple of sizeof(regval_t)
            */
            data[i] = ((regval_t*)(rbptr + 1))[(uint32_t)((upcast_read / sizeof(regval_t) + i) % (rbptr->size / sizeof(regval_t)))];
        }
        /* Since reads are not atomic, and writes always go through, check whether no overwrite happened. */
        if (rbptr->read != (uint32_t)upcast_read)
        {
            /* Overwrite happened. */
            fprintf(stderr, "Buffer overwrite detected.");
            return NULL;
        }
        /* Update read index. */
        rbptr->read = (uint32_t)((upcast_read + count * sizeof(regval_t)) % rbptr->size);
    }
    else
    {
        return NULL;
    }
    return data;
}

void rb_write(ringbuffer_t *rbptr, const regval_t *data)
{
    /* (rbptr + 1) is a pointer to the memory directly behind that used by the struct rbptr.
       That is where the real buffer begins.
       Divide rbptr->write by sizeof(regval_t) to go from byte addressing to regval_t addressing.
       This should be safe, since write offset is always a multiple of sizeof(regval_t)
    */
    memcpy((void*) (((regval_t*)(rbptr + 1)) + rbptr->write / sizeof(regval_t)), (void*) data, WRITE_DATACOUNT * sizeof(regval_t));
    /* Update write index. */
    rbptr->write += WRITE_DATACOUNT * sizeof(regval_t);
    /* Fix write index if it's beyond the size of the buffer. */
    if (rbptr->write == rbptr->size)
	rbptr->write -= rbptr->size;
    /* Fix read index if it is an overwrite.
    It will point to one write operation in advance of the write index. */
    if (rbptr->write == rbptr->read)
	rbptr->read += WRITE_DATACOUNT * sizeof(regval_t);
    return;
}

ringbuffer_t *rb_attach(int fd)
{
    /* mmap the shared memory file descriptor.
       Since the size of the buffer is unknown, map only the ringbuffer_t struct.*/
    ringbuffer_t *rb = (ringbuffer_t *)mmap(NULL, sizeof(rb), (PROT_READ | PROT_WRITE), MAP_SHARED, fd, 0);
    /* Check whether memory allocation succeeded. */
    if (rb == (ringbuffer_t *) -1)
    {
        fprintf(stderr,"rb_attach(): Failed to map memory of shared object. ");
        /* Print error message based on errno. */
        mmap_error_msg(errno);
        return NULL;
    }
    /* Remap ring buffer now that the size can be read. */
    rb = (ringbuffer_t *)mmap(rb, RB_MEMSIZE(rb->size), (PROT_READ | PROT_WRITE), MAP_SHARED, fd, 0);
    /* Check whether memory allocation succeeded. */
    if (rb == (ringbuffer_t *) -1)
    {
        fprintf(stderr,"rb_attach(): Failed to remap shared object. ");
        /* Print error message based on errno. */
        mmap_error_msg(errno);
        return NULL;
    }
    if (mlock(rb, RB_MEMSIZE(rb->size)) == -1)
    {
        fprintf(stderr, "rb_attach(): Failed to lock buffer pages into memory. Performance may be suboptimal./n");
        mlock_error_msg(errno);
    }
    return rb;
}

void rb_init_writer()
{
    ringbuffer_t *rb = rb_attach(3);
    rb_writer_read = &(rb->read);
    rb_writer_write = &(rb->write);
    rb_writer_buffer = (regval_t*)(rb + 1);
    rb_writer_end = rb_writer_buffer + rb->size;
}

void rb_write_attached(int arg0, int arg1)
{
    /* Divide *rb_write_write by sizeof(regval_t) because *rb_writer_write holds an
    offset in bytes, and C pointer arithmetic wants it in sizeof(regval_t) */
    regval_t* write_location = rb_writer_buffer + *rb_writer_write / sizeof(regval_t);
    write_location[0] = arg0;
    write_location[1] = arg1;
    //fprintf(stderr, "Wrote %08x, %08x to %p\n", arg0, arg1, (void*)write_location);
    regval_t *new_write_location = write_location + WRITE_DATACOUNT;
    //fprintf(stderr, "Location for new write: %p\n", (void*) new_write_location);
    /* Wrap linear memory space. */
    if (new_write_location == rb_writer_end)
        new_write_location = rb_writer_buffer;
    /* Kick forward the read pointer on an overwrite. */
    if (rb_writer_buffer + *rb_writer_read == new_write_location)
        *rb_writer_read = (uint32_t)(new_write_location - rb_writer_buffer) + WRITE_DATACOUNT;
    /* This line confused me for a while. The difference between two pointers is calculated
    as how many objects of the pointer's type fit inbetween. NOT IN BYTES!
    Multiply by sizeof(regval_t) to get it in bytes. */
    *rb_writer_write = (uint32_t)(new_write_location - rb_writer_buffer) * sizeof(regval_t);
    //fprintf(stderr, "sizeof(regval_t) = %x\n", sizeof(regval_t));
    //fprintf(stderr, "rb_writer_buffer: %p\n", (void*)rb_writer_buffer);
    //fprintf(stderr, "rb_writer_write: %08x\n", *rb_writer_write);
}
