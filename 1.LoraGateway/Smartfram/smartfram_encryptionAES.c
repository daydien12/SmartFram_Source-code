//AES.c

#include "smartfram_encryptionAES.h"
#include "smartfram_structpayload.h"

#define SizeArr 96

#define BUG_ENCRY_PRIVATEKEY 0
#define BUG_DNCRY_PRIVATEKEY 0

#define BUG_ENCRY_PUBLICKEY 0
#define BUG_DNCRY_PUBLICKEY 0


const uint8_t MaxBlock = 7; // so block toi da duoc su dung cho ma hoa(1 block = 2byte, max = 16*7 = 112 byte)

CRYP_HandleTypeDef hcryp;
DMA_HandleTypeDef hdma_aes_in;
DMA_HandleTypeDef hdma_aes_out;
RNG_HandleTypeDef hrng;

__ALIGN_BEGIN static const uint32_t pPrivateKeyAES[4] __ALIGN_END = {0x2B7E1516, 0x28AED2A6, 0xABF71588, 0x09CF4F3C};
__ALIGN_BEGIN uint32_t pLocalGetwayKeyAES[4] __ALIGN_END     			= {0x00000000, 0x00000000, 0x00000000, 0x00000000};
__ALIGN_BEGIN uint32_t pDeviceKeyAES[4] __ALIGN_END         			= {0x00000000, 0x00000000, 0x00000000 , 0x00000000};
__ALIGN_BEGIN static const uint32_t AESIV[4] __ALIGN_END          = {0x00010203, 0x04050607, 0x08090A0B, 0x0C0D0E0F};

/**
	Mo ta: Ham ma hoa su dung key chung
	Tham so:	
		-InputData: Mang(uint8_t) data can ma hoa 
		-SizeData: Size mang dau vao(de tinh toan block)
		-OutputData: Mang(uint8_t) data da duoc ma hoa
	Tra ve: size cua mang da ma hoa
**/
uint8_t Smartfram_EncryptedPrivate(uint8_t* InputData, uint8_t SizeData, uint8_t* OutputData);

/**
	Mo ta: Ham giai ma su dung Key chung
	Tham so:	
		-InputData: Mang(uint8_t) data can giai ma 
		-SizeData: Size mang dau vao(de tinh toan block)
		-OutputData: Mang(uint8_t) data da duoc giai ma
**/
void Smartfram_DecryptedPrivate(uint8_t* InputData, uint8_t SizeData, uint8_t* OutputData);
/**
	Mo ta: Ham ma hoa su dung DeviceKey
	Tham so:	
		-InputData: Mang(uint8_t) data can ma hoa 
		-Key: DeviceKey ma hoa tuong ung
		-SizeDataIn: Size mang dau vao(de tinh toan block)
		-SizeCopy: Size so phan tu khong ma hoa lop 2(Header, TypeMsg, Crc, Unicast)
		-OutputData: Mang(uint8_t) data da duoc ma hoa
	Tra ve: size cua mang da ma hoa
**/
uint8_t Smartfram_EncryptedPublic(uint8_t* InputData, uint32_t* Key, uint8_t SizeDataIn, uint8_t SizeCopy, uint8_t *OutputData);

/**
	Mo ta: Ham giai ma su dung DeviceKey
	Tham so:	
		-InputData: Mang(uint8_t) data can giai ma
		-Key: DeviceKey ma hoa tuong ung
		-SizeDataIn: Size mang dau vao(de tinh toan block)
		-SizeCopy: Size phan tu can copy vao OutputData va khong qua giai ma(Header, TypeMsg, Crc, Unicast)
		-OutputData: Mang(uint8_t) data da duoc ma hoa
**/
void Smartfram_DecryptedPublic(uint8_t* InputData, uint32_t* Key, uint8_t SizeDataIn, uint8_t SizeCopy, uint8_t *OutputData);

/**
	Mo ta: Ham debug
	Tham so:	
		-DataIn: Mang(uint8_t) can in ra
		-Length: do dai cua mang
		-Info: Thong tin cua mang(string)	
**/
static void SerialDebug(uint8_t* DataIn, uint8_t Length, char* Info);

/**
	Mo ta: Ham tinh toan block
	Tham so:	
		-LengthIn: size data
**/
static uint8_t CalculatorBlockEncrypted(uint8_t LengthIn);

