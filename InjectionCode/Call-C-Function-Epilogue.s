        LDR R0, HOTSITEID @if hotsiteid is less than 1024 use MOV instead.
        LDR R1, [LR]
        BL     0xADDRESSofOurFunction