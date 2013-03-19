/*
*********************************************************************************************************
*                                               KingSky
*                                          实时操作系统内核
*
*                                       (c) Copyright 2011, 庄桐泉
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
	
	if(int_nesting > 0)/*确保中断服务程子程序无法调用*/
	{
		return (ks_event*)KS_QUEUE_CREATE_ERROR;
	}
	pevent = (ks_event*)ks_malloc(sizeof(ks_event)); /*动态申请一个事件块*/
	if(pevent != (ks_event*)0) /*事件控制块内存申请成功*/
	{
		pqueue = (ks_queue*)ks_malloc(sizeof(ks_queue));/*申请一块消息队列块*/
		if(pqueue != (ks_queue*)0)/*成功申请一块消息队列块*/
		{
			pqueue->queue_start = start;
			pqueue->queue_end = &start[size-1];  /*初始化队列*/
			pqueue->queue_in = start;
			pqueue->queue_out = start;
			pqueue->queue_size = size;
			pqueue->queue_entries = 0;
			pevent->event_type = KS_EVENT_TYPE_Q; /*设置事件控制块的类型,要特别注意这个变量*/
			pevent->event_count = 0;  /*事件为信号量时才用*/
			pevent->pevent = pqueue;  /*连接队列控制块*/
			ks_event_list_init(pevent);/*初始化等待链表*/
		}
		else/*申请一块消息队列块失败*/
		{
			ks_free(pevent);         /*释放申请的事件块内存空间*/
			return (ks_event*)KS_QUEUE_CREATE_ERROR;
		}
		
	}
	else
	{
		Uart_Printf("I am wrong \n");/*显示错误信息*/
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
	if(int_nesting > 0)                     /*确保中断服务程子程序无法调用*/
	{
		*error = KS_QUEUE_WAIT_ISR;         /*返回中断调用的错误信息*/
		return (void*)0;
	}
#if KS_CHECK_EN > 0                         /*是否支持条件检验*/
	if(pevent == (ks_event*)0)              /*事件控制块指针为空*/
	{
		*error = KS_QUEUE_EVENT_NULL;       /*返回错误信息*/
		return (void*)0;
	}
	if(pevent->event_type != KS_EVENT_TYPE_Q)/*确保事件控制块的类型是队列*/
	{
		*error = KS_QUEUE_EVENT_TYPE;        /*返回错误信息*/
		return (void*)0;
	}
#endif
	KS_ENTER_CRITICAL();
	pqueue = (ks_queue*)pevent->pevent;		/*将指针指向队列控制块*/
	if(pqueue->queue_entries > 0)			/*检查队列消息是否有消息*/
	{
		msg = *pqueue->queue_out++;			/*取出消息队列中最先进来的消息*/
		pqueue->queue_entries++;         	/*更新消息队列中的消息数量*/
		if(pqueue->queue_out == pqueue->queue_end)
		{/*检查指针是否指向消息队列的尾部*/
			pqueue->queue_out = pqueue->queue_start;/*调整指针,形成循环消息队列*/
		}
		KS_EXIT_CRITICAL();
		*error = KS_NO_ERROR;
		return (msg);   					/*返回消息*/
	}
	
	/*如果队列没有消息，就运行下列消息*/
	current_thread->thread_state |= KS_STATE_WAIT_Q;/*设置状态标志，表示在等待消息队列的消息*/
	current_thread->thread_delay = timeout;        /*设置超时*/
	ks_event_wait(pevent);   					   /*挂起线程直到事件发生或超时期满*/
	//Uart_Printf("I don't get the message\n");
	KS_EXIT_CRITICAL();
	ks_schedule();           					   /*当前任务被挂起，进行调度*/
	
	/*只有超时或正确得到消息，才运行下面的程序*/
	KS_ENTER_CRITICAL();
	if((current_thread->thread_state & KS_STATE_WAIT_Q) >0)/*挂起状态未取消，是因为超时期满*/
	{
		
		ks_event_to(pevent); 					 /*因为超时使线程就绪*/
		Uart_Printf("I don't get the message\n");
		KS_EXIT_CRITICAL();
		*error = KS_QUEUE_TIMEOUT;
		return (void*)0;
	}
	/*不是因为超时期满而取得运行权,收到消息,运行下面程序*/

	msg = current_thread->pthread_msg;
	current_thread->pthread_msg = (void*)0;
	current_thread->thread_state = KS_STATE_READY; /*要注意这里，还没有完善,可以不要*/
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
	if(pevent == (ks_event*)0)   /*确保事件控制块可以用*/
	{
		return (KS_QUEUE_EVENT_NULL); /*返回错误信息*/
	}
#endif
	if(pevent->event_type != KS_EVENT_TYPE_Q) /*确保事件控制块类型正确*/
	{
		return (KS_QUEUE_EVENT_TYPE); /*返回错误信息*/
	}
	KS_ENTER_CRITICAL();
	if(pevent->event_wait_num != 0)/*检查消息队列中是否有线程在等待消息*/
	{
		ks_event_ready(pevent,msg,KS_STATE_WAIT_Q); /*若有，就使最高优先级的线程进入就绪*/
		KS_EXIT_CRITICAL();
		ks_schedule();     /*使就绪的最高优先级线程恢复运行*/
		return (KS_NO_ERROR);
	}
	/*没有线程在等待消息时，运行下面程序*/
	pqueue = (ks_queue*)pevent->pevent; /*将指针指向队列控制块*/
	if(pqueue->queue_entries >= pqueue->queue_size)/*消息队列是否已经满*/
	{
		KS_EXIT_CRITICAL();
		return (KS_QUEUE_FULL);  /*返回错误消息*/
	}
	*pqueue->queue_in++ = msg;  /*若消息未满，就将消息插入消息队列里面*/
	pqueue->queue_entries ++; /*更新消息队列中的消息数量*/
	if(pqueue->queue_in == pqueue->queue_end)/*检查指针是否指向消息队列的末端*/
	{
		pqueue->queue_in = pqueue->queue_start;   /*调整指针,构成循环消息队列*/
	}
	KS_EXIT_CRITICAL();
	return (KS_NO_ERROR);
}

