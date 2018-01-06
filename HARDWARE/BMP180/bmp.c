#include "BMP.h"
#include "delay.h"
#include "iic.h"
#include "usart.h"
#include <math.h>
#include <string.h>


//////////////////////////////////////////////////////////////////////////////////
//气压传感器BMP180的STM32驱动，本程序只供学习使用 
//////////////////////////////////////////////////////////////////////////////////

//BMP180校准系数
short AC1;
short AC2;
short AC3;
unsigned short AC4;
unsigned short AC5;
unsigned short AC6;
short B1;
short B2;
short MB;
short MC;
short MD;

long BMPUP = 0;
long BMPUT = 0;
float True_Temp=0;  		//实际温度 C
float True_Press=0;     //实际气压 Pa
float True_Altitude=0;  //实际高度 m

void BMP180_init(void)
{
	Read_CalibrationData(); //读取校准系数
}

short BMP180_ReadTwoByte(u8 ReadAddr)
{
		u8 date[2];
    short temp;     
		I2C1_Read_Len(BMP180_ADDR,ReadAddr,2,date);
		temp = date[0]<<8|date[1];
    return temp;                                                    
} 

// 读取校准系数
void Read_CalibrationData(void)
{
		
    AC1 = BMP180_ReadTwoByte(0xaa);
    AC2 = BMP180_ReadTwoByte(0xac);
    AC3 = BMP180_ReadTwoByte(0xae);
    AC4 = BMP180_ReadTwoByte(0xb0);
    AC5 = BMP180_ReadTwoByte(0xb2);
    AC6 = BMP180_ReadTwoByte(0xb4);
    B1 = BMP180_ReadTwoByte(0xb6);
    B2 = BMP180_ReadTwoByte(0xb8);
    MB = BMP180_ReadTwoByte(0xba);
    MC = BMP180_ReadTwoByte(0xbc);
    MD = BMP180_ReadTwoByte(0xbe); 	
}


// 读取没有经过补偿的压力值
long Get_BMP180UP(void)
{
    long UP=0; 
    I2C1_Write_One(BMP180_ADDR, 0xf4,0x34);       			//write 0x34 into reg 0xf4 
 //   delay_ms(10);                                    //wait 4.5ms
    UP = I2C1_Read_One(BMP180_ADDR, 0xf6); 
    UP &= 0x0000FFFF;	
    return UP;      
}

// 读取没有经过补偿的温度
long Get_BMP180UT(void)
{
    long UT;
 
    I2C1_Write_One(BMP180_ADDR,0xf4,0x2e);       //write 0x2E into reg 0xf4
//    delay_ms(10);                                   //wait 4.5ms
    UT = BMP180_ReadTwoByte(0xf6);          //read reg 0xF6(MSB),0xF7(LSB)
    
    return UT;
}


// 读取ID
unsigned char BMP180_Read_ID(void)
{
	return I2C1_Read_One(BMP180_ADDR, 0xd0); //读取ID
}



void Convert_UncompensatedToTrue(long UT,long UP)
{
    long X1,X2,X3,B3,B5,B6,B7,T,P;
    unsigned long B4;
     
    X1 = ((UT-AC6)*AC5)>>15;      //printf(\"X1:%ld \r\n\",X1);
    X2 = ((long)MC<<11)/(X1+MD);  //printf(\"X2:%ld \r\n\",X2);
    B5 = X1+X2;                        //printf(\"B5:%ld \r\n\",B5);
    T = (B5+8)>>4;                      //printf(\"T:%ld \r\n\",T);
    True_Temp = T/10.0;           // printf(\"Temperature:%.1f \r\n\",True_Temp);
 
    B6 = B5-4000;                       //printf(\"B6:%ld \r\n\",B6);
    X1 = (B2*B6*B6)>>23;              //printf(\"X1:%ld \r\n\",X1);
    X2 = (AC2*B6)>>11;                //printf(\"X2:%ld \r\n\",X2);
    X3 = X1+X2;                         //printf(\"X3:%ld \r\n\",X3);
    B3 = (((long)AC1*4+X3)+2)/4;    //printf(\"B3:%ld \r\n\",B3);
    X1 = (AC3*B6)>>13;                //printf(\"X1:%ld \r\n\",X1);
    X2 = (B1*(B6*B6>>12))>>16;      //printf(\"X2:%ld \r\n\",X2);
    X3 = ((X1+X2)+2)>>2;              //printf(\"X3:%ld \r\n\",X3);
    B4 = AC4*(unsigned long)(X3+32768)>>15;   //printf(\"B4:%lu \r\n\",B4);
    B7 = ((unsigned long)UP-B3)*50000;        //printf(\"B7:%lu \r\n\",B7);
    if (B7 < 0x80000000)
    {
        P = (B7*2)/B4;  
    }
    else P=(B7/B4)*2;                   //printf(\"P:%ld \r\n\",P);         
    X1 = (P/256.0)*(P/256.0);       //printf(\"X1:%ld \r\n\",X1);
    X1 = (X1*3038)>>16;               //printf(\"X1:%ld \r\n\",X1);
    X2 = (-7357*P)>>16;               //printf(\"X2:%ld \r\n\",X2);
    P = P+((X1+X2+3791)>>4);      //printf(\"P:%ld \r\n\",P);
    True_Press = P;               //  printf(\"Press:%.1fPa \r\n\",True_Press);
 
    True_Altitude = 44330*(1-pow((P/101325.0),(1.0/5.255)));            
    
		printf("True_Temp：%f	True_Press:%f\r\n",True_Temp,True_Press);
		
}

//读取补偿过的温度和压力
void BMP180_Read(void)
{

	BMPUT = Get_BMP180UT();
	
	BMPUP = Get_BMP180UP();
	
	
	Convert_UncompensatedToTrue(BMPUT,BMPUP);
}
