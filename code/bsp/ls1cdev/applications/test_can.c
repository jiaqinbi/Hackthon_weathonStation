/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * File      : test_can.c
����Ӳ��can0������ ��finsh ������ 
1.test_cansend()  ������������    ��ʼ��CAN�ڣ�250K����������100K, ���ͱ�׼����֡��1-8 ��8�����ݣ�����չ����֡��1-8 ��8������)
2. candump() ��ӡ����ǰCAN�Ĵ���ֵ
 */

#include <rtthread.h>
#include <ipc/completion.h>
#include <drivers/can.h>
#include <rthw.h>

#include <stdlib.h>  
#include "ls1c.h"

#include "ls1c_public.h"
#include "ls1c_regs.h"
#include "ls1c_clock.h"
#include "ls1c_can.h"
#include "ls1c_pin.h"

static CanRxMsg RxMessage;
static CanTxMsg TxMessage;

void candump(void)
{
    CAN_TypeDef* CANx;
    unsigned char temp;
    CANx =  CAN0;
    int i;

    temp =  CANx->MOD;
    rt_kprintf("\r\ncan0->MOD= 0x%02x  \r\n",temp);   

    temp =  CANx->CMR;
    rt_kprintf("\ncan0->CMR = 0x%02x  \r\n",temp);  

    temp =  CANx->SR;
    rt_kprintf("\ncan0->SR = 0x%02x  \r\n",temp); 

    temp =  CANx->IR;
    rt_kprintf("\ncan0->IR = 0x%02x  \r\n",temp); 

    temp =  CANx->IER;
    rt_kprintf("\ncan0->IER = 0x%02x  \r\n",temp); 

    temp =  CANx->BTR0;
    rt_kprintf("\ncan0->BTR0 = 0x%02x  \r\n",temp); 

    temp =  CANx->BTR1;
    rt_kprintf("\ncan0->BTR1 = 0x%02x  \r\n",temp); 

    temp =  CANx->CDR;
    rt_kprintf("\ncan0->CDR = 0x%02x  \r\n",temp); 

    temp =  CANx->RMCR;
    rt_kprintf("\ncan0->RMCR = 0x%02x  \r\n",temp); 
    rt_kprintf("\r\n  data =  ",temp); 
    for(i=0;i<8;i++)
    {
        temp = CANx->BUF[i];
        rt_kprintf(" 0x%02x  \r\n",temp); 
    }
    
   /* ���븴λģʽ */
  set_reset_mode(CANx);

    rt_kprintf("\r\n ���մ���:");
    temp =  CANx->IDE_RTR_DLC;
    rt_kprintf("  0x%02x ",temp); 
    temp =  CANx->ID[0];
    rt_kprintf("  0x%02x ",temp); 
    temp =  CANx->ID[1];
    rt_kprintf("  0x%02x ",temp); 
    temp =  CANx->ID[2];
    rt_kprintf("  0x%02x ",temp); 

    rt_kprintf("\r\n ��������:");
    temp =  CANx->ID[3];
    rt_kprintf("  0x%02x ",temp); 
    temp =  CANx->BUF[0];
    rt_kprintf("  0x%02x ",temp); 
    temp =  CANx->BUF[1];
    rt_kprintf("  0x%02x ",temp); 
    temp =  CANx->BUF[2];
    rt_kprintf("  0x%02x ",temp); 

    /* ���빤��ģʽ */
    set_start(CANx);    
}


static void ls1c_can0_irqhandlertest(int irq, void *param)  
{  
    CAN_TypeDef* CANx;
    int i;
    unsigned char status;
    CANx =  CAN0;
    /*���Ĵ�������ж�*/
   status = CANx->IR;
   rt_kprintf("\r\ncan0 int happened!\r\n");
    /*�����ж�*/
    if (( status & CAN_IR_RI) == CAN_IR_RI) 
    {
        /*���RI �ж�*/
       CAN_Receive(CANx, &RxMessage);
       CANx->CMR |= CAN_CMR_RRB; 
       CANx->CMR |= CAN_CMR_CDO; 
       rt_kprintf("\r\ncan0 receive:\r\n");
       rt_kprintf(" IDE=%d   RTR = %d DLC=%d  ",RxMessage.IDE, RxMessage.RTR ,RxMessage.DLC);
       if(RxMessage.IDE == CAN_Id_Standard)
       {
         rt_kprintf("\r\n Standard ID= %02X  ",RxMessage.StdId);
       }
       else if(RxMessage.IDE == CAN_Id_Extended)
       {
         rt_kprintf("\r\n Extended ID= %02X  ",RxMessage.ExtId);
       }

       if(RxMessage.RTR== CAN_RTR_DATA)
       {
         rt_kprintf("\r\ndata= ");
         for(i=0;i<RxMessage.DLC;i++)
         {
           rt_kprintf("0x%02X  ",RxMessage.Data[i]);
         }
       }
       else if(RxMessage.IDE == CAN_RTR_Remote)
       {
         rt_kprintf("\r\nCAN_RTR_Remote  no data!");
       }
       rt_kprintf("\r\n");
    }
    /*�����ж�*/
    else if (( status  & CAN_IR_TI) == CAN_IR_TI) 
    {
       rt_kprintf("\r\ncan0 send success! \r\n");
    }
    /*��������ж�*/
    else if (( status  & CAN_IR_TI) == CAN_IR_DOI) 
    {
       rt_kprintf("\r\ncan0 data over flow! \r\n");
    }
}  


