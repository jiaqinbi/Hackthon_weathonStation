#ifndef __BMP180_H
#define __BMP180_H

#define	BMP180_SlaveAddress     0xee	//��ѹ������������ַ

#define   OSS 0	 
#define   REG_Pressure 0x34 // oss=0 4.5


void Init_BMP180(void);         //��ʼ��IO  ��ȡУ׼����
void bmp180Convert(void);	//У׼�¶Ⱥ���ѹ��������ֱ����result_up��result_UT

#endif