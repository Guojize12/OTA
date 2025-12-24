#ifndef __APP_CONFIG_H
#define __APP_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp_config.h"
#include "app_version.h"
#include "app_main.h"
#include "app_user.h"
#include "app_memory.h"
#include "app_rtu_at.h"
#include "app_dtu.h"
#include "app_iap.h"
#include "app_log.h"

/**
 * @brief Function pointer type to void/void function
 */
typedef void (*func_ptr_t)(void);


// OTA标志值定义
#define     OTA_FLAG_MAGIC_NUMBER         1  /*OTA升级魔术数，Bootloader检测到此值则进入升级模式*/


#define  DEVICE_MODEL    "F103_TY"
#define  EEPROM_ENABLE_VALUE      9527
#define  DID_DEFAULT 12345

// OTA配置
#define  USE_APP_MD5_CHK          // 启用APP区MD5校验（回写时计算MD5，启动时校验MD5）



#define ApplicationAddress    0x800B400

// Bootloader区（45KB）
#define APP_IAP_ADDR_BOOTLOADER_START   0x08000000  // Bootloader起始地址
#define APP_IAP_ADDR_BOOTLOADER_END     0x0800B3FF  // Bootloader结束地址（45KB = 0xB400）

// APP主程序区（106KB，对称设计）
#define APP_IAP_ADDR_APP_START          0x0800B400  // APP主程序起始地址
#define APP_IAP_ADDR_APP_END            0x08025FFF  // APP主程序结束地址，app空间106KB
// 升级缓冲区（106KB，与APP对称）
#define APP_IAP_ADDR_BACKUP_ADDR   0x08026000  // OTA缓冲区固件存放起始地址
#define APP_IAP_ADDR_BACKUP_ADDR_               (APP_IAP_ADDR_BACKUP_ADDR+20)
#define APP_IAP_ADDR_OTA_BUFFER_END     0x0803FFFF  // OTA缓冲区结束地址（106KB）

#define FIRMWARE_SUB_LEN					              1024					//固件分包长度






enum
{
    OTA_ERR_APP_MD5=100,   			  			//校验运行的APP md5 校验错误
    OTA_ERR_JUMP_APP=101,   						//自检成功后跳转APP失败
    OTA_ERR_NO_OTA=102,           			//不升级
    OTA_ERR_MSG_TIMEOUT=103,      			//获取升级固件信息超时
    OTA_ERR_FIRMWARE_MD5=104,     			//升级固件 md5 校验错误
    OTA_ERR_FIRMWARE_HEAD=105,    			//升级固件 头数据 校验错误
    OTA_ERR_FIRMWARE_NUM=106,     			//获取升级固件分包数据错误
    OTA_ERR_FIRMWARE_TIMEOUT=107, 			//获取升级固件数据超时
    OTA_ERR_FIRMWARE_TOO_LONG=108,      //升级固件大小超过片上flash
    OTA_ERR_WRITE_BACK=109,             //升级固件已下载成功,因异常回写多次失败(先定5次)
    OTA_ERR_TIME_SYNC_TIMEOUT=110,      //时间同步超时(超过20次重试)
    OTA_ERR_POWER_ON_TIMEOUT=111,       //上电状态发送超时(超过20次重试)

    OTA_ERR_MAX
};


union IEEE754
{
    __IO uint8_t	  BUFF[4];
    __IO uint32_t		BUFF_u32;
    __IO int32_t		BUFF_s32;
    __IO float	BUFF_float;
};

/** 系统参数*/
typedef struct
{
    uint32_t  eeprom_enable;
    uint32_t  password;  //密码
    uint32_t  device_id; //设备ID
    uint8_t   version_soft[3];/** 软件版本*/
    uint8_t   version_hard[3];/** 硬件版本*/
    char      model[20];/** 产品型号*/
    uint8_t   signle;
    uint8_t   signle_num;
    uint8_t   signle_wait;
    uint8_t   wait;
    uint8_t   reset_time;
} sys_def;

/** OTA参数*/
typedef struct
{
    uint32_t  firmware_size;/** 固件大小*/
    uint32_t  firmware_size_sub;/** 固件分包*/
    uint8_t   firmware_md5[16];/** 固件MD5*/
    uint32_t  firmware_version;/** 固件版本*/
    uint32_t  firmware_addr_read;/** 读固件分包地址*/
    uint32_t  firmware_addr_write;/** 固件备份地址变量*/
    uint8_t   ota_status;/** ota状态*/
    uint8_t   ota_disenable;/** 升级使能,0功能正常,1boot待机*/
    uint8_t   write_back_times;/** 备份回写次数*/
} ota_def;

typedef struct
{
    sys_def 	sys_config;
    ota_def 	ota_config;
} user_def;

extern user_def  g_user_config;

//OTA_END

/** 系统参数*/
typedef struct
{
    uint32_t  did; //设备ID
    uint16_t  dtp; //设备类型
    uint16_t  dnm; //设备号
    uint16_t  com; //通信类型
    char  model[20];
} app_cfg_def;
extern app_cfg_def  g_app_cfg;

void APP_CONFIG_Init(void);
void APP_CONFIG_Reset(uint16_t id);

void APP_CONFIG_Did_Set(uint32_t did);
uint32_t APP_CONFIG_Did_Get(void);
void APP_CONFIG_Dtp_Set(uint32_t dtp);
uint32_t APP_CONFIG_Dtp_Get(void);
void APP_CONFIG_DNM_Set(uint32_t dnm);
uint32_t APP_CONFIG_DNM_Get(void);

void APP_CONFIG_COM_Set(uint32_t com);
uint32_t APP_CONFIG_COM_Get(void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_CONFIG_H */

/*****END OF FILE****/

