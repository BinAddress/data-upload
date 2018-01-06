#include "iic.h"
#include "delay.h"

 //MPU IIC ��ʱ����
void I2C1_Delay(void)
{
	delay_us(2);
}

//��ʼ��IIC
void I2C1_Init(void)
{					     
  GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);//��ʹ������IO PORTBʱ�� 
		
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;	 // �˿�����
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
  GPIO_Init(GPIOB, &GPIO_InitStructure);					 //�����趨������ʼ��GPIO 
	
  GPIO_SetBits(GPIOB,GPIO_Pin_6|GPIO_Pin_7);
	
}


//����IIC��ʼ�ź�
void I2C1_Start(void)
{
	MPU_SDA_OUT();     //sda�����
	I2C1_SDA=1;	  	  
	I2C1_SCL=1;
	I2C1_Delay();
 	I2C1_SDA=0;//START:when CLK is high,DATA change form high to low 
	I2C1_Delay();
	I2C1_SCL=0;//ǯסI2C���ߣ�׼�����ͻ�������� 
}	  
//����IICֹͣ�ź�
void I2C1_Stop(void)
{
	MPU_SDA_OUT();//sda�����
	I2C1_SCL=0;
	I2C1_SDA=0;//STOP:when CLK is high DATA change form low to high
 	I2C1_Delay();
	I2C1_SCL=1; 
	I2C1_SDA=1;//����I2C���߽����ź�
	I2C1_Delay();							   	
}
//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
u8 I2C1_Wait_Ack(void)
{
	u8 ucErrTime=0;
	I2C1_SDA_IN();      //SDA����Ϊ����  
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
	I2C1_SCL=0;//ʱ�����0 	   
	return 0;  
} 
//����ACKӦ��
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
//������ACKӦ��		    
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
//IIC����һ���ֽ�
//���شӻ�����Ӧ��
//1����Ӧ��
//0����Ӧ��			  
void I2C1_Send_Byte(u8 txd)
{                        
    u8 t;   
	MPU_SDA_OUT(); 	    
    I2C1_SCL=0;//����ʱ�ӿ�ʼ���ݴ���
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
//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK   
u8 I2C1_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	I2C1_SDA_IN();//SDA����Ϊ����
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
        I2C1_NAck();//����nACK
    else
        I2C1_Ack(); //����ACK   
    return receive;
}

//IIC��һ���ֽ� 
//reg:�Ĵ�����ַ 
//����ֵ:����������
u8 I2C1_Read_One(u8 addr, u8 reg)
{
	u8 res;
    I2C1_Start(); 
	I2C1_Send_Byte((addr<<1)&0xFE);//����������ַ+д����	
	I2C1_Wait_Ack();		//�ȴ�Ӧ�� 
	
	
    I2C1_Send_Byte(reg);	//д�Ĵ�����ַ
    I2C1_Wait_Ack();		//�ȴ�Ӧ��
    I2C1_Start();
	
	I2C1_Send_Byte((addr<<1)|1);//����������ַ+������	
    I2C1_Wait_Ack();		//�ȴ�Ӧ�� 
	res=I2C1_Read_Byte(0);//��ȡ����,����nACK 
    I2C1_Stop();			//����һ��ֹͣ���� 
	return res;		
	
}

//IICдһ���ֽ� 
//reg:�Ĵ�����ַ
//data:����
//����ֵ:0,����
//����,�������
u8 I2C1_Write_One(u8 addr, u8 reg,u8 data) 				 
{ 
  I2C1_Start(); 
	I2C1_Send_Byte((addr<<1)&0xFE);//����������ַ+д����	
	if(I2C1_Wait_Ack())	//�ȴ�Ӧ��
	{
		I2C1_Stop();		 
		return 1;		
	}
    I2C1_Send_Byte(reg);	//д�Ĵ�����ַ
    I2C1_Wait_Ack();		//�ȴ�Ӧ�� 
	I2C1_Send_Byte(data);//��������
	
	if(I2C1_Wait_Ack())	//�ȴ�ACK
	{
		I2C1_Stop();	 
		return 1;		 
	}		 
    I2C1_Stop();	 
	return 0;
}

//IIC����д
//addr:������ַ 
//reg:�Ĵ�����ַ
//len:д�볤��
//buf:������
//����ֵ:0,����
//    ����,�������
u8 I2C1_Write_Len(u8 addr,u8 reg,u8 len,u8 *buf)
{
	u8 i; 
    I2C1_Start(); 
	I2C1_Send_Byte((addr<<1)|0);//����������ַ+д����	
	if(I2C1_Wait_Ack())	//�ȴ�Ӧ��
	{
		I2C1_Stop();		 
		return 1;		
	}
    I2C1_Send_Byte(reg);	//д�Ĵ�����ַ
    I2C1_Wait_Ack();		//�ȴ�Ӧ��
	for(i=0;i<len;i++)
	{
		I2C1_Send_Byte(buf[i]);	//��������
		if(I2C1_Wait_Ack())		//�ȴ�ACK
		{
			I2C1_Stop();	 
			return 1;		 
		}		
	}    
    I2C1_Stop();	 
	return 0;	
} 

//IIC������
//addr:������ַ
//reg:Ҫ��ȡ�ļĴ�����ַ
//len:Ҫ��ȡ�ĳ���
//buf:��ȡ�������ݴ洢��
//����ֵ:0,����
//    ����,�������
u8 I2C1_Read_Len(u8 addr,u8 reg,u8 len,u8 *buf)
{ 
 	I2C1_Start(); 
	I2C1_Send_Byte((addr<<1)|0);//����������ַ+д����	
	if(I2C1_Wait_Ack())	//�ȴ�Ӧ��
	{
		I2C1_Stop();		 
		return 1;		
	}
    I2C1_Send_Byte(reg);	//д�Ĵ�����ַ
    I2C1_Wait_Ack();		//�ȴ�Ӧ��
    I2C1_Start();
	
	I2C1_Send_Byte((addr<<1)|1);//����������ַ+������	
    I2C1_Wait_Ack();		//�ȴ�Ӧ�� 
	while(len)
	{
		if(len==1)*buf=I2C1_Read_Byte(0);//������,����nACK 
		else *buf=I2C1_Read_Byte(1);		//������,����ACK  
		len--;
		buf++; 
	}    
    I2C1_Stop();	//����һ��ֹͣ���� 
	return 0;	
}
























