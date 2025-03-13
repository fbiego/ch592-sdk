/********************************** (C) COPYRIGHT *******************************
 * File Name          : app.h
 * Author             : WCH
 * Version            : V1.1
 * Date               : 2022/03/31
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

#define APP_NODE_EVT                       (1 << 0)
#define APP_NODE_PROVISION_EVT             (1 << 1)
#define APP_DELETE_NODE_TIMEOUT_EVT        (1 << 2)
#define APP_DELETE_LOCAL_NODE_EVT          (1 << 3)
#define APP_DELETE_NODE_INFO_EVT           (1 << 4)
#define APP_ASK_STATUS_NODE_TIMEOUT_EVT    (1 << 5)
#define APP_OTA_UPDATE_TIMEOUT_EVT         (1 << 6)
#define APP_SET_SUB_TIMEOUT_EVT            (1 << 7)

#define CMD_PROVISION_INFO                 0xA0
#define CMD_PROVISION_INFO_ACK             0x80
#define CMD_PROVISION                      0xA1
#define CMD_PROVISION_ACK                  0x81
#define CMD_DELETE_NODE                    0xA2
#define CMD_DELETE_NODE_ACK                0x82
#define CMD_DELETE_NODE_INFO               0xA3
#define CMD_DELETE_NODE_INFO_ACK           0x83
#define CMD_ASK_STATUS                     0xA4
#define CMD_ASK_STATUS_ACK                 0x84
#define CMD_TRANSFER                       0xA5
#define CMD_TRANSFER_RECEIVE               0x85
#define CMD_IMAGE_INFO                     0xA6
#define CMD_IMAGE_INFO_ACK                 0x86
#define CMD_UPDATE                         0xA7
#define CMD_UPDATE_ACK                     0x87
#define CMD_VERIFY                         0xA8
#define CMD_VERIFY_ACK                     0x88
#define CMD_END                            0xA9
#define CMD_SET_SUB                        0xAA
#define CMD_SET_SUB_ACK                    0x8A
#define CMD_LOCAL_RESET                    0xAF
#define CMD_LOCAL_RESET_ACK                0x8F

#define PERIPHERAL_CMD_LEN                 1
#define PROVISION_NET_KEY_LEN              16
#define ADDRESS_LEN                        2
#define UPDATE_ADDRESS_LEN                 2

//  1+1+4iv index+1flag
#define PROVISION_INFO_DATA_LEN            (PERIPHERAL_CMD_LEN + 1 + 4 + 1)
//  1+1+4iv index+1flag
#define PROVISION_INFO_ACK_DATA_LEN        (PERIPHERAL_CMD_LEN + 1 + 4 + 1)
//  1+16+2
#define PROVISION_DATA_LEN                 (PERIPHERAL_CMD_LEN + PROVISION_NET_KEY_LEN + ADDRESS_LEN)
//  1+2+1
#define PROVISION_ACK_DATA_LEN             (PERIPHERAL_CMD_LEN + ADDRESS_LEN + 1)
//  1+2
#define DELETE_NODE_DATA_LEN               (PERIPHERAL_CMD_LEN + ADDRESS_LEN)
//  1+2+1
#define DELETE_NODE_ACK_DATA_LEN           (PERIPHERAL_CMD_LEN + ADDRESS_LEN + 1)
//  1
#define DELETE_NODE_INFO_DATA_LEN          (PERIPHERAL_CMD_LEN)
//  1+2
#define DELETE_NODE_INFO_ACK_DATA_LEN      (PERIPHERAL_CMD_LEN + ADDRESS_LEN)
//  1+2
#define ASK_STATUS_DATA_LEN                (PERIPHERAL_CMD_LEN + ADDRESS_LEN)
//  1+2+1
#define ASK_STATUS_ACK_DATA_LEN            (PERIPHERAL_CMD_LEN + ADDRESS_LEN + 1)
//  1+2+N
#define TRANSFER_DATA_LEN                  (PERIPHERAL_CMD_LEN + ADDRESS_LEN)
//  1+2+N
#define TRANSFER_RECEIVE_DATA_LEN          (PERIPHERAL_CMD_LEN + ADDRESS_LEN)
// OTA 1+2
#define IMAGE_INFO_DATA_LEN                (PERIPHERAL_CMD_LEN + ADDRESS_LEN)
// OTA 1+2+4image+2+2+1
#define IMAGE_INFO_ACK_DATA_LEN            (PERIPHERAL_CMD_LEN + ADDRESS_LEN + 4 + 2 + 2 + 1)
// OTA 1+2+2flash+N
#define UPDATE_DATA_LEN                    (PERIPHERAL_CMD_LEN + ADDRESS_LEN + UPDATE_ADDRESS_LEN)
// OTA 1+2+2flash+1
#define UPDATE_ACK_DATA_LEN                (PERIPHERAL_CMD_LEN + ADDRESS_LEN + UPDATE_ADDRESS_LEN + 1)
// OTA 1+2+2flash+N
#define VERIFY_DATA_LEN                    (PERIPHERAL_CMD_LEN + ADDRESS_LEN + UPDATE_ADDRESS_LEN)
// OTA 1+2+2flash+1
#define VERIFY_ACK_DATA_LEN                (PERIPHERAL_CMD_LEN + ADDRESS_LEN + UPDATE_ADDRESS_LEN + 1)
// OTA 1+2
#define END_DATA_LEN                       (PERIPHERAL_CMD_LEN + ADDRESS_LEN)
//  1+2+1+2
#define SET_SUB_DATA_LEN                   (PERIPHERAL_CMD_LEN + ADDRESS_LEN + 1 + ADDRESS_LEN)
//  1+2+1
#define SET_SUB_ACK_DATA_LEN               (PERIPHERAL_CMD_LEN + ADDRESS_LEN + 1)
//  1
#define LOCAL_RESET_DATA_LEN               (PERIPHERAL_CMD_LEN)
//  1+1
#define LOCAL_RESET_ACK_DATA_LEN           (PERIPHERAL_CMD_LEN + 1)

// 
#define STATUS_SUCCESS                     0x00
#define STATUS_TIMEOUT                     0x01
#define STATUS_NOMEM                       0x02
#define STATUS_INVALID                     0x03

#define APP_MAX_TX_SIZE                    MAX(CONFIG_MESH_UNSEG_LENGTH_DEF, CONFIG_MESH_TX_SEG_DEF *BLE_MESH_APP_SEG_SDU_MAX - 8)

/* code4K152K152K4K136KimageAAPPimageBOTAimageIAPLIB */

