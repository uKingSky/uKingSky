/*
*********************************************************************************************************
*                                               KingSky
*                                          实时操作系统内核
*
*                                       (c) Copyright 2011, 庄桐泉
*                                          All Rights Reserved
*
* File         : ks_cfg.h
********************************************************************************************************* */
#ifndef  _KS_CFG_H
#define  _KS_CFG_H

#define     KS_THREAD_NAME_MAXLEN   (10)  	 /*每个线程名字的最长长度                     */
#define     KS_MAX_PRIO   		    (4)
#define		KS_MAX_THREAD           (5)   	 /*最大任务数，任务数量没有限制				  */
#define     OS_TICKS_PER_SEC        (200) 
#define 	KS_STACK_MAX_LEN        (1024)

#define     KS_RR_EN                (1)   	 /*是否支持时间片							  */
#define 	KS_TIME_PERC            (11)   	 /*时间片									  */
#define     KS_CHECK_EN             (1)      /*是否进行检验								  */

#define     KS_TICK_TYPE            (0)      /*选择时钟延时的单位，0为8位，1为16位，2为32位*/

#define     KS_THREAD_SUSPEND_EN    (0)      /*是否支持线程被挂起						  */

#define     KS_THREAD_NAME_EN       (0)

#define     KS_THREAD_RESUME_EN     (1)

#define     KS_THREAD_CHANGE_PRIO   (1)  	 /*是否支持动态改变任务优先级  				  */

#define     KS_THREAD_CLOSE_EN      (1)  	 /*是否支持关闭线程					          */

#define     KS_SCHEDULE_LOCK_EN     (1)   	 /*是否支持调度器上锁						  */

#define     KS_EVENT_EN             (1)   	 /*是否支持事件								  */

#define     KS_Q_EN                 (0)  	 /*是否支持消息队列							  */

#define     KS_SEM_EN               (1)      /*是否支持信号量							  */

#define     KS_MUTEX_EN             (1)      /*是否支持互斥信号量						  */





#endif