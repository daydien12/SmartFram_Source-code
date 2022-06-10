
#ifndef __JOINNETWORK_H__
#define __JOINNETWORK_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "smartfram_SysInit.h"
#include "smartfram_define.h"
typedef uint8_t(*sm_join_network_t)(void(*RadioSend)(uint8_t *str, uint8_t size), uint8_t Str_char[], uint8_t size);

typedef struct
{
	uint8_t Op;
	char Name[30];
	sm_join_network_t FunctionRun;
}sm_lora_cmd_func_t;
extern sm_lora_cmd_func_t JoinNetword_State[];

typedef struct 
{
	uint8_t Data[100];
	uint8_t Siize;
}tt_u;

uint32_t Smartfram_GetMACAdress1(void);
uint32_t Smartfram_GetMACAdress2(void);

uint8_t Smartfram_Test(tt_u *Data);

uint8_t Smartfram_JoinNetWork_SendRequetst(void(*RadioSend)(uint8_t *str, uint8_t size), uint8_t Str_char[], uint8_t size);
uint8_t Smartfram_JoinNetWork_SendConfirm(void(*RadioSend)(uint8_t *str, uint8_t size), uint8_t Str_char[], uint8_t size);

uint8_t Smartfram_JoinNetWork_GetAccept(void(*RadioSend)(uint8_t *str, uint8_t size), uint8_t Str_char[], uint8_t size);
uint8_t Smartfram_JoinNetWork_GetCompleted(void(*RadioSend)(uint8_t *str, uint8_t size), uint8_t Str_char[], uint8_t size);

uint8_t Smartfram_JoinNetWork_FindFunction(uint8_t State);
#ifdef __cplusplus
}
#endif

#endif
