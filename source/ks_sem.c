/*
*********************************************************************************************************
*                                               KingSky
*                                          ʵʱ����ϵͳ�ں�
*
*                                       (c) Copyright 2011, ׯͩȪ
*                                          All Rights Reserved
*
* File         : ks_sem.c
********************************************************************************************************* */
#include "\KingSky_vc6\KingSky\includes\KingSky.h"

#if (KS_SEM_EN > 0)&&(KS_EVENT_EN > 0)

ks_event *ks_sem_create(uint16 count)
{
	ks_event *pevent;                            /*����һ���¼����ƿ�ָ��*/
	if(int_nesting > 0)                          /*ȷ���жϷ�����ӳ����޷�����*/
	{
		return (ks_event*)KS_SEM_CREATE_ERROR;
	}
	pevent = (ks_event*)ks_malloc(sizeof(ks_event)); /*��̬����һ���¼���*/
	if(pevent != (ks_event*)0)                       /*�¼����ƿ��ڴ�����ɹ�*/
	{
		pevent->event_type = KS_EVENT_TYPE_SEM;/*�����¼����ƿ������Ϊ�ź�������*/
		pevent->event_count = count;           /*���ź����ĳ�ʼֵ�ŵ��¼����ƿ���*/
		pevent->pevent = (void*)0;             /*��.peventָ���ʼ��Ϊnull*/
		ks_event_list_init(pevent);            /*��ʼ���ȴ�����*/
	}
	else
	{
		//Uart_Printf("I am wrong int the create sem\n");/*��ʾ������Ϣ*/
	}
	return (pevent);
}

void ks_sem_waitfor(ks_event *pevent,uint16 timeout,uint8 *error)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
	if(int_nesting > 0)                     /*ȷ���жϷ�����ӳ����޷�����  */
	{
		*error = KS_QUEUE_WAIT_ISR;         /*�����жϵ��õĴ�����Ϣ		*/
		return;
	}
#if KS_CHECK_EN > 0                         /*�Ƿ�֧����������				*/
	if(pevent == (ks_event*)0)              /*�¼����ƿ�ָ��Ϊ��			*/
	{
		*error = KS_QUEUE_EVENT_NULL;       /*���ش�����Ϣ					*/
		return;
	}
	if(pevent->event_type != KS_EVENT_TYPE_SEM)/*ȷ���¼����ƿ���������ź���*/
	{
		*error = KS_QUEUE_EVENT_TYPE;        /*���ش�����Ϣ					*/
		return;
	}
#endif
	KS_ENTER_CRITICAL();
	if(pevent->event_count > 0)              /*�ź�������ֵ����0��˵���ź�����Ч			*/
	{
		pevent->event_count--;               /*���ź�����Ч�������ֵ�ݼ��������ߵõ��ź���	*/
		KS_EXIT_CRITICAL();
		*error = KS_NO_ERROR;
		return;
	}
	/*����ź����ļ���ֵΪ0������Ҫ�ȴ��¼��ķ���,�����������*/
	current_thread->thread_state |= KS_STATE_WAIT_SEM;	/*����״̬��־����ʾ�ڵȴ��ź���*/
	current_thread->thread_delay = (KS_TIME_TYPE)timeout;        		/*���ó�ʱ*/
	ks_event_wait(pevent);   					  	    /*�����߳�ֱ���¼�������ʱ����*/
	KS_EXIT_CRITICAL();
	ks_schedule(); 
	/*ֻ�г�ʱ����ȷ�õ���Ϣ������������ĳ���*/
	KS_ENTER_CRITICAL();
	if((current_thread->thread_state & KS_STATE_WAIT_SEM) >0)/*����״̬δȡ��������Ϊ��ʱ����*/
	{
		ks_event_to(pevent); 								 /*��Ϊ��ʱʹ�߳̾���*/
		KS_EXIT_CRITICAL();
		*error = KS_QUEUE_TIMEOUT;
		return;
	}
	/*****������Ϊ��ʱ������ȡ������Ȩ,�յ���Ϣ,�����������*****/
	current_thread->pthread_event = (ks_event*)0;
	KS_EXIT_CRITICAL();
	*error = KS_NO_ERROR;
	return;
}

uint8 ks_sem_sendmsg(ks_event*pevent)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
#if KS_CHECK_EN > 0
	if(pevent == (ks_event*)0)  			    /*ȷ���¼����ƿ������  */
	{
		return (KS_SEM_EVENT_NULL); 		    /*���ش�����Ϣ          */
	}
