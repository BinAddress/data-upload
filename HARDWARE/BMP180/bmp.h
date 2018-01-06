#ifndef __BMP180_H
#define __BMP180_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////
//������ֻ��ѧϰʹ�� 
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
//    u8  ExistFlag ;  //���ڱ�־
//  
//    BMP180_param  cal_param;//����ϵ��
//  
//    u8 Version ;               //�汾
//  
//    u32 UnsetTemperature ;     //δУ���¶�ֵ 
//    u32 UnsetGasPress    ;     //δУ����ѹֵ
//  
//    float Temperature ;         /*У�����¶�*/  
//    u32 GasPress ;             /*У������ѹ*/  
//  
//    float Altitude ;                /*����*/  
//      
//}BMP180_info ;


extern u8 BMP180_ID;          
extern float True_Temp;  		 //ʵ���¶� C
extern float True_Press;     //ʵ����ѹ Pa
extern float True_Altitude;  //ʵ�ʸ߶� m 

void BMP180_init(void);
unsigned char BMP180_Read_ID(void); // ��ȡID
void Read_CalibrationData(void); //��ȡУ׼ϵ��
long Get_BMP180UP(void);// ��ȡû�о���������ѹ��ֵ
long Get_BMP180UT(void);// ��ȡû�о����������¶�
void BMP180_Read(void); //��ȡ���������¶Ⱥ�ѹ��


	
#endif
