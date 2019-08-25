#include <rthw.h>

static rt_thread_t tid1 = RT_NULL;

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       512
#define THREAD_TIMESLICE        5

/*  ��̬�ź������ƿ� */
static struct rt_semaphore static_sem; 
/*  ָ��̬�ź�����ָ�� */ 
static rt_sem_t dynamic_sem = RT_NULL;

static void thread1_entry(void* parameter) 
{ 
    rt_err_t result; 
    rt_tick_t tick; 

    /* 1. staic semaphore demo */ 
    /*  ��õ�ǰ��OS Tick */ 
    tick = rt_tick_get(); 

    /*  ��ͼ�����ź��������ȴ� 10��OS Tick�󷵻� */ 
    result = rt_sem_take(&static_sem, 10); 
    if (result == -RT_ETIMEOUT) 
    { 
        /*  ��ʱ���ж��Ƿ�պ��� 10��OS Tick */ 
        if (rt_tick_get() - tick != 10) 
        { 
            rt_sem_detach(&static_sem); 
            return; 
        } 
        rt_kprintf("take semaphore timeout\n"); 
    } 
    else 
    {  /*  ��Ϊû�������ط��ͷ��ź��������Բ�Ӧ�óɹ������ź��� */ 
        rt_kprintf("take a static semaphore, failed.\n"); 
        rt_sem_detach(&static_sem); 
        return; 
    } 
  /*  �ͷ�һ���ź��� */ 
    rt_sem_release(&static_sem); 

    /*  ���õȴ���ʽ�����ź��� */ 
    result = rt_sem_take(&static_sem, RT_WAITING_FOREVER); 
    if (result != RT_EOK) 
    { 
        /*  ���ɹ������ʧ�� */ 
        rt_kprintf("take a static semaphore, failed.\n"); 
        rt_sem_detach(&static_sem); 
        return; 
    } 

    rt_kprintf("take a staic semaphore, done.\n"); 

    /*  �����ź������� */ 
    rt_sem_detach(&static_sem);
    tick = rt_tick_get(); 

    /*  ��ͼ�����ź��������ȴ� 10��OS Tick�󷵻� */ 
    result = rt_sem_take(dynamic_sem, 10); 
    if (result == -RT_ETIMEOUT) 
    { 
        /*  ��ʱ���ж��Ƿ�պ��� 10��OS Tick */ 
        if (rt_tick_get() - tick != 10) 
        { 
            rt_sem_delete(dynamic_sem); 
            return; 
        } 
        rt_kprintf("take semaphore timeout\n"); 
    } 
    else 
    { 
        /*  ��Ϊû�������ط��ͷ��ź��������Բ�Ӧ�óɹ������ź������������ʧ��*/ 
        rt_kprintf("take a dynamic semaphore, failed.\n"); 
        rt_sem_delete(dynamic_sem); 
        return; 
        }  /*  �ͷ�һ���ź��� */ 
    rt_sem_release(dynamic_sem); 

    /*  ���õȴ���ʽ�����ź��� */ 
    result = rt_sem_take(dynamic_sem, RT_WAITING_FOREVER); 
    if (result != RT_EOK) 
    { 
        /*  ���ɹ������ʧ�� */ 
        rt_kprintf("take a dynamic semaphore, failed.\n"); 
        rt_sem_delete(dynamic_sem); 
        return; 
    } 

    rt_kprintf("take a dynamic semaphore, done.\n"); 
    /*  ɾ���ź������� */ 
    rt_sem_delete(dynamic_sem); 
} 
void test_comu_03(void)
{
	tid1 = rt_thread_create("thread1",
		thread1_entry, (void*)10,       
		THREAD_STACK_SIZE, THREAD_PRIORITY  , THREAD_TIMESLICE);
	if (tid1 != RT_NULL)
		rt_thread_startup(tid1);

	rt_err_t result;    
	/*  ��ʼ����̬�ź�������ʼֵ�� 0 */ 
	result = rt_sem_init(&static_sem, "ssem", 0, RT_IPC_FLAG_FIFO); 
	if (result != RT_EOK) 
	{ 
			rt_kprintf("init dynamic semaphore failed.\n"); 
			return ; 
	} 
	/*  ����һ����̬�ź�������ʼֵ�� 0 */ 
	dynamic_sem = rt_sem_create("dsem", 0, RT_IPC_FLAG_FIFO); 
	if (dynamic_sem == RT_NULL) 
	{ 
			rt_kprintf("create dynamic semaphore failed.\n"); 
			return ; 
	}
}
#include <finsh.h>
/* ������ msh �����б��� */
MSH_CMD_EXPORT(test_comu_03, comu test);