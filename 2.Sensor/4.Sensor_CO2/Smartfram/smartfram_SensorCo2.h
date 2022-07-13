/*
 * max44009.h
 *
 *	The MIT License.
 *  Created on: 06.12.2018
 *      Author: Mateusz Salamon
 *		www.msalamon.pl
 *		mateusz@msalamon.pl
 *
 *	https://msalamon.pl/pomiar-natezenia-swiatla-z-wykorzystaniem-stm32/
 *	https://github.com/lamik/Light_Sensors_STM32
 */

#ifndef SOILMOISTURE_H_
#define SOILMOISTURE_H_
#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "smartfram_SysInit.h"
#include "smartfram_define.h"
extern ADC_HandleTypeDef    hadc;
void MX_ADC_Init(void);
void smartfram_ReadSensorCO2(uint32_t *Data);
void smartfram_CO2_LowpowerWakeupMode(uint8_t mode);
uint32_t ADC_ReadChannels(uint32_t channel);
#ifdef __cplusplus
}
#endif

#endif