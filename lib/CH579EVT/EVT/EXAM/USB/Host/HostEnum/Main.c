/********************************** (C) COPYRIGHT *******************************
* File Name          : Main.c
* Author             : WCH
* Version            : V1.0
* Date               : 2018/12/15
* Description 		 : USB设备枚举
*******************************************************************************/

/*
 使用此例程，请在 CH57x_usbhost.h中屏蔽 FOR_ROOT_UDISK_ONLY 和 DISK_BASE_BUF_LEN 宏定义
*/

#include "CH57x_common.h"

__align(4) UINT8  RxBuffer[ MAX_PACKET_SIZE ] ;      // IN, must even address
__align(4) UINT8  TxBuffer[ MAX_PACKET_SIZE ] ;      // OUT, must even address

int main()
{	
	UINT8	s;
    
	SetSysClock( CLK_SOURCE_HSE_32MHz );
    PWR_UnitModCfg( ENABLE, UNIT_SYS_PLL );		// 打开PLL
    DelayMs(5);
    
	GPIOA_SetBits(GPIO_Pin_9);
	GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);
	GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
	UART1_DefInit();
	PRINT( "Start @ChipID=%02X\n", R8_CHIP_ID );
	
    pHOST_RX_RAM_Addr = RxBuffer;
    pHOST_TX_RAM_Addr = TxBuffer;
	USB_HostInit();
	PRINT( "Wait Device In\n" );
	while(1)
	{
		s = ERR_SUCCESS;
		if ( R8_USB_INT_FG & RB_UIF_DETECT ) {  // 如果有USB主机检测中断则处理
			R8_USB_INT_FG = RB_UIF_DETECT ; 
			s = AnalyzeRootHub( );   
			if ( s == ERR_USB_CONNECT ) 		FoundNewDev = 1;
		}
		
		if ( FoundNewDev || s == ERR_USB_CONNECT ) {  // 有新的USB设备插入
			FoundNewDev = 0;
			mDelaymS( 200 );  // 由于USB设备刚插入尚未稳定,故等待USB设备数百毫秒,消除插拔抖动
			s = InitRootDevice();  // 初始化USB设备
			//if ( s != ERR_SUCCESS ) 	return( s );
		}
	}
}

