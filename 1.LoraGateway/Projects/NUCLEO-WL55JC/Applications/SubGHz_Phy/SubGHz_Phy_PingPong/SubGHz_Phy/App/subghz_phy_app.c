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

uint16_t BufferSize = BUFFER_SIZE;
uint8_t Buffer[100];
int8_t RssiValue = 0;
int8_t SnrValue = 0;

/* Led Timers objects*/
static  UTIL_TIMER_Object_t TimeOject_TimeCountOut;

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

static void CmdProcessNotify(void);

static void JoinNetworkTask_Process(void);
static void MessageReceive_Process(void);
static void MessageTransmission_Process(void);

static void Sys_JoinNetwork_TransmissionMode(uint8_t* str, uint8_t size);
static void Sys_JoinNetwork_ReceiveMode(uint16_t TimeOut);

static void Sys_Getway_Init(void);

static void FS_SartScan(void);
static void FS_StopScan(void);
static void FS_EndScan(void);
static void FS_RunInit(void);

static void Sys_Getway_Init(void)
{
  FlagCommandSYS.StartScanDevice = 0;
	FlagCommandSYS.StopScanDevice = 0;
	FlagCommandSYS.ResetDevice = 0;
	FlagCommandSYS.Count_Commant_1s = 0;
	FlagCommandSYS.Count_Commant_Reset_RX = 0;
	FlagCommandSYS.Flag_Commant_Reset_RX = 0;
	FlagCommandSYS.Flag_Commant_MSG = 0;
  smartfram_FlashEraseInit(); // load data tu flash ra
  FS_RunInit();
}

/* Exported functions ---------------------------------------------------------*/
void SubghzApp_Init(void)
{
  Sys_Getway_Init();
  
  //APP_LOG(TS_OFF, VLEVEL_M, "APP_VERSION= V%X.%X.%X\r\n", (uint8_t)(__APP_VERSION >> __APP_VERSION_MAIN_SHIFT),
  //    (uint8_t)(__APP_VERSION >> __APP_VERSION_SUB1_SHIFT), (uint8_t)(__APP_VERSION >> __APP_VERSION_SUB2_SHIFT));
		CMD_Init(CmdProcessNotify);

  /* Led Timers*/
  UTIL_TIMER_Create(&TimeOject_TimeCountOut, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, TimeObjectFS_CountOut, NULL);
  UTIL_TIMER_SetPeriod(&TimeOject_TimeCountOut, 10);
  UTIL_TIMER_Start(&TimeOject_TimeCountOut);

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
  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_JoinNetwotk), UTIL_SEQ_RFU, JoinNetworkTask_Process);
  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_MessageReceive), UTIL_SEQ_RFU, MessageReceive_Process);
  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_MessageTransmission), UTIL_SEQ_RFU, MessageTransmission_Process);
#endif
	MX_IWDG_Init(3);
	APP_PPRINTF( "\n|||||||(RS)||||||||\n");
}


/* Private functions ---------------------------------------------------------*/

static void JoinNetworkTask_Process(void)
{
  if (FlagCommandSYS.StartScanDevice == 1)
  {
    FlagCommandSYS.StartScanDevice = 0;
    FS_SartScan();
  }
	
	if (FlagCommandSYS.StopScanDevice == 1)
  {
    FlagCommandSYS.StopScanDevice = 0;
    FS_StopScan();
  }
	
	if (FlagCommandSYS.ResetDevice == 1)
  {
    FS_StopScan();
  }
	
	if (FlagCommandSYS.Flag_Commant_Reset_RX == 1)
	{
		FS_StopScan();
	}
	
}

static void MessageReceive_Process(void)
{
  decrypted_t DataDec;
  joinRequetstMsg_t* JoinRequetst;
  joinConfirmMsg_t* JoinConfirm;
	MX_IWDG_Reload();
  if (FlagSys_CheckModeRun == MODE_JOIN_NETWORK)
  {
    Sys_VarAndStt.SYS_Switch_Mode_Lora = LORA_TX_MODE;
    switch (Sys_VarAndStt.SYS_FSM_Status_JoinNetwork)
    {
      case FSM_JOIN_SCANDEVICE:
        if (Smartfram_JoinNetWork_GetRequetst(Buffer, BufferSize) == 1)
        {
          Sys_CountTime.Join_Nextwork_TimeOut = 0;
          Sys_VarAndStt.SYS_FSM_Status_JoinNetwork = FSM_JOIN_ACCEPT;
        }
        break;

      case FSM_JOIN_ACCEPT:
        if (Smartfram_JoinNetWork_GetConfirm(Buffer, BufferSize) == 1)
        {
          Sys_CountTime.Join_Nextwork_TimeOut = 0;
          Sys_VarAndStt.SYS_FSM_Status_JoinNetwork = FSM_JOIN_COMPLETED;
        }
        break;

      case FSM_JOIN_COMPLETED:
        if (Smartfram_JoinNetWork_GetJoinDone(Buffer, BufferSize) == 1)
        {
          Sys_CountTime.Join_Nextwork_TimeOut = 0;
          Sys_VarAndStt.SYS_FSM_Status_JoinNetwork = FSM_JOIN_DONE;
        }
        break;
      case FSM_JOIN_DONE:

        break;
    }
  }
  else if (FlagSys_CheckModeRun == MODE_RUN)
  {
    if (Smartfram_Messeger_GetDeviceToGetway(Buffer, BufferSize))
    {
			FlagCommandSYS.Flag_Commant_MSG =1;
      Sys_VarAndStt.SYS_Switch_Mode_Lora = LORA_ACK_MODE;
    }
  }
}

