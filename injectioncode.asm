;This is not finished yet.
;LDR R5, W+BUFF-Addr-in-mem
;MOV R5, {R5}, LSL#3 ; w+buff is a relative address
MOV R6, #w
MOV R7, #r
;MOV R7, BUFF-Addr-in-mem
MOV R5, [R6, #BUFF-Addr-in-mem], LSL#3 
STMFD R5, {LR,PC,SP,FP,R1,R2,R3,R4} ; saving multiple registers to R5. The storage structure is in full decending stack mode. 
MOV R5, R5, #32 ; increase the w+buff size with 32bit
CMP R5, #Endvalue; this might not work I have to test it.
MOVNE R5,#Buff
CMP R5,[R7, #buff], LSL#3
MOV R5, #sizeof-buff, LSR#3
STRNE R5, R7
MOV R5, #-32, LSR3
STR R5,	#w