/* FLASH */
#define FLASH_BLOCK_SIZE                   EEPROM_BLOCK_SIZE
#define IMAGE_SIZE                         152 * 1024

/* imageA */
#define IMAGE_A_FLAG                       0x01
#define IMAGE_A_START_ADD                  0x1000
#define IMAGE_A_SIZE                       IMAGE_SIZE

/* imageB */
#define IMAGE_B_FLAG                       0x02
#define IMAGE_B_START_ADD                  (IMAGE_A_START_ADD + IMAGE_SIZE)
#define IMAGE_B_SIZE                       IMAGE_SIZE

/* imageIAP */
#define IMAGE_IAP_FLAG                     0x03
#define IMAGE_IAP_START_ADD                (IMAGE_B_START_ADD + IMAGE_SIZE)
#define IMAGE_IAP_SIZE                     4 * 1024

/* DataFlash */
#define OTA_DATAFLASH_ADD                  0x00077000 - FLASH_ROM_MAX_SIZE

/* DataFlashOTA */
typedef struct
{
    unsigned char ImageFlag; //image
    unsigned char Revd[3];
} OTADataFlashInfo_t;

/******************************************************************************/

typedef struct
{
    uint16_t node_addr;
    uint16_t elem_count;
    uint16_t net_idx;
    uint16_t retry_cnt : 12,
        fixed : 1,
        blocked : 1;

} node_t;

