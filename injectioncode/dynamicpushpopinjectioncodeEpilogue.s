        LDR     r0, =[rb_writer_buffer] @ring buffer beginning address
        LDR     r0, [r0]
        MOV     r3, r0
        LDR     r2, =HOTSITEIDHERE
        SUB     r1, r0, #8  @calculating ringbuffer write offset address. Avoding memory pool
        LDR     r1, [r1]        @load the value of write offset
        ADD     r0, r1          @add the W+Buff
        MOV     r1,LR
        STMIA   r0!, {r2,r1}   @push the values of dereferenced LR and HOTSITEID(r1) to RB and increment the r0 with 8 bytes
        ADD     r1, r3, #1020 @End of Ring buffer is now in R1
        MOV     r2, r3 @ Begining of ring buffer no need if you are only using the r2 or r3
        TEQ     r1, r0          @is it end of buffer?, -- Line 4
        MOVEQ   r0, r2          @if yes update the r0   -- Line 5.
        SUB     r1, r2, #12
        LDR     r1, [r1]
        ADD     r2,r1, r2       @ rb_addr+r and store it in r2. Needs opti, run without res of TEQ --Line 6        
        TEQ     r0, r2  @if buffer+r = r0 then -- Line 6
        SUBEQ   r2, r2, r1 @calculating the RB addr avoiding load from memory pool--Line 6
        ADDEQ   r2, r2, #8 @W+20(size of buff)
        SUBEQ   r2, r0, r2 @r2 now contain r0-buffer+size -- Line 6
        SUBEQ   r1, r3, #12 @ Load the value of the Read offset
        STREQ   r2, [r1]  @ --line 7 and end of line 7 :P
        SUB     r1, r3, #8 @ Load the value of the Write offset- avoiding memory pool
        SUB     r0, r0, r3 @R3 actually holding the RB addres
        STR     r0, [r1] @fixed seg fault with additional LDR in line 20
