
#ifndef __SYS_BUG_H__
#define __SYS_BUG_H__
//50

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/*DEBUG smartfram_FlashSys LIBRARY*/
#define FLASH_DEBUG_INIT 									0
#define FLASH_DEBUG_SAVE 									0
#define FLASH_DEBUG_ERROR 								0
#define FLASH_DEBUG_CHECKMAC 							0
#define FLASH_DEBUG_REMOVE_PAGE 					0
#define FLASH_DEBUG_UPDATE_DEVICEKEY 			0
#define FLASH_DEBUG_FIND_PAGE_EMPTY 			0

/*DEBUG subghz_phy_app LIBRARY*/
#define APP_DEBUG_RADIO 0
#define APP_DEBUG_SCAN 0

/*DEBUG smartfram_JoinNetwork LIBRARY*/
#define JOINNETWORK_DEBUG_GET_DATA 0

/*DEBUG smartfram_Messeger LIBRARY*/
#define MSG_DEBUG_SEND 0
#define MSG_DEBUG_GET_SENSOR 0
#define MSG_DEBUG_GET_REMOVE_NOTE 0
#define MSG_DEBUG_GET_CHECK_COUNT_IDMSG 0
/*DEBUG smartfram_Messeger LIBRARY*/
#define COMMAND_DEBUG_GET_DATA_PI 0
#ifdef __cplusplus
}
#endif

#endif
