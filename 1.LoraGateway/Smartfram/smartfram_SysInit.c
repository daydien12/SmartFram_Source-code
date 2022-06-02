
#include "smartfram_SysInit.h"

sysCountTime_t Sys_CountTime;
varAndFlagSys_t Sys_VarAndStt;


void MX_IWDG_Init(float time_out)
{
__HAL_RCC_CLEAR_RESET_FLAGS();
 	uint16_t PrescalerCode;
  uint16_t Prescaler;
  uint16_t ReloadValue;
	uint16_t LsiFreq = 32000;
	if( (time_out *LsiFreq)/4 <= 4000)
	{
		PrescalerCode = 0x00;
		Prescaler = 4;
	}
	else if((time_out *LsiFreq)/8 <= 4000)
	{
		PrescalerCode = 0x01;
		Prescaler = 8;
	}
	else if((time_out *LsiFreq)/16 <= 4000)
	{
		PrescalerCode = 0x02;
		Prescaler = 16;
	}
	else if((time_out *LsiFreq)/32 <= 4000)
	{
		PrescalerCode = 0x03;
		Prescaler = 32;
	}
	else if((time_out *LsiFreq)/64 <= 4000)
	{
		PrescalerCode = 0x04;
		Prescaler = 64;
	}
	else if((time_out *LsiFreq)/128 <= 4000)
	{
		PrescalerCode = 0x05;
		Prescaler = 128;
	}
	else if((time_out *LsiFreq)/256 <= 4000)
	{
		PrescalerCode = 0x06;
		Prescaler = 256;
	}
	
//Calculated_timeout = ((Prescaler * ReloadValue)) / LsiFreq; 
	
	ReloadValue = (uint32_t)((time_out * LsiFreq)/Prescaler);
	IWDG->KR = 0x5555;      
  IWDG->PR =  PrescalerCode;       
  IWDG->RLR = ReloadValue;     
  IWDG->KR = 0xAAAA;
  IWDG->KR = 0xCCCC;  

}

void MX_IWDG_Reload(void)
{
  IWDG->KR = 0xAAAA; 
}