        PUSH    {R5, R6, R8} @note that it only works in Raspberry Pi 1 because of Hardcoded ring buffer address
        LDR     r5, =[rb_writer_buffer] @ring buffer beginning address
        LDR     r5, [r5]
        LDR     r8, =HOTSITEIDHERE
        SUB     r6, r5, #8  @calculating ringbuffer write offset address. Avoding memory pool
        LDR     r6, [r6]        @load the value of write offset
        ADD     r5, r6          @add the W+Buff
        MOV     r6,LR
        STMIA   r5!, {r8,r6}   @push the values of dereferenced LR and HOTSITEID(R6) to RB and increment the r5 with 8 bytes
        LDR     r6, =[rb_writer_end]      @Load the End of buffer address
        LDR     r6, [r6]
        SUB     r8, r6, #1020 @Ring buffer beginning address
        TEQ     r6, r5          @is it end of buffer?, -- Line 4
        MOVEQ   r5, r8          @if yes update the R5   -- Line 5.
        LDR     r6, =[rb_writer_read] @loading address of Read offset -- starting line 6
        LDR     r6, [r6]        @Load the value of Read offset -- Line 6
        LDR     r6, [r6]        @extra dereference missed here
        ADD     r8,r6, r8       @ rb_addr+r and store it in r8. Needs opti, run without res of TEQ --Line 6
        TEQ     r5, r8  @if buffer+r = r5 then -- Line 6
        SUBEQ   r8, r8, r6 @calculating the RB addr avoiding load from memory pool--Line 6
        ADDEQ   r8, r8, #8 @W+20(size of buff)
        SUBEQ   r8, r5, r8 @r8 now contain r5-buffer+size -- Line 6
        LDREQ   r6, =[rb_writer_read] @ Load the value of the Read offset
        LDREQ   r6, [r6]
        STREQ   r8, [r6]  @ --line 7 and end of line 7 :P
        LDR     r6, =[rb_writer_write] @ Load the value of the Write offset
        LDR     r6, [r6]
        MOV     r8, r6  @r8 holds rb base - avoid memory access
        ADD     r8, #8
        SUB     r5, r5, r8
        STR     r5, [r6] @fixed seg fault with additional LDR in line 20
        POP     {R5, R6, R8}
