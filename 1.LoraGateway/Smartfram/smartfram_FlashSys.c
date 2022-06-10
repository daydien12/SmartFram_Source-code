
#include "smartfram_FlashSys.h"
#include <stdlib.h>

/*Vung flash de luu thong tin Device 2 PAGE: 126,127*/
#define FLASH_USER_START_ADDR   ADDR_FLASH_PAGE_126   											
#define FLASH_USER_END_ADDR     (ADDR_FLASH_PAGE_127 + FLASH_PAGE_SIZE - 1)   

/*Vung flash de luu thong tin cua Getway Key và Unicast*/
#define FLASH_USER_NETKEY_ADDR ADDR_FLASH_PAGE_125

static FLASH_EraseInitTypeDef EraseInitStruct;
GetwayFlashData_t Sys_GetwayFlashArrData[MAX_ADD_DEVICE] = {0};
static void FS_Flash_AutoClear(void);
uint8_t FlagSys_CheckModeRun;

uint32_t Sys_Unicast = 0;
uint8_t Sys_Page = 0, Flag_CheckMAC = 0;

uint32_t FirstPage = 0, NbOfPages = 0;
uint32_t Address = 0, PageError = 0;



static void FS_FlashGetway_Erase(void); /* cân doi teno*/
static void FS_FlashGetway_Write(void);
static void FS_FlashGetway_Read(void);
static void FS_FlashGetway_WriteUnicastAndGetwayKey(uint8_t mode);
static void FS_FlashGetway_ReadUnicastAndGetwayKey(void);


static void FS_FlashGetway_UpdateDeviceKey(uint8_t page);
static void FS_FlashGetway_UpdateModeRun(void);
static uint32_t FS_FlashGetway_GetPage(uint32_t Address);

static void FS_FlashGetway_InitGetway(void);
static void FS_FlashGetway_InitUnicastAndGetwayKey(void);
static void FS_FlashGetway_ReadInit(void);
static void FS_FlashGetway_DebugPage(void);


void FS_FlashGetway_CreateGetwayKey(uint32_t *data, uint8_t mode);
void FS_FlashGetway_SetStatusMarkArray(uint8_t stt, uint8_t page);
void FS_FlashGetway_IncreaseUnicast(void);

uint8_t FS_FlashGetway_FindPageEmpty(void);
uint8_t FS_FlashGetway_CheckMacDevicey(uint32_t Data1, uint32_t Data2);

static void FS_Flash_AutoClear(void)
{
	uint8_t FlagAutoClear = 0;
	FS_FlashGetway_ReadInit();
	if((pLocalGetwayKeyAES[0] == 0xffffffff) || (pLocalGetwayKeyAES[1] == 0xffffffff)||(pLocalGetwayKeyAES[2] == 0xffffffff) || (pLocalGetwayKeyAES[3] == 0xffffffff))
	{
		FlagAutoClear = 1;
	}
	
	if(FlagAutoClear == 1)
	{
		FS_FlashGetway_InitGetway();
		FS_FlashGetway_InitUnicastAndGetwayKey();
	}
}


void smartfram_FlashEraseInit(void)
{
	uint8_t tt;
	#if SYS_FLASH_CLEAR
		FS_FlashGetway_InitGetway();
		FS_FlashGetway_InitUnicastAndGetwayKey();
	#endif
	FS_Flash_AutoClear();
	FS_FlashGetway_ReadInit();
	
	#if FLASH_DEBUG_INIT
		FS_FlashGetway_DebugPage();
	#endif
}

