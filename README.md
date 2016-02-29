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
At the moment there's no main application yet.

## Testing
To compile and test all current functionality, run:
 make runtests

## Optimisation points:
- Use thread instead of separate process for the checker
- Assume library calls are well-behaved and remove certain checks in ringbuffer.c:rb\_read()

## Known issues:
- rb_destroy is too aggressive. This will cause problems in certain cases. Needs to be fixed.
- If we load the ringbuffer library into an application, will our mmap conflict with its own memory allocation?
