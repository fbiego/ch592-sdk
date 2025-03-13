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
#define OTA_IAP_END_EVT         0x0008  //OTA IAP_END
#define OTA_RESTART_EVT         0x0010  //OTA RESTART

#define CHIP_ID             0x9160

#define IAP_SAFE_FLAG       0x0d0251e0

/* ------------------------------------------------------------------------------------------------
 *                                             OTA FLASH
 * ------------------------------------------------------------------------------------------------
 */

/* The entire user code is divided into four pieces, 4K, 84K, 84K, 12K, and the last three pieces are called imageA (user code area), imageB (backup code area) and imageIAP respectively. */

/* FLASH definition */
#define FLASH_BLOCK_SIZE       EEPROM_BLOCK_SIZE
#define IMAGE_SIZE             84 * 1024

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

#define IAP_STATE_SUCCESS      0x00               // IAP Success
#define IAP_STATE_RETRAN       0xFE               // IAP重传
#define IAP_STATE_FAILURE      0xFF               // IAP失败

/* IAP definition */
/* The following is the IAP download command definition */
#define CMD_HAND_SHAKE         0x5A               // Handshake command
#define CMD_HAND_SHAKE_ACK     0xA5               // Handshake ACK

#define CMD_IAP_PROM           0x80               // IAP编程命令
#define CMD_IAP_ERASE          0x81               // IAP erase command
#define CMD_IAP_VERIFY         0x82               // IAP verification command
#define CMD_IAP_END            0x83               // IAP结束标志
#define CMD_IAP_INFO           0x84               // IAP selects firmware to obtain device information
#define CMD_IAP_INFO_ACK       0x04               // IAP Select Firmware ACK
#define CMD_SINGLE_CHANNEL     0xC0               // Single carrier command
#define CMD_SINGLE_POWER       0xC1               // Single carrier power
#define CMD_CAPACITANCE        0xC2               // Load Capacitor
#define CMD_RESTART            0xE0               // Restart command
#define CMD_CONFIG_VID_PID     0xE1               // VID_PID Command
#define CMD_CONFIG_MANU_INFO    0xE2               // MANU_INFO command
#define CMD_CONFIG_PROD_INFO    0xE3               // PROD_INFO command
#define CMD_CONFIG_SERIAL_NUM   0xE4               // SERIAL_NUM command

#define CMD_STATE_ACK          0x0F               // Status ACK

#define HEX_NOT_LOCAL          0x01

/* Data frame length definition */
#define IAP_LEN                68

/* Stored in DataFlash address, cannot occupy Bluetooth location */
#define OTA_DATAFLASH_ADD      0x00077000 - FLASH_ROM_MAX_SIZE

/* 存放在DataFlash里的OTA信息 */
typedef struct
{
    unsigned char ImageFlag; // The current image flag of the record
    unsigned char Revd[3];
} OTADataFlashInfo_t;

/* OTA IAP Communication Protocol Definition */
/* 地址使用4倍偏移 */
typedef union
{
    struct
    {
        unsigned char cmd;          /* 命令码 0x5A */
        unsigned char len;          /* Subsequent data length */
        unsigned char string[7];    /* WCH@IAP */
    } handshake; /* 握手命令 */
    struct
    {
        unsigned char cmd;          /* Command code 0x81 */
        unsigned char len;          /* Subsequent data length */
        unsigned char addr[2];      /* Erase address */
        unsigned char block_num[2]; /* Number of erased blocks */

    } erase; /* Erase command */
    struct
    {
        unsigned char cmd;       /* 命令码 0x83 */
        unsigned char len;       /* Subsequent data length */
        unsigned char status[2]; /* Two byte state, reserved */
    } end;                       /* End command */
    struct
    {
        unsigned char cmd;              /* 命令码 0x82 */
        unsigned char len;              /* Subsequent data length */
        unsigned char addr[2];          /* Verification address */
        unsigned char buf[IAP_LEN - 4]; /* Verify data */
    } verify;                           /* Verification command */
    struct
    {
        unsigned char cmd;              /* Command code 0x80 */
        unsigned char len;              /* Subsequent data length */
        unsigned char addr[2];          /* address */
        unsigned char buf[IAP_LEN - 4]; /* Follow-up data */
    } program;                          /* Programming commands */
    struct
    {
        unsigned char cmd;              /* Command code 0x84 */
        unsigned char len;              /* Subsequent data length */
        unsigned char hex[4];          /* Firmware number */
    } info;                             /* Programming commands */
    struct
    {
        unsigned char buf[IAP_LEN]; /* Receive packets */
    } other;
} OTA_IAP_CMD_t;

/* Record the current Image */
extern unsigned char CurrImageFlag;
extern uint8_t ota_send_buf[];
void OTA_Init(void);

void OTA_Enable(void);

uint8_t OTA_IAPWriteData(unsigned char *p_data, unsigned char w_len);

void OTA_IAP_SendData(void);

#endif
