
#include "smartfram_SensorCo2.h"
#include "stm32_adv_trace.h"
#include "usart_if.h"
#define GPIO_WAKEUP_SENSOR_ADC GPIO_PIN_1
#define PORT_WAKEUP_SENSOR_ADC GPIOC
#define CLK_WAKEUP_ADC_SENSOR_ENABLE()                  __HAL_RCC_GPIOC_CLK_ENABLE()
#define CLK_WAKEUP_ADC_SENSOR_DISABLE()                 __HAL_RCC_GPIOC_CLK_ENABLE()

#define SIZE_DATA_READ 8
uint8_t Msg_Read[SIZE_DATA_READ] = {0x01, 0x03, 0x00, 0x02, 0x00, 0x01, 0x25, 0xCA};
uint16_t FS_CheckSum(uint8_t *buf, uint8_t len);

void smartfram_CO2_LowpowerWakeupMode(uint8_t mode)
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
	}
}


void smartfram_ReadSensorCO2(uint32_t *Data)
{
	
	uint16_t TempCO2 = 0, crc = 0;
	smartfram_CO2_LowpowerWakeupMode(1);
	HAL_Delay(500);
	vcom_Trace(Msg_Read,sizeof(Msg_Read));
  HAL_Delay(200);
	//smartfram_CO2_LowpowerWakeupMode(0);
	if (Flag_Receive == 1)
	{
		Flag_Receive = 0;
		crc = (Array_Receive[6]<<8) | Array_Receive[5];
		if(FS_CheckSum((uint8_t*)Array_Receive, 5) == crc)
		{
			TempCO2  = ((Array_Receive[3]<<8) | Array_Receive[4]);
			*Data = TempCO2;
		}
	}
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