
#include "smartfram_Messeger.h"

#define ACK 0x5A
#define ACK_REMOVE 0xF0
#define GET_ACK_REMOVE 0x01
uint32_t TempACK = ACK;

void FS_SetData(uint8_t *Arr, uint8_t Ack, uint8_t Pin, uint32_t IdMsg, uint32_t Data1, uint32_t Data2);
void FS_SetData_SHT30(uint8_t *Arr, uint8_t Ack, uint8_t Pin, uint32_t IdMsg, float Data1, float Data2);

void Smartfram_Messeger_SendDeviceToGetway(void(*RadioSend)(uint8_t *str, uint8_t size), uint8_t Str_char[], uint8_t size)
{
  encrypted_t DataEnc;
  uint8_t BufferCounter;
  smartfram_IncrementIdMsg(Sys_DataSensorRead.Data_Senser_Temperature,  Sys_DataSensorRead.Data_Senser_Humidity);

  msgDeviceGetway_t SendMsgDeviceToGetway;
  SendMsgDeviceToGetway.Header = 0xAA55;
  SendMsgDeviceToGetway.TypeMsg = 0x05;
  SendMsgDeviceToGetway.Crc = 0x55;
  SendMsgDeviceToGetway.Unicast = Sys_DeviceFlashData.Unicast;
	Smartfram_SYS_DebugSensor(1);
	#if MSG_DEBUG_SEND_ID_MSG
	 APP_PPRINTF( "\n|||||||(MSG DEBUG: IdMsg: %d)|||||||| \n\r", Sys_DeviceFlashData_MSG.ID_Msg);
	#endif
	
	/*ham hien thi gia tri cam bien*/
	Smartfram_SYS_DebugSensor(MSG_DEBUG_SEND_SENSOR);
	HAL_Delay(1000);
	FS_SetData_SHT30(SendMsgDeviceToGetway.Msg, (uint8_t)TempACK, Sys_DataSensorRead.Data_Battery, Sys_DeviceFlashData_MSG.ID_Msg, Sys_DataSensorRead.Data_Senser_Temperature*10, Sys_DataSensorRead.Data_Senser_Humidity*10);
	TempACK = ACK;
  DataEnc.TypePayload = DEVICETOGETWAY;
  DataEnc.EncryptedIn = (uint8_t*)&SendMsgDeviceToGetway;
  DataEnc.SizeDataIn = sizeof(msgDeviceGetway_t);
  Smartfram_Encrypted(&DataEnc);
  RadioSend(DataEnc.EncryptedOut, DataEnc.SizeDataOut);
}

uint8_t Smartfram_Messeger_GetGetwayToDevice(void(*RadioSend)(uint8_t *str, uint8_t size), uint8_t Str_char[], uint8_t size)
{
	char buffer_temp[30];
  decrypted_t DataDec;
  uint8_t BufferCounter;
	
  msgDeviceGetway_t *GetMsgGetwayToDevice;
  DataDec.DecryptedIn = Str_char;
  DataDec.SizeIn = size;
  DataDec.SizeTypeOut =  sizeof(msgDeviceGetway_t);
  DataDec.TypePayload = GETWAYTODEVICE;
  Smartfram_Decrypted(&DataDec);
  GetMsgGetwayToDevice = (msgDeviceGetway_t *)DataDec.DecryptedOut;
	
#if MSG_DEBUG_GET
  APP_PPRINTF( "\n|||||||(Check: %d)|||||||| \n\r", DataDec.Check);
  APP_PPRINTF( "Header: %x\n\r", GetMsgGetwayToDevice->Header);
  APP_PPRINTF( "TypeMsg: %x\n\r", GetMsgGetwayToDevice->TypeMsg);
  APP_PPRINTF( "Crc: %x\n\r", GetMsgGetwayToDevice->Crc);
  APP_PPRINTF( "Unicast: %x\n\r", GetMsgGetwayToDevice->Unicast);
  for (BufferCounter = 0; BufferCounter < 10; BufferCounter++)
  {
    APP_PPRINTF( "[%d]%x ", BufferCounter , GetMsgGetwayToDevice->Msg[BufferCounter]);
  }
  APP_PPRINTF( "\n---------------- \n\r");
#endif

  if (DataDec.Check == 1)
  {
    if (GetMsgGetwayToDevice->Msg[0] == GET_ACK_REMOVE)
    {
			TempACK = ACK_REMOVE;
      return 2;
    }
    else
    {
			if((GetMsgGetwayToDevice->Msg[1] != Sys_DeviceFlashData_MSG.TimeWakeup) && (GetMsgGetwayToDevice->Msg[1] > 0))
			{
				smartfram_UpdateMSG_TimeWakeup(GetMsgGetwayToDevice->Msg[1]);
			}
      return 1;
    }	
  }
  return 0;
}


void FS_SetData(uint8_t *Arr, uint8_t Ack, uint8_t Pin, uint32_t IdMsg, uint32_t Data1, uint32_t Data2)
{
	u32Tou8_u DataTemp;
	Arr[0] = Ack;
	Arr[1] = Pin;
	
	DataTemp.Data_In = IdMsg;
	Arr[2] = DataTemp.Data_Out[0];
	Arr[3] = DataTemp.Data_Out[1];
	
	DataTemp.Data_In = Data1;
	Arr[4] = DataTemp.Data_Out[0];
	Arr[5] = DataTemp.Data_Out[1];
	Arr[6] = DataTemp.Data_Out[2];
	
	DataTemp.Data_In = Data2;
	Arr[7] = DataTemp.Data_Out[0];
	Arr[8] = DataTemp.Data_Out[1];
	Arr[9] = DataTemp.Data_Out[2];
}


void FS_SetData_SHT30(uint8_t *Arr, uint8_t Ack, uint8_t Pin, uint32_t IdMsg, float Data1, float Data2)
{
	char buffer[30];
	uint8_t SignTemp = 0;
	u32Tou8_u DataTemp;

	

	Arr[0] = Ack;
	Arr[1] = Pin;
	
	DataTemp.Data_In = IdMsg;
	Arr[2] = DataTemp.Data_Out[0];
	Arr[3] = DataTemp.Data_Out[1];
	
	if(Data1 > 0)
	{
		SignTemp = 0;
	}
	else
	{
		SignTemp = 1;
		Data1 *= -1;
	}
	DataTemp.Data_In = (uint32_t)Data1;
	Arr[4] = DataTemp.Data_Out[0];
	Arr[5] = DataTemp.Data_Out[1];
	Arr[6] =SignTemp;
	
	if(Data2 > 0)
	{
		SignTemp = 0;
	}
	else
	{
		SignTemp = 1;
		Data2 *= -1;
	}
	DataTemp.Data_In = (uint32_t)Data2;
	Arr[7] = DataTemp.Data_Out[0];
	Arr[8] = DataTemp.Data_Out[1];
	Arr[9] = SignTemp;
}