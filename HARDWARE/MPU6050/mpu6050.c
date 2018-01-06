#include "mpu6050.h"
#include "sys.h"
#include "delay.h"
#include "usart.h"   
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h" 

char mpu_flag = 1;
dt_mpu mpu6050;
float Roll,Pitch,Yaw; 		//欧拉角

//初始化MPU6050
//返回值:0,成功
//    其他,错误代码
u8 MPU_Init(void)
{ 
	u8 res = 0;
	char i = 0;
  GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);//使能AFIO时钟 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//先使能外设IO PORTA时钟 
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;	 // 端口配置
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
  GPIO_Init(GPIOA, &GPIO_InitStructure);					 //根据设定参数初始化GPIOA

	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);//禁止JTAG,从而PA15可以做普通IO使用,否则PA15不能做普通IO!!!
	
	MPU_AD0_CTRL=0;			//控制MPU6050的AD0脚为低电平,从机地址为:0X68
		
	I2C1_Write_One(MPU_ADDR,MPU_PWR_MGMT1_REG,0X80);	//复位MPU6050
  delay_ms(100);
	I2C1_Write_One(MPU_ADDR,MPU_PWR_MGMT1_REG,0X00);	//唤醒MPU6050 
	MPU_Set_Gyro_Fsr(3);					//陀螺仪传感器,±2000dps
	MPU_Set_Accel_Fsr(0);					//加速度传感器,±2g
	MPU_Set_Rate(50);						//设置采样率50Hz
	I2C1_Write_One(MPU_ADDR,MPU_INT_EN_REG,0X00);	//关闭所有中断
	I2C1_Write_One(MPU_ADDR,MPU_USER_CTRL_REG,0X00);	//I2C主模式关闭
	I2C1_Write_One(MPU_ADDR,MPU_FIFO_EN_REG,0X00);	//关闭FIFO
	I2C1_Write_One(MPU_ADDR,MPU_INTBP_CFG_REG,0X80);	//INT引脚低电平有效
	res=I2C1_Read_One(MPU_ADDR,MPU_DEVICE_ID_REG);
	
	if(res==MPU_ADDR)//器件ID正确
	{
		I2C1_Write_One(MPU_ADDR,MPU_PWR_MGMT1_REG,0X01);	//设置CLKSEL,PLL X轴为参考
		I2C1_Write_One(MPU_ADDR,MPU_PWR_MGMT2_REG,0X00);	//加速度与陀螺仪都工作
		MPU_Set_Rate(50);						//设置采样率为50Hz
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
				mpu_flag = 0; //初始化失败
				printf("MPU Init Error\r \n");
				return 1;
			}
	} 
	
	return 0;
}
//设置MPU6050陀螺仪传感器满量程范围
//fsr:0,±250dps;1,±500dps;2,±1000dps;3,±2000dps
//返回值:0,设置成功
//    其他,设置失败 
u8 MPU_Set_Gyro_Fsr(u8 fsr)
{
	return I2C1_Write_One(MPU_ADDR,MPU_GYRO_CFG_REG,fsr<<3);//设置陀螺仪满量程范围  
}
//设置MPU6050加速度传感器满量程范围
//fsr:0,±2g;1,±4g;2,±8g;3,±16g
//返回值:0,设置成功
//    其他,设置失败 
u8 MPU_Set_Accel_Fsr(u8 fsr)
{
	return I2C1_Write_One(MPU_ADDR,MPU_ACCEL_CFG_REG,fsr<<3);//设置加速度传感器满量程范围  
}
//设置MPU6050的数字低通滤波器
//lpf:数字低通滤波频率(Hz)
//返回值:0,设置成功
//    其他,设置失败 
u8 MPU_Set_LPF(u16 lpf)
{
	u8 data=0;
	if(lpf>=188)data=1;
	else if(lpf>=98)data=2;
	else if(lpf>=42)data=3;
	else if(lpf>=20)data=4;
	else if(lpf>=10)data=5;
	else data=6; 
	return I2C1_Write_One(MPU_ADDR,MPU_CFG_REG,data);//设置数字低通滤波器  
}
//设置MPU6050的采样率(假定Fs=1KHz)
//rate:4~1000(Hz)
//返回值:0,设置成功
//    其他,设置失败 
u8 MPU_Set_Rate(u16 rate)
{
	u8 data;
	if(rate>1000)rate=1000;
	if(rate<4)rate=4;
	data=1000/rate-1;
	data=I2C1_Write_One(MPU_ADDR,MPU_SAMPLE_RATE_REG,data);	//设置数字低通滤波器
 	return MPU_Set_LPF(rate/2);	//自动设置LPF为采样率的一半
}

//得到温度值
//返回值:温度值(扩大了100倍)
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
//得到陀螺仪值(原始值)
//gx,gy,gz:陀螺仪x,y,z轴的原始读数(带符号)
//返回值:0,成功
//    其他,错误代码
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
//得到加速度值(原始值)
//gx,gy,gz:陀螺仪x,y,z轴的原始读数(带符号)
//返回值:0,成功
//    其他,错误代码
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
//			mpu6050.temp = MPU_Get_Temperature();	//得到温度值
//			MPU_Get_Accelerometer(&mpu6050.Acc.x,&mpu6050.Acc.y,&mpu6050.Acc.z);	//得到加速度传感器数据
//			MPU_Get_Gyroscope(&mpu6050.Gyro.x,&mpu6050.Gyro.y,&mpu6050.Gyro.z);	//得到陀螺仪数据
		}		
	}
}


