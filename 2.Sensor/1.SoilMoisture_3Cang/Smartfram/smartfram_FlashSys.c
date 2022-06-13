#include "smartfram_FlashSys.h"

#define FLASH_USER_START_ADDR   ADDR_FLASH_PAGE_125 
#define FLASH_USER_END_ADDR     (ADDR_FLASH_PAGE_125 + FLASH_PAGE_SIZE - 1)

#define FLASH_USER_ID_MESSEGER ADDR_FLASH_PAGE_126

static FLASH_EraseInitTypeDef EraseInitStruct;

deviceFlashData_t Sys_DeviceFlashData;
deviceMSGFlashData_t Sys_DeviceFlashData_MSG;

uint8_t FlagSys_CheckModeRun;

uint32_t FirstPage = 0, NbOfPages = 0;
uint32_t Address = 0, PageError = 0;
uint8_t vr_Sensor_ERROR = 0;
static uint32_t GetPage(uint32_t Address);

static void FS_FlashErase(void);
static void FS_DeviceFlash_Read(void);

static void FS_Flash_AutoClear(void);

void FS_DeviceFlash_Write(deviceFlashData_t *DataFlash);
void FS_FlashDevice_WriteMesseger(void);
void FS_FlashDevice_ReadMesseger(void);

static void FS_FlashUpdate_GetwayKey(void);
static void FS_FlashUpdate_DeviceKey(void);
static void FS_FlashUpdate_ModeRun(void);
void FS_FlashUpdate_All(void);

void FS_FlashDevice_Init(void);
void FS_FlashDevice_InitMsg(void);

void FS_FlashDevice_Debug(void);
void FS_FlashCreate_DeviceKey(uint32_t *data);

/**
	Mo ta: Ham luu thong tin ban tin accept
	Tham so:	
		-Str_char: là con tro 32bit
		-size: Size cua con tro dau vao 
**/
void smartfram_SaveJointInfoAccept(uint32_t *DataSaveFlash, uint8_t Size)
{
	uint32_t TempData[10];
	uint8_t i;
	for(i=0; i < Size; i++)
	{
		TempData[i] = DataSaveFlash[i];
	}
	
	// luu dia chi mac(64 bit) cua GW vao flash
	Sys_DeviceFlashData.MACGetway[0]	= DataSaveFlash[1];
	Sys_DeviceFlashData.MACGetway[1]	= DataSaveFlash[2];
	// luu Key cua GW (128 bit) vao flash
	Sys_DeviceFlashData.NetKey[0]			= DataSaveFlash[3];
	Sys_DeviceFlashData.NetKey[1] 		= DataSaveFlash[4];
	Sys_DeviceFlashData.NetKey[2] 		= DataSaveFlash[5];
	Sys_DeviceFlashData.NetKey[3] 		= DataSaveFlash[6];
	FS_FlashUpdate_GetwayKey();
	FS_DeviceFlash_Write(&Sys_DeviceFlashData);
	#if FLASH_DEBUG_SAVE
		APP_PPRINTF("\n(FLASH_BUG: Save Accept Done)\n"); 
	#endif
}

