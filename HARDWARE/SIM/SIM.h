#ifndef __SIM_H__
#define __SIM_H__

char SIM_Init(void);

char Send_TCP_IP(char *date); 

void SIM_Read_IRQ(char data);

void SIM_TIME_IRQ(void);

void SIM_SendString(char *data);

#endif 
