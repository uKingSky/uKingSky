/*
*********************************************************************************************************
*                                               KingSky
*                                          ʵʱ����ϵͳ�ں�
*
*                                       (c) Copyright 2011, ׯͩȪ
*                                          All Rights Reserved
*
* File         : ks_cfg.h
********************************************************************************************************* */
#ifndef  _KS_CFG_H
#define  _KS_CFG_H

#define     KS_THREAD_NAME_MAXLEN   (10)  	 /*ÿ���߳����ֵ������                     */
#define     KS_MAX_PRIO   		    (4)
#define		KS_MAX_THREAD           (5)   	 /*�������������������û������				  */
#define     OS_TICKS_PER_SEC        (200) 
#define 	KS_STACK_MAX_LEN        (1024)

#define     KS_RR_EN                (1)   	 /*�Ƿ�֧��ʱ��Ƭ							  */
#define 	KS_TIME_PERC            (11)   	 /*ʱ��Ƭ									  */
#define     KS_CHECK_EN             (1)      /*�Ƿ���м���								  */

#define     KS_TICK_TYPE            (0)      /*ѡ��ʱ����ʱ�ĵ�λ��0Ϊ8λ��1Ϊ16λ��2Ϊ32λ*/

#define     KS_THREAD_SUSPEND_EN    (0)      /*�Ƿ�֧���̱߳�����						  */

#define     KS_THREAD_NAME_EN       (0)

#define     KS_THREAD_RESUME_EN     (1)

#define     KS_THREAD_CHANGE_PRIO   (1)  	 /*�Ƿ�֧�ֶ�̬�ı��������ȼ�  				  */

#define     KS_THREAD_CLOSE_EN      (1)  	 /*�Ƿ�֧�ֹر��߳�					          */

#define     KS_SCHEDULE_LOCK_EN     (1)   	 /*�Ƿ�֧�ֵ���������						  */

#define     KS_EVENT_EN             (1)   	 /*�Ƿ�֧���¼�								  */

#define     KS_Q_EN                 (0)  	 /*�Ƿ�֧����Ϣ����							  */

#define     KS_SEM_EN               (1)      /*�Ƿ�֧���ź���							  */

#define     KS_MUTEX_EN             (1)      /*�Ƿ�֧�ֻ����ź���						  */





#endif