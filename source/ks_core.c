
/*
*********************************************************************************************************
*                                               KingSky
*                                          ʵʱ����ϵͳ�ں�
*
*                                       (c) Copyright 2011, ׯͩȪ
*                                          All Rights Reserved
*
* File         : ks_core.c
*˵�����ں˲���ϵͳ�ĺ����ļ�
********************************************************************************************************* */

#ifndef  KS_MASTER_FILE
#define  KS_GLOBALS
#include "\KingSky_vc6\KingSky\includes\KingSky.h"
#endif
/*��̬��������*/
static void ks_readylist_init(void);
static void ks_threadtable_init(void);
static void ks_idle_init(void);
static void ks_variable_init(void);
/*��������̵߳�ջ�ռ�,����ΪKS_STACK*/
KS_STACK thread_stack_idle[KS_STACK_MAX_LEN];

/*
 *********************************************************************************************************
 * 							����һ������߳�(ks_thread_allocate)
 *
 * ����: ����һ������߳̿�
 *
 * ���ã�ks_item_init(����ڵ��ʼ��)��(ks_thread_block*)0(����ʧ��)
 *
 * ����: ��
 *
 * ����: ptr(����Ŀ����߳̿�ָ��)��
 *
 * ע��: �ڴ����̵߳�ʱ����øú��������ݶ����readylist�������ж��Ƿ��п����߳̿�
 *********************************************************************************************************
 */
 ks_thread_block *ks_thread_allocate(void)     /*����һ������߳�              */
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif

	ks_thread_block *ptr;
	KS_ENTER_CRITICAL();
	ptr = pfree_thread;              		 	/*ptrָ������߳̿��ƿ��ͷָ�� */
	if(ptr !=(ks_thread_block*)0)     			/*�ж��Ƿ��п��е��߳̿鱻ʹ��	*/
	{
	 	pfree_thread = ptr->thread_next;  		/*���У������pfree_threadָ��	*/
		KS_EXIT_CRITICAL();
		return (ptr);                     		/*�������뵽��ָ��				*/
	}
	KS_EXIT_CRITICAL();
	return (ks_thread_block*)0;                 /*���޷��ɹ����򷵻�ʧ�ܱ�־    */
}

static void ks_item_init(ks_list_item *item)    /*˫������ڵ�ĳ�ʼ��			*/
{
	item->container = (void*)0;
}

/*
 *********************************************************************************************************
 * 							�̳߳�ʼ��(ks_thread_init)
 *
 * ����: ���߳̿��ƿ�����ݽ��г�ʼ��
 *
 * ���ã�ks_item_init(����ڵ��ʼ��)
 *
 * ����:*ptr(�߳̿��ƿ�)��*stk(ջ����ʼ��ַ)��prio(���ȼ�)��thread_name(�̵߳�����)
 *
 * ����: KS_NOERR_THREAD_INIT���̳߳�ʼ������ KS_ERR_THREAD_INIT���̳߳�ʼ������
 *
 * ע��:�ڴ����̵߳�ʱ����øú���
 *********************************************************************************************************
 */
