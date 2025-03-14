/* ********************************* (C) COPYRIGHT *******************************
* File Name          : OTA.h
* Author             : WCH
* Version            : V1.10
* Date               : 2018/12/14
* Description        : oad相关配置定义
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
****************************************************************************** */

/******************************************************************************/
#ifndef __OTA_H
#define __OTA_H

#define OTA_FLASH_ERASE_EVT     0x0004  // OTA Flash Erase Task
#define OTA_IAP_END_EVT         0x0010  //OTA_IAP_END_EVT
#define OTA_FLASH_PROM_EVT      0x0020  //OTA Flash
#define OTA_FLASH_VRIF_EVT      0x0040  //OTA Flash
#define OTA_RESTART_EVT         1<<8  //OTA RESTART

#define IAP_SAFE_FLAG       0x91600592

#define CHIP_ID             0x92
/* ------------------------------------------------------------------------------------------------
 *                                             OTA FLASH
 * ------------------------------------------------------------------------------------------------
 */

/* The entire user code is divided into four pieces, 4K, 216K, 216K, and 12K. The next three pieces are called imageA (APP), imageB (OTA) and imageIAP respectively. */

/* FLASH definition */
#define FLASH_BLOCK_SIZE       EEPROM_BLOCK_SIZE
#define IMAGE_SIZE             216 * 1024

/* imageA definition */
#define IMAGE_A_FLAG           0x01
#define IMAGE_A_START_ADD      4 * 1024
#define IMAGE_A_SIZE           IMAGE_SIZE

/* imageB definition */
#define IMAGE_B_FLAG           0x02
#define IMAGE_B_START_ADD      (IMAGE_A_START_ADD + IMAGE_SIZE)
#define IMAGE_B_SIZE           IMAGE_SIZE

/* imageIAP definition */
#define IMAGE_IAP_FLAG         0x03
#define IMAGE_IAP_START_ADD    (IMAGE_B_START_ADD + IMAGE_SIZE)
#define IMAGE_IAP_SIZE         12 * 1024

/* IAP definition */
#define IAP_STATE_SUCCESS      0x00               // IAP成功
#define IAP_STATE_RETRAN       0x01               // IAP retransmission
#define IAP_STATE_FAILURE      0xFF               // IAP failed

/* The following is the IAP download command definition */
#define CMD_HAND_SHAKE         0x5A               // Handshake command
#define CMD_HAND_SHAKE_ACK     0xA5               // Handshake ACK

#define CMD_IAP_PROM           0x80               // IAP programming commands
#define CMD_IAP_ERASE          0x81               // IAP erase command
#define CMD_IAP_VERIFY         0x82               // IAP verification command
#define CMD_IAP_END            0x83               // IAP End Flag
#define CMD_IAP_INFO           0x84               // IAP obtains device information
#define CMD_IAP_INFO_ACK       0x04               // IAP Select Firmware ACK

#define CMD_STATE_ACK          0x0F               // Status ACK

/* Data frame length definition */
#define IAP_LEN                64//36

#define IAP_SRC_RF             0x01
#define IAP_SRC_BLE            0x02
#define IAP_SRC_USB            0x03

/* Stored in DataFlash address, cannot occupy Bluetooth location */
#define OTA_DATAFLASH_ADD      0x00077000 - FLASH_ROM_MAX_SIZE

/* OTA information stored in DataFlash */
typedef struct
{
    unsigned char ImageFlag; // The current image flag of the record
    unsigned char Revd[3];
} OTADataFlashInfo_t;

/* OTA IAP Communication Protocol Definition */
/* Addresses use 4x offset */
typedef union
{
    struct
    {
        unsigned char cmd;          /* Command code 0x5A */
        unsigned char len;          /* Subsequent data length */
        unsigned char string[7];    /* WCH@IAP */
    } handshake; /* Handshake command */
    struct
    {
        unsigned char cmd;          /* Command code 0x81 */
        unsigned char len;          /* Subsequent data length */
        unsigned char addr[2];      /* Erase address */
        unsigned char block_num[2]; /* Number of erased blocks */

    } erase; /* Erase command */
    struct
    {
        unsigned char cmd;       /* Command code 0x83 */
        unsigned char len;       /* Subsequent data length */
        unsigned char status[2]; /* Two byte state, reserved */
    } end;                       /* End command */
    struct
    {
        unsigned char cmd;              /* 命令码 0x82 */
        unsigned char len;              /* 后续数据长度 */
        unsigned char addr[2];          /* Verification address */
        unsigned char buf[IAP_LEN - 4]; /* Verify data */
    } verify;                           /* Verification command */
    struct
    {
        unsigned char cmd;              /* Command code 0x80 */
        unsigned char len;              /* Subsequent data length */
        unsigned char addr[2];          /* address */
        unsigned char buf[IAP_LEN - 4]; /* 后续数据 */
    } program;                          /* 编程命令 */
    struct
    {
        unsigned char cmd;              /* 命令码 0x84 */
        unsigned char len;              /* Subsequent data length */
        unsigned char buf[IAP_LEN - 2]; /* Follow-up data */
    } info;                             /* Programming commands */
    struct
    {
        unsigned char buf[IAP_LEN]; /* Receive packets */
    } other;
} OTA_IAP_CMD_t;

/* Record the current Image */
extern unsigned char CurrImageFlag;
extern uint8_t ota_taskID;
extern __attribute__((aligned(4))) uint8_t flash_buf[256];

void OTA_Init(void);

void OTA_Enable(void);
void OTA_USB_IAPWriteData(unsigned char *p_data, unsigned char w_len);

void OTA_RF_IAPWritData(unsigned char *p_data, unsigned char w_len);
#endif
