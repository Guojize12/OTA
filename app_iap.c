#include "app_config.h"
#include "app_iap.h"
#include "eeprom_flash.h"  // 添加EEPROM Flash头文件
#include "stm32f1xx.h"     // 包含STM32F1系列的定义 (__IO, SRAM_BASE等)

#define ALIGN32bit(a)      			(((a + 3) / 4) * 4)
#define ALIGN64bit(a)               ALIGN32bit(a)  // STM32使用32位对齐
#define WRITEBACK_TIMES         5

extern MD5_CTX g_cont;

// 全局变量定义 - STM32F103适配
uint32_t JumpAddress;
pFunction Jump_To_Application;
uint16_t g_progress = 0;

/* SRAM definitions - STM32F103RC */
#define SRAM_SIZE                       (48 * 1024)   // STM32F103RC有48KB SRAM


int32_t APP_IAP_App_Load(void)
{
     JumpAddress = *(__IO uint32_t*) (ApplicationAddress + 4);

      /* Jump to user application */
      Jump_To_Application = (pFunction) JumpAddress;
      /* Initialize user application's Stack Pointer */
      __set_MSP(*(__IO uint32_t*) ApplicationAddress);
      Jump_To_Application();
	return  1;
}

void APP_IAP_Write_App_Size(uint32_t size)
{
    EEPROM_FLASH_WriteU32(APP_IAP_ADDR_APP_SIZE,size);
}

uint32_t APP_IAP_Read_App_Size(void)
{
    return EEPROM_FLASH_ReadU32(APP_IAP_ADDR_APP_SIZE);
}

uint32_t APP_IAP_Read_App_Md5(uint8_t *md5,uint8_t md5_len)
{
    EEPROM_FLASH_Read_Buf(APP_IAP_ADDR_APP_MD5, (uint16_t*)md5, (md5_len+1)/2);
    return md5_len;
}

void APP_IAP_Write_App_Md5(uint8_t *md5,uint8_t md5_len)
{
    EEPROM_FLASH_Write_Buf(APP_IAP_ADDR_APP_MD5, (uint16_t*)md5, (md5_len+1)/2);

}

uint32_t APP_IAP_Read_App_Version(void)
{
    return EEPROM_FLASH_ReadU32(APP_IAP_ADDR_APP_VERSION);
}

//jump to app
int APP_IAP_App_Run(void)
{
    LOG("jump to app --------------->!\n");	

    APP_IAP_App_Load(); //跳转APP运行
    LOG("jump to app err!\n");
    return OTA_ERR_JUMP_APP;
}

int APP_IAP_Err_Log_Check(void)
{
    int result = USR_FALSE;
    uint8_t log_num = APP_LOG_Read();
#ifdef USE_APP_MD5_CHK
    if(log_num>=OTA_ERR_APP_MD5)
#else
    if(log_num>=OTA_ERR_JUMP_APP)
#endif
    {
        LOG("\nLOG Err[%d]!!\n",log_num);
        result = USR_TRUE;
    }
    return result;
}

//Upgread check
int APP_IAP_Upgread_Status_Check(void)
{
    int result = USR_FALSE;
    switch(EEPROM_FLASH_ReadU32(APP_IAP_ADDR_STATUS_OTA))
    {
    case 1:
        LOG("ready to ota..\n");
        result = USR_TRUE;
        break;
    case 2:
        LOG("ready to copy firmware...\n");
        result = USR_FIRMWARE_BACKUP_OK;
        break;
    }

    return result;
}