uint8 ks_thread_init(ks_thread_block *ptr,KS_STACK *stk,KS_BASE_TYPE prio,const uint8 *const thread_name)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
	
	if(ptr != (ks_thread_block*)0)              /*�ɹ�����һ������߳�       */
	{
		KS_ENTER_CRITICAL();
#if KS_THREAD_NAME_EN > 0					    /*��ʼ���߳̿��ƿ������     */
		if(thread_name ==(const uint8 *const)0) /*���thread_nameΪ��		 */
		{
			strncpy(( uint8 *) ptr->thread_name, "", ( uint8 ) KS_THREAD_NAME_MAXLEN );
		}
		else									/*���thread_name��Ϊ��		 */
		{
			strncpy(( uint8 *) ptr->thread_name, ( const uint8 * ) thread_name, ( uint8 ) KS_THREAD_NAME_MAXLEN );
		}
		ptr->thread_name[( uint8 )KS_THREAD_NAME_MAXLEN -( uint8 )1] = '\0';
#endif
		ptr->top_stack    = stk;                   		   
		ptr->priority     =(KS_BASE_TYPE)prio;  /*���߳̿��ƿ�����ݽ��г�ʼ��*/
		ptr->thread_state = KS_STATE_READY;
		ptr->item_in_delaylist = 0;

#if KS_RR_EN >0
		ptr->insertlist_item.item_value = KS_TIME_PERC;    /*ʱ��Ƭ��ֵ					 */
#endif
		ks_item_init(&(ptr->insertlist_item));             /*��ʼ���߳�����              */
		KS_ITEM_OWNER(&(ptr->insertlist_item),ptr);        /*�ڵ�ָ���Ǹ��߳̿�          */
#if KS_EVENT_EN > 0
		ks_item_init(&(ptr->eventlist_item));              /*��ʼ�������¼�����Ľڵ�	 */
		KS_ITEM_OWNER(&(ptr->eventlist_item),ptr);         /*���ýڵ�ָ���ĸ��߳̿�      */
#endif
		KS_EXIT_CRITICAL();
		return (KS_NOERR_THREAD_INIT);
	}
	
	return (KS_ERR_THREAD_INIT);
}
/*
 *********************************************************************************************************
 * 							���������ʼ��(ks_list_initialize)
 *
 * ����: ��һ��������г�ʼ��,����ͷ�ڵ�list_end,item_indexָ��ͷ�ڵ� 
 *
 * ���ã���
 *
 * ����:list(Ҫ��ʼ���ĸ�����)
 *
 * ����: ��
 *
 * ע��: 
 *********************************************************************************************************
 */
void ks_list_initialize( ks_list * newlist )                        
{
	newlist->item_index = ( ks_list_item * ) &( newlist->list_end );/*item_indexָ�������ͷ���  */
	newlist->list_end.item_value = 0;								/*��ʱ��ʱ����                */
	newlist->list_end.item_next = ( ks_list_item * )&( newlist->list_end);
	newlist->list_end.item_prev = ( ks_list_item * ) &( newlist->list_end);
	newlist->item_numbers = 0;
#if KS_RR_EN >0
	newlist->rr_flag = 0;                                           /*ʱ��Ƭ��־λ����	          */
#endif
}
/*
 *********************************************************************************************************
 * 							����ڵ����(ks_list_insertend)
 *
 * ����: ����ڵ����
 *
 * ���ã���
 *
 * ����:list(���ĸ��������)��list_item(Ҫ���������ڵ�)
 *
 * ����: ��
 *
 * ע��: ��Ҫ���������ͽڵ���Ϊ�������룬�磺ks_list_insertend((ks_list*)&readylist[prio],(ks_list_item*)&ktb->insertlist_item);
 *********************************************************************************************************
 */
void ks_list_insertend( ks_list *list, ks_list_item *newitem)
{
	volatile ks_list_item *  item_index;
	item_index = list->item_index;                 /*item_indexָ������������list��ͷ�ڵ�item_index*/
	newitem->item_next = item_index->item_next;
	newitem->item_prev = list->item_index;
	item_index->item_next->item_prev = ( volatile ks_list_item * ) newitem;
	item_index->item_next = ( volatile ks_list_item * ) newitem;
	list->item_index = ( volatile ks_list_item * ) newitem;
	
	newitem->container = ( void * ) list;
	( list->item_numbers )++;                       /*����Ľڵ�������                              */
}
/*
 *********************************************************************************************************
 * 							����ڵ�ɾ��(ks_item_remove)
 *
 * ����: ����ڵ�ɾ��
 *
 * ���ã���
 *
 * ����:list_item(Ҫɾ��������ڵ�)
 *
 * ����: ��
 *
 * ע��: ��Ҫɾ���Ľڵ���Ϊ�������룬�磺ks_item_remove((ks_list_item*)&ktb->insertlist_item);
 *********************************************************************************************************
 */
