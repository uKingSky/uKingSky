
/*
*********************************************************************************************************
*                                               KingSky
*                                          实时操作系统内核
*
*                                       (c) Copyright 2011, 庄桐泉
*                                          All Rights Reserved
*
* File         : KingSky.h
********************************************************************************************************* */
#ifndef _KINGSKY_H
#define _KINGSKY_H

#ifdef __cplusplus
extern "C" {
#endif

/*
*********************************************************************************************************
*                                           工程包含的头文件
*********************************************************************************************************
*/
#include "ks_cpu.h"
#include "ks_cfg.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <mmsystem.h>	//包含时钟函数的头文件，需要windows.h的支持
#include <conio.h>



#ifdef   KS_GLOBALS
#define  KS_EXT
#else
#define  KS_EXT  extern
#endif

#if KS_MAX_PRIO < 256
	typedef  uint8 KS_BASE_TYPE;
#elif KS_MAX_PRIO <65536
	typedef uint16 KS_BASE_TYPE ;
#else
	typedef uint32 KS_BASE_TYPE ;
#endif

#if  KS_TICK_TYPE == 0          //延时时间的单位
	typedef uint8  KS_TIME_TYPE;
#elif KS_TICK_TYPE == 1
	typedef uint16 KS_TIME_TYPE;
#elif KS_TICK_TYPE == 2
	typedef uint32 KS_TIME_TYPE;
#endif

#ifndef KS_DEL_NO_WAIT              /*仅在没有线程等待消息时才删除消息队列*/
	#define KS_DEL_NO_WAIT  (0x00)
#endif
#ifndef KS_DEL_ALWAYS              /*无论有无线程等待消息时都删除消息队列*/
	#define KS_DEL_ALWAYS   (0x01)
#endif

/*下面是线程的状态*/

#ifndef KS_STATE_READY
	#define KS_STATE_READY      	(0x01)        /*线程处于就绪状态			*/
#endif
#ifndef KS_STATE_SUSPEND
	#define KS_STATE_SUSPEND    	(0x02)		  /*线程处于挂起状态			*/
#endif
#ifndef KS_STATE_FREE
	#define KS_STATE_FREE       	(0x04)        /*线程处于空闲状态，也就被删除*/
#endif
#ifndef KS_STATE_WAIT_Q
	#define KS_STATE_WAIT_Q     	(0x08)        /*正在等待消息队列的类型		*/
#endif
#ifndef KS_STATE_WAIT_SEM
	#define KS_STATE_WAIT_SEM   	(0x10)        /*正在等待信号量的类型		*/
#endif
#ifndef KS_STATE_WAIT_MUTEX
	#define KS_STATE_WAIT_MUTEX     (0x20)        /*正在等待互斥信号量的类型	*/
#endif


 
 #ifndef KS_EVENT_TYPE_Q             
	#define KS_EVENT_TYPE_Q     (0x01)         /*事件类型--队列      */
#endif
#ifndef KS_EVENT_TYPE_SEM    
    #define  KS_EVENT_TYPE_SEM  (0x02)         /*事件类型--信号量    */
 #endif
#ifndef KS_EVENT_TYPE_MUTEX
	#define KS_EVENT_TYPE_MUTEX (0x03)         /*事件类型--互斥信号量*/
#endif
/*对当前任务数进行不同情况的定义*/

#if KS_MAX_THREAD < 256
	typedef uint8 KS_THREAD_NUM ;
#elif KS_MAX_THREAD < 65536
	typedef uint16 KS_THREAD_NUM ;
#else
	typedef uint32 KS_THREAD_NUM ;
#endif
	
typedef void (*THREAD_ADDR)(void *);
typedef void * Q_HANDLE;

/*
*********************************************************************************************************
*                                           工程定义的结构体
*********************************************************************************************************
*/
/*---------------------------定义链表节点的结构------------------------------*/
typedef struct KS_ENDLIST_ITEM
{
	uint32                         item_value;
	volatile struct KS_LIST_ITEM * item_next;
	volatile struct KS_LIST_ITEM * item_prev;
}ks_endlist_item;

typedef struct KS_LIST_ITEM
{
	uint32                         item_value;	         /*链表的数据域				    */			            
	volatile struct KS_LIST_ITEM * item_next;	
	volatile struct KS_LIST_ITEM * item_prev;
	void                         * owner;				/*指向此链表结点所在的线程控制块*/			
	void                         * container;			/* 指向此链表结点所在的链表     */				
}ks_list_item;
typedef struct KS_LIST
{
	volatile KS_BASE_TYPE      item_numbers;            /*链表节点的数量				*/
	volatile ks_list_item    * item_index;			
	volatile ks_endlist_item   list_end;	
#if KS_RR_EN > 0
	volatile uint8             rr_flag;   
#endif                	
} ks_list;
/*---------------------------定义事件的结构体-----------------------------------*/
#if KS_EVENT_EN > 0
typedef struct KS_EVENT{
	void * pevent;      		 		 /*指向消息队列的指针                   */
#if KS_SEM_EN > 0
	uint16 event_count;		     		 /*计数器(当事件是信号量时)，最大为65535*/
#endif
#if KS_MUTEX_EN > 0
	KS_BASE_TYPE pip;
	KS_BASE_TYPE mutex_caller;           /*Mutex的调用者                        */
#endif
	uint8  event_type;     				 /*事件类型                             */
	KS_BASE_TYPE  event_wait_num;        /*消息队列中有线程在等待消息的数量     */
	KS_BASE_TYPE top_eventlist_priority;
    volatile ks_list *event_wait_list;	 /*事件等待链表                         */
}ks_event;
#endif


/*---------------------------定义线程控制块的结构-----------------------------------*/
typedef struct KS_THREAD_BLOCK{
	volatile KS_STACK * top_stack;                  /*定义指向堆栈栈顶的指针        */
	volatile KS_STACK * start_stack;                /*定义指向堆栈起始地址的指针    */
	ks_list_item	    insertlist_item;            /*便于将线程插入链表中          */
	KS_BASE_TYPE        priority;                   /*线程优先级(0==最高)没有限制   */
	uint8    			thread_state;               /*线程的状态,（0==就绪)         */
	uint8               item_in_delaylist;          /*节点是否在延时链表里的标志位  */
#if KS_THREAD_NAME_EN > 0
	uint8     thread_name[KS_THREAD_NAME_MAXLEN];   /*定义线程的名字                */
#endif
#if KS_EVENT_EN > 0
	ks_event            *pthread_event;              /*指向事件控制块的指针         */
	ks_list_item         eventlist_item;             /*用于将线程控制块插入事件链表 */
#endif
#if KS_Q_EN > 0             
	void       *pthread_msg;                     	/*指向传递消息队列的消息的指针  */
#endif 
	struct   KS_THREAD_BLOCK * thread_next;
	KS_TIME_TYPE    thread_delay;                   /*任务延时或等待超时的时钟节拍数*/
}ks_thread_block;

#if KS_Q_EN > 0
typedef struct KS_Q{
	void              **queue_start; 	/*指向消息队列的指针数组的起始地址         */
	void              **queue_end;  	/*指向消息队列结束单元的下一个地址         */
	void              **queue_in;    	/*指向消息队列结束单元的下一个地址指针     */
	void              **queue_out;   	/*指向消息队列中下一个取出消息的位置的指针 */
	uint16            queue_size;    	/*消息队列中总的单元数,最大值为65535       */
	uint16            queue_entries; 	/*消息队列中当前的消息数量，最大数为65536  */
}ks_queue;
#endif


#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef KS_FALSE
	#define KS_FALSE  (0)
#endif

#ifndef KS_TRUE
	#define KS_TRUE   (1)
#endif

typedef ks_thread_block* KS_HANDLE;
#define KS_THREAD_SELF (KS_HANDLE)0xff  


/*
*********************************************************************************************************
*                                           工程定义
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                           ks_core.c的头文件
*********************************************************************************************************
*/

void                 ks_list_initialize( ks_list * newlist );

void                 ks_list_insertend( ks_list *list, ks_list_item *newitem);

void 				 ks_item_remove( ks_list_item *list_item );//删除链表的节点

void                 ks_system_initialize(void);

KS_STACK             * ks_stack_init(THREAD_ADDR thread_addr,void *p_arg,KS_STACK *stk1);

uint8 ks_thread_init(ks_thread_block *ptr,KS_STACK *stk,KS_BASE_TYPE prio,const uint8 *const thread_name);

ks_thread_block      *ks_thread_allocate(void);//申请一块空闲线程

void 				 ks_schedule(void);//调度

void				 ks_start(void);//系统启动函数

void 				 ks_time_tick(void);//节拍服务子程序

void                 ks_int_enter(void);//进入中断

void                 ks_int_exit(void);
void                 ks_schedule_lock(void);  /*调度器上锁*/

void                 ks_schedule_unlock(void); /*调度器解锁*/

/*下面是事件控制块的函数*/

/*初始化事件控制块*/
#if KS_EVENT_EN > 0
void                 ks_event_list_init(ks_event *pevent); /*使事件等待链表中的线程进入事件的就绪态      */
ks_thread_block     *ks_event_ready(ks_event *pevent,void *msg,uint8 msk);/*使线程进入等待某事件发生状态 */
void                 ks_event_wait(ks_event *pevent);     /*线程等待超时而将线程置为就绪态               */
void                 ks_event_to(ks_event *pevent);
#endif
/*下面是内存管理的函数*/
void                *ks_malloc(uint32 size);
void                 ks_free(void *pv);

/*
*********************************************************************************************************
*                                           ks_thread.c的头文件
*********************************************************************************************************
*/

KS_HANDLE 			ks_thread_create(THREAD_ADDR thread_addr,
						 			 const uint8 *const thread_name,
						  			 void *p_arg,
						 			 KS_STACK *stk,
						 			 KS_BASE_TYPE prio);
void                 ks_thread_idle(void *pram);
void				 ks_thread_delay(KS_TIME_TYPE ticks);
uint8 				 ks_thread_suspend(KS_HANDLE handle);
uint8                ks_thread_resume(KS_HANDLE handle);
uint8                ks_thread_close(KS_HANDLE handle);

/*
*********************************************************************************************************
*                                           ks_cpu.c的头文件
*********************************************************************************************************
*/
void                 ks_start_high(void);
void                 ks_thread_sw(void);
void                 ks_int_sw(void);
void CALLBACK ks_tick_isr(unsigned int a,unsigned int b,unsigned long c,unsigned long d,unsigned long e);


/*
*********************************************************************************************************
*                                           ks_queue.c的头文件
*********************************************************************************************************
*/
#if (KS_Q_EN > 0)&& (KS_EVENT_EN > 0)
ks_event     *ks_queue_create(Q_HANDLE*start,uint16 size);
void		 *ks_queue_waitfor(ks_event *pevent,uint16 timeout,uint8 *error);
uint8		  ks_queue_sendmsg(ks_event*pevent,void *msg);
void	     *ks_queue_accept(ks_event*pevent,uint8 *error);
uint8 		  ks_queue_flush(ks_event *pevent);
ks_event 	 *ks_queue_delete(ks_event*pevent,uint8 opt,uint8 *error);
#endif

/*
*********************************************************************************************************
*                                           ks_sem.c的头文件
*********************************************************************************************************
*/
#if (KS_SEM_EN > 0)&& (KS_EVENT_EN > 0)
ks_event     *ks_sem_create(uint16 count);
void          ks_sem_waitfor(ks_event *pevent,uint16 timeout,uint8 *error);
uint8         ks_sem_sendmsg(ks_event*pevent);
uint16        ks_sem_accept(ks_event*pevent);
ks_event     *ks_sem_delete(ks_event*pevent,uint8 opt,uint8*error);
#endif

/*
*********************************************************************************************************
*                                           ks_mutex.c的头文件
*********************************************************************************************************
*/
#if (KS_EVENT_EN > 0)&& (KS_MUTEX_EN > 0)
ks_event 	*ks_mutex_create(KS_BASE_TYPE prio,uint8 *error);
void 		 ks_mutex_waitfor(ks_event * pevent,uint16 timeout,uint8 *error);
uint8 		 ks_mutex_sendmsg(ks_event*pevent);
uint8 		 ks_mutex_accept(ks_event*pevent,uint8 *error);
ks_event   * ks_mutex_delete(ks_event*pevent,uint8 opt,uint8 *error);
#endif


/*
*********************************************************************************************************
*                                          工程定义的全局变量
*********************************************************************************************************
*/
KS_EXT ks_list readylist[KS_MAX_PRIO+1];//就绪链表

KS_EXT    ks_list volatile delaylist[1];

KS_EXT ks_thread_block thread_table [KS_MAX_THREAD];

KS_EXT ks_thread_block *pfree_thread;//指向空闲线程控制块的指针

KS_EXT uint8 ks_running;//系统的运行状态

KS_EXT uint8 lock_nesting;      /*调度器上锁计数器*/

KS_EXT ks_thread_block *current_thread; //volatile

#if KS_RR_EN >0

KS_EXT ks_thread_block *next_thread;

#endif

KS_EXT ks_thread_block *high_thread;

KS_EXT uint8 int_nesting;

KS_EXT  KS_HANDLE h_idle;
KS_EXT KS_THREAD_NUM current_thread_number;//定义当前的任务数

KS_EXT KS_BASE_TYPE top_readylist_priority;//就绪链表的最高优先级

KS_EXT KS_BASE_TYPE  current_running_priority;









/*
*********************************************************************************************************
*                                          工程定义的宏函数
*********************************************************************************************************
*/
#define KS_ITEM_INIT(item)  (ks_list_item *)item->container = (void *)0 

#define KS_ITEM_OWNER(item,owner1) (item)->owner = (void*)owner1















#ifdef __cplusplus
}
#endif

#endif /*文件结束*/
