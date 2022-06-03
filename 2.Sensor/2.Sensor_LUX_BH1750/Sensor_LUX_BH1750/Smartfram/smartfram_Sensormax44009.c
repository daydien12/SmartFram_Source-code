
#include "smartfram_Sensormax44009.h"

I2C_HandleTypeDef 	*max44009_i2c;	// Handler to I2C interface


MAX44009_STATUS MAX44009_Init(I2C_HandleTypeDef *hi2c)
{
	uint8_t Config = 0;
	max44009_i2c = hi2c;
	MAX44009_ContinuousMode(1);
	MAX44009_CurrentDivisionRatio(1);
	return MAX44009_OK;
}


MAX44009_STATUS MAX44009_ReadInterruptStatus(uint8_t *Status)
{
	if(HAL_OK == HAL_I2C_Mem_Read(max44009_i2c, MAX44009_ADDRESS, MAX44009_INTERRUPT_STATUS_REGISTER, 1, Status, 1, 10))
	{
		return MAX44009_OK;
	}

	return MAX44009_ERROR;
}


MAX44009_STATUS MAX44009_WriteInterruptEnable(uint8_t Enable)
{
	Enable = (Enable? 1:0);
	if(HAL_OK == HAL_I2C_Mem_Write(max44009_i2c, MAX44009_ADDRESS, MAX44009_INTERRPUT_ENABLE_REGISTER, 1, &Enable, 1, 10))
	{
		return MAX44009_OK;
	}

	return MAX44009_ERROR;
}


MAX44009_STATUS MAX44009_ReadConfigurationRegister(uint8_t *Config)
{
	if(HAL_OK == HAL_I2C_Mem_Read(max44009_i2c, MAX44009_ADDRESS, MAX44009_CONFIGURATION_REGISTER, 1, Config, 1, 10))
	{
		return MAX44009_OK;
	}

	return MAX44009_ERROR;
}


MAX44009_STATUS MAX44009_WriteConfigurationRegister(uint8_t Config)
{
	if(HAL_OK == HAL_I2C_Mem_Write(max44009_i2c, MAX44009_ADDRESS, MAX44009_CONFIGURATION_REGISTER, 1, &Config, 1, 10))
	{
		return MAX44009_OK;
	}

	return MAX44009_ERROR;
}


MAX44009_STATUS MAX44009_ContinuousMode(uint8_t Enable)
{
	uint8_t Config;

	Enable = (Enable? 1:0);

	if(MAX44009_OK == MAX44009_ReadConfigurationRegister(&Config))
	{
		Config &= 0x7F; // Clear BIT7
		Config |= (Enable<<7);
		if(MAX44009_OK == MAX44009_WriteConfigurationRegister(Config))
		{
			return MAX44009_OK;
		}
	}
	return MAX44009_ERROR;
}


MAX44009_STATUS MAX44009_ManualConfiguration(uint8_t Enable)
{
	uint8_t Config;

	Enable = (Enable? 1:0);

	if(MAX44009_OK == MAX44009_ReadConfigurationRegister(&Config))
	{
		Config &= 0xBF; // Clear BIT6
		Config |= (Enable<<6);
		if(MAX44009_OK == MAX44009_WriteConfigurationRegister(Config))
		{
			return MAX44009_OK;
		}
	}
	return MAX44009_ERROR;
}


MAX44009_STATUS MAX44009_CurrentDivisionRatio(uint8_t Enable)
{
	uint8_t Config;

	Enable = (Enable? 1:0);

	if(MAX44009_OK == MAX44009_ReadConfigurationRegister(&Config))
	{
		Config &= 0xF7; // Clear BIT3
		Config |= (Enable<<3);
		if(MAX44009_OK == MAX44009_WriteConfigurationRegister(Config))
		{
			return MAX44009_OK;
		}
	}
	return MAX44009_ERROR;
}


