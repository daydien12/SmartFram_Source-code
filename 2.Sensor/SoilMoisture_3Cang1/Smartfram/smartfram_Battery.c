#include "smartfram_Battery.h"

void smartfram_GetBatteryLevel(uint32_t *Data)
{
	uint8_t i;
	uint32_t measuredLevel = 0;
	uint32_t batteryLevel = 0;
	
	for(i=0; i<100; i++)
	{
		measuredLevel += ADC_ReadChannels(ADC_CHANNEL_7);
	}
	measuredLevel /= 1000; 
	
	if (measuredLevel > VDD_BAT)
  {
    batteryLevel = LORAWAN_MAX_BAT;
  }
  else if (measuredLevel < VDD_MIN)
  {
    batteryLevel = 0;
  }
  else
  {
    batteryLevel = (((uint32_t)(measuredLevel - VDD_MIN) * LORAWAN_MAX_BAT) / (VDD_BAT - VDD_MIN));
  }
 *Data = batteryLevel;
}