/*
*********************************************************************************************************
*                                               KingSky
*                                          ʵʱ����ϵͳ�ں�
*
*                                       (c) Copyright 2011, ׯͩȪ
*                                          All Rights Reserved
*
* File         : ks_queue.c
********************************************************************************************************* */
#include "KingSky.h"

#if KS_Q_EN > 0
ks_event *ks_queue_create(Q_HANDLE*start,uint16 size)
{
	ks_event *pevent;
	ks_queue *pqueue;
	
	if(int_nesting > 0)/*ȷ���жϷ�����ӳ����޷�����*/
	{
		return (ks_event*)KS_QUEUE_CREATE_ERROR;
	}
	pevent = (ks_event*)ks_malloc(sizeof(ks_event)); /*��̬����һ���¼���*/
	if(pevent != (ks_event*)0) /*�¼����ƿ��ڴ�����ɹ�*/
	{
		pqueue = (ks_queue*)ks_malloc(sizeof(ks_queue));/*����һ����Ϣ���п�*/
		if(pqueue != (ks_queue*)0)/*�ɹ�����һ����Ϣ���п�*/
		{
			pqueue->queue_start = start;
			pqueue->queue_end = &start[size-1];  /*��ʼ������*/
			pqueue->queue_in = start;
			pqueue->queue_out = start;
			pqueue->queue_size = size;
			pqueue->queue_entries = 0;
			pevent->event_type = KS_EVENT_TYPE_Q; /*�����¼����ƿ������,Ҫ�ر�ע���������*/
			pevent->event_count = 0;  /*�¼�Ϊ�ź���ʱ����*/
			pevent->pevent = pqueue;  /*���Ӷ��п��ƿ�*/
			ks_event_list_init(pevent);/*��ʼ���ȴ�����*/
		}
		else/*����һ����Ϣ���п�ʧ��*/
		{
			ks_free(pevent);         /*�ͷ�������¼����ڴ�ռ�*/
			return (ks_event*)KS_QUEUE_CREATE_ERROR;
		}
		
	}
	else
	{
		Uart_Printf("I am wrong \n");/*��ʾ������Ϣ*/
	}
	return (pevent);
}
void *ks_queue_waitfor(ks_event *pevent,uint16 timeout,uint8 *error)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
	void *msg;
	ks_queue *pqueue;
	if(int_nesting > 0)                     /*ȷ���жϷ�����ӳ����޷�����*/
	{
		*error = KS_QUEUE_WAIT_ISR;         /*�����жϵ��õĴ�����Ϣ*/
		return (void*)0;
	}
#if KS_CHECK_EN > 0                         /*�Ƿ�֧����������*/
	if(pevent == (ks_event*)0)              /*�¼����ƿ�ָ��Ϊ��*/
	{
		*error = KS_QUEUE_EVENT_NULL;       /*���ش�����Ϣ*/
		return (void*)0;
	}
	if(pevent->event_type != KS_EVENT_TYPE_Q)/*ȷ���¼����ƿ�������Ƕ���*/
	{
		*error = KS_QUEUE_EVENT_TYPE;        /*���ش�����Ϣ*/
		return (void*)0;
	}
#endif
	KS_ENTER_CRITICAL();
	pqueue = (ks_queue*)pevent->pevent;		/*��ָ��ָ����п��ƿ�*/
	if(pqueue->queue_entries > 0)			/*��������Ϣ�Ƿ�����Ϣ*/
	{
		msg = *pqueue->queue_out++;			/*ȡ����Ϣ���������Ƚ�������Ϣ*/
		pqueue->queue_entries++;         	/*������Ϣ�����е���Ϣ����*/
		if(pqueue->queue_out == pqueue->queue_end)
		{/*���ָ���Ƿ�ָ����Ϣ���е�β��*/
			pqueue->queue_out = pqueue->queue_start;/*����ָ��,�γ�ѭ����Ϣ����*/
		}
		KS_EXIT_CRITICAL();
		*error = KS_NO_ERROR;
		return (msg);   					/*������Ϣ*/
	}
	
	/*�������û����Ϣ��������������Ϣ*/
	current_thread->thread_state |= KS_STATE_WAIT_Q;/*����״̬��־����ʾ�ڵȴ���Ϣ���е���Ϣ*/
	current_thread->thread_delay = timeout;        /*���ó�ʱ*/
	ks_event_wait(pevent);   					   /*�����߳�ֱ���¼�������ʱ����*/
	//Uart_Printf("I don't get the message\n");
	KS_EXIT_CRITICAL();
	ks_schedule();           					   /*��ǰ���񱻹��𣬽��е���*/
	
	/*ֻ�г�ʱ����ȷ�õ���Ϣ������������ĳ���*/
	KS_ENTER_CRITICAL();
	if((current_thread->thread_state & KS_STATE_WAIT_Q) >0)/*����״̬δȡ��������Ϊ��ʱ����*/
	{
		
		ks_event_to(pevent); 					 /*��Ϊ��ʱʹ�߳̾���*/
		Uart_Printf("I don't get the message\n");
		KS_EXIT_CRITICAL();
		*error = KS_QUEUE_TIMEOUT;
		return (void*)0;
	}
	/*������Ϊ��ʱ������ȡ������Ȩ,�յ���Ϣ,�����������*/

	msg = current_thread->pthread_msg;
	current_thread->pthread_msg = (void*)0;
	current_thread->thread_state = KS_STATE_READY; /*Ҫע�������û������,���Բ�Ҫ*/
	current_thread->pthread_event = (ks_event*)0;
	
	KS_EXIT_CRITICAL();
	*error = KS_NO_ERROR;
	return (msg);
}

