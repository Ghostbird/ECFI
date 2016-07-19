        PUSH    {r0, r1, r3}
        LDR	R0, =HOTSITEIDHERE
        LDR	r1, [FORWARDEDGELOCATION]
        LDR     r3, =rb_write_attached
        BLX     r3
        POP	{r0, r1, r3}