void ks_item_remove( ks_list_item *list_item )
{
	ks_list * list;
	
	list_item->item_next->item_prev = list_item->item_prev;
	list_item->item_prev->item_next = list_item->item_next;
	list = ( ks_list * ) list_item->container;
	
	if( list->item_index == list_item )
	{
		list->item_index = list_item->item_prev;
	}
	
	list_item->container = (void*)0;
	( list->item_numbers )--;
}
/*
 *********************************************************************************************************
 * 							���������ʼ��(ks_readylist_init)
 *
 * ����: ���������ʼ��
 *
 * ���ã���
 *
 * ����: ��
 *
 * ����: ��
 *
 * ע��: ��ÿ�����ȼ����г�ʼ����Ҳ����Ϊÿ�����ȼ�����һ��˫��ѭ������
 *********************************************************************************************************
 */
static void ks_readylist_init(void)
{
#if KS_MAX_PRIO < 256                  /*�����ں�������������ȼ�����i���ж���ͳ�ʼ��                */
	uint8 i = 0;
#elif KS_MAX_PRIO <65536
	uint16 i =0;
#else
	uint32 i = 0;
#endif
	for (i = 0;i < KS_MAX_PRIO+1;i++ ) /*��ÿ�����ȼ����г�ʼ����Ҳ����Ϊÿ�����ȼ�����һ��˫��ѭ������*/
	{
		ks_list_initialize((ks_list *)&readylist[i]);
	}
}
/*
 *********************************************************************************************************
 * 							�߳̿������ʼ��(ks_threadtable_init)
 *
 * ����: ������ľ������������ӳɵ�������
 *
 * ���ã���
 *
 * ����: ��
 *
 * ����: ��
 *
 * ע��: ÿ�δ����߳��£��ӵ���������ժȡһ����pfree_threadָ�������ͷ��㴦
 *********************************************************************************************************
 */
static void ks_threadtable_init(void)
{
#if KS_MAX_PRIO < 256
	uint8 i = 0;
#elif KS_MAX_PRIO <65536
	uint16 i =0;
#else
	uint32 i = 0;
#endif
	ks_thread_block *ptcb1;
	ks_thread_block *ptcb2;
	ptcb1 = &thread_table[0];
	ptcb2 = &thread_table[1];
	for (i =0;i<KS_MAX_PRIO;i++)            /*������ľ����������ӳɵ�������*/
	{
		ptcb1->thread_next = ptcb2;
		ptcb1 ++;
		ptcb2 ++;
	}
	ptcb1->thread_next = (ks_thread_block*)0;
	pfree_thread = &thread_table[0];
}
/*
 *********************************************************************************************************
 * 							ϵͳ��ʼ��ʱ������ʼ��(ks_variable_init)
 *
 * ����: ����ϵͳ�����ã��ֱ�Բ�ͬ�������г�ʼ����
 *
 * ���ã���
 *
 * ����: ��
 *
 * ����: ��
 *
 * ע��: KS_MAX_PRIO��ks_cfg.h�ж���,�ú���Ϊ��̬�����������ڱ�Դ�ļ����á�
 *********************************************************************************************************
 */
