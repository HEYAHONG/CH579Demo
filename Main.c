#include "CH57x_common.h"
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSPort.h"

/*
主程序
注意:由于CH579的某些寄存器的保护方式，硬件初始化应在RTOS未启动前或者临界区中执行。
*/

void main_task1(void *arg)
{
 
  while(1)
  {
    vTaskDelay(5);
  }
}

void main_task2(void *arg)
{
 
  while(1)
  {
     vTaskDelay(5);
  }
}

extern void HardwareInit(void);

extern void BLE_main(void *arg);

int main()
{  
		/*初始化硬件*/
		HardwareInit();

		/*创建任务*/
		xTaskCreate( main_task1, "main_task1",128, NULL, 1, NULL );
		xTaskCreate( main_task2, "main_task2",128, NULL, 1, NULL );
		
		xTaskCreate( BLE_main, "BLE_task",512, NULL, 1, NULL );//蓝牙任务
	
		FreeRTOS_Start();//启动FreeRTOS（更新时钟，启用调度器）
    while(1);    
}

void HardwareInit(void)
{
	SetSysClock(CLK_SOURCE_HSI_32MHz);
	
	{
		//UART1 Init
		GPIOA_SetBits( GPIO_Pin_9 );
		GPIOA_ModeCfg( GPIO_Pin_9, GPIO_ModeOut_PP_5mA );                    
		UART1_DefInit(); 
	}

}
