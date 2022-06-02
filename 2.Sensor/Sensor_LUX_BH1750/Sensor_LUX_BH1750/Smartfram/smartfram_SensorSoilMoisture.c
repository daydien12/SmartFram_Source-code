
#include "smartfram_SensorSoilMoisture.h"
#include "stm32_adv_trace.h"
#include "usart_if.h"
#define GPIO_WAKEUP_SENSOR_ADC GPIO_PIN_1
#define PORT_WAKEUP_SENSOR_ADC GPIOC
#define CLK_WAKEUP_ADC_SENSOR_ENABLE()                  __HAL_RCC_GPIOC_CLK_ENABLE()
#define CLK_WAKEUP_ADC_SENSOR_DISABLE()                 __HAL_RCC_GPIOC_CLK_ENABLE()

#define SIZE_DATA_READ 8
uint8_t Msg_Read[SIZE_DATA_READ] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x02, 0xc4, 0x0b};

ADC_HandleTypeDef hadc;
void FS_ADC_LowpowerWakeupMode(uint8_t mode);
uint16_t FS_CheckSum(uint8_t *buf, uint8_t len);

void FS_ADC_LowpowerWakeupMode(uint8_t mode)
{
	if(mode)
	{
		GPIO_InitTypeDef  gpio_init_structure = {0};
		CLK_WAKEUP_ADC_SENSOR_ENABLE();

		gpio_init_structure.Pin = GPIO_WAKEUP_SENSOR_ADC;
		gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
		gpio_init_structure.Pull = GPIO_NOPULL;
		gpio_init_structure.Speed = GPIO_SPEED_FREQ_HIGH;

		HAL_GPIO_Init(PORT_WAKEUP_SENSOR_ADC, &gpio_init_structure);
		HAL_GPIO_WritePin(PORT_WAKEUP_SENSOR_ADC, GPIO_WAKEUP_SENSOR_ADC, GPIO_PIN_SET);
	}
	else
	{
		GPIO_InitTypeDef  gpio_init_structure = {0};
		CLK_WAKEUP_ADC_SENSOR_ENABLE();

		gpio_init_structure.Pin = GPIO_WAKEUP_SENSOR_ADC;
		gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
		gpio_init_structure.Pull = GPIO_NOPULL;
		gpio_init_structure.Speed = GPIO_SPEED_FREQ_HIGH;
		HAL_GPIO_Init(PORT_WAKEUP_SENSOR_ADC, &gpio_init_structure);
		HAL_GPIO_WritePin(PORT_WAKEUP_SENSOR_ADC, GPIO_WAKEUP_SENSOR_ADC, GPIO_PIN_RESET);
		//HAL_GPIO_DeInit(PORT_WAKEUP_SENSOR_ADC, GPIO_WAKEUP_SENSOR_ADC);
	}
}

void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hadc->Instance==ADC)
  {
    __HAL_RCC_ADC_CLK_ENABLE();
  }

}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc)
{
  if(hadc->Instance==ADC)
  {
    __HAL_RCC_ADC_CLK_DISABLE();
	}
}

void MX_ADC_Init(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};

  hadc.Instance = ADC;
  hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc.Init.Resolution = ADC_RESOLUTION_12B;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc.Init.LowPowerAutoWait = DISABLE;
  hadc.Init.LowPowerAutoPowerOff = DISABLE;
  hadc.Init.ContinuousConvMode = DISABLE;
  hadc.Init.NbrOfConversion = 1;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.DMAContinuousRequests = DISABLE;
  hadc.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  hadc.Init.SamplingTimeCommon1 = ADC_SAMPLETIME_160CYCLES_5;
  hadc.Init.SamplingTimeCommon2 = ADC_SAMPLETIME_160CYCLES_5;
  hadc.Init.OversamplingMode = DISABLE;
  hadc.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
  if (HAL_ADC_Init(&hadc) != HAL_OK)
  {
    Error_Handler();
  }
}

void smartfram_ReadSensorSoilMoisture(uint32_t *Data)
{
	
	uint16_t Hum = 0, Temp = 0, crc = 0;
	FS_ADC_LowpowerWakeupMode(1);
	HAL_Delay(500);
	vcom_Trace(Msg_Read,sizeof(Msg_Read));
  HAL_Delay(100);
	FS_ADC_LowpowerWakeupMode(0);

	if (Flag_Receive == 1)
	{
		char buffer_temp[50];
		crc = (Array_Receive[8]<<8) | Array_Receive[7];
		Flag_Receive = 0;
		if(FS_CheckSum((uint8_t*)Array_Receive, 7) == crc)
		{
			Hum  = (Array_Receive[3]<<8) | Array_Receive[4];
			Temp = (Array_Receive[5]<<8) | Array_Receive[6];
		}
	}
	if(Hum > 1000)
	{
		*Data = 1000;
	}
	else if(Hum < 0)
	{
		*Data  = 0;
	}
	else
	{
		*Data = Hum;
	}
}

uint32_t ADC_ReadChannels(uint32_t channel)
{
  uint32_t ADCxConvertedValues = 0;
  ADC_ChannelConfTypeDef sConfig = {0};
  MX_ADC_Init();

  if (HAL_ADCEx_Calibration_Start(&hadc) != HAL_OK)
  {
    Error_Handler();
  }

  sConfig.Channel = channel;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_ADC_Start(&hadc) != HAL_OK)
  {
    Error_Handler();
  }
	
  HAL_ADC_PollForConversion(&hadc, HAL_MAX_DELAY);


  HAL_ADC_Stop(&hadc) ;  

  ADCxConvertedValues = HAL_ADC_GetValue(&hadc);

  HAL_ADC_DeInit(&hadc);

  return ADCxConvertedValues;
}

uint16_t FS_CheckSum(uint8_t *buf, uint8_t len)
{
	uint16_t crc = 0xFFFF, pos, i;
  for ( pos = 0; pos < len; pos++)
  {
  crc ^= (uint16_t)buf[pos];    // XOR byte into least sig. byte of crc

  for ( i = 8; i != 0; i--) {    // Loop over each bit
    if ((crc & 0x0001) != 0) {      // If the LSB is set
      crc >>= 1;                    // Shift right and XOR 0xA001
      crc ^= 0xA001;
    }
    else                            // Else LSB is not set
      crc >>= 1;                    // Just shift right
    }
  }
  return crc;
}