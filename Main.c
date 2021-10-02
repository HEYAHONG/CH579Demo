#include "CH57x_common.h"
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSPort.h"

/*
������
ע��:����CH579��ĳЩ�Ĵ����ı�����ʽ��Ӳ����ʼ��Ӧ��RTOSδ����ǰ�����ٽ�����ִ�С�
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
	SetSysClock(CLK_SOURCE_HSI_32MHz);
}