#endif
	if(pevent->event_type != KS_EVENT_TYPE_SEM) /*ȷ���¼����ƿ�������ȷ*/
	{
		return (KS_SEM_EVENT_TYPE); 		    /*���ش�����Ϣ			*/
	}
	KS_ENTER_CRITICAL();
	if(pevent->event_wait_num != 0)			 				 /*�����Ϣ�������Ƿ����߳��ڵȴ���Ϣ */
	{
		ks_event_ready(pevent,(void*)0,KS_STATE_WAIT_SEM); 	 /*���У���ʹ������ȼ����߳̽������ */
		KS_EXIT_CRITICAL();
		ks_schedule();    									 /*ʹ������������ȼ��ָ̻߳�����     */
		return (KS_NO_ERROR);
	}
	/*********û���߳��ڵȴ���Ϣʱ�������������************/
	if(pevent->event_count < 65535)   	  /*ȷ���ź�����ֵ�����*/
	{
		pevent->event_count++;            /*�ź�����ֵ����   	*/
		KS_EXIT_CRITICAL();
		return (KS_NO_ERROR);
	}
	KS_EXIT_CRITICAL();
	return(KS_SEM_OVF);
}

uint16 ks_sem_accept(ks_event*pevent)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
#if KS_CHECK_EN > 0
	if(pevent == (ks_event*)0)  			 /*ȷ���¼����ƿ������ */
	{
		return (0); 						 /*���ش�����Ϣ			*/
	}
#endif	
	KS_ENTER_CRITICAL();
	if(pevent->event_count > 0)             /*����ź�����ֵ�Ƿ�Ϊ��*/
	{ 
		pevent->event_count--;              /*�ź�����ֵ�ݼ�		*/
	}
	KS_EXIT_CRITICAL();
	return (pevent->event_count);           /*�����ź�����ֵ		*/
}

ks_event *ks_sem_delete(ks_event*pevent,uint8 opt,uint8*error)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
	uint8 thread_waiting;
	if(int_nesting > 0)						/*ȷ���жϷ�����ӳ����޷�����*/
	{
		*error = KS_QUEUE_DEL_ISR;			/*�����жϵ��õĴ�����Ϣ	  */
		return (pevent);
	}
#if KS_CHECK_EN > 0   						/*�Ƿ�֧����������			  */
	if(pevent == (ks_event*)0)				/*�¼����ƿ�ָ��Ϊ��   		  */
	{
		*error = KS_SEM_EVENT_NULL;			/*���ش�����Ϣ				  */
		return (pevent);
	}
	if(pevent->event_type != KS_EVENT_TYPE_SEM)/*ȷ���¼����ƿ�������Ƕ��� */
	{
		*error = KS_SEM_EVENT_TYPE;			 /*���ش�����Ϣ				  */
		return (pevent);
	}
#endif
	KS_ENTER_CRITICAL();
	if(pevent->event_wait_num != 0)			/*�����Ϣ�������Ƿ����߳��ڵȴ���Ϣ*/
	{
		thread_waiting = KS_TRUE;			/*���߳��ڵȴ���Ϣ					*/
	}
	else
	{
		thread_waiting = KS_FALSE;			/*û���߳��ڵȴ���Ϣ				*/
	}
	switch(opt)
	{
	case KS_DEL_NO_WAIT:/*����û���̵߳ȴ���Ϣʱ��ɾ����Ϣ����*/
		if(thread_waiting == KS_FALSE)
		{
			KS_EXIT_CRITICAL();
			ks_free((void*)pevent->event_wait_list);       /*�ͷ��¼��ȴ�����Ŀռ�*/
			ks_free(pevent);                        /*�ͷ��¼����ƿ�Ŀռ�	*/
				
			*error = KS_NO_ERROR;
			return ((ks_event*)0);
		}
		else
		{
			KS_EXIT_CRITICAL();
			*error = KS_DEL_THREAD_WAITING;         /*���ش�����Ϣ				   */
			return (pevent);
		}
	case KS_DEL_ALWAYS:
		while(pevent->event_wait_num != 0) 		   /*�����еȴ���Ϣ���̶߳����ھ���*/
		{
			ks_event_ready(pevent,(void*)0,KS_STATE_WAIT_SEM);
		}
		KS_EXIT_CRITICAL();
		ks_free((void*)pevent->event_wait_list);/*�ͷ��¼��ȴ�����Ŀռ�         */
		ks_free(pevent);						/*�ͷ��¼����ƿ�Ŀռ�			 */
		if(thread_waiting == KS_TRUE)			
		{
			ks_schedule();						/*���������ڵȴ���Ϣ���͵���һ�� */
		}
		*error = KS_NO_ERROR;
		return ((ks_event*)0);	
	default:									/*�������						 */
		KS_EXIT_CRITICAL();
		*error = KS_ERROR_INVALID_OPT;
		return (pevent);
	}
	
}


#endif