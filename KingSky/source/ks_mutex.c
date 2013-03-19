/*
*********************************************************************************************************
*                                               KingSky
*                                          ʵʱ����ϵͳ�ں�
*
*                                       (c) Copyright 2011, ׯͩȪ
*                                          All Rights Reserved
*
* File         : ks_mutex.h
********************************************************************************************************* */
#include "KingSky.h"

#if KS_MAX_PRIO < 256
	#define KS_MUTEX_AVALIABLE  0xff                       
#elif KS_MAX_PRIO <65536
	#define KS_MUTEX_AVALIABLE  0xffff                     
#else
	#define KS_MUTEX_AVALIABLE  0xffffffff                  
#endif

#if (KS_EVENT_EN > 0)&& (KS_MUTEX_EN > 0)

ks_event *ks_mutex_create(KS_BASE_TYPE prio,uint8 *error)
{
	ks_event * pevent;
	if(int_nesting > 0)    		 /*ȷ���жϷ�����ӳ����޷�����*/
	{
		return ((ks_event*)KS_MUTEX_CREATE_ERROR);
	}
#if KS_RR_EN >0
	if(prio > KS_MAX_PRIO) 		 /*��ֹ�������				   */
	{
		prio = KS_MAX_PRIO - 1;
	}
#else							 /*���ϵͳ��֧��ʱ����ת��    */
	#if KS_CHECK_EN >0
	if(prio > KS_MAX_PRIO)
	{
		return ((ks_event*)0);   /*�����¼���ָ��              */
	}
	#endif
	if(readylist[prio].item_numbers >= 1)
	{
		return ((ks_event*)0);
	}
#endif
	pevent = (ks_event*)ks_malloc(sizeof(ks_event));     /*��̬����һ���¼���   */
	if(pevent == (ks_event*)0)      					 /*�¼�������ʧ��     	*/
	{
		*error = KS_EVENT_TYPE_NULL;					 /*���ô�����Ϣ			*/
		return((ks_event*)0);							 /*�����¼���ָ��       */
	}
	pevent->event_type = KS_EVENT_TYPE_MUTEX;            /*�����¼����ƿ������ */
	pevent->pip = prio;                                  /*����PIP              */
#if KS_MAX_PRIO < 256
	pevent->mutex_caller = 0xff;                         /*��ʼ��mutex_caller   */
#elif KS_MAX_PRIO <65536
	pevent->mutex_caller = 0xffff;                       /*��ʾ���ڻ�û��ռ���� */
#else
	pevent->mutex_caller = 0xffffffff;                   
#endif
	pevent->pevent = (void*)0;                           /*û���߳��ڵȴ�       */
	ks_event_list_init(pevent);							 /*��ʼ���ȴ�����       */
	*error = KS_NO_ERROR;
	return(pevent);                                      /*�����¼����ƿ�ָ��   */
}

void ks_mutex_waitfor(ks_event * pevent,uint16 timeout,uint8 *error)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
	ks_thread_block *ptr;
	uint8 thread_ready;
	if(int_nesting > 0)                     	 /*ȷ���жϷ�����ӳ����޷�����			  */
	{
		*error = KS_MUTEX_WAIT_ISR;         	 /*�����жϵ��õĴ�����Ϣ				  */
		return;
	}
#if KS_CHECK_EN > 0                         	 /*�Ƿ�֧����������            			  */
	if(pevent == (ks_event*)0)             		 /*�¼����ƿ�ָ��Ϊ��					  */
	{
		*error = KS_EVENT_TYPE_NULL;       		 /*���ô�����Ϣ							  */
		return;
	}
