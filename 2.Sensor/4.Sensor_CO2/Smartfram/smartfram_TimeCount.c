#include "smartfram_TimeCount.h"

timeCount_t SYS_TimeCountALL;
uint8_t TimeRandom = 0;

long map(long x, long in_min, long in_max, long out_min, long out_max);

void FS_TimeCountWakeUp(void);
void FS_TimeSetInit();
void FS_TimeDebug();
uint8_t FS_TimeRandom(void);

void smartfram_TimeCountRun(void)
{
	FS_TimeSetInit();
	FS_TimeCountWakeUp();
}

uint8_t FS_TimeRandom(void)
{
	uint32_t counter = 0;
	uint32_t aRandom32bit;
	Smartfram_RNG_Init();
	if (HAL_RNG_GenerateRandomNumber(&hrng, &aRandom32bit) != HAL_OK)
	{
		Error_Handler();
	}
	return (aRandom32bit&0xffff);
}

void FS_TimeSetInit()
{
	if(Sys_DeviceFlashData_MSG.SendMiss == 0)
	{
		if(SYS_TimeCountALL.FlagRandom_Time == 0)
		{
			TimeRandom = map(FS_TimeRandom(), 0,255,1,60);
			SYS_TimeCountALL.TimeWakeup = (Sys_DeviceFlashData_MSG.TimeWakeup*60)+TimeRandom;
			#if TIMECOUNT_DEBUG_RANDOM_TIME
				FS_TimeDebug();
			#endif
			SYS_TimeCountALL.FlagRandom_Time = 1;
			SYS_TimeCountALL.FlagUpdateSendMiss = 0;
		}
	}
	else
	{
		if(SYS_TimeCountALL.FlagRandom_Time == 0)
		{
			TimeRandom = map(FS_TimeRandom(), 0,255,1,60);
			//SYS_TimeCountALL.TimeWakeup = TimeRandom;
			SYS_TimeCountALL.TimeWakeup = (1*60)+TimeRandom;
			#if TIMECOUNT_DEBUG_RANDOM_TIME
				FS_TimeDebug();
			#endif
			SYS_TimeCountALL.FlagRandom_Time = 1;
			SYS_TimeCountALL.FlagUpdateSendMiss = 1; 
			if(Sys_DeviceFlashData_MSG.SendMiss != 0)
			{
				smartfram_UpdateMSG_SendMiss(0);
			}
		}
	}
}

void FS_TimeCountWakeUp(void)
{
	if(!SYS_TimeCountALL.FlagCountDone && FlagSys_CheckModeRun == MODE_RUN)
	{
		SYS_TimeCountALL.CounterWakeUp_1Second++;
		if(SYS_TimeCountALL.CounterWakeUp_1Second >= SYS_TimeCountALL.TimeWakeup)
		{				
			SYS_TimeCountALL.CounterWakeUp_1Second = 0;
			SYS_TimeCountALL.FlagCountDone = 1;
			SYS_TimeCountALL.TimeWakeup = 1;
		}
	}
}

void FS_TimeDebug()
{
	APP_PPRINTF("ModeRun: %x \n"		,Sys_DeviceFlashData.ModeRun);
	APP_PPRINTF("TypeSensor: %x \n"	,Sys_DeviceFlashData.TypeSensor);
	APP_PPRINTF("Unicast: %d \n"		,Sys_DeviceFlashData.Unicast);
	APP_PPRINTF("SendMiss: %d \n"		,Sys_DeviceFlashData_MSG.SendMiss);
	APP_PPRINTF("CO2: %d \n"				,Sys_DeviceFlashData_MSG.CO2);
	APP_PPRINTF("ERROR: %d \n"			,Sys_DeviceFlashData_MSG.ERROR);
	APP_PPRINTF("TimeWakeup: %d \n"	,Sys_DeviceFlashData_MSG.TimeWakeup);
	APP_PPRINTF("Random: %d\n\r"		,TimeRandom);
	APP_PPRINTF("SYS_TimeCountALL.TimeWakeup: %d s |%d gio : %d phut : %d s\n\r", SYS_TimeCountALL.TimeWakeup, (SYS_TimeCountALL.TimeWakeup/3600),(SYS_TimeCountALL.TimeWakeup%3600/60), (SYS_TimeCountALL.TimeWakeup%3600%60));
}

long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}