//DV
/*!
   \file      subghz_phy_app.c

   \brief     Ping-Pong implementation

   \copyright Revised BSD License, see section \ref LICENSE.

   \code
                  ______                              _
                 / _____)             _              | |
                ( (____  _____ ____ _| |_ _____  ____| |__
                 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
                 _____) ) ____| | | || |_| ____( (___| | | |
                (______/|_____)_|_|_| \__)_____)\____)_| |_|
                (C)2013-2017 Semtech

   \endcode

   \author    Miguel Luis ( Semtech )

   \author    Gregory Cristian ( Semtech )
*/
/**
  ******************************************************************************

             Portions COPYRIGHT 2020 STMicroelectronics

    @file    subghz_phy_app.c
    @author  MCD Application Team
    @brief   Application of the SubGHz_Phy Middleware
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "stm32_timer.h"
#include "sys_app.h"
#include "subghz_phy_app.h"
#include "radio.h"
#include "stm32_seq.h"
#include "utilities_def.h"
#include "app_version.h"
#include "stdio.h"
#include "smartfram_SysInit.h"

/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* External variables ---------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

typedef struct
{
	uint8_t FlagJoinNetwork;
	uint8_t FlagSendMsg;
	uint8_t FlagResetSoftware;
	
	uint8_t CountWainSendMsg;
	uint8_t CountResetSoftware;
	uint8_t CountEndSend;
	
	uint8_t FlagStartReadSensor;
	uint8_t CountStopReadCheckSensor;
}appVar_t;

appVar_t VarApp;
static void FS_VarAppInit(void);
static void FS_StartJoin(void);
static void FS_EndJoin(void);

static void FS_StartSendMsg(void);
static void FS_RunSendMsg(void);
static void FS_WainSendMsg(void);
static void FS_EndSendMsg(void);
static void FS_RemoveSendMsg(void);

static void FS_ResetSoftware(void);

static void FS_ReadSensor(void);

/*May trang thai thuc hien tham gia mang --------------------------------------*/
uint16_t BufferSize = BUFFER_SIZE;
uint8_t Buffer[100];
int8_t RssiValue = 0;
int8_t SnrValue = 0;

/* Led Timers objects*/
static  UTIL_TIMER_Object_t TimeOject_TimeCountOut;
static  UTIL_TIMER_Object_t TimeOject_TimeReadButton;
/* Radio events function pointer */
static RadioEvents_t RadioEvents;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/*!
   @brief  Function executed on when led timer elapses
   @param  LED context
   @retval none
*/
static void TimeObjectFS_CountOut(void *context);
static void TimeObjectFS_ReadButton(void *context);
/*!
   @brief Function to be executed on Radio Tx Done event
   @param  none
   @retval none
*/
static void OnTxDone(void);

/*!
   @brief Function to be executed on Radio Rx Done event
   @param  payload sent
   @param  payload size
   @param  rssi
   @param  snr
   @retval none
*/
static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);

/*!
   @brief Function executed on Radio Tx Timeout event
   @param  none
   @retval none
*/
static void OnTxTimeout(void);

/*!
   @brief Function executed on Radio Rx Timeout event
   @param  none
   @retval none
*/
static void OnRxTimeout(void);

/*!
   @brief Function executed on Radio Rx Error event
   @param  none
   @retval none
*/
static void OnRxError(void);

/*!
   @brief PingPong state machine implementation
   @param  none
   @retval none
*/
static void MessageReceive_Process(void);

/*!
   @brief PingPong state machine implementation
   @param  none
   @retval none
*/
static void MessageTransmission_Process(void);

/**
    @brief  call back when LoRaWan Stack needs update
    @param  none
    @retval none
*/
static void CmdProcessNotify(void);

/*!
   @brief TestTask state machine implementation
   @param  none
   @retval none
*/