typedef union
{
    struct
    {
        uint8_t cmd;         /*  CMD_PROVISION_INFO */
        uint8_t set_flag;    /*  10*/
        uint8_t iv_index[4]; /* iv index */
        uint8_t flag;        /* Net key refresh flag */
    } provision_info;        /*  */
    struct
    {
        uint8_t cmd;         /*  CMD_PROVISION_INFO_ACK */
        uint8_t status;      /* */
        uint8_t iv_index[4]; /* iv index */
        uint8_t flag;        /* Net key refresh flag */
    } provision_info_ack;    /*  */
    struct
    {
        uint8_t cmd;                            /*  CMD_PROVISION */
        uint8_t net_key[PROVISION_NET_KEY_LEN]; /*  */
        uint8_t addr[ADDRESS_LEN];              /*  */
    } provision;                                /*  */
    struct
    {
        uint8_t cmd;               /*  CMD_PROVISION_ACK */
        uint8_t addr[ADDRESS_LEN]; /*  */
        uint8_t status;            /*  */
    } provision_ack;               /*  */
    struct
    {
        uint8_t cmd;               /*  CMD_DELETE_NODE */
        uint8_t addr[ADDRESS_LEN]; /*  */
    } delete_node;                 /*  */
    struct
    {
        uint8_t cmd;               /*  CMD_DELETE_NODE_ACK */
        uint8_t addr[ADDRESS_LEN]; /*  */
        uint8_t status;            /*  */
    } delete_node_ack;             /*  */
    struct
    {
        uint8_t cmd;    /*  CMD_DELETE_NODE_INFO */
    } delete_node_info; /*  */
    struct
    {
        uint8_t cmd;               /*  CMD_DELETE_NODE_INFO_ACK */
        uint8_t addr[ADDRESS_LEN]; /*  */
    } delete_node_info_ack;        /*  */
    struct
    {
        uint8_t cmd;               /*  CMD_ASK_STATUS */
        uint8_t addr[ADDRESS_LEN]; /*  */
    } ask_status;                  /*  */
    struct
    {
        uint8_t cmd;               /*  CMD_ASK_STATUS_ACK */
        uint8_t addr[ADDRESS_LEN]; /*  */
        uint8_t status;            /*  */
    } ask_status_ack;              /*  */
    struct
    {
        uint8_t cmd;                       /*  CMD_TRANSFER */
        uint8_t addr[ADDRESS_LEN];         /*  */
        uint8_t data[APP_MAX_TX_SIZE - 3]; /* */
    } transfer;                            /*  */
    struct
    {
        uint8_t cmd;                       /*  CMD_TRANSFER_ACK */
        uint8_t addr[ADDRESS_LEN];         /*  */
        uint8_t data[APP_MAX_TX_SIZE - 3]; /* */
    } transfer_receive;                    /*  */
    struct
    {
        uint8_t cmd;               /*  CMD_IMAGE_INFO */
        uint8_t addr[ADDRESS_LEN]; /*  */
    } image_info;                  /* OTA */
    struct
    {
        uint8_t cmd;               /*  CMD_IMAGE_INFO_ACK */
        uint8_t addr[ADDRESS_LEN]; /*  */
        uint8_t image_size[4];     /* image */
        uint8_t block_size[2];     /* falsh */
        uint8_t chip_id[2];        /*  */
        uint8_t status;            /*  */
    } image_info_ack;              /* OTA */
    struct
    {
        uint8_t cmd;                             /*  CMD_UPDATE */
        uint8_t addr[ADDRESS_LEN];               /*  */
        uint8_t update_addr[UPDATE_ADDRESS_LEN]; /*  */
        uint8_t data[APP_MAX_TX_SIZE - 5];       /* */
    } update;                                    /* OTA */
    struct
    {
        uint8_t cmd;                             /*  CMD_UPDATE_ACK */
        uint8_t addr[ADDRESS_LEN];               /*  */
        uint8_t update_addr[UPDATE_ADDRESS_LEN]; /*  */
        uint8_t status;                          /*  */
    } update_ack;                                /* OTA */
    struct
    {
        uint8_t cmd;                             /*  CMD_VERIFY */
        uint8_t addr[ADDRESS_LEN];               /*  */
        uint8_t update_addr[UPDATE_ADDRESS_LEN]; /*  */
        uint8_t data[APP_MAX_TX_SIZE - 5];       /* */
    } verify;                                    /* OTA */
    struct
    {
        uint8_t cmd;                             /*  CMD_VERIFY_ACK */
        uint8_t addr[ADDRESS_LEN];               /*  */
        uint8_t update_addr[UPDATE_ADDRESS_LEN]; /*  */
        uint8_t status;                          /*  */
    } verify_ack;                                /* OTA */
    struct
    {
        uint8_t cmd;               /*  CMD_END */
        uint8_t addr[ADDRESS_LEN]; /*  */
    } end;                         /* OTA */
    struct
    {
        uint8_t cmd;                   /*  CMD_SET_SUB */
        uint8_t addr[ADDRESS_LEN];     /*  */
        uint8_t add_flag;              /* 10 */
        uint8_t sub_addr[ADDRESS_LEN]; /*  */
    } set_sub;                         /*  */
    struct
    {
        uint8_t cmd;               /*  CMD_SET_SUB_ACK */
        uint8_t addr[ADDRESS_LEN]; /*  */
        uint8_t status;            /*  */
    } set_sub_ack;                 /*  */
    struct
    {
        uint8_t cmd; /*  CMD_LOCAL_RESET */
    } local_reset;   /*  */
    struct
    {
        uint8_t cmd;    /*  CMD_LOCAL_RESET */
        uint8_t status; /*  */
    } local_reset_ack;  /*  */
    struct
    {
        uint8_t buf[APP_MAX_TX_SIZE]; /* */
    } data;
} app_mesh_manage_t;

void App_Init(void);

void App_peripheral_reveived(uint8_t *pValue, uint16_t len);

/******************************************************************************/

/******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif
