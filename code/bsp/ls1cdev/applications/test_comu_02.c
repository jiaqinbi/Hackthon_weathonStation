#include <rthw.h>

static rt_thread_t tid1 = RT_NULL;
static rt_thread_t tid2 = RT_NULL;

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       512
#define THREAD_TIMESLICE        5

static rt_uint32_t cnt;
static void thread1_entry(void* parameter)
{
        rt_uint32_t no;
        rt_uint32_t level;

        no = (rt_uint32_t) parameter;
        while(1)
        {
                /* �ر��ж� */
                level = rt_hw_interrupt_disable();
                cnt += no;
                /* �ָ��ж� */
                rt_hw_interrupt_enable(level);

                rt_kprintf("thread[%d]'s counter is %d\n", no, cnt);
                rt_thread_delay(no);
        }
}

void test_comu_02(void)
{
	tid1 = rt_thread_create("thread1",
	    thread1_entry, (void*)10,       
	    THREAD_STACK_SIZE, THREAD_PRIORITY - 1 , THREAD_TIMESLICE);
	if (tid1 != RT_NULL)
	    rt_thread_startup(tid1);
	
	tid2 = rt_thread_create("thread2",
	    thread1_entry,   (void*)20,       
	    THREAD_STACK_SIZE, THREAD_PRIORITY , THREAD_TIMESLICE);
	if (tid2 != RT_NULL)
	    rt_thread_startup(tid2);
}
#include <finsh.h>
/* ������ msh �����б��� */
MSH_CMD_EXPORT(test_comu_02, comu test);