/**
	Mo ta: Ham khoi tao key(cho ma hoa, giai ma)
	Tham so:	
		-Key: Privatekey, Device
**/
static void Encrypted_Init(uint32_t* Key);

/**
	Mo ta: Ham luu check sum vao chuoi ma hoa
	Tham so:	
		-DataIn: Mang(uint8_t) can luu ma check sum
		-DataOut: Mang(uint8_t) dao luu check sum
		-TypePayload: Loai ban tin (REQUETST, ACCEPT ...)
**/
static void CreateCheckSum(uint8_t* DataIn, uint8_t* DataOut, typePayload_t TypePayload);

/**
	Mo ta: Ham ma hoa su dung DeviceKey
	Tham so:	
		-DataIn: Mang(uint8_t) can tao check sum 
		-Length: size chuoi dau vao
	Tra ve: tra ve ma checksum
**/
static uint8_t CheckSum(uint8_t* DataIn, uint8_t Length);

/**
	Mo ta: Ham kiem tra thong tin chuoi(Header, TypeMsg, Crc)
	Tham so:	
		-DataIn: Mang(uint8_t) can kiem tra
		-TypePayload: oai ban tin (REQUETST, ACCEPT ...)
		-AdressUnicast: Lây Unicast
		-Mode: là 1: kiem tra (Header, TypeMsg) lop giai ma 1, là 0: kiem tra (CRC) lop giai ma 2
		Tra ve: 1: dung, 0:sai
**/
static uint8_t CheckDecrypted(uint8_t* DataIn, typePayload_t TypePayload, uint16_t *AdressUnicast, uint8_t Mode);

/**
	Mo ta: Ham kiem tra thong tin chuoi(Header, TypeMsg)
	Tham so:	
		-HeaderIn: Header nhan duoc
		-HeaderCheck: Header tuong ung voi ban tin(0xAA55)
		-TypeMsgIn: TypeMsgIn nhan duoc
		-TypeMsgCheck: TypeMsgIn(0x01, 0x02, 0x03, 0x04, 0x05, 0x06)
		Tra ve: 1: dung, 0:sai
**/
static uint8_t Check_Header_TypeMsg(uint16_t HeaderIn, uint16_t HeaderCheck, uint8_t TypeMsgIn, uint8_t TypeMsgCheck);

void Smartfram_Encrypted(encrypted_t* data)
{
  uint8_t TempData[100] = {0}, TempData1[100] = {0};
  uint8_t SizeArray = 0, SizeArray2 = 0 ;
  switch (data->TypePayload)
  {
    case ACCEPT:
      CreateCheckSum(data->EncryptedIn, TempData, data->TypePayload);
      data->SizeDataOut = Smartfram_EncryptedPrivate(TempData, data->SizeDataIn, data->EncryptedOut);
      break;

    case COMPLETED:
      CreateCheckSum(data->EncryptedIn, TempData1, data->TypePayload);
      SizeArray = Smartfram_EncryptedPublic(TempData1, pDeviceKeyAES, data->SizeDataIn, SM_AES_COMPLETED_DF, TempData); //dung key Network
      data->SizeDataOut = Smartfram_EncryptedPrivate(TempData, SizeArray, data->EncryptedOut);
      break;

    case GETWAYTODEVICE:
      CreateCheckSum(data->EncryptedIn, TempData1, data->TypePayload);
      SizeArray = Smartfram_EncryptedPublic(TempData1, pDeviceKeyAES, data->SizeDataIn, SM_AES_MSGDEVICEGETGAY_DF, TempData); //dung key Network
      data->SizeDataOut = Smartfram_EncryptedPrivate(TempData, SizeArray, data->EncryptedOut);
      break;

    case REQUETST:
    case CONFIRM:
    case DEVICETOGETWAY:
      APP_PPRINTF( "\n|||||||(NULL )|||||||| \n\r");
      break;
  }
}

