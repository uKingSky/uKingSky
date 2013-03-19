/*
*********************************************************************************************************
*                                               KingSky
*                                          ʵʱ����ϵͳ�ں�
*
*                                       (c) Copyright 2011, ׯͩȪ
*                                          All Rights Reserved
*
* File         : ks_thread.c
********************************************************************************************************* */
#include "KingSky.h"


KS_HANDLE ks_thread_create(THREAD_ADDR thread_addr,
						   const uint8 *const thread_name,
						   void *p_arg,
						   KS_STACK *stk,
						   KS_BASE_TYPE prio)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif

	KS_STACK *stk1;
	ks_thread_block *ptr;
	uint8 err;
#if KS_RR_EN >0
	if(prio > KS_MAX_PRIO)            /*��ֹ���������һ��Ҫ���ƺ�*/
	{
		prio = KS_MAX_PRIO - 1;
	}
#else
	#if KS_CHECK_EN >0
	if(prio > KS_MAX_PRIO)
	{
		return(ks_thread_block*)(KS_PRIO_ERROR);
	}
	#endif
#endif
	ptr  = ks_thread_allocate();                     /*����һ����е��߳�                   */
	stk1 = ks_stack_init (thread_addr,p_arg,stk);    /*�̶߳�ջ���г�ʼ��,���������ջ��ջ��*/
	err  = ks_thread_init(ptr,stk1,prio,thread_name);/*��������߳̽��г�ʼ��               */
	
	if(err == KS_NOERR_THREAD_INIT)
	{
		/*���ж�,ע�ⲻҪ���жϵ�ʱ��̫����̫��Ӱ��ʵʱ��,�ж���Ӧʱ��*/
		KS_ENTER_CRITICAL();
		current_thread_number ++;                      /*�߳�����һ							*/
		if(current_thread_number == (KS_THREAD_NUM)1)  /*�ж��Ƿ��һ�δ����߳�             */
		{
			high_thread = ptr;
			current_running_priority = ptr->priority;  /*���õ�ǰ���е����ȼ�				*/
		}
		else                                           /*������ǵ�һ�δ����߳�				*/
		{
			if(ks_running == KS_FALSE)  				/*�ж�ϵͳ�Ƿ��Ѿ�����				*/ 
			{
				if(current_thread->priority > ptr->priority)
				{
					high_thread = ptr;
					current_running_priority = ptr->priority;/*����current_running_priority��ֵ*/
				}
			}
		}
		if(top_readylist_priority > ptr->priority) 
		{
			top_readylist_priority = ptr->priority;          /*����top_readylist_priority��ֵ  */
		}
		ks_list_insertend((ks_list*)&readylist[ptr->priority],(ks_list_item*)&(ptr->insertlist_item));/*���̲߳��뵽���ȼ�����������*/
#if KS_RR_EN > 0
		if(readylist[ptr->priority].item_numbers > 1)
		{
			readylist[ptr->priority].rr_flag = 1;            /*����rr_flag��־                 */
		}
#else
		if(readylist[ptr->priority].item_numbers > 1)        /*���ϵͳ��֧����ͬ���ȼ�		   */
		{
			ks_item_remove((ks_list_item*)&(ptr->insertlist_item));
			ptr->thread_next = pfree_thread;                 /*�ջ���Դ               		    */
			pfree_thread = ptr;
			return(ks_thread_block*)(KS_PRIO_ERROR);
		}
#endif
		
		KS_EXIT_CRITICAL();
		if(ks_running == KS_TRUE)							/*�ж�ϵͳ�Ƿ�������				*/
		{
			if(current_thread->priority > ptr->priority)
			{
				ks_schedule();								/*���е���							*/
			}
		
		}
	}
	else if(err == KS_ERR_THREAD_INIT)						/*�̳߳�ʼ��ʧ�ܺ�Ĵ���			*/
	{
	
		return(ks_thread_block*)(KS_PRIO_ERROR);			/*ʧ�ܴ���							*/
	}
	return ptr;
}

