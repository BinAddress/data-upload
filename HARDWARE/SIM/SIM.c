#include "sim.h"
#include "usart.h"
#include "delay.h"
#include "time.h"
#include <string.h>
#include "led.h"

#define SIM_Max 200 					  //SIM���泤��
char SIM_Buf[SIM_Max] = {0};		
u8 SIM_First_Int;
static unsigned char tiems = 0;
static unsigned char tiems_s = 0;
static unsigned char tiems_flag = 0;

const char *ip_string = "AT+CIPSTART=\"TCP\",\"123.206.216.144\",1234\r\n";	//IP��¼������

char Wait_CREG(void); //��ѯע��״̬
void CLR_Buf2(void);
void Set_ATE0(void);
char Connect_Server(void);
char Second_AT_Command(char *b,char *a,u8 wait_time);//��������
char Second_AT_Command_3Ci(char *b,char *a,u8 wait_time,char num);//��������Ͷ��
void SIM_SendBit(char data);

void SIM_Read_IRQ(char data)
{
		//printf("%c",data);
	
		SIM_Buf[SIM_First_Int++] = data;  	  //�����յ����ַ����浽������
		if(SIM_First_Int >= SIM_Max)       		//���������,������ָ��ָ�򻺴���׵�ַ
		{
			SIM_First_Int = 0;
		}
}

void SIM_Time_IRQ(void) 
{
	if(tiems_flag) //��ʼ����
	{
		if(++tiems >= 2)
		{
			tiems = 0;
			tiems_s++; //1s �ۼ�1��
		}
	}
}


void SIM_SendString(char *data)
{
	while(*data)
	{
		while(USART_GetFlagStatus(UART4, USART_FLAG_TC)==RESET); 
		USART_SendData(UART4 ,*data++);//���͵�ǰ�ַ�	
	}
}

void SIM_SendBit(char data)
{
		while(USART_GetFlagStatus(UART4, USART_FLAG_TC)==RESET); 
		USART_SendData(UART4 ,data);//���͵�ǰ�ַ�	
}


char SIM_Init(void)
{
	if(!Wait_CREG())//��ѯ�����Ƿ�ע��
	{
		printf("SIM ע��ʧ��\r\n");
		return 0;
	}
		
	Set_ATE0(); //ȡ������

	if(!Connect_Server())//���ӷ�����
	{
		printf("SIM ���ӷ�����ʧ��\r\n");
		return 0;
	}
	
	Second_AT_Command("AT+CIPSEND",">",2); //set send modo

	return 0xff;
}



char Send_TCP_IP(char *date)
{
	
	delay_ms(1000);
	Second_AT_Command("AT+CIPSEND",">",2); //set send modo
	SIM_SendString(date);	
	SIM_SendString("\32\0");	
	
	return 0;
}

/*******************************************************************************
* ������ : Wait_CREG
* ����   : �ȴ�ģ��ע��ɹ�
* ����   : 
* ���   : 
* ����   : ע��ɹ����� 1  ���򷵻�0
* ע��   : 
*******************************************************************************/
char Wait_CREG(void)
{
	u8 i = 0;
	u8 k;
	CLR_Buf2(); //��ջ���	        
	SIM_SendString("AT+CREG?\r\n");
	delay_ms(500);
	
	i = 0;
  while(i < 30)        			
	{
		for(k=0;k<SIM_Max;k++)      			
		{
			if(SIM_Buf[k] == ':')
			{
				if((SIM_Buf[k+4] == '1')||(SIM_Buf[k+4] == '5'))
				{
						return 1; //ע��ɹ�
				}
				else
				{
					if(k >= SIM_Max-1)
					{       
						SIM_SendString("AT+CREG?\r\n");
						delay_ms(500);	
					}
				}
			}
			else
			{
				if(k >= SIM_Max-1)
				{       
					SIM_SendString("AT+CREG?\r\n");
					delay_ms(500);				
				}
			}
		}
		
		i++;
	}
	
	return 0; //��ʱ�˳�
}

/*******************************************************************************
* ������ : CLR_Buf2
* ����   : �����������
* ����   : 
* ���   : 
* ����   : 
* ע��   : 
*******************************************************************************/
void CLR_Buf2(void)
{
	u16 k;
	for(k=0;k<SIM_Max;k++)      //��������������
	{
		SIM_Buf[k] = 0x00;
	}
    SIM_First_Int = 0;              //�����ַ�������ʼ�洢λ��
}

