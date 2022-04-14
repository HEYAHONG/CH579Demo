#include "CH57x_common.h"
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSPort.h"

extern void BLE_init(void);
extern void BLE_loop(void);

/*
����ӳ��
*/
void vApplicationIdleHook()
{
	BLE_loop();//ʹ�ÿ�������ִ������ѭ��,�������񲻿ɳ�ʱ��ռ��CPUʱ��,�����������Ͽ����ӡ�
}

/*
������
ע��:����CH579��ĳЩ�Ĵ����ı�����ʽ��Ӳ����ʼ��Ӧ��RTOSδ����ǰ�����ٽ�����ִ�С�
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
		/*��ʼ��Ӳ��*/
		HardwareInit();

		/*��������*/
		xTaskCreate( main_task1, "main_task1",128, NULL, 1, NULL );
		xTaskCreate( main_task2, "main_task2",128, NULL, 1, NULL );
			
		FreeRTOS_Start();//����FreeRTOS������ʱ�ӣ����õ�������
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
