	.arch armv6
	.eabi_attribute 28, 1	@ Tag_ABI_VFP_args
	.fpu vfp
	.eabi_attribute 20, 1	@ Tag_ABI_FP_denormal
	.eabi_attribute 21, 1	@ Tag_ABI_FP_exceptions
	.eabi_attribute 23, 3	@ Tag_ABI_FP_number_model
	.eabi_attribute 24, 1	@ Tag_ABI_align8_needed
	.eabi_attribute 25, 1	@ Tag_ABI_align8_preserved
	.eabi_attribute 26, 2	@ Tag_ABI_enum_size
	.eabi_attribute 30, 6	@ Tag_ABI_optimization_goals
	.eabi_attribute 34, 1	@ Tag_CPU_unaligned_access
	.eabi_attribute 18, 4	@ Tag_ABI_PCS_wchar_t
	.file	"BOFM_RB.c"
@ GNU C11 (Raspbian 5.4.0-4) version 5.4.0 20160609 (arm-linux-gnueabihf)
@	compiled by GNU C version 5.4.0 20160609, GMP version 6.1.0, MPFR version 3.1.4-p2, MPC version 1.0.3
@ warning: GMP header version 6.1.0 differs from library version 6.1.1.
@ GGC heuristics: --param ggc-min-expand=62 --param ggc-min-heapsize=61719
@ options passed:  -I include -imultilib . -imultiarch arm-linux-gnueabihf
@ -D _XOPEN_SOURCE=500 src/BOFM_RB.c -march=armv6 -mfloat-abi=hard
@ -mfpu=vfp -mtls-dialect=gnu -fverbose-asm
@ options enabled:  -faggressive-loop-optimizations -fauto-inc-dec
@ -fchkp-check-incomplete-type -fchkp-check-read -fchkp-check-write
@ -fchkp-instrument-calls -fchkp-narrow-bounds -fchkp-optimize
@ -fchkp-store-bounds -fchkp-use-static-bounds
@ -fchkp-use-static-const-bounds -fchkp-use-wrappers -fcommon
@ -fdelete-null-pointer-checks -fdwarf2-cfi-asm -fearly-inlining
@ -feliminate-unused-debug-types -ffunction-cse -fgcse-lm -fgnu-runtime
@ -fgnu-unique -fident -finline-atomics -fira-hoist-pressure
@ -fira-share-save-slots -fira-share-spill-slots -fivopts
@ -fkeep-static-consts -fleading-underscore -flifetime-dse
@ -flto-odr-type-merging -fmath-errno -fmerge-debug-strings -fpeephole
@ -fprefetch-loop-arrays -freg-struct-return
@ -fsched-critical-path-heuristic -fsched-dep-count-heuristic
@ -fsched-group-heuristic -fsched-interblock -fsched-last-insn-heuristic
@ -fsched-rank-heuristic -fsched-spec -fsched-spec-insn-heuristic
@ -fsched-stalled-insns-dep -fsemantic-interposition -fshow-column
@ -fsigned-zeros -fsplit-ivs-in-unroller -fstdarg-opt
@ -fstrict-volatile-bitfields -fsync-libcalls -ftrapping-math
@ -ftree-coalesce-vars -ftree-cselim -ftree-forwprop -ftree-loop-if-convert
@ -ftree-loop-im -ftree-loop-ivcanon -ftree-loop-optimize
@ -ftree-parallelize-loops= -ftree-phiprop -ftree-reassoc -ftree-scev-cprop
@ -funit-at-a-time -fverbose-asm -fzero-initialized-in-bss -marm -mglibc
@ -mlittle-endian -mpic-data-is-text-relative -msched-prolog
@ -munaligned-access -mvectorize-with-neon-quad

	.comm	rb_writer_read,4,4
	.comm	rb_writer_write,4,4
	.comm	rb_writer_buffer,4,4
	.comm	rb_writer_end,4,4
	.comm	cfg_offset,8,8
	.section	.rodata
	.align	2
