#include "sound.h"
#include "usart.h"

void sound_send(char *data)
{
	uart2_send(data);
}

//������ͣ
void sound_broadcast(void)
{
	char data[10] = {0};
	data[0] = 0x7E;
	data[1] = 0x03;
	data[2] = 0x11;
	data[3] = 0x12;
	data[4] = 0xEF;
	data[5] = 0x00;
	sound_send(data);
}

//ָ���ļ��в���

void sound_broadcast_files_file(char files, char file)
{
	char data[10] = {0};
	char len = 0;
	char sum = 0;
	int i = 0;
	len = 0;
	
	data[len++] = 0x7E;	//��ʼ��
	
	data[len++] = 0x05;	//����
	data[len++] = 0x42; //������
	data[len++] = files;	//�ļ��к�
	data[len++] = file;	//������
	
	sum = data[1]; 			//����У����
	for(i = 2; i<len; i++)
	{
		sum ^= data[i];
	}
	data[len++] = sum; //У����
	
	data[len++] = 0xEF; //������
	
	
	sound_send(data);
}
