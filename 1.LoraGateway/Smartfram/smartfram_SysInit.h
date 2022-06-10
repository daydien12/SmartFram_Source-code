
#ifndef __SYS_INIT_H__
#define __SYS_INIT_H__


#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "smartfram_command.h"
#include "smartfram_structpayload.h"
#include "smartfram_encryptionAES.h"
#include "smartfram_JoinNetwork.h"
#include "smartfram_FlashSys.h"
#include "smartfram_Messeger.h"
#include "smartfram_SerialBUG.h"

/*TIME COUNT SYS -------------------------------------------------------------*/
#define TIMEOUT_RX_SYS 10              //((48*5(s))/60(s) = 4Phut

/*chuyen che do truyen hay nhan -----------------------------------------------*/
typedef enum
{
  LORA_TX_MODE,
  LORA_RX_MODE,
  LORA_ACK_MODE,
} switchMode_TX_RX_t;

/*May trang thai thuc hien tham gia mang --------------------------------------*/
typedef enum
{
  FSM_JOIN_SCANDEVICE,
  FSM_JOIN_ACCEPT,
  FSM_JOIN_COMPLETED,
  FSM_JOIN_DONE,
} fsmJoinNetworkGetway_t;

/*Value SYS --------------------------------------*/
typedef struct
{
	switchMode_TX_RX_t 			SYS_Switch_Mode_Lora;
	fsmJoinNetworkGetway_t	SYS_FSM_Status_JoinNetwork;
}varAndFlagSys_t;
extern varAndFlagSys_t Sys_VarAndStt;

typedef struct
{
  uint16_t Join_Nextwork_TimeOut;
  uint16_t TimeBreakModeScan;
} sysCountTime_t;
extern sysCountTime_t Sys_CountTime;

/*che do hoat dong(che do join, che do chay binh thuong) -----------------------*/
enum
{
	MODE_JOIN_NETWORK = 0,
	MODE_RUN = 1,
};
/*dung de chuyen sang che do bug khong gui len may tinh nhung PI*/
#define SYS_MODE_BUG_NOT_SEND_TO_PI 0
#define SYS_FLASH_CLEAR 0 // xoa flash

void MX_IWDG_Init(float time_out);
void MX_IWDG_Reload(void);

#ifdef __cplusplus
}
#endif

#endif
