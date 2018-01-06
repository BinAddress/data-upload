#ifndef __MPUIIC_H
#define __MPUIIC_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������V3
//MPU6050 IIC���� ����	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2015/1/17
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////
 	   		   
//IO��������
#define I2C1_SDA_IN()  {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=((u32)8<<28);}
#define MPU_SDA_OUT() {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=((u32)3<<28);}

//IO��������	 
#define I2C1_SCL    PBout(6) 			//SCL
#define I2C1_SDA    PBout(7) 			//SDA
#define MPU_READ_SDA   PBin(7) 		//����SDA

//IIC���в�������
void I2C1_Delay(void);				//MPU IIC��ʱ����
void I2C1_Init(void);                //��ʼ��IIC��IO��				 
void I2C1_Start(void);				//����IIC��ʼ�ź�
void I2C1_Stop(void);	  			//����IICֹͣ�ź�
void I2C1_Send_Byte(u8 txd);			//IIC����һ���ֽ�
u8 I2C1_Read_Byte(unsigned char ack);//IIC��ȡһ���ֽ�
u8 I2C1_Wait_Ack(void); 				//IIC�ȴ�ACK�ź�
void I2C1_Ack(void);					//IIC����ACK�ź�
void I2C1_NAck(void);				//IIC������ACK�ź�

u8 I2C1_Read_One(u8 addr, u8 reg); 									//��һ������
u8 I2C1_Write_One(u8 addr, u8 reg,u8 data);  				//дһ������
u8 I2C1_Read_Len(u8 addr,u8 reg,u8 len,u8 *buf); 		//������
u8 I2C1_Write_Len(u8 addr,u8 reg,u8 len,u8 *buf);  	//����д

#endif
















