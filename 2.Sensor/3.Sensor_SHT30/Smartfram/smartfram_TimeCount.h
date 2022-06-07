
#ifndef __TIME_COUNT_H__
#define __TIME_COUNT_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "smartfram_SysInit.h"
#include "smartfram_define.h"

typedef struct 
{
	uint32_t CounterWakeUp_1Second;
	uint32_t TimeWakeup;
	
	uint8_t FlagRandom_Time;
	uint8_t FlagCountDone;
	uint8_t FlagUpdateSendMiss;
}timeCount_t;

extern timeCount_t SYS_TimeCountALL;

void smartfram_TimeCountRun(void);

#ifdef __cplusplus
}
#endif

#endif