static void ks_variable_init(void)
{
	
#if KS_MAX_PRIO < 256							/*�������Ϊ8λ����				*/
	top_readylist_priority = 0xff;          
	current_running_priority = 0xff;
#elif KS_MAX_PRIO < 65536						/*�������Ϊ16λ����			*/
	top_readylist_priority = 0xffff;
	current_running_priority = 0xffff;
#else
	top_readylist_priority = 0xffffffff;		/*�������Ϊ32λ����			*/
	current_running_priority = 0xffffffff;
#endif
	ks_running = KS_FALSE;
	int_nesting = 0;
	lock_nesting = 0;
	current_thread_number = 0;
#if KS_RR_EN > 0								/*�Ƿ�ʹ��ʱ��Ƭ��				*/
	next_thread = (ks_thread_block*)0; 
#endif
}
/*
 *********************************************************************************************************
 * 							  ���������߳�(ks_idle_init)
 *
 * ����: ���������̣߳���ϵͳû���������ȼ���������ʱ��ϵͳ�����������������̡߳�
 *
 * ���ã�ks_thread_create���̴߳�����������ks_thread_idle�������̺߳�����
 *
 * ����: ��
 *
 * ����: ��
 *
 * ע��: KS_STK_GROWTH == 1����ʾջ��������ʽ�Ǵ�������������KS_STK_GROWTH == 0��
 		 ��ʾջ��������ʽ�Ǵ������� ����������KS_STK_GROWTH�Ĳ�ֵͬ�����������̡߳�
 		
 *********************************************************************************************************
 */
static void ks_idle_init(void)
{
#if KS_STK_GROWTH == 1
	h_idle = ks_thread_create(ks_thread_idle,(uint8*)"idle",(void*)0,&thread_stack_idle[KS_STACK_MAX_LEN-1],KS_MAX_PRIO);
#elif KS_STK_GROWTH == 0
	h_idle = ks_thread_create(ks_thread_idle,(uint8*)"idle",(void*)0,&thread_stack_idle[0],KS_MAX_PRIO);
#endif
}
/*
 *********************************************************************************************************
 * 									ϵͳ��ʼ��(ks_system_initialize)
 *
 * ����: ϵͳ����ǰ�ĳ�ʼ��������ȫ�ֱ�����ʼ�������������ʼ������ʱ�����ʼ�����߳̿������ʼ��
 *
 * ���ã�ks_variable_init��������ʼ������ks_readylist_init�����������ʼ��)��ks_threadtable_init(�߳̿������ʼ��)
 *       ks_list_initialize(��ʱ�����ʼ��)��ks_idle_init(���������߳�)
 *
 * ����: ��
 *
 * ����: ��
 *
 * ע��: KS_STK_GROWTH == 1����ʾջ��������ʽ�Ǵ�������������KS_STK_GROWTH == 0��
 *		 ��ʾջ��������ʽ�Ǵ������� ����������KS_STK_GROWTH�Ĳ�ֵͬ�����������̡߳�
 *		
 *********************************************************************************************************
 */
void ks_system_initialize(void)
{
	
	ks_variable_init();							  		/*��һЩȫ�ֱ������г�ʼ��*/
	ks_readylist_init();						  		/*�Ծ���������г�ʼ��    */
	ks_threadtable_init();
	ks_list_initialize((ks_list *)&delaylist[0]); 		/*����ʱ������г�ʼ��    */
	ks_idle_init();
}

void ks_schedule(void)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
	KS_BASE_TYPE top = 0;
	KS_ENTER_CRITICAL();
	/*�������ĵ��Ƚ�ֹ�ģ���������������ǰ�����жϷ����ӳ����̵߳ĵ����ǲ�����ġ�*/
	if(int_nesting == 0&& lock_nesting == 0)                     
	{
		for (top = top_readylist_priority;top < (KS_BASE_TYPE)(KS_MAX_PRIO+(KS_BASE_TYPE)1);++top)
		{
			if(readylist[top].item_numbers != 0)/*��������*/
			{
				break;
			}
		}/*���ҵ��������������ȼ�����top*/
		
	
		top_readylist_priority = top;                                        /*����top_readylist_priority��ֵ			   */
		if(current_running_priority != top_readylist_priority) 				 /*����������������ȼ����ǵ�ǰ�̵߳����ȼ�  */
		{
			current_running_priority = top_readylist_priority; 				 /*���µ�ǰ���е����ȼ���ֵ                    */
			high_thread = (ks_thread_block*)readylist[top].item_index->owner;/*����ǰָ��ָ�򼴽����е�������ȼ����߳�    */
			high_thread->item_in_delaylist = 0;								 /*�������е��߳̿��ھ��������������ʱ������*/
			KS_EXIT_CRITICAL();
			KS_THREAD_SW();													 /*�����̼߳���ĵ���                          */
		}
#if KS_RR_EN >0																 /*�Ƿ�ʹ��ʱ��Ƭ							   */
		else 
		{
			if((readylist[current_running_priority].item_numbers) >= (KS_BASE_TYPE)1)
			{
				if(next_thread == (ks_thread_block*)0)                       /*ȷ��next_thread��Ϊ��					   */
				{
					KS_EXIT_CRITICAL();
					return;
				}
				high_thread = next_thread;									/*�������е��߳̿�ָ��ָ��next_thread		   */
				KS_THREAD_SW();                                             /*�̼߳���ĵ���							   */
			}
		}
#endif		
		/*������ҳ���������ȼ�������������̬�����񣬾Ͳ����е��ȣ�ֱ�����С�*/
	}

	KS_EXIT_CRITICAL();
}

