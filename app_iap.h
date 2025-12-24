/**
  ******************************************************************************
  * File Name          : app_iap.h
  * Description        : This file provides code for the configuration
  *                      of the iap instances.
  ******************************************************************************
  */
	
#ifndef __APP_IAP_H
#define __APP_IAP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
typedef  void (*pFunction)(void);

void APP_IAP_Handle(void);

int APP_IAP_Check(void);
int APP_IAP_App_Run(void);

void APP_IAP_Write_App_Size(uint32_t size);
void APP_IAP_Write_App_Md5(uint8_t *md5,uint8_t md5_len);

uint32_t APP_IAP_Read_App_Size(void);
uint32_t APP_IAP_Read_App_Md5(uint8_t *md5,uint8_t md5_len);
int32_t APP_IAP_App_Load(void);
 
#ifdef __cplusplus
}
#endif

#endif /*__APP_IAP_H */ 

/*****END OF FILE****/
