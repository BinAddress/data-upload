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

#define FID "FID,ADN23434S"  //�ɻ�ID

 int main(void)
 {
	u8 can_send_buf[8] = {8};
	char sim_send_buf[50] = {8};
	int i = 0;
	
	delay_init();	    	 //��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	
	uart1_init(9600);//debug
	uart4_init(9600);//SIM
	uart5_init(9600);//GPS
	
	I2C1_Init(); //BMP180+6050
	SPI2_Init();
	CAN_Mode_Init(CAN_SJW_1tq,CAN_BS2_8tq,CAN_BS1_7tq,5,CAN_Mode_Normal);//������450Kbps
	TIM3_Init(4999,7199);//SIM 500ms �ж�һ��
	 
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
		
		if(Main_Flag) //ÿ�� 3S �ϴ�һ������
		{
			Main_Flag = 0;
			
			//FID
			strcpy(sim_send_buf,FID);
			Send_TCP_IP(FID);
			printf("%s\r\n",sim_send_buf);
			
			//����ѹ
			strcpy(sim_send_buf,"Pa,");
			sprintf(&sim_send_buf[3],"%f",True_Press);			
			Send_TCP_IP(sim_send_buf);
			printf("%s\r\n",sim_send_buf);

			//�¶�
			strcpy(sim_send_buf,"Temp,");
			sprintf(&sim_send_buf[3],"%f",True_Temp);			
			Send_TCP_IP(sim_send_buf);
			printf("%s\r\n",sim_send_buf);				

			//GPS
			printGpsBuffer();			
			
			//init ID Card
			Rc522_Init();		
		}
		
		if(Read_Card_ID()) //����Ƿ�ˢ��
		{
			Can_Send_Msg(can_send_buf,8);//֪ͨң����ת
			
			//UID �ϴ�
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

