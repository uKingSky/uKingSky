/*
*********************************************************************************************************
*                                               KingSky
*                                          实时操作系统内核
*
*                                       (c) Copyright 2011, 庄桐泉
*                                          All Rights Reserved
*
* File         : error.h
********************************************************************************************************* */
#ifndef _ERROR_H
#define _ERROR_H

/*以下是线程控制块对应的错误信息		*/
#define KS_ERR_THREAD_INIT          (0x00)
#define KS_NOERR_THREAD_INIT		(0x01)
#define KS_PRIO_ERROR               (0x02)
#define KS_PRIO_NO_ERROR            (0x03)
#define KS_THREAD_SUSPEND_IDLE      (0x04)
#define KS_THREAD_SUSPEND_ERROR     (0x05)
#define KS_NO_ERROR                 (0x06)
#define KS_THREAD_SUSPEND_AGAIN     (0x07)
#define KS_THREAD_RESUME_ERROR      (0x08)
#define KS_PRIO_CHANGE_ERROR        (0x09)
#define KS_THREAD_DEL_ISR           (0x0a)
#define KS_THREAD_DEL_IDLE          (0x0b)
#define KS_THREAD_DEL_ERROR         (0x0c)

/*以下是消息队列对应的错误信息			*/
#define KS_QUEUE_CREATE_ERROR       (0x00)	 /*创建消息队列时出错         	 */
#define KS_QUEUE_WAIT_ISR           (0x0d)   /*在ISR调用ks_queue_waitfor	 */
#define KS_QUEUE_EVENT_NULL         (0x0e)   /*不存在消息队列事件			 */
#define KS_QUEUE_EVENT_TYPE         (0x0f)	 /*队列类型错误					 */
#define KS_QUEUE_TIMEOUT            (0x10)	 /*等待消息队列超时错误			 */
#define KS_QUEUE_FULL               (0x11)	 /*消息队列已经满				 */
#define KS_QUEUE_EMPTY              (0x12)	 /*消息队列为空					 */
#define KS_QUEUE_DEL_ISR            (0x13)	 /*在ISR中删除队列				 */
#define KS_DEL_THREAD_WAITING       (0x14)	 /*删除队列时有线程在等待		 */
#define KS_ERROR_INVALID_OPT        (0x15)	 /*删除队列时，opt参数无效		 */

/*以下是信号量对应的错误信息			*/
#define KS_SEM_CREATE_ERROR         (0x00)    /*创建信号量时出错         	 */
#define KS_SEM_WAIT_ISR             (0x17) 	  /*ISR调用ks_sem_waitfor   	 */
#define KS_SEM_EVENT_NULL           (0x18)    /*指针不是指向事件控制块的	 */
#define KS_SEM_EVENT_TYPE           (0x19)	  /*信号量类型错误				 */
#define KS_SEM_OVF                  (0x1a)    /*信号量值溢出				 */
#define KS_SEM_DEL_ISR              (0x1b)	  /*在ISR中删除信号量			 */

/*以下是互斥信号量对应的错误信息		*/
#define KS_MUTEX_CREATE_ERROR       (0x00)	  /*创建互斥信号量时出错         */
#define KS_EVENT_TYPE_NULL          (0x1c)    /*事件的类型不存在  			 */
#define KS_MUTEX_WAIT_ISR           (0x1d)    /*ISR调用ks_mutex_waitfor		 */
#define KS_MUTEX_TIMEOUT            (0x1e)	  /*等待消息信号量超时错误		 */
#define KS_MUTEX_TYPE_ERROR         (0x1f)	  /*事件类型不是信号量类型		 */
#define KS_ERR_NOT_MUTEX_OWNER      (0x20)    /*释放者不是Mutex的占有者    	 */

#endif