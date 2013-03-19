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
#include "\KingSky_vc6\KingSky\includes\KingSky.h"


 KS_STACK * ks_stack_init(THREAD_ADDR thread_addr,void *p_arg,KS_STACK *stk1)
{
    KS_STACK *stk;
	stk    = (uint32 *)stk1;                /* Load stack pointer                                      */
    *--stk = (uint32)p_arg;                   /* Simulate call to function with argument                 */                                    
	*--stk = (uint32)0X00000000;	    //子程序是从当前esp＋4处取得传入的参数，所以此处要空出4个字节
	*--stk = (uint32)thread_addr;          /* Put pointer to task   on top of stack                   */
    *--stk = (uint32)0x00000202;				/* EFL = 0X00000202												*/
	*--stk = (uint32)0xAAAAAAAA;                /* EAX = 0xAAAAAAAA                                              */
    *--stk = (uint32)0xCCCCCCCC;                /* ECX = 0xCCCCCCCC                                             */
    *--stk = (uint32)0xDDDDDDDD;                /* EDX = 0xDDDDDDDD                                             */
    *--stk = (uint32)0xBBBBBBBB;                /* EBX = 0xBBBBBBBB                                             */
    *--stk = (uint32)0x00000000;                /* ESP = 0x00000000  esp可以任意，因为                          */
    *--stk = (uint32)0x11111111;                /* EBP = 0x11111111                                             */
    *--stk = (uint32)0x22222222;                /* ESI = 0x22222222                                             */
    *--stk = (uint32)0x33333333;                /* EDI = 0x33333333                                              */
    return (stk);
}

 void ks_start_high(void)
 {
	 ks_running = TRUE;
	 _asm{
		     mov ebx, [current_thread]	;ks_thread_block第一个参数就是esp
			 mov esp, [ebx]		;恢复堆栈
			 
			 popad		;恢复所有通用寄存器，共8个
			 popfd		;恢复标志寄存器
			 ret			;ret 指令相当于pop eip 但保护模式下不容许使用eip
			 ;永远都不返回
	 }
 }

 void ks_thread_sw(void)
 {
	 _asm{
		     lea	 eax, nextstart	;任务切换回来后从nextstart开始
			 push eax
			 pushfd				;标志寄存器的值
			 pushad				;保存EAX -- EDI		
			 mov ebx, [current_thread]
			 mov [ebx], esp		;把堆栈入口的地址保存到当前TCB结构中
	 }
	 current_thread = high_thread;	
	 _asm{
		     mov ebx, [high_thread]
			 mov esp, [ebx]		;得到high_thread的esp
			 
			 popad				;恢复所有通用寄存器，共8个
			 popfd				;恢复标志寄存器
			 ret				;跳转到指定任务运行
	 }
nextstart:			//任务切换回来的运行地址
		 return;
}

 extern CONTEXT Context;
 extern HANDLE mainhandle;
 
 void ks_int_sw(void)
 {
	volatile KS_STACK *sp;

	 sp = (KS_STACK *)Context.Esp;	//得到主线程当前堆栈指针
	 //在堆栈中保存相应寄存器。
	 *--sp = Context.Eip;	//先保存eip
	 *--sp = Context.EFlags;	//保存efl
	 *--sp = Context.Eax;
	 *--sp = Context.Ecx;
	 *--sp = Context.Edx;
	 *--sp = Context.Ebx;
	 *--sp = Context.Esp;	//此时保存的esp是错误的，但OSTCBCur保存了正确的
	 *--sp = Context.Ebp;
	 *--sp = Context.Esi;
	 *--sp = Context.Edi;	
	 current_thread->top_stack = (KS_STACK *)sp;	//保存当前esp
	 current_thread = high_thread;		//得到当前就绪最高优先级任务的tcb
	 sp = high_thread->top_stack;
	 
	 //恢复所有处理器的寄存器
	 Context.Edi = *sp++;
	 Context.Esi = *sp++;
	 Context.Ebp = *sp++;
	 Context.Esp = *sp++;		//此时上下文中得到的esp是不正确的
	 Context.Ebx = *sp++;
	 Context.Edx = *sp++;
	 Context.Ecx = *sp++;
	 Context.Eax = *sp++;
	 Context.EFlags = *sp++; 
	 Context.Eip = *sp++;
	 
	 Context.Esp = (uint32)sp;		//得到正确的esp
	 
	 SetThreadContext(mainhandle, &Context);	//保存主线程上下文
}

 /*********************************************************************************************************
;                                            HANDLE TICK ISR
;
; Description:  此函数为时钟调度关键程序，通过timeSetEvent函数来产生时钟节拍，timeSetEvent将产生一个
				线程调用此函数，此函数将与主任务同步运行，因此并不是中断主程序才来运行此函数的
				但此函数将模拟中断的产生，如果发现中断处于关闭状态，则退出
;
; Arguments  : none
;
; Returns    : none
;
; Note(s)    :  此函数与中断函数最不同的是没有立即保存寄存器，而且不能用iret指令，所以OSIntCtxSw()实现也不一样
				它是要返回调用函数的
;*********************************************************************************************************/

void CALLBACK ks_tick_isr(unsigned int a,unsigned int b,unsigned long c,unsigned long d,unsigned long e)
{
//	printf("int the isr\n");
	if(!flag)
		return;	//如果当前中断被屏蔽则返回

	SuspendThread(mainhandle);		//中止主线程的运行，模拟中断产生.但没有保存寄存器
	if(!flag)
	{//在suspendthread完成以前，flagEn可能被再次改掉
		ResumeThread(mainhandle);	//模拟中断返回，主线程得以继续执行		
		return;	//如果当前中断被屏蔽则返回
	}
	GetThreadContext(mainhandle, &Context);	//得到主线程上下文，为切换任务做准备
	
	int_nesting++;
	if (int_nesting == 1) {
		current_thread->top_stack = (KS_STACK *)Context.Esp;	//保存当前esp
	}	
	ks_time_tick();		//ucos内部定时
	ks_int_exit();		//由于不能使用中断返回指令，所以此函数是要返回的
	ResumeThread(mainhandle);	//模拟中断返回，主线程得以继续执行
}