
#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "smartfram_SysInit.h"
#include "smartfram_define.h"
#define MAXINPUT	(20)


#define BUTTON GPIO_PIN_6
#define PORT_BT GPIOC 

#define GPIO_WAKEUP_SENSOR GPIO_PIN_10
#define PORT_WAKEUP_SENSOR GPIOA
#define CLK_WAKEUP_SENSOR_ENABLE()                  __HAL_RCC_GPIOA_CLK_ENABLE()
#define CLK_WAKEUP_SENSOR_DISABLE()                 __HAL_RCC_GPIOA_CLK_ENABLE()


#define GPIO_WAKEUP_SENSOR_ADC GPIO_PIN_1
#define PORT_WAKEUP_SENSOR_ADC GPIOC
#define CLK_WAKEUP_ADC_SENSOR_ENABLE()                  __HAL_RCC_GPIOC_CLK_ENABLE()
#define CLK_WAKEUP_ADC_SENSOR_DISABLE()                 __HAL_RCC_GPIOC_CLK_ENABLE()



typedef struct
{
	unsigned char	vruc_DataOld;
	unsigned int 	vruc_CountAccess;
	unsigned char	vruc_Status;
	unsigned char vruc_FlagChange;
	unsigned char vruc_JoinNetword;
	unsigned char vruc_ClearFlash;
}	TS_TypeInput;

typedef struct
{
	uint8_t Count_CheckMode;
	uint8_t Count_ResetFlash;
	
	uint8_t Flag_JoinNetWorkMode;
	uint8_t Flag_SendMesseger;
	uint8_t vruc_StopAll;
}var_Status_t;
extern var_Status_t GPIO_ValueStatus;

void smartfram_GPIO_Init(void);
void smartfram_GPIO_Run(uint8_t Mode, void(*ClearFlash)(void));
void smartfram_GPIO_CheckModeLed(uint8_t Mode, void(*ClearFlash)(void));
void smartfram_GPIO_WakeupReadSensor(uint8_t mode);
void smartfram_GPIO_WakeupReadSensorCO2(uint8_t mode);
#ifdef __cplusplus
}
#endif

#endif