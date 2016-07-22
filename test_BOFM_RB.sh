#!/bin/sh
make bin/cfi-checker
# Check if target needs to be remade.
make -q asm/BOFM_RB.s
remake=$?
if [ $remake -eq 1 ]:
then
    make asm/BOFM_RB.s
    python3 injector.py asm/BOFM_RB.s dot/BOFM_RB.dot asm/BOFM_RBi.s
    make asm_to_bin/BOFM_RBi
fi
rm /dev/shm/rb_cfi_BOFM_RBi
LD_PRELOAD=bin/lib/libringbuffer.so bin/cfi-checker asm_to_bin/BOFM_RBi cfg/BOFM_RB.bin
cp /dev/shm/rb_cfi_BOFM_RBi ./testbuffer2.bin
