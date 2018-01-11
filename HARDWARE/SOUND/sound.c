#include "sound.h"
#include "usart.h"

void sound_send(char *data)
{
	uart2_send(data);
}

//播放暂停
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

//指定文件夹播放

void sound_broadcast_files_file(char files, char file)
{
	char data[10] = {0};
	char len = 0;
	char sum = 0;
	int i = 0;
	len = 0;
	
	data[len++] = 0x7E;	//起始码
	
	data[len++] = 0x05;	//长度
	data[len++] = 0x42; //操作码
	data[len++] = files;	//文件夹号
	data[len++] = file;	//歌曲名
	
	sum = data[1]; 			//计算校验码
	for(i = 2; i<len; i++)
	{
		sum ^= data[i];
	}
	data[len++] = sum; //校验码
	
	data[len++] = 0xEF; //结束码
	
	
	sound_send(data);
}
