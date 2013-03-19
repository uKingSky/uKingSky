
/*
*********************************************************************************************************
*                                               KingSky
*                                          实时操作系统内核
*
*                                       (c) Copyright 2011, 庄桐泉
*                                          All Rights Reserved
*
* File         : ks_core.c
*说明：内核操作系统的核心文件
********************************************************************************************************* */

#ifndef  KS_MASTER_FILE
#define  KS_GLOBALS
#include "\KingSky_vc6\KingSky\includes\KingSky.h"
#endif
/*静态函数声明*/
static void ks_readylist_init(void);
static void ks_threadtable_init(void);
static void ks_idle_init(void);
static void ks_variable_init(void);
/*定义空闲线程的栈空间,类型为KS_STACK*/
KS_STACK thread_stack_idle[KS_STACK_MAX_LEN];

/*
 *********************************************************************************************************
 * 							申请一块空闲线程(ks_thread_allocate)
 *
 * 描述: 申请一块空闲线程块
 *
 * 调用：ks_item_init(链表节点初始化)、(ks_thread_block*)0(申请失败)
 *
 * 参数: 无
 *
 * 返回: ptr(申请的空闲线程块指针)、
 *
 * 注意: 在创建线程的时候调用该函数，根据定义的readylist的数量判断是否有空闲线程块
 *********************************************************************************************************
 */
 ks_thread_block *ks_thread_allocate(void)     /*申请一块空闲线程              */
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif

	ks_thread_block *ptr;
	KS_ENTER_CRITICAL();
	ptr = pfree_thread;              		 	/*ptr指向空闲线程控制块的头指针 */
	if(ptr !=(ks_thread_block*)0)     			/*判断是否有空闲的线程块被使用	*/
	{
	 	pfree_thread = ptr->thread_next;  		/*若有，则调整pfree_thread指针	*/
		KS_EXIT_CRITICAL();
		return (ptr);                     		/*返回申请到得指针				*/
	}
	KS_EXIT_CRITICAL();
	return (ks_thread_block*)0;                 /*若无法成功，则返回失败标志    */
}

static void ks_item_init(ks_list_item *item)    /*双向链表节点的初始化			*/
{
	item->container = (void*)0;
}

/*
 *********************************************************************************************************
 * 							线程初始化(ks_thread_init)
 *
 * 描述: 对线程控制块的数据进行初始化
 *
 * 调用：ks_item_init(链表节点初始化)
 *
 * 参数:*ptr(线程控制块)、*stk(栈的起始地址)、prio(优先级)、thread_name(线程的名字)
 *
 * 返回: KS_NOERR_THREAD_INIT：线程初始化无误； KS_ERR_THREAD_INIT：线程初始化错误
 *
 * 注意:在创建线程的时候调用该函数
 *********************************************************************************************************
 */
uint8 ks_thread_init(ks_thread_block *ptr,KS_STACK *stk,KS_BASE_TYPE prio,const uint8 *const thread_name)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
	
	if(ptr != (ks_thread_block*)0)              /*成功申请一块空闲线程       */
	{
		KS_ENTER_CRITICAL();
#if KS_THREAD_NAME_EN > 0					    /*初始化线程控制块的数据     */
		if(thread_name ==(const uint8 *const)0) /*如果thread_name为空		 */
		{
			strncpy(( uint8 *) ptr->thread_name, "", ( uint8 ) KS_THREAD_NAME_MAXLEN );
		}
		else									/*如果thread_name不为空		 */
		{
			strncpy(( uint8 *) ptr->thread_name, ( const uint8 * ) thread_name, ( uint8 ) KS_THREAD_NAME_MAXLEN );
		}
		ptr->thread_name[( uint8 )KS_THREAD_NAME_MAXLEN -( uint8 )1] = '\0';
#endif
		ptr->top_stack    = stk;                   		   
		ptr->priority     =(KS_BASE_TYPE)prio;  /*对线程控制块的数据进行初始化*/
		ptr->thread_state = KS_STATE_READY;
		ptr->item_in_delaylist = 0;

#if KS_RR_EN >0
		ptr->insertlist_item.item_value = KS_TIME_PERC;    /*时间片赋值					 */
#endif
		ks_item_init(&(ptr->insertlist_item));             /*初始化线程链表              */
		KS_ITEM_OWNER(&(ptr->insertlist_item),ptr);        /*节点指向那个线程块          */
#if KS_EVENT_EN > 0
		ks_item_init(&(ptr->eventlist_item));              /*初始化插入事件链表的节点	 */
		KS_ITEM_OWNER(&(ptr->eventlist_item),ptr);         /*设置节点指向哪个线程块      */
#endif
		KS_EXIT_CRITICAL();
		return (KS_NOERR_THREAD_INIT);
	}
	
	return (KS_ERR_THREAD_INIT);
}
/*
 *********************************************************************************************************
 * 							抽象链表初始化(ks_list_initialize)
 *
 * 描述: 对一个链表进行初始化,建立头节点list_end,item_index指向头节点 
 *
 * 调用：无
 *
 * 参数:list(要初始化哪个链表)
 *
 * 返回: 无
 *
 * 注意: 
 *********************************************************************************************************
 */
