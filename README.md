# ECFI-178B
Backward Edge Control Flow Integrity for Real-Time Embedded Systems

## Prerequisites
A (non-exhaustive) list of software necessary to build the project

- gcc
- make
- doxygen (optional, documentation)
- graphviz (optional, documentation)

## Building
At the moment there's no main application yet.

## Testing
To compile and test all current functionality, run:
 make runtests

## Optimisation points:
- Map shared memory object on stack instead of heap in ringbuffer.c:rb\_create()
- Use thread instead of separate process for the checker
- Use direct memcpy instead of library function for write
- Assume library calls are well-behaved and remove certain checks in ringbuffer.c:rb\_read() and rb\_write()
- Assume reads and writes have a single fixed size
  - Simplify some checks
  - Assume the buffer size is a multiple of this fixed size
    - Treat reads and writes as acting on linear memory and memcpy them at once.
- Don't use array assignment and for loop in rb\_write(). Unroll the loop and skip the array.

## Known issues:
- rb_destroy is too aggressive. This will cause problems in certain cases. Needs to be fixed.
- If we load the ringbuffer library into an application, will our mmap conflict with its own memory allocation?
