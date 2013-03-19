

#ifndef  CONFIG_H
#define  CONFIG_H

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL  		0
#endif

#ifndef OK
#define OK		1
#endif

#ifndef FAIL
#define FAIL		0
#endif

#define U32 unsigned int
#define U16 unsigned short
#define S32 int
#define S16 short int
#define U8  unsigned char
#define	S8  char

/*********************【uC/OS-II的特殊代码】******************/
#define		USER_USING_MODE    0x10             // 用户模式,ARM代码
                                                // 只能是0x10,0x30,0x1f,0x3f之一

#include "KingSky.h"

/**********************【ARM的特殊代码】******************/
#include	"S3C2440slib.h" 
#include	"S3C2440addr.h"
#include	"option.h"
#include	"S3C2440lib.h"

void TargetInit(void);

// IRQ中断向量地址表
//extern  uint32 VICVectAddr[];


/**********************【应用程序配置】******************/
#include	<stdio.h>
#include	<ctype.h>
#include	<stdlib.h>
#include	<stdarg.h>
#include	<string.h>

/******************【其它及自定义头文件】*****************/
#include	"lcd.h" 
#include	"TIMER.H"
#include	"mmu.h"

/*******************【用户自定义变量】*******************/
#define RGB(r,g,b)		(unsigned int)( (r << 16) + (g << 8) + b )
#define FROM_BCD(n)		((((n) >> 4) * 10) + ((n) & 0xf))
#define TO_BCD(n)		((((n) / 10) << 4) | ((n) % 10))


#endif
