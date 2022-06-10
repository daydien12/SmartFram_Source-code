
#ifndef __FLASHSYS_H__
#define __FLASHSYS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "smartfram_SysInit.h"
#include "smartfram_define.h"
typedef struct 
{
	uint8_t ModeRun;				// 1 byte
	uint8_t TypeSensor;			// 1 byte
	uint16_t Option;				// 2 byte
	uint32_t Unicast;				// 4 byte
	uint32_t DeviceKey[4];	// 16 byte
	uint32_t NetKey[4];			// 16 byte
	uint32_t MACGetway[2];	// 8 byte
}deviceFlashData_t;
extern deviceFlashData_t Sys_DeviceFlashData;

typedef struct
{
	uint32_t TimeWakeup;
	uint32_t SendMiss;
	float Calib_MIN;
	uint32_t Temperature ;
	uint32_t Humidity;
	uint32_t Sensor_ERROR;
	uint32_t ID_Msg;
}deviceMSGFlashData_t;
extern deviceMSGFlashData_t Sys_DeviceFlashData_MSG;
extern uint8_t vr_Sensor_ERROR;
#define FLASH_SIZE_DATA_READ (sizeof(deviceFlashData_t))
#define FLASH_SIZE_DATA_WRITE (FLASH_SIZE_DATA_READ/8)

#define FLASH_SIZE_DATA_READ_MSG (sizeof(deviceMSGFlashData_t))
#define FLASH_SIZE_DATA_WRITE_MSG (FLASH_SIZE_DATA_READ_MSG/8)

#define ADDR_FLASH_PAGE_125   ((uint32_t)0x0803E800) /* Base @ of Page 125, 2 Kbytes */
#define ADDR_FLASH_PAGE_126   ((uint32_t)0x0803F000) /* Base @ of Page 126, 2 Kbytes */
#define ADDR_FLASH_PAGE_127   ((uint32_t)0x0803F800) /* Base @ of Page 127, 2 Kbytes */

/*che do hoat dong(che do join, che do chay binh thuong) -----------------------*/
#define MODE_JOIN_NETWORK 0
#define MODE_RUN 1
#define MODE_LOWPOWER 2

extern uint8_t FlagSys_CheckModeRun;

void smartfram_FlashEraseInit(void);

void smartfram_SaveJointInfoAccept(uint32_t *DataSaveFlash, uint8_t Size);
void smartfram_SaveJointInfoCompleted(uint32_t Unicast, uint8_t ModeRun);

void smartfram_RemoteNode(void);
void smartfram_IncrementIdMsg(uint32_t Temp, uint32_t Hum);
void smartfram_UpdateMSG_TimeWakeup(uint32_t Time);
void smartfram_UpdateMSG_SendMiss(uint32_t Stt_SendMiss);

#ifdef __cplusplus
}
#endif

#endif