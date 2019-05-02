; Print.s
; Student names: change this to your names or look very silly
; Last modification date: change this to the last modification date or look very silly
; Runs on LM4F120 or TM4C123
; EE319K lab 7 device driver for any LCD
;
; As part of Lab 7, students need to implement these LCD_OutDec and LCD_OutFix
; This driver assumes two low-level LCD functions
; ST7735_OutChar   outputs a single 8-bit ASCII character
; ST7735_OutString outputs a null-terminated string 

    IMPORT   ST7735_OutChar
    IMPORT   ST7735_OutString
    EXPORT   LCD_OutDec
    EXPORT   LCD_OutFix

    AREA    |.text|, CODE, READONLY, ALIGN=2
    THUMB

;-----------------------LCD_OutDec-----------------------
; Output a 32-bit number in unsigned decimal format
; Input: R0 (call by value) 32-bit unsigned number
; Output: none
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutDec
   	MOV R3, #10
   	MOV R12,#0
LOOP   UDIV R1, R0, R3	;R1=R0/10
          	CMP R1, #0   ;If R1 =0, store the value on the stack
          	BEQ DONE
          	MUL R2, R1, R3  
          	SUB R2, R0, R2	;R2 = R0%10, R2 contains the reminder
          	STR R2, [SP, R12]  ;store R2 value on the stack
          	ADD R12,#4   ;move the stack pointer to the next position
          	MOV R0,R1	;R0 is the quotient
          	B LOOP
DONE
          	STR R0,[SP,R12]
LOOP1  		LDR R0,[SP,R12]
          	ADD R0,#0X30    	;Print out the ASCII value for the numbers
          	STR LR,[SP,R12]
          	BL ST7735_OutChar
          	LDR LR,[SP,R12]
          	SUBS R12,#4      	;move the stack pointer to the next position
          	BGE LOOP1     	;check if it is the end of the array
          	BX  LR
;* * * * * * * * End of LCD_OutDec * * * * * * * *

; -----------------------LCD _OutFix----------------------
; Output characters to LCD display in fixed-point format
; unsigned decimal, resolution 0.001, range 0.000 to 9.999
; Inputs:  R0 is an unsigned 32-bit number
; Outputs: none
; E.g., R0=0,    then output "0.000 "
;       R0=3,    then output "0.003 "
;       R0=89,   then output "0.089 "
;       R0=123,  then output "0.123 "
;       R0=9999, then output "9.999 "
;       R0>9999, then output "*.*** "
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutFix
   	PUSH{R0-R4,LR}	
   	MOV R12, #9999   	;if the value is bigger than 9999, print out "*.*** "
   	CMP R0, R12
   	BHI ERROR
   	
   	MOV R3,#10
   	MOV R4,#4      	;R4 contains the count
LOOP2  UDIV R1, R0,R3  	;R1 = R0/10
          	CMP R4, #0    ;if R4 = 0, go to print (four values)
          	BEQ DONE2
          	MUL R2, R1, R3  	
          	SUB R2, R0, R2	;R2 contains the reminder, R2 = R0%10
          	PUSH{R2,LR}
          	MOV R0,R1   	; R0 is the quotient
          	SUB R4,#1   	;count = count -1
          	B LOOP2
DONE2
   	MOV R1,#4        	;R1 is the count for print
PRINT  POP{R0,LR}
          	ADD R0,#0X30  	;Print out the ASCII value for the numbers
          	PUSH{R1,LR}
          	BL ST7735_OutChar
          	POP{R1,LR}
          	SUBS R1,#1     	;print four numbers
          	MOV R2,#3
          	CMP R1,R2      	;go to print the decimal point
          	BNE SKIP
                  	MOV R0, #0x2E
                  	PUSH{R1,LR}
                  	BL ST7735_OutChar  	;print the decimal point
                  	POP{R1,LR}
SKIP   ADDS R1,#0
          	BEQ FINISH   	;go back to continue printing
          	B PRINT
          	
 
 
ERROR
   	MOV R0, #0x2A                   	;output "*.*** "
   	PUSH{R0,LR}
   	BL ST7735_OutChar
   	POP{R0,LR}
   	MOV R0, #0x2E
   	PUSH{R0,LR}
   	BL ST7735_OutChar
   	POP{R0,LR} 	
   	MOV R0, #0x2A
   	PUSH{R0,LR}
   	BL ST7735_OutChar
   	POP{R0,LR}
   	PUSH{R0,LR}
   	BL ST7735_OutChar
   	POP{R0,LR} 	
   	PUSH{R0,LR}
   	BL ST7735_OutChar
   	POP{R0,LR}
   	B FINISH
   	
FINISH
   	POP{R0-R4,LR}
   	BX   LR
 
     
;* * * * * * * * End of LCD_OutFix * * * * * * * *

     ALIGN                           ; make sure the end of this section is aligned
     END                             ; end of file