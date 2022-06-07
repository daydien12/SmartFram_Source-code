
#ifndef __MESSEGER_H__
#define __MESSEGER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "smartfram_SysInit.h"
#include "smartfram_define.h"
typedef union 
{
	uint32_t Data_In;
	uint8_t Data_Out[4];
}u32Tou8_u;

typedef union 
{
	float Data_In;
	uint8_t Data_Out[4];
}floatTou8_u;


extern uint32_t TempACK;
void Smartfram_Messeger_SendDeviceToGetway(void(*RadioSend)(uint8_t *str, uint8_t size), uint8_t Str_char[], uint8_t size);
uint8_t Smartfram_Messeger_GetGetwayToDevice(void(*RadioSend)(uint8_t *str, uint8_t size), uint8_t Str_char[], uint8_t size);
#ifdef __cplusplus
}
#endif

#endif