/**
	Mo ta: Ham luu thong tin ban tin Completed
	Tham so:	
		-Unicast: la dinh danh duoc GW cap cho DV
		-ModeRun: chuyen DV tu che do join sang che do run
**/
void smartfram_SaveJointInfoCompleted(uint32_t Unicast, uint8_t ModeRun)
{
	Sys_DeviceFlashData.Unicast = Unicast;
	Sys_DeviceFlashData.ModeRun = ModeRun;
	FS_DeviceFlash_Write(&Sys_DeviceFlashData);
	
	#if FLASH_DEBUG_SAVE
		APP_PPRINTF("\n(FLASH_BUG: Save Completed Done)\n"); 
		FS_FlashDevice_Debug();
	#endif
}
/**
	Mo ta: Ham tcap nhap thoi gian thuc giac trong flash
	Tham so:	
		-Time: thoi gian tinh bang Phut
**/
void smartfram_UpdateMSG_TimeWakeup(uint32_t Time)
{
	#if FLASH_DEBUG_UPDATE_TIME
		APP_PPRINTF("Time Update: %d \n\r", Time);
	#endif
	if(Time >= 1)
	{
		FS_FlashDevice_ReadMesseger();
		Sys_DeviceFlashData_MSG.TimeWakeup = Time;
		FS_FlashDevice_WriteMesseger();
	}
	else
	{
		FS_FlashDevice_ReadMesseger();
		Sys_DeviceFlashData_MSG.TimeWakeup = 3;
		FS_FlashDevice_WriteMesseger();
	}
}
/**
	Mo ta: Ham cap nhap trang thai gui ban tin
	Tham so:	
		-Stt_SendMiss: 1 gui that bai, 0 gui thanh cong
**/
void smartfram_UpdateMSG_SendMiss(uint32_t Stt_SendMiss)
{
	if((Stt_SendMiss == 0) || (Stt_SendMiss == 1))
	{
		FS_FlashDevice_ReadMesseger();
		Sys_DeviceFlashData_MSG.SendMiss = Stt_SendMiss;
		FS_FlashDevice_WriteMesseger();
	}
	else
	{
		FS_FlashDevice_ReadMesseger();
		Sys_DeviceFlashData_MSG.SendMiss = 0;
		FS_FlashDevice_WriteMesseger();
	}
}
/**
	Mo ta: Ham tang so ban tin
**/
void smartfram_IncrementIdMsg(void)
{
	FS_FlashDevice_ReadMesseger();
	Sys_DeviceFlashData_MSG.ID_Msg++;
	Sys_DeviceFlashData_MSG.SensorSoilMoisture = Sys_DataSensorRead.Data_Senser_SoilMoisture;
	Sys_DeviceFlashData_MSG.ERROR = vr_Sensor_ERROR;
	FS_FlashDevice_WriteMesseger();
	#if FLASH_DEBUG_INCREASE_IDMSG
		APP_PPRINTF("\n(FLASH_BUG: Increment IdMsg: %d)\n", Sys_DeviceFlashData_MSG.ID_Msg);
	#endif
}

/**
	Mo ta: Ham xoa flash tu dong
**/
static void FS_Flash_AutoClear(void)
{
	uint8_t FlagAutoClear = 0;
	FS_FlashUpdate_All();
	if((Sys_DeviceFlashData.ModeRun != 0) && (Sys_DeviceFlashData.ModeRun != 1))
	{
		FlagAutoClear = 1;
	}
	
	if(FlagAutoClear == 1)
	{
		FS_FlashDevice_Init();
		FS_FlashDevice_InitMsg();
	}
}

/**
	Mo ta: Ham khoi tao flash
**/
void smartfram_FlashEraseInit(void)
{
	int tempdata = 0;
	#if FLASH_CLEAR
		FS_FlashDevice_Init();
		FS_FlashDevice_InitMsg();
	#endif
	
	#if AUTO_CLEAR_FLASH
		FS_Flash_AutoClear();
	#endif
	
	/*
	if((Sys_DeviceFlashData_MSG.SendMiss != 0) && (Sys_DeviceFlashData_MSG.ID_Msg != 1))
	{
		smartfram_UpdateMSG_SendMiss(0);
	}
	*/
	
	tempdata = Sys_DeviceFlashData_MSG.TimeWakeup;
	if(tempdata <= 0)
	{
		smartfram_UpdateMSG_TimeWakeup(1);
	}
	
	FS_FlashUpdate_All();

	#if FLASH_DEBUG_INIT
		FS_FlashDevice_Debug();
	#endif
}

/**
	Mo ta: Ham xoa khoi mang
**/
void smartfram_RemoteNode(void)
{
	FS_FlashDevice_Init();
	FS_FlashDevice_InitMsg();
	HAL_Delay(100);
	NVIC_SystemReset();	
}


