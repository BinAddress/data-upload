#include "iic.h"
#include "delay.h"

 //MPU IIC 延时函数
void I2C1_Delay(void)
{
	delay_us(2);
}

//初始化IIC
void I2C1_Init(void)
{					     
  GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);//先使能外设IO PORTB时钟 
		
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;	 // 端口配置
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
  GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化GPIO 
	
  GPIO_SetBits(GPIOB,GPIO_Pin_6|GPIO_Pin_7);
	
}


//产生IIC起始信号
void I2C1_Start(void)
{
	MPU_SDA_OUT();     //sda线输出
	I2C1_SDA=1;	  	  
	I2C1_SCL=1;
	I2C1_Delay();
 	I2C1_SDA=0;//START:when CLK is high,DATA change form high to low 
	I2C1_Delay();
	I2C1_SCL=0;//钳住I2C总线，准备发送或接收数据 
}	  
//产生IIC停止信号
void I2C1_Stop(void)
{
	MPU_SDA_OUT();//sda线输出
	I2C1_SCL=0;
	I2C1_SDA=0;//STOP:when CLK is high DATA change form low to high
 	I2C1_Delay();
	I2C1_SCL=1; 
	I2C1_SDA=1;//发送I2C总线结束信号
	I2C1_Delay();							   	
}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 I2C1_Wait_Ack(void)
{
	u8 ucErrTime=0;
	I2C1_SDA_IN();      //SDA设置为输入  
	I2C1_SDA=1;I2C1_Delay();	   
	I2C1_SCL=1;I2C1_Delay();	 
	while(MPU_READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			I2C1_Stop();
			return 1;
		}
	}
	I2C1_SCL=0;//时钟输出0 	   
	return 0;  
} 
//产生ACK应答
void I2C1_Ack(void)
{
	I2C1_SCL=0;
	MPU_SDA_OUT();
	I2C1_SDA=0;
	I2C1_Delay();
	I2C1_SCL=1;
	I2C1_Delay();
	I2C1_SCL=0;
}
//不产生ACK应答		    
void I2C1_NAck(void)
{
	I2C1_SCL=0;
	MPU_SDA_OUT();
	I2C1_SDA=1;
	I2C1_Delay();
	I2C1_SCL=1;
	I2C1_Delay();
	I2C1_SCL=0;
}					 				     
//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
void I2C1_Send_Byte(u8 txd)
{                        
    u8 t;   
	MPU_SDA_OUT(); 	    
    I2C1_SCL=0;//拉低时钟开始数据传输
    for(t=0;t<8;t++)
    {              
        I2C1_SDA=(txd&0x80)>>7;
        txd<<=1; 	  
		    I2C1_SCL=1;
		    I2C1_Delay(); 
		    I2C1_SCL=0;	
		    I2C1_Delay();
    }	 
} 	    
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
u8 I2C1_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	I2C1_SDA_IN();//SDA设置为输入
    for(i=0;i<8;i++ )
	{
        I2C1_SCL=0; 
        I2C1_Delay();
		I2C1_SCL=1;
        receive<<=1;
        if(MPU_READ_SDA)receive++;   
		I2C1_Delay(); 
    }					 
    if (!ack)
        I2C1_NAck();//发送nACK
    else
        I2C1_Ack(); //发送ACK   
    return receive;
}

//IIC读一个字节 
//reg:寄存器地址 
//返回值:读到的数据
u8 I2C1_Read_One(u8 addr, u8 reg)
{
	u8 res;
    I2C1_Start(); 
	I2C1_Send_Byte((addr<<1)&0xFE);//发送器件地址+写命令	
	I2C1_Wait_Ack();		//等待应答 
	
	
    I2C1_Send_Byte(reg);	//写寄存器地址
    I2C1_Wait_Ack();		//等待应答
    I2C1_Start();
	
	I2C1_Send_Byte((addr<<1)|1);//发送器件地址+读命令	
    I2C1_Wait_Ack();		//等待应答 
	res=I2C1_Read_Byte(0);//读取数据,发送nACK 
    I2C1_Stop();			//产生一个停止条件 
	return res;		
	
}

//IIC写一个字节 
//reg:寄存器地址
//data:数据
//返回值:0,正常
//其他,错误代码
u8 I2C1_Write_One(u8 addr, u8 reg,u8 data) 				 
{ 
  I2C1_Start(); 
	I2C1_Send_Byte((addr<<1)&0xFE);//发送器件地址+写命令	
	if(I2C1_Wait_Ack())	//等待应答
	{
		I2C1_Stop();		 
		return 1;		
	}
    I2C1_Send_Byte(reg);	//写寄存器地址
    I2C1_Wait_Ack();		//等待应答 
	I2C1_Send_Byte(data);//发送数据
	
	if(I2C1_Wait_Ack())	//等待ACK
	{
		I2C1_Stop();	 
		return 1;		 
	}		 
    I2C1_Stop();	 
	return 0;
}

//IIC连续写
//addr:器件地址 
//reg:寄存器地址
//len:写入长度
//buf:数据区
//返回值:0,正常
//    其他,错误代码
u8 I2C1_Write_Len(u8 addr,u8 reg,u8 len,u8 *buf)
{
	u8 i; 
    I2C1_Start(); 
	I2C1_Send_Byte((addr<<1)|0);//发送器件地址+写命令	
	if(I2C1_Wait_Ack())	//等待应答
	{
		I2C1_Stop();		 
		return 1;		
	}
    I2C1_Send_Byte(reg);	//写寄存器地址
    I2C1_Wait_Ack();		//等待应答
	for(i=0;i<len;i++)
	{
		I2C1_Send_Byte(buf[i]);	//发送数据
		if(I2C1_Wait_Ack())		//等待ACK
		{
			I2C1_Stop();	 
			return 1;		 
		}		
	}    
    I2C1_Stop();	 
	return 0;	
} 

//IIC连续读
//addr:器件地址
//reg:要读取的寄存器地址
//len:要读取的长度
//buf:读取到的数据存储区
//返回值:0,正常
//    其他,错误代码
u8 I2C1_Read_Len(u8 addr,u8 reg,u8 len,u8 *buf)
{ 
 	I2C1_Start(); 
	I2C1_Send_Byte((addr<<1)|0);//发送器件地址+写命令	
	if(I2C1_Wait_Ack())	//等待应答
	{
		I2C1_Stop();		 
		return 1;		
	}
    I2C1_Send_Byte(reg);	//写寄存器地址
    I2C1_Wait_Ack();		//等待应答
    I2C1_Start();
	
	I2C1_Send_Byte((addr<<1)|1);//发送器件地址+读命令	
    I2C1_Wait_Ack();		//等待应答 
	while(len)
	{
		if(len==1)*buf=I2C1_Read_Byte(0);//读数据,发送nACK 
		else *buf=I2C1_Read_Byte(1);		//读数据,发送ACK  
		len--;
		buf++; 
	}    
    I2C1_Stop();	//产生一个停止条件 
	return 0;	
}
























