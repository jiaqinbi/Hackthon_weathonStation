#include <rtthread.h>

/* ��ʱ���Ŀ��ƿ� */
static struct rt_timer timer1;
static struct rt_timer timer2;

/* ��ʱ��1��ʱ���� */
static void timeout1(void* parameter)
{
    rt_kprintf("periodic timer is timeout\n");
}

/* ��ʱ��2��ʱ���� */
static void timeout2(void* parameter)
{
    rt_kprintf("one shot timer is timeout\n");
}

void test_timer_02(void)
{

    /* ��ʼ����ʱ�� */
    rt_timer_init(&timer1, "timer1",  /* ��ʱ�������� timer1 */
                    timeout1, /* ��ʱʱ�ص��Ĵ����� */
                    RT_NULL, /* ��ʱ��������ڲ��� */
                    100, /* ��ʱ���ȣ���OS TickΪ��λ����100��OS Tick */
                    RT_TIMER_FLAG_PERIODIC); /* �����Զ�ʱ�� */
    /* ������ʱ�� */
    rt_timer_start(&timer1);
    rt_timer_init(&timer2, "timer2",   /* ��ʱ�������� timer2 */
                    timeout2, /* ��ʱʱ�ص��Ĵ����� */
                    RT_NULL, /* ��ʱ��������ڲ��� */
                    300, /* ��ʱ����Ϊ300��OS Tick */
                    RT_TIMER_FLAG_ONE_SHOT); /* ���ζ�ʱ�� */

    /* ������ʱ�� */
    rt_timer_start(&timer2);
}
#include <finsh.h>
/* ������ msh �����б��� */
MSH_CMD_EXPORT(test_timer_02, timer test);