uint8 ks_queue_sendmsg(ks_event*pevent,void *msg)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
	ks_queue *pqueue;
#if KS_CHECK_EN > 0
	if(pevent == (ks_event*)0)   /*ȷ���¼����ƿ������*/
	{
		return (KS_QUEUE_EVENT_NULL); /*���ش�����Ϣ*/
	}
#endif
	if(pevent->event_type != KS_EVENT_TYPE_Q) /*ȷ���¼����ƿ�������ȷ*/
	{
		return (KS_QUEUE_EVENT_TYPE); /*���ش�����Ϣ*/
	}
	KS_ENTER_CRITICAL();
	if(pevent->event_wait_num != 0)/*�����Ϣ�������Ƿ����߳��ڵȴ���Ϣ*/
	{
		ks_event_ready(pevent,msg,KS_STATE_WAIT_Q); /*���У���ʹ������ȼ����߳̽������*/
		KS_EXIT_CRITICAL();
		ks_schedule();     /*ʹ������������ȼ��ָ̻߳�����*/
		return (KS_NO_ERROR);
	}
	/*û���߳��ڵȴ���Ϣʱ�������������*/
	pqueue = (ks_queue*)pevent->pevent; /*��ָ��ָ����п��ƿ�*/
	if(pqueue->queue_entries >= pqueue->queue_size)/*��Ϣ�����Ƿ��Ѿ���*/
	{
		KS_EXIT_CRITICAL();
		return (KS_QUEUE_FULL);  /*���ش�����Ϣ*/
	}
	*pqueue->queue_in++ = msg;  /*����Ϣδ�����ͽ���Ϣ������Ϣ��������*/
	pqueue->queue_entries ++; /*������Ϣ�����е���Ϣ����*/
	if(pqueue->queue_in == pqueue->queue_end)/*���ָ���Ƿ�ָ����Ϣ���е�ĩ��*/
	{
		pqueue->queue_in = pqueue->queue_start;   /*����ָ��,����ѭ����Ϣ����*/
	}
	KS_EXIT_CRITICAL();
	return (KS_NO_ERROR);
}

/*����������Ϣ������ȡ����Ϣ*/
void *ks_queue_accept(ks_event*pevent,uint8 *error)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
	void *msg;
	ks_queue *pqueue;
#if KS_CHECK_EN > 0   /*�Ƿ�֧����������*/
	if(pevent == (ks_event*)0)/*�¼����ƿ�ָ��Ϊ��*/
	{
		*error = KS_QUEUE_EVENT_NULL;/*���ش�����Ϣ*/
		return (void*)0;
	}
	if(pevent->event_type != KS_EVENT_TYPE_Q)/*ȷ���¼����ƿ�������Ƕ���*/
	{
		*error = KS_QUEUE_EVENT_TYPE;/*���ش�����Ϣ*/
		return (void*)0;
	}
#endif
	KS_ENTER_CRITICAL();
	pqueue = (ks_queue*)pevent->pevent;/*��ָ��ָ����п��ƿ�*/
	if(pqueue->queue_entries > 0)/*��������Ϣ�Ƿ�����Ϣ*/
	{
		msg = *pqueue->queue_out++;/*ȡ����Ϣ���������Ƚ�������Ϣ*/
		pqueue->queue_entries++;         /*������Ϣ�����е���Ϣ����*/
		if(pqueue->queue_out == pqueue->queue_end)
		{/*���ָ���Ƿ�ָ����Ϣ���е�β��*/
			pqueue->queue_out = pqueue->queue_start;/*����ָ��,�γ�ѭ����Ϣ����*/
		}
		KS_EXIT_CRITICAL();
		*error = KS_NO_ERROR;
		return (msg);   /*������Ϣ*/
	}
	else/*��Ϣ����Ϊ��*/
	{
		*error = KS_QUEUE_EMPTY; 
		msg = (void*)0;/*���ؿ�ָ��*/
	}
	KS_EXIT_CRITICAL();
	return (msg);
}

