/*
*********************************************************************************************************
*                                               KingSky
*                                          实时操作系统内核
*
*                                       (c) Copyright 2011, 庄桐泉
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
	if(prio > KS_MAX_PRIO)            /*防止数组溢出，一定要控制好*/
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
	ptr  = ks_thread_allocate();                     /*申请一块空闲的线程                   */
	stk1 = ks_stack_init (thread_addr,p_arg,stk);    /*线程堆栈进行初始化,返回任务堆栈的栈顶*/
	err  = ks_thread_init(ptr,stk1,prio,thread_name);/*对申请的线程进行初始化               */
	
	if(err == KS_NOERR_THREAD_INIT)
	{
		/*关中断,注意不要关中断的时间太长，太长影响实时性,中断响应时间*/
		KS_ENTER_CRITICAL();
		current_thread_number ++;                      /*线程数加一							*/
		if(current_thread_number == (KS_THREAD_NUM)1)  /*判断是否第一次创建线程             */
		{
			high_thread = ptr;
			current_running_priority = ptr->priority;  /*设置当前运行的优先级				*/
		}
		else                                           /*如果不是第一次创建线程				*/
		{
			if(ks_running == KS_FALSE)  				/*判断系统是否已经启动				*/ 
			{
				if(current_thread->priority > ptr->priority)
				{
					high_thread = ptr;
					current_running_priority = ptr->priority;/*更新current_running_priority的值*/
				}
			}
		}
		if(top_readylist_priority > ptr->priority) 
		{
			top_readylist_priority = ptr->priority;          /*更新top_readylist_priority的值  */
		}
		ks_list_insertend((ks_list*)&readylist[ptr->priority],(ks_list_item*)&(ptr->insertlist_item));/*将线程插入到优先级就绪链表中*/
#if KS_RR_EN > 0
		if(readylist[ptr->priority].item_numbers > 1)
		{
			readylist[ptr->priority].rr_flag = 1;            /*设置rr_flag标志                 */
		}
#else
		if(readylist[ptr->priority].item_numbers > 1)        /*如果系统不支持相同优先级		   */
		{
			ks_item_remove((ks_list_item*)&(ptr->insertlist_item));
			ptr->thread_next = pfree_thread;                 /*收回资源               		    */
			pfree_thread = ptr;
			return(ks_thread_block*)(KS_PRIO_ERROR);
		}
#endif
		
		KS_EXIT_CRITICAL();
		if(ks_running == KS_TRUE)							/*判断系统是否在运行				*/
		{
			if(current_thread->priority > ptr->priority)
			{
				ks_schedule();								/*进行调度							*/
			}
		
		}
	}
	else if(err == KS_ERR_THREAD_INIT)						/*线程初始化失败后的处理			*/
	{
	
		return(ks_thread_block*)(KS_PRIO_ERROR);			/*失败处理							*/
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
#if KS_RR_EN >0                        /*是否支持相同优先级*/
		if(readylist[current_running_priority].item_numbers > (KS_BASE_TYPE)1)
		{
			if(current_thread->insertlist_item.item_next->item_value == 0)/*运行的链表是否在链表的尾部*/
			{
				next_thread =(ks_thread_block*)current_thread->insertlist_item.item_next->item_next->owner;
			}
			else if(current_thread->insertlist_item.item_next->item_value > 0)
			{
				next_thread =(ks_thread_block*)current_thread->insertlist_item.item_next->owner;
			}
		}
#endif
		ks_item_remove((ks_list_item*)&current_thread->insertlist_item);/*将当前运行态的任务块节点从就绪链表中删除*/
		
												 /*将节点插入到延时链表中			   */
		current_thread->thread_delay = ticks;    /*将ticks的值赋给thread_delay         */
		current_thread->item_in_delaylist = 1;   /*标志正在运行的线程将插入到延时链表中*/
		ks_list_insertend((ks_list*)&delaylist[0],(ks_list_item*)&current_thread->insertlist_item);
		KS_EXIT_CRITICAL(); 
		ks_schedule();        					 /*调度								   */
	}
												/*如果ticks等于0表示不延时,该任务继续执行*/	
}

