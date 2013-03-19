/*
*********************************************************************************************************
*                                               KingSky
*                                          ʵʱ����ϵͳ�ں�
*
*                                       (c) Copyright 2011, ׯͩȪ
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
	*--stk = (uint32)0X00000000;	    //�ӳ����Ǵӵ�ǰesp��4��ȡ�ô���Ĳ��������Դ˴�Ҫ�ճ�4���ֽ�
	*--stk = (uint32)thread_addr;          /* Put pointer to task   on top of stack                   */
    *--stk = (uint32)0x00000202;				/* EFL = 0X00000202												*/
	*--stk = (uint32)0xAAAAAAAA;                /* EAX = 0xAAAAAAAA                                              */
    *--stk = (uint32)0xCCCCCCCC;                /* ECX = 0xCCCCCCCC                                             */
    *--stk = (uint32)0xDDDDDDDD;                /* EDX = 0xDDDDDDDD                                             */
    *--stk = (uint32)0xBBBBBBBB;                /* EBX = 0xBBBBBBBB                                             */
    *--stk = (uint32)0x00000000;                /* ESP = 0x00000000  esp�������⣬��Ϊ                          */
    *--stk = (uint32)0x11111111;                /* EBP = 0x11111111                                             */
    *--stk = (uint32)0x22222222;                /* ESI = 0x22222222                                             */
    *--stk = (uint32)0x33333333;                /* EDI = 0x33333333                                              */
    return (stk);
}

 void ks_start_high(void)
 {
	 ks_running = TRUE;
	 _asm{
		     mov ebx, [current_thread]	;ks_thread_block��һ����������esp
			 mov esp, [ebx]		;�ָ���ջ
			 
			 popad		;�ָ�����ͨ�üĴ�������8��
			 popfd		;�ָ���־�Ĵ���
			 ret			;ret ָ���൱��pop eip ������ģʽ�²�����ʹ��eip
			 ;��Զ��������
	 }
 }

 void ks_thread_sw(void)
 {
	 _asm{
		     lea	 eax, nextstart	;�����л��������nextstart��ʼ
			 push eax
			 pushfd				;��־�Ĵ�����ֵ
			 pushad				;����EAX -- EDI		
			 mov ebx, [current_thread]
			 mov [ebx], esp		;�Ѷ�ջ��ڵĵ�ַ���浽��ǰTCB�ṹ��
	 }
	 current_thread = high_thread;	
	 _asm{
		     mov ebx, [high_thread]
			 mov esp, [ebx]		;�õ�high_thread��esp
			 
			 popad				;�ָ�����ͨ�üĴ�������8��
			 popfd				;�ָ���־�Ĵ���
			 ret				;��ת��ָ����������
	 }
nextstart:			//�����л����������е�ַ
		 return;
}

 extern CONTEXT Context;
 extern HANDLE mainhandle;
 
 void ks_int_sw(void)
 {
	volatile KS_STACK *sp;

	 sp = (KS_STACK *)Context.Esp;	//�õ����̵߳�ǰ��ջָ��
	 //�ڶ�ջ�б�����Ӧ�Ĵ�����
	 *--sp = Context.Eip;	//�ȱ���eip
	 *--sp = Context.EFlags;	//����efl
	 *--sp = Context.Eax;
	 *--sp = Context.Ecx;
	 *--sp = Context.Edx;
	 *--sp = Context.Ebx;
	 *--sp = Context.Esp;	//��ʱ�����esp�Ǵ���ģ���OSTCBCur��������ȷ��
	 *--sp = Context.Ebp;
	 *--sp = Context.Esi;
	 *--sp = Context.Edi;	
	 current_thread->top_stack = (KS_STACK *)sp;	//���浱ǰesp
	 current_thread = high_thread;		//�õ���ǰ����������ȼ������tcb
	 sp = high_thread->top_stack;
	 
	 //�ָ����д������ļĴ���
	 Context.Edi = *sp++;
	 Context.Esi = *sp++;
	 Context.Ebp = *sp++;
	 Context.Esp = *sp++;		//��ʱ�������еõ���esp�ǲ���ȷ��
	 Context.Ebx = *sp++;
	 Context.Edx = *sp++;
	 Context.Ecx = *sp++;
	 Context.Eax = *sp++;
	 Context.EFlags = *sp++; 
	 Context.Eip = *sp++;
	 
	 Context.Esp = (uint32)sp;		//�õ���ȷ��esp
	 
	 SetThreadContext(mainhandle, &Context);	//�������߳�������
}

 /*********************************************************************************************************
;                                            HANDLE TICK ISR
;
; Description:  �˺���Ϊʱ�ӵ��ȹؼ�����ͨ��timeSetEvent����������ʱ�ӽ��ģ�timeSetEvent������һ��
				�̵߳��ô˺������˺�������������ͬ�����У���˲������ж�������������д˺�����
				���˺�����ģ���жϵĲ�������������жϴ��ڹر�״̬�����˳�
;
; Arguments  : none
;
; Returns    : none
;
; Note(s)    :  �˺������жϺ����ͬ����û����������Ĵ��������Ҳ�����iretָ�����OSIntCtxSw()ʵ��Ҳ��һ��
				����Ҫ���ص��ú�����
;*********************************************************************************************************/

void CALLBACK ks_tick_isr(unsigned int a,unsigned int b,unsigned long c,unsigned long d,unsigned long e)
{
//	printf("int the isr\n");
	if(!flag)
		return;	//�����ǰ�жϱ������򷵻�

	SuspendThread(mainhandle);		//��ֹ���̵߳����У�ģ���жϲ���.��û�б���Ĵ���
	if(!flag)
	{//��suspendthread�����ǰ��flagEn���ܱ��ٴθĵ�
		ResumeThread(mainhandle);	//ģ���жϷ��أ����̵߳��Լ���ִ��		
		return;	//�����ǰ�жϱ������򷵻�
	}
	GetThreadContext(mainhandle, &Context);	//�õ����߳������ģ�Ϊ�л�������׼��
	
	int_nesting++;
	if (int_nesting == 1) {
		current_thread->top_stack = (KS_STACK *)Context.Esp;	//���浱ǰesp
	}	
	ks_time_tick();		//ucos�ڲ���ʱ
	ks_int_exit();		//���ڲ���ʹ���жϷ���ָ����Դ˺�����Ҫ���ص�
	ResumeThread(mainhandle);	//ģ���жϷ��أ����̵߳��Լ���ִ��
}