void smartfram_SaveJointInfoRequetst(uint32_t *DataSaveFlash, uint8_t Size)
{
	uint8_t  i;
	uint32_t *arr = DataSaveFlash, TempData[4];
	
	for(i=0; i<3; i++)
	{
		TempData[i] = arr[i];
	}
	
	Sys_Page = FS_FlashGetway_CheckMacDevicey(TempData[1], TempData[2]); //kiem tra dia chi MAC nay da ton tai trong tap data chua
	if(Sys_Page < MAX_ADD_DEVICE) // Sys_Page < 128 da ton tai
	{
		Flag_CheckMAC = 1; // co bao dia chi MAC da ton tai
	}
	else // Sys_Page > 128 chua ton tai
	{
		Flag_CheckMAC = 0;
		Sys_Page = FS_FlashGetway_FindPageEmpty(); // tim xem page nao chua su dung
		Sys_GetwayFlashArrData[Sys_Page].MACDevice[0] = TempData[1];
		Sys_GetwayFlashArrData[Sys_Page].MACDevice[1] = TempData[2];
		FS_FlashGetway_Write();
	}
	#if FLASH_DEBUG_SAVE
		APP_PPRINTF("\n(FLASH_BUG: Save Requetst Done)\n"); 
	#endif
}

void smartfram_SaveJointInfoConfirm(uint32_t *DataSaveFlash, uint8_t Size)
{
	uint32_t *arr = DataSaveFlash;
	Sys_GetwayFlashArrData[Sys_Page].TypeSensor = arr[1];
	Sys_GetwayFlashArrData[Sys_Page].DeviceKey[0] = arr[4];
	Sys_GetwayFlashArrData[Sys_Page].DeviceKey[1] = arr[5];
	Sys_GetwayFlashArrData[Sys_Page].DeviceKey[2] = arr[6];
	Sys_GetwayFlashArrData[Sys_Page].DeviceKey[3] = arr[7];
	FS_FlashGetway_UpdateDeviceKey(Sys_Page); //Thuc hien update key de ma hoa ban tin Completed
	FS_FlashGetway_SetStatusMarkArray (1, Sys_Page); // Thuc hien set trang thai da su dung phan tu nay trong mang
	if(Flag_CheckMAC == 0) // Kiem tra neu chua co dia chi Mac trog flash thi se cap phat dia chi unicast moi
	{
		FS_FlashGetway_IncreaseUnicast(); // Tang gia tri unicast len
		Sys_GetwayFlashArrData[Sys_Page].Unicast = Sys_Unicast; // Dua unicast vao phan du luu cua mang thong tin device
	}
	else
	{
		FS_FlashGetway_IncreaseUnicast(); // Tang gia tri unicast len
		Sys_GetwayFlashArrData[Sys_Page].Unicast = Sys_Unicast; // Dua unicast vao phan du luu cua mang thong tin device
	}
	#if FLASH_DEBUG_SAVE
		APP_PPRINTF("\n(FLASH_BUG: Save Confirm Done)\n"); 
	#endif
}

void smartfram_SaveJointInfoJoinDone(void)
{
	FS_FlashGetway_Write(); //luu thong tin vao flash
	#if FLASH_DEBUG_SAVE
		APP_PPRINTF("\n(FLASH_BUG: Save JoinDone Done)\n"); 
	#endif
}

uint8_t smartfram_FindUnicast(uint32_t Unicast)
{
	uint8_t i = 0;
	
	for(i = 0; i < MAX_ADD_DEVICE; i++)
	{
		if(	Sys_GetwayFlashArrData[i].Unicast == Unicast)
		{
			return i;
		}
	}
	#if FLASH_DEBUG_ERROR
		APP_PPRINTF("\n-----------------(FLASH_BUG: Khong tim thay Unicast nay)------------------------- \n"); 
	#endif
	return ARR_FULL;
}


void smartfram_SetKeyDevice(uint8_t Pgae)
{
	FS_FlashGetway_UpdateDeviceKey(Pgae);
}


