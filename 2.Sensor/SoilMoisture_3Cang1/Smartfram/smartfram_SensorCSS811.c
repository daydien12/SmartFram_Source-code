#include "smartfram_SensorCSS811.h"
I2C_HandleTypeDef 	*CSS811_i2c;	

typedef struct
{
	uint8_t ERROR;
	uint8_t DATA_READY;
	 uint8_t APP_VALID;
	uint8_t FW_MODE;
}status;
status CS811_Status;

typedef struct
{
	uint8_t INT_THRESH;
	uint8_t INT_DATARDY;
	uint8_t DRIVE_MODE;
}meas_mode;
meas_mode CS811_MeasMode;

uint8_t Status;

uint8_t CheckStatus = 0;
void FS_CSS811_SWReset();
void FS_CSS811_SetStatus(uint8_t data);
void FS_CSS811_InitStatus();

void FS_CSS811_InitMeasMode();
uint8_t FS_CSS811_GetMeasMode();
void FS_CS811_disableInterrupt();
void FS_CS811_SetDriverMode(uint8_t data);

uint8_t Smartfram_SensorCO2_Init(I2C_HandleTypeDef *hi2c, uint8_t Mode)
{
	uint8_t command_buffer[1];
	CSS811_i2c = hi2c;
	FS_CSS811_InitStatus();
	FS_CSS811_SWReset();
	HAL_Delay(100);
	HAL_I2C_Mem_Read(CSS811_i2c, (CCS811_Add << 1u), CCS811_HW_ID,1, &Status, 1, 10);
	if(Status != CCS811_HW_ID_CODE)
	{
		return 0;
	}
	
	command_buffer[0] = CCS811_BOOTLOADER_APP_START;
	HAL_I2C_Master_Transmit(CSS811_i2c, (CCS811_Add << 1u), command_buffer, 1,100);
	HAL_Delay(100);
	HAL_I2C_Mem_Read(CSS811_i2c, (CCS811_Add << 1u), CCS811_STATUS,1, &Status,1, 10);
	FS_CSS811_SetStatus(Status);
	if (CS811_Status.ERROR)
    return 0;
  if (!CS811_Status.FW_MODE)
    return 0;
	
	FS_CS811_disableInterrupt();
	FS_CS811_SetDriverMode(Mode);
	
	//while (!Smartfram_Available());
	return 1;
}

uint8_t Smartfram_SensorRead(uint32_t *DataCO2, uint32_t *DataTVOC)
{
	if (Smartfram_Available()) 
	{
		uint8_t buf[8];
		HAL_I2C_Mem_Read(CSS811_i2c, (CCS811_Add << 1u), CCS811_ALG_RESULT_DATA,1, buf,8, 1000);
		*DataCO2 = ((uint16_t)buf[0] << 8) | ((uint16_t)buf[1]);
		*DataTVOC = ((uint16_t)buf[2] << 8) | ((uint16_t)buf[3]);
		return 1;
	}
	else
	{
		/*
		uint8_t buf[8];
		HAL_I2C_Mem_Read(CSS811_i2c, (CCS811_Add << 1u), CCS811_ALG_RESULT_DATA,1, buf,8, 1000);
		*DataCO2 = ((uint16_t)buf[0] << 8) | ((uint16_t)buf[1]);
		*DataTVOC = ((uint16_t)buf[2] << 8) | ((uint16_t)buf[3]);
		*/
	}
	return 0;
}

uint8_t Smartfram_Available()
{
	HAL_I2C_Mem_Read(CSS811_i2c, (CCS811_Add << 1u), CCS811_STATUS,1, &Status,1, 10);
	FS_CSS811_SetStatus(Status);
	if(!CS811_Status.DATA_READY)
	{
		return 0;
	}
	return 1;
}

void FS_CSS811_SWReset()
{
	 uint8_t seq[] = {0x11, 0xE5, 0x72, 0x8A};
	 HAL_I2C_Mem_Write(CSS811_i2c, (CCS811_Add << 1u), CCS811_SW_RESET, 1, seq, 4, 100);
}

void FS_CSS811_SetStatus(uint8_t data)
{
	CS811_Status.ERROR = data & 0x01;
	CS811_Status.DATA_READY = (data >> 3) & 0x01;
	CS811_Status.APP_VALID = (data >> 4) & 0x01;
	CS811_Status.FW_MODE = (data >> 7) & 0x01;
}

void FS_CSS811_InitStatus()
{
	CS811_Status.ERROR      = 0;
	CS811_Status.DATA_READY = 0;
	CS811_Status.APP_VALID  = 0;
	CS811_Status.FW_MODE    = 0;
}

void FS_CS811_disableInterrupt()
{
	CS811_MeasMode.DRIVE_MODE   = 0;
	CS811_MeasMode.INT_DATARDY  = 0;
	CS811_MeasMode.INT_THRESH 	= 0;
	uint8_t seq[] = {FS_CSS811_GetMeasMode()};
	HAL_I2C_Mem_Write(CSS811_i2c, (CCS811_Add << 1u), CCS811_MEAS_MODE, 1, seq, 1, 100);
}

void FS_CS811_SetDriverMode(uint8_t data)
{
	CS811_MeasMode.DRIVE_MODE   = data;
	CS811_MeasMode.INT_DATARDY  = 0;
	CS811_MeasMode.INT_THRESH 	= 0;
	uint8_t seq[] = {FS_CSS811_GetMeasMode()};
	HAL_I2C_Mem_Write(CSS811_i2c, (CCS811_Add << 1u), CCS811_MEAS_MODE, 1, seq, 1, 100);
}

uint8_t FS_CSS811_GetMeasMode()
{
	return (CS811_MeasMode.INT_THRESH << 2) | (CS811_MeasMode.INT_DATARDY << 3) | (CS811_MeasMode.DRIVE_MODE << 4);
}