/**
	Mo ta: Ham khoi tao Flash luu thong tin mang
**/
void FS_FlashDevice_Init(void)
{
	uint32_t CreateKey[4];
	FS_FlashCreate_DeviceKey(CreateKey);
	 
	Sys_DeviceFlashData.ModeRun = 0x00;
	Sys_DeviceFlashData.TypeSensor = TypeSenserNode;
	Sys_DeviceFlashData.Unicast = 0x00;
	Sys_DeviceFlashData.Option = 0x00000000;
	Sys_DeviceFlashData.NetKey[0] = 0x00000000;
	Sys_DeviceFlashData.NetKey[1] = 0x00000000;
	Sys_DeviceFlashData.NetKey[2] = 0x00000000;
	Sys_DeviceFlashData.NetKey[3] = 0x00000000;
	Sys_DeviceFlashData.DeviceKey[0] = CreateKey[0]; 
	Sys_DeviceFlashData.DeviceKey[1] = CreateKey[1];
	Sys_DeviceFlashData.DeviceKey[2] = CreateKey[2];
	Sys_DeviceFlashData.DeviceKey[3] = CreateKey[3];
	Sys_DeviceFlashData.MACGetway[0] = 0x00000000;
	Sys_DeviceFlashData.MACGetway[1] = 0x00000000;
	FS_DeviceFlash_Write(&Sys_DeviceFlashData);
}


/**
	Mo ta: Ham khoi tao flash luu thong tin ban tin
**/
void FS_FlashDevice_InitMsg(void)
{
	Sys_DeviceFlashData_MSG.TimeWakeup = TIME_WAKEUP_INIT;
	Sys_DeviceFlashData_MSG.SendMiss =  1;
	Sys_DeviceFlashData_MSG.Calib_MIN =  0;
	Sys_DeviceFlashData_MSG.SensorSoilMoisture = 0;
	Sys_DeviceFlashData_MSG.ERROR = 0;
	Sys_DeviceFlashData_MSG.Calib_PAR_C = 0;
	Sys_DeviceFlashData_MSG.ID_Msg = 0;
	FS_FlashDevice_WriteMesseger();
}


/**
	Mo ta: Ham doc flash va cap nhau key
**/
void FS_FlashUpdate_All(void)
{
	FS_DeviceFlash_Read();
	FS_FlashDevice_ReadMesseger();
	FS_FlashUpdate_ModeRun();
	FS_FlashUpdate_GetwayKey();
	FS_FlashUpdate_DeviceKey();
	vr_Sensor_ERROR = Sys_DeviceFlashData_MSG.ERROR;
}


/**
	Mo ta: Ham tao Key random
	Tham so:	
		-data: la mot con tro 32 bit
**/
void FS_FlashCreate_DeviceKey(uint32_t *data)
{
	uint32_t counter = 0;
	uint32_t aRandom32bit[8];
	
	#if FLASH_DEBUG_CREATE_DEVICE_KEY
		APP_PPRINTF("\n(FLASH_BUG: Create New Device Key)\n");
	#endif
	Smartfram_RNG_Init();
	for (counter = 0; counter < 4; counter++)
	{
		if (HAL_RNG_GenerateRandomNumber(&hrng, &aRandom32bit[counter]) != HAL_OK)
		{
			
			Error_Handler();
		}
		data[counter] = aRandom32bit[counter];
		#if FLASH_DEBUG_CREATE_DEVICE_KEY
		 APP_PPRINTF("%x \n\r", data[counter]);
		#endif
	}
}

/**
	Mo ta: Ham cap nhap Netkey(GW key)
**/
static void FS_FlashUpdate_GetwayKey(void)
{
	pNetKeyAES[0] = Sys_DeviceFlashData.NetKey[0];
	pNetKeyAES[1] = Sys_DeviceFlashData.NetKey[1];
	pNetKeyAES[2] = Sys_DeviceFlashData.NetKey[2];
	pNetKeyAES[3] = Sys_DeviceFlashData.NetKey[3];
}


/**
	Mo ta: Ham cap nhap Device Key
**/
static void FS_FlashUpdate_DeviceKey(void)
{
	pLocalDeviceKeyAES[0] = Sys_DeviceFlashData.DeviceKey[0];
	pLocalDeviceKeyAES[1] = Sys_DeviceFlashData.DeviceKey[1];
	pLocalDeviceKeyAES[2] = Sys_DeviceFlashData.DeviceKey[2];
	pLocalDeviceKeyAES[3] = Sys_DeviceFlashData.DeviceKey[3];
}

/**
	Mo ta: Ham cap nhap trang thai hoat dong
**/
static void FS_FlashUpdate_ModeRun(void)
{
	FlagSys_CheckModeRun = Sys_DeviceFlashData.ModeRun;
}

