
#include "smartfram_JoinNetwork.h"
#include "smartfram_structpayload.h"
#include "smartfram_encryptionAES.h"
#include "smartfram_FlashSys.h"

uint32_t Smartfram_GetMACAdress1(void);
uint32_t Smartfram_GetMACAdress2(void);

void Smartfram_JoinNetWork_SendAccept(void(*RadioSend)(uint8_t *str, uint8_t size))
{
	encrypted_t DataEnc;
	joinAcceptMsg_t joinAccept;
	
	joinAccept.Header = 0xAA55;
	joinAccept.TypeMsg = 0x02;
	joinAccept.MacAdress[0] = Smartfram_GetMACAdress1();
	joinAccept.MacAdress[1] = Smartfram_GetMACAdress2();
	joinAccept.Netkey[0] = pLocalGetwayKeyAES[0];
	joinAccept.Netkey[1] = pLocalGetwayKeyAES[1];
	joinAccept.Netkey[2] = pLocalGetwayKeyAES[2];
	joinAccept.Netkey[3] = pLocalGetwayKeyAES[3];
	DataEnc.TypePayload = ACCEPT;

	DataEnc.EncryptedIn = (uint8_t*)&joinAccept;
	DataEnc.SizeDataIn = sizeof(joinAcceptMsg_t);
	Smartfram_Encrypted(&DataEnc);
  RadioSend(DataEnc.EncryptedOut, DataEnc.SizeDataOut);
}

void Smartfram_JoinNetWork_SendCompleted(void(*RadioSend)(uint8_t *str, uint8_t size))
{
	encrypted_t DataEnc;
	joinCompletedMsg_t joinCompleted;
	
	joinCompleted.Header = 0xAA55;
	joinCompleted.TypeMsg = 0x04;
	joinCompleted.MacAdress[0] =  Smartfram_GetMACAdress1();
	joinCompleted.MacAdress[1] =  Smartfram_GetMACAdress2();
	joinCompleted.Unicast = Sys_Unicast;
	DataEnc.TypePayload = COMPLETED;
	DataEnc.EncryptedIn = (uint8_t*)&joinCompleted;
	DataEnc.SizeDataIn = sizeof(joinCompletedMsg_t);
	Smartfram_Encrypted(&DataEnc);
	RadioSend(DataEnc.EncryptedOut, DataEnc.SizeDataOut);
}

uint8_t Smartfram_JoinNetWork_GetRequetst(uint8_t Str_char[], uint8_t size)
{
	decrypted_t DataDec;
	joinRequetstMsg_t* JoinRequetst;
	DataDec.DecryptedIn = Str_char;
	DataDec.SizeIn = size;
	DataDec.SizeTypeOut = sizeof(joinRequetstMsg_t);
	DataDec.TypePayload = REQUETST;
	Smartfram_Decrypted(&DataDec);
	
	#if JOINNETWORK_DEBUG_GET_DATA
	APP_PPRINTF( "\n|||||||(JOIN NETWORK DEBUG: ACCEPT_DONE Check = %d)|||||||| \n\r",DataDec.Check);
	JoinRequetst = (joinRequetstMsg_t *)DataDec.DecryptedOut;
	APP_PPRINTF( "JoinRequetst->Header: %x\n\r", JoinRequetst->Header);
	APP_PPRINTF( "JoinRequetst->TypeMsg: %x\n\r", JoinRequetst->TypeMsg);
	APP_PPRINTF( "JoinRequetst->Crc: %x\n\r", JoinRequetst->Crc);
	APP_PPRINTF( "JoinRequetst->MacAdress \n\r");
	APP_PPRINTF( "%x\n\r", JoinRequetst->MacAdress[0]);
	APP_PPRINTF( "%x\n\r", JoinRequetst->MacAdress[1]);
	APP_PPRINTF( "\n---------------- \n\r");
	#endif
	if(DataDec.Check == 1)
	{
		smartfram_SaveJointInfoRequetst((uint32_t*)DataDec.DecryptedOut, DataDec.SizeTypeOut);
		return 1;
	}
	return 0;
}