static void FS_FlashGetway_UpdateDeviceKey(uint8_t page)
{
	pDeviceKeyAES[0] = Sys_GetwayFlashArrData[page].DeviceKey[0];
	pDeviceKeyAES[1] = Sys_GetwayFlashArrData[page].DeviceKey[1];
	pDeviceKeyAES[2] = Sys_GetwayFlashArrData[page].DeviceKey[2];
	pDeviceKeyAES[3] = Sys_GetwayFlashArrData[page].DeviceKey[3];
	
	#if FLASH_DEBUG_UPDATE_DEVICEKEY
		APP_PPRINTF("\n-----------------(FLASH_BUG: Update Device Key)------------------------- \n"); 
		APP_PPRINTF( "page: %d\n\r", page);
		APP_PPRINTF( "pDeviceKeyAES0: %x\n\r", pDeviceKeyAES[0]);
		APP_PPRINTF( "pDeviceKeyAES1: %x\n\r", pDeviceKeyAES[1]);
		APP_PPRINTF( "pDeviceKeyAES2: %x\n\r", pDeviceKeyAES[2]);
		APP_PPRINTF( "pDeviceKeyAES3: %x\n\r", pDeviceKeyAES[3]);
	#endif
	
	/*	
	APP_PPRINTF( "pDeviceKeyAES0: %x\n\r", Sys_GetwayFlashArrData[0].DeviceKey[0]);
	APP_PPRINTF( "pDeviceKeyAES1: %x\n\r", Sys_GetwayFlashArrData[0].DeviceKey[1]);
	APP_PPRINTF( "pDeviceKeyAES2: %x\n\r", Sys_GetwayFlashArrData[0].DeviceKey[2]);
	APP_PPRINTF( "pDeviceKeyAES3: %x\n\r", Sys_GetwayFlashArrData[0].DeviceKey[3]);
	
	APP_PPRINTF( "pDeviceKeyAES0: %x\n\r", Sys_GetwayFlashArrData[1].DeviceKey[0]);
	APP_PPRINTF( "pDeviceKeyAES1: %x\n\r", Sys_GetwayFlashArrData[1].DeviceKey[1]);
	APP_PPRINTF( "pDeviceKeyAES2: %x\n\r", Sys_GetwayFlashArrData[1].DeviceKey[2]);
	APP_PPRINTF( "pDeviceKeyAES3: %x\n\r", Sys_GetwayFlashArrData[1].DeviceKey[3]);
	*/
}


static void FS_FlashGetway_InitGetway(void)
{
	uint8_t page = 0, i;
	for(i=0; i < 128; i++)
	{
		page = i;
		Sys_GetwayFlashArrData[page].Status 				= 0x00;
		Sys_GetwayFlashArrData[page].TypeSensor 		= 0x00;
		Sys_GetwayFlashArrData[page].Option 				= 0x0000;
		Sys_GetwayFlashArrData[page].Unicast 				= 0x00000000;
		Sys_GetwayFlashArrData[page].DeviceKey[0] 	= 0x00000000; 
		Sys_GetwayFlashArrData[page].DeviceKey[1] 	=	0x00000000;
		Sys_GetwayFlashArrData[page].DeviceKey[2] 	= 0x00000000;
		Sys_GetwayFlashArrData[page].DeviceKey[3] 	= 0x00000000;
		Sys_GetwayFlashArrData[page].MACDevice[0] 	= 0x00000000;
		Sys_GetwayFlashArrData[page].MACDevice[1] 	= 0x00000000;
	}
	FS_FlashGetway_Write();
}

static void FS_FlashGetway_InitUnicastAndGetwayKey(void)
{
	/*Mode = 1 Create New GetwayKey and Set Unicast = 0*/
	FS_FlashGetway_WriteUnicastAndGetwayKey(1);
}

static void FS_FlashGetway_ReadInit(void)
{
	FS_FlashGetway_ReadUnicastAndGetwayKey();
	FS_FlashGetway_Read(); 
}

