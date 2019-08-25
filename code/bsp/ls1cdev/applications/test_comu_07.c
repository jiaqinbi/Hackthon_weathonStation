#include <rthw.h>

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       512
#define THREAD_TIMESLICE        5

static rt_thread_t tid1 = RT_NULL;
static rt_thread_t tid2 = RT_NULL;
static rt_thread_t tid3 = RT_NULL;
static rt_mutex_t mutex = RT_NULL;

/* �߳�1��� */
static void thread1_entry(void* parameter)
{
        /* ���õ����ȼ��߳����� */
        rt_thread_delay(10);

        /* ��ʱthread3����mutex������thread2�ȴ�����mutex */

        /* ���thread2��thread3�����ȼ���� */
        if (tid2->current_priority != tid3->current_priority)
        {
            /* ���ȼ�����ͬ������ʧ�� */
            rt_kprintf("\r\ntest1 failed��\r\n");
            return;
        }
}

/* �߳�2��� */
static void thread2_entry(void* parameter)
{
        rt_err_t result;

        /* ���õ����ȼ��߳����� */
        rt_thread_delay(5);

        while (1)
        {
                /*
                 * ��ͼ���л���������ʱthread3���У�Ӧ��thread3�����ȼ�����
                 * ��thread2��ͬ�����ȼ�
                 */
                result = rt_mutex_take(mutex, RT_WAITING_FOREVER);

                if (result == RT_EOK)
                {
                        /* �ͷŻ����� */
                        rt_mutex_release(mutex);
                }
        }
}

/* �߳�3��� */
static void thread3_entry(void* parameter)
{
        rt_tick_t tick;
        rt_err_t result;

        while (1)
        {
                result = rt_mutex_take(mutex, RT_WAITING_FOREVER);
                result = rt_mutex_take(mutex, RT_WAITING_FOREVER);
                if (result != RT_EOK)
                {
                    rt_kprintf("\r\ntest3 failed��\r\n");
                    return ;
                }

                /* ��һ����ʱ���ѭ�����ܹ�50��OS Tick */
                tick = rt_tick_get();
                while (rt_tick_get() - tick < 50) ;

                rt_mutex_release(mutex);
                rt_mutex_release(mutex);
        }
}

void test_comu_07(void)
{
    /* ���������� */
    mutex = rt_mutex_create("mutex", RT_IPC_FLAG_FIFO);
    if (mutex == RT_NULL)
    {
      rt_kprintf("\r\ntest0 failed��\r\n");  
			return ;
    }

    tid1 = rt_thread_create("t1",
        thread1_entry, RT_NULL, 
        THREAD_STACK_SIZE, THREAD_PRIORITY - 1, THREAD_TIMESLICE);
    if (tid1 != RT_NULL)
        rt_thread_startup(tid1);

    tid2 = rt_thread_create("t2",
        thread2_entry, RT_NULL, 
        THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
    if (tid2 != RT_NULL)
        rt_thread_startup(tid2);

    tid3 = rt_thread_create("t3",
        thread3_entry,  RT_NULL, 
        THREAD_STACK_SIZE, THREAD_PRIORITY + 1, THREAD_TIMESLICE);
    if (tid3 != RT_NULL)
        rt_thread_startup(tid3);
}
#include <finsh.h>
/* ������ msh �����б��� */
MSH_CMD_EXPORT(test_comu_07, comu test);