static void MessageTransmission_Process(void)
{
	MX_IWDG_Reload();
  if (FlagSys_CheckModeRun == MODE_JOIN_NETWORK)
  {
    switch (Sys_VarAndStt.SYS_FSM_Status_JoinNetwork)
    {
      case FSM_JOIN_SCANDEVICE:
				#if APP_DEBUG_SCAN
				 APP_PPRINTF( "\n|||||||(APP DEBUG: FSM_JOIN_SCANDEVICE)||||||||\n");
				#endif
        Radio.SetChannel(RF_FREQUENCY);
        Radio.Rx(TIME_RX_JOIN);
        break;

      case FSM_JOIN_ACCEPT:
				#if APP_DEBUG_SCAN
				 APP_PPRINTF( "\n|||||||(APP DEBUG: FSM_JOIN_ACCEPT)||||||||\n");
				#endif

        if (Sys_VarAndStt.SYS_Switch_Mode_Lora == LORA_TX_MODE)
        {
          HAL_Delay(TIME_TX_JOIN);
          Smartfram_JoinNetWork_SendAccept(Sys_JoinNetwork_TransmissionMode);
          Sys_VarAndStt.SYS_Switch_Mode_Lora = LORA_RX_MODE;
        }
        else
        {
          Sys_JoinNetwork_ReceiveMode(TIME_RX_JOIN);
        }
        break;

      case FSM_JOIN_COMPLETED:
				#if APP_DEBUG_SCAN
				 APP_PPRINTF( "\n|||||||(APP DEBUG: FSM_JOIN_COMPLETED)||||||||\n");
				#endif
        if (Sys_VarAndStt.SYS_Switch_Mode_Lora == LORA_TX_MODE)
        {
          HAL_Delay(TIME_TX_JOIN);
          Smartfram_JoinNetWork_SendCompleted(Sys_JoinNetwork_TransmissionMode);
          Sys_VarAndStt.SYS_Switch_Mode_Lora = LORA_RX_MODE;
        }
        else
        {
          Sys_JoinNetwork_ReceiveMode(TIME_RX_JOIN);
        }
        break;

      case FSM_JOIN_DONE:
				#if APP_DEBUG_SCAN
				 APP_PPRINTF( "\n|||||||(APP DEBUG: FSM_JOIN_DONE)||||||||\n");
				#endif
        FS_EndScan();
        break;
    }
  }
  else if (FlagSys_CheckModeRun == MODE_RUN)
  {
    switch (Sys_VarAndStt.SYS_Switch_Mode_Lora)
    {
      case LORA_RX_MODE:
        Radio.SetChannel(RF_FREQUENCY);
        Radio.Rx(2000);
        break;

      case LORA_TX_MODE:
        HAL_Delay(1000);
        Smartfram_Messeger_SendGetwayToDevice(Sys_JoinNetwork_TransmissionMode);
        Sys_VarAndStt.SYS_Switch_Mode_Lora = LORA_RX_MODE;
        break;

      case LORA_ACK_MODE:
				HAL_Delay(2000);
				Smartfram_Messeger_SendGetwayToDevice(Sys_JoinNetwork_TransmissionMode);
				Sys_VarAndStt.SYS_Switch_Mode_Lora = LORA_RX_MODE;
			
				/*
				if(FlagCommandSYS.GetACKSensor == 1)
				{
					HAL_Delay(1000);
					Smartfram_Messeger_SendGetwayToDevice(Sys_JoinNetwork_TransmissionMode);
					Sys_VarAndStt.SYS_Switch_Mode_Lora = LORA_RX_MODE;
					FlagCommandSYS.GetACKSensor = 0;
				}
				else
				{
					UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_MessageTransmission), CFG_SEQ_Prio_0);
				}
			*/
        break;
    }
  }
}

