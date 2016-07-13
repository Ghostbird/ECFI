;  This sections are needed for Cortex M7 which expects

Stack_Size      EQU     0x00000400

                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
__initial_sp


                PRESERVE8


; Vector Table Mapped to Address 0 at Reset

                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors
                EXPORT  __Vectors_End
                EXPORT  __Vectors_Size

__Vectors       DCD     __initial_sp              ; Top of Stack
                DCD     Reset_Handler             ; Reset Handler
__Vectors_End

__Vectors_Size  EQU     __Vectors_End - __Vectors


                AREA    |.text|, CODE, READONLY

; Reset Handler
				EXPORT Reset_Handler

Reset_Handler	

; Setup needed structure
Base			EQU		0xb6fb9000
R				EQU		0x02
W				EQU		0x06
Buffer			EQU		0x0A
R_Abs			EQU		Base+R
W_Abs			EQU		Base+W
Buffer_Abs		EQU		Base+Buffer
EndBuf			EQU		0xb6fba02e
	
				LDR		r5, =Base ; Uses literal pool
				LDR		r6, =0x10
				STR		r6, [r5, #R]
				LDR		r6, =0x12
				STR		r6, [r5, #W]

; Start tasks
; Task 1
				LDR 	r5, =Buffer_Abs ; Uses literal pool
				LDR		r6, =W_Abs ; Uses literal pool
				LDR		r6, [r6]
				ADD		r5, r6
				
; Tasks 2 and 3
				STMIA 	r5!, {r0-r3,lr} ; Adds 20 to r5 after storing registers
				; All the following instructions increment r5 by 4 after storing register
				STR		sp, [r5], #4 ; sp can not be in STMIA reg_list (http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0552a/BABCAEDD.html)
				STR		fp, [r5], #4 ; fp can be in STMIA reg_list, but is here for maintaining the same order specified in the pdf (fp after sp)
				; I can not store pc, I get this: main.s(62): error: A1477E: This register combination results in UNPREDICTABLE behaviour
				;STR		pc, [r5], #4 ; pc can not be in STMIA reg_list
				
; Task 4 and 5
				LDR		r6, =EndBuf ; Uses literal pool
				TEQ		r5, r6
				LDREQ	r5, =Buffer_Abs ; Uses literal pool
				
; Task 6 and 7
				; '((value of R) + Buffer) != R5' is equivalent to '(value of R) != (R5 - Buffer)'
				; Thus, calculate (R5 - Buffer) since I need it more times
				LDR		r6, =Buffer_Abs
				SUB		r6, r5, r6 ; r6 = (r5 - Buffer)
				; Now reuse r5 to store address of R (can I alter r5?)
				; Both r5 and r6 are needed later, use r7 for value of R
				LDR		r5, =R_Abs
				LDR		r7, [r5]
				TEQ		r7, r6 ; '(value of R) != (R5 - Buffer)'
				; (r5 + sizeof(R0, R1, R2, R3, LR, PC, FP, SP) - (Buffer value)) is equivalent to '(r5 - Buffer) + sizeof(...)'
				; Reuse r7 to store '(r5 - Buffer) + sizeof(...)'
				ADDNE	r7, r6, #28 ; sizeof(...) is 28 and not 32 because I'm not considering 'pc' which I could not store into the buffer
				STRNE	r7, [r5] ; store value in R

; Task 8
				LDR		r5, =W_Abs
				STR		r6, [r5]


Stop			B	Stop
				

				END
	