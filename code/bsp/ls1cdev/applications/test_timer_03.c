#include <rtthread.h>

/* ��ʱ���Ŀ��ƿ� */
static rt_timer_t timer1;
static rt_uint8_t count;

/* ��ʱ����ʱ���� */
static void timeout1(void* parameter)
{
    rt_kprintf("periodic timer is timeout\n");

    count ++;
    /* ������8��ʱ�����Ķ�ʱ���ĳ�ʱ���� */
    if (count >= 8)
    {
        int timeout_value = 500;
        /* ���ƶ�ʱ�����Ķ�ʱ����ʱʱ�䳤�� */
        rt_timer_control(timer1, RT_TIMER_CTRL_SET_TIME, (void*)&timeout_value);
        count = 0;
    }
}

void test_timer_03(void)
{

	/* ������ʱ��1 */
	timer1 = rt_timer_create("timer1",  /* ��ʱ�������� timer1 */
							timeout1, /* ��ʱʱ�ص��Ĵ����� */
							RT_NULL, /* ��ʱ��������ڲ��� */
							100, /* ��ʱ���ȣ���OS TickΪ��λ����10��OS Tick */
							RT_TIMER_FLAG_PERIODIC); /* �����Զ�ʱ�� */
	/* ������ʱ�� */
	if (timer1 != RT_NULL)
		rt_timer_start(timer1);
}
#include <finsh.h>
/* ������ msh �����б��� */
MSH_CMD_EXPORT(test_timer_03, timer test);