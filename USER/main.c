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
#include "rc522.h"
#include <string.h>



 int main(void)
 {

	delay_init();	    	 //延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	
	 
	uart1_init(9600);//debug
	uart4_init(9600);//SIM
	//uart5_init(9600);//GPS
	
	I2C1_Init(); //BMP180+6050
	SPI2_Init();
	TIM3_Init(4999,7199);//SIM 500ms 中断一次
	 
	printf("System Init\r\n");
	 
	LED_Init();
	BMP180_init();
	Rc522_Init();
	//SIM_Init();
	
	printf("Devices Init\r\n");
		 
 	while(1)
	{

		//Read_GPS();
		
		//BMP180_Read();
		
		delay_ms(100);
		LED = !LED;
		
		Read_Card_ID();


		
		
	}
	
 }

