#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"
#include "gps.h"
#include "iic.h"
#include "bmp.h"
#include "sim.h"
#include "time.h"

 int main(void)
 {
	delay_init();	    	 //��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	LED_Init();
	 
	uart1_init(9600);//debug
	uart4_init(9600);//SIM

	//uart5_init(9600);//GPS
	
	//I2C1_Init(); //BMP180+6050
	
	TIM3_Init(4999,7199);//SIM 500ms �ж�һ��
	 
	printf("System Init\r\n");
	
	SIM_Init();
	//BMP180_init();
	 
	
	
	printf("Devices Init\r\n");
		 
 	while(1)
	{

		//Read_GPS();
		
		//BMP180_Read();
		
		delay_ms(500);
		LED = !LED;
		
	}
	
 }