/**
   @brief  callback to get the board 64 bits unique ID
   @param  unique ID
   @retval none
*/
static void JoinNetworkTask_Process(void);
static void Sys_JoinNetwork_TransmissionMode(uint8_t* str, uint8_t size);
static void Sys_JoinNetwork_ReceiveMode(uint16_t TimeOut);
int i=0;
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Exported functions ---------------------------------------------------------*/
void SubghzApp_Init(void)
{
  /* USER CODE BEGIN SubghzApp_Init_1 */
	FS_VarAppInit();
	Smartfram_SYS_Init();
  /* USER CODE END SubghzApp_Init_1 */
  /* Print APP version*/
  CMD_Init(CmdProcessNotify);
	/*
  APP_LOG(TS_OFF, VLEVEL_M, "APP_VERSION= V%X.%X.%X\r\n", (uint8_t)(__APP_VERSION >> __APP_VERSION_MAIN_SHIFT),
          (uint8_t)(__APP_VERSION >> __APP_VERSION_SUB1_SHIFT), (uint8_t)(__APP_VERSION >> __APP_VERSION_SUB2_SHIFT));
	*/
  /* Led Timers*/
	
	#if APP_TASK_TEST
  UTIL_TIMER_Create(&TimeOject_TimeCountOut, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, TimeObjectFS_CountOut, NULL);
  UTIL_TIMER_SetPeriod(&TimeOject_TimeCountOut, 1000);
  UTIL_TIMER_Start(&TimeOject_TimeCountOut);
	UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_JoinNetwotk), UTIL_SEQ_RFU, JoinNetworkTask_Process);
	#endif
	
  UTIL_TIMER_Create(&TimeOject_TimeReadButton, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, TimeObjectFS_ReadButton, NULL);
  UTIL_TIMER_SetPeriod(&TimeOject_TimeReadButton, 1000);
  UTIL_TIMER_Start(&TimeOject_TimeReadButton);

  /* Radio initialization */
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.RxDone = OnRxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  RadioEvents.RxTimeout = OnRxTimeout;
  RadioEvents.RxError = OnRxError;
  Radio.Init(&RadioEvents);

#if (( USE_MODEM_LORA == 1 ) && ( USE_MODEM_FSK == 0 ))
  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                    true, 0, 0, LORA_IQ_INVERSION_ON, TX_TIMEOUT_VALUE);

  Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                    LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                    LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                    0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

  Radio.SetMaxPayloadLength(MODEM_LORA, BUFFER_SIZE);

  Radio.SetChannel(RF_FREQUENCY);
  Radio.Rx(RX_TIMEOUT_VALUE);
	
	UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_Vcom), UTIL_SEQ_RFU, CMD_Process);
	UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_MessageReceive), UTIL_SEQ_RFU, MessageReceive_Process);
  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_MessageTransmission), UTIL_SEQ_RFU, MessageTransmission_Process);
#endif
}

/* USER CODE BEGIN EF */

/* USER CODE END EF */

/* Private functions ---------------------------------------------------------*/
#if APP_TASK_TEST
static void JoinNetworkTask_Process(void)
{
	SmartFram_SYS_ReadAllSensor();
	Smartfram_SYS_DebugSensor(1);

	/*
  if (Flag_RXUart == 1)
	{
		SmartFram_SYS_ReadAllSensor();
		Smartfram_SYS_DebugSensor(1);
		NVIC_SystemReset();
		Flag_RXUart = 0;
	}
	*/
}
#endif

static void MessageReceive_Process(void)
{
	uint8_t Temp_Stt = 0;
	uint8_t tempData[100];
	uint8_t ASize = 0;
	tt_u ff;
  if (FlagSys_CheckModeRun == MODE_JOIN_NETWORK)
	{
		if(VarApp.FlagJoinNetwork == 1)
		{
			if(Sys_VarAndStt.SYS_FSM_Status_JoinNetwork == FSM_JOIN_REQUETST_TX)
			{
				Sys_VarAndStt.SYS_FSM_Status_JoinNetwork = FSM_JOIN_ACCEPT_RX;
				if(Sys_VarAndStt.SYS_FSM_Status_JoinNetwork == FSM_JOIN_ACCEPT_RX)
				{
					Temp_Stt = Smartfram_JoinNetWork_FindFunction(FSM_JOIN_ACCEPT_RX);
					Sys_VarAndStt.SYS_Switch_Mode_Lora = LORA_TX_MODE;
					if(JoinNetword_State[Temp_Stt].FunctionRun(Sys_JoinNetwork_TransmissionMode, Buffer, BufferSize) == 1)
					{
						Sys_VarAndStt.SYS_FSM_Status_JoinNetwork = FSM_JOIN_CONFIRM_TX;
						Sys_CountTime.Join_Nextwork_TimeOut = 0;
					}
					else
					{
						Sys_VarAndStt.SYS_FSM_Status_JoinNetwork = FSM_JOIN_REQUETST_TX;
						Sys_CountTime.Join_Nextwork_TimeOut = 0;
					}
				}
			}
			else if(Sys_VarAndStt.SYS_FSM_Status_JoinNetwork == FSM_JOIN_CONFIRM_TX)
			{
				Sys_VarAndStt.SYS_FSM_Status_JoinNetwork = FSM_JOIN_COMPLETED_RX;
				if(Sys_VarAndStt.SYS_FSM_Status_JoinNetwork == FSM_JOIN_COMPLETED_RX)
				{
					Temp_Stt = Smartfram_JoinNetWork_FindFunction(FSM_JOIN_COMPLETED_RX);
					Sys_VarAndStt.SYS_Switch_Mode_Lora = LORA_TX_MODE;
					if(JoinNetword_State[Temp_Stt].FunctionRun(Sys_JoinNetwork_TransmissionMode, Buffer, BufferSize) == 1)
					{
						Sys_VarAndStt.SYS_FSM_Status_JoinNetwork = FSM_JOIN_DONE_TX;
						Sys_CountTime.Join_Nextwork_TimeOut = 0;
					}
					else
					{
						Sys_VarAndStt.SYS_FSM_Status_JoinNetwork = FSM_JOIN_REQUETST_TX;
						Sys_CountTime.Join_Nextwork_TimeOut = 0;
					}
				}
			}
		}
	}
	else if (FlagSys_CheckModeRun == MODE_RUN)
	{
		Temp_Stt = Smartfram_Messeger_GetGetwayToDevice(Sys_JoinNetwork_TransmissionMode, Buffer, BufferSize);
		if(Temp_Stt == 1)
		{
			if(Sys_DeviceFlashData_MSG.SendMiss == 1)
			{
				smartfram_UpdateMSG_SendMiss(0);
			}
			FS_EndSendMsg();
		}else if(Temp_Stt == 2)
		{
			Sys_VarAndStt.SYS_Switch_Mode_Lora = LORA_REMOVE_MODE;
		}
	}
}

