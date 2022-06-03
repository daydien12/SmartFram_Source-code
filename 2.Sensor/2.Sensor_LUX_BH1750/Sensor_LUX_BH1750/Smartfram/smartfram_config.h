
#ifndef _SYS_CONF_H_
#define _SYS_CONF_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
  * @brief Maximum battery level in mV
  */
#define VDD_BAT            (370)

/**
  * @brief Minimum battery level in mV
  */
#define VDD_MIN              300

/**
  * Defines the maximum battery level
  */
#define LORAWAN_MAX_BAT   100

/* Thoi gian thuc giac */
#define TIME_WAKEUP_INIT 2
#define APP_TASK_TEST 	 0
/* Chon loai cam bien */
enum
{
	SENSOR_HUM_TEMP,
	NONE,
	SENSOR_SOIL_MOISTURE,
  SENSOR_LIGHT,
	SENSOR_CO2,
};
#define TypeSenserNode SENSOR_LIGHT
/* Clear Flash*/
#define FLASH_CLEAR      			0

#ifdef __cplusplus
}
#endif

#endif
