#ifndef __MPUIIC_H
#define __MPUIIC_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板V3
//MPU6050 IIC驱动 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2015/1/17
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////
 	   		   
//IO方向设置
#define I2C1_SDA_IN()  {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=((u32)8<<28);}
#define MPU_SDA_OUT() {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=((u32)3<<28);}

//IO操作函数	 
#define I2C1_SCL    PBout(6) 			//SCL
#define I2C1_SDA    PBout(7) 			//SDA
#define MPU_READ_SDA   PBin(7) 		//输入SDA

//IIC所有操作函数
void I2C1_Delay(void);				//MPU IIC延时函数
void I2C1_Init(void);                //初始化IIC的IO口				 
void I2C1_Start(void);				//发送IIC开始信号
void I2C1_Stop(void);	  			//发送IIC停止信号
void I2C1_Send_Byte(u8 txd);			//IIC发送一个字节
u8 I2C1_Read_Byte(unsigned char ack);//IIC读取一个字节
u8 I2C1_Wait_Ack(void); 				//IIC等待ACK信号
void I2C1_Ack(void);					//IIC发送ACK信号
void I2C1_NAck(void);				//IIC不发送ACK信号

u8 I2C1_Read_One(u8 addr, u8 reg); 									//读一个数据
u8 I2C1_Write_One(u8 addr, u8 reg,u8 data);  				//写一个数据
u8 I2C1_Read_Len(u8 addr,u8 reg,u8 len,u8 *buf); 		//连续读
u8 I2C1_Write_Len(u8 addr,u8 reg,u8 len,u8 *buf);  	//连续写

#endif
















