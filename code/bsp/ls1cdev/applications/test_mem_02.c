#include <rthw.h>

static rt_thread_t tid1 = RT_NULL;

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       512
#define THREAD_TIMESLICE        5

/* �߳�1���*/
static void thread1_entry(void* parameter)
{
    int i;
    char *ptr[20]; /* ���ڷ���20�������ڴ���ָ��*/

    /* ��ָ������*/
    for (i = 0; i < 20; i ++) ptr[i] = RT_NULL;

    while(1)
    {
        for (i = 0; i < 20; i++)
        {
            /* ÿ�η���(1 << i)��С�ֽ������ڴ�ռ�*/
            ptr[i] = rt_malloc(1 << i);

            /* �������ɹ�*/
            if (ptr[i] != RT_NULL)
            {
                rt_kprintf("get memory: 0x%x\n", ptr[i]);
                /* �ͷ��ڴ��*/
                rt_free(ptr[i]);
                ptr[i] = RT_NULL;
            }
        }
    }
}

void test_mem_02(void)
{
	tid1 = rt_thread_create("thread1",
			thread1_entry, RT_NULL,       
			THREAD_STACK_SIZE, THREAD_PRIORITY  , THREAD_TIMESLICE);
	if (tid1 != RT_NULL)
			rt_thread_startup(tid1);
}
#include <finsh.h>
/* ������ msh �����б��� */
MSH_CMD_EXPORT(test_mem_02, comu test);