#include <rtthread.h>

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       512
#define THREAD_TIMESLICE        5

static struct rt_thread thread1;
static rt_uint8_t thread1_stack[THREAD_STACK_SIZE];
static struct rt_thread thread2;
static rt_uint8_t thread2_stack[THREAD_STACK_SIZE];
/* �߳�1��� */
static void thread1_entry(void* parameter)
{
	rt_uint32_t count = 0;

	while (1)
	{
		/* �߳�1���õ����ȼ����У�һֱ��ӡ����ֵ */
		rt_kprintf("thread count: %d\n", count ++);
		rt_thread_delay(RT_TICK_PER_SECOND);
	}
}
/* �߳�2��� */	
static void thread2_entry(void* parameter)
{
	/* �߳�2ӵ�нϸߵ����ȼ�������ռ�߳�1�����ִ�� */	
	/* �߳�2��������˯��10��OS Tick */
	rt_thread_delay(RT_TICK_PER_SECOND*10);

	/*�߳�2���Ѻ�ֱ��ִ���߳�1���룬�߳�1���Ӿ����̶߳�����ɾ��*/
	rt_thread_detach(&thread1);

	/*�߳�2��������10��OS TickȻ���˳�*/
	rt_thread_delay(10);	
}

void test_thread_07(void)
{
	rt_err_t result; 
	result = rt_thread_init(&thread1, "thread1", 
			thread1_entry, RT_NULL, 
			&thread1_stack[0], sizeof(thread1_stack), 
			THREAD_PRIORITY + 1, THREAD_TIMESLICE);
	if (result == RT_EOK) 
			rt_thread_startup(&thread1);

	result = rt_thread_init(&thread2, "thread2", 
			thread2_entry, RT_NULL, 
			&thread2_stack[0], sizeof(thread2_stack), 
			THREAD_PRIORITY, THREAD_TIMESLICE);
	if (result == RT_EOK) 
			rt_thread_startup(&thread2);

}

#include <finsh.h>
/* ������ msh �����б��� */
MSH_CMD_EXPORT(test_thread_07, thread test);