void ks_start(void)
{
	if(ks_running == KS_FALSE)             /*�ж�ϵͳ�Ƿ��Ѿ�����      */
	{
		
		current_thread = high_thread;
		ks_start_high();
		/*���溯��ʵ��ks_running = KS_TRUE;��������ϵͳ����״̬��־ΪTRUE��
		  ����׼��������������ȼ���������������Ҫ�û����д��         */
	}
}


void ks_time_tick(void)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
     KS_BASE_TYPE i,num,prio = 0;
	 volatile  ks_list_item * volatile p;					/*����ָ������ڵ��ָ��		*/
	 ks_thread_block *ktb;									/*����ָ���߳̿��ƿ��ָ��		*/
	 num = delaylist[0].item_numbers;                   	/*��ʱ����ڵ������		    */
	 
	 p = delaylist[0].item_index->item_next->item_next; 	/*pָ������ĵ�һ�������ݵĽڵ� */
	 
	 for(i =0; i < num; i++)                                /*�ӵ�һ���߳̿��ƿ鿪ʼ����	*/
	{
		KS_ENTER_CRITICAL();     							/*�����ٽ���					*/                      

		ktb = (ks_thread_block*)(p->owner);					/*ָ��ktbָ����������߳̿��ƿ�	*/			
		if(ktb->thread_delay != 0)
		{
			if(--(ktb->thread_delay) == 0)					/*��ʱ�ĵδ����ݼ�				*/
			{
				if(!(ktb->thread_state & KS_STATE_SUSPEND)) /*�����߳��Ƿ񱻹���			*/
				{
					p = p->item_prev;
					prio = ktb->priority;
					ks_item_remove((ks_list_item*)&ktb->insertlist_item); /*��ktb��ָ����߳̿�Ӿ���������ɾ�� */
					ks_list_insertend((ks_list*)&readylist[prio],(ks_list_item*)&ktb->insertlist_item);/*��ktb��ָ����߳̿���뵽��ʱ������*/
					ktb->item_in_delaylist = 1;				 			  /*��ʾktb��ָ����߳̿�����ʱ������   */
					if (prio < top_readylist_priority)
					{
						top_readylist_priority = prio;					  /*����top_readylist_priority��ֵ      */
					}
				}
				 else													  /*�����̱߳������״̬				*/
				{
						ktb->thread_delay = 1;                            /*��ʱ����1�����ֹ���״̬				*/
				}
				
			}
		}
		p = p->item_next;												 /*ָ��pָ����һ���߳̿��ƿ�			*/
		KS_EXIT_CRITICAL();												 /*�˳��ٽ���							*/
	}
	 
