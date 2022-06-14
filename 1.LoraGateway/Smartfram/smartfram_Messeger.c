#include "smartfram_Messeger.h"
#include "smartfram_JoinNetwork.h"
#include "smartfram_structpayload.h"
#include "smartfram_encryptionAES.h"
#include "smartfram_FlashSys.h"
#include "stdio.h"
#include "usart_if.h"
#include "smartfram_command.h"

#define DEBUG_DATA_SENSOR 0

#define DEBUG_MESSEGER 0
#define ACK 0x5A
#define ACK_REMOVE 0xF0

enum
{
	SENSOR_HUM_TEMP,
	NONE,
	SENSOR_SOIL_MOISTURE,
  SENSOR_LIGHT,
	SENSOR_CO2,
};

msgDeviceGetway_t SendMsgGetwayToDevice;
uint16_t Sys_ArrIDMsg[MAX_ADD_DEVICE] = {0};

void FS_SetData(uint8_t TypdeSensor, uint8_t *Arr, uint8_t *Ack, uint8_t *Pin, uint32_t *IdMsg, float *Data1, float *Data2);

//void FS_SetData( uint8_t *Arr, uint8_t *Ack, uint8_t *Pin, uint32_t *IdMsg, uint32_t *Data1, uint32_t *Data2);

void Smartfram_Messeger_SendGetwayToDevice(void(*RadioSend)(uint8_t *str, uint8_t size))
{
  encrypted_t DataEnc;
  uint8_t BufferCounter;
	uint8_t TempPage, i;
	#if SYS_MODE_BUG_NOT_SEND_TO_PI
	SendMsgGetwayToDevice.Unicast = 1;
	SendMsgGetwayToDevice.Msg[0] = 0;
	SendMsgGetwayToDevice.Msg[1] = 1;
	FlagCommandSYS.GetACKSensor = 1;
	#endif
	
	SendMsgGetwayToDevice.Header = 0xAA55;
	SendMsgGetwayToDevice.TypeMsg = 0x06;
	for(i=2; i<10; i++)
	{
		SendMsgGetwayToDevice.Msg[2] = 0x00;
	}
	FlagCommandSYS.GetACKSensor = 0;
	
	#if MSG_DEBUG_SEND
	APP_PPRINTF( "\n|||||||(MSG DEBUG: Send Getway To Device)|||||||| \n\r");
	APP_PPRINTF( "Header: %x\n\r", SendMsgGetwayToDevice.Header);
	APP_PPRINTF( "TypeMsg: %x\n\r", SendMsgGetwayToDevice.TypeMsg);
	APP_PPRINTF( "Crc: %x\n\r", SendMsgGetwayToDevice.Crc);
	APP_PPRINTF( "Unicast: %x\n\r", SendMsgGetwayToDevice.Unicast);
	APP_PPRINTF( "IdMsg: %d\n\r", SendMsgGetwayToDevice.IdMsg);
	APP_PPRINTF( "Length: %x\n\r", SendMsgGetwayToDevice.Length);
	APP_PPRINTF( "DELETE: %d\n\r", SendMsgGetwayToDevice.Msg[0]);
	APP_PPRINTF( "TIME: %d\n\r", SendMsgGetwayToDevice.Msg[1]);
	#endif
	
	DataEnc.TypePayload = GETWAYTODEVICE;
	DataEnc.EncryptedIn = (uint8_t*)&SendMsgGetwayToDevice;
	DataEnc.SizeDataIn = sizeof(msgDeviceGetway_t);
	Smartfram_Encrypted(&DataEnc);
	RadioSend(DataEnc.EncryptedOut, DataEnc.SizeDataOut);
	if(SendMsgGetwayToDevice.Msg[0] == 1)
	{
		TempPage = smartfram_FindUnicast(SendMsgGetwayToDevice.Unicast);
		smartfram_FlashRemoveInforDevice(TempPage);
		Sys_ArrIDMsg[TempPage] = 0;
	}
}