/**
	Mo ta: Ham Debug
**/
void FS_FlashDevice_Debug(void)
{
	char buffer_temp[30];
	HAL_Delay(1000);
	APP_PPRINTF("\n------------------------------------------ \n");
	APP_PPRINTF("ModeRun: %x \n", Sys_DeviceFlashData.ModeRun);
	APP_PPRINTF("TypeSensor: %x \n", Sys_DeviceFlashData.TypeSensor);
	APP_PPRINTF("Option: %x \n", Sys_DeviceFlashData.Option);
	APP_PPRINTF("Unicast: %x \n", Sys_DeviceFlashData.Unicast);
	APP_PPRINTF("DeviceKey[0]: %x \n", Sys_DeviceFlashData.DeviceKey[0]);
	APP_PPRINTF("DeviceKey[1]: %x \n", Sys_DeviceFlashData.DeviceKey[1]);
	APP_PPRINTF("DeviceKey[2]: %x \n", Sys_DeviceFlashData.DeviceKey[2]);
	APP_PPRINTF("DeviceKey[3]: %x \n", Sys_DeviceFlashData.DeviceKey[3]);
	APP_PPRINTF("MACThietbi[0]: %x \n", Smartfram_GetMACAdress1());
	APP_PPRINTF("MACThietbi[1]: %x \n", Smartfram_GetMACAdress2());
	APP_PPRINTF("NetKey[0]: %x \n", Sys_DeviceFlashData.NetKey[0]);
	APP_PPRINTF("NetKey[1]: %x \n", Sys_DeviceFlashData.NetKey[1]);
	APP_PPRINTF("NetKey[2]: %x \n", Sys_DeviceFlashData.NetKey[2]);
	APP_PPRINTF("NetKey[3]: %x \n", Sys_DeviceFlashData.NetKey[3]);
	APP_PPRINTF("MACGetway[0]: %x \n", Sys_DeviceFlashData.MACGetway[0]);
	APP_PPRINTF("MACGetway[1]: %x \n", Sys_DeviceFlashData.MACGetway[1]);
	APP_PPRINTF("Sys ID Messeger %d \n",Sys_DeviceFlashData_MSG.ID_Msg);
	APP_PPRINTF("Size: %d\n", sizeof(deviceFlashData_t));
	
	APP_PPRINTF("TimeWakeup: %d \n"	, Sys_DeviceFlashData_MSG.TimeWakeup);
	
	APP_PPRINTF("SendMiss: %d \n"		,Sys_DeviceFlashData_MSG.SendMiss);
	
	sprintf(buffer_temp, "%f",Sys_DeviceFlashData_MSG.Calib_MIN);
	APP_PPRINTF("Calib_MIN: %s \n"		,buffer_temp);
	
	sprintf(buffer_temp, "%d",Sys_DeviceFlashData_MSG.SensorSoilMoisture);
	APP_PPRINTF("Calib_PAR_A: %s \n"	,buffer_temp);
	
	sprintf(buffer_temp, "%d",Sys_DeviceFlashData_MSG.ERROR);
	APP_PPRINTF("Calib_PAR_B: %s \n"	,buffer_temp);
	
	sprintf(buffer_temp, "%f",Sys_DeviceFlashData_MSG.Calib_PAR_C);
	APP_PPRINTF("Calib_PAR_C: %s \n"	,buffer_temp);
	
	APP_PPRINTF("ID_Msg: %d \n"			, Sys_DeviceFlashData_MSG.ID_Msg);
}

static uint32_t GetPage(uint32_t Addr)
{
  return (Addr - FLASH_BASE) / FLASH_PAGE_SIZE;;
}
static void FS_FlashErase(void)
{
	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);
	
	FirstPage = GetPage(FLASH_USER_START_ADDR);
	NbOfPages = GetPage(FLASH_USER_END_ADDR) - FirstPage + 1;
	EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.Page        = FirstPage;
  EraseInitStruct.NbPages     = NbOfPages;
	
	if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
  { 
		#if 0
			APP_PPRINTF("\n FlashErase Error\n");
		#endif
  }
	
	HAL_FLASH_Lock();
}

