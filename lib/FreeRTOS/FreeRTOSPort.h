#ifndef FREERTOS__H
#define FREERTOS__H

#include "stdint.h"
#include "stdlib.h"

extern uint32_t SystemCoreClock;

//CLK发生变化时需要调用
void FreeRTOS_CLK_Update(void);

//FreeRTOS启动调度器封装，正常情况下不会返回
void FreeRTOS_Start(void);

#endif