void ks_list_initialize( ks_list * newlist )                        
{
	newlist->item_index = ( ks_list_item * ) &( newlist->list_end );/*item_index指向链表的头结点  */
	newlist->list_end.item_value = 0;								/*延时的时间数                */
	newlist->list_end.item_next = ( ks_list_item * )&( newlist->list_end);
	newlist->list_end.item_prev = ( ks_list_item * ) &( newlist->list_end);
	newlist->item_numbers = 0;
#if KS_RR_EN >0
	newlist->rr_flag = 0;                                           /*时间片标志位清零	          */
#endif
}
/*
 *********************************************************************************************************
 * 							链表节点插入(ks_list_insertend)
 *
 * 描述: 链表节点插入
 *
 * 调用：无
 *
 * 参数:list(在哪个链表插入)、list_item(要插入的链表节点)
 *
 * 返回: 无
 *
 * 注意: 将要插入的链表和节点作为参数传入，如：ks_list_insertend((ks_list*)&readylist[prio],(ks_list_item*)&ktb->insertlist_item);
 *********************************************************************************************************
 */
void ks_list_insertend( ks_list *list, ks_list_item *newitem)
{
	volatile ks_list_item *  item_index;
	item_index = list->item_index;                 /*item_index指向欲插入链表list的头节点item_index*/
	newitem->item_next = item_index->item_next;
	newitem->item_prev = list->item_index;
	item_index->item_next->item_prev = ( volatile ks_list_item * ) newitem;
	item_index->item_next = ( volatile ks_list_item * ) newitem;
	list->item_index = ( volatile ks_list_item * ) newitem;
	
	newitem->container = ( void * ) list;
	( list->item_numbers )++;                       /*链表的节点数递增                              */
}
/*
 *********************************************************************************************************
 * 							链表节点删除(ks_item_remove)
 *
 * 描述: 链表节点删除
 *
 * 调用：无
 *
 * 参数:list_item(要删除的链表节点)
 *
 * 返回: 无
 *
 * 注意: 将要删除的节点作为参数传入，如：ks_item_remove((ks_list_item*)&ktb->insertlist_item);
 *********************************************************************************************************
 */
void ks_item_remove( ks_list_item *list_item )
{
	ks_list * list;
	
	list_item->item_next->item_prev = list_item->item_prev;
	list_item->item_prev->item_next = list_item->item_next;
	list = ( ks_list * ) list_item->container;
	
	if( list->item_index == list_item )
	{
		list->item_index = list_item->item_prev;
	}
	
	list_item->container = (void*)0;
	( list->item_numbers )--;
}
/*
 *********************************************************************************************************
 * 							就绪链表初始化(ks_readylist_init)
 *
 * 描述: 就绪链表初始化
 *
 * 调用：无
 *
 * 参数: 无
 *
 * 返回: 无
 *
 * 注意: 对每个优先级进行初始化，也就是为每个优先级建立一个双向循环链表
 *********************************************************************************************************
 */