void ks_thread_idle(void *pram)
{
	pram = pram;
	Uart_SendString("I am the idle\n");
	for(;;)//进去之后就再也没出来了
	{
		//空闲任务空转
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
	if(handle->thread_state & KS_STATE_SUSPEND > 0) /*判断要被挂起的线程是否应经被挂起 */
	{
		return (KS_THREAD_SUSPEND_AGAIN);
	}
	if(handle == (KS_HANDLE)KS_PRIO_ERROR)         /*判断要被挂起的线程是否正确		   */
	{
		return (KS_THREAD_SUSPEND_ERROR);
	}
	if(handle == h_idle)							/*判断被挂起的线程是否是空闲线程   */
	{
		return (KS_THREAD_SUSPEND_IDLE);
	}
#endif
	KS_ENTER_CRITICAL();
	
	if(handle == KS_THREAD_SELF)  					/*判断被挂起的线程是否是正在运行的线程*/
	{
		self = TRUE;								/*self置1							  */
		handle = current_thread;
	}
	else if( handle == current_thread)             /*判断被挂起的线程是否是正在运行的线程 */
	{
		self = TRUE;
	}
	if(handle->item_in_delaylist == 0)			  /*如果被删除的线程在就绪链表里		  */
	{
		ks_item_remove((ks_list_item*)&handle->insertlist_item);
		ks_list_insertend((ks_list*)&delaylist[0],(ks_list_item*)&handle->insertlist_item);
		handle->item_in_delaylist = 1;           /*标志被挂起的线程在延时链表里			  */
	}
	handle->thread_state |= KS_STATE_SUSPEND;    /*设置被挂起线程的状态为挂起态			  */
	KS_EXIT_CRITICAL();
	if(self == TRUE)                             /*如果被挂起的线程是正在运行的线程		  */
	{
		ks_schedule();							 /*线程级别的调度						  */
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
	if(handle == (KS_HANDLE)KS_PRIO_ERROR)            /*判断要被唤醒的线程是否正确   */
	{
		return (KS_THREAD_SUSPEND_ERROR);
	}
#endif
    KS_ENTER_CRITICAL();
    if((handle->thread_state & KS_STATE_SUSPEND) > 0)/*判断被唤醒的线程是否已经被挂起 */
    {
    	handle->thread_state &= ~KS_STATE_SUSPEND;   /*取消线程被挂起的状态           */
    	ks_item_remove((ks_list_item*)&handle->insertlist_item);
		ks_list_insertend((ks_list*)&readylist[handle->priority],(ks_list_item*)&handle->insertlist_item);
		handle->item_in_delaylist = 0;				/*被唤醒的线程不在延时链表里	  */
		KS_EXIT_CRITICAL();
		ks_schedule();                               /*调度							  */
    }
    else
    {
    	KS_EXIT_CRITICAL();
    }
    return (KS_NO_ERROR);

}
#endif

#if KS_THREAD_CHANGE_PRIO > 0 //以后再完成
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
	if(int_nesting > 0)              /*确保中断内不能调用               */
	{
		return (KS_THREAD_DEL_ISR);  /*若在中断内调用，则返回错误标识符 */
	}
#if KS_CHECK_EN >0
	if(handle == (KS_HANDLE)KS_PRIO_ERROR)   /*确保要删除线程是存在的   */
	{
		return (KS_THREAD_DEL_ERROR);
	}
	if(handle == h_idle)                    /*确保删除的不是空闲任务    */
	{
		return(KS_THREAD_DEL_IDLE);
	}
#endif
	KS_ENTER_CRITICAL();
	if(handle == KS_THREAD_SELF)     /*允许删除自己                    */
	{
		handle = current_thread;     /*将当前运行线程块指针赋给handle  */
	}
	if(handle->thread_state != KS_STATE_FREE)    /*确保线程是存在的    */
	{
		ks_item_remove((ks_list_item*)&handle->insertlist_item);/*将线程块从就绪链表或延时链表中删除  */
		handle->thread_delay = 0;            /*将延时数清零，以确保自己重开中断后，ISR不再使该任务就绪*/
		current_thread_number--;             /*将任务数减1                                            */
		handle->thread_next = pfree_thread;  /*将被删除的线程控制块放回空闲链表中去以便被其它线程使用 */
		pfree_thread = handle;
		handle->thread_state = KS_STATE_FREE;/*空闲状态，可以被申请使用                               */
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