/*******************************************************************************
* ������ : Set_ATE0
* ����   : ȡ������
* ����   : 
* ���   : 
* ����   : 
* ע��   : 
*******************************************************************************/
void Set_ATE0(void)
{
	Second_AT_Command("ATE0","OK",3);								//ȡ������		
}

/*******************************************************************************
* ������ : Connect_Server
* ����   : GPRS���ӷ���������
* ����   : 
* ���   : 
* ����   : 
* ע��   : 
*******************************************************************************/
char Connect_Server(void)
{
	SIM_SendString("AT+CIPCLOSE=1");	//�ر�����
	
	if(!Second_AT_Command_3Ci("AT+CIPSHUT","SHUT OK",20,3))//�ر��ƶ�����
	{		
		printf("SIM �ر��ƶ�����ʧ��\r\n");
		return 0;
	}
	
	if(!Second_AT_Command_3Ci("AT+CGCLASS=\"B\"","OK",10,3))//����GPRS�ƶ�̨���ΪB,֧�ְ����������ݽ��� 
	{
		printf("SIM �����ƶ�ƽ̨ʧ��\r\n");
		return 0;
	}	
	
	if(!Second_AT_Command_3Ci("AT+CGDCONT=1,\"IP\",\"CMNET\"","OK",10,3))//����PDP������,��������Э��,��������Ϣ
	{		
		printf("SIM ����PDP������ʧ��\r\n");
		return 0;
	}
	
	if(!Second_AT_Command_3Ci("AT+CGATT=1","OK",10,3))//����GPRSҵ��
	{		
		printf("SIM ����GPRSҵ��ʧ��\r\n");
		return 0;
	}
	
	if(!Second_AT_Command_3Ci("AT+CIPCSGP=1,\"CMNET\"","OK",10,3))//����ΪGPRS����ģʽ
	{		
		printf("SIM ����ΪGPRS����ģʽʧ��\r\n");
		return 0;
	}
	
	if(!Second_AT_Command_3Ci("AT+CIPHEAD=1","OK",10,3))//���ý���������ʾIPͷ(�����ж�������Դ,���ڵ�·������Ч)
	{		
		printf("SIM ���ý���������ʾIPͷʧ��\r\n");
		return 0;
	}
	
	if(!Second_AT_Command_3Ci((char*)ip_string,"OK",10,3)) //���÷�����IP
	{
		printf("SIM ���÷�����IPʧ��\r\n");
		return 0;
	}

	delay_ms(100);
	CLR_Buf2();
	return 1;
}

// ����ʧ�ܣ������·���
char Second_AT_Command_3Ci(char *b,char *a,u8 wait_time,char num)     
{
	char i;
	
	i = 0;
	while(i <= num)
	{
		delay_ms(100);
		if(Second_AT_Command(b,a,wait_time))
		{
			return 1;
		}		
		i++;
	}
	
	return 0;
}

char Second_AT_Command(char *b,char *a,u8 wait_time)         
{

	char *c;
	c = b;										//�����ַ�����ַ��c
	CLR_Buf2(); 
  
	tiems_s = 0;
	tiems_flag = 0;
	while(1)                    
	{
		if(strstr(SIM_Buf,a)==NULL) //δ�ҵ�
		{
				if(!tiems_flag) //���û�п�ʼ����
				{
					b = c;						
					SIM_SendString(b); //��������
					SIM_SendString("\r\n");
					
					tiems = 0;
					tiems_s = 0; //ʱ������
					tiems_flag = 1;		//��ʼ����			
				}
				else
				{
					//�Ѿ���ʼ����
					if(tiems_s >= wait_time)
					{
						//����ʱ��
						//printf("time:%d\r\n",tiems_s);
						tiems_s = 0;
						tiems_flag = 0;
						CLR_Buf2(); 
						return 0;
					}
					else
					{
						//Ϊ��ʱ
					}
				}
    }
 	  else
		{
			//printf("time:%d\r\n",tiems_s);
			tiems_s = 0;
			tiems_flag = 0;			
			CLR_Buf2(); 
			return 1; //�������
		}
	}

}








