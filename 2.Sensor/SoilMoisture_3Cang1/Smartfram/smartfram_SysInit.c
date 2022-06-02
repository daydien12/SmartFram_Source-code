#include "smartfram_SysInit.h"
IWDG_HandleTypeDef hiwdg;
sysCountTime_t Sys_CountTime;
varAndFlagSys_t Sys_VarAndStt;
sysDataReadSensor_t Sys_DataSensorRead;

static void InitValue(void);
static void Sys_Run_Init(void);
static void Sys_ModeDeviceInit(void);
static void Sys_JoinNetwork_Init(void);

void SmartFram_SYS_ReadAllSensor()
{
  smartfram_ReadSensorSoilMoisture(&Sys_DataSensorRead.Data_Senser_SoilMoisture);
  smartfram_GetBatteryLevel(&Sys_DataSensorRead.Data_Battery);
}

void Smartfram_SYS_Init(void)
{
  InitValue();
  smartfram_GPIO_Init();
  smartfram_FlashEraseInit();
  Sys_ModeDeviceInit();
	smartfram_ADC_LowpowerWakeupMode(0);
  MX_IWDG_Init(3);
}

void Smartfram_SYS_DebugSensor(uint8_t mode)
{
  char buffer_temp[30];
  if (mode == 1)
  {
		sprintf(buffer_temp, "%d", Sys_DataSensorRead.Data_Senser_SoilMoisture);
		APP_PPRINTF( "ADC = %s\n\r", buffer_temp);
    sprintf(buffer_temp, "%d", Sys_DataSensorRead.Data_Battery);
    APP_PPRINTF(" PIN: %s\r\n", buffer_temp);
  }
}

void InitValue(void)
{
  Sys_CountTime.Join_RUN_TimeOut = 0;
  Sys_CountTime.Join_Nextwork_TimeOut = 0;
	Sys_CountTime.Join_Nextwork_StopJoin = 0;//StopJoin
  Sys_DataSensorRead.Data_Senser_SoilMoisture = 0;
  Sys_DataSensorRead.Data_Battery = 0;
}

static void Sys_JoinNetwork_Init(void)
{
  FlagSys_CheckModeRun = Sys_DeviceFlashData.ModeRun;
  Sys_CountTime.Join_Nextwork_TimeOut = 0;
  Sys_VarAndStt.SYS_Switch_Mode_Lora = LORA_TX_MODE;
  Sys_VarAndStt.SYS_FSM_Status_JoinNetwork = FSM_JOIN_REQUETST_TX;
}

static void Sys_Run_Init(void)
{
  FlagSys_CheckModeRun = Sys_DeviceFlashData.ModeRun;
  Sys_CountTime.Join_RUN_TimeOut = 0;
  Sys_VarAndStt.SYS_Switch_Mode_Lora = LORA_RX_MODE;
}

static void Sys_ModeDeviceInit(void)
{
  if (FlagSys_CheckModeRun == MODE_JOIN_NETWORK)
  {
    Sys_JoinNetwork_Init();
  }
  else if (FlagSys_CheckModeRun == MODE_RUN)
  {
    Sys_Run_Init();
  }
}

void MX_IWDG_Init(float time_out)
{
  __HAL_RCC_CLEAR_RESET_FLAGS();
  uint16_t PrescalerCode;
  uint16_t Prescaler;
  uint16_t ReloadValue;
  uint16_t LsiFreq = 32000;
  if ( (time_out * LsiFreq) / 4 <= 4000)
  {
    PrescalerCode = 0x00;
    Prescaler = 4;
  }
  else if ((time_out * LsiFreq) / 8 <= 4000)
  {
    PrescalerCode = 0x01;
    Prescaler = 8;
  }
  else if ((time_out * LsiFreq) / 16 <= 4000)
  {
    PrescalerCode = 0x02;
    Prescaler = 16;
  }
  else if ((time_out * LsiFreq) / 32 <= 4000)
  {
    PrescalerCode = 0x03;
    Prescaler = 32;
  }
  else if ((time_out * LsiFreq) / 64 <= 4000)
  {
    PrescalerCode = 0x04;
    Prescaler = 64;
  }
  else if ((time_out * LsiFreq) / 128 <= 4000)
  {
    PrescalerCode = 0x05;
    Prescaler = 128;
  }
  else if ((time_out * LsiFreq) / 256 <= 4000)
  {
    PrescalerCode = 0x06;
    Prescaler = 256;
  }

  ReloadValue = (uint32_t)((time_out * LsiFreq) / Prescaler);
  IWDG->KR = 0x5555;
  IWDG->PR =  PrescalerCode;
  IWDG->RLR = ReloadValue;
  IWDG->KR = 0xAAAA;
  IWDG->KR = 0xCCCC;

}

void MX_IWDG_Reload(void)
{
  IWDG->KR = 0xAAAA;
}