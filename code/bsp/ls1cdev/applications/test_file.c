#include <dfs_posix.h> /* ����Ҫʹ���ļ�����ʱ����Ҫ�������ͷ�ļ�*/
/* �����ļ���������һ����������� */
void test_file()
{
	int fd;
	char s[] = "RT-Thread Programmer!", buffer[80];
	/* �� /text.txt ��д�룬������ļ��������������ļ� */
	fd = open("/text.txt", O_WRONLY | O_CREAT,0);
	if (fd >= 0)
	{
		write(fd, s, sizeof(s));
		close(fd);
	}
	/* �� /text.txt ׼������ȡ���� */
	fd = open("/text.txt", O_RDONLY,0);
	if (fd >= 0)
	{
		read(fd, buffer, sizeof(buffer));
		close(fd);
	}
	rt_kprintf("%s", buffer);
}

#include <finsh.h>
/* ������ msh �����б��� */
MSH_CMD_EXPORT(test_file, file test);