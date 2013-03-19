/*
*********************************************************************************************************
*                                               KingSky
*                                          ʵʱ����ϵͳ�ں�
*
*                                       (c) Copyright 2011, ׯͩȪ
*                                          All Rights Reserved
*
* File         : ks_cpu.h
********************************************************************************************************* */
#ifndef _KS_CPU_H
#define _KS_CPU_H

/*
*********************************************************************************************************
*                                            CPU ��������
									����ͬCPUӦ�ö��岻ͬ��������)
*                                         
**********************************************************************************************************/

typedef unsigned char   uint8;                  /* �޷���8λ���ͱ���                        */
typedef signed   char   int8;                   /* �з���8λ���ͱ���                        */
typedef unsigned short  uint16;                 /* �޷���16λ���ͱ���                       */
typedef signed   short  int16;                  /* �з���16λ���ͱ���                       */
typedef unsigned int    uint32;                 /* �޷���32λ���ͱ���                       */
typedef signed   int    int32;                  /* �з���32λ���ͱ���                       */
typedef float           fp32;                   /* �����ȸ�������32λ���ȣ�                 */
typedef double          fp64;                   /* ˫���ȸ�������64λ���ȣ�                 */
typedef uint32          KS_STACK;               /*�����ջ32λ��                            */

typedef uint32          KS_CPU_SR;               /*����CPU״̬�ֵĿ��Ϊ32λ                */
extern  uint8 flag ;		

#define  KS_CRITICAL_METHOD    (1)


#if      KS_CRITICAL_METHOD == 1
#define  KS_ENTER_CRITICAL()  flag=0                   /* ��ֹ��ʱ�� ����                      */
#define  KS_EXIT_CRITICAL()   flag=1				//����ʱ������                /* Enable  interrupts                        */
#endif

#if      KS_CRITICAL_METHOD == 3
#define  KS_ENTER_CRITICAL()  (cpu_sr = ks_cpu_save())   /* �жϽ�ֹ                        */
#define  KS_EXIT_CRITICAL()   (ks_cpu_restore(cpu_sr))   /* �жϴ�                        */
#endif

#define  KS_STK_GROWTH        (1)                        /*��ջ��������                      */

extern void ks_thread_sw(void);

#define  KS_THREAD_SW()        ks_thread_sw()                          


#if KS_CRITICAL_METHOD == 3                          
KS_CPU_SR  ks_cpu_save(void);
void       ks_cpu_restore(KS_CPU_SR cpu_sr);
#endif





#endif