/* ********************************* (C) COPYRIGHT *******************************
* File Name          : ota.h
* Author             : WCH
* Version            : V1.10
* Date               : 2018/12/14
* Description        : oad相关配置定义
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
****************************************************************************** */

#ifndef __OTA_H
#define __OTA_H

/* ------------------------------------------------------------------------------------------------
 *                                             OTA FLASH
 * ------------------------------------------------------------------------------------------------
 */

/* The entire user code is divided into four pieces, 4K, 84K, 84K, 12K, and the last three pieces are called imageA (user code area), imageB (backup code area) and imageIAP respectively. */

/* FLASH定义 */
#define FLASH_BLOCK_SIZE       EEPROM_BLOCK_SIZE
#define IMAGE_SIZE             84 * 1024

/* imageA definition */
#define IMAGE_A_FLAG           0x01
#define IMAGE_A_START_ADD      0x1000
#define IMAGE_A_SIZE           IMAGE_SIZE

/* imageB definition */
#define IMAGE_B_FLAG           0x02
#define IMAGE_B_START_ADD      (IMAGE_A_START_ADD + IMAGE_SIZE)
#define IMAGE_B_SIZE           IMAGE_SIZE

/* imageIAP定义 */
#define IMAGE_IAP_FLAG         0x03
#define IMAGE_IAP_START_ADD    (IMAGE_B_START_ADD + IMAGE_SIZE)
#define IMAGE_IAP_SIZE         12 * 1024

/* IAP definition */
/* The following is the IAP download command definition */
#define CMD_IAP_PROM           0x80               // IAP编程命令
#define CMD_IAP_ERASE          0x81               // IAP erase command
#define CMD_IAP_VERIFY         0x82               // IAP verification command
#define CMD_IAP_END            0x83               // IAP End Flag
#define CMD_IAP_INFO           0x84               // IAP obtains device information

/* Data frame length definition */
#define IAP_LEN                20

/* Stored in DataFlash address, cannot occupy Bluetooth location */
#define OTA_DATAFLASH_ADD      0x00077000 - FLASH_ROM_MAX_SIZE

/* 存放在DataFlash里的OTA信息 */
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
        unsigned char cmd;          /* 命令码 0x81 */
        unsigned char len;          /* Subsequent data length */
        unsigned char addr[2];      /* Erase address */
        unsigned char block_num[2]; /* 擦除块数 */

    } erase; /* Erase command */
    struct
    {
        unsigned char cmd;       /* Command code 0x83 */
        unsigned char len;       /* Subsequent data length */
        unsigned char status[2]; /* 两字节状态，保留 */
    } end;                       /* End command */
    struct
    {
        unsigned char cmd;              /* 命令码 0x82 */
        unsigned char len;              /* Subsequent data length */
        unsigned char addr[2];          /* Verification address */
        unsigned char buf[IAP_LEN - 4]; /* 校验数据 */
    } verify;                           /* Verification command */
    struct
    {
        unsigned char cmd;              /* 命令码 0x80 */
        unsigned char len;              /* Subsequent data length */
        unsigned char addr[2];          /* address */
        unsigned char buf[IAP_LEN - 4]; /* Follow-up data */
    } program;                          /* Programming commands */
    struct
    {
        unsigned char cmd;              /* Command code 0x84 */
        unsigned char len;              /* 后续数据长度 */
        unsigned char buf[IAP_LEN - 2]; /* 后续数据 */
    } info;                             /* 编程命令 */
    struct
    {
        unsigned char buf[IAP_LEN]; /* Receive packets */
    } other;
} OTA_IAP_CMD_t;

/* 记录当前的Image */
extern unsigned char CurrImageFlag;

#endif