uint8 ks_queue_flush(ks_event *pevent)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
	ks_queue *pqueue;
#if KS_CHECK_EN > 0   /*�Ƿ�֧����������*/
	if(pevent == (ks_event*)0)/*�¼����ƿ�ָ��Ϊ��*/
	{
		return (KS_QUEUE_EVENT_NULL);/*���ش�����Ϣ*/
	}
	if(pevent->event_type != KS_EVENT_TYPE_Q)/*ȷ���¼����ƿ�������Ƕ���*/
	{
		return (KS_QUEUE_EVENT_TYPE);/*���ش�����Ϣ*/
	}
#endif
	KS_ENTER_CRITICAL();
	pqueue = (ks_queue*)pevent->pevent;		/*��ָ��ָ����п��ƿ�*/
	pqueue->queue_in = pqueue->queue_start; /*��ָ���������ʼ״̬*/
	pqueue->queue_out = pqueue->queue_start;
	pqueue->queue_entries = 0;				/*��Ϣ��������*/
	KS_EXIT_CRITICAL();
	return (KS_NO_ERROR);
}

ks_event *ks_queue_delete(ks_event*pevent,uint8 opt,uint8 *error)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
	ks_queue *pqueue;
	uint8 thread_waiting;
	if(int_nesting > 0)/*ȷ���жϷ�����ӳ����޷�����*/
	{
		*error = KS_QUEUE_DEL_ISR;/*�����жϵ��õĴ�����Ϣ*/
		return (pevent);
	}
#if KS_CHECK_EN > 0   /*�Ƿ�֧����������*/
	if(pevent == (ks_event*)0)/*�¼����ƿ�ָ��Ϊ��*/
	{
		*error = KS_QUEUE_EVENT_NULL;/*���ش�����Ϣ*/
		return (pevent);
	}
	if(pevent->event_type != KS_EVENT_TYPE_Q)/*ȷ���¼����ƿ�������Ƕ���*/
	{
		*error = KS_QUEUE_EVENT_TYPE;/*���ش�����Ϣ*/
		return (pevent);
	}
#endif
	KS_ENTER_CRITICAL();
	if(pevent->event_wait_num != 0)/*�����Ϣ�������Ƿ����߳��ڵȴ���Ϣ*/
	{
		thread_waiting = KS_TRUE;/*���߳��ڵȴ���Ϣ*/
	}
	else
	{
		thread_waiting = KS_FALSE;/*û���߳��ڵȴ���Ϣ*/
	}
	switch(opt)
	{
		case KS_DEL_NO_WAIT:/*����û���̵߳ȴ���Ϣʱ��ɾ����Ϣ����*/
			if(thread_waiting == KS_FALSE)
			{
				pqueue = (ks_queue*)pevent->pevent;		/*��ָ��ָ����п��ƿ�*/
				KS_EXIT_CRITICAL();
				ks_free(pqueue);                        /*�ͷŶ��е�ָ��ռ�*/
				ks_free(pevent->event_wait_list);       /*�ͷ��¼��ȴ�����Ŀռ�*/
				ks_free(pevent);                        /*�ͷ��¼����ƿ�Ŀռ�*/
				
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
			while(pevent->event_wait_num != 0) /*�����еȴ���Ϣ���̶߳����ھ���*/
			{
				ks_event_ready(pevent,(void*)0,KS_STATE_WAIT_Q);
			}
			pqueue = (ks_queue*)pevent->pevent;		/*��ָ��ָ����п��ƿ�*/
			KS_EXIT_CRITICAL();
			ks_free(pqueue);						/*�ͷŶ��е�ָ��ռ�*/
			ks_free(pevent->event_wait_list);		/*�ͷ��¼��ȴ�����Ŀռ�*/
			ks_free(pevent);						/*�ͷ��¼����ƿ�Ŀռ�*/
			if(thread_waiting == KS_TRUE)/*���������ڵȴ���Ϣ���͵���һ��*/
			{
				ks_schedule();
			}
			*error = KS_NO_ERROR;
			return (ks_event*)0;	
		default:/*�������*/
			KS_EXIT_CRITICAL();
			*error = KS_ERROR_INVALID_OPT;
			return (pevent);
	}
}

#endif