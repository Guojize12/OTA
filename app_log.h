#ifndef __APP_LOG_H
#define __APP_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

void APP_LOG_Write(uint16_t log_num);
uint16_t APP_LOG_Read(void);
	
#ifdef __cplusplus
}
#endif

#endif /* __APP_LOG_H */

/*****END OF FILE****/
