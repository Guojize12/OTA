#include "bsp_config.h"
#include "app_config.h"
#include "app_log.h"

void APP_LOG_Write(uint16_t log_num)
{
    EEPROM_FLASH_WriteU16(APP_IAP_ADDR_LOG,log_num);
}

uint16_t APP_LOG_Read(void)
{
    uint16_t log_num = EEPROM_FLASH_ReadU16(APP_IAP_ADDR_LOG); 
    if(log_num==0xFF)
    {
        log_num=0;
    }
    LOGT("log_num:%d\n",log_num);
    return log_num;
}

/*****END OF FILE****/

