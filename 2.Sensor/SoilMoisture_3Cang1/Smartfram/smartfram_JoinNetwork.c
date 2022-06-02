
#include "smartfram_JoinNetwork.h"

enum FSM_JOIN
{
  JOIN_REQUETST_TX 	= 0x01,
	JOIN_ACCEPT_RX 		= 0x02,
  JOIN_CONFIRM_TX 	= 0x03,
	JOIN_COMPLETED_RX = 0x04,
  JOIN_DONE_TX			= 0x05,
};

sm_lora_cmd_func_t JoinNetword_State[] = {
																					{JOIN_REQUETST_TX		, "FSM_JOIN_REQUETST_TX"		, Smartfram_JoinNetWork_SendRequetst	},
																					{JOIN_ACCEPT_RX			, "FSM_JOIN_ACCEPT_RX"			, Smartfram_JoinNetWork_GetAccept			},
																					{JOIN_CONFIRM_TX		, "FSM_JOIN_CONFIRM_TX"			, Smartfram_JoinNetWork_SendConfirm		},
																					{JOIN_COMPLETED_RX	, "FSM_JOIN_COMPLETED_RX"		, Smartfram_JoinNetWork_GetCompleted	},
																				};

/**
	Mo ta: Ham gui ban tin requetst cho getway
	Tham so:	
		là mot con tro ham
**/
uint8_t Smartfram_JoinNetWork_SendRequetst(void(*RadioSend)(uint8_t *str, uint8_t size), uint8_t Str_char[], uint8_t size)
{
	
	encrypted_t DataEnc;
	joinRequetstMsg_t JoinRequetst;
	
	JoinRequetst.Header = 0xAA55;
	JoinRequetst.TypeMsg = 0x01;
	JoinRequetst.MacAdress[0] =  Smartfram_GetMACAdress1();
	JoinRequetst.MacAdress[1] =  Smartfram_GetMACAdress2();
	
	DataEnc.TypePayload = REQUETST;
	DataEnc.EncryptedIn = (uint8_t*)&JoinRequetst;
	DataEnc.SizeDataIn = sizeof(joinRequetstMsg_t);
	Smartfram_Encrypted(&DataEnc);
	RadioSend(DataEnc.EncryptedOut, DataEnc.SizeDataOut);
	#if JOINNETWORK_DEBUG_SEND
		APP_PPRINTF( "\n|||||||(SendRequetst)|||||||| \n\r");
	#endif
	return 2;
}

/**
	Mo ta: Ham gui ban tin Confirm cho getway
	Tham so:	
		là mot con tro ham
**/
uint8_t Smartfram_JoinNetWork_SendConfirm(void(*RadioSend)(uint8_t *str, uint8_t size), uint8_t Str_char[], uint8_t size)
{
	encrypted_t DataEnc;
	joinConfirmMsg_t JoinConfirm;
	
	JoinConfirm.Header = 0xAA55;
	JoinConfirm.TypeMsg = 0x03;
	JoinConfirm.Option = Sys_DeviceFlashData.TypeSensor;
	JoinConfirm.MacAdress[0] =  Smartfram_GetMACAdress1();
	JoinConfirm.MacAdress[1] =  Smartfram_GetMACAdress2();
	
	JoinConfirm.Devkey[0] = pLocalDeviceKeyAES[0];
	JoinConfirm.Devkey[1] = pLocalDeviceKeyAES[1];
	JoinConfirm.Devkey[2] = pLocalDeviceKeyAES[2];
	JoinConfirm.Devkey[3] = pLocalDeviceKeyAES[3];
	
	DataEnc.TypePayload = CONFIRM;
	DataEnc.EncryptedIn = (uint8_t*)&JoinConfirm;
	DataEnc.SizeDataIn = sizeof(joinConfirmMsg_t);
	Smartfram_Encrypted(&DataEnc);
  RadioSend(DataEnc.EncryptedOut, DataEnc.SizeDataOut);
	#if JOINNETWORK_DEBUG_SEND
		APP_PPRINTF( "\n|||||||(SendConfirm)|||||||| \n\r");
	#endif
	return 2;
}
/**
	Mo ta: Ham get duu lieu tu ban tin phan hoi accept tu GW
	Tham so:	
		-Str_char: Mang(uint8_t) data data nhan duoc
		-size: Size mang dau vao
	Tra ve: neu ban tin dung thì tra ve 1 
**/
uint8_t Smartfram_JoinNetWork_GetAccept(void(*RadioSend)(uint8_t *str, uint8_t size), uint8_t Str_char[], uint8_t size)
{
	decrypted_t DataDec;
  joinAcceptMsg_t *JoinAccept;
  joinCompletedMsg_t *joinComplete;
	DataDec.DecryptedIn = Str_char;
	DataDec.SizeIn = size;
	DataDec.SizeTypeOut = sizeof(joinAcceptMsg_t);
	DataDec.TypePayload = ACCEPT;
	Smartfram_Decrypted(&DataDec);
	JoinAccept = (joinAcceptMsg_t *)DataDec.DecryptedOut;
	APP_PPRINTF( "\n|||||||(REQUETST_DONE Check: %d)|||||||| \n\r",DataDec.Check);
	#if JOINNETWORK_DEBUG_GET
	APP_PPRINTF( "\n|||||||(REQUETST_DONE Check: %d)|||||||| \n\r",DataDec.Check);
	
	APP_PPRINTF( "\n|||||||(REQUETST_DONE Check: %d)|||||||| \n\r",DataDec.Check);
	APP_PPRINTF( "JoinAccept->Header: %x\n\r", JoinAccept->Header);
	APP_PPRINTF( "JoinAccept->TypeMsg: %x\n\r", JoinAccept->TypeMsg);
	APP_PPRINTF( "JoinAccept->Crc: %x\n\r", JoinAccept->Crc);
	APP_PPRINTF( "JoinAccept->MacAdress \n\r");
	APP_PPRINTF( "%x\n\r", JoinAccept->MacAdress[0]);
	APP_PPRINTF( "%x\n\r", JoinAccept->MacAdress[1]);
	APP_PPRINTF( "JoinAccept->Netkey\n\r");
	APP_PPRINTF( "%x\n\r", JoinAccept->Netkey[0]);
	APP_PPRINTF( "%x\n\r", JoinAccept->Netkey[1]);
	APP_PPRINTF( "%x\n\r", JoinAccept->Netkey[2]);
	APP_PPRINTF( "%x\n\r", JoinAccept->Netkey[3]);
	APP_PPRINTF( "\n---------------- \n\r");
	#endif
	if(DataDec.Check == 1)
	{
		smartfram_SaveJointInfoAccept((uint32_t*)DataDec.DecryptedOut, sizeof(joinAcceptMsg_t)/4);
		return 1;
	}
	return 0;
}

