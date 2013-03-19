/*
*********************************************************************************************************
*                                               KingSky
*                                          实时操作系统内核
*
*                                       (c) Copyright 2011, 庄桐泉
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
	if(int_nesting > 0)    		 /*确保中断服务程子程序无法调用*/
	{
		return ((ks_event*)KS_MUTEX_CREATE_ERROR);
	}
#if KS_RR_EN >0
	if(prio > KS_MAX_PRIO) 		 /*防止数组溢出				   */
	{
		prio = KS_MAX_PRIO - 1;
	}
#else							 /*如果系统不支持时间轮转法    */
	#if KS_CHECK_EN >0
	if(prio > KS_MAX_PRIO)
	{
		return ((ks_event*)0);   /*返回事件空指针              */
	}
	#endif
	if(readylist[prio].item_numbers >= 1)
	{
		return ((ks_event*)0);
	}
#endif
	pevent = (ks_event*)ks_malloc(sizeof(ks_event));     /*动态申请一个事件块   */
	if(pevent == (ks_event*)0)      					 /*事件块申请失败     	*/
	{
		*error = KS_EVENT_TYPE_NULL;					 /*设置错误信息			*/
		return((ks_event*)0);							 /*返回事件空指针       */
	}
	pevent->event_type = KS_EVENT_TYPE_MUTEX;            /*设置事件控制块的类型 */
	pevent->pip = prio;                                  /*保存PIP              */
#if KS_MAX_PRIO < 256
	pevent->mutex_caller = 0xff;                         /*初始化mutex_caller   */
#elif KS_MAX_PRIO <65536
	pevent->mutex_caller = 0xffff;                       /*表示现在还没有占用者 */
#else
	pevent->mutex_caller = 0xffffffff;                   
#endif
	pevent->pevent = (void*)0;                           /*没有线程在等待       */
	ks_event_list_init(pevent);							 /*初始化等待链表       */
	*error = KS_NO_ERROR;
	return(pevent);                                      /*返回事件控制块指针   */
}

void ks_mutex_waitfor(ks_event * pevent,uint16 timeout,uint8 *error)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
	ks_thread_block *ptr;
	uint8 thread_ready;
	if(int_nesting > 0)                     	 /*确保中断服务程子程序无法调用			  */
	{
		*error = KS_MUTEX_WAIT_ISR;         	 /*返回中断调用的错误信息				  */
		return;
	}
#if KS_CHECK_EN > 0                         	 /*是否支持条件检验            			  */
	if(pevent == (ks_event*)0)             		 /*事件控制块指针为空					  */
	{
		*error = KS_EVENT_TYPE_NULL;       		 /*设置错误信息							  */
		return;
	}
#endif
	if(pevent->event_type != KS_EVENT_TYPE_MUTEX)  /*确保事件控制块的类型是队列			  */
	{
		*error = KS_MUTEX_TYPE_ERROR;         	   /*设置错误信息				    	  */
		return;
	}
	KS_ENTER_CRITICAL();
	if(pevent->mutex_caller == KS_MUTEX_AVALIABLE)/*检查是否有可用的Mutex     			  */
	{											  /*若有就占用							  */
		pevent->mutex_caller = current_thread->priority;	/*保存占用者的优先级		  */
		pevent->pevent = (void*)current_thread;				/*将指针指向占用者的线程控制块*/
		KS_EXIT_CRITICAL();
		*error = KS_NO_ERROR;                             	/*设置返回信息				  */
		return;												/*返回						  */
	}
	
	ptr = (ks_thread_block*)(pevent->pevent); 				/*将指针指向Mutex占用者的线程控制块  */
	if(ptr->priority != pevent->pip && pevent->mutex_caller > current_thread->priority)
	{		
															/*判断是否需要提升Mutex占用者的优先级*/
		if(ptr->item_in_delaylist == 0)						/*判断占用者是否就绪				 */
		{													/*使占用者脱离就绪				  	 */
			ks_item_remove((ks_list_item*)&(ptr->insertlist_item));
			thread_ready = KS_TRUE;							/*设置就绪状态标志					 */
		}
		else
		{
			thread_ready = KS_FALSE;						/*设置非就绪状态标志				 */
		}
		ptr->priority = pevent->pip;						/*将占用者的优先级改为PIP						*/
		if(thread_ready == KS_TRUE)							/*如果原占用者是就绪的							*/
		{
			ks_list_insertend((ks_list*)&readylist[ptr->priority],(ks_list_item*)&(ptr->insertlist_item));
		}                                               	/*使占用者用新的的优先级就绪					*/
	}
	current_thread->thread_state |= KS_STATE_WAIT_MUTEX;	/*设置状态标志，表示在等待消息互斥信号量的消息	*/
	current_thread->thread_delay = timeout;        			/*设置超时										*/
	ks_event_wait(pevent);   					   			/*挂起线程直到事件发生或超时期满				*/
	KS_EXIT_CRITICAL();
	ks_schedule();           					   			/*当前任务被挂起，进行调度						*/
	/*只有超时或正确得到消息，才运行下面的程序*/
	KS_ENTER_CRITICAL();
	if((current_thread->thread_state & KS_STATE_WAIT_MUTEX) >0)/*挂起状态未取消，是因为超时期满			    */
	{
		
		ks_event_to(pevent); 								 /*因为超时使线程就绪			 			    */
		Uart_Printf("I don't get the message\n");
		KS_EXIT_CRITICAL();
		*error = KS_MUTEX_TIMEOUT;
		return;
	}
	/*不是因为超时期满而取得运行权,收到消息,运行下面程序*/
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
	if(int_nesting > 0)                     	   /*确保中断服务程子程序无法调用	 */
	{
		return(KS_MUTEX_WAIT_ISR);
	}
