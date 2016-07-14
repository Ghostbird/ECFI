        PUSH    {R5, R6, R8} @note that it only works in Raspberry Pi 1 because of Hardcoded ring buffer address
        LDR     r5, =0xb6fba010 @ring buffer beginning address
        LDR     r8, =HOTSITEIDHERE
        SUB     r6, r5, #8  @calculating ringbuffer write offset address. Avoding memory pool
        LDR     r6, [r6]        @load the value of write offset
        ADD     r5, r6          @add the W+Buff
        STMIA   r5!, {r0,r1,r2,r8,LR}   @push the values to the ring buffer and increment the r5 with 20 bytes, Need Hot-site ID here
        LDR     r6, =0xb6fba40c         @Load the End of buffer address
        SUB     r8, r6, #1020 @Ring buffer beginning address
        TEQ     r6, r5          @is it end of buffer?, -- Line 4
        MOVEQ   r5, r8          @if yes update the R5   -- Line 5.
        LDR     r6, =0xb6fba004 @loading address of Read offset -- starting line 6
        LDR     r6, [r6]        @Load the value of Read offset -- Line 6
        ADD     r8,r6, r8       @ rb_addr+r and store it in r8. Needs opti, run without res of TEQ --Line 6
        TEQ     r5, r8  @if buffer+r = r5 then -- Line 6
        SUBEQ   r8, r8, r6 @calculating the RB address(0xb6fba010) avoiding load from memory pool--Line 6
        ADDEQ   r8, r8, #20 @W+20(size of buff)
        SUBEQ   r8, r5, r8 @r8 now contain r5-buffer+size -- Line 6
        LDREQ   r6, =0xb6fba004 @ Load the value of the Read offset
        STREQ   r8, [r6]  @ --line 7 and end of line 7 :P
        LDR     r6, =0xb6fba008 @ Load the value of the Write offset
        MOV     r8, r6  @r8 holds rb base - avoid memory access
        ADD     r8, #8
        SUB     r5, r5, r8
        STR     r5, [r6] @fixed seg fault with additional LDR in line 20
        POP     {R5, R6, R8}
