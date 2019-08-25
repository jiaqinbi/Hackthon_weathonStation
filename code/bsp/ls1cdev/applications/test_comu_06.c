#include <rthw.h>

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       1024
#define THREAD_TIMESLICE        5

#define N 5 /* ������ѧ�ҵ���Ŀ 5 */
#define LEFT_PHD(i) ((i+N-1)%N) /* ��ѧ�� i ��ߵ���ѧ�� */
#define RIGHT_PHD(i) ((i+1)%N) /* ��ѧ�� i �ұߵ���ѧ�� */
#define LEFT_PHD(i) ((i+N-1)%N) /* ��ѧ�� i ��ߵ���ѧ�� */
#define RIGHT_PHD(i) ((i+1)%N) /* ��ѧ�� i �ұߵ���ѧ�� */

struct rt_semaphore sem[N]; /* ÿλ��ѧ��һ���ź��� */
struct rt_semaphore sem_lock; /* �����ֵ�ź���ʵ���ٽ������� */
enum _phd_state 
{ /* ����ʹ��ö�����ͱ�ʾ��ѧ��״̬*/
    THINKING = 0,
    HUNGRY,
    EATING,
} phd_state[N]; /* ������ѧ��״̬���� */
const char * status_string[N] =
{
    "thinking",
    "hungry",
    "eating",
};

static void test(int i)
{
    if (phd_state[i] == HUNGRY &&
    phd_state[LEFT_PHD(i)] != EATING &&
    phd_state[RIGHT_PHD(i)] != EATING)
    {
        phd_state[i] = EATING;
        /* ���Եõ����ӣ��ʷ����ź��� */
        rt_sem_release(&sem[i]);
    }
}

static void take_forks(int i)
{
    /* �����ٽ���*/
    rt_sem_take(&sem_lock, RT_WAITING_FOREVER);
    phd_state[i] = HUNGRY;
    test(i);
    /* �˳��ٽ���*/
    rt_sem_release(&sem_lock);
    /* ��������� EATING ״̬��������ѧ�� */
    rt_sem_take(&sem[i], RT_WAITING_FOREVER);
}

static void put_forks(int i)
{
    /* �����ٽ���*/
    rt_sem_take(&sem_lock, RT_WAITING_FOREVER);
    phd_state[i] = THINKING;
    test(LEFT_PHD(i));
    test(RIGHT_PHD(i));
    /* �˳��ٽ���*/
    rt_sem_release(&sem_lock);
}

/* ��ѧ���߳� */
static void phd_thread_entry(void* parameter)
{
    int i;
    i = (int)parameter;
    rt_kprintf("phd %i starts...\n", i);
    while(1)
    {
        /* thinking */
        rt_thread_delay(RT_TICK_PER_SECOND);
        rt_kprintf("phd %d is %s\n", i, status_string[phd_state[i]]);
        /* take forks */
        take_forks(i);
        /* eating */
        rt_kprintf("phd %d is %s\n", i, status_string[phd_state[i]]);
        rt_thread_delay(RT_TICK_PER_SECOND*2);
        /* put forks */
        put_forks(i);
    }
}

void test_comu_06(void)
{
	int i;
	rt_thread_t tid;
	rt_err_t result;
	/* ��ʼ���ź��� */
	result = rt_sem_init(&sem_lock , "lock", 1, RT_IPC_FLAG_FIFO);
	if (result != RT_EOK)
	return ;
	for (i=0; i<5; i++)
	{
		result = rt_sem_init(&sem[i] , "sem", 0, RT_IPC_FLAG_FIFO);
		if (result != RT_EOK)
		return ;
	}

	for (i=0; i<5; i++)
	{
		tid = rt_thread_create(
		"phd",
		phd_thread_entry,
		(void *)i,
		THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE*3);
		if(tid != RT_NULL)
			rt_thread_startup(tid);
	}
}
#include <finsh.h>
/* ������ msh �����б��� */
MSH_CMD_EXPORT(test_comu_06, comu test);