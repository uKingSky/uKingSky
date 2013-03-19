
#include "config.h"

#define LED1		(1<<5)		// rGPB[5] =1 ;
#define LED2		(1<<6)		// rGPB[5] =1 ;
#define LED3		(1<<7)		// rGPB[5] =1 ;
#define LED4		(1<<8)		// rGPB[5] =1 ;



U8 Key_Scan1( void )
{
	Delay( 80 ) ;
	if((rGPFDAT&(1<< 0)) == 0 )
	{
		rGPBDAT = rGPBDAT & ~(LED1);			//좋LED1

		return 4;
	}
	else if((rGPFDAT&(1<< 2)) == 0 )
	{
		rGPBDAT = rGPBDAT & ~(LED2);			//좋LED2

		return 3;
	}
	else if( (rGPGDAT&(1<< 3)) == 0 )
	{
		rGPBDAT = rGPBDAT & ~(LED3);			//좋LED3

		return 2 ;
	}
	else if( (rGPGDAT&(1<< 11)) == 0 )
	{
		rGPBDAT = rGPBDAT & ~(LED4);			//좋LED4

		return 1 ;
	}
	else
	{
		rGPBDAT = rGPBDAT & ~0x1e0|0x1e0;   //LED[8:5] => 1;

		return 0xff;
	}

}

static void  Key_ISR1(void)
{
	U8 key;
	U32 r;

	if(rINTPND==BIT_EINT8_23) {
		ClearPending(BIT_EINT8_23);
		if(rEINTPEND&(1<<11)) {
		//Uart_Printf("eint11\n");
			rEINTPEND |= 1<< 11;
		}
		if(rEINTPEND&(1<<19)) {
		//	Uart_Printf("eint19\n");		
			rEINTPEND |= 1<< 19;
		}
	}
	if(rINTPND==BIT_EINT0) {
		//Uart_Printf("eint0\n");
		ClearPending(BIT_EINT0);
	}
	if(rINTPND==BIT_EINT2) {
		//Uart_Printf("eint2\n");
		ClearPending(BIT_EINT2);
	}
	key=Key_Scan1();
	if( key == 0xff )
		Uart_Printf( "Interrupt occur... Key is released!\n") ;
	else
		Uart_Printf( "Interrupt occur... K%d is pressed!\n", key) ;

}

void KeyScan_Test1(void)
{
	Uart_Printf("\nKey Scan Test, press ESC key to exit !\n");	

	rGPGCON = rGPGCON & (~((3<<22)|(3<<6))) | ((2<<22)|(2<<6)) ;		//GPG11,3 set EINT
	rGPFCON = rGPFCON & (~((3<<4)|(3<<0))) | ((2<<4)|(2<<0)) ;		//GPF2,0 set EINT
	
	rEXTINT0 &= ~(7|(7<<8));	
	rEXTINT0 |= (0|(0<<8));	//set eint0,2 falling edge int
	rEXTINT1 &= ~(7<<12);
	rEXTINT1 |= (0<<12);	//set eint11 falling edge int
	rEXTINT2 &= ~(0xf<<12);
	rEXTINT2 |= (0<<12);	//set eint19 falling edge int

	rEINTPEND |= (1<<11)|(1<<19);		//clear eint 11,19
	rEINTMASK &= ~((1<<11)|(1<<19));	//enable eint11,19
	ClearPending(BIT_EINT0|BIT_EINT2|BIT_EINT8_23);
	pISR_EINT0 = pISR_EINT2 = pISR_EINT8_23 = (U32)Key_ISR1;
	EnableIrq(BIT_EINT0|BIT_EINT2|BIT_EINT8_23);	
}
