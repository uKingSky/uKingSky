
#define	GLOBAL_CLK		1
#include "config.h"

ks_event *mutex;
KS_STACK  main_thread_stack[KS_STACK_MAX_LEN];
KS_STACK  thread1_stack[KS_STACK_MAX_LEN];
KS_STACK  thread2_stack[KS_STACK_MAX_LEN];
KS_STACK  thread3_stack[KS_STACK_MAX_LEN];

static void ks_thread1(void *param);
static void ks_thread2(void *param);
static void ks_thread3(void *param);

KS_HANDLE h_thread1,h_thread2,h_thread3;

void ks_thread_main(void *param)
{
	
#if KS_CRITICAL_METHOD == 3		/* Allocate storage for CPU status register */
		KS_CPU_SR  cpu_sr;
#endif
	param = param;
	KS_ENTER_CRITICAL();
	Timer0Init();				//initial timer0 for ucos time tick
	ISRInit();				    //initial interrupt prio or enable or disable
	//KeyScan_Test1();

	KS_EXIT_CRITICAL();
	
	h_thread1 = ks_thread_create(ks_thread1,"thread1",(void*)0,&thread1_stack[KS_STACK_MAX_LEN-1],1);
	h_thread2 = ks_thread_create(ks_thread2,"thread2",(void*)0,&thread2_stack[KS_STACK_MAX_LEN-1],2);
	h_thread3 = ks_thread_create(ks_thread3,"thread3",(void*)0,&thread3_stack[KS_STACK_MAX_LEN-1],3);
	for(;;)
	{
		Uart_SendString("main\n");
		ks_thread_delay(100);
	}
}

uint8 count = 0;
static void ks_thread1(void *param)
{
	uint8 err;
	uint8 *msg;
	param = param;
	
	for(;;)
	{
	   if(count == 1)
	   {
	   		ks_mutex_waitfor(mutex,200,&err);
			Uart_Printf("err = %d\n",err);
			count++;
	   }
		Uart_SendString(" 1\n");
    	ks_thread_delay(100);
	}
}

static void ks_thread2(void *param)
{
	uint8 err;
	param = param;
	ks_mutex_waitfor(mutex,200,&err);
	Uart_Printf("err = %d\n",err);
	ks_mutex_sendmsg(mutex);
	count++;
	for(;;)
	{
	   
		Uart_SendString(" 2\n");
		ks_thread_delay(100);
	}
}
static void ks_thread3(void *param)
{
	uint8 err;
	
	param = param;
	for(;;)
	{
		Uart_SendString(" 3\n");
		ks_thread_delay(100);
	}
}


void Main(void)
{
	 KS_HANDLE h_main;
	 uint8 err;
  	 TargetInit();                              /*ARM板子初始化*/
	 ks_system_initialize();                    /*系统初始化*/
	 h_main = ks_thread_create(ks_thread_main,NULL,(void*)0,&main_thread_stack[KS_STACK_MAX_LEN-1],0);
	 mutex = ks_mutex_create(1,&err);
	 if(h_main == (KS_HANDLE)KS_PRIO_ERROR)
	 {	
	 	return;//失败处理
	 }
	 ks_start();	 

}


