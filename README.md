# ECFI-178B
Backward Edge Control Flow Integrity for Real-Time Embedded Systems

## Prerequisites
A (non-exhaustive) list of software necessary to build the project

- gcc
- make
- doxygen (optional, documentation)
- graphviz (optional, documentation)
- valgrind (optional, memory usage analysis)

## Building
```bash
make
```

## Testing
To compile and test the ringbuffer library:
```bash
make runtests
```

## Running
```bash
    LD_PRELOAD=bin/lib bin/cfi-checker echo test
    rm /dev/shm/rt_cfi_echo
```
This last line removes the shared object that persist after the checker is finished.
See Known Issues.

## Optimisation points:
- Use thread instead of separate process for the checker
- Assume library calls are well-behaved and remove certain checks in ringbuffer.c:rb\_read()

## Known issues:
- rb_destroy is too aggressive. This will cause problems in certain cases. Needs to be fixed.
- checker process needs a SIGTERM handler that nicely destroys ring buffer in case it is kill()-ed.
- execve causes shm_unlink() and munmap() like behaviour, and this must be accounted for.
