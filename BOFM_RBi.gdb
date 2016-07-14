set env LD_PRELOAD bin/lib/libringbuffer.so
file bin/cfi-checker
b *0x0001107c
run ./BOFM_RBi
set follow-fork-mode child
b main
