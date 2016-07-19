        LDR	R0, =HOTSITEIDHERE @if hotsiteid is less than 1024 use MOV instead.
        MOV	R1, LR
        LDR     r3, =rb_write_attached
        BLX     r3