uint8_t Smartfram_Messeger_GetDeviceToGetway(uint8_t Str_char[], uint8_t size)
{
  decrypted_t DataDec;
  uint8_t BufferCounter;
  uint8_t TempPage;
  char buffer[100];
	uint8_t Pin = 0, Ack = 0;
	uint32_t IdMsg = 0;
	float	Data1 = 0, Data2 = 0;
	
  msgDeviceGetway_t *GetMsgDeviceToGetway;
  DataDec.DecryptedIn = Str_char;
  DataDec.SizeIn = size;
  DataDec.SizeTypeOut =  sizeof(msgDeviceGetway_t);
  DataDec.TypePayload = DEVICETOGETWAY;
  Smartfram_Decrypted(&DataDec);
  GetMsgDeviceToGetway = (msgDeviceGetway_t *)DataDec.DecryptedOut;
  TempPage = smartfram_FindUnicast(GetMsgDeviceToGetway->Unicast);
	
	FS_SetData(Sys_GetwayFlashArrData[TempPage].TypeSensor ,GetMsgDeviceToGetway->Msg, &Ack, &Pin, &IdMsg, &Data1, &Data2);
	
  if ((DataDec.Check == 1) &&  (Sys_ArrIDMsg[TempPage] < IdMsg))
  {
#if MSG_DEBUG_GET_CHECK_COUNT_IDMSG
    APP_PPRINTF( "\n|||||||(MSG DEBUG: Get Device To Getway)|||||||| \n\r");
    APP_PPRINTF( "Header: %x\n\r", GetMsgDeviceToGetway->Header);
    APP_PPRINTF( "TypeMsg: %x\n\r", GetMsgDeviceToGetway->TypeMsg);
    APP_PPRINTF( "Crc: %x\n\r", GetMsgDeviceToGetway->Crc);
    APP_PPRINTF( "Unicast: %x\n\r", GetMsgDeviceToGetway->Unicast);
		APP_PPRINTF( "ACK: %x\n\r", Ack);
    APP_PPRINTF( "IdMsg: %d\n\r", IdMsg);
	 for (BufferCounter = 0; BufferCounter < 10; BufferCounter++)
  {
    APP_PPRINTF( "[%d]%x ", BufferCounter , GetMsgDeviceToGetway->Msg[BufferCounter]);
  }
  APP_PPRINTF( "\n---------------- \n\r");
#endif

    Sys_ArrIDMsg[TempPage] = IdMsg;
		if(Ack == ACK)
		{
			switch (Sys_GetwayFlashArrData[TempPage].TypeSensor)
			{
				case SENSOR_LIGHT:
					Smartfram_Command_LIGHT((uint32_t)Data1, Pin, TempPage,'\n');
					#if DEBUG_DATA_SENSOR
					APP_PPRINTF("Uni:%d LIGHT: %d Pin:%d IdMsg:%d \n", GetMsgDeviceToGetway->Unicast,(uint32_t)Data1, Pin, Sys_ArrIDMsg[TempPage]);
					#endif
					#if MSG_DEBUG_GET_SENSOR
					APP_PPRINTF("\n|||||||(MSG DEBUG: LIGHT SENSOR = %d)|||||||| \n", DataDec.Check);
					APP_PPRINTF("Unicast: %x\n",GetMsgDeviceToGetway->Unicast);
					APP_PPRINTF("ACK:   %x \n" ,Ack);
					APP_PPRINTF("LIGHT: %d\n"  ,(uint32_t)Data1);
					APP_PPRINTF("PIN:   %d\n"  ,Pin);
					APP_PPRINTF("IdMsg: %d\n"  ,Sys_ArrIDMsg[TempPage]);
					APP_PPRINTF("smartfram_FindUnicast: %d\n", TempPage);
					APP_PPRINTF("\n---------------- \n");
					#endif
					break;

				case SENSOR_CO2:
					Smartfram_Command_CCS811((uint32_t)Data1, (uint32_t)Data2, Pin, TempPage, '\n');
				#if MSG_DEBUG_GET_SENSOR
						APP_PPRINTF("\n|||||||(MSG DEBUG: CO2 SENSOR = %d)|||||||| \n\r", DataDec.Check);
						APP_PPRINTF("Unicast: %x\n", GetMsgDeviceToGetway->Unicast);
						APP_PPRINTF("ACK:   %x \n"   ,Ack);
						sprintf(buffer, "%.2f", (((float)Data1) ));
						APP_PPRINTF( "CO2: %s \n" , buffer);
						sprintf(buffer, "%.2f", (((float)Data2)));
						APP_PPRINTF( "TVOC: %s \n" , buffer);
						APP_PPRINTF("PIN:   %d \n"   ,Pin);
						APP_PPRINTF("IdMsg: %d\n\r", Sys_ArrIDMsg[TempPage]);
						APP_PPRINTF("\n smartfram_FindUnicast: %d\n", TempPage);
						APP_PPRINTF("\n---------------- \n\r");
					#endif
					break;

				case SENSOR_HUM_TEMP:
					Smartfram_Command_TEMP_HUM(Data1, Data2, Pin, TempPage, '\n');
					#if DEBUG_DATA_SENSOR
					sprintf(buffer, "Uni:%d Data1:%.2f Data2:%.2f Pin:%d IdMsg:%d",GetMsgDeviceToGetway->Unicast, Data1/10.0, Data2/10.0,Pin, Sys_ArrIDMsg[TempPage]);
					APP_PPRINTF( "%s \n" , buffer);
					#endif
					#if MSG_DEBUG_GET_SENSOR
						//APP_PPRINTF( "\n|||||||(MSG DEBUG: HUM_TEMP SENSOR = %d)|||||||| \n\r", DataDec.Check);
						APP_PPRINTF( "Unicast: %x\n", GetMsgDeviceToGetway->Unicast);
						APP_PPRINTF( "ACK:     %x \n" , Ack);
				
						sprintf(buffer, "%.2f", (((float)Data1) / 10.0));
						APP_PPRINTF( "TEMP: %s \n" , buffer);
				
						sprintf(buffer, "%.2f", (((float)Data2) / 10.0));
						APP_PPRINTF( "HUM: %s \n" , buffer);
				
						APP_PPRINTF( "PIN: %d \n" , Pin);
						APP_PPRINTF( "IdMsg: %d\n\r", Sys_ArrIDMsg[TempPage]);
						//APP_PPRINTF("\n smartfram_FindUnicast: %d\n", TempPage);
						//APP_PPRINTF( "\n---------------- \n\r");
					#endif
					break;

				case SENSOR_SOIL_MOISTURE:
					Smartfram_Command_SoilMoisture((uint32_t)Data1, Pin, TempPage,'\n');
					#if DEBUG_DATA_SENSOR
					APP_PPRINTF("Uni:%d ADC: %d Pin:%d IdMsg:%d \n", GetMsgDeviceToGetway->Unicast,(uint32_t)Data1, Pin, Sys_ArrIDMsg[TempPage]);
					#endif
					#if MSG_DEBUG_GET_SENSOR
						APP_PPRINTF( "\n|||||||(CMSG DEBUG: SOIL_MOISTURE SENSOR = %d)|||||||| \n\r", DataDec.Check);
						APP_PPRINTF( "Unicast: %x\n\r",GetMsgDeviceToGetway->Unicast);
						APP_PPRINTF( "ACK:     %x \n" ,Ack);
						APP_PPRINTF( "ADC:     %d \n" ,(uint32_t)Data1);
						APP_PPRINTF( "PIN:     %d \n" ,Pin);
						APP_PPRINTF( "IdMsg: %d\n\r", Sys_ArrIDMsg[TempPage]);
						APP_PPRINTF("\n smartfram_FindUnicast: %d\n", TempPage);
						APP_PPRINTF( "\n---------------- \n\r");
					#endif
					break;
			}
		}
		else if(Ack == ACK_REMOVE)
		{
			smartfram_FlashRemoveInforDevice(TempPage);
			Sys_ArrIDMsg[TempPage] = 0;
			#if MSG_DEBUG_GET_REMOVE_NOTE
				APP_PPRINTF( "CMSG DEBUG: IdMsg: %d\n\r", Sys_ArrIDMsg[TempPage]);
				APP_PPRINTF( "\n--------(CMSG DEBUG: REMOVE DONE)-------- \n\r");
			#endif
		}
    /*Sau khi kiem tra ban tin hop le thi phai tim devicekey de ma hoa ban tin de gui ban tin ACK phan hoi cho device*/
    smartfram_SetKeyDevice(TempPage);
    return 1;
  }
  return 0;
}