.LC0:
	.ascii	"Final LR value in secret function is %p \012\000"
	.align	2
.LC1:
	.ascii	"Secret Function to execute system command...\012\000"
	.align	2
.LC2:
	.ascii	"setterm \342\200\223term linux \342\200\223foregrou"
	.ascii	"nd red -clear\000"
	.align	2
.LC3:
	.ascii	"cat /etc/passwd\000"
	.align	2
.LC4:
	.ascii	"Tango Down...\000"
	.text
	.align	2
	.global	secretfunction
	.type	secretfunction, %function
secretfunction:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
	stmfd	sp!, {fp, lr}	@,
	add	fp, sp, #4	@,,
	mov	r3, lr	@ D.5042, ecx2
	mov	r1, r3	@, D.5042
	ldr	r0, .L2	@,
	bl	printf	@
	ldr	r0, .L2+4	@,
	bl	puts	@
	ldr	r0, .L2+8	@,
	bl	system	@
	ldr	r0, .L2+12	@,
	bl	system	@
	ldr	r0, .L2+16	@,
	bl	puts	@
	mov	r0, #0	@,
	bl	exit	@
.L3:
	.align	2
.L2:
	.word	.LC0
	.word	.LC1
	.word	.LC2
	.word	.LC3
	.word	.LC4
	.size	secretfunction, .-secretfunction
	.section	.rodata
	.align	2
.LC5:
	.ascii	"LR value in vuln() function after calling gets is %"
	.ascii	"p \012\000"
	.text
	.align	2
	.global	vuln
	.type	vuln, %function