#if KS_RR_EN > 0														 /*�Ƿ�ʹ��ʱ��Ƭת						*/
	KS_ENTER_CRITICAL();												 /*�����ٽ���							*/
	
	if(readylist[current_thread->priority].rr_flag == (uint8)1)			 /*��ǰ�߳̿����������Ƿ���rr_flag(ʱ��Ƭת)�ı��*/
	{
		if(--(current_thread->insertlist_item.item_value) ==0)           /*�Ե�ǰ���߳̿��ʱ��Ƭ�ݼ�					  */
		{
			if (readylist[current_thread->priority].item_numbers > (KS_BASE_TYPE)1)
			{
				if(current_thread->insertlist_item.item_next->item_value == 0)/*�ж�current_thread�Ľڵ����������Ƿ�Ϊ���һ���ڵ�*/
				{
					high_thread =(ks_thread_block*)current_thread->insertlist_item.item_next->item_next->owner;/*high_threadָ����һ���ڵ�����Ӧ���߳̿��ƿ�*/
				}
				else														 /*high_threadָ����һ���ڵ�����Ӧ���߳̿��ƿ�		*/
				{
					high_thread =(ks_thread_block*)current_thread->insertlist_item.item_next->owner;
				}
			}
			current_thread->insertlist_item.item_value = KS_TIME_PERC;       /*�����߳̿��ƿ�current_thread��ʱ��ƬΪKS_TIME_PERC*/
		}
	}

	KS_EXIT_CRITICAL();														 /*�˳��ٽ���						*/
#endif	
	
}

void ks_int_enter(void)//�����ж�
{
	if(ks_running == KS_TRUE)		  /*�ж�ϵͳ�Ƿ�����				  */
	{
		if(int_nesting < 255)         /*�ж��ж�Ƕ��int_nesting�Ƿ����255*/
		{
			int_nesting ++;           /*�����ж�Ƕ��int_nesting����		  */
		}
	}
}
extern void ks_int_sw(void);

void ks_int_exit(void)                /*�ж��뿪*/
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
	KS_BASE_TYPE top = 0;

	KS_ENTER_CRITICAL();
	if(ks_running == KS_TRUE)
	{
		if(int_nesting > 0) 
		{
			int_nesting --;							/*�ж�ǰ�����ݼ�				   */
		}
		if(int_nesting == 0 && lock_nesting == 0)   /*�ж��ж�Ƕ���Ƿ�Ϊ0���Լ��Ƿ���*/
		{
													/*�����������ȼ�				   */                                     
			for (top = top_readylist_priority;top < (KS_BASE_TYPE)(KS_MAX_PRIO+(KS_BASE_TYPE)1);++top)
			{
				if(readylist[top].item_numbers != 0)/*���ҵ��������������ȼ�����top  */
				{
					break;
				}
			}										
				top_readylist_priority = top;
		
			if(current_running_priority != top_readylist_priority)   /*����������������ȼ����ǵ�ǰ�̵߳����ȼ�  			*/
			{
				current_running_priority = top_readylist_priority;   /*���µ�ǰ���е����ȼ���ֵ                    			*/
				high_thread = (ks_thread_block*)readylist[top].item_index->owner;/*����ǰָ��ָ�򼴽����е�������ȼ�������	*/
				high_thread->item_in_delaylist = 0;					 /*�������е��߳̿��ھ��������������ʱ������			*/
				ks_int_sw(); 										 /*�����жϼ���ĵ���						   			*/
		
				
			}

			else if(current_running_priority == top_readylist_priority)
			{
				if(readylist[current_thread->priority].item_numbers > 1)
				{
					ks_int_sw();									/*�����жϼ���ĵ���									*/
				}
			}
		}
	}	
	KS_EXIT_CRITICAL(); /*�˳��ٽ���	*/
}

#if KS_SCHEDULE_LOCK_EN > 0

void ks_schedule_lock(void)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
	if(ks_running == KS_TRUE)  /*ȷ��ϵͳ��������״̬								*/
	{
		KS_ENTER_CRITICAL();   /*�����ٽ���											*/
		if(lock_nesting < 255) /*����Ƕ�׼������Ƿ�<255��ϵͳ���֧��255��Ƕ�׼�����*/
		{
			lock_nesting++;    /*Ƕ�׼�������1										*/
		}
		KS_EXIT_CRITICAL();    /*�˳��ٽ���											*/                                                                                                                                                                                                                                                                                                                                    
	}
}

