
#ifndef __SYS_BUG_H__
#define __SYS_BUG_H__
//50

#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
/*debug smartfram_encryptionAES.h*/
#define BUG_ENCRY_PRIVATEKEY 0
#define BUG_DNCRY_PRIVATEKEY 0
#define BUG_ENCRY_PUBLICKEY 0
#define BUG_DNCRY_PUBLICKEY 0
#define BUG_ENCRY_CHECK_HEADER_TYPEMSG 0
#define BUG_ENCRY_CHECKSUM 0

/*debug smartfram_JoinNetwork.h*/
#define JOINNETWORK_DEBUG_SEND 0
#define JOINNETWORK_DEBUG_GET 0

/*debug smartfram_FlashSys.h*/
#define FLASH_DEBUG_INIT 								0
#define FLASH_DEBUG_SAVE 								0
#define FLASH_DEBUG_UPDATE_TIME 				0
#define FLASH_DEBUG_INCREASE_IDMSG 			0
#define FLASH_DEBUG_CREATE_DEVICE_KEY 	0

/*debug  smartfram_Messeger.h*/
#define MSG_DEBUG_GET 1
#define MSG_DEBUG_SEND_SENSOR 0
#define MSG_DEBUG_SEND_ID_MSG 0

/*debug subghz_phy_app.h*/
#define APP_DEBUG_SENSOR			0
#define APP_DEBUG_RADIO 			0
/*debug smartfram_TimeCount.h*/
#define TIMECOUNT_DEBUG_RANDOM_TIME 1
#ifdef __cplusplus
}
#endif

#endif
