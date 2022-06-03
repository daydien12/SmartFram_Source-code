
#ifndef __SYS_INIT_H__
#define __SYS_INIT_H__


#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/

#include "stdio.h"
#include <math.h>
#include "i2c.h"
#include "lora_command.h"
#include "smartfram_GPIO.h"
#include "smartfram_TimeCount.h"
#include "smartfram_config.h"

#include "smartfram_structpayload.h"
#include "smartfram_encryptionAES.h"
#include "smartfram_JoinNetwork.h"
#include "smartfram_Messeger.h"
#include "smartfram_FlashSys.h"

#include "smartfram_Sensormax44009.h"
#include "smartfram_SensorSHT3x.h"
#include "smartfram_SensorCSS811.h"
#include "smartfram_SensorSoilMoisture.h"
#include "smartfram_Battery.h"

typedef struct 
{
	uint32_t Data_Senser_SoilMoisture;
	uint32_t Data_Battery;
}sysDataReadSensor_t;
extern sysDataReadSensor_t Sys_DataSensorRead;

typedef struct
{
  uint16_t Join_Nextwork_TimeOut;
  uint16_t Join_RUN_TimeOut;
	uint16_t Join_Nextwork_StopJoin;//StopJoin
} sysCountTime_t;
extern sysCountTime_t Sys_CountTime;

/*chuyen che do truyen hay nhan -----------------------------------------------*/
typedef enum
{
  LORA_TX_MODE,
  LORA_RX_MODE,
  LORA_REMOVE_MODE,
} switchMode_TX_RX_t;

/*May trang thai thuc hien tham gia mang --------------------------------------*/
typedef enum
{
  FSM_JOIN_REQUETST_TX 	= 0x01,
	FSM_JOIN_ACCEPT_RX 		= 0x02,
  FSM_JOIN_CONFIRM_TX 	= 0x03,
	FSM_JOIN_COMPLETED_RX = 0x04,
  FSM_JOIN_DONE_TX			= 0x05,
} fsmJoinNetworkDevice_t;

/*Value SYS --------------------------------------*/
typedef struct
{
	switchMode_TX_RX_t 			SYS_Switch_Mode_Lora;
	fsmJoinNetworkDevice_t	SYS_FSM_Status_JoinNetwork;
	uint8_t 								SYS_ModeRun;
	uint8_t									SYS_RX;
}varAndFlagSys_t;
extern varAndFlagSys_t Sys_VarAndStt;

/*TIME COUNT SYS -------------------------------------------------------------*/
#define TIMEOUT_RX_SYS 2 

/* Clear Flash*/
#define AUTO_CLEAR_FLASH 			1

/* Cài dat thoi gian JOIN mang */
#define TIME_TX_JOIN 100
#define TIME_RX_JOIN 1000

void Smartfram_SYS_Init(void);
void Smartfram_SYS_DebugSensor(uint8_t mode);
void SmartFram_SYS_ReadAllSensor();
void MX_IWDG_Init(float time_out);
void MX_IWDG_Reload(void);
#ifdef __cplusplus
}
#endif

#endif