uint8_t Smartfram_JoinNetWork_GetConfirm(uint8_t Str_char[], uint8_t size)
{
	decrypted_t DataDec;
	joinConfirmMsg_t* JoinConfirm;
	DataDec.DecryptedIn = Str_char;
	DataDec.SizeIn = size;
	DataDec.SizeTypeOut = sizeof(joinConfirmMsg_t);
	DataDec.TypePayload = CONFIRM;
	Smartfram_Decrypted(&DataDec);
	
	#if JOINNETWORK_DEBUG_GET_DATA
		APP_PPRINTF( "\n|||||||(JOIN NETWORK DEBUG: COMPLETED_DONE Check = %d)|||||||| \n\r",DataDec.Check);
		JoinConfirm = (joinConfirmMsg_t *)DataDec.DecryptedOut;
		APP_PPRINTF( "JoinConfirm->Header: %x\n\r", JoinConfirm->Header);
		APP_PPRINTF( "JoinConfirm->TypeMsg: %x\n\r", JoinConfirm->TypeMsg);
		APP_PPRINTF( "JoinConfirm->Crc: %x\n\r", JoinConfirm->Crc);
		APP_PPRINTF( "JoinConfirm->Option: %x\n\r", JoinConfirm->Option);
		APP_PPRINTF( "JoinConfirm->MacAdress \n\r");
		APP_PPRINTF( "%x\n\r", JoinConfirm->MacAdress[0]);
		APP_PPRINTF( "%x\n\r", JoinConfirm->MacAdress[1]);
		APP_PPRINTF( "JoinConfirm->Devkey\n\r");
		APP_PPRINTF( "%x\n\r", JoinConfirm->Devkey[0]);
		APP_PPRINTF( "%x\n\r", JoinConfirm->Devkey[1]);
		APP_PPRINTF( "%x\n\r", JoinConfirm->Devkey[2]);
		APP_PPRINTF( "%x\n\r", JoinConfirm->Devkey[3]);
		APP_PPRINTF( "\n---------------- \n\r");
	#endif
	if(DataDec.Check == 1)
	{
		smartfram_SaveJointInfoConfirm((uint32_t*)DataDec.DecryptedOut, DataDec.SizeTypeOut);
		return 1;
	}
	return 0;
}

uint8_t Smartfram_JoinNetWork_GetJoinDone(uint8_t Str_char[], uint8_t size)
{
	decrypted_t DataDec;
	joinConfirmMsg_t* JoinConfirm;
	DataDec.DecryptedIn = Str_char;
	DataDec.SizeIn = size;
	DataDec.SizeTypeOut = sizeof(joinConfirmMsg_t);
	DataDec.TypePayload = CONFIRM;
	Smartfram_Decrypted(&DataDec);
	
	#if JOINNETWORK_DEBUG_GET_DATA
		APP_PPRINTF( "\n|||||||(JOIN NETWORK DEBUG: COMPLETED_DONE Check: %d)|||||||| \n\r",DataDec.Check);
		JoinConfirm = (joinConfirmMsg_t *)DataDec.DecryptedOut;
		APP_PPRINTF( "JoinConfirm->Header: %x\n\r", JoinConfirm->Header);
		APP_PPRINTF( "JoinConfirm->TypeMsg: %x\n\r", JoinConfirm->TypeMsg);
		APP_PPRINTF( "JoinConfirm->Crc: %x\n\r", JoinConfirm->Crc);
		APP_PPRINTF( "JoinConfirm->Option: %x\n\r", JoinConfirm->Option);
		APP_PPRINTF( "JoinConfirm->MacAdress \n\r");
		APP_PPRINTF( "%x\n\r", JoinConfirm->MacAdress[0]);
		APP_PPRINTF( "%x\n\r", JoinConfirm->MacAdress[1]);
		APP_PPRINTF( "JoinConfirm->Devkey\n\r");
		APP_PPRINTF( "%x\n\r", JoinConfirm->Devkey[0]);
		APP_PPRINTF( "%x\n\r", JoinConfirm->Devkey[1]);
		APP_PPRINTF( "%x\n\r", JoinConfirm->Devkey[2]);
		APP_PPRINTF( "%x\n\r", JoinConfirm->Devkey[3]);
		APP_PPRINTF( "\n---------------- \n\r");
	#endif
	if(DataDec.Check == 1)
	{
		smartfram_SaveJointInfoJoinDone();
		return 1;
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