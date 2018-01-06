#include "mpu6050.h"
#include "sys.h"
#include "delay.h"
#include "usart.h"   
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h" 

char mpu_flag = 1;
dt_mpu mpu6050;
float Roll,Pitch,Yaw; 		//ŷ����

//��ʼ��MPU6050
//����ֵ:0,�ɹ�
//    ����,�������
u8 MPU_Init(void)
{ 
	u8 res = 0;
	char i = 0;
  GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);//ʹ��AFIOʱ�� 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//��ʹ������IO PORTAʱ�� 
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;	 // �˿�����
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
  GPIO_Init(GPIOA, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOA

	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);//��ֹJTAG,�Ӷ�PA15��������ͨIOʹ��,����PA15��������ͨIO!!!
	
	MPU_AD0_CTRL=0;			//����MPU6050��AD0��Ϊ�͵�ƽ,�ӻ���ַΪ:0X68
		
	I2C1_Write_One(MPU_ADDR,MPU_PWR_MGMT1_REG,0X80);	//��λMPU6050
  delay_ms(100);
	I2C1_Write_One(MPU_ADDR,MPU_PWR_MGMT1_REG,0X00);	//����MPU6050 
	MPU_Set_Gyro_Fsr(3);					//�����Ǵ�����,��2000dps
	MPU_Set_Accel_Fsr(0);					//���ٶȴ�����,��2g
	MPU_Set_Rate(50);						//���ò�����50Hz
	I2C1_Write_One(MPU_ADDR,MPU_INT_EN_REG,0X00);	//�ر������ж�
	I2C1_Write_One(MPU_ADDR,MPU_USER_CTRL_REG,0X00);	//I2C��ģʽ�ر�
	I2C1_Write_One(MPU_ADDR,MPU_FIFO_EN_REG,0X00);	//�ر�FIFO
	I2C1_Write_One(MPU_ADDR,MPU_INTBP_CFG_REG,0X80);	//INT���ŵ͵�ƽ��Ч
	res=I2C1_Read_One(MPU_ADDR,MPU_DEVICE_ID_REG);
	
	if(res==MPU_ADDR)//����ID��ȷ
	{
		I2C1_Write_One(MPU_ADDR,MPU_PWR_MGMT1_REG,0X01);	//����CLKSEL,PLL X��Ϊ�ο�
		I2C1_Write_One(MPU_ADDR,MPU_PWR_MGMT2_REG,0X00);	//���ٶ��������Ƕ�����
		MPU_Set_Rate(50);						//���ò�����Ϊ50Hz
 	}
	else 
	{
		return 1;
	}
	
	mpu_flag = 1;	
	i=0;
	while(mpu_dmp_init())
 	{
			delay_ms(10);
			if(++i >= 30)
			{
				mpu_flag = 0; //��ʼ��ʧ��
				printf("MPU Init Error\r \n");
				return 1;
			}
	} 
	
	return 0;
}
//����MPU6050�����Ǵ����������̷�Χ
//fsr:0,��250dps;1,��500dps;2,��1000dps;3,��2000dps
//����ֵ:0,���óɹ�
//    ����,����ʧ�� 
u8 MPU_Set_Gyro_Fsr(u8 fsr)
{
	return I2C1_Write_One(MPU_ADDR,MPU_GYRO_CFG_REG,fsr<<3);//���������������̷�Χ  
}
//����MPU6050���ٶȴ����������̷�Χ
//fsr:0,��2g;1,��4g;2,��8g;3,��16g
//����ֵ:0,���óɹ�
//    ����,����ʧ�� 
u8 MPU_Set_Accel_Fsr(u8 fsr)
{
	return I2C1_Write_One(MPU_ADDR,MPU_ACCEL_CFG_REG,fsr<<3);//���ü��ٶȴ����������̷�Χ  
}
//����MPU6050�����ֵ�ͨ�˲���
//lpf:���ֵ�ͨ�˲�Ƶ��(Hz)
//����ֵ:0,���óɹ�
//    ����,����ʧ�� 
u8 MPU_Set_LPF(u16 lpf)
{
	u8 data=0;
	if(lpf>=188)data=1;
	else if(lpf>=98)data=2;
	else if(lpf>=42)data=3;
	else if(lpf>=20)data=4;
	else if(lpf>=10)data=5;
	else data=6; 
	return I2C1_Write_One(MPU_ADDR,MPU_CFG_REG,data);//�������ֵ�ͨ�˲���  
}
//����MPU6050�Ĳ�����(�ٶ�Fs=1KHz)
//rate:4~1000(Hz)
//����ֵ:0,���óɹ�
//    ����,����ʧ�� 
u8 MPU_Set_Rate(u16 rate)
{
	u8 data;
	if(rate>1000)rate=1000;
	if(rate<4)rate=4;
	data=1000/rate-1;
	data=I2C1_Write_One(MPU_ADDR,MPU_SAMPLE_RATE_REG,data);	//�������ֵ�ͨ�˲���
 	return MPU_Set_LPF(rate/2);	//�Զ�����LPFΪ�����ʵ�һ��
}

//�õ��¶�ֵ
//����ֵ:�¶�ֵ(������100��)
short MPU_Get_Temperature(void)
{
    u8 buf[2]; 
    short raw;
	float temp;
	I2C1_Read_Len(MPU_ADDR,MPU_TEMP_OUTH_REG,2,buf); 
    raw=((u16)buf[0]<<8)|buf[1];  
    temp=36.53+((double)raw)/340;  
    return temp*100;;
}
//�õ�������ֵ(ԭʼֵ)
//gx,gy,gz:������x,y,z���ԭʼ����(������)
//����ֵ:0,�ɹ�
//    ����,�������
u8 MPU_Get_Gyroscope(short *gx,short *gy,short *gz)
{
    u8 buf[6],res;  
	res=I2C1_Read_Len(MPU_ADDR,MPU_GYRO_XOUTH_REG,6,buf);
	if(res==0)
	{
		*gx=((u16)buf[0]<<8)|buf[1];  
		*gy=((u16)buf[2]<<8)|buf[3];  
		*gz=((u16)buf[4]<<8)|buf[5];
	} 	
    return res;;
}
//�õ����ٶ�ֵ(ԭʼֵ)
//gx,gy,gz:������x,y,z���ԭʼ����(������)
//����ֵ:0,�ɹ�
//    ����,�������
u8 MPU_Get_Accelerometer(short *ax,short *ay,short *az)
{
    u8 buf[6],res;  
	res=I2C1_Read_Len(MPU_ADDR,MPU_ACCEL_XOUTH_REG,6,buf);
	if(res==0)
	{
		*ax=((u16)buf[0]<<8)|buf[1];  
		*ay=((u16)buf[2]<<8)|buf[3];  
		*az=((u16)buf[4]<<8)|buf[5];
	} 	
    return res;;
}

void MPU6050_Read(void)
{
	if(mpu_flag) //mpu init ok
	{
		if(mpu_dmp_get_data(&Pitch,&Roll,&Yaw)==0)
		{ 
//			mpu6050.temp = MPU_Get_Temperature();	//�õ��¶�ֵ
//			MPU_Get_Accelerometer(&mpu6050.Acc.x,&mpu6050.Acc.y,&mpu6050.Acc.z);	//�õ����ٶȴ���������
//			MPU_Get_Gyroscope(&mpu6050.Gyro.x,&mpu6050.Gyro.y,&mpu6050.Gyro.z);	//�õ�����������
		}		
	}
}