#endif
	if(pevent->event_type != KS_EVENT_TYPE_MUTEX)  /*ȷ���¼����ƿ�������Ƕ���			  */
	{
		*error = KS_MUTEX_TYPE_ERROR;         	   /*���ô�����Ϣ				    	  */
		return;
	}
	KS_ENTER_CRITICAL();
	if(pevent->mutex_caller == KS_MUTEX_AVALIABLE)/*����Ƿ��п��õ�Mutex     			  */
	{											  /*���о�ռ��							  */
		pevent->mutex_caller = current_thread->priority;	/*����ռ���ߵ����ȼ�		  */
		pevent->pevent = (void*)current_thread;				/*��ָ��ָ��ռ���ߵ��߳̿��ƿ�*/
		KS_EXIT_CRITICAL();
		*error = KS_NO_ERROR;                             	/*���÷�����Ϣ				  */
		return;												/*����						  */
	}
	
	ptr = (ks_thread_block*)(pevent->pevent); 				/*��ָ��ָ��Mutexռ���ߵ��߳̿��ƿ�  */
	if(ptr->priority != pevent->pip && pevent->mutex_caller > current_thread->priority)
	{		
															/*�ж��Ƿ���Ҫ����Mutexռ���ߵ����ȼ�*/
		if(ptr->item_in_delaylist == 0)						/*�ж�ռ�����Ƿ����				 */
		{													/*ʹռ�����������				  	 */
			ks_item_remove((ks_list_item*)&(ptr->insertlist_item));
			thread_ready = KS_TRUE;							/*���þ���״̬��־					 */
		}
		else
		{
			thread_ready = KS_FALSE;						/*���÷Ǿ���״̬��־				 */
		}
		ptr->priority = pevent->pip;						/*��ռ���ߵ����ȼ���ΪPIP						*/
		if(thread_ready == KS_TRUE)							/*���ԭռ�����Ǿ�����							*/
		{
			ks_list_insertend((ks_list*)&readylist[ptr->priority],(ks_list_item*)&(ptr->insertlist_item));
		}                                               	/*ʹռ�������µĵ����ȼ�����					*/
	}
	current_thread->thread_state |= KS_STATE_WAIT_MUTEX;	/*����״̬��־����ʾ�ڵȴ���Ϣ�����ź�������Ϣ	*/
	current_thread->thread_delay = timeout;        			/*���ó�ʱ										*/
	ks_event_wait(pevent);   					   			/*�����߳�ֱ���¼�������ʱ����				*/
	KS_EXIT_CRITICAL();
	ks_schedule();           					   			/*��ǰ���񱻹��𣬽��е���						*/
	/*ֻ�г�ʱ����ȷ�õ���Ϣ������������ĳ���*/
	KS_ENTER_CRITICAL();
	if((current_thread->thread_state & KS_STATE_WAIT_MUTEX) >0)/*����״̬δȡ��������Ϊ��ʱ����			    */
	{
		
		ks_event_to(pevent); 								 /*��Ϊ��ʱʹ�߳̾���			 			    */
		Uart_Printf("I don't get the message\n");
		KS_EXIT_CRITICAL();
		*error = KS_MUTEX_TIMEOUT;
		return;
	}
	/*������Ϊ��ʱ������ȡ������Ȩ,�յ���Ϣ,�����������*/
	pevent->pevent = (ks_event*)0;
	KS_EXIT_CRITICAL();
	*error = KS_NO_ERROR;
	return ;
}

uint8 ks_mutex_sendmsg(ks_event*pevent)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
	ks_thread_block *ptr;
	if(int_nesting > 0)                     	   /*ȷ���жϷ�����ӳ����޷�����	 */
	{
		return(KS_MUTEX_WAIT_ISR);
	}
#if KS_CHECK_EN > 0                         	   /*�Ƿ�֧����������            	 */
	if(pevent == (ks_event*)0)             		   /*�¼����ƿ�ָ��Ϊ��				 */
	{
		return(KS_EVENT_TYPE_NULL);
	}
#endif
	if(pevent->event_type != KS_EVENT_TYPE_MUTEX)  /*ȷ���¼����ƿ�������Ƕ���		 */
	{
		return(KS_MUTEX_TYPE_ERROR);
	}
	KS_ENTER_CRITICAL();
	if(current_thread != (ks_thread_block*)pevent->pevent)
	{
		KS_EXIT_CRITICAL();
		return (KS_ERR_NOT_MUTEX_OWNER);
	}
	if(current_thread->priority == pevent->pip)
	{
		ks_item_remove((ks_list_item*)&(current_thread->insertlist_item));
		current_thread->priority = pevent->mutex_caller;
		ks_list_insertend((ks_list*)&readylist[current_thread->priority],(ks_list_item*)&(current_thread->insertlist_item));
	}
	if(pevent->event_wait_num != 0)			 					 	 /*����Ƿ����߳��ڵȴ���Ϣ       	 */
	{
		ptr = ks_event_ready(pevent,(void*)0,KS_STATE_WAIT_MUTEX); 	 /*���У���ʹ������ȼ����߳̽������*/
		pevent->mutex_caller = ptr->priority;
		pevent->pevent = ptr;
		KS_EXIT_CRITICAL();
		ks_schedule();    										     /*ʹ������������ȼ��ָ̻߳�����	 */
		return (KS_NO_ERROR);
	}
	pevent->mutex_caller = KS_MUTEX_AVALIABLE;
	pevent->pevent = (void*)0;
	KS_EXIT_CRITICAL();
	return(KS_NO_ERROR);
}