/**
	Mo ta: Ham get duu lieu tu ban tin phan hoi Completed tu GW
	Tham so:	
		-Str_char: Mang(uint8_t) data data nhan duoc
		-size: Size mang dau vao
	Tra ve: neu ban tin dung thì tra ve 1 
**/
uint8_t Smartfram_JoinNetWork_GetCompleted(void(*RadioSend)(uint8_t *str, uint8_t size), uint8_t Str_char[], uint8_t size)
{
	decrypted_t DataDec;
  joinCompletedMsg_t *joinComplete;
	DataDec.DecryptedIn = Str_char;
	DataDec.SizeIn = size;
	DataDec.SizeTypeOut = sizeof(joinCompletedMsg_t);
	DataDec.TypePayload = COMPLETED;
	Smartfram_Decrypted(&DataDec);
	joinComplete = (joinCompletedMsg_t *)DataDec.DecryptedOut;
	APP_PPRINTF( "\n|||||||(CONFIRM_DONE  Check: %d)|||||||| \n\r",DataDec.Check);
	#if JOINNETWORK_DEBUG_GET
	APP_PPRINTF( "\n|||||||(CONFIRM_DONE  Check: %d)|||||||| \n\r",DataDec.Check);
	
	APP_PPRINTF( "\n|||||||(Check: %d)|||||||| \n\r",DataDec.Check);
	APP_PPRINTF( "joinComplete->Header: %x\n\r", joinComplete->Header);
	APP_PPRINTF( "joinComplete->TypeMsg: %x\n\r", joinComplete->TypeMsg);
	APP_PPRINTF( "joinComplete->Crc: %x\n\r", joinComplete->Crc);
	APP_PPRINTF( "joinComplete->MacAdress \n\r");
	APP_PPRINTF( "%x\n\r", joinComplete->MacAdress[0]);
	APP_PPRINTF( "%x\n\r", joinComplete->MacAdress[1]);
	APP_PPRINTF( "joinComplete->Unicast: %x\n\r", joinComplete->Unicast);
	APP_PPRINTF( "\n---------------- \n\r");
	#endif
	if(DataDec.Check == 1)
	{
		smartfram_SaveJointInfoCompleted(joinComplete->Unicast, MODE_RUN);
		return 1;
	}
	return 0;
}

uint8_t Smartfram_Test(tt_u *Data)
{
	APP_PPRINTF( "\nData: %s, Size: %d \n\r", Data->Data , Data->Siize);
	return 0;
}

uint8_t Smartfram_JoinNetWork_FindFunction(uint8_t State)
{
	for(int i=0; i<(sizeof(JoinNetword_State)/sizeof(*JoinNetword_State)); i++)
	{
		if(State == JoinNetword_State[i].Op)
		{
			return i;
		}
	}
	return 0;
}

uint32_t Smartfram_GetMACAdress1(void)
{
	return  LL_FLASH_GetUDN();
}

uint32_t Smartfram_GetMACAdress2(void)
{
	uint32_t temp =  (LL_FLASH_GetSTCompanyID()<<8) | LL_FLASH_GetDeviceID();
	return  temp;
}