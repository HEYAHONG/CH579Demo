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
������
ע��:����CH579��ĳЩ�Ĵ����ı�����ʽ��Ӳ����ʼ��Ӧ��RTOSδ����ǰ�����ٽ�����ִ�С�
*/

void main_task1(void *arg)
{
 
  while(1)
  {
		ETH_CheckState();//��̫�������״̬
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
			
		/*��ʼ��Ӳ��*/
		HardwareInit();
	
		//��ʼ��LWIPЭ��ջ
	  tcpip_init(NULL,NULL);
	
		//��ʼ��ETH
		ETH_Init();
	
		/*��������*/
		xTaskCreate( main_task1, "main_task1",128, NULL, 2, NULL );
		xTaskCreate( main_task2, "main_task2",128, NULL, 2, NULL );
	
	
		FreeRTOS_Start();//����FreeRTOS������ʱ�ӣ����õ�������
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
	//��ʼ��ʱ��
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

