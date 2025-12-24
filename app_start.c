#include "app_config.h"

/***********

main.c 中的 初始化禁用
stm32f1xx_it.c 中的 串口中断禁用

***********/

static void APP_START_BSP_Init(void)
{
	
	  BSP_CONFIG_Init();
	
    MX_DMA_Init();
    LOG("I AM BOOTLOADER\n");
    BSP_RTC_Init();
    BSP_TIM_Init();
    BSP_IWDG_Init();

    EEPROM_FLASH_Init();
}

static void APP_START_Main(void)
{
    APP_MAIN_Handle();
}

void APP_START_Init(void)
{
    APP_START_BSP_Init();
    APP_START_Main();
}

/*****END OF FILE****/

