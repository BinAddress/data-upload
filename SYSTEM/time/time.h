#ifndef __TIME_H__
#define __TIME_H__

#include "sys.h"

extern char Main_Flag;
extern char LED_Flag;

void Main_Time(void);

void TIM3_Init(u16 arr,u16 psc);

#endif