void ks_thread_delay(KS_TIME_TYPE ticks)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
	
	if(ticks >0)
	{
		KS_ENTER_CRITICAL();
#if KS_RR_EN >0                        /*�Ƿ�֧����ͬ���ȼ�*/
		if(readylist[current_running_priority].item_numbers > (KS_BASE_TYPE)1)
		{
			if(current_thread->insertlist_item.item_next->item_value == 0)/*���е������Ƿ��������β��*/
			{
				next_thread =(ks_thread_block*)current_thread->insertlist_item.item_next->item_next->owner;
			}
			else if(current_thread->insertlist_item.item_next->item_value > 0)
			{
				next_thread =(ks_thread_block*)current_thread->insertlist_item.item_next->owner;
			}
		}
#endif
		ks_item_remove((ks_list_item*)&current_thread->insertlist_item);/*����ǰ����̬�������ڵ�Ӿ���������ɾ��*/
		
												 /*���ڵ���뵽��ʱ������			   */
		current_thread->thread_delay = ticks;    /*��ticks��ֵ����thread_delay         */
		current_thread->item_in_delaylist = 1;   /*��־�������е��߳̽����뵽��ʱ������*/
		ks_list_insertend((ks_list*)&delaylist[0],(ks_list_item*)&current_thread->insertlist_item);
		KS_EXIT_CRITICAL(); 
		ks_schedule();        					 /*����								   */
	}
												/*���ticks����0��ʾ����ʱ,���������ִ��*/	
}

void ks_thread_idle(void *pram)
{
	pram = pram;
	Uart_SendString("I am the idle\n");
	for(;;)//��ȥ֮�����Ҳû������
	{
		//���������ת
	}
}

#if KS_THREAD_SUSPEND_EN >0

uint8 ks_thread_suspend(KS_HANDLE handle)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
	uint8 self = FALSE;
#if KS_CHECK_EN > 0
	if(handle->thread_state & KS_STATE_SUSPEND > 0) /*�ж�Ҫ��������߳��Ƿ�Ӧ�������� */
	{
		return (KS_THREAD_SUSPEND_AGAIN);
	}
	if(handle == (KS_HANDLE)KS_PRIO_ERROR)         /*�ж�Ҫ��������߳��Ƿ���ȷ		   */
	{
		return (KS_THREAD_SUSPEND_ERROR);
	}
	if(handle == h_idle)							/*�жϱ�������߳��Ƿ��ǿ����߳�   */
	{
		return (KS_THREAD_SUSPEND_IDLE);
	}
#endif
	KS_ENTER_CRITICAL();
	
	if(handle == KS_THREAD_SELF)  					/*�жϱ�������߳��Ƿ����������е��߳�*/
	{
		self = TRUE;								/*self��1							  */
		handle = current_thread;
	}
	else if( handle == current_thread)             /*�жϱ�������߳��Ƿ����������е��߳� */
	{
		self = TRUE;
	}
	if(handle->item_in_delaylist == 0)			  /*�����ɾ�����߳��ھ���������		  */
	{
		ks_item_remove((ks_list_item*)&handle->insertlist_item);
		ks_list_insertend((ks_list*)&delaylist[0],(ks_list_item*)&handle->insertlist_item);
		handle->item_in_delaylist = 1;           /*��־��������߳�����ʱ������			  */
	}
	handle->thread_state |= KS_STATE_SUSPEND;    /*���ñ������̵߳�״̬Ϊ����̬			  */
	KS_EXIT_CRITICAL();
	if(self == TRUE)                             /*�����������߳����������е��߳�		  */
	{
		ks_schedule();							 /*�̼߳���ĵ���						  */
	}
	return (KS_NO_ERROR);
}
#endif

#if KS_THREAD_RESUME_EN > 0
	
