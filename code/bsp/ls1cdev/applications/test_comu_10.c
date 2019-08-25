#include <rthw.h>

static rt_thread_t tid1 = RT_NULL;
static rt_thread_t tid2 = RT_NULL;

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       512
#define THREAD_TIMESLICE        5

/* ������ƿ� */
static struct rt_mailbox mb;
/* ���ڷ��ʼ����ڴ�� */
static char mb_pool[128];

static char mb_str1[] = "I'm a mail!";
static char mb_str2[] = "this is another mail!";

/* �߳�1��� */
static void thread1_entry(void* parameter)
{
    unsigned char* str;

    while (1)
    {
        rt_kprintf("thread1: try to recv a mail\n");

        /* ����������ȡ�ʼ� */
        if (rt_mb_recv(&mb, (rt_ubase_t*)&str, RT_WAITING_FOREVER)
                == RT_EOK)
        {
            /* ��ʾ�������� */
            rt_kprintf("thread1: get a mail, the content:%s\n", str);

            /* ��ʱ10��OS Tick */
            rt_thread_delay(10);
        }
    }
}

/* �߳�2��� */
static void thread2_entry(void* parameter)
{
    rt_uint8_t count;

    count = 0;
    while (1)
    {
        count ++;
         /*��������2���ַ����ĵ�ַ*/
        if (count & 0x1)
        {
            /* ����mb_str1��ַ�������� */
            rt_mb_send(&mb, (rt_uint32_t)&mb_str1[0]);
        }
        else
        {
            /* ����mb_str2��ַ�������� */
            rt_mb_send(&mb, (rt_uint32_t)&mb_str2[0]);
        }

        /* ��ʱ20��OS Tick */
        rt_thread_delay(20);
    }
}

void test_comu_10(void)
{

    /* ��ʼ��һ��mailbox */
    rt_mb_init(&mb,
        "mbt",             /* ������mbt */
        &mb_pool[0],       /* �����õ����ڴ����mb_pool */
        sizeof(mb_pool)/4, /* ��С��mb_pool/4����Ϊÿ���ʼ��Ĵ�С��4�ֽ� */
        RT_IPC_FLAG_FIFO); /* ����FIFO��ʽ�����̵߳ȴ� */

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
}
#include <finsh.h>
/* ������ msh �����б��� */
MSH_CMD_EXPORT(test_comu_10, comu test);