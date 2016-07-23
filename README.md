# ECFI-178B
Backward Edge Control Flow Integrity for Real-Time Embedded Systems

## Prerequisites
A (non-exhaustive) list of software necessary to build the project

- gcc
- make
- python3-pydot
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
make asm/[name of the file].s
```

### Compile to binary ###
To compile to assembly (for example the BOF4.c file) and then to binary:
```bash
make asm_to_bin/[THE-File-Name]
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
4. Compile and run once:
    <pre><code>make -B bin/cfi-checker bin/injection-test
    LD_PRELOAD=bin/lib/libringbuffer.so bin/cfi-checker bin/injection-test</code></pre>
5. Open src/cfi-checker.c again.
6. Comment line 29 (cfi_record)
7. Uncomment line 30 (cfi_check_record)
8. Compile again.
9. Run again.
10. Check the files checker.out and checker.err for messages. If there are no errors, the check succeeded. If the behaviour was different at some point, you will be show a summary of the differences.

To compile and test the BOF4 executable:
```bash
make asm_to_bin/[filename]
LD_PRELOAD=bin/lib/libringbuffer.so bin/cfi-checker bin/BOF4
```

## Debugging
To load the cfi-checker injection test in gdb, with debug symbols:
```bash
./gdb.sh asm_to_bin/Filename
```
To end up in the right process, you may have to `set follow-fork-mode child` at an appropriate point in gdb.

## Running
```bash
make -B all
LD_PRELOAD=bin/lib/libringbuffer.so bin/cfi-checker <path to executable>
```

## Generate Dot File CFG from ANGR ##
To generate CFG using angr, both angr and angrutils are required. It is recommended to install these in a virutalenv. The example below assumes that they are installed in a virtualenv named _angr_.
The angr install through pip may return an error due to a bug in libcapstone. To fix this problem, search for libcapstone in the file tree indicated by the installer, and move it to the location where the installer says it is missing. 
After the installation of both angr and angrutils, change the visualize.py code of angrutils with the modified angrutils code that generates CFG dot files instead of PNG files. To generate the dot file:
```bash
workon angr
ipython
```
Then in iPython:
```python
import angr
from angrutils import *
proj = angr.Project("BOFM", load_options={'auto_load_libs':False})
main = proj.loader.main_bin.get_symbol("main")
start_state = proj.factory.blank_state(addr=main.addr)
cfg = proj.analyses.CFGAccurate(fail_fast=True, enable_symbolic_back_traversal=True, starts=[main.addr], initial_state=start_state)
plot_cfg(cfg, "ais3_cfg", asminst=True, remove_imports=True, remove_path_terminator=True)  
```
The plot CFG will generate a file named test.dot which is the dot file of the CFG for the binary.

## Architecture ##
Memory layout for ring buffer:

![Please read include/ringbuffer.h if you cannot see this picture.](doc/diagrams/ringbuffer_memory.png "The memory layout of a ringbuffer in use.")

Activity diagram of the CFI-Checker.

![Please read src/cfi-checker.c if you cannot see this picture.](doc/diagrams/cfi-checker_activity.png "Activity diagram that shows how the processes interact.")

### ASLR compensation ###
The injected set-up code from injectioncode/Call-C-Function-Setup.s calls the function cfg_setup() from include/ringbuffer.h and passes two arguments.
These arguments are:
# The address of main() during injection.
# The address of main() at run-time.

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