static void ls1c_can1_irqhandlertest(int irq, void *param)  
{  
    CAN_TypeDef* CANx;
    int i;
    unsigned char status;
    CANx =  CAN1;
    /*���Ĵ�������ж�*/
   status = CANx->IR;
   rt_kprintf("\r\ncan1 int happened!\r\n");
    /*�����ж�*/
    if (( status & CAN_IR_RI) == CAN_IR_RI) 
    {
        /*���RI �ж�*/
       CAN_Receive(CANx, &RxMessage);
       CANx->CMR |= CAN_CMR_RRB; 
       CANx->CMR |= CAN_CMR_CDO; 
       rt_kprintf("\r\ncan1 receive:\r\n");
       rt_kprintf(" IDE=%d   RTR = %d DLC=%d  ",RxMessage.IDE, RxMessage.RTR ,RxMessage.DLC);
       if(RxMessage.IDE == CAN_Id_Standard)
       {
         rt_kprintf("\r\n Standard ID= %02X  ",RxMessage.StdId);
       }
       else if(RxMessage.IDE == CAN_Id_Extended)
       {
         rt_kprintf("\r\n Extended ID= %02X  ",RxMessage.ExtId);
       }

       if(RxMessage.RTR== CAN_RTR_DATA)
       {
         rt_kprintf("\r\ndata= ");
         for(i=0;i<RxMessage.DLC;i++)
         {
           rt_kprintf("0x%02X  ",RxMessage.Data[i]);
         }
       }
       else if(RxMessage.IDE == CAN_RTR_Remote)
       {
         rt_kprintf("\r\nCAN_RTR_Remote  no data!");
       }
       rt_kprintf("\r\n");
    }
    /*�����ж�*/
    else if (( status  & CAN_IR_TI) == CAN_IR_TI) 
    {
       rt_kprintf("\r\ncan1 send success! \r\n");
    }
    /*��������ж�*/
    else if (( status  & CAN_IR_TI) == CAN_IR_DOI) 
    {
       rt_kprintf("\r\ncan1 data over flow! \r\n");
    }
}  

void Can_Config(CAN_TypeDef* CANx, Ls1c_CanBPS_t bps)
{
    unsigned char initresult;

    if( (CAN_TypeDef* )LS1C_REG_BASE_CAN0 == CANx)
    {
        pin_set_purpose(54, PIN_PURPOSE_OTHER);
        pin_set_purpose(55, PIN_PURPOSE_OTHER);
        pin_set_remap(54, PIN_REMAP_THIRD);
        pin_set_remap(55, PIN_REMAP_THIRD);
    }
    else if( (CAN_TypeDef* )LS1C_REG_BASE_CAN1 == CANx)
    {
        pin_set_purpose(56, PIN_PURPOSE_OTHER);
        pin_set_purpose(57, PIN_PURPOSE_OTHER);
        pin_set_remap(56, PIN_REMAP_THIRD);
        pin_set_remap(57, PIN_REMAP_THIRD);
    }
    
    CAN_InitTypeDef        CAN_InitStructure;
  
    CAN_InitStructure.CAN_Mode = CAN_Mode_STM;
    CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;

    /* BaudRate= f(APB)/((1+BS1+BS2)(SJW*2*Prescaler))=126000000/[(1+7+2)*1*2*63]=100000=100K*/
    /* BPS     PRE   BS1   BS2   ���40K
       1M      9       4       2   
       800K    8       7       2   
       500K    9       11      2   
       250K    36      4       2   
       125K    36      11      2   
       100K    63      7       2   
       50K     63      16      3`  
       40K     63      16      8   
    */
    switch (bps)
    {
        case LS1C_CAN1MBaud: 
            CAN_InitStructure.CAN_Prescaler = 9;
            CAN_InitStructure.CAN_BS1 = CAN_BS1_4tq;
            CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq;
        break;
        case LS1C_CAN800kBaud: 
            CAN_InitStructure.CAN_Prescaler = 8;
            CAN_InitStructure.CAN_BS1 = CAN_BS1_7tq;
            CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq;
        break;
        case LS1C_CAN500kBaud: 
            CAN_InitStructure.CAN_Prescaler = 9;
            CAN_InitStructure.CAN_BS1 = CAN_BS1_11tq;
            CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq;
        break;
        case LS1C_CAN250kBaud: 
            CAN_InitStructure.CAN_Prescaler = 36;
            CAN_InitStructure.CAN_BS1 = CAN_BS1_4tq;
            CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq;
        break;
        case LS1C_CAN125kBaud: 
            CAN_InitStructure.CAN_Prescaler = 36;
            CAN_InitStructure.CAN_BS1 = CAN_BS1_11tq;
            CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq;
        break;
        case LS1C_CAN100kBaud: 
            CAN_InitStructure.CAN_Prescaler = 63;
            CAN_InitStructure.CAN_BS1 = CAN_BS1_7tq;
            CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq;
        break;
        case LS1C_CAN50kBaud: 
            CAN_InitStructure.CAN_Prescaler = 63;
            CAN_InitStructure.CAN_BS1 = CAN_BS1_16tq;
            CAN_InitStructure.CAN_BS2 = CAN_BS2_3tq;
        break;
        case LS1C_CAN40kBaud: 
            CAN_InitStructure.CAN_Prescaler = 63;
            CAN_InitStructure.CAN_BS1 = CAN_BS1_16tq;
            CAN_InitStructure.CAN_BS2 = CAN_BS2_8tq;
        break;
        default: //100K
            CAN_InitStructure.CAN_Prescaler = 63;
            CAN_InitStructure.CAN_BS1 = CAN_BS1_7tq;
            CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq;
        break;
    }
    initresult = CAN_Init(CANx, &CAN_InitStructure);// ���ó�100K
	
    if( (CAN_TypeDef* )LS1C_REG_BASE_CAN0 == CANx)
    {
       /* ��ʼ��CAN0�����ж�*/
      rt_hw_interrupt_install(LS1C_CAN0_IRQ, ls1c_can0_irqhandlertest, RT_NULL, "can0");  
      rt_hw_interrupt_umask(LS1C_CAN0_IRQ); 
     }
    if( (CAN_TypeDef* )LS1C_REG_BASE_CAN1 == CANx)
    {
       /* ��ʼ��CAN0�����ж�*/
      rt_hw_interrupt_install(LS1C_CAN1_IRQ, ls1c_can1_irqhandlertest, RT_NULL, "can1");  
      rt_hw_interrupt_umask(LS1C_CAN1_IRQ); 
     }
}

