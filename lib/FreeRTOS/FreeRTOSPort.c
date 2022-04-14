#include "FreeRTOSPort.h"
#include "CH57x_common.h"
#include "FreeRTOS.h"
#include "task.h"

uint32_t SystemCoreClock=32000000;

//内部时钟
#define RC_32MHz (32000000)
#define RC_32KHz (32000)
//外部时钟(需要根据实际情况修改)
#define XT_32MHz (32000000)
#define XT_32KHz (32768)

static void __Update_Core_CLk()//更新SystemCoreClock
{
	uint32_t Fck32m = (R16_CLK_SYS_CFG&RB_CLK_OSC32M_XT) ? XT_32MHz : RC_32MHz;
	uint32_t Fck32k = (R8_CK32K_CONFIG&RB_CLK_OSC32K_XT) ? XT_32KHz : RC_32KHz;
	uint32_t Fpll = Fck32m * 15;
	uint32_t Fsys = (((RB_CLK_SYS_MOD&R16_CLK_SYS_CFG)>>7)&0x01) ? ( (((RB_CLK_SYS_MOD&R16_CLK_SYS_CFG)>>6)&0x01) ? Fck32k : Fck32m ) : (((((RB_CLK_SYS_MOD&R16_CLK_SYS_CFG)>>6)&0x01) ? Fpll : Fck32m ) / (R16_CLK_SYS_CFG & RB_CLK_PLL_DIV));
	SystemCoreClock=Fsys;
}


/* Constants required to manipulate the NVIC. */
#define portNVIC_SYSTICK_CTRL_REG             ( *( ( volatile uint32_t * ) 0xe000e010 ) )
#define portNVIC_SYSTICK_LOAD_REG             ( *( ( volatile uint32_t * ) 0xe000e014 ) )
#define portNVIC_SYSTICK_CURRENT_VALUE_REG    ( *( ( volatile uint32_t * ) 0xe000e018 ) )
#define portNVIC_INT_CTRL_REG                 ( *( ( volatile uint32_t * ) 0xe000ed04 ) )
#define portNVIC_SHPR3_REG                    ( *( ( volatile uint32_t * ) 0xe000ed20 ) )
#define portNVIC_SYSTICK_CLK_BIT              ( 1UL << 2UL )
#define portNVIC_SYSTICK_INT_BIT              ( 1UL << 1UL )
#define portNVIC_SYSTICK_ENABLE_BIT           ( 1UL << 0UL )
#define portNVIC_SYSTICK_COUNT_FLAG_BIT       ( 1UL << 16UL )
#define portNVIC_PENDSVSET_BIT                ( 1UL << 28UL )
#define portMIN_INTERRUPT_PRIORITY            ( 255UL )
#define portNVIC_PENDSV_PRI                   ( portMIN_INTERRUPT_PRIORITY << 16UL )
#define portNVIC_SYSTICK_PRI                  ( portMIN_INTERRUPT_PRIORITY << 24UL )

/* Constants required to set up the initial stack. */
#define portINITIAL_XPSR                      ( 0x01000000 )

/* The systick is a 24-bit counter. */
#define portMAX_24_BIT_NUMBER                 ( 0xffffffUL )



//CLK发生变化时需要调用
void FreeRTOS_CLK_Update(void)
{
	__Update_Core_CLk();
	
	//重新初始化systick
	{
				/* Stop and reset the SysTick. */
        portNVIC_SYSTICK_CTRL_REG = 0UL;
        /* Configure SysTick to interrupt at the requested rate. */
        portNVIC_SYSTICK_LOAD_REG = ( configCPU_CLOCK_HZ / configTICK_RATE_HZ ) - 1UL;
        portNVIC_SYSTICK_CTRL_REG = portNVIC_SYSTICK_CLK_BIT | portNVIC_SYSTICK_INT_BIT | portNVIC_SYSTICK_ENABLE_BIT;	
	}
	
}

//FreeRTOS启动调度器封装，正常情况下不会返回
void FreeRTOS_Start(void)
{
	__Update_Core_CLk();
	vTaskStartScheduler();//启动调度器	
}



static StackType_t IdleTaskStack[512];//空闲任务
static StaticTask_t IdleTaskTCB;
static StackType_t TimerTaskStack[configMINIMAL_STACK_SIZE];
static StaticTask_t TimerTaskTCB;
//空闲任务所需内存
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, 
                                StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
		*ppxIdleTaskTCBBuffer=&IdleTaskTCB;
		*ppxIdleTaskStackBuffer=IdleTaskStack; 
		*pulIdleTaskStackSize=configMINIMAL_STACK_SIZE;
}
//定时器任务所需内存
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, 
StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )
{
		*ppxTimerTaskTCBBuffer=&TimerTaskTCB;
		*ppxTimerTaskStackBuffer=TimerTaskStack; 
		*pulTimerTaskStackSize=configMINIMAL_STACK_SIZE;
}
