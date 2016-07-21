#!/bin/sh
make bin/cfi-checker
make asm/BOFM_RB.s
python3 injector.py asm/BOFM_RB.s dot/BOFM_RB.dot asm/BOFM_RBi.s
make asm_to_bin/BOFM_RBi
rm /dev/shm/rb_cfi_BOFM_RBi
./gdb.sh asm_to_bin/BOFM_RBi cfg/BOFM_RB.bin