void Smartfram_Decrypted(decrypted_t* data)
{
  uint8_t TempData[100] = {0};
  uint16_t AdressUnicast = 0;

  switch (data->TypePayload)
  {
    case REQUETST:
      Smartfram_DecryptedPrivate(data->DecryptedIn, data->SizeIn, data->DecryptedOut);
      if (CheckDecrypted(data->DecryptedOut, data->TypePayload, &AdressUnicast, 1) == 1)
      {
        data->Check = 1;
      }
      else
      {
        data->Check = 0;
      }
      break;

    case CONFIRM:
      Smartfram_DecryptedPrivate(data->DecryptedIn, data->SizeIn, TempData);
      if (CheckDecrypted(TempData, data->TypePayload, &AdressUnicast, 1) == 1)
      {
        Smartfram_DecryptedPublic(TempData, pLocalGetwayKeyAES, data->SizeTypeOut , SM_AES_CONFIRM_DF, data->DecryptedOut);
        if (CheckDecrypted(data->DecryptedOut, data->TypePayload, &AdressUnicast, 0))
        {
          data->Check = 1;
        }
        else
        {
          data->Check = 0;
        }
      }
      else
      {
        data->Check = 0;
      }
      break;

    case DEVICETOGETWAY:
      Smartfram_DecryptedPrivate(data->DecryptedIn, data->SizeIn, TempData);
      if (CheckDecrypted(TempData, data->TypePayload, &AdressUnicast, 1) == 1)
      {
        Smartfram_DecryptedPublic(TempData, pLocalGetwayKeyAES, data->SizeTypeOut , SM_AES_MSGDEVICEGETGAY_DF, data->DecryptedOut);
        if (CheckDecrypted(data->DecryptedOut, data->TypePayload, &AdressUnicast, 0))
        {
          data->Check = 1;
        }
        else
        {
          data->Check = 0;
        }
      }
      else
      {
        data->Check = 0;
      }
      break;

    case ACCEPT:
    case COMPLETED:
    case GETWAYTODEVICE:
      APP_PPRINTF( "\n|||||||(NULL )|||||||| \n\r");
      break;
  }
}

uint8_t Smartfram_EncryptedPrivate(uint8_t *InputData, uint8_t Length, uint8_t *OutputData)
{
  uint8_t aEncryptedText[SizeArr] = {0};
  uint8_t aInputText[SizeArr]     = {0};
  uint8_t *TempInputData = InputData;
  uint8_t TempLength;
  uint8_t CountAddZero;
  uint8_t CountCopyData;

  TempLength = CalculatorBlockEncrypted(Length);

  for (CountAddZero = 0; CountAddZero < TempLength; CountAddZero++)
  {
    aInputText[CountAddZero] = *TempInputData++;
  }

  Encrypted_Init((uint32_t*)pPrivateKeyAES);

  if (HAL_CRYP_Encrypt_DMA(&hcryp, (uint32_t*)aInputText, TempLength, (uint32_t*)aEncryptedText) != HAL_OK)
  {
    /* Processing Error */
    Error_Handler();
  }

  while (HAL_CRYP_GetState(&hcryp) != HAL_CRYP_STATE_READY) {}

  for (CountCopyData = 0; CountCopyData < TempLength; CountCopyData++)
  {
    OutputData[CountCopyData] =  aEncryptedText[CountCopyData];
  }

#if BUG_ENCRY_PRIVATEKEY
  SerialDebug(aInputText, TempLength, "IN: Encrypted PrivateKey");
  SerialDebug(aEncryptedText, TempLength, "OUT: Encrypted PrivateKey");
#endif
  return TempLength;
}

void Smartfram_DecryptedPrivate(uint8_t *InputData, uint8_t Length, uint8_t *OutputData)
{
  uint8_t aDecryptedText[SizeArr] = {0};
  uint8_t aInputText[SizeArr]     = {0};
  uint8_t TempLength;
  uint8_t CountCopyData;
  uint8_t CountAddZero;
  uint8_t *TempInputData = InputData;

  TempLength = CalculatorBlockEncrypted(Length);

  for (CountAddZero = 0; CountAddZero < TempLength; CountAddZero++)
  {
    aInputText[CountAddZero] = *TempInputData++;
  }


  Encrypted_Init((uint32_t*)pPrivateKeyAES);

  if (HAL_CRYP_Decrypt_DMA(&hcryp, (uint32_t*)aInputText, TempLength, (uint32_t*)aDecryptedText) != HAL_OK)
  {
    /* Processing Error */
    Error_Handler();
  }

  while (HAL_CRYP_GetState(&hcryp) != HAL_CRYP_STATE_READY) {}

  for (CountCopyData = 0; CountCopyData < TempLength; CountCopyData++)
  {
    OutputData[CountCopyData] =  aDecryptedText[CountCopyData];
  }

#if BUG_DNCRY_PRIVATEKEY
  SerialDebug(InputData, TempLength, "IN: Decrypted PrivateKey");
  SerialDebug(aDecryptedText, TempLength, "OUT: Decrypted PrivateKey");
#endif
}

