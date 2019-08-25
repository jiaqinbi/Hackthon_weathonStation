#include <rtthread.h>

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       512
#define THREAD_TIMESLICE        5

static rt_thread_t tid1 = RT_NULL;
static rt_thread_t tid2 = RT_NULL;

/* �߳���� */
static void thread_entry(void* parameter)
{
    rt_uint32_t count = 0;
    rt_uint32_t no = (rt_uint32_t) parameter; /* ����̵߳���ڲ��� */

    while (1)
    {
        /* ��ӡ�̼߳���ֵ��� */
        rt_kprintf("thread%d count: %d\n", no, count ++);

        /* ����10��OS Tick */
        rt_thread_delay(10);
    }
}

void test_thread_06(void)
{
	tid1 = rt_thread_create("thread1",
		thread_entry, (void*)1, /* �߳������thread_entry, ��ڲ�����1 */
		THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
	if (tid1 != RT_NULL)
		rt_thread_startup(tid1);
	else
		return ;

	tid2 = rt_thread_create("thread2",
		thread_entry, (void*)2, /* �߳������thread_entry, ��ڲ�����2 */
		THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
	if (tid2 != RT_NULL)
		rt_thread_startup(tid2);
	else
		return ;
}

#include <finsh.h>
/* ������ msh �����б��� */
MSH_CMD_EXPORT(test_thread_06, thread test);