uint8 ks_thread_resume(KS_HANDLE handle)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
#if KS_CHECK_EN > 0
	if(handle == (KS_HANDLE)KS_PRIO_ERROR)            /*�ж�Ҫ�����ѵ��߳��Ƿ���ȷ   */
	{
		return (KS_THREAD_SUSPEND_ERROR);
	}
#endif
    KS_ENTER_CRITICAL();
    if((handle->thread_state & KS_STATE_SUSPEND) > 0)/*�жϱ����ѵ��߳��Ƿ��Ѿ������� */
    {
    	handle->thread_state &= ~KS_STATE_SUSPEND;   /*ȡ���̱߳������״̬           */
    	ks_item_remove((ks_list_item*)&handle->insertlist_item);
		ks_list_insertend((ks_list*)&readylist[handle->priority],(ks_list_item*)&handle->insertlist_item);
		handle->item_in_delaylist = 0;				/*�����ѵ��̲߳�����ʱ������	  */
		KS_EXIT_CRITICAL();
		ks_schedule();                               /*����							  */
    }
    else
    {
    	KS_EXIT_CRITICAL();
    }
    return (KS_NO_ERROR);

}
#endif

#if KS_THREAD_CHANGE_PRIO > 0 //�Ժ������
uint8 ks_thread_change_prio(KS_HANDLE handle,KS_BASE_TYPE new_prio)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
#if KS_CHECK_EN >0
	if(handle == (KS_HANDLE)KS_PRIO_ERROR)
	{
		return (KS_PRIO_CHANGE_ERROR);
	}
	if(new_prio > KS_MAX_PRIO)
	{
		return(KS_PRIO_ERROR);
	}
#endif
	KS_ENTER_CRITICAL();
	if(handle == KS_THREAD_SELF) 
	{
		handle = current_thread;
	}
	
	//handle->priority = new_prio;
	

	return (KS_NO_ERROR);
}
#endif

#if KS_THREAD_CLOSE_EN > 0
uint8 ks_thread_close(KS_HANDLE handle)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
	if(int_nesting > 0)              /*ȷ���ж��ڲ��ܵ���               */
	{
		return (KS_THREAD_DEL_ISR);  /*�����ж��ڵ��ã��򷵻ش����ʶ�� */
	}
#if KS_CHECK_EN >0
	if(handle == (KS_HANDLE)KS_PRIO_ERROR)   /*ȷ��Ҫɾ���߳��Ǵ��ڵ�   */
	{
		return (KS_THREAD_DEL_ERROR);
	}
	if(handle == h_idle)                    /*ȷ��ɾ���Ĳ��ǿ�������    */
	{
		return(KS_THREAD_DEL_IDLE);
	}
#endif
	KS_ENTER_CRITICAL();
	if(handle == KS_THREAD_SELF)     /*����ɾ���Լ�                    */
	{
		handle = current_thread;     /*����ǰ�����߳̿�ָ�븳��handle  */
	}
	if(handle->thread_state != KS_STATE_FREE)    /*ȷ���߳��Ǵ��ڵ�    */
	{
		ks_item_remove((ks_list_item*)&handle->insertlist_item);/*���߳̿�Ӿ����������ʱ������ɾ��  */
		handle->thread_delay = 0;            /*����ʱ�����㣬��ȷ���Լ��ؿ��жϺ�ISR����ʹ���������*/
		current_thread_number--;             /*����������1                                            */
		handle->thread_next = pfree_thread;  /*����ɾ�����߳̿��ƿ�Żؿ���������ȥ�Ա㱻�����߳�ʹ�� */
		pfree_thread = handle;
		handle->thread_state = KS_STATE_FREE;/*����״̬�����Ա�����ʹ��                               */
		KS_EXIT_CRITICAL();
		ks_schedule();
	}
	else
	{
		KS_EXIT_CRITICAL();
		return (KS_THREAD_DEL_ERROR);
	}
	
	return (KS_NO_ERROR);
}
#endif