static void ks_readylist_init(void)
{
#if KS_MAX_PRIO < 256                  /*根据内核配置中最多优先级数对i进行定义和初始化                */
	uint8 i = 0;
#elif KS_MAX_PRIO <65536
	uint16 i =0;
#else
	uint32 i = 0;
#endif
	for (i = 0;i < KS_MAX_PRIO+1;i++ ) /*对每个优先级进行初始化，也就是为每个优先级建立一个双向循环链表*/
	{
		ks_list_initialize((ks_list *)&readylist[i]);
	}
}
/*
 *********************************************************************************************************
 * 							线程块链表初始化(ks_threadtable_init)
 *
 * 描述: 将定义的就绪链表又连接成单向链表
 *
 * 调用：无
 *
 * 参数: 无
 *
 * 返回: 无
 *
 * 注意: 每次创建线程事，从单向链表中摘取一个，pfree_thread指向链表的头结点处
 *********************************************************************************************************
 */
static void ks_threadtable_init(void)
{
#if KS_MAX_PRIO < 256
	uint8 i = 0;
#elif KS_MAX_PRIO <65536
	uint16 i =0;
#else
	uint32 i = 0;
#endif
	ks_thread_block *ptcb1;
	ks_thread_block *ptcb2;
	ptcb1 = &thread_table[0];
	ptcb2 = &thread_table[1];
	for (i =0;i<KS_MAX_PRIO;i++)            /*将定义的就绪链表连接成单向链表*/
	{
		ptcb1->thread_next = ptcb2;
		ptcb1 ++;
		ptcb2 ++;
	}
	ptcb1->thread_next = (ks_thread_block*)0;
	pfree_thread = &thread_table[0];
}
/*
 *********************************************************************************************************
 * 							系统初始化时变量初始化(ks_variable_init)
 *
 * 描述: 根据系统的配置，分别对不同变量进行初始化。
 *
 * 调用：无
 *
 * 参数: 无
 *
 * 返回: 无
 *
 * 注意: KS_MAX_PRIO在ks_cfg.h中定义,该函数为静态函数，这能在本源文件调用。
 *********************************************************************************************************
 */
static void ks_variable_init(void)
{
	
#if KS_MAX_PRIO < 256							/*定义变量为8位变量				*/
	top_readylist_priority = 0xff;          
	current_running_priority = 0xff;
#elif KS_MAX_PRIO < 65536						/*定义变量为16位变量			*/
	top_readylist_priority = 0xffff;
	current_running_priority = 0xffff;
#else
	top_readylist_priority = 0xffffffff;		/*定义变量为32位变量			*/
	current_running_priority = 0xffffffff;
#endif
	ks_running = KS_FALSE;
	int_nesting = 0;
	lock_nesting = 0;
	current_thread_number = 0;
#if KS_RR_EN > 0								/*是否使能时间片轮				*/
	next_thread = (ks_thread_block*)0; 
#endif
}
/*
 *********************************************************************************************************
 * 							  创建空闲线程(ks_idle_init)
 *
 * 描述: 创建空闲线程，当系统没有其他优先级可以运行时，系统会调用运行这个空闲线程。
 *
 * 调用：ks_thread_create（线程创建函数）、ks_thread_idle（空闲线程函数）
 *
 * 参数: 无
 *
 * 返回: 无
 *
 * 注意: KS_STK_GROWTH == 1：表示栈的增长方式是从上往下增长，KS_STK_GROWTH == 0：
 		 表示栈的增长方式是从下往上 增长，根据KS_STK_GROWTH的不同值，创建空闲线程。
 		
 *********************************************************************************************************
 */
