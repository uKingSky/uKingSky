;*********************************************************************************************************
;                                              KingSky
;                                         实时操作系统内核
;
;                                    (c) Copyright 2011，庄桐泉
;                                         All Rights Reserved
;
;                                               ARM920T Port
;                                            ADS v1.2 Compiler
;                                            Samsung S3C2440A
;
; 文件:   : os_cpu_a.s 
; CPU :   : S3C2440
; History : 
;  OSCtxSw(), OSIntCtxSw()  OSStartHighRdy() OS_CPU_IRQ_ISR() OSTickISR()
;******************************************************************************************************** */

SRCPND   	EQU     0x4a000000    ; Source pending
INTPND   	EQU     0x4a000010    ; 中断请求状体
rEINTPEND   EQU     0x560000a8
INTOFFSET   EQU     0x4a000014

USERMODE    EQU 	0x10
FIQMODE     EQU 	0x11
IRQMODE     EQU 	0x12
SVCMODE     EQU 	0x13
ABORTMODE   EQU 	0x17
UNDEFMODE   EQU 	0x1b
MODEMASK    EQU 	0x1f
NOINT       EQU 	0xc0

	IMPORT  ks_running
	IMPORT  current_thread
	IMPORT  high_thread
	IMPORT  int_nesting
	
	IMPORT  HandleEINT0
	

	IMPORT  ks_int_enter
	IMPORT  ks_int_exit
	IMPORT  ks_time_tick
	IMPORT  ks_int_exit

	
  
	EXPORT  ks_start_high
	EXPORT  ks_cpu_save
	EXPORT  ks_cpu_restore
	EXPORT  ks_thread_sw
	EXPORT  ks_int_sw
	EXPORT  KS_CPU_IRQ_ISR 	
	EXPORT  ks_tick_isr
	
	
	AREA KS_ARM, CODE, READONLY
	
ks_start_high
;----------------------------------------------------------------------------------	
; 	ks_running = KS_TRUE;
;----------------------------------------------------------------------------------	
	MSR     CPSR_cxsf,#SVCMODE|NOINT     ;Switch to SVC mode with IRQ&FIQ disable
	LDR		R0, =ks_running              ; ks_running =KS_TRUE
	MOV		R1, #1
	STRB 	R1, [R0]
;----------------------------------------------------------------------------------		
; 	SP = current_thread->top_stack;
;----------------------------------------------------------------------------------	
	LDR 	R0, =current_thread
	LDR 	R0, [R0]         
	LDR 	SP, [R0]  
	
;----------------------------------------------------------------------------------		
; Prepare to return to proper mode
;----------------------------------------------------------------------------------	
	LDMFD 	SP!, {R0}  
	MSR 	SPSR_cxsf, R0
	LDMFD 	SP!, {R0-R12, LR, PC}^ 
	
ks_thread_sw

	STMFD	SP!, {LR}           ;PC
	STMFD	SP!, {R0-R12, LR}   ;R0-R12 LR
	MRS		R0,  CPSR           ;Push CPSR
	STMFD	SP!, {R0}	
;----------------------------------------------------------------------------------
; 		current_thread->top_stack = SP
;----------------------------------------------------------------------------------		
	LDR		R0, =current_thread
	LDR		R0, [R0]
	STR		SP, [R0]
;----------------------------------------------------------------------------------			
; current_thread = high_thread;
;----------------------------------------------------------------------------------		
	LDR		R0, =high_thread
	LDR		R1, =current_thread
	LDR		R0, [R0]
	STR		R0, [R1]
	
;----------------------------------------------------------------------------------		
;  SP = high_thread->top_stack;
;----------------------------------------------------------------------------------		
	LDR		R0, =high_thread
	LDR		R0, [R0]
	LDR		SP, [R0]
	
;----------------------------------------------------------------------------------	
;Restore New task context
;----------------------------------------------------------------------------------	
	LDMFD 	SP!, {R0}		       ;POP CPSR
	MSR 	SPSR_cxsf, R0
	LDMFD 	SP!, {R0-R12, LR, PC}^


ks_tick_isr
	
	MOV     R5,LR	
	MOV 	R1, #1
	MOV		R1, R1, LSL #10		; Timer0 Source Pending Reg.
	LDR 	R0, =SRCPND
	LDR     R2, [R0]
	ORR     R1, R1,R2
	STR 	R1, [R0]

	LDR		R0, =INTPND
	LDR		R1, [R0]
	STR		R1, [R0]
	
	 
    BL		 ks_time_tick        ;调用时钟函数
	
	
	 MOV     PC, R5        		; Return    
	
	
