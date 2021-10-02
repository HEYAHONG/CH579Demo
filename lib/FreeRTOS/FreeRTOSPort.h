#ifndef FREERTOS__H
#define FREERTOS__H

#include "stdint.h"
#include "stdlib.h"

extern uint32_t SystemCoreClock;

//CLK�����仯ʱ��Ҫ����
void FreeRTOS_CLK_Update(void);

//FreeRTOS������������װ����������²��᷵��
void FreeRTOS_Start(void);

#endif
