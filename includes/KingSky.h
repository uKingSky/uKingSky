
/*
*********************************************************************************************************
*                                               KingSky
*                                          ʵʱ����ϵͳ�ں�
*
*                                       (c) Copyright 2011, ׯͩȪ
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
*                                           ���̰�����ͷ�ļ�
*********************************************************************************************************
*/
#include "ks_cpu.h"
#include "ks_cfg.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <mmsystem.h>	//����ʱ�Ӻ�����ͷ�ļ�����Ҫwindows.h��֧��
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

#if  KS_TICK_TYPE == 0          //��ʱʱ��ĵ�λ
	typedef uint8  KS_TIME_TYPE;
#elif KS_TICK_TYPE == 1
	typedef uint16 KS_TIME_TYPE;
#elif KS_TICK_TYPE == 2
	typedef uint32 KS_TIME_TYPE;
#endif

#ifndef KS_DEL_NO_WAIT              /*����û���̵߳ȴ���Ϣʱ��ɾ����Ϣ����*/
	#define KS_DEL_NO_WAIT  (0x00)
#endif
#ifndef KS_DEL_ALWAYS              /*���������̵߳ȴ���Ϣʱ��ɾ����Ϣ����*/
	#define KS_DEL_ALWAYS   (0x01)
#endif

/*�������̵߳�״̬*/

#ifndef KS_STATE_READY
	#define KS_STATE_READY      	(0x01)        /*�̴߳��ھ���״̬			*/
#endif
#ifndef KS_STATE_SUSPEND
	#define KS_STATE_SUSPEND    	(0x02)		  /*�̴߳��ڹ���״̬			*/
#endif
#ifndef KS_STATE_FREE
	#define KS_STATE_FREE       	(0x04)        /*�̴߳��ڿ���״̬��Ҳ�ͱ�ɾ��*/
#endif
#ifndef KS_STATE_WAIT_Q
	#define KS_STATE_WAIT_Q     	(0x08)        /*���ڵȴ���Ϣ���е�����		*/
#endif
#ifndef KS_STATE_WAIT_SEM
	#define KS_STATE_WAIT_SEM   	(0x10)        /*���ڵȴ��ź���������		*/
#endif
#ifndef KS_STATE_WAIT_MUTEX
	#define KS_STATE_WAIT_MUTEX     (0x20)        /*���ڵȴ������ź���������	*/
#endif


 
 #ifndef KS_EVENT_TYPE_Q             
	#define KS_EVENT_TYPE_Q     (0x01)         /*�¼�����--����      */
#endif
#ifndef KS_EVENT_TYPE_SEM    
    #define  KS_EVENT_TYPE_SEM  (0x02)         /*�¼�����--�ź���    */
 #endif
#ifndef KS_EVENT_TYPE_MUTEX
	#define KS_EVENT_TYPE_MUTEX (0x03)         /*�¼�����--�����ź���*/
#endif
/*�Ե�ǰ���������в�ͬ����Ķ���*/

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
*                                           ���̶���Ľṹ��
*********************************************************************************************************
*/
/*---------------------------��������ڵ�Ľṹ------------------------------*/
typedef struct KS_ENDLIST_ITEM
{
	uint32                         item_value;
	volatile struct KS_LIST_ITEM * item_next;
	volatile struct KS_LIST_ITEM * item_prev;
}ks_endlist_item;

typedef struct KS_LIST_ITEM
{
	uint32                         item_value;	         /*�����������				    */			            
	volatile struct KS_LIST_ITEM * item_next;	
	volatile struct KS_LIST_ITEM * item_prev;
	void                         * owner;				/*ָ������������ڵ��߳̿��ƿ�*/			
	void                         * container;			/* ָ������������ڵ�����     */				
}ks_list_item;
typedef struct KS_LIST
{
	volatile KS_BASE_TYPE      item_numbers;            /*����ڵ������				*/
	volatile ks_list_item    * item_index;			
	volatile ks_endlist_item   list_end;	
#if KS_RR_EN > 0
	volatile uint8             rr_flag;   
#endif                	
} ks_list;
/*---------------------------�����¼��Ľṹ��-----------------------------------*/
#if KS_EVENT_EN > 0
typedef struct KS_EVENT{
	void * pevent;      		 		 /*ָ����Ϣ���е�ָ��                   */
#if KS_SEM_EN > 0
	uint16 event_count;		     		 /*������(���¼����ź���ʱ)�����Ϊ65535*/
#endif
#if KS_MUTEX_EN > 0
	KS_BASE_TYPE pip;
	KS_BASE_TYPE mutex_caller;           /*Mutex�ĵ�����                        */
#endif
	uint8  event_type;     				 /*�¼�����                             */
	KS_BASE_TYPE  event_wait_num;        /*��Ϣ���������߳��ڵȴ���Ϣ������     */
	KS_BASE_TYPE top_eventlist_priority;
    volatile ks_list *event_wait_list;	 /*�¼��ȴ�����                         */
}ks_event;
#endif


