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

extern int net_main(void);

void main_task2(void *arg)
{
  while(1)
  {
     vTaskDelay(5);
  }
}

extern void HardwareInit(void);

void  net_main_task(void *arg);//网络主任务

int main()
{  
		/*初始化硬件*/
		HardwareInit();

		/*创建任务*/
		xTaskCreate( main_task1, "main_task1",128, NULL, 1, NULL );
		xTaskCreate( main_task2, "main_task2",128, NULL, 1, NULL );
	
		xTaskCreate( net_main_task, "net_main_task",512, NULL, 1, NULL );//网络主任务占用堆栈较大	
	
		FreeRTOS_Start();//启动FreeRTOS（更新时钟，启用调度器）
    while(1);    
}

void HardwareInit(void)
{
	PWR_UnitModCfg(ENABLE, UNIT_SYS_PLL);                                      /* PLL上电 */
	
	SetSysClock(CLK_SOURCE_HSE_32MHz);                                          /* 外部晶振 PLL 输出32MHz */
	//SetSysClock(CLK_SOURCE_HSI_32MHz);//初始化时钟
	
	{
		//初始化串口,printf打印输出
		GPIOA_SetBits( GPIO_Pin_9 );
		GPIOA_ModeCfg( GPIO_Pin_9, GPIO_ModeOut_PP_5mA );                           /* 串口1的IO口设置 */
		UART1_DefInit(); //初始化UART1,默认打印
	}
	
}

void HardFault_Handler(void)
{
	while(1);
}
