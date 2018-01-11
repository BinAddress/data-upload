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
#include "mpu6050.h"
#include "sound.h"
#include <string.h>

#define FID "FID,ADN23434S"  //�ɻ�ID

u8 can_send_buf[10] = {0};

void Data_OK(void);
	
 int main(void)
 {
	char sim_send_buf[50] = {0};
	int i = 0;
	char user_card_flag = 0;
	char user_card_start_flag = 0;
	
	user_card_flag = 0;
	
	delay_init();	    	 //��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	
	uart1_init(9600);//debug
	uart2_init(9600);//����ģ��
	uart4_init(9600);//SIM
	uart5_init(9600);//GPS
	
	I2C1_Init(); //BMP180+6050
	SPI2_Init();
	CAN_Mode_Init(CAN_SJW_1tq,CAN_BS2_3tq,CAN_BS1_8tq,3,CAN_Mode_Normal);//CAN��ͨģʽ��ʼ��, ������500Kbps 
	TIM3_Init(4999,7199);//SIM 500ms �ж�һ��
	
	sound_broadcast_files_file(0x01,0x01);	
	
	printf("System Init\r\n");
	LED_Init();
	Rc522_Init();
	
	delay_ms(1000); //�ȴ�������ȡ���
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
		
	sound_broadcast_files_file(0x01,0x02);

	while(1) //����û��Ƿ�ˢ��
	{
		if(Read_Card_ID())
		{
			Data_OK();//����ң������������
			
			user_card_flag = 1;
			user_card_start_flag = 1;
			
			//UID ���ڴ�ӡ
			strcpy(sim_send_buf,"UID,");				
			for(i = 0; i < 6; i++)
			{
				sim_send_buf[4+i] = RFID_ID[i]+0x30;
			}
			sim_send_buf[10] = 0;

			PcdAntennaOff();	//�ر�����		
			
			sound_broadcast_files_file(0x01,0x03);
			printf("%s\r\n",sim_send_buf);
			
			break;
		}
		
		if(Main_Flag) //ÿ�� 3S ��ʼ��
		{
			Main_Flag = 0;
			LED = !LED;
			Rc522_Init();
			if(user_card_start_flag)
			{
				PcdAntennaOff();	//�ر�����	
			}
		}
		
	}
	
	SIM_Init();
	BMP180_init();
	//	MPU_Init();
	printf("Devices Init\r\n");
	
 	while(1) //�����ϴ�
	{
		
		if(LED_Flag)
		{
			LED_Flag = 0;
			LED = !LED;
		}
		
//		MPU6050_Read();
		
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
			sprintf(&sim_send_buf[5],"%f",True_Temp);			
			Send_TCP_IP(sim_send_buf);
			printf("%s\r\n",sim_send_buf);		

			//GPS
			printGpsBuffer();	

			if(user_card_flag)
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

//			//Roll
//			strcpy(sim_send_buf,"Roll,");
//			sprintf(&sim_send_buf[5],"%f",Roll);			
//			Send_TCP_IP(sim_send_buf);
//			printf("%s\r\n",sim_send_buf);

//			//Pitch
//			strcpy(sim_send_buf,"Pitch,");
//			sprintf(&sim_send_buf[6],"%f",Pitch);			
//			Send_TCP_IP(sim_send_buf);
//			printf("%s\r\n",sim_send_buf);

//			//Yaw
//			strcpy(sim_send_buf,"Yaw,");
//			sprintf(&sim_send_buf[4],"%f",Yaw);			
//			Send_TCP_IP(sim_send_buf);
//			printf("%s\r\n",sim_send_buf);
			
			//init ID Card
//			Rc522_Init();		
		}
		
//		//����Ƿ�ˢ��
//		if(Read_Card_ID() || user_card_start_flag) 
//		{
//			user_card_start_flag = 0;
//			
//			Data_OK();//����ң������������
//			
//			user_card_flag = 1;
//			
//			//UID �ϴ�
//			strcpy(sim_send_buf,"UID,");				
//			for(i = 0; i < 6; i++)
//			{
//				sim_send_buf[4+i] = RFID_ID[i]+0x30;
//			}
//			sim_send_buf[10] = 0;				
//			Send_TCP_IP(sim_send_buf);
//			printf("%s\r\n",sim_send_buf);
//			
//		}
		
	}
	
 }

 //����ң������������
 void Data_OK(void)
 {
		

		can_send_buf[0] = 0xFF;
		can_send_buf[1] = 0xAA;
		can_send_buf[2] = 0xBB;
		can_send_buf[3] = 0x00;
		can_send_buf[4] = 0x00;
		can_send_buf[5] = 0x00;
		can_send_buf[6] = 0x00;
		can_send_buf[7] = 0xFD;

		Can_Send_Msg(can_send_buf,8);//֪ͨң����ת
 }
