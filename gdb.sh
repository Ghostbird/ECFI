#!/bin/sh
# One monstrous command to make GDB end up exactly at the main of the exectuable passed on the command line, running in bin/cfi-checker.
gdb -ex "set env LD_PRELOAD bin/lib/libringbuffer.so" -ex "file bin/cfi-checker" -ex "b main" -ex "b *0x00011108" -ex "run $@" # -ex "continue" -ex "set follow-fork-mode child" -ex "continue"
