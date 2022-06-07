/**
  ******************************************************************************
  * @file    lora_command.c
  * @author  MCD Application Team
  * @brief   Main command driver dedicated to command AT
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "platform.h"
#include "lora_command.h"
#include "stm32_adv_trace.h"
#include "sys_app.h"
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* External variables ---------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* comment the following to have help message */
/* #define NO_HELP */
/* #define AT_RADIO_ACCESS */

/* Private typedef -----------------------------------------------------------*/
/**
  * @brief  Structure defining an AT Command
  */


/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

char Flag_Receive = 0;
char Array_Receive[10];
char Count_Data = 0;

char data;

/**
  * @brief  CMD_GetChar callback from ADV_TRACE
  * @param  rxChar th char received
  * @param  size
  * @param  error
  * @retval None
  */
static void CMD_GetChar(uint8_t *rxChar, uint16_t size, uint8_t error);

/**
  * @brief  CNotifies the upper layer that a charchter has been receveid
  * @param  None
  * @retval None
  */
static void (*NotifyCb)(void);

/**
  * @brief  Remove backspace and its preceding character in the Command string
  * @param  Command string to process
  * @retval 0 when OK, otherwise error
  */
static int32_t CMD_ProcessBackSpace(char *cmd);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Exported functions --------------------------------------------------------*/
void CMD_Init(void (*CmdProcessNotify)(void))
{
  /* USER CODE BEGIN CMD_Init_1 */

  /* USER CODE END CMD_Init_1 */
  UTIL_ADV_TRACE_StartRxProcess(CMD_GetChar);
  /* register call back*/
  if (CmdProcessNotify != NULL)
  {
    NotifyCb = CmdProcessNotify;
  }
  
}

void CMD_Process(void)
{
 //APP_PPRINTF("\n |%c| \n ", data);
}

/* USER CODE BEGIN EF */

/* USER CODE END EF */


static void CMD_GetChar(uint8_t *rxChar, uint16_t size, uint8_t error)
{
  data = *rxChar;
	
	Array_Receive[Count_Data] = data;
	Count_Data++;
	if(Count_Data >= 9)
	{
		Array_Receive[Count_Data] = 0;
		Flag_Receive = 1;
		Count_Data = 0;
	}
	
  if (NotifyCb != NULL)
  {
    NotifyCb();
  }
  /* USER CODE BEGIN CMD_GetChar_2 */

  /* USER CODE END CMD_GetChar_2 */
}


/* USER CODE BEGIN PrFD */

/* USER CODE END PrFD */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
