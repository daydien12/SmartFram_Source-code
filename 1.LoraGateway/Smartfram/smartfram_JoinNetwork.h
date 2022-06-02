
#ifndef __JOINNETWORK_H__
#define __JOINNETWORK_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "sys_app.h"


void Smartfram_JoinNetWork_SendAccept(void(*RadioSend)(uint8_t *str, uint8_t size));
void Smartfram_JoinNetWork_SendCompleted(void(*RadioSend)(uint8_t *str, uint8_t size));

uint8_t Smartfram_JoinNetWork_GetRequetst(uint8_t Str_char[], uint8_t size);
uint8_t Smartfram_JoinNetWork_GetConfirm(uint8_t Str_char[], uint8_t size);

uint8_t Smartfram_JoinNetWork_GetJoinDone(uint8_t Str_char[], uint8_t size);
#ifdef __cplusplus
}
#endif

#endif
