#include <rthw.h>

static rt_thread_t tid1 = RT_NULL;
static rt_thread_t tid2 = RT_NULL;
static rt_thread_t tid3 = RT_NULL;

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       512
#define THREAD_TIMESLICE        5

/* ��Ϣ���п��ƿ� */
static struct rt_messagequeue mq;
/* ��Ϣ�������õ��ķ�����Ϣ���ڴ�� */
static char msg_pool[2048];

/* �߳�1��� */
static void thread1_entry(void* parameter)
{
    char buf[128];
    while (1)
    {
        rt_memset(&buf[0], 0, sizeof(buf));
        /* ����Ϣ�����н�����Ϣ */
        if (rt_mq_recv(&mq, &buf[0], sizeof(buf), RT_WAITING_FOREVER)
                == RT_EOK)
        {
            rt_kprintf("thread1: recv a msg, the content:%s\n", buf);
        }
        rt_thread_delay(10);
    }
}

/* �߳�2��� */
static void thread2_entry(void* parameter)
{
    int i, result;
    char buf[] = "this is message No.x";
    while (1)
    {
        for (i = 0; i < 10; i++)
        {
            buf[sizeof(buf) - 2] = '0' + i;
            rt_kprintf("thread2: send message - %s\n", buf);
            /* ������Ϣ����Ϣ������ */
            result = rt_mq_send(&mq, &buf[0], sizeof(buf));
            if ( result == -RT_EFULL)
            {
                rt_kprintf("message queue full, delay 1s\n");
                rt_thread_delay(100);
            }
        }
        rt_thread_delay(10);
    }
}

/* �߳�3��� */
static void thread3_entry(void* parameter)
{
    char buf[] = "this is an urgent message!";
    while (1)
    {
        rt_kprintf("thread3: send an urgent message\n");
        /* ���ͽ�����Ϣ����Ϣ������ */
        rt_mq_urgent(&mq, &buf[0], sizeof(buf));
        rt_thread_delay(25);
    }
}

void test_comu_11(void)
{
	/* ��ʼ����Ϣ���� */
	rt_mq_init(&mq, "mqt", 
		&msg_pool[0], /* �ڴ��ָ��msg_pool */
		128 - sizeof(void*), /* ÿ����Ϣ�Ĵ�С�� 128 - void* */
		sizeof(msg_pool),  /* �ڴ�صĴ�С��msg_pool�Ĵ�С */
		RT_IPC_FLAG_FIFO); /* ����ж���̵߳ȴ�������FIFO�ķ���������Ϣ */

	tid1 = rt_thread_create("thread1",
			thread1_entry, RT_NULL,       
			THREAD_STACK_SIZE, THREAD_PRIORITY  , THREAD_TIMESLICE);
	if (tid1 != RT_NULL)
			rt_thread_startup(tid1);

	tid2 = rt_thread_create("thread2",
			thread2_entry,   RT_NULL,       
			THREAD_STACK_SIZE, THREAD_PRIORITY , THREAD_TIMESLICE);
	if (tid2 != RT_NULL)
			rt_thread_startup(tid2);

	tid3 = rt_thread_create("thread3",
			thread3_entry,   RT_NULL,       
			THREAD_STACK_SIZE, THREAD_PRIORITY , THREAD_TIMESLICE);
	if (tid3 != RT_NULL)
			rt_thread_startup(tid3);
}
#include <finsh.h>
/* ������ msh �����б��� */
MSH_CMD_EXPORT(test_comu_11, comu test);