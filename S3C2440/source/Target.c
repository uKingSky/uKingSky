/**************************************************
**	�ļ���:Target.c
**	�汾��:V 1.0
**	�ļ�˵��:��ʼ��S3C2440������ϵͳ����ʱ�ӣ���ʼ���˿ڣ�MMU�����ڵ�
***************************************************/

#include "config.h"
#include <rt_heap.h>	

void TargetInit(void)
{
	int i;
	U8 key;
	U32 mpll_val=0;

    
	i = 2 ;	//use 400M!
		
	switch ( i ) {
	case 0:	//200
		key = 12;
		mpll_val = (92<<12)|(4<<4)|(1);
		break;
	case 1:	//300
		key = 14;
		mpll_val = (67<<12)|(1<<4)|(1);
		break;
	case 2:	//400
		key = 14;
		mpll_val = (92<<12)|(1<<4)|(1);
		break;
	case 3:	//440!!!
		key = 14;
		mpll_val = (102<<12)|(1<<4)|(1);
		break;
	default:
		key = 14;
		mpll_val = (92<<12)|(1<<4)|(1);
		break;
	}
	
	//init FCLK=400M, so change MPLL first
	ChangeMPllValue((mpll_val>>12)&0xff, (mpll_val>>4)&0x3f, mpll_val&3);
	ChangeClockDivider(key, 12);    

    
	MMU_DisableICache();
	MMU_DisableDCache();
 	Port_Init();
	MMU_Init();
	_init_alloc(0x17fe000, 0x17ff000);/*����ѵ���ʼ��ַ�ͽ�����ַ��4k�Ŀռ�*/
	Delay(0);
	Uart_Init(0,115200);
	Uart_Select(0);
	Uart_SendString("Board init complete.\n");
}