#ifdef USE_APP_MD5_CHK
//MD5 check
int APP_IAP_MD5_Check(void)
{
    int result = USR_FALSE;

    uint8_t readBuf[FIRMWARE_SUB_LEN] = {0};
    uint32_t readLen=0;
    uint32_t processe_size = 0;
    uint32_t firmware_size = APP_IAP_Read_App_Size();

    uint8_t digest[16]= {0};
    uint8_t app_md5[16] = {0};
    APP_IAP_Read_App_Md5(app_md5,16);

    if(firmware_size<100 && memcmp(app_md5,digest,16)==0)
    {
        LOG("APP new!!\n");
        result = USR_TRUE;
    }
    else
    {
#if 0
        LOG("read app size <%d>\n",firmware_size);
#endif
        MD5_Init(&g_cont);

        while(1)
        {
            if(firmware_size-processe_size < FIRMWARE_SUB_LEN) //小于SPI_FLASH_SECTOR_SIZE,则readLen为实际长度
            {
                readLen = ALIGN32bit(firmware_size-processe_size);	//4字节对齐
//                LOG("ota last copy packet [%d]\n",readLen);
            }
            else
            {
                readLen=FIRMWARE_SUB_LEN;
            }

            BSP_FLASH_Read(APP_IAP_ADDR_APP_START+processe_size,(uint32_t *)readBuf,readLen);

            MD5_Update(&g_cont,readBuf,readLen);
            processe_size+=readLen;

            if(processe_size >= firmware_size)
            {
                break; //读取完成
            }
        }

        MD5_Final(&g_cont, digest);

        if(memcmp(app_md5,digest,16)==0)
        {
            result = USR_TRUE;
#if 0
            LOG("chk app md5 success!\n");
            LOG("md5: ");
            LOG_HEX(digest,16);
#endif
        }
        else
        {
            LOGT("chk app md5 err\n");
            LOGT("md5 old: ");
            LOG_HEX(app_md5,16);
            LOGT("md5 new: ");
            LOG_HEX(digest,16);
        }
    }
    return result;
}
#endif
int APP_IAP_Check(void)
{
    int ret;

    //错误log相关
    ret = APP_IAP_Err_Log_Check();

    if(ret == USR_TRUE)
    {
        ret = USR_LOG_SEND;
        goto end;
    }

    //升级标志位检测
    ret = APP_IAP_Upgread_Status_Check();
    if(ret == USR_TRUE)
    {
        ret = USR_OTA_TRUE;
        goto end;
    }
    else if(ret == USR_FIRMWARE_BACKUP_OK)
    {
        g_user_config.ota_config.write_back_times=EEPROM_FLASH_ReadU16(APP_IAP_ADDR_WRITE_BACK);
        if(g_user_config.ota_config.write_back_times >= WRITEBACK_TIMES)
        {
            APP_LOG_Write(OTA_ERR_WRITE_BACK);
            BSP_DELAY_MS(100);
            NVIC_SystemReset();
        }
        goto end;
    }

#ifdef USE_APP_MD5_CHK
    //MD5校验
    LOGT("boot:checking APP MD5...\n");
    ret = APP_IAP_MD5_Check();
    if(ret == USR_FALSE)
    {
        LOGT("boot:APP MD5 check failed!\n");
        ret = OTA_ERR_APP_MD5; //app的MD5错误
        goto end;
    }
    LOGT("boot:APP MD5 check OK\n");
#endif

    ret = USR_OTA_FALSE;
end:
    return ret;
}
void APP_IAP_Upgrade(void)
{
    uint8_t readBuf[FIRMWARE_SUB_LEN];
    uint32_t readLen=FIRMWARE_SUB_LEN;
    uint32_t processe_size = 0;
    uint32_t firmware_size = g_user_config.ota_config.firmware_size;//获取固件大小

    if(firmware_size > (APP_IAP_ADDR_APP_END-APP_IAP_ADDR_APP_START))
    {
        firmware_size = (APP_IAP_ADDR_APP_END-APP_IAP_ADDR_APP_START);
    }
    BSP_FLASH_Erase_Pages(APP_IAP_ADDR_APP_START, APP_IAP_ADDR_APP_START+firmware_size);//擦除app区

    LOG("New Firmware size <%d>\n",firmware_size);
    LOG("app start addr <0x%X>\n",APP_IAP_ADDR_APP_START);

#ifdef USE_APP_MD5_CHK
    MD5_Init(&g_cont);
#endif

    while(1)
    {
        readLen = FIRMWARE_SUB_LEN;
        if(firmware_size-processe_size < FIRMWARE_SUB_LEN) //小于SPI_FLASH_SECTOR_SIZE,则readLen为实际长度
        {
            memset(readBuf,0,FIRMWARE_SUB_LEN);
            readLen = ALIGN32bit(firmware_size-processe_size);	//4字节对齐
            LOG("ota last copy packet [%d]\n",readLen);
        }

        // 【安全检查】防止写入地址越界到Boot区或其他区域
        uint32_t write_addr = APP_IAP_ADDR_APP_START + processe_size;
        if(write_addr < APP_IAP_ADDR_APP_START || 
           write_addr >= APP_IAP_ADDR_APP_END ||
           (write_addr + readLen) > APP_IAP_ADDR_APP_END)
        {
            LOGT("writeback addr check failed! addr=0x%X, len=%d\n", write_addr, readLen);
            LOGT("APP range: 0x%X - 0x%X\n", APP_IAP_ADDR_APP_START, APP_IAP_ADDR_APP_END);
            APP_LOG_Write(OTA_ERR_WRITE_BACK);
            BSP_DELAY_MS(100);
            NVIC_SystemReset();
        }

        BSP_FLASH_Read(APP_IAP_ADDR_BACKUP_ADDR_+processe_size,(uint32_t*)readBuf,readLen);//读取备份区固件

        int write_result = BSP_FLASH_Write(write_addr, (uint32_t*)readBuf, readLen);//回写app区
        if(write_result != 0)
        {
            LOGT("write back err!! result=%d, addr=0x%X\n", write_result, write_addr);
            BSP_DELAY_MS(100);
            NVIC_SystemReset();
        }

#ifdef USE_APP_MD5_CHK
        MD5_Update(&g_cont,readBuf,readLen);
#endif

        processe_size += readLen;//已处理大小
        g_progress = 100+processe_size*100/firmware_size;
        LOG("writeback process %d%%\n",g_progress-100);
#ifdef USE_IWDG
        BSP_IWDG_Refresh();//喂狗
#endif
        if(processe_size >= firmware_size)
        {
            break; //升级成功
        }
    }
#ifdef USE_APP_MD5_CHK
    uint8_t digest[16]= {0};
    MD5_Final(&g_cont, digest);
    LOGT("writeback MD5: ");
    LOG_HEX(digest, 16);
    APP_IAP_Write_App_Md5(digest, 16); //记录app MD5
    LOGT("writeback MD5 saved to EEPROM\n");
#endif
    EEPROM_FLASH_WriteU32(APP_IAP_ADDR_STATUS_OTA,0);//复位升级标志位（改为U32与读取匹配）
    EEPROM_FLASH_WriteU16(APP_IAP_ADDR_APP_OK,0);//复位运行志位

}

void APP_IAP_Handle(void)
{
    APP_IAP_Upgrade();//写固件到 app区

    LOG("to reboot..\n");
    BSP_DELAY_MS(3000);
    NVIC_SystemReset();
    while(1);

}

/*****END OF FILE****/

