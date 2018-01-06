#include "sim.h"
#include "usart.h"
#include "delay.h"
#include "time.h"
#include <string.h>
#include "led.h"

#define SIM_Max 200 					  //SIM缓存长度
char SIM_Buf[SIM_Max] = {0};		
u8 SIM_First_Int;
static unsigned char tiems = 0;
static unsigned char tiems_s = 0;
static unsigned char tiems_flag = 0;

const char *ip_string = "AT+CIPSTART=\"TCP\",\"123.206.216.144\",1234\r\n";	//IP登录服务器

char Wait_CREG(void); //查询注册状态
void CLR_Buf2(void);
void Set_ATE0(void);
char Connect_Server(void);
char Second_AT_Command(char *b,char *a,u8 wait_time);//发送命令
char Second_AT_Command_3Ci(char *b,char *a,u8 wait_time,char num);//发送命令发送多次
void SIM_SendBit(char data);

void SIM_Read_IRQ(char data)
{
		//printf("%c",data);
	
		SIM_Buf[SIM_First_Int++] = data;  	  //将接收到的字符串存到缓存中
		if(SIM_First_Int >= SIM_Max)       		//如果缓存满,将缓存指针指向缓存的首地址
		{
			SIM_First_Int = 0;
		}
}

void SIM_Time_IRQ(void) 
{
	if(tiems_flag) //开始计数
	{
		if(++tiems >= 2)
		{
			tiems = 0;
			tiems_s++; //1s 累加1次
		}
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
		USART_SendData(UART4 ,data);//发送当前字符	
}


char SIM_Init(void)
{
	if(!Wait_CREG())//查询网络是否注册
	{
		printf("SIM 注册失败\r\n");
		return 0;
	}
		
	Set_ATE0(); //取消回显

	if(!Connect_Server())//连接服务器
	{
		printf("SIM 连接服务器失败\r\n");
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
						return 1; //注册成功
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
char Connect_Server(void)
{
	SIM_SendString("AT+CIPCLOSE=1");	//关闭连接
	
	if(!Second_AT_Command_3Ci("AT+CIPSHUT","SHUT OK",20,3))//关闭移动场景
	{		
		printf("SIM 关闭移动场景失败\r\n");
		return 0;
	}
	
	if(!Second_AT_Command_3Ci("AT+CGCLASS=\"B\"","OK",10,3))//设置GPRS移动台类别为B,支持包交换和数据交换 
	{
		printf("SIM 设置移动平台失败\r\n");
		return 0;
	}	
	
	if(!Second_AT_Command_3Ci("AT+CGDCONT=1,\"IP\",\"CMNET\"","OK",10,3))//设置PDP上下文,互联网接协议,接入点等信息
	{		
		printf("SIM 设置PDP上下文失败\r\n");
		return 0;
	}
	
	if(!Second_AT_Command_3Ci("AT+CGATT=1","OK",10,3))//附着GPRS业务
	{		
		printf("SIM 附着GPRS业务失败\r\n");
		return 0;
	}
	
	if(!Second_AT_Command_3Ci("AT+CIPCSGP=1,\"CMNET\"","OK",10,3))//设置为GPRS连接模式
	{		
		printf("SIM 设置为GPRS连接模式失败\r\n");
		return 0;
	}
	
	if(!Second_AT_Command_3Ci("AT+CIPHEAD=1","OK",10,3))//设置接收数据显示IP头(方便判断数据来源,仅在单路连接有效)
	{		
		printf("SIM 设置接收数据显示IP头失败\r\n");
		return 0;
	}
	
	if(!Second_AT_Command_3Ci((char*)ip_string,"OK",10,3)) //设置服务器IP
	{
		printf("SIM 设置服务器IP失败\r\n");
		return 0;
	}

	delay_ms(100);
	CLR_Buf2();
	return 1;
}

// 发送失败，则重新发送
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
	c = b;										//保存字符串地址到c
	CLR_Buf2(); 
  
	tiems_s = 0;
	tiems_flag = 0;
	while(1)                    
	{
		if(strstr(SIM_Buf,a)==NULL) //未找到
		{
				if(!tiems_flag) //如果没有开始计数
				{
					b = c;						
					SIM_SendString(b); //发送命令
					SIM_SendString("\r\n");
					
					tiems = 0;
					tiems_s = 0; //时间清零
					tiems_flag = 1;		//开始计数			
				}
				else
				{
					//已经开始计数
					if(tiems_s >= wait_time)
					{
						//超过时间
						//printf("time:%d\r\n",tiems_s);
						tiems_s = 0;
						tiems_flag = 0;
						CLR_Buf2(); 
						return 0;
					}
					else
					{
						//为超时
					}
				}
    }
 	  else
		{
			//printf("time:%d\r\n",tiems_s);
			tiems_s = 0;
			tiems_flag = 0;			
			CLR_Buf2(); 
			return 1; //命令完成
		}
	}

}