static void MessageTransmission_Process(void)
{
	uint8_t Temp_Stt = 0;
  if (FlagSys_CheckModeRun == MODE_JOIN_NETWORK)
	{
		if(VarApp.FlagJoinNetwork == 1)
		{
			if (Sys_VarAndStt.SYS_Switch_Mode_Lora == LORA_TX_MODE)
			{
				if(Sys_VarAndStt.SYS_FSM_Status_JoinNetwork == FSM_JOIN_REQUETST_TX)
				{
					HAL_Delay(TIME_TX_JOIN);
					Temp_Stt = Smartfram_JoinNetWork_FindFunction(FSM_JOIN_REQUETST_TX);
					if(JoinNetword_State[Temp_Stt].FunctionRun(Sys_JoinNetwork_TransmissionMode, Buffer, BufferSize) == 2)
					{
						Sys_VarAndStt.SYS_Switch_Mode_Lora = LORA_RX_MODE;
					}
				}
				else if(Sys_VarAndStt.SYS_FSM_Status_JoinNetwork == FSM_JOIN_CONFIRM_TX)
				{
					HAL_Delay(TIME_TX_JOIN);
					Temp_Stt = Smartfram_JoinNetWork_FindFunction(FSM_JOIN_CONFIRM_TX);
					if(JoinNetword_State[Temp_Stt].FunctionRun(Sys_JoinNetwork_TransmissionMode, Buffer, BufferSize) == 2)
					{
						Sys_VarAndStt.SYS_Switch_Mode_Lora = LORA_RX_MODE;
					}
				}
				else if(Sys_VarAndStt.SYS_FSM_Status_JoinNetwork == FSM_JOIN_DONE_TX)
				{
					HAL_Delay(TIME_TX_JOIN);
					Temp_Stt = Smartfram_JoinNetWork_FindFunction(FSM_JOIN_CONFIRM_TX);
					if(JoinNetword_State[Temp_Stt].FunctionRun(Sys_JoinNetwork_TransmissionMode, Buffer, BufferSize) == 2)
					{
						FS_EndJoin();
					}
				}
			}
			else if(Sys_VarAndStt.SYS_Switch_Mode_Lora == LORA_RX_MODE)
			{
				Sys_JoinNetwork_ReceiveMode(TIME_RX_JOIN);
			}
		}
	}
	else if (FlagSys_CheckModeRun == MODE_RUN)
	{
		switch (Sys_VarAndStt.SYS_Switch_Mode_Lora)
    {
			case LORA_TX_MODE:
				FS_RunSendMsg();
				break;
			
			case LORA_RX_MODE:
				FS_WainSendMsg();
				break;
			
			case LORA_REMOVE_MODE:
				FS_RemoveSendMsg();
				break;
		}
	}
}

static void OnTxDone(void)
{
	#if APP_DEBUG_RADIO
		APP_PPRINTF("OnTxDone\n\r");
	#endif
  Radio.Sleep();
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_MessageTransmission), CFG_SEQ_Prio_0);
}

