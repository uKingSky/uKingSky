/*
*********************************************************************************************************
*                                               KingSky
*                                          实时操作系统内核
*
*                                       (c) Copyright 2011, 庄桐泉
*                                          All Rights Reserved
*
* File         : ks_sem.c
********************************************************************************************************* */
#include "\KingSky_vc6\KingSky\includes\KingSky.h"

#if (KS_SEM_EN > 0)&&(KS_EVENT_EN > 0)

ks_event *ks_sem_create(uint16 count)
{
	ks_event *pevent;                            /*定义一个事件控制块指针*/
	if(int_nesting > 0)                          /*确保中断服务程子程序无法调用*/
	{
		return (ks_event*)KS_SEM_CREATE_ERROR;
	}
	pevent = (ks_event*)ks_malloc(sizeof(ks_event)); /*动态申请一个事件块*/
	if(pevent != (ks_event*)0)                       /*事件控制块内存申请成功*/
	{
		pevent->event_type = KS_EVENT_TYPE_SEM;/*设置事件控制块的类型为信号量类型*/
		pevent->event_count = count;           /*将信号量的初始值放到事件控制块中*/
		pevent->pevent = (void*)0;             /*将.pevent指针初始化为null*/
		ks_event_list_init(pevent);            /*初始化等待链表*/
	}
	else
	{
		//Uart_Printf("I am wrong int the create sem\n");/*显示错误信息*/
	}
	return (pevent);
}

void ks_sem_waitfor(ks_event *pevent,uint16 timeout,uint8 *error)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
	if(int_nesting > 0)                     /*确保中断服务程子程序无法调用  */
	{
		*error = KS_QUEUE_WAIT_ISR;         /*返回中断调用的错误信息		*/
		return;
	}
#if KS_CHECK_EN > 0                         /*是否支持条件检验				*/
	if(pevent == (ks_event*)0)              /*事件控制块指针为空			*/
	{
		*error = KS_QUEUE_EVENT_NULL;       /*返回错误信息					*/
		return;
	}
	if(pevent->event_type != KS_EVENT_TYPE_SEM)/*确保事件控制块的类型是信号量*/
	{
		*error = KS_QUEUE_EVENT_TYPE;        /*返回错误信息					*/
		return;
	}
#endif
	KS_ENTER_CRITICAL();
	if(pevent->event_count > 0)              /*信号量计数值大于0，说明信号量有效			*/
	{
		pevent->event_count--;               /*若信号量有效，则计数值递减，调用者得到信号量	*/
		KS_EXIT_CRITICAL();
		*error = KS_NO_ERROR;
		return;
	}
	/*如果信号量的计数值为0，则需要等待事件的发生,运行下面程序*/
	current_thread->thread_state |= KS_STATE_WAIT_SEM;	/*设置状态标志，表示在等待信号量*/
	current_thread->thread_delay = (KS_TIME_TYPE)timeout;        		/*设置超时*/
	ks_event_wait(pevent);   					  	    /*挂起线程直到事件发生或超时期满*/
	KS_EXIT_CRITICAL();
	ks_schedule(); 
	/*只有超时或正确得到消息，才运行下面的程序*/
	KS_ENTER_CRITICAL();
	if((current_thread->thread_state & KS_STATE_WAIT_SEM) >0)/*挂起状态未取消，是因为超时期满*/
	{
		ks_event_to(pevent); 								 /*因为超时使线程就绪*/
		KS_EXIT_CRITICAL();
		*error = KS_QUEUE_TIMEOUT;
		return;
	}
	/*****不是因为超时期满而取得运行权,收到消息,运行下面程序*****/
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
	if(pevent == (ks_event*)0)  			    /*确保事件控制块可以用  */
	{
		return (KS_SEM_EVENT_NULL); 		    /*返回错误信息          */
	}