void FS_FlashDevice_WriteMesseger(void)
{
	uint8_t i;
	uint64_t *TempFlashData;
	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);
	FirstPage = GetPage(FLASH_USER_ID_MESSEGER);
	NbOfPages = GetPage(FLASH_USER_ID_MESSEGER) - FirstPage + 1;
	EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.Page        = FirstPage;
  EraseInitStruct.NbPages     = NbOfPages;
	
	if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
  {
		#if 0
			APP_PPRINTF("\n FlashErase Error\n");
		#endif
  }

	Address = FLASH_USER_ID_MESSEGER;
	TempFlashData = (uint64_t*)&Sys_DeviceFlashData_MSG;
	for(i=0; i < 4; i++)
	{
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, Address, TempFlashData[i]) == HAL_OK)
    {
      Address = Address + 8;
    }
	}
	HAL_FLASH_Lock();
}

void FS_FlashDevice_ReadMesseger(void)
{
	deviceMSGFlashData_t *TempData;
	uint8_t i;
	uint8_t DataRead[FLASH_SIZE_DATA_READ_MSG]={0}; 
	
	Address = FLASH_USER_ID_MESSEGER;
	for(i=0; i < FLASH_SIZE_DATA_READ_MSG; i++)
	{
		DataRead[i] = *(__IO uint8_t *)Address;
		Address = Address + 1;
	}
	TempData = (deviceMSGFlashData_t*)&DataRead;
	
	Sys_DeviceFlashData_MSG.TimeWakeup 	= TempData->TimeWakeup;
	Sys_DeviceFlashData_MSG.SendMiss 		= TempData->SendMiss;
	Sys_DeviceFlashData_MSG.Calib_MIN 	= TempData->Calib_MIN;
	Sys_DeviceFlashData_MSG.SensorSoilMoisture = TempData->SensorSoilMoisture;
	Sys_DeviceFlashData_MSG.ERROR = TempData->ERROR;
	Sys_DeviceFlashData_MSG.Calib_PAR_C = TempData->Calib_PAR_C;
	Sys_DeviceFlashData_MSG.ID_Msg 			= TempData->ID_Msg;
}

void FS_DeviceFlash_Write(deviceFlashData_t *DataFlash)
{
	uint8_t i;
	uint64_t *TempFlashData = (uint64_t*)DataFlash;
	Address = FLASH_USER_START_ADDR;
	FS_FlashErase();
	HAL_FLASH_Unlock();
	for(i=0; i < FLASH_SIZE_DATA_WRITE; i++)
	{
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, Address, TempFlashData[i]) == HAL_OK)
    {
      Address = Address + 8;
    }
	}
	HAL_FLASH_Lock();
}

void FS_DeviceFlash_Read()
{
	deviceFlashData_t *TempData;
	uint8_t i;
	uint8_t DataRead[FLASH_SIZE_DATA_READ]={0}; 
	
	Address = FLASH_USER_START_ADDR;
	for(i=0; i < FLASH_SIZE_DATA_READ; i++)
	{
		DataRead[i] = *(__IO uint8_t *)Address;
		Address = Address + 1;
	}
	
	TempData = (deviceFlashData_t*)&DataRead;
	
	Sys_DeviceFlashData.ModeRun 			= TempData->ModeRun;
	Sys_DeviceFlashData.TypeSensor 		= TempData->TypeSensor;
	Sys_DeviceFlashData.Option 				= TempData->Option;
	Sys_DeviceFlashData.Unicast 			= TempData->Unicast;
	Sys_DeviceFlashData.DeviceKey[0] 	= TempData->DeviceKey[0]; 
	Sys_DeviceFlashData.DeviceKey[1] 	= TempData->DeviceKey[1];
	Sys_DeviceFlashData.DeviceKey[2] 	= TempData->DeviceKey[2];
	Sys_DeviceFlashData.DeviceKey[3] 	= TempData->DeviceKey[3];
	Sys_DeviceFlashData.NetKey[0] 		= TempData->NetKey[0];
	Sys_DeviceFlashData.NetKey[1] 		= TempData->NetKey[1];
	Sys_DeviceFlashData.NetKey[2] 		= TempData->NetKey[2];
	Sys_DeviceFlashData.NetKey[3]			= TempData->NetKey[3];
	Sys_DeviceFlashData.MACGetway[0]	= TempData->MACGetway[0];
	Sys_DeviceFlashData.MACGetway[1]	= TempData->MACGetway[1];
}