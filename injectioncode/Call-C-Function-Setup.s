        PUSH    {r0, r1, r3}         @
        SUB     r1, pc, #4           @ Subtract 4 from PC to get start of main.
        LDR	r0, =MAINADDRESSHERE @ Main address in CFG injected here.
        BL      r3, cfg_setup        @ Execute function cfg_offset
        POP	{r0, r1, r3}         @