static void ks_idle_init(void)
{
#if KS_STK_GROWTH == 1
	h_idle = ks_thread_create(ks_thread_idle,(uint8*)"idle",(void*)0,&thread_stack_idle[KS_STACK_MAX_LEN-1],KS_MAX_PRIO);
#elif KS_STK_GROWTH == 0
	h_idle = ks_thread_create(ks_thread_idle,(uint8*)"idle",(void*)0,&thread_stack_idle[0],KS_MAX_PRIO);
#endif
}
/*
 *********************************************************************************************************
 * 									系统初始化(ks_system_initialize)
 *
 * 描述: 系统启动前的初始化，包括全局变量初始化、就绪链表初始化、延时链表初始化、线程块链表初始化
 *
 * 调用：ks_variable_init（变量初始化）、ks_readylist_init（就绪链表初始化)、ks_threadtable_init(线程块链表初始化)
 *       ks_list_initialize(延时链表初始化)、ks_idle_init(创建空闲线程)
 *
 * 参数: 无
 *
 * 返回: 无
 *
 * 注意: KS_STK_GROWTH == 1：表示栈的增长方式是从上往下增长，KS_STK_GROWTH == 0：
 *		 表示栈的增长方式是从下往上 增长，根据KS_STK_GROWTH的不同值，创建空闲线程。
 *		
 *********************************************************************************************************
 */
void ks_system_initialize(void)
{
	
	ks_variable_init();							  		/*对一些全局变量进行初始化*/
	ks_readylist_init();						  		/*对就绪链表进行初始化    */
	ks_threadtable_init();
	ks_list_initialize((ks_list *)&delaylist[0]); 		/*对延时链表进行初始化    */
	ks_idle_init();
}

void ks_schedule(void)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
	KS_BASE_TYPE top = 0;
	KS_ENTER_CRITICAL();
	/*如果任务的调度禁止的（调度上锁），或当前处于中断服务子程序，线程的调度是不允许的。*/
	if(int_nesting == 0&& lock_nesting == 0)                     
	{
		for (top = top_readylist_priority;top < (KS_BASE_TYPE)(KS_MAX_PRIO+(KS_BASE_TYPE)1);++top)
		{
			if(readylist[top].item_numbers != 0)/*遍历查找*/
			{
				break;
			}
		}/*查找到就绪表的最高优先级就是top*/
		
	
		top_readylist_priority = top;                                        /*更新top_readylist_priority的值			   */
		if(current_running_priority != top_readylist_priority) 				 /*如果就绪表的最高优先级不是当前线程的优先级  */
		{
			current_running_priority = top_readylist_priority; 				 /*更新当前运行的优先级的值                    */
			high_thread = (ks_thread_block*)readylist[top].item_index->owner;/*将当前指针指向即将运行的最高优先级的线程    */
			high_thread->item_in_delaylist = 0;								 /*即将运行的线程块在就绪链表里，不在延时链表里*/
			KS_EXIT_CRITICAL();
			KS_THREAD_SW();													 /*进行线程级别的调度                          */
		}
#if KS_RR_EN >0																 /*是否使能时间片							   */
		else 
		{
			if((readylist[current_running_priority].item_numbers) >= (KS_BASE_TYPE)1)
			{
				if(next_thread == (ks_thread_block*)0)                       /*确保next_thread不为空					   */
				{
					KS_EXIT_CRITICAL();
					return;
				}
				high_thread = next_thread;									/*即将运行的线程块指针指向next_thread		   */
				KS_THREAD_SW();                                             /*线程级别的调度							   */
			}
		}
#endif		
		/*如果查找出来最高优先级的任务还是运行态的任务，就不进行调度，直接运行。*/
	}

	KS_EXIT_CRITICAL();
}

void ks_start(void)
{
	if(ks_running == KS_FALSE)             /*判断系统是否已经启动      */
	{
		
		current_thread = high_thread;
		ks_start_high();
		/*上面函数实现ks_running = KS_TRUE;将多任务系统运行状态标志为TRUE；
		  调用准备就绪的最高优先级任务启动函数，要用汇编来写。         */
	}
}


