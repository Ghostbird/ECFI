;This is not finished yet.
;LDR R5, W+BUFF-Addr-in-mem
;MOV R5, {R5}, LSL#3 ; w+buff is a relative address
MOV R5, #W+BUFF-Addr-in-mem, LSL#3 
STMFD R5, {LR,PC,SP,FP,R1,R2,R3,R4} ; saving multiple registers to R5. The storage structure is in full decending stack mode. 
MOV R5, R5, #32 ; increase the w+buff size with 32bit
CMP R5, #Endvalue
MOVNE R5,#Buff
CMP R5,#r+buff, LSL#3
STRNE R5#sizeof-buff,LSR3, #r
STR R5#-5,LSR3,	#w