MAX44009_STATUS MAX44009_IntegrationTime(max44009_timer Timer)
{
	uint8_t Config;

	if(Timer > 7) Timer = 7;

	if(MAX44009_OK == MAX44009_ReadConfigurationRegister(&Config))
	{
		Config &= 0xF8; // Clear BIT[0:2]
		Config |= Timer;
		if(MAX44009_OK == MAX44009_WriteConfigurationRegister(Config))
		{
			return MAX44009_OK;
		}
	}
	return MAX44009_ERROR;
}

MAX44009_STATUS MAX44009_ReadLightLowResolution(float *Result)
{
	uint8_t tmp;

	if(HAL_OK == HAL_I2C_Mem_Read(max44009_i2c, MAX44009_ADDRESS, MAX44009_LUX_HIGH_BYTE_REGISTER, 1, &tmp, 1, 10))
	{
		uint8_t exponent = tmp>>4;
		uint32_t mantisa = tmp & 0x0F;
		mantisa <<= exponent;

		*Result = ((float)mantisa * 0.72);
		return MAX44009_OK;
	}
	return MAX44009_ERROR;
}
//RD_EDIT: ham doc cam bien anh sang BH1750
#define NEW_SENSOR_ADDR_H		(0x5c<<1)
#define NEW_SENSOR_ADDR_L		(0x23<<1)
#define NEW_SENSOR_ADDR 		NEW_SENSOR_ADDR_H
#define REGISTER_1				0x01
#define REGISTER_2				0x11
MAX44009_STATUS MAX44009_ReadLightHighResolution(float *Result)
{
	//*Result  = 200;
	unsigned char rigister_set = 0x01;
	unsigned char buf_i2c = 0x01;
	uint8_t tmp[3];
	HAL_I2C_Master_Transmit(&hi2c3, NEW_SENSOR_ADDR, &buf_i2c,1,20);
	HAL_Delay(1);
	buf_i2c = 0x11;
	HAL_I2C_Master_Transmit(&hi2c3, NEW_SENSOR_ADDR, &buf_i2c, 1,20);
	HAL_Delay(200);
	if(HAL_OK == HAL_I2C_Master_Receive(&hi2c3, NEW_SENSOR_ADDR,tmp,3,20))
	{
		*Result  = tmp[0]<<8|tmp[1];
		//*Result  = 200;
		return MAX44009_OK;
	}
	return MAX44009_ERROR;
	//return MAX44009_OK;
}

MAX44009_STATUS MAX44009_SetThreshold(uint8_t Register, float Threshold)
{
	uint8_t result;
	uint32_t mantisa = (uint32_t)(round(Threshold * 22.222222));
	uint8_t exponent = 0;

	while(mantisa > 255)
	{
		mantisa >>= 1;
		exponent++;
	}

	mantisa = (mantisa>>4) & 0x0F;
	exponent = (exponent<<4);
	result = exponent | mantisa;

	if(HAL_OK == HAL_I2C_Mem_Write(max44009_i2c, MAX44009_ADDRESS, Register, 1, &result, 1, 10))
	{
		return MAX44009_OK;
	}
	return MAX44009_ERROR;
}


MAX44009_STATUS MAX44009_SetUpperThreshold(float Threshold)
{
	if(MAX44009_OK == MAX44009_SetThreshold(MAX44009_UPPER_THRESHOLD_REGISTER, Threshold))
	{
		return MAX44009_OK;
	}
	return MAX44009_ERROR;
}

MAX44009_STATUS MAX44009_SetLowerThreshold(float Threshold)
{
	if(MAX44009_OK == MAX44009_SetThreshold(MAX44009_LOWER_THRESHOLD_REGISTER, Threshold))
	{
		return MAX44009_OK;
	}
	return MAX44009_ERROR;
}

MAX44009_STATUS MAX44009_SetThresholdTimer(uint8_t Timer)
{
	if(HAL_OK == HAL_I2C_Mem_Write(max44009_i2c, MAX44009_ADDRESS, MAX44009_THRESHOLD_TIMER_REGISTER, 1, &Timer, 1, 10))
	{
		return MAX44009_OK;
	}
	return MAX44009_ERROR;
}