void ks_schedule_unlock(void)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
	if(ks_running == KS_TRUE)      /*ȷ��ϵͳ��������״̬				*/
	{
		KS_ENTER_CRITICAL();
		if(lock_nesting > 0)
		{
			lock_nesting--;        /*Ƕ�׼�������1						*/
			if(lock_nesting==0 && int_nesting == 0)/*ȷ�������߲���ISR	*/
			{
				KS_EXIT_CRITICAL();
				ks_schedule();     /*���е���							*/
			}
			else
			{
				KS_EXIT_CRITICAL();/*�˳��ٽ���							*/
			}
		}
		else
		{
			KS_EXIT_CRITICAL();   /*�˳��ٽ���							*/
		}
	}
}

#endif

#if KS_EVENT_EN > 0

 void ks_event_list_init(ks_event *pevent)
{
#if KS_MAX_PRIO < 256      /*���ݲ�ͬ�����ȼ��������岻ͬ��i������op_eventlist_priority���г�ʼ��*/
	uint8 i = 0;
	pevent->top_eventlist_priority = 0xff;
#elif KS_MAX_PRIO <65536
	uint16 i =0;
	pevent->top_eventlist_priority = 0xffff;
#else
	uint32 i = 0;
	pevent->top_eventlist_priority = 0xffffffff;
#endif
	pevent->event_wait_num = 0;       /*�ȴ���Ϣ���߳�����,0=û���߳��ڵȴ���Ϣ*/
	/*��ÿ�����ȼ���������г�ʼ��*/
	pevent->event_wait_list = (ks_list*)ks_malloc((KS_MAX_PRIO+1)*sizeof(ks_list));
	for (i = 0;i < KS_MAX_PRIO+1;i++ )
	{
		ks_list_initialize((ks_list *)&pevent->event_wait_list[i]);
	}
	
}