uint8_t Smartfram_EncryptedPublic(uint8_t* InputData, uint32_t* Key, uint8_t SizeDataIn, uint8_t SizeCopy, uint8_t *OutputData)
{

  uint8_t aEncryptedText[SizeArr] = {0};
  uint8_t aInputText[SizeArr]     = {0};
  uint8_t *TempInputData = InputData;
  uint8_t TempLength;
  uint8_t CountAddZero;
  uint8_t CountCopyData;

  TempLength = CalculatorBlockEncrypted(SizeDataIn);

  for (CountAddZero = 0; CountAddZero < TempLength + SizeCopy; CountAddZero++)
  {
    if (CountAddZero < SizeCopy)
    {
      OutputData[CountAddZero] = TempInputData[CountAddZero];
    }
    else
    {
      aInputText[CountAddZero - SizeCopy] = TempInputData[CountAddZero];
    }
  }

  Encrypted_Init(Key);

  if (HAL_CRYP_Encrypt_DMA(&hcryp, (uint32_t*)aInputText, TempLength, (uint32_t*)aEncryptedText) != HAL_OK)
  {
    /* Processing Error */
    Error_Handler();
  }

  while (HAL_CRYP_GetState(&hcryp) != HAL_CRYP_STATE_READY) {}

  for (CountCopyData = SizeCopy; CountCopyData < TempLength + SizeCopy; CountCopyData++)
  {
    OutputData[CountCopyData] =  aEncryptedText[CountCopyData - SizeCopy];
  }
#if BUG_ENCRY_PUBLICKEY
  SerialDebug(InputData, TempLength, "IN: Encrypted DeviceKey");
  SerialDebug(aEncryptedText, TempLength, "OUT: Encrypted DeviceKey");
#endif
  return TempLength + SizeCopy;
}

void Smartfram_DecryptedPublic(uint8_t* InputData, uint32_t* Key, uint8_t SizeDataIn, uint8_t SizeCopy, uint8_t *OutputData)
{
  uint8_t aDecryptedText[SizeArr] = {0};
  uint8_t aInputText[SizeArr]     = {0};
  uint8_t *TempInputData = InputData;
  uint8_t TempLength;
  uint8_t CountAddZero;
  uint8_t CountCopyData;

  TempLength = CalculatorBlockEncrypted(SizeDataIn);

  for (CountAddZero = 0; CountAddZero < TempLength + SizeCopy; CountAddZero++)
  {
    if (CountAddZero < SizeCopy)
    {
      OutputData[CountAddZero] = TempInputData[CountAddZero];
    }
    else
    {
      aInputText[CountAddZero - SizeCopy] = TempInputData[CountAddZero];
    }
  }

  Encrypted_Init(Key);

  if (HAL_CRYP_Decrypt_DMA(&hcryp, (uint32_t*)aInputText, TempLength, (uint32_t*)aDecryptedText) != HAL_OK)
  {
    /* Processing Error */
    Error_Handler();
  }

  while (HAL_CRYP_GetState(&hcryp) != HAL_CRYP_STATE_READY) {}

  for (CountCopyData = SizeCopy; CountCopyData < SizeDataIn; CountCopyData++)
  {
    OutputData[CountCopyData] =  aDecryptedText[CountCopyData - SizeCopy];
  }

#if BUG_DNCRY_PUBLICKEY
  SerialDebug(aInputText, TempLength, "IN: Decrypted DeviceKey");
  SerialDebug(aDecryptedText, TempLength, "OUT: Decrypted DeviceKey");
#endif
}

void Smartfram_RNG_Init(void)
{
	
  LL_RCC_MSI_SetRange(LL_RCC_MSIRANGE_11);
  LL_RCC_MSI_SetCalibTrimming(0);
  LL_RCC_PLL_ConfigDomain_RNG(LL_RCC_PLLSOURCE_MSI, LL_RCC_PLLM_DIV_4, 8, LL_RCC_PLLQ_DIV_2);
  LL_RCC_PLL_EnableDomain_RNG();
  LL_RCC_PLL_Enable();

  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_MSI);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_MSI)
  {
  }
  LL_AHB3_GRP1_EnableClock(LL_AHB3_GRP1_PERIPH_RNG);

  /* USER CODE BEGIN RNG_Init 2 */
  hrng.Instance = RNG;
  hrng.Init.ClockErrorDetection = RNG_CED_ENABLE;
	HAL_RNG_MspInit(&hrng);
  if (HAL_RNG_Init(&hrng) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE END RNG_Init 2 */
}

