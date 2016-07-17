        PUSH    {r0, r1}
        LDR R0, HOTSITEID
        LDR r1, [lr]
        BL     0xADDRESSofOurFunction
        POP    {r0, r1}