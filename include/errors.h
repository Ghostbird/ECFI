/*! This function converts an errno set during shm_open() or shm_unlink() to a human readable error message on stderr.
\param errornum The error number from errno. */
void shm_error_msg(int errornum);

/*! This function converts an errno set during mmap() to a human readable error message on stderr.
\param errornum The error number from errno. */
void mmap_error_msg(int errornum);

/*! This function converts an errno set during ftruncate() to a human readable error message on stderr.
\param errornum The error number from errno. */
void ftrunc_error_msg(int errornum);

/** This function converts an errno set during close() to a human readable error message on stderr. 
\param errornum The error number from errno. */
void close_error_msg(int errornum);

/** This function converts an errno set during fork() to a human readable error message on stderr. 
\param errornum The error number from errno. */
void fork_error_msg(int errornum);

/** This function converts an errno set during mlock() to a human readable error message on stderr. 
\param errornum The error number from errno. */
void mlock_error_msg(int errornum);