#if KS_CHECK_EN > 0                         	   /*是否支持条件检验            	 */
	if(pevent == (ks_event*)0)             		   /*事件控制块指针为空				 */
	{
		return(KS_EVENT_TYPE_NULL);
	}
#endif
	if(pevent->event_type != KS_EVENT_TYPE_MUTEX)  /*确保事件控制块的类型是队列		 */
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
	if(pevent->event_wait_num != 0)			 					 	 /*检查是否有线程在等待消息       	 */
	{
		ptr = ks_event_ready(pevent,(void*)0,KS_STATE_WAIT_MUTEX); 	 /*若有，就使最高优先级的线程进入就绪*/
		pevent->mutex_caller = ptr->priority;
		pevent->pevent = ptr;
		KS_EXIT_CRITICAL();
		ks_schedule();    										     /*使就绪的最高优先级线程恢复运行	 */
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
	if(int_nesting > 0)                     	   /*确保中断服务程子程序无法调用	 */
	{
		*error = KS_MUTEX_WAIT_ISR;
		return (0);
	}
	#if KS_CHECK_EN > 0                            /*是否支持条件检验            	 */
	if(pevent == (ks_event*)0)             		   /*事件控制块指针为空				 */
	{
		*error = KS_EVENT_TYPE_NULL;
		return (0);
	}
#endif
	if(pevent->event_type != KS_EVENT_TYPE_MUTEX)  /*确保事件控制块的类型是队列		 */
	{
		*error = KS_MUTEX_TYPE_ERROR;
		return (0);
	}
	KS_ENTER_CRITICAL();
	if(pevent->mutex_caller == KS_MUTEX_AVALIABLE) 			/*检查是否有可用的Mutex       */
	{											   			/*若有就占用				  */
		pevent->mutex_caller = current_thread->priority;	/*保存占用者的优先级		  */
		pevent->pevent = (void*)current_thread;				/*将指针指向占用者的线程控制块*/
		KS_EXIT_CRITICAL();
		*error = KS_NO_ERROR;                             	/*设置返回信息				  */
		return (1);										    /*返回						  */
	}
	KS_EXIT_CRITICAL();
	*error = KS_NO_ERROR;                             	    /*设置返回信息				  */ 
	return (0);
}

ks_event * ks_mutex_delete(ks_event*pevent,uint8 opt,uint8 *error)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
	uint8 thread_waiting;
	if(int_nesting > 0)                     	   /*确保中断服务程子程序无法调用*/
	{
		*error = KS_MUTEX_WAIT_ISR;
		return ((ks_event*)0);
	}
	#if KS_CHECK_EN > 0                            /*是否支持条件检验            */
	if(pevent == (ks_event*)0)             		   /*事件控制块指针为空			 */
	{
		*error = KS_EVENT_TYPE_NULL;
		return ((ks_event*)0);
	}
#endif
	if(pevent->event_type != KS_EVENT_TYPE_MUTEX)  /*确保事件控制块的类型是队列	*/
	{
		*error = KS_MUTEX_TYPE_ERROR;
		return ((ks_event*)0);
	}
	KS_ENTER_CRITICAL();
	if(pevent->event_wait_num != 0)			/*检查消息队列中是否有线程在等待消息*/
	{
		thread_waiting = KS_TRUE;			/*有线程在等待消息                  */
	}
	else
	{
		thread_waiting = KS_FALSE;			/*没有线程在等待消息             	*/
	}
	switch(opt)
	{
	case KS_DEL_NO_WAIT:/*仅在没有线程等待消息时才删除消息队列					*/
		if(thread_waiting == KS_FALSE)
		{
			KS_EXIT_CRITICAL();
			ks_free(pevent->event_wait_list);       /*释放事件等待链表的空间	*/
			ks_free(pevent);                        /*释放事件控制块的空间  	*/
				
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
		while(pevent->event_wait_num != 0) 			/*将所有等待消息的线程都置于就绪*/
		{
			ks_event_ready(pevent,(void*)0,KS_STATE_WAIT_MUTEX);
		}
		KS_EXIT_CRITICAL();
		ks_free(pevent->event_wait_list);			/*释放事件等待链表的空间*/
		ks_free(pevent);							/*释放事件控制块的空间*/
		if(thread_waiting == KS_TRUE)			
		{
			ks_schedule();							/*若有任务在等待消息，就调度一次*/
		}
		*error = KS_NO_ERROR;
		return ((ks_event*)0);	
	default:										/*其它情况*/
		KS_EXIT_CRITICAL();
		*error = KS_ERROR_INVALID_OPT;
		return (pevent);
	}
	
	
	
}










#endif