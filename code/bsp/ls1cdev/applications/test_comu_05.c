#include <rthw.h>

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       512
#define THREAD_TIMESLICE        5   

/* ָ�������ߺ��������߳̿��ƿ��ָ�� */
static rt_thread_t producer_tid = RT_NULL;
static rt_thread_t consumer_tid = RT_NULL;

/* �����ܹ����������Ԫ�ظ���Ϊ5 */
#define MAXSEM     5

/* ���ڷ����������������� */
rt_uint32_t array[MAXSEM];

/* ָ�������ߡ���������array�����еĶ�дλ�� */
static rt_uint32_t set, get;

struct rt_semaphore sem_lock;
struct rt_semaphore sem_empty, sem_full;

/* �������߳���� */
static void producer_thread_entry(void* parameter)
{
    rt_int32_t cnt = 0;

    /* ����100�� */
    while( cnt < 100)
    {
        /* ��ȡһ����λ */
        rt_sem_take(&sem_empty, RT_WAITING_FOREVER);

        /* �޸�array���ݣ����� */
        rt_sem_take(&sem_lock, RT_WAITING_FOREVER);
        array[set%MAXSEM] = cnt + 1;
        rt_kprintf("the producer generates a number: %d\n", 
            array[set%MAXSEM]);
        set++;
        rt_sem_release(&sem_lock);

        /* ����һ����λ */
        rt_sem_release(&sem_full);
        cnt++;

        /* ��ͣһ��ʱ�� */
        rt_thread_delay(50);
    }

    rt_kprintf("the producer exit!\n");
}

/* �������߳���� */
static void consumer_thread_entry(void* parameter)
{
    rt_uint32_t no;
    rt_uint32_t sum = 0;

    /* ��n���̣߳�����ڲ��������� */
    no = (rt_uint32_t)parameter;

    while(1)
    {
        /* ��ȡһ����λ */
        rt_sem_take(&sem_full, RT_WAITING_FOREVER);

        /* �ٽ������������в��� */
        rt_sem_take(&sem_lock, RT_WAITING_FOREVER);
        sum += array[get%MAXSEM];
        rt_kprintf("the consumer[%d] get a number:%d\n", no, array[get%MAXSEM]);
        get++;
        rt_sem_release(&sem_lock);

        /* �ͷ�һ����λ */
        rt_sem_release(&sem_empty);

        /* ������������100����Ŀʱ���������߳���Ӧֹͣ */
        if (get == 100) break;

        /* ��ͣһС��ʱ�� */
        rt_thread_delay(10);
    }

    rt_kprintf("the consumer[%d] sum is %d \n ", no, sum);
    rt_kprintf("the consumer[%d] exit!\n");
}

void test_comu_05(void)
{
	/* �����������߳� */
	producer_tid = rt_thread_create("producer",
		producer_thread_entry, /* �߳������producer_thread_entry */
		RT_NULL, /* ��ڲ�����RT_NULL */
		THREAD_STACK_SIZE, THREAD_PRIORITY - 1, THREAD_TIMESLICE);
	if (producer_tid != RT_NULL)
		rt_thread_startup(producer_tid);

	/* �����������߳� */
	consumer_tid = rt_thread_create("consumer",
	   consumer_thread_entry,/* �߳������consumer_thread_entry */
		RT_NULL, /* ��ڲ�����RT_NULL */
		THREAD_STACK_SIZE, THREAD_PRIORITY + 1, THREAD_TIMESLICE);
	if (consumer_tid != RT_NULL)
		rt_thread_startup(consumer_tid);

	/* ��ʼ��3���ź��� */
	rt_sem_init(&sem_lock , "lock",   1,    RT_IPC_FLAG_FIFO);
	rt_sem_init(&sem_empty,"empty", MAXSEM, RT_IPC_FLAG_FIFO);
	rt_sem_init(&sem_full , "full",   0,   RT_IPC_FLAG_FIFO);
}
#include <finsh.h>
/* ������ msh �����б��� */
MSH_CMD_EXPORT(test_comu_05, comu test);