static void FS_FlashGetway_DebugPage()
{
	uint8_t page = 0;
	uint8_t TimeSerial = 40;
	HAL_Delay(1000);
	APP_PPRINTF("\n------------------------------------------ \n"); 
	APP_PPRINTF("\n Netkey[0]: %x\n",pLocalGetwayKeyAES[0]);
	APP_PPRINTF("\n Netkey[1]: %x\n",pLocalGetwayKeyAES[1]);
	APP_PPRINTF("\n Netkey[2]: %x\n",pLocalGetwayKeyAES[2]);
	APP_PPRINTF("\n Netkey[3]: %x\n",pLocalGetwayKeyAES[3]);
	APP_PPRINTF("\n Sys_Unicast: %d\n",Sys_Unicast);
	for(page=0; page<128; page++)
	{
		if(Sys_GetwayFlashArrData[page].Status == 1)
		{
			APP_PPRINTF("\n------------------------------------------ \n"); 
			HAL_Delay(TimeSerial);
			APP_PPRINTF("Status: %x \n", Sys_GetwayFlashArrData[page].Status);
			HAL_Delay(TimeSerial);
			APP_PPRINTF("TypeSensor: %x \n", Sys_GetwayFlashArrData[page].TypeSensor);
			HAL_Delay(TimeSerial);
			APP_PPRINTF("Option: %x \n", Sys_GetwayFlashArrData[page].Option);
			HAL_Delay(TimeSerial);
			APP_PPRINTF("Unicast: %d \n", Sys_GetwayFlashArrData[page].Unicast);
			HAL_Delay(TimeSerial);
			APP_PPRINTF("DeviceKey[0]: %x \n", Sys_GetwayFlashArrData[page].DeviceKey[0]);
			HAL_Delay(TimeSerial);
			APP_PPRINTF("DeviceKey[1]: %x \n", Sys_GetwayFlashArrData[page].DeviceKey[1]);
			HAL_Delay(TimeSerial);
			APP_PPRINTF("DeviceKey[2]: %x \n", Sys_GetwayFlashArrData[page].DeviceKey[2]);
			HAL_Delay(TimeSerial);
			APP_PPRINTF("DeviceKey[3]: %x \n", Sys_GetwayFlashArrData[page].DeviceKey[3]);
			HAL_Delay(TimeSerial);
			APP_PPRINTF("MACDevice[0]: %x \n", Sys_GetwayFlashArrData[page].MACDevice[0]);
			HAL_Delay(TimeSerial);
			APP_PPRINTF("MACDevice[1]: %x \n", Sys_GetwayFlashArrData[page].MACDevice[1]);
			HAL_Delay(TimeSerial);
			APP_PPRINTF("page: %d\n", page);
			HAL_Delay(1000);
		}
	}
}

void FS_FlashGetway_SetStatusMarkArray (uint8_t stt, uint8_t page)
{
	Sys_GetwayFlashArrData[page].Status = stt;
}

uint8_t FS_FlashGetway_CheckMacDevicey(uint32_t Data1, uint32_t Data2)
{
	uint8_t i;
	for(i=0; i < MAX_ADD_DEVICE; i++)
	{
		if(Data1 == Sys_GetwayFlashArrData[i].MACDevice[0] && Data2 == Sys_GetwayFlashArrData[i].MACDevice[1])
		{
			#if FLASH_DEBUG_CHECKMAC
				APP_PPRINTF("\n-----------------(FLASH_BUG: Da co Mac Deviceyy)------------------------- \n"); 
			#endif
			return i;
		}
	}
	#if FLASH_DEBUG_CHECKMAC
		APP_PPRINTF("\n-----------------(FLASH_BUG: Chua co Mac Deviceyy)------------------------- \n"); 
	#endif
	return ARR_FULL;
}

uint8_t FS_FlashGetway_FindPageEmpty(void)
{
	uint8_t i = 0;
	for(i = 0; i < MAX_ADD_DEVICE; i++)
	{
		if(	Sys_GetwayFlashArrData[i].Status == 0)
		{
			#if FLASH_DEBUG_FIND_PAGE_EMPTY
				APP_PPRINTF("\n-----------------(FLASH_BUG: Co PAGE chong)------------------------- \n"); 
			#endif
			return i;
		}
	}
	#if FLASH_DEBUG_FIND_PAGE_EMPTY
		APP_PPRINTF("\n-----------------(FLASH_BUG: Day PAGE)------------------------- \n"); 
	#endif
	return ARR_FULL;
}

