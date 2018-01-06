#ifndef __BMP180_H
#define __BMP180_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////
//本程序只供学习使用 
//////////////////////////////////////////////////////////////////////////////////
 
#define BMP180_ADDR 0x77


//typedef struct {  
//    short AC1 ;  
//    short AC2 ;  
//    short AC3 ;  
//    short AC4 ;  
//    short AC5 ;  
//    short AC6 ;  
//    short B1 ;  
//    short B2 ;  
//    short MB ;  
//    short MC ;  
//    short MD ;  
//}BMP180_param; 


//typedef struct {  
//    u8  ExistFlag ;  //存在标志
//  
//    BMP180_param  cal_param;//修正系数
//  
//    u8 Version ;               //版本
//  
//    u32 UnsetTemperature ;     //未校正温度值 
//    u32 UnsetGasPress    ;     //未校正气压值
//  
//    float Temperature ;         /*校正后温度*/  
//    u32 GasPress ;             /*校正后气压*/  
//  
//    float Altitude ;                /*海拔*/  
//      
//}BMP180_info ;


extern u8 BMP180_ID;          
extern float True_Temp;  		 //实际温度 C
extern float True_Press;     //实际气压 Pa
extern float True_Altitude;  //实际高度 m 

void BMP180_init(void);
unsigned char BMP180_Read_ID(void); // 读取ID
void Read_CalibrationData(void); //读取校准系数
long Get_BMP180UP(void);// 读取没有经过补偿的压力值
long Get_BMP180UT(void);// 读取没有经过补偿的温度
void BMP180_Read(void); //读取补偿过的温度和压力


	
#endif
