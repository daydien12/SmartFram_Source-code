
#ifndef __STRUCTPAYLOAD_H__
#define __STRUCTPAYLOAD_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

#define SM_AES_CONFIRM_DF         4
#define SM_AES_COMPLETED_DF       4
#define SM_AES_MSGDEVICEGETGAY_DF 8
#define ARR_MSG 4
/**
	*encrypted: |
	*unencrypted: /
	*/
typedef struct
{
	uint16_t Header;			// 2 byte |
	uint8_t TypeMsg;			// 1 byte |
	uint8_t Crc;					// 1 byte |
	uint32_t MacAdress[2];// 8 byte |
}joinRequetstMsg_t;			// 12 byte 

typedef struct
{
	uint16_t Header;			// 2  byte |
	uint8_t TypeMsg;			// 1  byte |
	uint8_t Crc;					// 1  byte |
	uint32_t MacAdress[2];// 8  byte |
	uint32_t Netkey[4];		// 16 byte |
}joinAcceptMsg_t;				// 28 byte 


typedef struct
{
	uint16_t Header;			// 2 byte /
	uint8_t TypeMsg;			// 1 byte /
	uint8_t Crc;					// 1 byte /
	
	uint32_t Option;      // 4 byte |
	uint32_t MacAdress[2];// 8 byte |
	uint32_t Devkey[4];		// 16 byte |
	
}joinConfirmMsg_t;			// 30 byte 


typedef struct
{
	uint16_t Header;			// 2 byte /
	uint8_t TypeMsg;			// 1 byte /
	uint8_t Crc;					// 1 byte /
	
	uint32_t MacAdress[2];// 8 byte |
	uint32_t Unicast;			// 2 byte |
}joinCompletedMsg_t;		// 14 byte 


typedef struct
{
	uint16_t Header;			// 2 byte /
	uint8_t TypeMsg;			// 1 byte /
	uint8_t Crc;					// 1 byte /
	uint16_t Unicast;			// 2 byte /
	
	uint8_t Msg[10];			// 10 byte|
}msgDeviceGetway_t;			// 76 byte 


#ifdef __cplusplus
}
#endif

#endif