uint8 ks_mutex_accept(ks_event*pevent,uint8 *error)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
	if(int_nesting > 0)                     	   /*ȷ���жϷ�����ӳ����޷�����	 */
	{
		*error = KS_MUTEX_WAIT_ISR;
		return (0);
	}
	#if KS_CHECK_EN > 0                            /*�Ƿ�֧����������            	 */
	if(pevent == (ks_event*)0)             		   /*�¼����ƿ�ָ��Ϊ��				 */
	{
		*error = KS_EVENT_TYPE_NULL;
		return (0);
	}
#endif
	if(pevent->event_type != KS_EVENT_TYPE_MUTEX)  /*ȷ���¼����ƿ�������Ƕ���		 */
	{
		*error = KS_MUTEX_TYPE_ERROR;
		return (0);
	}
	KS_ENTER_CRITICAL();
	if(pevent->mutex_caller == KS_MUTEX_AVALIABLE) 			/*����Ƿ��п��õ�Mutex       */
	{											   			/*���о�ռ��				  */
		pevent->mutex_caller = current_thread->priority;	/*����ռ���ߵ����ȼ�		  */
		pevent->pevent = (void*)current_thread;				/*��ָ��ָ��ռ���ߵ��߳̿��ƿ�*/
		KS_EXIT_CRITICAL();
		*error = KS_NO_ERROR;                             	/*���÷�����Ϣ				  */
		return (1);										    /*����						  */
	}
	KS_EXIT_CRITICAL();
	*error = KS_NO_ERROR;                             	    /*���÷�����Ϣ				  */ 
	return (0);
}

ks_event * ks_mutex_delete(ks_event*pevent,uint8 opt,uint8 *error)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
	uint8 thread_waiting;
	if(int_nesting > 0)                     	   /*ȷ���жϷ�����ӳ����޷�����*/
	{
		*error = KS_MUTEX_WAIT_ISR;
		return ((ks_event*)0);
	}
	#if KS_CHECK_EN > 0                            /*�Ƿ�֧����������            */
	if(pevent == (ks_event*)0)             		   /*�¼����ƿ�ָ��Ϊ��			 */
	{
		*error = KS_EVENT_TYPE_NULL;
		return ((ks_event*)0);
	}
#endif
	if(pevent->event_type != KS_EVENT_TYPE_MUTEX)  /*ȷ���¼����ƿ�������Ƕ���	*/
	{
		*error = KS_MUTEX_TYPE_ERROR;
		return ((ks_event*)0);
	}
	KS_ENTER_CRITICAL();
	if(pevent->event_wait_num != 0)			/*�����Ϣ�������Ƿ����߳��ڵȴ���Ϣ*/
	{
		thread_waiting = KS_TRUE;			/*���߳��ڵȴ���Ϣ                  */
	}
	else
	{
		thread_waiting = KS_FALSE;			/*û���߳��ڵȴ���Ϣ             	*/
	}
	switch(opt)
	{
	case KS_DEL_NO_WAIT:/*����û���̵߳ȴ���Ϣʱ��ɾ����Ϣ����					*/
		if(thread_waiting == KS_FALSE)
		{
			KS_EXIT_CRITICAL();
			ks_free(pevent->event_wait_list);       /*�ͷ��¼��ȴ�����Ŀռ�	*/
			ks_free(pevent);                        /*�ͷ��¼����ƿ�Ŀռ�  	*/
				
			*error = KS_NO_ERROR;
			return ((ks_event*)0);
		}
		else
		{
			KS_EXIT_CRITICAL();
			*error = KS_DEL_THREAD_WAITING;         /*���ش�����Ϣ*/
			return (pevent);
		}
	case KS_DEL_ALWAYS:	
		while(pevent->event_wait_num != 0) 			/*�����еȴ���Ϣ���̶߳����ھ���*/
		{
			ks_event_ready(pevent,(void*)0,KS_STATE_WAIT_MUTEX);
		}
		KS_EXIT_CRITICAL();
		ks_free(pevent->event_wait_list);			/*�ͷ��¼��ȴ�����Ŀռ�*/
		ks_free(pevent);							/*�ͷ��¼����ƿ�Ŀռ�*/
		if(thread_waiting == KS_TRUE)			
		{
			ks_schedule();							/*���������ڵȴ���Ϣ���͵���һ��*/
		}
		*error = KS_NO_ERROR;
		return ((ks_event*)0);	
	default:										/*�������*/
		KS_EXIT_CRITICAL();
		*error = KS_ERROR_INVALID_OPT;
		return (pevent);
	}
	
	
	
}










#endif