#endif
	if(pevent->event_type != KS_EVENT_TYPE_SEM) /*确保事件控制块类型正确*/
	{
		return (KS_SEM_EVENT_TYPE); 		    /*返回错误信息			*/
	}
	KS_ENTER_CRITICAL();
	if(pevent->event_wait_num != 0)			 				 /*检查消息队列中是否有线程在等待消息 */
	{
		ks_event_ready(pevent,(void*)0,KS_STATE_WAIT_SEM); 	 /*若有，就使最高优先级的线程进入就绪 */
		KS_EXIT_CRITICAL();
		ks_schedule();    									 /*使就绪的最高优先级线程恢复运行     */
		return (KS_NO_ERROR);
	}
	/*********没有线程在等待消息时，运行下面程序************/
	if(pevent->event_count < 65535)   	  /*确保信号量的值不溢出*/
	{
		pevent->event_count++;            /*信号量的值递增   	*/
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
	if(pevent == (ks_event*)0)  			 /*确保事件控制块可以用 */
	{
		return (0); 						 /*返回错误信息			*/
	}
#endif	
	KS_ENTER_CRITICAL();
	if(pevent->event_count > 0)             /*检查信号量的值是否为零*/
	{ 
		pevent->event_count--;              /*信号量的值递减		*/
	}
	KS_EXIT_CRITICAL();
	return (pevent->event_count);           /*返回信号量的值		*/
}

ks_event *ks_sem_delete(ks_event*pevent,uint8 opt,uint8*error)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
	uint8 thread_waiting;
	if(int_nesting > 0)						/*确保中断服务程子程序无法调用*/
	{
		*error = KS_QUEUE_DEL_ISR;			/*返回中断调用的错误信息	  */
		return (pevent);
	}
#if KS_CHECK_EN > 0   						/*是否支持条件检验			  */
	if(pevent == (ks_event*)0)				/*事件控制块指针为空   		  */
	{
		*error = KS_SEM_EVENT_NULL;			/*返回错误信息				  */
		return (pevent);
	}
	if(pevent->event_type != KS_EVENT_TYPE_SEM)/*确保事件控制块的类型是队列 */
	{
		*error = KS_SEM_EVENT_TYPE;			 /*返回错误信息				  */
		return (pevent);
	}
#endif
	KS_ENTER_CRITICAL();
	if(pevent->event_wait_num != 0)			/*检查消息队列中是否有线程在等待消息*/
	{
		thread_waiting = KS_TRUE;			/*有线程在等待消息					*/
	}
	else
	{
		thread_waiting = KS_FALSE;			/*没有线程在等待消息				*/
	}
	switch(opt)
	{
	case KS_DEL_NO_WAIT:/*仅在没有线程等待消息时才删除消息队列*/
		if(thread_waiting == KS_FALSE)
		{
			KS_EXIT_CRITICAL();
			ks_free((void*)pevent->event_wait_list);       /*释放事件等待链表的空间*/
			ks_free(pevent);                        /*释放事件控制块的空间	*/
				
			*error = KS_NO_ERROR;
			return ((ks_event*)0);
		}
		else
		{
			KS_EXIT_CRITICAL();
			*error = KS_DEL_THREAD_WAITING;         /*返回错误消息				   */
			return (pevent);
		}
	case KS_DEL_ALWAYS:
		while(pevent->event_wait_num != 0) 		   /*将所有等待消息的线程都置于就绪*/
		{
			ks_event_ready(pevent,(void*)0,KS_STATE_WAIT_SEM);
		}
		KS_EXIT_CRITICAL();
		ks_free((void*)pevent->event_wait_list);/*释放事件等待链表的空间         */
		ks_free(pevent);						/*释放事件控制块的空间			 */
		if(thread_waiting == KS_TRUE)			
		{
			ks_schedule();						/*若有任务在等待消息，就调度一次 */
		}
		*error = KS_NO_ERROR;
		return ((ks_event*)0);	
	default:									/*其它情况						 */
		KS_EXIT_CRITICAL();
		*error = KS_ERROR_INVALID_OPT;
		return (pevent);
	}
	
}


#endif