void ks_time_tick(void)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
     KS_BASE_TYPE i,num,prio = 0;
	 volatile  ks_list_item * volatile p;					/*定义指向链表节点的指针		*/
	 ks_thread_block *ktb;									/*定义指向线程控制块的指针		*/
	 num = delaylist[0].item_numbers;                   	/*延时链表节点的数量		    */
	 
	 p = delaylist[0].item_index->item_next->item_next; 	/*p指向链表的第一个有数据的节点 */
	 
	 for(i =0; i < num; i++)                                /*从第一块线程控制块开始遍历	*/
	{
		KS_ENTER_CRITICAL();     							/*进入临界区					*/                      

		ktb = (ks_thread_block*)(p->owner);					/*指针ktb指向遍历到得线程控制块	*/			
		if(ktb->thread_delay != 0)
		{
			if(--(ktb->thread_delay) == 0)					/*延时的滴答数递减				*/
			{
				if(!(ktb->thread_state & KS_STATE_SUSPEND)) /*检验线程是否被挂起			*/
				{
					p = p->item_prev;
					prio = ktb->priority;
					ks_item_remove((ks_list_item*)&ktb->insertlist_item); /*将ktb所指向的线程块从就绪链表中删除 */
					ks_list_insertend((ks_list*)&readylist[prio],(ks_list_item*)&ktb->insertlist_item);/*将ktb所指向的线程块插入到延时链表中*/
					ktb->item_in_delaylist = 1;				 			  /*表示ktb所指向的线程块在延时链表中   */
					if (prio < top_readylist_priority)
					{
						top_readylist_priority = prio;					  /*更新top_readylist_priority的值      */
					}
				}
				 else													  /*若是线程被挂起的状态				*/
				{
						ktb->thread_delay = 1;                            /*延时数置1，保持挂起状态				*/
				}
				
			}
		}
		p = p->item_next;												 /*指针p指向下一个线程控制块			*/
		KS_EXIT_CRITICAL();												 /*退出临界区							*/
	}
	 
#if KS_RR_EN > 0														 /*是否使能时间片转						*/
	KS_ENTER_CRITICAL();												 /*进入临界区							*/
	
	if(readylist[current_thread->priority].rr_flag == (uint8)1)			 /*当前线程块所在链表是否有rr_flag(时间片转)的标记*/
	{
		if(--(current_thread->insertlist_item.item_value) ==0)           /*对当前的线程块的时间片递减					  */
		{
			if (readylist[current_thread->priority].item_numbers > (KS_BASE_TYPE)1)
			{
				if(current_thread->insertlist_item.item_next->item_value == 0)/*判断current_thread的节点在链表中是否为最后一个节点*/
				{
					high_thread =(ks_thread_block*)current_thread->insertlist_item.item_next->item_next->owner;/*high_thread指向下一个节点所对应的线程控制块*/
				}
				else														 /*high_thread指向下一个节点所对应的线程控制块		*/
				{
					high_thread =(ks_thread_block*)current_thread->insertlist_item.item_next->owner;
				}
			}
			current_thread->insertlist_item.item_value = KS_TIME_PERC;       /*设置线程控制块current_thread的时间片为KS_TIME_PERC*/
		}
	}

	KS_EXIT_CRITICAL();														 /*退出临界区						*/
#endif	
	
}

void ks_int_enter(void)//进入中断
{
	if(ks_running == KS_TRUE)		  /*判断系统是否启动				  */
	{
		if(int_nesting < 255)         /*判断中断嵌套int_nesting是否大于255*/
		{
			int_nesting ++;           /*否则中断嵌套int_nesting递增		  */
		}
	}
}
extern void ks_int_sw(void);

void ks_int_exit(void)                /*中断离开*/
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
	KS_BASE_TYPE top = 0;

	KS_ENTER_CRITICAL();
	if(ks_running == KS_TRUE)
	{
		if(int_nesting > 0) 
		{
			int_nesting --;							/*中断前套数递减				   */
		}
		if(int_nesting == 0 && lock_nesting == 0)   /*判断中断嵌套是否为0，以及是否被锁*/
		{
													/*遍历查找优先级				   */                                     
			for (top = top_readylist_priority;top < (KS_BASE_TYPE)(KS_MAX_PRIO+(KS_BASE_TYPE)1);++top)
			{
				if(readylist[top].item_numbers != 0)/*查找到就绪表的最高优先级就是top  */
				{
					break;
				}
			}										
				top_readylist_priority = top;
		
			if(current_running_priority != top_readylist_priority)   /*如果就绪表的最高优先级不是当前线程的优先级  			*/
			{
				current_running_priority = top_readylist_priority;   /*更新当前运行的优先级的值                    			*/
				high_thread = (ks_thread_block*)readylist[top].item_index->owner;/*将当前指针指向即将运行的最高优先级的任务	*/
				high_thread->item_in_delaylist = 0;					 /*即将运行的线程块在就绪链表里，不在延时链表里			*/
				ks_int_sw(); 										 /*进行中断级别的调度						   			*/
		
				
			}

			else if(current_running_priority == top_readylist_priority)
			{
				if(readylist[current_thread->priority].item_numbers > 1)
				{
					ks_int_sw();									/*进行中断级别的调度									*/
				}
			}
		}
	}	
	KS_EXIT_CRITICAL(); /*退出临界区	*/
}

