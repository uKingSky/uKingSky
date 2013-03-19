/*
*********************************************************************************************************
*                                               KingSky
*                                          实时操作系统内核
*
*                                       (c) Copyright 2011, 庄桐泉
*                                          All Rights Reserved
*
* File         : ks_cpu.h
********************************************************************************************************* */
#ifndef _KS_CPU_H
#define _KS_CPU_H

/*
*********************************************************************************************************
*                                            CPU 数据类型
									（不同CPU应该定义不同数据类型)
*                                         
**********************************************************************************************************/

typedef unsigned char   uint8;                  /* 无符号8位整型变量                        */
typedef signed   char   int8;                   /* 有符号8位整型变量                        */
typedef unsigned short  uint16;                 /* 无符号16位整型变量                       */
typedef signed   short  int16;                  /* 有符号16位整型变量                       */
typedef unsigned int    uint32;                 /* 无符号32位整型变量                       */
typedef signed   int    int32;                  /* 有符号32位整型变量                       */
typedef float           fp32;                   /* 单精度浮点数（32位长度）                 */
typedef double          fp64;                   /* 双精度浮点数（64位长度）                 */
typedef uint32          KS_STACK;               /*定义堆栈32位宽                            */

typedef uint32          KS_CPU_SR;               /*定义CPU状态字的宽度为32位                */
extern  uint8 flag ;		

#define  KS_CRITICAL_METHOD    (1)


#if      KS_CRITICAL_METHOD == 1
#define  KS_ENTER_CRITICAL()  flag=0                   /* 禁止定时器 调度                      */
#define  KS_EXIT_CRITICAL()   flag=1				//容许定时器调度                /* Enable  interrupts                        */
#endif

#if      KS_CRITICAL_METHOD == 3
#define  KS_ENTER_CRITICAL()  (cpu_sr = ks_cpu_save())   /* 中断禁止                        */
#define  KS_EXIT_CRITICAL()   (ks_cpu_restore(cpu_sr))   /* 中断打开                        */
#endif

#define  KS_STK_GROWTH        (1)                        /*堆栈增长方向                      */

extern void ks_thread_sw(void);

#define  KS_THREAD_SW()        ks_thread_sw()                          


#if KS_CRITICAL_METHOD == 3                          
KS_CPU_SR  ks_cpu_save(void);
void       ks_cpu_restore(KS_CPU_SR cpu_sr);
#endif





#endif