void FS_SetData(uint8_t TypdeSensor, uint8_t *Arr, uint8_t *Ack, uint8_t *Pin, uint32_t *IdMsg, float *Data1, float *Data2)
{
 *Ack =  Arr[0];
 *Pin =  Arr[1];
	
	u8Tou32_u DataTemp;
	DataTemp.Data_In[0] = Arr[2];
	DataTemp.Data_In[1] = Arr[3];
	*IdMsg = DataTemp.Data_Out;
	

	if(TypdeSensor == SENSOR_HUM_TEMP)
	{
		DataTemp.Data_In[0] = Arr[4];
		DataTemp.Data_In[1] = Arr[5];
		
		if( Arr[6] == 1)
		{
			*Data1 = DataTemp.Data_Out;
			*Data1 *= (-1.0);
		}
		else
		{
			*Data1 = DataTemp.Data_Out;
		}
		

		DataTemp.Data_In[0] = Arr[7];
		DataTemp.Data_In[1] = Arr[8];
		if( Arr[9] == 1)
		{
			*Data2 = DataTemp.Data_Out*(-1);
			*Data2 *= (-1.0);
		}
		else
		{
			*Data2 = DataTemp.Data_Out;
		}
		//APP_PPRINTF( "DATA1: %d -- DATA2: %d \n\r", Arr[6], Arr[9]);
	}
	else
	{
		DataTemp.Data_In[0] = Arr[4];
		DataTemp.Data_In[1] = Arr[5];
		DataTemp.Data_In[2] = Arr[6];
		*Data1 = DataTemp.Data_Out;
		
		DataTemp.Data_In[0] = Arr[7];
		DataTemp.Data_In[1] = Arr[8];
		DataTemp.Data_In[2] = Arr[9];
		*Data2 = DataTemp.Data_Out;
	}
}
