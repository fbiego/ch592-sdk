/********************************** (C) COPYRIGHT *******************************
 * File Name          : app.h
 * Author             : WCH
 * Version            : V1.1
 * Date               : 2021/11/18
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#ifndef app_H
#define app_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/

/* The entire user code is divided into five pieces, 4K, 152K, 152K, 4K, 136K, and the next four pieces are called imageA (APP), imageB (OTA), imageIAP and LIB respectively. */

/* FLASH definition */
#define FLASH_BLOCK_SIZE       EEPROM_BLOCK_SIZE
#define IMAGE_SIZE             152 * 1024

/* imageA definition */
#define IMAGE_A_FLAG           0x01
#define IMAGE_A_START_ADD      0x1000
#define IMAGE_A_SIZE           IMAGE_SIZE

/* imageB definition */
#define IMAGE_B_FLAG           0x02
#define IMAGE_B_START_ADD      (IMAGE_A_START_ADD + IMAGE_SIZE)
#define IMAGE_B_SIZE           IMAGE_SIZE

/* imageIAP definition */
#define IMAGE_IAP_FLAG         0x03
#define IMAGE_IAP_START_ADD    (IMAGE_B_START_ADD + IMAGE_SIZE)
#define IMAGE_IAP_SIZE         4 * 1024

/* Stored in DataFlash address, cannot occupy Bluetooth location */
#define OTA_DATAFLASH_ADD      0x00077000 - FLASH_ROM_MAX_SIZE

/* OTA information stored in DataFlash */
typedef struct
{
    unsigned char ImageFlag; // The current image flag of the record
    unsigned char Revd[3];
} OTADataFlashInfo_t;
/******************************************************************************/

/******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif
