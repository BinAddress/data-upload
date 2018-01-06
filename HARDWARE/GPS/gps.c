#include "gps.h"
#include "usart.h"	  
#include "delay.h"
#include "sim.h"
#include <string.h>

#define GPS_REC_LEN  			200  	//定义最大接收字节数 200

_SaveData Save_Data;

void parseGpsBuffer(void);
void printGpsBuffer(void);


u16 point1 = 0;
char USART_RX_BUF[GPS_REC_LEN];     //GPS接收缓冲

void GPS_Send_IRQ(char data)
{
		if(data == '$')
		{
			point1 = 0;	
		}
		
		USART_RX_BUF[point1++] = data;

		if(USART_RX_BUF[0] == '$' && USART_RX_BUF[4] == 'M' && USART_RX_BUF[5] == 'C')			//确定是否收到"GPRMC/GNRMC"这一帧数据
		{
			if(data == '\n')									   
			{
				memset(Save_Data.GPS_Buffer, 0, GPS_Buffer_Length);      //清空
				memcpy(Save_Data.GPS_Buffer, USART_RX_BUF, point1); 	//保存数据
				Save_Data.isGetData = true;
				point1 = 0;
				memset(USART_RX_BUF, 0, GPS_REC_LEN);      //清空				
			}	
					
		}
		
		if(point1 >= GPS_REC_LEN)
		{
			point1 = GPS_REC_LEN;
		}	
}

char Read_GPS(void)
{		
	parseGpsBuffer();
	
	//printGpsBuffer();
	
	return 0;
}

//分析缓存数据
void parseGpsBuffer(void) 
{
	char *subString;
	char *subStringNext;
	char i = 0;
	if (Save_Data.isGetData)
	{
		Save_Data.isGetData = false;

		for (i = 0 ; i <= 6 ; i++)
		{
			if (i == 0)
			{
				if ((subString = strstr(Save_Data.GPS_Buffer, ",")) == NULL)
				{
					//未找到 ","
				}
			}
			else
			{
				subString++;
				if ((subStringNext = strstr(subString, ",")) != NULL)
				{
					char usefullBuffer[2]; 
					switch(i)
					{
						case 1:memcpy(Save_Data.UTCTime, subString, subStringNext - subString);break;	//获取UTC时间
						case 2:memcpy(usefullBuffer, subString, subStringNext - subString);break;	//获取UTC时间
						case 3:memcpy(Save_Data.latitude, subString, subStringNext - subString);break;	//获取纬度信息
						case 4:memcpy(Save_Data.N_S, subString, subStringNext - subString);break;	//获取N/S
						case 5:memcpy(Save_Data.longitude, subString, subStringNext - subString);break;	//获取经度信息
						case 6:memcpy(Save_Data.E_W, subString, subStringNext - subString);break;	//获取E/W
						default:break;
					}

					subString = subStringNext;
					Save_Data.isParseData = true;
					if(usefullBuffer[0] == 'A')
						Save_Data.isUsefull = true;
					else if(usefullBuffer[0] == 'V')
						Save_Data.isUsefull = false;

				}
				else
				{
						//未找到 ","
				}
			}


		}
	}
}



char gpsbuf[30];

//打印分析完成的GPS数据
void printGpsBuffer(void)  
{
	if (Save_Data.isParseData)
	{
		Save_Data.isParseData = false;
		
		strcpy(gpsbuf,"UTCTime,");
		strcpy(&gpsbuf[8],Save_Data.UTCTime);
		Send_TCP_IP(gpsbuf);		
		printf("%s\r\n",gpsbuf);

		if(Save_Data.isUsefull)
		{
			Save_Data.isUsefull = false;
				
			strcpy(gpsbuf,"latitude,");
			strcpy(&gpsbuf[9],Save_Data.latitude);
			Send_TCP_IP(gpsbuf);
			printf("%s\r\n",gpsbuf);			

			strcpy(gpsbuf,"N_S,");
			strcpy(&gpsbuf[4],Save_Data.N_S);
			Send_TCP_IP(gpsbuf);
			printf("%s\r\n",gpsbuf);			
					
			strcpy(gpsbuf,"longitude,");
			strcpy(&gpsbuf[10],Save_Data.longitude);
			Send_TCP_IP(gpsbuf);
			printf("%s\r\n",gpsbuf);	
			
			strcpy(gpsbuf,"E_W,");
			strcpy(&gpsbuf[4],Save_Data.E_W);
			Send_TCP_IP(gpsbuf);
			printf("%s\r\n",gpsbuf);
			
		}
		else
		{
			printf("GPS DATA is not usefull!\r\n");
		}
		
	}
}