/*---------------------------�����߳̿��ƿ�Ľṹ-----------------------------------*/
typedef struct KS_THREAD_BLOCK{
	volatile KS_STACK * top_stack;                  /*����ָ���ջջ����ָ��        */
	volatile KS_STACK * start_stack;                /*����ָ���ջ��ʼ��ַ��ָ��    */
	ks_list_item	    insertlist_item;            /*���ڽ��̲߳���������          */
	KS_BASE_TYPE        priority;                   /*�߳����ȼ�(0==���)û������   */
	uint8    			thread_state;               /*�̵߳�״̬,��0==����)         */
	uint8               item_in_delaylist;          /*�ڵ��Ƿ�����ʱ������ı�־λ  */
#if KS_THREAD_NAME_EN > 0
	uint8     thread_name[KS_THREAD_NAME_MAXLEN];   /*�����̵߳�����                */
#endif
#if KS_EVENT_EN > 0
	ks_event            *pthread_event;              /*ָ���¼����ƿ��ָ��         */
	ks_list_item         eventlist_item;             /*���ڽ��߳̿��ƿ�����¼����� */
#endif
#if KS_Q_EN > 0             
	void       *pthread_msg;                     	/*ָ�򴫵���Ϣ���е���Ϣ��ָ��  */
#endif 
	struct   KS_THREAD_BLOCK * thread_next;
	KS_TIME_TYPE    thread_delay;                   /*������ʱ��ȴ���ʱ��ʱ�ӽ�����*/
}ks_thread_block;

#if KS_Q_EN > 0
typedef struct KS_Q{
	void              **queue_start; 	/*ָ����Ϣ���е�ָ���������ʼ��ַ         */
	void              **queue_end;  	/*ָ����Ϣ���н�����Ԫ����һ����ַ         */
	void              **queue_in;    	/*ָ����Ϣ���н�����Ԫ����һ����ַָ��     */
	void              **queue_out;   	/*ָ����Ϣ��������һ��ȡ����Ϣ��λ�õ�ָ�� */
	uint16            queue_size;    	/*��Ϣ�������ܵĵ�Ԫ��,���ֵΪ65535       */
	uint16            queue_entries; 	/*��Ϣ�����е�ǰ����Ϣ�����������Ϊ65536  */
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
*                                           ���̶���
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                           ks_core.c��ͷ�ļ�
*********************************************************************************************************
*/

void                 ks_list_initialize( ks_list * newlist );

void                 ks_list_insertend( ks_list *list, ks_list_item *newitem);

void 				 ks_item_remove( ks_list_item *list_item );//ɾ������Ľڵ�

void                 ks_system_initialize(void);

KS_STACK             * ks_stack_init(THREAD_ADDR thread_addr,void *p_arg,KS_STACK *stk1);

uint8 ks_thread_init(ks_thread_block *ptr,KS_STACK *stk,KS_BASE_TYPE prio,const uint8 *const thread_name);

ks_thread_block      *ks_thread_allocate(void);//����һ������߳�

void 				 ks_schedule(void);//����

void				 ks_start(void);//ϵͳ��������

void 				 ks_time_tick(void);//���ķ����ӳ���

void                 ks_int_enter(void);//�����ж�

void                 ks_int_exit(void);
void                 ks_schedule_lock(void);  /*����������*/

void                 ks_schedule_unlock(void); /*����������*/

/*�������¼����ƿ�ĺ���*/

/*��ʼ���¼����ƿ�*/
#if KS_EVENT_EN > 0
void                 ks_event_list_init(ks_event *pevent); /*ʹ�¼��ȴ������е��߳̽����¼��ľ���̬      */
ks_thread_block     *ks_event_ready(ks_event *pevent,void *msg,uint8 msk);/*ʹ�߳̽���ȴ�ĳ�¼�����״̬ */
void                 ks_event_wait(ks_event *pevent);     /*�̵߳ȴ���ʱ�����߳���Ϊ����̬               */
void                 ks_event_to(ks_event *pevent);
#endif
/*�������ڴ����ĺ���*/
void                *ks_malloc(uint32 size);
void                 ks_free(void *pv);

/*
*********************************************************************************************************
*                                           ks_thread.c��ͷ�ļ�
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
*                                           ks_cpu.c��ͷ�ļ�
*********************************************************************************************************
*/
void                 ks_start_high(void);
void                 ks_thread_sw(void);
void                 ks_int_sw(void);
void CALLBACK ks_tick_isr(unsigned int a,unsigned int b,unsigned long c,unsigned long d,unsigned long e);


/*
*********************************************************************************************************
*                                           ks_queue.c��ͷ�ļ�
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
*                                           ks_sem.c��ͷ�ļ�
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
*                                           ks_mutex.c��ͷ�ļ�
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
*                                          ���̶����ȫ�ֱ���
*********************************************************************************************************
*/
KS_EXT ks_list readylist[KS_MAX_PRIO+1];//��������

KS_EXT    ks_list volatile delaylist[1];

KS_EXT ks_thread_block thread_table [KS_MAX_THREAD];

KS_EXT ks_thread_block *pfree_thread;//ָ������߳̿��ƿ��ָ��

KS_EXT uint8 ks_running;//ϵͳ������״̬

KS_EXT uint8 lock_nesting;      /*����������������*/

KS_EXT ks_thread_block *current_thread; //volatile

#if KS_RR_EN >0

KS_EXT ks_thread_block *next_thread;

#endif

KS_EXT ks_thread_block *high_thread;

KS_EXT uint8 int_nesting;

KS_EXT  KS_HANDLE h_idle;
KS_EXT KS_THREAD_NUM current_thread_number;//���嵱ǰ��������

KS_EXT KS_BASE_TYPE top_readylist_priority;//���������������ȼ�

KS_EXT KS_BASE_TYPE  current_running_priority;









/*
*********************************************************************************************************
*                                          ���̶���ĺ꺯��
*********************************************************************************************************
*/
#define KS_ITEM_INIT(item)  (ks_list_item *)item->container = (void *)0 

#define KS_ITEM_OWNER(item,owner1) (item)->owner = (void*)owner1















#ifdef __cplusplus
}
#endif

#endif /*�ļ�����*/
