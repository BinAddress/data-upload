#include "sim.h"
#include "usart.h"
#include "delay.h"
#include "time.h"
#include <string.h>


#define SIM_Max 200 					  //SIM���泤��
char SIM_Buf[SIM_Max] = {0};		
u8 SIM_First_Int;
const char *ip_string = "AT+CIPSTART=\"TCP\",\"123.206.216.144\",1234\r\n";	//IP��¼������
vu8 SIM_Timer_start;	//��ʱ��0��ʱ����������
u8 SIM_Times=0;
u8 SIM_shijian=0;

char Wait_CREG(void); //��ѯע��״̬
void CLR_Buf2(void);
void Set_ATE0(void);
void Connect_Server(void);
void Second_AT_Command(char *b,char *a,u8 wait_time);
u8 Find(char *a);
void SIM_SendBit(char data);


void SIM_Read_IRQ(char data)
{
		SIM_Buf[SIM_First_Int++] = data;  	  //�����յ����ַ����浽������
		if(SIM_First_Int >= SIM_Max)       		//���������,������ָ��ָ�򻺴���׵�ַ
		{
			SIM_First_Int = 0;
		}
}

void SIM_TIME_IRQ(void)
{
		if(SIM_Timer_start)
		SIM_Times++;
		if(SIM_Times > SIM_shijian)
		{
			SIM_Timer_start = 0;
			SIM_Times = 0;
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
		USART_SendData(UART4 ,data++);//���͵�ǰ�ַ�	
}


char SIM_Init(void)
{
	if(!Wait_CREG())//��ѯ�����Ƿ�ע��
	{
		printf("SIM ע��ʧ��\r\n");
		return 0;
	}
		
	Set_ATE0(); //ȡ������

	//Connect_Server();//���ӷ�����
	
	//Second_AT_Command("AT+CIPSEND",">",2); //set send modo

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
	
	i = 0;
  while(i < 30)        			
	{
		CLR_Buf2();         
		SIM_SendString("AT+CREG?\r\n");
		delay_ms(1000);  	
		
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
					//δ�ҵ� 1 �� 5
				}
			}
			else
			{
			 //δ�ҵ� ��
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
void Connect_Server(void)
{
	SIM_SendString("AT+CIPCLOSE=1");	//�ر�����
  delay_ms(100);
	Second_AT_Command("AT+CIPSHUT","SHUT OK",2);		//�ر��ƶ�����
	Second_AT_Command("AT+CGCLASS=\"B\"","OK",2);//����GPRS�ƶ�̨���ΪB,֧�ְ����������ݽ��� 
	Second_AT_Command("AT+CGDCONT=1,\"IP\",\"CMNET\"","OK",2);//����PDP������,��������Э��,��������Ϣ
	Second_AT_Command("AT+CGATT=1","OK",2);//����GPRSҵ��
	Second_AT_Command("AT+CIPCSGP=1,\"CMNET\"","OK",2);//����ΪGPRS����ģʽ
	Second_AT_Command("AT+CIPHEAD=1","OK",2);//���ý���������ʾIPͷ(�����ж�������Դ,���ڵ�·������Ч)
	Second_AT_Command((char*)ip_string,"OK",5);
	delay_ms(100);
	CLR_Buf2();
}


/*******************************************************************************
* ������ : Second_AT_Command
* ����   : ����ATָ���
* ����   : �������ݵ�ָ�롢���͵ȴ�ʱ��(��λ��S)
* ���   : 
* ����   : 
* ע��   : 
*******************************************************************************/

void Second_AT_Command(char *b,char *a,u8 wait_time)         
{
	u8 i;
	char *c;
	c = b;										//�����ַ�����ַ��c
	CLR_Buf2(); 
  i = 0;
	while(i == 0)                    
	{
		if(!Find(a)) 
		{
			if(SIM_Timer_start == 0)
			{
				b = c;							//���ַ�����ַ��b
				for (; *b!='\0';b++)
				{
					SIM_SendBit(*b);//UART2_SendData(*b);
				}
				
				SIM_SendString("\r\n");	
				SIM_Times = 0;
				SIM_shijian = wait_time;
				SIM_Timer_start = 1;
		   }
    }
 	  else
		{
			i = 1;
			SIM_Timer_start = 0;
		}
	}
	
	CLR_Buf2(); 
}

/*******************************************************************************
* ������ : Find
* ����   : �жϻ������Ƿ���ָ�����ַ���
* ����   : 
* ���   : 
* ����   : unsigned char:1 �ҵ�ָ���ַ���0 δ�ҵ�ָ���ַ� 
* ע��   : 
*******************************************************************************/

u8 Find(char *a)
{ 
  if(strstr(SIM_Buf,a)!=NULL)
	    return 1;
	else
			return 0;
}






