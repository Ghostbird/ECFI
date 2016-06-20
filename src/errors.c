#include <errno.h>       /* Error number macros */
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
        fprintf(stderr,"An unknown error has occurred during shm_open() or shm_unlink() with errno: %i.\n", errornum);
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
        fprintf(stderr,"An unknown error has occurred during mmap() with errno: %i.\n", errornum);
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
        fprintf(stderr,"An unknown error has occurred during ftruncate() with errno: %i.\n", errornum);
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
        fprintf(stderr,"An unknown error has occurred with errno: %i.\n", errornum);
    }
}

void fork_error_msg(int errornum)
{
    switch (errornum)
    {
    case EAGAIN:
        fprintf(stderr,"Either a kernel imposed limit on the number of threads has been reached or the caller is operating under the SCHED_DEADLINE scheduling policy and does not have the reset-on-fork flag set.  See sched(7).\n");
        break;
    case ENOMEM:
        fprintf(stderr,"fork() failed to allocate the necessary kernel structures because memory is tight.");
        break;
    case ENOSYS:
        fprintf(stderr,"fork() is not supported on this platform.");
        break;
    default:
        fprintf(stderr,"An unknown error occurred during fork() with errno: %i.\n", errornum);
    }
}

void mlock_error_msg(int errornum)
{
    switch(errornum)
    {
    case ENOMEM:
        fprintf(stderr,"Error ENOMEM can be caused by many different scenario, see the man page for mlock(2).\n");
        break;
    case EPERM:
        fprintf(stderr,"The caller is not privileged, but needs privilege to perform the requested operation.\n");
        break;
    case EAGAIN:
        fprintf(stderr,"Some or all of the specified address range could not be locked.\n");
        break;
    case EINVAL:
        fprintf(stderr,"The result of addition addr+len was less than addr, or addr was not a multiple of the page size. See mlock(2).\n");
        break;
    default:
        fprintf(stderr,"An unknown error occurred during mlock() with errno: %i.\n", errornum);
    }
}

void munlock_error_msg(int errornum)
{
    switch(errornum)
    {
    case EPERM:
        fprintf(stderr,"The caller is not privileged, but needs privilege to perform the requested operation.\n");
        break;
    default:
        fprintf(stderr,"An unknown error occurred during mlock() with errno: %i.\n", errornum);
    }
}

void exec_error_msg(int errornum)
{
    switch(errornum)
    {
    case E2BIG:
        fprintf(stderr,"The total number of bytes in the environment (envp) and argument list (argv) is too large.\n");
        break;
    case EACCES:
        fprintf(stderr,"EITHER Search permission is denied on a component of the path prefix of filename or the name of a script interpreter.\n");
        fprintf(stderr,"OR The file or a script interpreter is not a regular file.\n");
        fprintf(stderr,"OR Execute permission is denied for the file or a script or ELF interpreter.\n");
        fprintf(stderr,"OR The filesystem is mounted noexec.\n");
        break;
    case EAGAIN:
        fprintf(stderr,"Having changed its real UID using one of the set*uid() calls, the caller was—and is now still—above its RLIMIT_NPROC resource limit.\n");
        break;
    case EFAULT:
        fprintf(stderr,"filename or one of the pointers in the vectors argv or envp points outside your accessible address space.\n");
        break;
    case EINVAL:
        fprintf(stderr,"An ELF executable had more than one PT_INTERP segment (i.e., tried to name more than one interpreter).\n");
        break;
    case EIO:
        fprintf(stderr,"An I/O error occurred.\n");
        break;
    case EISDIR:
        fprintf(stderr,"An ELF interpreter was a directory\n");
        break;
    case ELIBBAD:
        fprintf(stderr,"An ELF interpreter was not in a recognized format\n");
        break;
    case ELOOP:
        fprintf(stderr,"EITHER Too many symbolic links were encountered in resolving filename or the name of a script or ELF interpreter.\n");
        fprintf(stderr,"OR The maximum recursion limit was reached during recursive script interpretation.\n");
        break;
    case EMFILE:
        fprintf(stderr,"The per-process limit on the number of open file descriptors has been reached.\n");
        break;
    case ENAMETOOLONG:
        fprintf(stderr,"Argument filename is too long.\n");
        break;
    case ENFILE:
        fprintf(stderr,"The system-wide limit on the total number of open files has been reached.\n");
        break;
    case ENOENT:
        fprintf(stderr,"The file filename or a script or ELF interpreter does not exist, or a shared library needed for the file or interpreter cannot be found.\n");
        break;
    case ENOEXEC:
        fprintf(stderr,"An executable is not in a recognized format, is for the wrong architecture, or has some other format error that means it cannot be executed.\n");
        break;
    case ENOMEM:
        fprintf(stderr,"Insufficient kernel memory was available.\n");
        break;
    case ENOTDIR:
        fprintf(stderr,"A component of the path prefix of filename or a script or ELF interpreter is not a directory.\n");
        break;
    case EPERM:
        fprintf(stderr,"EITHER The filesystem is mounted nosuid, the user is not the superuser, and the file has the set-user-ID or set-group-ID bit set.\n");
        fprintf(stderr,"OR The process is being traced, the user is not the superuser and the file has the set-user-ID or set-group-ID bit set.\n");
        fprintf(stderr,"OR A \"capability-dumb\" application would not obtain the full set of permitted capabilities granted by the executable file.\n");
        break;
    case ETXTBSY:
        fprintf(stderr,"The specified executable was open for writing by one or more processes.\n");
        break;
    default:
        fprintf(stderr,"An unknown error occurred during exec*() with errno: %i.\n", errornum);
    }
}
