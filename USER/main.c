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
#include "can.h"
#include <string.h>

#define FID "FID,ADN23434S"  //飞机ID

 int main(void)
 {
	u8 can_send_buf[8] = {8};
	char sim_send_buf[50] = {8};
	int i = 0;
	
	delay_init();	    	 //延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	
	uart1_init(9600);//debug
	uart4_init(9600);//SIM
	uart5_init(9600);//GPS
	
	I2C1_Init(); //BMP180+6050
	SPI2_Init();
	CAN_Mode_Init(CAN_SJW_1tq,CAN_BS2_8tq,CAN_BS1_7tq,5,CAN_Mode_Normal);//波特率450Kbps
	TIM3_Init(4999,7199);//SIM 500ms 中断一次
	 
	printf("System Init\r\n");
	 
	LED_Init();
	BMP180_init();
	Rc522_Init();
	SIM_Init();
	
	printf("Devices Init\r\n");
		 
 	while(1)
	{
		
		if(LED_Flag)
		{
			LED_Flag = 0;
			LED = !LED;
		}
		
		Read_GPS();
		
		BMP180_Read();
		
		if(Main_Flag) //每隔 3S 上传一次数据
		{
			Main_Flag = 0;
			
			//FID
			strcpy(sim_send_buf,FID);
			Send_TCP_IP(FID);
			printf("%s\r\n",sim_send_buf);
			
			//大气压
			strcpy(sim_send_buf,"Pa,");
			sprintf(&sim_send_buf[3],"%f",True_Press);			
			Send_TCP_IP(sim_send_buf);
			printf("%s\r\n",sim_send_buf);

			//温度
			strcpy(sim_send_buf,"Temp,");
			sprintf(&sim_send_buf[3],"%f",True_Temp);			
			Send_TCP_IP(sim_send_buf);
			printf("%s\r\n",sim_send_buf);				

			//GPS
			printGpsBuffer();			
			
			//init ID Card
			Rc522_Init();		
		}
		
		if(Read_Card_ID()) //检测是否刷卡
		{
			Can_Send_Msg(can_send_buf,8);//通知遥控中转
			
			//UID 上传
			strcpy(sim_send_buf,"UID,");				
			for(i = 0; i < 6; i++)
			{
				sim_send_buf[4+i] = RFID_ID[i]+0x30;
			}
			sim_send_buf[10] = 0;				
			Send_TCP_IP(sim_send_buf);
			printf("%s\r\n",sim_send_buf);
			
		}


		
		
	}
	
 }