/*ʹ�¼��ȴ������е��߳̽����̵߳ľ���̬*/
ks_thread_block * ks_event_ready(ks_event *pevent,void *msg,uint8 msk)
{
	KS_BASE_TYPE top = 0;
	ks_thread_block *high_event_thread;
	for (top = pevent->top_eventlist_priority;top < (KS_BASE_TYPE)(KS_MAX_PRIO+(KS_BASE_TYPE)1);++top)
	{
		if(pevent->event_wait_list[top].item_numbers != 0)
		{
				break;
		}
	}
	/*���ҵ��ȴ������е�������ȼ�����top*/
	pevent->top_eventlist_priority = top;
    /*֧��ʱ����ת��*/
#if KS_RR_EN > 0
	if(pevent->event_wait_list[top].item_numbers > 1)/*�ж��ͬ�����ȼ�������£�����ѡ���ȵ�����߳̿�*/
	{
		high_event_thread =(ks_thread_block*)pevent->event_wait_list[top].item_index->item_next->item_next->owner;
	}
	else
#endif
	{
		high_event_thread =(ks_thread_block*)pevent->event_wait_list[top].item_index->owner;
	}
	
	ks_item_remove((ks_list_item*)&high_event_thread->insertlist_item);/*���¼��ȴ������е�������ȼ��߳�ɾ��*/
	ks_item_remove((ks_list_item*)&high_event_thread->eventlist_item);
	
	high_event_thread->thread_delay = 0;		    /*ֱ�ӽ��ñ�������						  */ 								
	high_event_thread->pthread_event = (ks_event*)0;/*�߳̿��ƿ���ָ���¼����ƿ��ָ����ΪNULL*/
#if KS_Q_EN > 0
 /*���ks_event_ready()������Ϣ���з��ͺ������ã�
 ��ô�ú�����Ҫ����Ӧ����Ϣ���ݸ��ȴ�������������ȼ����߳̿飬���ҷ�������������ƿ���*/
	high_event_thread->pthread_msg = msg;
#else
	msg = msg;								  /*��ֹ���������뾯��							  */
#endif
	high_event_thread->thread_state &= ~msk;  /*λ������msk��Ϊ�������ݸ���,�ָ��߳�ԭ�ȵ�״̬*/
	
	if(high_event_thread->thread_state == KS_STATE_READY)/*���̲߳��뵽�߳̾�������			  */
	{
		ks_list_insertend((ks_list*)&readylist[top],(ks_list_item*)&high_event_thread->insertlist_item);
	}
	else												/*���̲߳��뵽�߳���ʱ������          */	
	{
		ks_list_insertend((ks_list*)&delaylist[0],(ks_list_item*)&high_event_thread->insertlist_item);
	}
	pevent->event_wait_num--; 							/*�ȴ���Ϣ���߳�����1				  */
	return (high_event_thread);
}
/*ʹ�߳̽���ȴ�ĳ�¼�����״̬*/
void ks_event_wait(ks_event *pevent)
{
	current_thread->pthread_event = pevent; /*���¼����ƿ��ָ��ŵ��߳̿��ƿ���*/
#if KS_RR_EN >0
		if(readylist[current_running_priority].item_numbers > (KS_BASE_TYPE)1)
		{
			if(current_thread->insertlist_item.item_next->item_value == 0)
			{
				next_thread =(ks_thread_block*)current_thread->insertlist_item.item_next->item_next->owner;
			}
			else if(current_thread->insertlist_item.item_next->item_value > 0)
			{
				next_thread =(ks_thread_block*)current_thread->insertlist_item.item_next->owner;
			}
		}
#endif
	ks_item_remove((ks_list_item*)&current_thread->insertlist_item);/*���̴߳��߳̾�������ɾ��*/
	ks_list_insertend((ks_list*)&delaylist[0],(ks_list_item*)&current_thread->insertlist_item);
										/*���̷߳ŵ��¼��ȴ�������				*/
	 ks_list_insertend((ks_list*)&pevent->event_wait_list[current_thread->priority],(ks_list_item*)&current_thread->eventlist_item);
	pevent->event_wait_num ++;          /*�ȴ���Ϣ���߳�����1					*/
	if(current_thread->priority < pevent->top_eventlist_priority)
	{									/*����pevent->top_eventlist_priority��ֵ*/
		pevent->top_eventlist_priority = current_thread->priority;
	}
}
/*�̵߳ȴ���ʱ�����߳���Ϊ����̬*/
void ks_event_to(ks_event *pevent)
{
	ks_item_remove((ks_list_item*)&current_thread->eventlist_item);   /*���̴߳��¼��ȴ�������ɾ��  */
																	  /*���̲߳��뵽�߳̾���������	*/
	current_thread->thread_state = KS_STATE_READY; 					  /*�̵߳�״̬��־Ϊ����̬		*/
	current_thread->pthread_event = (ks_event*)0; 					  /*ɾ��ָ���¼����ƿ��ָ��    */
	pevent->event_wait_num --;      								  /*�ȴ���Ϣ���߳�����1     	*/
}
#endif

/*�ڴ����뺯��*/
void *ks_malloc(uint32 size)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
	void *preturn;
	KS_ENTER_CRITICAL();            /*�����ٽ���			*/
	preturn = malloc(size);         /*�����ڴ�				*/
	KS_EXIT_CRITICAL();             /*�뿪�ٽ���			*/
	return preturn;                 /*����������ڴ�ָ���ַ*/
}

void ks_free(void *pv)
{
#if KS_CRITICAL_METHOD == 3
	KS_CPU_SR cpu_sr;
#endif
	if(pv)
	{
		KS_ENTER_CRITICAL();            /*�����ٽ���	*/
		free(pv);                       /*�ͷ��ڴ�		*/
		KS_EXIT_CRITICAL();             /*�뿪�ٽ���	*/
	}
}




