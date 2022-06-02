
#ifndef __MESSEGER_H__
#define __MESSEGER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "smartfram_structpayload.h"

typedef union 
{
	uint8_t Data_In[4];
	uint32_t Data_Out;
}u8Tou32_u;


extern uint32_t IdMsg;
extern msgDeviceGetway_t SendMsgGetwayToDevice;
void Smartfram_Messeger_SendGetwayToDevice(void(*RadioSend)(uint8_t *str, uint8_t size));
uint8_t Smartfram_Messeger_GetDeviceToGetway(uint8_t Str_char[], uint8_t size);
#ifdef __cplusplus
}
#endif

#endif
