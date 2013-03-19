/*
*********************************************************************************************************
*                                               KingSky
*                                          ʵʱ����ϵͳ�ں�
*
*                                       (c) Copyright 2011, ׯͩȪ
*                                          All Rights Reserved
*
* File         : error.h
********************************************************************************************************* */
#ifndef _ERROR_H
#define _ERROR_H

/*�������߳̿��ƿ��Ӧ�Ĵ�����Ϣ		*/
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

/*��������Ϣ���ж�Ӧ�Ĵ�����Ϣ			*/
#define KS_QUEUE_CREATE_ERROR       (0x00)	 /*������Ϣ����ʱ����         	 */
#define KS_QUEUE_WAIT_ISR           (0x0d)   /*��ISR����ks_queue_waitfor	 */
#define KS_QUEUE_EVENT_NULL         (0x0e)   /*��������Ϣ�����¼�			 */
#define KS_QUEUE_EVENT_TYPE         (0x0f)	 /*�������ʹ���					 */
#define KS_QUEUE_TIMEOUT            (0x10)	 /*�ȴ���Ϣ���г�ʱ����			 */
#define KS_QUEUE_FULL               (0x11)	 /*��Ϣ�����Ѿ���				 */
#define KS_QUEUE_EMPTY              (0x12)	 /*��Ϣ����Ϊ��					 */
#define KS_QUEUE_DEL_ISR            (0x13)	 /*��ISR��ɾ������				 */
#define KS_DEL_THREAD_WAITING       (0x14)	 /*ɾ������ʱ���߳��ڵȴ�		 */
#define KS_ERROR_INVALID_OPT        (0x15)	 /*ɾ������ʱ��opt������Ч		 */

/*�������ź�����Ӧ�Ĵ�����Ϣ			*/
#define KS_SEM_CREATE_ERROR         (0x00)    /*�����ź���ʱ����         	 */
#define KS_SEM_WAIT_ISR             (0x17) 	  /*ISR����ks_sem_waitfor   	 */
#define KS_SEM_EVENT_NULL           (0x18)    /*ָ�벻��ָ���¼����ƿ��	 */
#define KS_SEM_EVENT_TYPE           (0x19)	  /*�ź������ʹ���				 */
#define KS_SEM_OVF                  (0x1a)    /*�ź���ֵ���				 */
#define KS_SEM_DEL_ISR              (0x1b)	  /*��ISR��ɾ���ź���			 */

/*�����ǻ����ź�����Ӧ�Ĵ�����Ϣ		*/
#define KS_MUTEX_CREATE_ERROR       (0x00)	  /*���������ź���ʱ����         */
#define KS_EVENT_TYPE_NULL          (0x1c)    /*�¼������Ͳ�����  			 */
#define KS_MUTEX_WAIT_ISR           (0x1d)    /*ISR����ks_mutex_waitfor		 */
#define KS_MUTEX_TIMEOUT            (0x1e)	  /*�ȴ���Ϣ�ź�����ʱ����		 */
#define KS_MUTEX_TYPE_ERROR         (0x1f)	  /*�¼����Ͳ����ź�������		 */
#define KS_ERR_NOT_MUTEX_OWNER      (0x20)    /*�ͷ��߲���Mutex��ռ����    	 */

#endif