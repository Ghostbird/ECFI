#include "ringbuffer.h"
#include <sys/mman.h>    /* shm_open(), shm_unlink() and mmap() */
#include <sys/stat.h>    /* Mode constraints for shm_open() */
#include <fcntl.h>       /* Flag values for O_ constants for shm_open() */
#include <unistd.h>      /* ftruncate() and close() */
#include <errno.h>       /* errno */
#include <stdio.h>       /* fprintf() and stderr */

/*! Modulus calculation for positive divisors.
    - \a a The dividend
    - \a b The divisor, cannot be zero
*/
#define MOD(a,b) ((((a) % (b)) + (b)) % (b))

/*! This function converts an \var errno set during \func shm_open() or \func shm_unlink() to a human readable error message on \var stderr. */
void shm_error_msg(int errornum)
{
    switch (errornum)
    {
    /* Messages for errornum values as detailed in man page.
    Some codes from the man page are omitted since they are not applicable.*/
    case EACCES:
        fprintf(stderr,"Insufficient permissions.\n");
        break;
    case EEXIST:
        fprintf(stderr,"A shared memory object with this name already exists.\n");
        break;
    case EINVAL:
        fprintf(stderr,"Invalid name provided for shared memory object.\n");
        break;
    case EMFILE:
        fprintf(stderr,"This process has reached its limit of open file descriptors.\n");
        break;
    case ENAMETOOLONG:
        fprintf(stderr,"Name of shared memory object exceeds system's maximum path length.\n");
        break;
    case ENFILE:
        fprintf(stderr,"The system-wide limit on the total number of open file descriptors has been reached.\n");
        break;
    case ENOENT:
        fprintf(stderr,"Attempted to unlink nonexistent shared memory object.\n");
        break;
    default:
        fprintf(stderr,"An unknown error has occurred with errno: %d.\n", errornum);
    }
}

/** This function converts an \var errno set during \func mmap() to a human readable error message on \var stderr. */
void mmap_error_msg(int errornum)
{
    switch (errornum)
    {
    /* Messages for errornum values as detailed in man page.
    Some codes from the man page are omitted since they are not applicable.*/
    case EACCES:
        fprintf(stderr,"Insufficient permissions.\n");
        break;
    case EAGAIN:
        fprintf(stderr,"The file has been locked or too much memory has been locked.\n");
        break;
    case EBADF:
        fprintf(stderr,"Invalid file descriptor.\n");
        break;
    case EINVAL:
        /* We can exclude not liking address and offset since we don't use either. */
        fprintf(stderr,"We don't like invalid length (too large or zero).\n");
        break;
    case ENFILE:
        fprintf(stderr,"The system-wide limit on the total number of open file descriptors has been reached.\n");
        break;
    case ENODEV:
        fprintf(stderr,"The underlying filesystem of the file does not support memory mapping.\n");
        break;
    case ENOMEM:
        fprintf(stderr,"No memory is available or the process' maximum number of mappings would have been exceeded.\n");
        break;
    case EPERM:
        fprintf(stderr,"The operation was prevented by a file seal.\n");
        break;
    case EOVERFLOW:
        fprintf(stderr,"The lenght + offset would overflow the unsigned long.\n");
        break;
    default:
        fprintf(stderr,"An unknown error has occurred with errno: %d.\n", errornum);
    }
}

/** This function converts an \var errno set during \func ftruncate() to a human readable error message on \var stderr. */
void ftrunc_error_msg(int errornum)
{
    /* Messages for errornum values as detailed in man page.
    Some codes from the man page are omitted since they are not applicable.*/
    switch (errornum)
    {
    case EACCES:
        fprintf(stderr,"Insufficient permissions.\n");
        break;
    case EBADFD:
        fprintf(stderr,"The given file descriptor is invalid or the file is not open for writing.\n");
        break;
    case EFBIG:
        fprintf(stderr,"The argument length is larger than the maximum file size.\n");
        break;
    case EINTR:
        fprintf(stderr,"While blocked waiting to complete, the call was interrupted by a signal handler.\n");
        break;
    case EINVAL:
        fprintf(stderr,"The argument length is negative or larger than the maximum file size, the file is not open for writing, or the file descriptor does not reference a regular file.\n");
        break;
    case EIO:
        fprintf(stderr,"An I/O error occurred.\n");
        break;
    case EPERM:
        fprintf(stderr,"The underlying filesystem does not support extending a file beyond its current size, or the operation was prevented by a file seal.\n");
        break;
    default:
        fprintf(stderr,"An unknown error has occurred with errno: %d.\n", errornum);
    }
}

/** This function converts an \var errno set during \func close() to a human readable error message on \var stderr. */
void close_error_msg(int errornum)
{
    switch (errornum)
    {
    case EBADF:
        fprintf(stderr,"Invalid file descriptor.\n");
        break;
    case EINTR:
        fprintf(stderr,"Call was interrupted by a signal.\n");
        break;
    case EIO:
        fprintf(stderr,"An I/O error occurred.\n");
        break;
    default:
        fprintf(stderr,"An unknown error has occurred with errno: %d.\n", errornum);
    }
}



ringbuffer_t *rb_create(uint32_t bufsize)
{
    /* Bail out in case of idiotic request */
    if (bufsize == 0) return NULL;

    ringbuffer_t *rb = NULL;
    /* Open shared memory object. */
    int shmfd = shm_open(SHMNAME, (O_RDWR | O_CREAT | O_EXCL), (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP));
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
    if (rb == NULL && shm_unlink(SHMNAME) == -1)
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

void rb_destroy(ringbuffer_t *rbptr)
{
    /* Unmap the memory allocated to the buffer. */
    if (munmap(rbptr, rbptr->size + sizeof(ringbuffer_t)) == -1)
    {
        fprintf(stderr, "rb_destroy(): Failed to unmap memory. MEMORY LEAK!\n");
    }
    /* Unlink shared memory object. */
    if (shm_unlink(SHMNAME) == -1)
    {
        fprintf(stderr, "rb_destroy(): Failed to unlink shared memory object. Next attempt to shm_open(%s) will fail!\n",SHMNAME);
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

void rb_write(regval_t r0, regval_t r1, regval_t r2, regval_t r3, regval_t pc, regval_t lr, regval_t fp, regval_t sp, ringbuffer_t *rbptr)
{
    /* Optimise the writing. Better just unroll the write loop and write the arguments explicitly. */
    regval_t data[] = {r0, r1, r2, r3, pc, lr, fp, sp};
    /* Bail out on stupid input */
    if (rbptr == NULL)
    {
        fprintf(stderr, "rb_write(): Write failed, cannot write to NULL pointer");
        return;
    }
    /* Upcast to avoid uint wrapping when rbptr->read - rbptr->write < 0 */
    int64_t upcast_write = (int64_t)rbptr->write;
    /* Upcast to avoid uint wrapping when rbptr->read + count > UINT32_MAX */
    int64_t upcast_read  = (int64_t)rbptr->read;
    if ((uint32_t)(MOD((upcast_read - upcast_write), rbptr->size)) >= WRITE_DATACOUNT)
    {
        /* Copy register values one by one.
        Can maybe be optimised. */
        for (uint32_t i = 0; i < WRITE_DATACOUNT; i++)
        {
            rbptr->start[(uint32_t)((upcast_write + i) % rbptr->size)] = data[i];
        }
        /* Update write index. */
        rbptr->write = (uint32_t)((upcast_write + WRITE_DATACOUNT) % rbptr->size);
    }
    else
    {
        fprintf(stderr, "rb_write(): Write failed, no space in buffer.\n");
    }
    return;
}