ks_int_sw
;----------------------------------------------------------------------------------			
; current_thread = high_thread;
;----------------------------------------------------------------------------------		
	LDR		R0, =high_thread
	LDR		R1, =current_thread
	LDR		R0, [R0]
	STR		R0, [R1]
	
;----------------------------------------------------------------------------------		
; 		SP = high_thread->top_stack;
;----------------------------------------------------------------------------------		
	LDR		R0, =high_thread
	LDR		R0, [R0]
	LDR		SP, [R0]
	
;----------------------------------------------------------------------------------	
; Restore New Task context
;----------------------------------------------------------------------------------	
	LDMFD 	SP!, {R0}              ;POP CPSR
	MSR 	SPSR_cxsf, R0
	LDMFD 	SP!, {R0-R12, LR, PC}^
	
		
	
KS_CPU_IRQ_ISR 	

	STMFD   SP!, {R1-R3}			; We will use R1-R3 as temporary registers
;----------------------------------------------------------------------------
;   R1--SP
;	R2--PC 
;   R3--SPSR
;------------------------------------------------------------------------
	MOV     R1, SP
	ADD     SP, SP, #12             ;Adjust IRQ stack pointer
	SUB     R2, LR, #4              ;Adjust PC for return address to task

	MRS     R3, SPSR				; Copy SPSR (Task CPSR)
	
   

	MSR     CPSR_cxsf, #SVCMODE|NOINT   ;Change to SVC mode

									; SAVE TASK''S CONTEXT ONTO OLD TASK''S STACK
									
	STMFD   SP!, {R2}				; Push task''s PC 
	STMFD   SP!, {R4-R12, LR}		; Push task''s LR,R12-R4
	
	LDMFD   R1!, {R4-R6}			; Load Task''s R1-R3 from IRQ stack 
	STMFD   SP!, {R4-R6}			; Push Task''s R1-R3 to SVC stack
	STMFD   SP!, {R0}			    ; Push Task''s R0 to SVC stack
	
	STMFD   SP!, {R3}				; Push task''s CPSR
	; BL      prink
	LDR     R0,=int_nesting        ;int_nesting++
	LDRB    R1,[R0]
	ADD     R1,R1,#1
	STRB    R1,[R0] 
	
	
	CMP     R1,#1                   ;if(OSIntNesting==1){
	BNE     %F1
	 
	LDR     R4,=high_thread            ;OSTCBHighRdy->OSTCBStkPtr=SP;
	LDR     R5,[R4]
	STR     SP,[R5]                 ;}
	
1
	
	MSR    CPSR_c,#IRQMODE|NOINT    ;Change to IRQ mode to use IRQ stack to handle interrupt
	
	LDR     R0, =INTOFFSET
    LDR     R0, [R0]
       
    LDR     R1, IRQIsrVect
    MOV     LR, PC    ; Save LR befor jump to the C function we need return back
                        
    LDR     PC, [R1, R0, LSL #2]            ; Call KS_CPU_IRQ_ISR_handler();  程序卡死在这里了 
   
    MSR		CPSR_c,#SVCMODE|NOINT   ;Change to SVC mode
    
    BL 		ks_int_exit              ;Call OSIntExit

    
    LDMFD   SP!,{R4}               ;POP the task''s CPSR 
    MSR		SPSR_cxsf,R4
    LDMFD   SP!,{R0-R12,LR,PC}^	   ;POP new Task''s context

IRQIsrVect DCD HandleEINT0	
	
		
		
	
ks_cpu_save
	MRS     R0, CPSR				; Set IRQ and FIQ bits in CPSR to disable all interrupts
	ORR     R1, R0, #0xC0
	MSR     CPSR_c, R1
	MRS     R1, CPSR				; Confirm that CPSR contains the proper interrupt disable flags
	AND     R1, R1, #0xC0
	CMP     R1, #0xC0
	BNE     ks_cpu_save				; Not properly disabled (try again)
	MOV     PC, LR					; Disabled, return the original CPSR contents in R0

ks_cpu_restore
	MSR     CPSR_c, R0
	MOV     PC, LR
	
	
	END
	
	
	


