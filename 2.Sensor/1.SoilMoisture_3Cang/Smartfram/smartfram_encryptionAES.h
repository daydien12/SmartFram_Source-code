
#ifndef __ENCRYPTION_H__
#define __ENCRYPTION_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "sys_app.h"

__ALIGN_BEGIN extern uint32_t pNetKeyAES[] __ALIGN_END;
__ALIGN_BEGIN extern uint32_t pLocalDeviceKeyAES[] __ALIGN_END;

extern RNG_HandleTypeDef hrng;
/*Cac buoc cho DV join mang*/
typedef enum
{
  REQUETST,
  ACCEPT,
  CONFIRM,
  COMPLETED,
  DEVICETOGETWAY,
	GETWAYTODEVICE,
} typePayload_t;

/**
	EncryptedIn: chuoi can ma hoa
	SizeDataIn:	size chuoi dau can ma hoa
	EncryptedOut: chuoi da ma hoa xong
	SizeDataOut: size cua chuoi da ma hoa
	
**/
typedef struct 
{
 uint8_t *EncryptedIn;
 uint8_t SizeDataIn;
 uint8_t EncryptedOut[100];
 uint8_t SizeDataOut;
 typePayload_t TypePayload;
}encrypted_t;

/**
	DecryptedIn: chuoi can giai ma
	SizeIn:size chuoi dau can giai ma
	SizeTypeOut: size cua data out
	DecryptedOut: Chuoi giai ma
	Check: kiem tra dieu kien
**/
typedef struct 
{
 uint8_t *DecryptedIn;
 uint8_t SizeIn;
 uint8_t SizeTypeOut;
 uint8_t DecryptedOut[100];
 uint8_t Check;
 typePayload_t TypePayload;
}decrypted_t;


void Smartfram_Error_Handler(void);
void Smartfram_DMA_Init(void);
void Smartfram_AES_Init(void);
void Smartfram_RNG_Init(void);


/**
	Mo ta: Ham ma hoa 
	Tham so: encrypted_t 
**/
void Smartfram_Encrypted(encrypted_t* data);

/**
	Mo ta: Ham gia ma 
	Tham so: decrypted_t 
**/
void Smartfram_Decrypted(decrypted_t* data);

#ifdef __cplusplus
}
#endif

#endif
