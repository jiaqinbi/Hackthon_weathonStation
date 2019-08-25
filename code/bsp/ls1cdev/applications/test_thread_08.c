#include <rtthread.h>

static rt_thread_t tid1 = RT_NULL;
static rt_thread_t tid2 = RT_NULL;

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       512
#define THREAD_TIMESLICE        5
/* �߳�1��� */
static void thread1_entry(void* parameter)
{
    rt_uint32_t count = 0;
    int i=0;

    for(i = 0 ; i < 10 ; i++)
    {
        /* ִ��yield��Ӧ���л���thread2ִ��*/
        rt_thread_yield();
        /* ��ӡ�߳�1�����*/
        rt_kprintf("thread1: count = %d\n", count ++);
    }
}
/* �߳�2��� */
 static void thread2_entry(void* parameter)
 {
        rt_uint32_t count = 0;
    int i=0;

    for(i = 0 ; i < 10 ; i++)
    {
        /* ��ӡ�߳�2�����*/
        rt_kprintf("thread2: count = %d\n", count ++);
        /* ִ��yield��Ӧ���л���thread1ִ��*/
        rt_thread_yield();
    }
}

void test_thread_08(void)
{
	tid1 = rt_thread_create("thread1",
		thread1_entry, RT_NULL, 
		THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
	if (tid1 != RT_NULL)
		rt_thread_startup(tid1);

	tid2 = rt_thread_create("thread2",
		thread2_entry, RT_NULL,
		THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
	if (tid2 != RT_NULL)
		rt_thread_startup(tid2);

}

#include <finsh.h>
/* ������ msh �����б��� */
MSH_CMD_EXPORT(test_thread_08, thread test);