vuln:
	@ args = 0, pretend = 0, frame = 24
	@ frame_needed = 1, uses_anonymous_args = 0
	stmfd	sp!, {fp, lr}	@,
	add	fp, sp, #4	@,,
	sub	sp, sp, #24	@,,
	str	r0, [fp, #-24]	@ arg, arg
	sub	r3, fp, #16	@ tmp111,,
	mov	r0, r3	@, tmp111
	bl	gets	@
	mov	r3, lr	@ D.5048, ecx2
	mov	r1, r3	@, D.5048
	ldr	r0, .L5	@,
	bl	printf	@
	mov	r0, r0	@ nop
	sub	sp, fp, #4	@,,
	@ sp needed	@
	ldmfd	sp!, {fp, pc}	@
.L6:
	.align	2
.L5:
	.word	.LC5
	.size	vuln, .-vuln
	.align	2
	.global	dummya
	.type	dummya, %function
dummya:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	@ link register save eliminated.
	str	fp, [sp, #-4]!	@,
	add	fp, sp, #0	@,,
	sub	sp, sp, #12	@,,
	str	r0, [fp, #-8]	@ a, a
	str	r1, [fp, #-12]	@ b, b
	mov	r3, #5	@ D.5050,
	mov	r0, r3	@, <retval>
	sub	sp, fp, #0	@,,
	@ sp needed	@
	ldr	fp, [sp], #4	@,
	bx	lr	@
	.size	dummya, .-dummya
	.align	2
	.global	dummyb
	.type	dummyb, %function
dummyb:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	@ link register save eliminated.
	str	fp, [sp, #-4]!	@,
	add	fp, sp, #0	@,,
	sub	sp, sp, #12	@,,
	str	r0, [fp, #-8]	@ a, a
	str	r1, [fp, #-12]	@ b, b
	mov	r3, #5	@ D.5051,
	mov	r0, r3	@, <retval>
	sub	sp, fp, #0	@,,
	@ sp needed	@
	ldr	fp, [sp], #4	@,
	bx	lr	@
	.size	dummyb, .-dummyb
	.align	2
	.global	func1
	.type	func1, %function
func1:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	stmfd	sp!, {fp, lr}	@,
	add	fp, sp, #4	@,,
	sub	sp, sp, #8	@,,
	str	r0, [fp, #-8]	@ fp, fp
	ldr	r3, [fp, #-8]	@ tmp110, fp
	mov	r1, #6	@,
	mov	r0, #5	@,
	blx	r3	@ tmp110
	sub	sp, fp, #4	@,,
	@ sp needed	@
	ldmfd	sp!, {fp, pc}	@
	.size	func1, .-func1
	.section	.rodata
	.align	2
.LC6:
	.ascii	"The value of ring buffer read pointer is %p \012\000"
	.align	2
.LC7:
	.ascii	"The value of ring buffer write pointer is %p \012\000"
	.align	2
.LC8:
	.ascii	"The value of ring buffer pointer is %p \012\000"
	.align	2
.LC9:
	.ascii	"The value of ring buffer end is %p \012\000"
	.align	2
.LC10:
	.ascii	"LR value before calling vuln() in main() is %p \012"
	.ascii	"\000"
	.align	2
.LC11:
	.ascii	"LR value after calling vuln() in main() is %p \012\000"
	.align	2
.LC12:
	.ascii	"Normal Execution, No Buffer Overflow Occurred.\000"
	.text
	.align	2
	.global	main
	.type	main, %function
main:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	stmfd	sp!, {fp, lr}	@,
	add	fp, sp, #4	@,,
	sub	sp, sp, #8	@,,
	str	r0, [fp, #-8]	@ argc, argc
	str	r1, [fp, #-12]	@ argv, argv
	bl	rb_init_writer	@
	ldr	r3, .L15	@ tmp120,
	ldr	r3, [r3]	@ D.5052, rb_writer_read
	mov	r1, r3	@, D.5052
	ldr	r0, .L15+4	@,
	bl	printf	@
	ldr	r3, .L15+8	@ tmp121,
	ldr	r3, [r3]	@ D.5052, rb_writer_write
	mov	r1, r3	@, D.5052
	ldr	r0, .L15+12	@,
	bl	printf	@
	ldr	r3, .L15+16	@ tmp122,
	ldr	r3, [r3]	@ D.5053, rb_writer_buffer
	mov	r1, r3	@, D.5053
	ldr	r0, .L15+20	@,
	bl	printf	@
	ldr	r3, .L15+24	@ tmp123,
	ldr	r3, [r3]	@ D.5053, rb_writer_end
	mov	r1, r3	@, D.5053
	ldr	r0, .L15+28	@,
	bl	printf	@
	ldr	r0, .L15+32	@,
	bl	func1	@
	mov	r3, lr	@ D.5054, ecx3
	mov	r1, r3	@, D.5054
	ldr	r0, .L15+36	@,
	bl	printf	@
	ldr	r3, [fp, #-12]	@ tmp124, argv
	add	r3, r3, #4	@ D.5055, tmp124,
	ldr	r3, [r3]	@ D.5056, *_15
	mov	r0, r3	@, D.5056
	bl	vuln	@
	mov	r3, lr	@ D.5054, ecx2
	mov	r1, r3	@, D.5054
	ldr	r0, .L15+40	@,
	bl	printf	@
	ldr	r0, .L15+44	@,
	bl	puts	@
	mov	r3, #0	@ D.5054,
	mov	r0, r3	@, <retval>
	sub	sp, fp, #4	@,,
	@ sp needed	@
	ldmfd	sp!, {fp, pc}	@
.L16:
	.align	2
.L15:
	.word	rb_writer_read
	.word	.LC6
	.word	rb_writer_write
	.word	.LC7
	.word	rb_writer_buffer
	.word	.LC8
	.word	rb_writer_end
	.word	.LC9
	.word	dummya
	.word	.LC10
	.word	.LC11
	.word	.LC12
	.size	main, .-main
	.ident	"GCC: (Raspbian 5.4.0-4) 5.4.0 20160609"
	.section	.note.GNU-stack,"",%progbits
