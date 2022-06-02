
#ifndef __FLASHSYS_H__
#define __FLASHSYS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "smartfram_SysInit.h"
/*Struct cac truong thong tin luu vao bo nho flash ---------------------------*/
typedef struct
{
	uint8_t Status;					// 1 byte
	uint8_t TypeSensor;			// 1 byte
	uint16_t Option;				// 2 byte
	uint32_t Unicast;				// 4 byte
	uint32_t DeviceKey[4];	// 16 byte
	uint32_t MACDevice[2];	// 8 byte
}GetwayFlashData_t;  			// 32 byte


/*Macro so byte doc và ghi vao trong bo nho flash---------------------------*/
#define FLASH_SIZE_DATA_READ (sizeof(Sys_GetwayFlashArrData)) // tinh theo 1byte
#define FLASH_SIZE_DATA_WRITE (FLASH_SIZE_DATA_READ/8) 				// tinh theo 8byte(1 lan luu 8byte)
#define MAX_ADD_DEVICE 128 																		// 1 Device chien 32 byte, 128*32 = 4096 byte, bang 2 vung flash 4Kbytes

#define ADDR_FLASH_PAGE_125   ((uint32_t)0x0803E800) /* Base @ of Page 125, 2 Kbytes */
#define ADDR_FLASH_PAGE_126   ((uint32_t)0x0803F000) /* Base @ of Page 126, 2 Kbytes */
#define ADDR_FLASH_PAGE_127   ((uint32_t)0x0803F800) /* Base @ of Page 127, 2 Kbytes */

#define ARR_FULL 150

/*Bien nay dung de luu tru, truy xuat thong tin duoc load ra tu flash ------*/
extern GetwayFlashData_t Sys_GetwayFlashArrData[];
/*Bien nay de kiem tra trang thai khoi tao va chay khi moi khoi dong -------*/
extern uint8_t FlagSys_CheckModeRun;
extern uint32_t Sys_Unicast;
extern uint8_t Sys_Page;

void smartfram_FlashEraseInit(void);
void smartfram_SaveJointInfoRequetst(uint32_t *DataSaveFlash, uint8_t Size);
void smartfram_SaveJointInfoConfirm(uint32_t *DataSaveFlash, uint8_t Size);
void smartfram_SaveJointInfoJoinDone(void);

void smartfram_SetKeyDevice(uint8_t Pgae);
void smartfram_FlashRemoveInforDevice(uint8_t Page);

uint8_t smartfram_FindUnicast(uint32_t Unicast);
#ifdef __cplusplus
}
#endif

#endif
