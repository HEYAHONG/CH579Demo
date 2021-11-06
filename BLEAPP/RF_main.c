/******************************************************************************/
/* 头文件包含 */
#include "CONFIG.h"
#include "CH57x_common.h"
#include "HAL.h"
#include "RF_PHY.h"
#include "FreeRTOS.h"
#include "task.h"

/*********************************************************************
 * GLOBAL TYPEDEFS
 */
__align(4) u32 MEM_BUF[BLE_MEMHEAP_SIZE/4];

/*******************************************************************************
* Function Name  : BLE_main
* Description    : BLE 主函数
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void BLE_main(void *arg)
{
  PRINT("BLE main start.\n");
  {
    PRINT("BLE Version %s\n",VER_LIB);
  }
	
	taskENTER_CRITICAL();
  CH57X_BLEInit();
  HAL_Init();
  RF_RoleInit( );
  RF_Init( );
	taskEXIT_CRITICAL();
	
	while(1)
	{
		TMOS_SystemProcess();
		FreeRTOS_CLK_Update();//恢复systick
		vTaskDelay(1);		
	}
}

/******************************** endfile @ main ******************************/
