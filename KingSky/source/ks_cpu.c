/*
*********************************************************************************************************
*                                               KingSky
*                                          实时操作系统内核
*
*                                       (c) Copyright 2011, 庄桐泉
*                                          All Rights Reserved
*
* File         : ks_cpu.c
********************************************************************************************************* */
#include "KingSky.h"


 KS_STACK * ks_stack_init(THREAD_ADDR thread_addr,void *p_arg,KS_STACK *stk1)
{
    KS_STACK *stk;
  
    stk      = stk1;                 /* 将堆栈指针加载进来                                     */
    
    *(stk)   = (KS_STACK)thread_addr; /* Entry Point                                           */
    *(--stk) = (uint32)0;         	/* LR                                                      */
    *(--stk) = (uint32)0;         	/* R12                                                     */
    *(--stk) = (uint32)0;         	/* R11                                                     */
    *(--stk) = (uint32)0;         	/* R10                                                     */
    *(--stk) = (uint32)0;         	/* R9                                                      */
    *(--stk) = (uint32)0;         	/* R8                                                      */
    *(--stk) = (uint32)0;         	/* R7                                                      */
    *(--stk) = (uint32)0;         	/* R6                                                      */
    *(--stk) = (uint32)0;         	/* R5                                                      */
    *(--stk) = (uint32)0;         	/* R4                                                      */
    *(--stk) = (uint32)0;         	/* R3                                                      */
    *(--stk) = (uint32)0;         	/* R2                                                      */
    *(--stk) = (uint32)0;         	/* R1                                                      */
    *(--stk) = (uint32)p_arg;		/* R0 : 参数                                               */
    *(--stk) = (uint32)0x00000013L; /* CPSR  (SVC mode, Enable both IRQ and FIQ interrupts)    */
    return (stk);

}