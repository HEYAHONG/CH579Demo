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

extern int net_main(void);

void main_task2(void *arg)
{
  while(1)
  {
     vTaskDelay(5);
  }
}

extern void HardwareInit(void);

void  net_main_task(void *arg);//����������

int main()
{  
		/*��ʼ��Ӳ��*/
		HardwareInit();

		/*��������*/
		xTaskCreate( main_task1, "main_task1",128, NULL, 1, NULL );
		xTaskCreate( main_task2, "main_task2",128, NULL, 1, NULL );
	
		xTaskCreate( net_main_task, "net_main_task",512, NULL, 1, NULL );//����������ռ�ö�ջ�ϴ�	
	
		FreeRTOS_Start();//����FreeRTOS������ʱ�ӣ����õ�������
    while(1);    
}

void HardwareInit(void)
{
	PWR_UnitModCfg(ENABLE, UNIT_SYS_PLL);                                      /* PLL�ϵ� */
	
	SetSysClock(CLK_SOURCE_HSE_32MHz);                                          /* �ⲿ���� PLL ���32MHz */
	//SetSysClock(CLK_SOURCE_HSI_32MHz);//��ʼ��ʱ��
	
	{
		//��ʼ������,printf��ӡ���
		GPIOA_SetBits( GPIO_Pin_9 );
		GPIOA_ModeCfg( GPIO_Pin_9, GPIO_ModeOut_PP_5mA );                           /* ����1��IO������ */
		UART1_DefInit(); //��ʼ��UART1,Ĭ�ϴ�ӡ
	}
	
}

void HardFault_Handler(void)
{
	while(1);
}
