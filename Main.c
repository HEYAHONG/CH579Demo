#include "CH57x_common.h"
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSPort.h"
#include "lwip/init.h"
#include "sys/socket.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#if !NO_SYS
#include "lwip/tcpip.h"
#endif
#include "ETH.h"

/*
主程序
注意:由于CH579的某些寄存器的保护方式，硬件初始化应在RTOS未启动前或者临界区中执行。
*/

void main_task1(void *arg)
{
 
  while(1)
  {
		ETH_CheckState();//以太网卡检查状态
    vTaskDelay(1);
  }
}



void main_task2(void *arg)
{
  vTaskDelay(3000);
	{
			ip4_addr_t dns;
			ip4addr_aton("172.18.0.1",&dns);
			dns_setserver(0,&dns);
	}
  while(1)
  {
		vTaskDelay(3000);
  }
}

extern void HardwareInit(void);

int main()
{ 
			
		/*初始化硬件*/
		HardwareInit();
	
		//初始化LWIP协议栈
	  tcpip_init(NULL,NULL);
	
		//初始化ETH
		ETH_Init();
	
		/*创建任务*/
		xTaskCreate( main_task1, "main_task1",128, NULL, 2, NULL );
		xTaskCreate( main_task2, "main_task2",128, NULL, 2, NULL );
	
	
		FreeRTOS_Start();//启动FreeRTOS（更新时钟，启用调度器）
    while(1);    
}

void DebugInit(void)		
{
    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
}

void HardwareInit(void)
{
	//初始化时钟
	SetSysClock(CLK_SOURCE_HSI_32MHz);
	
	DebugInit();
	
	
	
}

void HardFault_Handler(void)
{
	printf("HardFault \r\n");
	while(1)
	{
	
	}
}

