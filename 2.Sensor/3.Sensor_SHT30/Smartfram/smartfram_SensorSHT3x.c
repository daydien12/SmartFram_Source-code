#include "smartfram_SensorSHT3x.h"

I2C_HandleTypeDef 	*sht30_i2c;
uint8_t writeCommand(uint16_t command);

float humidity;
float temp;

static void  reset();
uint16_t readStatus(void);
static uint8_t crc8(const uint8_t *data, int len);
uint8_t writeCommand(uint16_t command)
{
	uint8_t command_buffer[2] = {(command & 0xff00u) >> 8u, command & 0xffu};
	if (HAL_I2C_Master_Transmit(sht30_i2c, (SHT31_DEFAULT_ADDR << 1u), command_buffer, sizeof(command_buffer),100) != HAL_OK) 
	{
		return 0;
	}
	return 1;
}

static void reset()
{
	writeCommand(SHT31_SOFTRESET);
	HAL_Delay(10);
}

uint16_t readStatus(void)
{
	uint8_t data[3];
	uint16_t stat;
	writeCommand(SHT31_READSTATUS);
	if (HAL_I2C_Master_Receive(sht30_i2c, (SHT31_DEFAULT_ADDR << 1u), (uint8_t*)&data, sizeof(data), 10) != HAL_OK) {
		return 0;
	}
	stat = data[0];
	stat <<= 8;
  stat |= data[1];
	return stat;
}

static uint8_t crc8(const uint8_t *data, int len) {
  const uint8_t POLYNOMIAL = 0x31;
  uint8_t crc = 0xFF;

  for (int j = len; j; --j) {
    crc ^= *data++;

    for (int i = 8; i; --i) {
      crc = (crc & 0x80) ? (crc << 1) ^ POLYNOMIAL : (crc << 1);
    }
  }
  return crc;
}

uint8_t SmartFram_SHT30_Init(I2C_HandleTypeDef *hi2c)
{
	sht30_i2c = hi2c;
	reset();
	return readStatus() != 0xFFFF;
}

void SmartFram_SHT30_SetHeater(uint8_t h)
{
	 if (h)
    writeCommand(SHT31_HEATEREN);
  else
    writeCommand(SHT31_HEATERDIS);
  HAL_Delay(1);
}

uint8_t SmartFram_SHT30_isHeaterEnabled()
{
	uint16_t regValue = readStatus();
	
  return ((regValue >> SHT31_REG_HEATER_BIT)&1);
}

float SmartFram_SHT30_ReadTemperature(void)
{
	//if (!SmartFram_SHT30_ReadTempHum())
   // return 0;
  return temp;
}

float SmartFram_SHT30_ReadHumidity(void)
{
	 //if (!SmartFram_SHT30_ReadTempHum())
   // return 0;
  return humidity;
}

uint8_t SmartFram_SHT30_ReadTempHum()
{
	uint8_t readbuffer[6] ={0}, i;

  writeCommand(SHT31_MEAS_HIGHREP);

  HAL_Delay(50);

 if (HAL_I2C_Master_Receive(sht30_i2c, (SHT31_DEFAULT_ADDR << 1u), (uint8_t *)readbuffer, sizeof(readbuffer), 1000) != HAL_OK)
	{
		return 0;
	}
  if (readbuffer[2] != crc8(readbuffer, 2) ||
      readbuffer[5] != crc8(readbuffer + 3, 2))
    return 0;
	
  int32_t stemp = (int32_t)(((uint32_t)readbuffer[0] << 8) | readbuffer[1]);
  stemp = ((4375 * stemp) >> 14) - 4500;
  temp = (float)stemp / 100.0f;
  uint32_t shum = ((uint32_t)readbuffer[3] << 8) | readbuffer[4];
  shum = (625 * shum) >> 12;
  humidity = (float)shum / 100.0f;
  return 1;
}