void FS_FlashGetway_IncreaseUnicast(void)
{
	FS_FlashGetway_ReadUnicastAndGetwayKey();
	Sys_Unicast++;
	FS_FlashGetway_WriteUnicastAndGetwayKey(0);
}

void smartfram_FlashRemoveInforDevice(uint8_t Page)
{
	if(Page != ARR_FULL)
	{
	/*Doc tat ca cac Page tu Flash*/
	FS_FlashGetway_Read();
	
	/*Cho page can xoa bang 0*/
	Sys_GetwayFlashArrData[Page].Status 				= 0x00;
	Sys_GetwayFlashArrData[Page].TypeSensor 		= 0x00;
	Sys_GetwayFlashArrData[Page].Option 				= 0x0000;
	Sys_GetwayFlashArrData[Page].Unicast 				= 0x00000000;
	Sys_GetwayFlashArrData[Page].DeviceKey[0] 	= 0x00000000; 
	Sys_GetwayFlashArrData[Page].DeviceKey[1] 	=	0x00000000;
	Sys_GetwayFlashArrData[Page].DeviceKey[2] 	= 0x00000000;
	Sys_GetwayFlashArrData[Page].DeviceKey[3] 	= 0x00000000;
	Sys_GetwayFlashArrData[Page].MACDevice[0] 	= 0x00000000;
	Sys_GetwayFlashArrData[Page].MACDevice[1] 	= 0x00000000;
	/*Luu lai cac page*/
	FS_FlashGetway_Write();
	#if FLASH_DEBUG_REMOVE_PAGE
		APP_PPRINTF("\n-----------------(FLASH_BUG: Xoa thanh cong)------------------------- \n"); 
		FS_FlashGetway_DebugPage();
	#endif
	}
	else
	{
		#if FLASH_DEBUG_ERROR
			APP_PPRINTF("\n-----------------(FLASH_BUG: Khong tim thay Unicast can xoa)------------------------- \n"); 
		#endif
	}
}

/*---------------------------------------------------------------------------------------(Read, Write Flash)---------------------------------------------------------------------------------------*/
static uint32_t FS_FlashGetway_GetPage(uint32_t Addr)
{
  return (Addr - FLASH_BASE) / FLASH_PAGE_SIZE;;
}

static void FS_FlashGetway_Erase(void)
{
	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);
	FirstPage = FS_FlashGetway_GetPage(FLASH_USER_START_ADDR);
	NbOfPages = FS_FlashGetway_GetPage(FLASH_USER_END_ADDR) - FirstPage + 1;
	EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.Page        = FirstPage;
  EraseInitStruct.NbPages     = NbOfPages;
	if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
  {
		APP_PPRINTF("\n FlashErase Error\n");
  }
	HAL_FLASH_Lock();
}

static void FS_FlashGetway_Write()
{
	uint16_t i;
	uint64_t *TempFlashData = (uint64_t*)Sys_GetwayFlashArrData;
	FS_FlashGetway_Erase();
	Address = FLASH_USER_START_ADDR;
	HAL_FLASH_Unlock();
	for(i=0; i < FLASH_SIZE_DATA_WRITE; i++)
	{
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, Address, TempFlashData[i]) == HAL_OK)
    {
      Address = Address + 8;  /* increment to next double word*/
    }
	}
	HAL_FLASH_Lock();
}

