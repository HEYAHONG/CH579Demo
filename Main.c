#include "CH57x_common.h"
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSPort.h"

extern void BLE_init(void);
extern void BLE_loop(void);

/*
空闲映射
*/
void vApplicationIdleHook()
{
	BLE_loop();//使用空闲任务执行蓝牙循环,其他任务不可长时间占用CPU时间,否则蓝牙将断开连接。
}

/*
主程序
注意:由于CH579的某些寄存器的保护方式，硬件初始化应在RTOS未启动前或者临界区中执行。
*/

void main_task1(void *arg)
{
 
  while(1)
  {
    vTaskDelay(1000);
  }
}

void main_task2(void *arg)
{
 
  while(1)
  {
     vTaskDelay(1000);
  }
}

extern void HardwareInit(void);



int main()
{  
		/*初始化硬件*/
		HardwareInit();

		/*创建任务*/
		xTaskCreate( main_task1, "main_task1",128, NULL, 1, NULL );
		xTaskCreate( main_task2, "main_task2",128, NULL, 1, NULL );
			
		FreeRTOS_Start();//启动FreeRTOS（更新时钟，启用调度器）
    while(1);    
}

void HardwareInit(void)
{
	SetSysClock(CLK_SOURCE_HSE_32MHz);
	
	{
		//UART1 Init
		GPIOA_SetBits( GPIO_Pin_9 );
		GPIOA_ModeCfg( GPIO_Pin_9, GPIO_ModeOut_PP_5mA );                    
		UART1_DefInit(); 
	}
	
	BLE_init();

}