#if KS_SCHEDULE_LOCK_EN > 0

void ks_schedule_lock(void)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
	if(ks_running == KS_TRUE)  /*确保系统处于运行状态								*/
	{
		KS_ENTER_CRITICAL();   /*进入临界区											*/
		if(lock_nesting < 255) /*锁定嵌套计数器是否<255，系统最多支持255的嵌套计数器*/
		{
			lock_nesting++;    /*嵌套计数器加1										*/
		}
		KS_EXIT_CRITICAL();    /*退出临界区											*/                                                                                                                                                                                                                                                                                                                                    
	}
}

void ks_schedule_unlock(void)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
	if(ks_running == KS_TRUE)      /*确保系统处于运行状态				*/
	{
		KS_ENTER_CRITICAL();
		if(lock_nesting > 0)
		{
			lock_nesting--;        /*嵌套计数器减1						*/
			if(lock_nesting==0 && int_nesting == 0)/*确保调用者不是ISR	*/
			{
				KS_EXIT_CRITICAL();
				ks_schedule();     /*进行调度							*/
			}
			else
			{
				KS_EXIT_CRITICAL();/*退出临界区							*/
			}
		}
		else
		{
			KS_EXIT_CRITICAL();   /*退出临界区							*/
		}
	}
}

#endif

#if KS_EVENT_EN > 0

 void ks_event_list_init(ks_event *pevent)
{
#if KS_MAX_PRIO < 256      /*根据不同的优先级数量定义不同的i，并对op_eventlist_priority进行初始化*/
	uint8 i = 0;
	pevent->top_eventlist_priority = 0xff;
#elif KS_MAX_PRIO <65536
	uint16 i =0;
	pevent->top_eventlist_priority = 0xffff;
#else
	uint32 i = 0;
	pevent->top_eventlist_priority = 0xffffffff;
#endif
	pevent->event_wait_num = 0;       /*等待消息的线程数量,0=没有线程在等待消息*/
	/*对每个优先级的链表进行初始化*/
	pevent->event_wait_list = (ks_list*)ks_malloc((KS_MAX_PRIO+1)*sizeof(ks_list));
	for (i = 0;i < KS_MAX_PRIO+1;i++ )
	{
		ks_list_initialize((ks_list *)&pevent->event_wait_list[i]);
	}
	
}