static void FS_FlashGetway_Read()
{
	uint8_t i, CountArr = 0;
	Address = FLASH_USER_START_ADDR;
	while(Address < FLASH_USER_END_ADDR)
	{
		GetwayFlashData_t *TempData;
		uint8_t DataRead[sizeof(GetwayFlashData_t)]={0}; 
		for(i=0; i < sizeof(GetwayFlashData_t); i++)
		{
			DataRead[i] = *(__IO uint8_t *)Address;
			Address = Address + 1;
		}
		TempData = (GetwayFlashData_t*)&DataRead;
		Sys_GetwayFlashArrData[CountArr].Status 			= TempData->Status;
		Sys_GetwayFlashArrData[CountArr].TypeSensor 	= TempData->TypeSensor;
		Sys_GetwayFlashArrData[CountArr].Option 			= TempData->Option;
		Sys_GetwayFlashArrData[CountArr].Unicast 			= TempData->Unicast;
		Sys_GetwayFlashArrData[CountArr].DeviceKey[0] = TempData->DeviceKey[0]; 
		Sys_GetwayFlashArrData[CountArr].DeviceKey[1] = TempData->DeviceKey[1];
		Sys_GetwayFlashArrData[CountArr].DeviceKey[2] = TempData->DeviceKey[2];
		Sys_GetwayFlashArrData[CountArr].DeviceKey[3] = TempData->DeviceKey[3];
		Sys_GetwayFlashArrData[CountArr].MACDevice[0] = TempData->MACDevice[0];
		Sys_GetwayFlashArrData[CountArr].MACDevice[1] = TempData->MACDevice[1];
		CountArr++;
	}
}

static void FS_FlashGetway_WriteUnicastAndGetwayKey(uint8_t mode)
{
	uint8_t i;
	uint64_t *TempFlashData;
	uint32_t GetwayKey[5]={0};
	
	/*Tao Random Getwaykey va Unicast = 0*/
	FS_FlashGetway_CreateGetwayKey(GetwayKey, mode); // Mode = 1 -> Create New, Mode = 0 -> Update
	
	/*Start Clear Flash*/
	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);
	FirstPage = FS_FlashGetway_GetPage(FLASH_USER_NETKEY_ADDR);
	NbOfPages = FS_FlashGetway_GetPage(FLASH_USER_NETKEY_ADDR) - FirstPage + 1;
	EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.Page        = FirstPage;
  EraseInitStruct.NbPages     = NbOfPages;
	if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
  {
		APP_PPRINTF("\n FlashErase Error\n");
  }
	/*End Clear Flash*/
	
	/*Start Write Flash*/
	Address = FLASH_USER_NETKEY_ADDR;
	TempFlashData = (uint64_t*)&GetwayKey;
	for(i=0; i < 3; i++)
	{
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, Address, TempFlashData[i]) == HAL_OK)
    {
      Address = Address + 8;  /* increment to next double word*/
    }
	}
	HAL_FLASH_Lock();
	/*End Write Flash*/
}

static void FS_FlashGetway_ReadUnicastAndGetwayKey(void)
{
	uint32_t TempData;
	uint8_t i;
	Address = FLASH_USER_NETKEY_ADDR;
	for(i=0; i < 4; i++)
	{
		pLocalGetwayKeyAES[i] = *(__IO uint32_t *)Address;
		Address = Address + 4;
	}
	Sys_Unicast = *(__IO uint32_t *)Address;
}

void FS_FlashGetway_CreateGetwayKey(uint32_t *data, uint8_t mode)
{
	if(mode == 1)
	{
		uint8_t counter = 0;
		uint32_t aRandom32bit[8];
		srand(LL_FLASH_GetUDN());
		for (counter = 0; counter < 4; counter++)
		{
			if (HAL_RNG_GenerateRandomNumber(&hrng, &aRandom32bit[counter]) != HAL_OK)
			{
				Error_Handler();
			}
			data[counter] = aRandom32bit[counter]; //Create Getway Key
		}
		data[4] = 0x00000000; //Unicast Init
	}
	else
	{
		data[0] = pLocalGetwayKeyAES[0];
		data[1] = pLocalGetwayKeyAES[1];
		data[2] = pLocalGetwayKeyAES[2];
		data[3] = pLocalGetwayKeyAES[3];
		data[4] = Sys_Unicast;
	}
}
