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

        for(;count < 10; )
        {
                rt_kprintf("thread count: %d\n", count ++);
                rt_thread_delay(RT_TICK_PER_SECOND);
              /* countΪ2ʱ�������� */
                if(count == 2) 
                {
                    rt_kprintf("thread1 suspend\n"); /* �������� */
                    rt_thread_suspend(tid1);                        
                    rt_schedule();                  /* ����ִ���̵߳��� */
                    rt_kprintf("thread1 resumed\n");                    
                }
        }
}

/* �߳�2��� */
static void thread2_entry(void* parameter)
{
        rt_thread_delay(RT_TICK_PER_SECOND*5);

        /* �����߳�1 */
        rt_thread_resume(tid1);

        rt_thread_delay(10);
}

void test_thread_11(void)
{
	tid1 = rt_thread_create("thread1",
		thread1_entry, RT_NULL,       
		THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
	if (tid1 != RT_NULL)
		rt_thread_startup(tid1);

	tid2 = rt_thread_create("thread2",
		thread2_entry,   RT_NULL,       
		THREAD_STACK_SIZE, THREAD_PRIORITY-1, THREAD_TIMESLICE);
	if (tid2 != RT_NULL)
		rt_thread_startup(tid2);
}

#include <finsh.h>
/* ������ msh �����б��� */
MSH_CMD_EXPORT(test_thread_11, thread test);