static void OnTxDone(void)
{
#if APP_DEBUG_RADIO
  APP_PPRINTF("OnTxDone\n");
#endif
  Radio.Sleep();
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_MessageTransmission), CFG_SEQ_Prio_0);
}

static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
	uint8_t i;
#if APP_DEBUG_RADIO
  APP_PPRINTF("OnRxDone\n");
  APP_PPRINTF("RssiValue=%d dBm, SnrValue=%d\n", rssi, snr);
#endif
  Radio.Sleep();
  BufferSize = size;
  memcpy(Buffer, payload, BufferSize);
	
	/*
	APP_PPRINTF("DATA: %s, Size: %d\n", Buffer, BufferSize);
	for(i=0; i<BufferSize; i++)
	{
		APP_PPRINTF("%x ", Buffer[i]);
	}
	APP_PPRINTF("\n----------\n");
	*/
  //APP_LOG(TS_ON, VLEVEL_L, "size: %d\n\r", BufferSize);
  RssiValue = rssi;
  SnrValue = snr;
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_MessageReceive), CFG_SEQ_Prio_0);
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_MessageTransmission), CFG_SEQ_Prio_0);
}

static void OnTxTimeout(void)
{
#if APP_DEBUG_RADIO
  APP_PPRINTF("OnTxTimeout\n");
#endif
  Radio.Sleep();
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_MessageTransmission), CFG_SEQ_Prio_0);
}

static void OnRxTimeout(void)
{
#if APP_DEBUG_RADIO
  APP_PPRINTF("OnRxTimeout\n");
#endif
  Radio.Sleep();
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_MessageTransmission), CFG_SEQ_Prio_0);
}

static void OnRxError(void)
{
#if APP_DEBUG_RADIO
  APP_PPRINTF("OnRxError\n");
#endif
  Radio.Sleep();
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_MessageTransmission), CFG_SEQ_Prio_0);
}

static void CmdProcessNotify(void)
{
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_Vcom), 0);
}

static void TimeObjectFS_CountOut(void *context)
{
	if(FlagCommandSYS.Flag_Commant_MSG == 1)
	{
		if(FlagCommandSYS.Count_Commant_1s >=100)
		{
			if(FlagCommandSYS.Count_Commant_Reset_RX >= 10)
			{
				FlagCommandSYS.Flag_Commant_Reset_RX = 1;
				FlagCommandSYS.Count_Commant_Reset_RX = 0;
			}
			FlagCommandSYS.Count_Commant_Reset_RX ++;
			FlagCommandSYS.Count_Commant_1s = 0;
		}
		FlagCommandSYS.Count_Commant_1s++;
	}
	else
	{
		FlagCommandSYS.Count_Commant_1s = 0;
		FlagCommandSYS.Count_Commant_Reset_RX = 0;
		FlagCommandSYS.Flag_Commant_Reset_RX = 0;
		FlagCommandSYS.Flag_Commant_MSG = 0;
	}
	
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_JoinNetwotk), CFG_SEQ_Prio_0);
  UTIL_TIMER_Start(&TimeOject_TimeCountOut);
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
      Sys_VarAndStt.SYS_FSM_Status_JoinNetwork = FSM_JOIN_SCANDEVICE;
    }
    Sys_CountTime.Join_Nextwork_TimeOut++;
  }
}

static void FS_SartScan(void)
{
  FlagSys_CheckModeRun = MODE_JOIN_NETWORK;
  Sys_CountTime.Join_Nextwork_TimeOut = 0;
  Sys_VarAndStt.SYS_Switch_Mode_Lora = LORA_RX_MODE;
  Sys_VarAndStt.SYS_FSM_Status_JoinNetwork = FSM_JOIN_SCANDEVICE;
}
static void FS_StopScan(void)
{
	HAL_Delay(1000);
	NVIC_SystemReset();
}
static void FS_EndScan(void)
{
  Smartfram_Command_ScanACK(Sys_GetwayFlashArrData[Sys_Page].MACDevice[0], Sys_GetwayFlashArrData[Sys_Page].MACDevice[1], Sys_GetwayFlashArrData[Sys_Page].Unicast, Sys_GetwayFlashArrData[Sys_Page].TypeSensor, '\n');
  HAL_Delay(1000);
 // NVIC_SystemReset();
}

static void FS_RunInit(void)
{
  FlagSys_CheckModeRun = MODE_RUN;
  Sys_VarAndStt.SYS_Switch_Mode_Lora = LORA_RX_MODE;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
