#include "sim.h"
#include "usart.h"
#include "delay.h"
#include "time.h"
#include <string.h>


#define SIM_Max 200 					  //SIM缓存长度
char SIM_Buf[SIM_Max] = {0};		
u8 SIM_First_Int;
const char *ip_string = "AT+CIPSTART=\"TCP\",\"123.206.216.144\",1234\r\n";	//IP登录服务器
vu8 SIM_Timer_start;	//定时器0延时启动计数器
u8 SIM_Times=0;
u8 SIM_shijian=0;

char Wait_CREG(void); //查询注册状态
void CLR_Buf2(void);
void Set_ATE0(void);
void Connect_Server(void);
void Second_AT_Command(char *b,char *a,u8 wait_time);
u8 Find(char *a);
void SIM_SendBit(char data);


void SIM_Read_IRQ(char data)
{
		SIM_Buf[SIM_First_Int++] = data;  	  //将接收到的字符串存到缓存中
		if(SIM_First_Int >= SIM_Max)       		//如果缓存满,将缓存指针指向缓存的首地址
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
		USART_SendData(UART4 ,*data++);//发送当前字符	
	}
}

void SIM_SendBit(char data)
{
		while(USART_GetFlagStatus(UART4, USART_FLAG_TC)==RESET); 
		USART_SendData(UART4 ,data++);//发送当前字符	
}


char SIM_Init(void)
{
	if(!Wait_CREG())//查询网络是否注册
	{
		printf("SIM 注册失败\r\n");
		return 0;
	}
		
	Set_ATE0(); //取消回显

	//Connect_Server();//连接服务器
	
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
* 函数名 : Wait_CREG
* 描述   : 等待模块注册成功
* 输入   : 
* 输出   : 
* 返回   : 注册成功返回 1  否则返回0
* 注意   : 
*******************************************************************************/
char Wait_CREG(void)
{
	u8 i = 0;
	u8 k;
	CLR_Buf2(); //清空缓存
	
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
						return 1; //注册成功
				}
				else
				{
					//未找到 1 或 5
				}
			}
			else
			{
			 //未找到 ：
			}
		}
		
		i++;
	}
	
	return 0; //超时退出
}

/*******************************************************************************
* 函数名 : CLR_Buf2
* 描述   : 清除缓存数据
* 输入   : 
* 输出   : 
* 返回   : 
* 注意   : 
*******************************************************************************/
void CLR_Buf2(void)
{
	u16 k;
	for(k=0;k<SIM_Max;k++)      //将缓存内容清零
	{
		SIM_Buf[k] = 0x00;
	}
    SIM_First_Int = 0;              //接收字符串的起始存储位置
}

/*******************************************************************************
* 函数名 : Set_ATE0
* 描述   : 取消回显
* 输入   : 
* 输出   : 
* 返回   : 
* 注意   : 
*******************************************************************************/
void Set_ATE0(void)
{
	Second_AT_Command("ATE0","OK",3);								//取消回显		
}

/*******************************************************************************
* 函数名 : Connect_Server
* 描述   : GPRS连接服务器函数
* 输入   : 
* 输出   : 
* 返回   : 
* 注意   : 
*******************************************************************************/
void Connect_Server(void)
{
	SIM_SendString("AT+CIPCLOSE=1");	//关闭连接
  delay_ms(100);
	Second_AT_Command("AT+CIPSHUT","SHUT OK",2);		//关闭移动场景
	Second_AT_Command("AT+CGCLASS=\"B\"","OK",2);//设置GPRS移动台类别为B,支持包交换和数据交换 
	Second_AT_Command("AT+CGDCONT=1,\"IP\",\"CMNET\"","OK",2);//设置PDP上下文,互联网接协议,接入点等信息
	Second_AT_Command("AT+CGATT=1","OK",2);//附着GPRS业务
	Second_AT_Command("AT+CIPCSGP=1,\"CMNET\"","OK",2);//设置为GPRS连接模式
	Second_AT_Command("AT+CIPHEAD=1","OK",2);//设置接收数据显示IP头(方便判断数据来源,仅在单路连接有效)
	Second_AT_Command((char*)ip_string,"OK",5);
	delay_ms(100);
	CLR_Buf2();
}


/*******************************************************************************
* 函数名 : Second_AT_Command
* 描述   : 发送AT指令函数
* 输入   : 发送数据的指针、发送等待时间(单位：S)
* 输出   : 
* 返回   : 
* 注意   : 
*******************************************************************************/

void Second_AT_Command(char *b,char *a,u8 wait_time)         
{
	u8 i;
	char *c;
	c = b;										//保存字符串地址到c
	CLR_Buf2(); 
  i = 0;
	while(i == 0)                    
	{
		if(!Find(a)) 
		{
			if(SIM_Timer_start == 0)
			{
				b = c;							//将字符串地址给b
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
* 函数名 : Find
* 描述   : 判断缓存中是否含有指定的字符串
* 输入   : 
* 输出   : 
* 返回   : unsigned char:1 找到指定字符，0 未找到指定字符 
* 注意   : 
*******************************************************************************/

u8 Find(char *a)
{ 
  if(strstr(SIM_Buf,a)!=NULL)
	    return 1;
	else
			return 0;
}






