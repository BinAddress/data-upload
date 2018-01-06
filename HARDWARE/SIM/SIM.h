#ifndef __SIM_H__
#define __SIM_H__

char SIM_Init(void);

char Send_TCP_IP(char *date); 

void SIM_Read_IRQ(char data); //串口中断

void SIM_Time_IRQ(void); //定时器中断

void SIM_SendString(char *data); //发送串口指令

#endif 