/**
    Enable DMA controller clock
*/
void Smartfram_DMA_Init(void)
{
  /* DMA controller clock enable */
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  /* DMA1_Channel2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);
}

/**
    @brief AES Initialization Function
    @param None
    @retval None
*/
void Smartfram_AES_Init(void)
{
  /* USER CODE BEGIN AES_Init 0 */

  /* USER CODE END AES_Init 0 */

  /* USER CODE BEGIN AES_Init 1 */

  /* USER CODE END AES_Init 1 */
  hcryp.Instance = AES;
  hcryp.Init.DataType = CRYP_DATATYPE_32B;
  hcryp.Init.KeySize = CRYP_KEYSIZE_128B;
  hcryp.Init.pKey = (uint32_t *)pPrivateKeyAES;
  hcryp.Init.Algorithm = CRYP_AES_ECB;
  hcryp.Init.DataWidthUnit = CRYP_DATAWIDTHUNIT_WORD;
  hcryp.Init.HeaderWidthUnit = CRYP_HEADERWIDTHUNIT_WORD;
  hcryp.Init.KeyIVConfigSkip = CRYP_KEYIVCONFIG_ALWAYS;
  if (HAL_CRYP_Init(&hcryp) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN AES_Init 2 */

  /* USER CODE END AES_Init 2 */
}

void Smartfram_Error_Handler(void)
{
  APP_LOG(1, 1, "loi");
}

static void SerialDebug(uint8_t* DataIn, uint8_t Length, char* Info)
{
  uint8_t BufferCounter;
  uint8_t *ArrSerial;
  uint8_t CountBlock = 0;

  ArrSerial = DataIn;
  APP_PPRINTF("\n\r ===========================(%s)===========================\n\r", Info);
  for (BufferCounter = 0; BufferCounter < Length; BufferCounter++)
  {
    APP_PPRINTF("[0x%02X]", *ArrSerial++);
    CountBlock++;
    HAL_Delay(50);
    if (CountBlock == 16)
    {
      CountBlock = 0;
      APP_PPRINTF(" Blocka %d \n\r", BufferCounter / 16);
    }
  }
  APP_PPRINTF("\n---------------(END)---------------\n\r");
}

static uint8_t CalculatorBlockEncrypted(uint8_t LengthIn)
{
  uint8_t CountLength;
  uint8_t TempLength;
  for (CountLength = 1; CountLength < MaxBlock; CountLength++)
  {
    TempLength = 16 * CountLength;
    if (TempLength >= LengthIn)
      break;
    else
      TempLength = 0;
  }
  return TempLength;
}

static void Encrypted_Init(uint32_t* Key)
{
  if (HAL_CRYP_DeInit(&hcryp) != HAL_OK)
  {
    Error_Handler();
  }

  hcryp.Instance = AES;
  hcryp.Init.DataType      = CRYP_DATATYPE_8B;
  hcryp.Init.KeySize       = CRYP_KEYSIZE_128B;
  hcryp.Init.Algorithm     = CRYP_AES_CBC;
  hcryp.Init.pKey          = (uint32_t *)Key;
  hcryp.Init.pInitVect     = (uint32_t *)AESIV;
  hcryp.Init.DataWidthUnit = CRYP_DATAWIDTHUNIT_BYTE;
  if (HAL_CRYP_Init(&hcryp) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }
}

static void CreateCheckSum(uint8_t* DataIn, uint8_t* DataOut, typePayload_t TypePayload)
{
  uint8_t SizeData = 0;
  joinRequetstMsg_t *CreateCheckSumRequetst;
  joinAcceptMsg_t *CreateCheckSumAccept;
  joinConfirmMsg_t *CreateCheckSumConfirm;
  joinCompletedMsg_t *CreateCheckSumCompleted;
  msgDeviceGetway_t *CreateCheckSumDeviceToGetway, *CreateCheckSumGetwayToDevice;

  switch (TypePayload)
  {
    case REQUETST:
      CreateCheckSumRequetst = (joinRequetstMsg_t *)DataIn;
      SizeData =  sizeof(joinRequetstMsg_t);
      CreateCheckSumRequetst->Crc = CheckSum(DataIn, SizeData);
      memcpy(DataOut, (uint8_t*)CreateCheckSumRequetst, SizeData);
      break;

    case ACCEPT:
      CreateCheckSumAccept = (joinAcceptMsg_t *)DataIn;
      SizeData = sizeof(joinAcceptMsg_t);
      CreateCheckSumAccept->Crc =   CheckSum(DataIn, SizeData);
      memcpy(DataOut, (uint8_t*)CreateCheckSumAccept, SizeData);

      break;

    case CONFIRM:
      CreateCheckSumConfirm = (joinConfirmMsg_t *)DataIn;
      SizeData = sizeof(joinConfirmMsg_t);
      CreateCheckSumConfirm->Crc =  CheckSum(DataIn, SizeData);
      memcpy(DataOut, (uint8_t*)CreateCheckSumConfirm, SizeData);
      break;

    case COMPLETED:
      CreateCheckSumCompleted = (joinCompletedMsg_t *)DataIn;
      SizeData = sizeof(joinCompletedMsg_t);
      CreateCheckSumCompleted->Crc = CheckSum(DataIn, SizeData);
      memcpy(DataOut, (uint8_t*)CreateCheckSumCompleted, SizeData);
      break;

    case DEVICETOGETWAY:
      CreateCheckSumDeviceToGetway = (msgDeviceGetway_t *)DataIn;
      SizeData = sizeof(msgDeviceGetway_t);
      CreateCheckSumDeviceToGetway->Crc = CheckSum(DataIn, SizeData);
      memcpy(DataOut, (uint8_t*)CreateCheckSumDeviceToGetway, SizeData);
      break;

    case GETWAYTODEVICE:
      CreateCheckSumGetwayToDevice = (msgDeviceGetway_t *)DataIn;
      SizeData = sizeof(msgDeviceGetway_t);
      CreateCheckSumGetwayToDevice->Crc =  CheckSum(DataIn, SizeData);;
      memcpy(DataOut, (uint8_t*)CreateCheckSumGetwayToDevice, SizeData);
      break;
  }
}

static uint8_t CheckDecrypted(uint8_t* DataIn, typePayload_t TypePayload, uint16_t *AdressUnicast, uint8_t Mode)
{
  uint8_t SizeData = 0;
  uint16_t HeaderCheck = 0xAA55;
  joinRequetstMsg_t *CreateCheckSumRequetst;
  joinAcceptMsg_t *CreateCheckSumAccept;
  joinConfirmMsg_t *CreateCheckSumConfirm;
  joinCompletedMsg_t *CreateCheckSumCompleted;
  msgDeviceGetway_t *CreateCheckSumDeviceToGetway, *CreateCheckSumGetwayToDevice;

  switch (TypePayload)
  {
    case REQUETST:
      CreateCheckSumRequetst = (joinRequetstMsg_t *)DataIn;
      SizeData =  sizeof(joinRequetstMsg_t);
      //APP_PPRINTF( "\n|||||||(CheckIn: %x, CheckOUT: %x )|||||||| \n\r", CreateCheckSumRequetst->Crc, CheckSum(DataIn, SizeData));
      if (Check_Header_TypeMsg(CreateCheckSumRequetst->Header, HeaderCheck, CreateCheckSumRequetst->TypeMsg, 0x01) && (CreateCheckSumRequetst->Crc == CheckSum(DataIn, SizeData)))
      {
        return 1;
      }
      break;

    case ACCEPT:

      CreateCheckSumAccept = (joinAcceptMsg_t *)DataIn;
      SizeData = sizeof(joinAcceptMsg_t);
      //APP_PPRINTF( "\n|||||||(CheckIn: %x, CheckOUT: %x )|||||||| \n\r", CreateCheckSumAccept->Crc, CheckSum(DataIn, SizeData));
      if (Check_Header_TypeMsg(CreateCheckSumAccept->Header, HeaderCheck, CreateCheckSumAccept->TypeMsg, 0x02) && (CreateCheckSumAccept->Crc == CheckSum(DataIn, SizeData)))
      {
        return 1;
      }
      break;

    case CONFIRM:
      CreateCheckSumConfirm = (joinConfirmMsg_t *)DataIn;
      SizeData = sizeof(joinConfirmMsg_t);
      if (Mode == 1)
      {
        if (Check_Header_TypeMsg(CreateCheckSumConfirm->Header, HeaderCheck, CreateCheckSumConfirm->TypeMsg, 0x03))
        {
          return 1;
        }
      }
      else
      {
        //APP_PPRINTF( "\n|||||||(CheckIn: %x, CheckOUT: %x )|||||||| \n\r", CreateCheckSumConfirm->Crc, CheckSum(DataIn, SizeData));
        if (CreateCheckSumConfirm->Crc == CheckSum(DataIn, SizeData))
        {
          return 1;
        }
      }
      break;
    case COMPLETED:
      CreateCheckSumCompleted = (joinCompletedMsg_t *)DataIn;
      SizeData = sizeof(joinCompletedMsg_t);
      if (Mode == 1)
      {
        if (Check_Header_TypeMsg(CreateCheckSumCompleted->Header, HeaderCheck, CreateCheckSumCompleted->TypeMsg, 0x04))
        {
          return 1;
        }
      }
      else
      {
        //APP_PPRINTF( "\n|||||||(CheckIn: %x, CheckOUT: %x )|||||||| \n\r", CreateCheckSumCompleted->Crc, CheckSum(DataIn, SizeData));
        if (CreateCheckSumCompleted->Crc == CheckSum(DataIn, SizeData))
        {
          return 1;
        }
      }
      break;

    case DEVICETOGETWAY:
      CreateCheckSumDeviceToGetway = (msgDeviceGetway_t *)DataIn;
      SizeData = sizeof(msgDeviceGetway_t);
      if (Mode == 1)
      {
        if (Check_Header_TypeMsg(CreateCheckSumDeviceToGetway->Header, HeaderCheck, CreateCheckSumDeviceToGetway->TypeMsg, 0x05))
        {
          *AdressUnicast = CreateCheckSumDeviceToGetway->Unicast;
          return 1;
        }
      }
      else
      {
        //APP_PPRINTF( "\n|||||||(CheckIn: %x, CheckOUT: %x )|||||||| \n\r", CreateCheckSumDeviceToGetway->Crc, CheckSum(DataIn, SizeData));
        if (CreateCheckSumDeviceToGetway->Crc == CheckSum(DataIn, SizeData))
        {
          *AdressUnicast = CreateCheckSumDeviceToGetway->Unicast;
          return 1;
        }
      }
      break;

    case GETWAYTODEVICE:
      CreateCheckSumGetwayToDevice = (msgDeviceGetway_t *)DataIn;
      SizeData = sizeof(msgDeviceGetway_t);
      if (Mode == 1)
      {

        if (Check_Header_TypeMsg(CreateCheckSumGetwayToDevice->Header, HeaderCheck, CreateCheckSumGetwayToDevice->TypeMsg, 0x06))
        {
          *AdressUnicast = CreateCheckSumGetwayToDevice->Unicast;
          return 1;
        }
      }
      else
      {
        if (CreateCheckSumGetwayToDevice->Crc == CheckSum(DataIn, SizeData))
        {
          *AdressUnicast = CreateCheckSumGetwayToDevice->Unicast;
          return 1;
        }
      }
      break;
  }
  return 0;
}

static uint8_t Check_Header_TypeMsg(uint16_t HeaderIn, uint16_t HeaderCheck, uint8_t TypeMsgIn, uint8_t TypeMsgCheck)
{
#if 0
  APP_PPRINTF( "\n|||||||(HeaderIn: %x, HeaderCheck: %x | TypeMsgIn: %x, TypeMsgCheck: %x)|||||||| \n\r", HeaderIn, HeaderCheck, TypeMsgIn, TypeMsgCheck);
#endif
  if ((HeaderIn == HeaderCheck) && (TypeMsgIn == TypeMsgCheck))
  {
    return 1;
  }
  return 0;
}

static uint8_t CheckSum(uint8_t* DataIn, uint8_t Length)
{
  uint8_t *Tempdata = DataIn;
  uint8_t BufferCounter;
  uint8_t TempCreateCheckSum = 0;

  for (BufferCounter = 0; BufferCounter < Length; BufferCounter++)
  {
    if (BufferCounter != 3)
    {
      TempCreateCheckSum ^= Tempdata[BufferCounter];
    }
  }
  return TempCreateCheckSum;
}


