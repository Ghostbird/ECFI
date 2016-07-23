        PUSH    {r0, r1}             @
        SUB     r1, pc, #4           @ Subtract 4 from PC to get start of main.
        LDR	r0, =MAINADDRESSHERE @ Main address in CFG injected here.
        BL      [cfg_setup]          @ Execute function cfg_offset
        BL      [rb_init_writer]     @ Execute function rb_init_writer
        POP	{r0, r1}             @