void test_cansend(void)
{
    unsigned char temp;
    unsigned char initresult;

    Can_Config(CAN0, LS1C_CAN100kBaud);
    Can_Config(CAN1, LS1C_CAN100kBaud);

    CAN_SetBps(CAN0, LS1C_CAN250kBaud);
    CAN_SetBps(CAN1, LS1C_CAN250kBaud);

    CAN_FilterInitTypeDef canfilter;
    canfilter.IDE = 1;   /*0: ʹ�ñ�׼��ʶ��1: ʹ����չ��ʶ��*/
    canfilter.RTR = 1;  /*0: ����֡     1: Զ��֡*/
    canfilter.MODE = 0; /* 0- ˫�˲���ģʽ;1-���˲���ģʽ*/
    canfilter.First_Data= 0x5A; /*˫�˲���ģʽ����Ϣ��һ�������ֽ�*/
    canfilter.Data_Mask = 0x00;/*˫�˲���ģʽ����Ϣ��һ�������ֽ�����*/
    canfilter.ID = 0x00010002;
/*���մ���   ˫�˲���-  ��չ֡id1  =  x<<13  (x=1 )=0x4000  id2 =  x<<13  ( x=2)  =0x2000 13-29λ*/
/*���մ���   ˫�˲���-  ��׼֡id 1 =  1  (data=0x5A)   id2 = 2   */	
/*���մ���   ���˲���-  ��չ֡id  = 0x00010002    */	
/*���մ���   ���˲���-  ��׼֡id = 0x02  */	
/*rtr λ������Ӱ�죬���ֲ᲻ͬ */	

    canfilter.IDMASK = 0x0; /*��������*/
    CAN_FilterInit(CAN0, &canfilter); 
    CAN_FilterInit(CAN1, &canfilter); 

    int i;
    TxMessage.StdId = 1;
    TxMessage.ExtId = 1;
    TxMessage.RTR = CAN_RTR_DATA;
    TxMessage.IDE = CAN_Id_Standard;
    TxMessage.DLC = 8;
    for( i=0; i<8;i++)
    {
      TxMessage.Data[i] = i+1;
      rt_kprintf("%02x ",  TxMessage.Data[i]);
    }
    CAN_Transmit(CAN0, &TxMessage);
    CAN_Transmit(CAN1, &TxMessage);
    
    TxMessage.StdId = 1;
    TxMessage.ExtId = 2;
    TxMessage.RTR = CAN_RTR_DATA;
    TxMessage.IDE = CAN_Id_Extended;
    TxMessage.DLC = 8;
    for( i=0; i<8;i++)
    {
      TxMessage.Data[i] = i+1;
      rt_kprintf("%02x ",  TxMessage.Data[i]);
    }
    CAN_Transmit(CAN0, &TxMessage);	
    CAN_Transmit(CAN1, &TxMessage);	
}

#include  <finsh.h> 
FINSH_FUNCTION_EXPORT(test_cansend, test_cansend  e.g.test_cansend());
FINSH_FUNCTION_EXPORT(candump, candump  e.g.candump());
/* ������ msh �����б��� */
MSH_CMD_EXPORT(test_cansend, can send  sample);
MSH_CMD_EXPORT(candump, can candump);
