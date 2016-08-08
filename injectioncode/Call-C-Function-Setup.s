        PUSH    {r0, r1, r3}
        BL      rb_init_writer
        LDR     R0, =10
        LDR     r1, =[main]
        LDR     r3, =rb_write_attached
        BLX     r3
        POP     {r0, r1, r3}
