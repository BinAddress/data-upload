#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"
#include "gps.h"
#include "iic.h"
#include "bmp.h"

 int main(void)
 {
	delay_init();	    	 //��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	
	uart1_init(9600);//debug
	uart4_init(9600);//SIM
	uart5_init(9600);//GPS
	
	I2C1_Init();
	
	printf("System Init\r\n");
	
	BMP180_init();
	 
	printf("Devices Init\r\n");

 	while(1)
	{

		//Read_GPS();
		
		BMP180_Read();
		
		//delay_ms(500);
		
	}
	
 }

