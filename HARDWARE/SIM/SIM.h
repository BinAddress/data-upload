#ifndef __SIM_H__
#define __SIM_H__

char SIM_Init(void);

char Send_TCP_IP(char *date); 

void SIM_Read_IRQ(char data); //�����ж�

void SIM_Time_IRQ(void); //��ʱ���ж�

void SIM_SendString(char *data); //���ʹ���ָ��

#endif 
