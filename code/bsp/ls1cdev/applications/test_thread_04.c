#include <rtthread.h>

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       512
#define THREAD_TIMESLICE        5

/*  ��̬�߳�1 �Ķ��������ʱ�õ���ջ */  
static struct rt_thread thread1;
static rt_uint8_t thread1_stack[THREAD_STACK_SIZE]; 

/*  ��̬�߳�2 �Ķ��� */  
static rt_thread_t thread2 = RT_NULL;

/*  �߳� 1���  */  
static void thread1_entry (void* parameter) 
{ 
    int  count = 0; 

    while (1) 
    { 
				rt_kprintf( "%d\n" , ++count);

				/*  ��ʱ100 ��OS Tick */  
				rt_thread_delay(100); 
    } 
} 

/*  �߳� 2���  */  
static void thread2_entry (void* parameter) 
{ 
    int  count = 0; 
    while (1) 
    { 
        rt_kprintf( "Thread2 count:%d\n" , ++count); 

        /*  ��ʱ50��OS Tick */  
        rt_thread_delay(50); 
    } 
}

void test_thread_04(void)
{
	rt_err_t result; 

	/*  ��ʼ���߳�1 */ 
	/*  �̵߳������thread1_entry ��������RT_NULL 
	 *  �߳�ջ��thread1_stack ջ�ռ���512 ��  
	 *  ���ȼ���25 ��ʱ��Ƭ��5��OS Tick 
	 */  
	result = rt_thread_init(&thread1, "thread1", 
			thread1_entry, RT_NULL, 
			&thread1_stack[0], sizeof(thread1_stack), 
			THREAD_PRIORITY, THREAD_TIMESLICE); 

	/*  �����߳�1 */  
	if (result == RT_EOK) rt_thread_startup(&thread1); 

	/*  �����߳�2 */ 
	/*  �̵߳������thread2_entry,  ������RT_NULL 
	 *  ջ�ռ���512 �����ȼ���25 ��ʱ��Ƭ��5��OS Tick 
	 */  
	thread2 = rt_thread_create( "thread2", thread2_entry, RT_NULL, 
			THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE); 

	/*  �����߳�2 */  
	if (thread2 != RT_NULL) rt_thread_startup(thread2);
}
#include <finsh.h>
/* ������ msh �����б��� */
MSH_CMD_EXPORT(test_thread_04, thread test);