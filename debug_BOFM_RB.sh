#!/bin/sh
make DEBUG=-ggdb bin/cfi-checker
# Check if target needs to be remade.
#make -q asm/BOFM_RB.s
#remake=$?
#if [ $remake -eq 1 ]:
#then
    make asm/BOFM_RB.s DEBUG=-ggdb
    python3 injector.py asm/BOFM_RB.s dot/BOFM_RB.dot asm/BOFM_RBi.s
    make DEBUG=-ggdb asm_to_bin/BOFM_RBi
#fi
rm /dev/shm/rb_cfi_BOFM_RBi
./gdb.sh asm_to_bin/BOFM_RBi cfg/BOFM_RB.bin
