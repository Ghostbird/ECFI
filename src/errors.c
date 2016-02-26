#include <errno.h>       /* errno */
#include <stdio.h>       /* fprintf() and stderr */

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
