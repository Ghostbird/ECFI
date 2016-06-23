# ECFI-178B
Backward Edge Control Flow Integrity for Real-Time Embedded Systems

## Prerequisites
A (non-exhaustive) list of software necessary to build the project

- gcc
- make
- gcc-python3-plugin (CFG generation)
- doxygen (optional, documentation)
- graphviz (optional, documentation)
- valgrind (optional, memory usage analysis)

## Building
```bash
make
```

### Compile to assembly ###
To compile to assembly (for example the BOF4.c file) and then to binary:
```bash
make asm/BOF4.s
gcc -lrt bin/lib/libringbuffer.so bin/obj/errors.o asm/BOF4.s
```

### Cross compiling
Override the CC variable on the command line with your cross compiler:
```bash
make CC=arm-linux-gnueabihf-gcc bin/BOF4
```

## Testing
To compile and test the ringbuffer library:
```bash
make runtests
```
To test the record write/verify functionality:
1. Open src/cfi-checker.c
2. Comment lines 28 and 30. (cfi_print and cfi_check_record)
3. Uncomment line 29 (cfi_record)
4. Compile and run once.```bash
make -B bin/cfi-checker bin/injection-test
LD_PRELOAD=bin/lib/libringbuffer.so bin/cfi-checker bin/injection-test
```
5. Open src/cfi-checker.c again.
6. Comment line 29 (cfi_record)
7. Uncomment line 30 (cfi_check_record)
8. Compile again.
9. Run again.
10. Check the files checker.out and checker.err for messages. If there are no errors, the check succeeded. If the behaviour was different at some point, you will be show a summary of the differences.

To compile and test the BOF4 executable:
```bash
make -B all
LD_PRELOAD=bin/lib/libringbuffer.so bin/cfi-checker bin/BOF4
```

## Debugging
To load the cfi-checker injection test in gdb, with debug symbols:
```bash
make -B debug 
LD_PRELOAD=bin/lib/libringbuffer.so gdb --args bin/cfi-checker bin/injection-test
```
To end up in the right process, you may have to `set follow-fork-mode child` at an appropriate point in gdb.

## Running
```bash
make -B all
LD_PRELOAD=bin/lib/libringbuffer.so bin/cfi-checker <path to executable>
```

## Generate CFGs ##
Example: Compile _src/injection-test.c_ and generate CFGs for all functions.
```bash
make cfg/injection-test.cfg
```
At the moment this creates a dummy _src/injection-test.c_ file and creates CFGs for every function compiled in the _cfg_ directory. The functions are in both DOT and SVG format.
Note: If you want to cross-compile and generate CFGs you'll need the GCC Python plugin for your specific cross compiler. You'll probably have to compile it from source.

## Architecture ##
Memory layout for ring buffer:

![Please read include/ringbuffer.h if you cannot see this picture.](doc/diagrams/ringbuffer_memory.png "The memory layout of a ringbuffer in use.")

Activity diagram of the CFI-Checker.

![Please read src/cfi-checker.c if you cannot see this picture.](doc/diagrams/cfi-checker_activity.png "Activity diagram that shows how the processes interact.")

## Optimisation points:
- Use thread instead of separate process for the checker

## Known issues:
- rb_destroy is too aggressive. This will cause problems in certain cases. Needs to be fixed.
- Check correct memory usage of structs (maybe use offsetof)
- If the checker is aborted it may be necessary to manually delete the ringbuffer: ```rm /dev/shm/rb_cfi_*```
- Ring buffer is (world?) writable.
- Ringbuffer administration has critical sections. Need to verify whether violating them only causes data loss, or invalidates program invariants.

## Real-Time Features:
- Priority handling using Ringbuffer
- Lock free implementation
- Formally verifiable hard Real-Time in the injected code.

## Todo:
C function for injection:
- get previous frame pointer
- calculate function arguments
- calculate stack pointer

ANGR:
- Function arguments in CFG.
