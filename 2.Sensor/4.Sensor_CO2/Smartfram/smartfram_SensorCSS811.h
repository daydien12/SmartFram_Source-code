
#ifndef __smartfram_SensorCSS811_H__
#define __smartfram_SensorCSS811_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "smartfram_SysInit.h"
#include "smartfram_define.h"

#define CCS811_Add  0x5A
#define CCS811_HW_ID_CODE 0x81

enum {
  CCS811_STATUS = 0x00,
  CCS811_MEAS_MODE = 0x01,
  CCS811_ALG_RESULT_DATA = 0x02,
  CCS811_RAW_DATA = 0x03,
  CCS811_ENV_DATA = 0x05,
  CCS811_NTC = 0x06,
  CCS811_THRESHOLDS = 0x10,
  CCS811_BASELINE = 0x11,
  CCS811_HW_ID = 0x20,
  CCS811_HW_VERSION = 0x21,
  CCS811_FW_BOOT_VERSION = 0x23,
  CCS811_FW_APP_VERSION = 0x24,
  CCS811_ERROR_ID = 0xE0,
  CCS811_SW_RESET = 0xFF,
};

enum {
  CCS811_BOOTLOADER_APP_ERASE = 0xF1,
  CCS811_BOOTLOADER_APP_DATA = 0xF2,
  CCS811_BOOTLOADER_APP_VERIFY = 0xF3,
  CCS811_BOOTLOADER_APP_START = 0xF4
};

enum {
  CCS811_DRIVE_MODE_IDLE = 0x00,
  CCS811_DRIVE_MODE_1SEC = 0x01,
  CCS811_DRIVE_MODE_10SEC = 0x02,
  CCS811_DRIVE_MODE_60SEC = 0x03,
  CCS811_DRIVE_MODE_250MS = 0x04,
};

uint8_t Smartfram_SensorCO2_Init(I2C_HandleTypeDef *hi2c, uint8_t Mode);
uint8_t Smartfram_SensorRead(uint32_t *DataCO2, uint32_t *DataTVOC);
uint8_t Smartfram_Available();
#ifdef __cplusplus
}
#endif

#endif