static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
	#if APP_DEBUG_RADIO
		APP_PPRINTF("OnRxDone\n\r");
		APP_PPRINTF("RssiValue=%d dBm, SnrValue=%d\n\r", rssi, snr);
	#endif
  Radio.Sleep();
  BufferSize = size;
  memcpy(Buffer, payload, BufferSize);
  RssiValue = rssi;
  SnrValue = snr;
	UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_MessageTransmission), CFG_SEQ_Prio_0);
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_MessageReceive), CFG_SEQ_Prio_0);
}

static void OnTxTimeout(void)
{
	#if APP_DEBUG_RADIO
		APP_PPRINTF("OnTxTimeout\n\r");
	#endif
  Radio.Sleep();
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_MessageTransmission), CFG_SEQ_Prio_0);
}

static void OnRxTimeout(void)
{
	#if APP_DEBUG_RADIO
		APP_PPRINTF("OnRxTimeout\n\r");
	#endif
  Radio.Sleep();
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_MessageTransmission), CFG_SEQ_Prio_0);
}

static void OnRxError(void)
{
	#if APP_DEBUG_RADIO
		APP_PPRINTF("OnRxError\n\r");
	#endif
  Radio.Sleep();
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_MessageTransmission), CFG_SEQ_Prio_0);
}

static void CmdProcessNotify(void)
{
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_Vcom), 0);
}

#if APP_TASK_TEST
static void TimeObjectFS_CountOut(void *context)
{	
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_JoinNetwotk), CFG_SEQ_Prio_0);
  UTIL_TIMER_Start(&TimeOject_TimeCountOut);
}
#endif

static void TimeObjectFS_ReadButton(void *context)
{
	MX_IWDG_Reload();
	if(GPIO_ValueStatus.Flag_JoinNetWorkMode == 1)
	{
		FS_StartJoin();
		GPIO_ValueStatus.Flag_JoinNetWorkMode = 0;
	}
	
	if(GPIO_ValueStatus.Flag_SendMesseger == 1)
	{
		VarApp.FlagStartReadSensor = 1;
		GPIO_ValueStatus.Flag_SendMesseger = 0;
	}
	
	if(SYS_TimeCountALL.TimeWakeup == 1)
	{
		VarApp.FlagStartReadSensor = 1;
		HAL_GPIO_WritePin(SYS_LED1_GPIO_PORT,SYS_LED1_PIN,1);
		SYS_TimeCountALL.TimeWakeup = 0;
	}
	
	if(GPIO_ValueStatus.vruc_StopAll == 1)
	{
		VarApp.FlagSendMsg = 0;
		VarApp.CountWainSendMsg = 0;
		GPIO_ValueStatus.vruc_StopAll = 0;
	}
	
	FS_ResetSoftware();
	FS_ReadSensor();
	smartfram_GPIO_Run(FlagSys_CheckModeRun, smartfram_RemoteNode);
  UTIL_TIMER_Start(&TimeOject_TimeReadButton);
}

static void Sys_JoinNetwork_TransmissionMode(uint8_t* str, uint8_t size)
{
  Radio.SetChannel(RF_FREQUENCY);
  HAL_Delay(Radio.GetWakeupTime() + TCXO_WORKAROUND_TIME_MARGIN);
  Radio.Send((uint8_t *)str, size);
}

static void Sys_JoinNetwork_ReceiveMode(uint16_t TimeOut)
{
  if (FlagSys_CheckModeRun == MODE_JOIN_NETWORK)
  {
    Radio.SetChannel(RF_FREQUENCY);
    Radio.Rx(TimeOut);
    if (Sys_CountTime.Join_Nextwork_TimeOut >= TIMEOUT_RX_SYS)
    {
			Sys_CountTime.Join_Nextwork_TimeOut = 0;
			Sys_VarAndStt.SYS_Switch_Mode_Lora = LORA_TX_MODE;
			Sys_VarAndStt.SYS_FSM_Status_JoinNetwork = FSM_JOIN_REQUETST_TX;
			if(Sys_CountTime.Join_Nextwork_StopJoin >= 30)//StopJoin
			{
				VarApp.FlagResetSoftware = 1;
			}
			Sys_CountTime.Join_Nextwork_StopJoin++;
    }
    Sys_CountTime.Join_Nextwork_TimeOut++;
  }
}