/*无条件从消息队列中取得消息*/
void *ks_queue_accept(ks_event*pevent,uint8 *error)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
	void *msg;
	ks_queue *pqueue;
#if KS_CHECK_EN > 0   /*是否支持条件检验*/
	if(pevent == (ks_event*)0)/*事件控制块指针为空*/
	{
		*error = KS_QUEUE_EVENT_NULL;/*返回错误信息*/
		return (void*)0;
	}
	if(pevent->event_type != KS_EVENT_TYPE_Q)/*确保事件控制块的类型是队列*/
	{
		*error = KS_QUEUE_EVENT_TYPE;/*返回错误信息*/
		return (void*)0;
	}
#endif
	KS_ENTER_CRITICAL();
	pqueue = (ks_queue*)pevent->pevent;/*将指针指向队列控制块*/
	if(pqueue->queue_entries > 0)/*检查队列消息是否有消息*/
	{
		msg = *pqueue->queue_out++;/*取出消息队列中最先进来的消息*/
		pqueue->queue_entries++;         /*更新消息队列中的消息数量*/
		if(pqueue->queue_out == pqueue->queue_end)
		{/*检查指针是否指向消息队列的尾部*/
			pqueue->queue_out = pqueue->queue_start;/*调整指针,形成循环消息队列*/
		}
		KS_EXIT_CRITICAL();
		*error = KS_NO_ERROR;
		return (msg);   /*返回消息*/
	}
	else/*消息队列为空*/
	{
		*error = KS_QUEUE_EMPTY; 
		msg = (void*)0;/*返回空指针*/
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
#if KS_CHECK_EN > 0   /*是否支持条件检验*/
	if(pevent == (ks_event*)0)/*事件控制块指针为空*/
	{
		return (KS_QUEUE_EVENT_NULL);/*返回错误信息*/
	}
	if(pevent->event_type != KS_EVENT_TYPE_Q)/*确保事件控制块的类型是队列*/
	{
		return (KS_QUEUE_EVENT_TYPE);/*返回错误信息*/
	}
#endif
	KS_ENTER_CRITICAL();
	pqueue = (ks_queue*)pevent->pevent;		/*将指针指向队列控制块*/
	pqueue->queue_in = pqueue->queue_start; /*将指针调整到初始状态*/
	pqueue->queue_out = pqueue->queue_start;
	pqueue->queue_entries = 0;				/*消息数量清零*/
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
	if(int_nesting > 0)/*确保中断服务程子程序无法调用*/
	{
		*error = KS_QUEUE_DEL_ISR;/*返回中断调用的错误信息*/
		return (pevent);
	}
#if KS_CHECK_EN > 0   /*是否支持条件检验*/
	if(pevent == (ks_event*)0)/*事件控制块指针为空*/
	{
		*error = KS_QUEUE_EVENT_NULL;/*返回错误信息*/
		return (pevent);
	}
	if(pevent->event_type != KS_EVENT_TYPE_Q)/*确保事件控制块的类型是队列*/
	{
		*error = KS_QUEUE_EVENT_TYPE;/*返回错误信息*/
		return (pevent);
	}
#endif
	KS_ENTER_CRITICAL();
	if(pevent->event_wait_num != 0)/*检查消息队列中是否有线程在等待消息*/
	{
		thread_waiting = KS_TRUE;/*有线程在等待消息*/
	}
	else
	{
		thread_waiting = KS_FALSE;/*没有线程在等待消息*/
	}
	switch(opt)
	{
		case KS_DEL_NO_WAIT:/*仅在没有线程等待消息时才删除消息队列*/
			if(thread_waiting == KS_FALSE)
			{
				pqueue = (ks_queue*)pevent->pevent;		/*将指针指向队列控制块*/
				KS_EXIT_CRITICAL();
				ks_free(pqueue);                        /*释放队列的指针空间*/
				ks_free(pevent->event_wait_list);       /*释放事件等待链表的空间*/
				ks_free(pevent);                        /*释放事件控制块的空间*/
				
				*error = KS_NO_ERROR;
				return ((ks_event*)0);
			}
			else
			{
				KS_EXIT_CRITICAL();
				*error = KS_DEL_THREAD_WAITING;         /*返回错误消息*/
				return (pevent);
			}
		case KS_DEL_ALWAYS:
			while(pevent->event_wait_num != 0) /*将所有等待消息的线程都置于就绪*/
			{
				ks_event_ready(pevent,(void*)0,KS_STATE_WAIT_Q);
			}
			pqueue = (ks_queue*)pevent->pevent;		/*将指针指向队列控制块*/
			KS_EXIT_CRITICAL();
			ks_free(pqueue);						/*释放队列的指针空间*/
			ks_free(pevent->event_wait_list);		/*释放事件等待链表的空间*/
			ks_free(pevent);						/*释放事件控制块的空间*/
			if(thread_waiting == KS_TRUE)/*若有任务在等待消息，就调度一次*/
			{
				ks_schedule();
			}
			*error = KS_NO_ERROR;
			return (ks_event*)0;	
		default:/*其它情况*/
			KS_EXIT_CRITICAL();
			*error = KS_ERROR_INVALID_OPT;
			return (pevent);
	}
}

#endif