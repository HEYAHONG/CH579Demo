/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.1
* Date               : 2019/11/05
* Description        : 
*******************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "CONFIG.h"
#include "CH57x_common.h"
#include "HAL.h"
#include "peripheral.h"
#include "central.h"

/*********************************************************************
 * GLOBAL TYPEDEFS
 */
__align(4) u32 MEM_BUF[BLE_MEMHEAP_SIZE/4];

#if (defined (BLE_MAC)) && (BLE_MAC == TRUE)
u8C MacAddr[6] = {0x84,0xC2,0xE4,0x03,0x02,0x02};
#endif

/*******************************************************************************
* Function Name  : BLE_main
* Description    : À¶ÑÀÖ÷º¯Êý
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
#include "FreeRTOS.h"
#include "task.h"
void BLE_init()
{
	taskENTER_CRITICAL();
  CH57X_BLEInit( );
	HAL_Init( );
	GAPRole_PeripheralInit( );
	GAPRole_CentralInit( );
	Peripheral_Init( );
	Central_Init( );
	taskEXIT_CRITICAL();
}
void BLE_loop()
{
	TMOS_SystemProcess( );
	FreeRTOS_CLK_Update();
}

/******************************** endfile @ main ******************************/
