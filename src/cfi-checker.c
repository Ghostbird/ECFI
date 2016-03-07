#include "ringbuffer.h"     /* The ring buffer */
#include <stdio.h>          /* printf(), fprintf(), sprintf() */
#include <fcntl.h>          /* fcntl() */
#include <stdlib.h>         /* stderr, exit() and macros */
#include <errno.h>          /* errno variable for error handling. */
#include <string.h>         /* strlen() */
#include <strings.h>        /* index(), rindes() */
#include <signal.h>         /* kill() */
#include <sys/wait.h>       /* waitpid() */
#include <sys/types.h>      /* signal macros */
#include <unistd.h>         /* fork() */
#include <sched.h>          /* sched_yield() */
#include "errors.h"         /* translate and print errno as human readable error message. */
#include "cfi-heuristics.h" /* cfi-checker heuristics. */

#define RB_PREFIX "rb_cfi_"

void checker(ringbuffer_info_t *rb_info)
{
    regval_t data[WRITE_DATACOUNT];
    /* Infinite loop */
    while(1)
    {
        /* Read all available data. */
        while (rb_read(rb_info->rb, data, WRITE_DATACOUNT) != NULL)
        {
            cfi_print(data);
        }
    sched_yield();
    }
}

int main(int argc, char *argv[])
{
    /* It's nice for debugging to echo the input. Remove later. */
    printf("Checker ran with arguments:\n");
    for (int i = 0; i < argc; i++)
    {
        printf(" %i: %s\n", i, argv[i]);
    }
    /* No arguments are supplied. We need at least one.*/
    if (argc == 1)
    {
        /* Print usage hint and exit. */
        fprintf(stderr, "USAGE: %s <program to run + arguments>\n",argv[0]);
        exit(EXIT_FAILURE);
    }
    /* Allocate space to make a nice name for the ringbuffer.*/
    char* basename;
    if (index(argv[1], '/') == NULL)
    {
        /* No forwardslashes in the name. Use full name. */
        basename = argv[1];
    }
    else
    {
        /* Start after the last forwardslash. */
        basename = rindex(argv[1], '/') + 1;
    }
    size_t namesize = strlen("cfi-buffer") + strlen(basename) + 1;
    char *name = malloc(namesize);
    if (name == NULL)
    {
        fprintf(stderr, "Failed to allocate memory. Aborting...\n");
        exit(EXIT_FAILURE);
    }
    if (snprintf(name, namesize, "%s%s", RB_PREFIX, basename) >= (int)namesize)
    {
        fprintf(stderr, "Name was truncated but this should not happen. Aborting...\n");
        exit(EXIT_FAILURE);
    }
    /* Add terminating null character to name. */
    name[namesize-1] = '\0';
    /* Create the ring buffer for the checker. */
    ringbuffer_info_t *rb_info = rb_create(256, name);
    if (rb_info == NULL)
    {
        fprintf(stderr, "Could not create ringbuffer. Aborting...\n");
        exit(EXIT_FAILURE);
    }
    /* Flush to all standard file descriptors to prevent double output due to fork() */
    fflush(NULL);
    pid_t checker_pid = fork();
    if (checker_pid == -1)
    {
        fprintf(stderr, "Failed to spawn child process for checker. Aborting...\n");
    }
    else if (checker_pid == 0)
    {
        printf("Checker is starting.\n");
        fflush(NULL);
        /* We'll lose connection to the shell when the parent process exits.
           Redirect stdout and stderr to logs. */
        freopen("checker.out","w",stdout);
        freopen("checker.err","w",stderr);
        /* Start the checker. */
        checker(rb_info);
        /* Unreachable code. Checker does not return, it is terminated. */
        fprintf(stderr, "This is impossible, checker() should not return.");
        exit(EXIT_FAILURE);
    }
    /* Flush to all standard file descriptors to prevent double output due to fork() */
    fflush(NULL);
    pid_t program_pid = fork();
    if (program_pid == -1) /* Fork failed */
    {
        fork_error_msg(errno);
        fprintf(stderr, "Failed to spawn child process for program. Aborting...\n");
    }
    else if (program_pid == 0) /* Child process. */
    {
        if (fcntl(rb_info->fd, F_SETFD, fcntl(rb_info->fd, F_GETFD) & ~FD_CLOEXEC) == -1)
        {
            fprintf(stderr, "Failed to remove FD_CLOEXEC flag on fd %i. Aborting...\n", rb_info->fd);
        }
        else
        {
            printf("Running %s\n", argv[1]);
            fflush(NULL);
            /* We'll lose connection to the shell when the parent process exits.
            Redirect stdout and stderr to logs. */
            freopen("writer.out","w",stdout);
            freopen("writer.err","w",stderr);

            /* Execute program supplied in arguments. */
            execvp(argv[1], &argv[1]);
            fprintf(stderr, "This is impossible. execv() should have replaced the execution context.\n");
        }
    }
    else
    {
        /* Here the parent process exits and the children are attached to the system process. */
        printf("Parent process exits now.\n");
        exit(EXIT_SUCCESS);
    }
    fprintf(stderr, "Something has gone wrong. Killing child.\n");
    /* If we're here, something went wrong. Stop the checker. */
    int killret = kill(checker_pid, SIGTERM);
    switch (killret)
    {
        case 0:
        /* Wait for the checker to stop. */
        waitpid(program_pid, NULL, 0);
        break;
        case ESRCH:
        /* Child has already terminated. */
        break;
        case EINVAL:
        case EPERM:
        default:
        fprintf(stderr, "Failed to kill checker process during abort. You may have to kill it manually.\n");
    }
    exit(EXIT_FAILURE);
    /* Unreachable, will compiler detect it? */
    return 0;
}