/*使事件等待链表中的线程进入线程的就绪态*/
ks_thread_block * ks_event_ready(ks_event *pevent,void *msg,uint8 msk)
{
	KS_BASE_TYPE top = 0;
	ks_thread_block *high_event_thread;
	for (top = pevent->top_eventlist_priority;top < (KS_BASE_TYPE)(KS_MAX_PRIO+(KS_BASE_TYPE)1);++top)
	{
		if(pevent->event_wait_list[top].item_numbers != 0)
		{
				break;
		}
	}
	/*查找到等待链表中的最高优先级就是top*/
	pevent->top_eventlist_priority = top;
    /*支持时间轮转法*/
#if KS_RR_EN > 0
	if(pevent->event_wait_list[top].item_numbers > 1)/*有多个同等优先级的情况下，优先选择先到达的线程块*/
	{
		high_event_thread =(ks_thread_block*)pevent->event_wait_list[top].item_index->item_next->item_next->owner;
	}
	else
#endif
	{
		high_event_thread =(ks_thread_block*)pevent->event_wait_list[top].item_index->owner;
	}
	
	ks_item_remove((ks_list_item*)&high_event_thread->insertlist_item);/*将事件等待链表中的最高优先级线程删除*/
	ks_item_remove((ks_list_item*)&high_event_thread->eventlist_item);
	
	high_event_thread->thread_delay = 0;		    /*直接将该变量清零						  */ 								
	high_event_thread->pthread_event = (ks_event*)0;/*线程控制块中指向事件控制块的指针设为NULL*/
#if KS_Q_EN > 0
 /*如果ks_event_ready()是由消息队列发送函数调用，
 那么该函数还要将相应的消息传递给等待链表中最高优先级的线程块，并且放在它的任务控制块中*/
	high_event_thread->pthread_msg = msg;
#else
	msg = msg;								  /*防止编译器编译警告							  */
#endif
	high_event_thread->thread_state &= ~msk;  /*位屏蔽码msk作为参数传递给它,恢复线程原先的状态*/
	
	if(high_event_thread->thread_state == KS_STATE_READY)/*将线程插入到线程就绪表中			  */
	{
		ks_list_insertend((ks_list*)&readylist[top],(ks_list_item*)&high_event_thread->insertlist_item);
	}
	else												/*将线程插入到线程延时链表中          */	
	{
		ks_list_insertend((ks_list*)&delaylist[0],(ks_list_item*)&high_event_thread->insertlist_item);
	}
	pevent->event_wait_num--; 							/*等待消息的线程数减1				  */
	return (high_event_thread);
}
/*使线程进入等待某事件发生状态*/
void ks_event_wait(ks_event *pevent)
{
	current_thread->pthread_event = pevent; /*将事件控制块的指针放到线程控制块中*/
#if KS_RR_EN >0
		if(readylist[current_running_priority].item_numbers > (KS_BASE_TYPE)1)
		{
			if(current_thread->insertlist_item.item_next->item_value == 0)
			{
				next_thread =(ks_thread_block*)current_thread->insertlist_item.item_next->item_next->owner;
			}
			else if(current_thread->insertlist_item.item_next->item_value > 0)
			{
				next_thread =(ks_thread_block*)current_thread->insertlist_item.item_next->owner;
			}
		}
#endif
	ks_item_remove((ks_list_item*)&current_thread->insertlist_item);/*将线程从线程就绪表中删除*/
	ks_list_insertend((ks_list*)&delaylist[0],(ks_list_item*)&current_thread->insertlist_item);
										/*将线程放到事件等待链表中				*/
	 ks_list_insertend((ks_list*)&pevent->event_wait_list[current_thread->priority],(ks_list_item*)&current_thread->eventlist_item);
	pevent->event_wait_num ++;          /*等待消息的线程数加1					*/
	if(current_thread->priority < pevent->top_eventlist_priority)
	{									/*更新pevent->top_eventlist_priority的值*/
		pevent->top_eventlist_priority = current_thread->priority;
	}
}
/*线程等待超时而将线程置为就绪态*/
void ks_event_to(ks_event *pevent)
{
	ks_item_remove((ks_list_item*)&current_thread->eventlist_item);   /*将线程从事件等待链表中删除  */
																	  /*将线程插入到线程就绪链表中	*/
	current_thread->thread_state = KS_STATE_READY; 					  /*线程的状态标志为就绪态		*/
	current_thread->pthread_event = (ks_event*)0; 					  /*删除指向事件控制块的指针    */
	pevent->event_wait_num --;      								  /*等待消息的线程数减1     	*/
}
#endif

/*内存申请函数*/
void *ks_malloc(uint32 size)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
	void *preturn;
	KS_ENTER_CRITICAL();            /*进入临界区			*/
	preturn = malloc(size);         /*申请内存				*/
	KS_EXIT_CRITICAL();             /*离开临界区			*/
	return preturn;                 /*返回申请的内存指针地址*/
}

void ks_free(void *pv)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
	if(pv)
	{
		KS_ENTER_CRITICAL();            /*进入临界区	*/
		free(pv);                       /*释放内存		*/
		KS_EXIT_CRITICAL();             /*离开临界区	*/
	}
}