static void FS_VarAppInit(void)
{
	VarApp.FlagJoinNetwork                 = 0;
	VarApp.FlagSendMsg                     = 0;
	VarApp.CountWainSendMsg                = 0;
	VarApp.CountResetSoftware							 = 0;
	VarApp.FlagResetSoftware							 = 0;
	VarApp.CountEndSend                    = 0;
	VarApp.FlagStartReadSensor             = 0;
	VarApp.CountStopReadCheckSensor				 = 0;
	SYS_TimeCountALL.TimeWakeup            = 0;
	SYS_TimeCountALL.CounterWakeUp_1Second = 0;
	SYS_TimeCountALL.FlagRandom_Time       = 0;
	SYS_TimeCountALL.FlagCountDone         = 0;
	SYS_TimeCountALL.FlagUpdateSendMiss    = 0;
}

static void FS_StartJoin(void)
{
	VarApp.FlagJoinNetwork = 1;
	UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_MessageTransmission), CFG_SEQ_Prio_0);
}

static void FS_EndJoin(void)
{
	VarApp.FlagJoinNetwork = 0;
	FlagSys_CheckModeRun = MODE_RUN;
	Sys_VarAndStt.SYS_Switch_Mode_Lora = LORA_RX_MODE;
	HAL_GPIO_WritePin(SYS_LED1_GPIO_PORT,SYS_LED1_PIN,0);
	HAL_GPIO_WritePin(SYS_LED2_GPIO_PORT,SYS_LED2_PIN,0);
	HAL_GPIO_WritePin(SYS_LED3_GPIO_PORT,SYS_LED3_PIN,0);
	VarApp.FlagResetSoftware = 1;
}

static void FS_StartSendMsg(void)
{
	VarApp.FlagSendMsg = 1;
	Sys_VarAndStt.SYS_Switch_Mode_Lora = LORA_TX_MODE;
	UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_MessageTransmission), CFG_SEQ_Prio_0);
}

static void FS_RunSendMsg(void)
{
	HAL_Delay(500);
	Smartfram_Messeger_SendDeviceToGetway(Sys_JoinNetwork_TransmissionMode, Buffer, BufferSize);
	Sys_VarAndStt.SYS_Switch_Mode_Lora = LORA_RX_MODE;
}

static void FS_WainSendMsg(void)
{
	if(VarApp.FlagSendMsg == 1)
	{
		Radio.SetChannel(RF_FREQUENCY);
		Radio.Rx(1000);
		if(VarApp.CountWainSendMsg >= 4)
		{
			if(VarApp.CountEndSend >=0)
			{
				if(Sys_DeviceFlashData_MSG.SendMiss == 0 && SYS_TimeCountALL.FlagUpdateSendMiss == 0)
				{
					smartfram_UpdateMSG_SendMiss(1);
				}
				FS_EndSendMsg();
				VarApp.CountWainSendMsg  = 0;
			}
			VarApp.CountEndSend++;
		}
		VarApp.CountWainSendMsg++;
	}
}

static void FS_EndSendMsg(void)
{
	VarApp.FlagSendMsg = 0;
	Sys_VarAndStt.SYS_Switch_Mode_Lora = LORA_RX_MODE;
	HAL_GPIO_WritePin(SYS_LED1_GPIO_PORT,SYS_LED1_PIN,0);
	HAL_GPIO_WritePin(SYS_LED2_GPIO_PORT,SYS_LED2_PIN,0);
	HAL_GPIO_WritePin(SYS_LED3_GPIO_PORT,SYS_LED3_PIN,0);
  VarApp.FlagResetSoftware = 1;
}

static void FS_RemoveSendMsg(void)
{
	TempACK = 0xF0F0F0F0;
	Smartfram_Messeger_SendDeviceToGetway(Sys_JoinNetwork_TransmissionMode, Buffer, BufferSize);
	smartfram_RemoteNode();
	VarApp.FlagResetSoftware = 1;
}

static void FS_ResetSoftware(void)
{
	if(VarApp.FlagResetSoftware == 1)
	{
		if(VarApp.CountResetSoftware >= 1)
		{
				NVIC_SystemReset();
		}
		VarApp.CountResetSoftware++;
	}
}

static void FS_ReadSensor(void)
{
	if(VarApp.FlagStartReadSensor == 1)
	{
		SmartFram_SYS_ReadAllSensor();
		if((Sys_DataSensorRead.Data_Senser_Lux !=0)||(VarApp.CountStopReadCheckSensor >= 3))
		{
			VarApp.FlagStartReadSensor = 0;
			FS_StartSendMsg();
		}
		else
		{
				VarApp.CountStopReadCheckSensor++;
		}
	}
}

/* USER CODE BEGIN PrFD */

/* USER CODE END PrFD */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
