/********************************** (C) COPYRIGHT *******************************
 * File Name          : app_vendor_model_srv.h
 * Author             : WCH
 * Version            : V1.1
 * Date               : 2021/11/18
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#ifndef app_vendor_model_srv_H
#define app_vendor_model_srv_H

#ifdef __cplusplus
extern "C" {
#endif

#include "MESH_LIB.h"

#define MSG_PIN                              GPIO_Pin_12

#define CID_WCH                              0x07D7

#define OP_VENDOR_MESSAGE_TRANSPARENT_CFM    BLE_MESH_MODEL_OP_3(0xCB, CID_WCH)
#define OP_VENDOR_MESSAGE_TRANSPARENT_WRT    BLE_MESH_MODEL_OP_3(0xCC, CID_WCH)
#define OP_VENDOR_MESSAGE_TRANSPARENT_ACK    BLE_MESH_MODEL_OP_3(0xCD, CID_WCH)
#define OP_VENDOR_MESSAGE_TRANSPARENT_IND    BLE_MESH_MODEL_OP_3(0xCE, CID_WCH)
#define OP_VENDOR_MESSAGE_TRANSPARENT_MSG    BLE_MESH_MODEL_OP_3(0xCF, CID_WCH)

#define BLE_MESH_MODEL_ID_WCH_SRV            0x0000
#define BLE_MESH_MODEL_ID_WCH_CLI            0x0001

#define VENDOR_MODEL_SRV_TRANS_EVT           (1 << 0)
#define VENDOR_MODEL_SRV_RSP_TIMEOUT_EVT     (1 << 1)
#define VENDOR_MODEL_SRV_INDICATE_EVT        (1 << 2)

/**
 * @brief 
 */
struct bt_adv_trans_cb
{
    void (*start)(int err, void *cb_data);
    void (*end)(int err, void *cb_data);
};

/**
 * @brief 
 */
struct send_param
{
    uint16_t net_idx;
    uint16_t app_idx;
    uint16_t addr;
    uint8_t  tid;
    uint8_t  trans_cnt;
    int32_t  period;
    int32_t  rand;
    uint8_t  send_ttl;

    void                         *cb_data;
    const struct bt_adv_trans_cb *cb;
};

/**
 * @brief indicate 
 */
struct bt_mesh_indicate
{
    struct send_param param;
    struct net_buf   *buf;
};

/**
 * @brief trans 
 */
struct bt_mesh_trans
{
    struct send_param param;
    struct net_buf   *buf;
};

/**
 * @brief write 
 */
struct bt_mesh_write
{
    struct send_param param;
    struct net_buf   *buf;
};

/**
 * @brief write 
 */
struct bt_mesh_vendor_model_write
{
    uint8_t *pdata;
    uint16_t len;
    uint16_t addr;
};

/**
 * @brief srv_trans 
 */
struct bt_mesh_vendor_model_srv_trans
{
    uint8_t *pdata;
    uint16_t len;
    uint16_t addr;
};

/**
 * @brief Event header
 */
struct vendor_model_srv_EventHdr
{
    uint8_t  status;
    uint32_t opcode;
};

/**
 * @brief Server event union
 */
union vendor_model_srv_Event_t
{
    struct bt_mesh_vendor_model_srv_trans trans;
    struct bt_mesh_vendor_model_write     write;
};

/**
 * @brief Server event struct
 */
typedef struct
{
    struct vendor_model_srv_EventHdr vendor_model_srv_Hdr;
    union vendor_model_srv_Event_t   vendor_model_srv_Event;
} vendor_model_srv_status_t;

typedef void (*vendor_model_srv_rsp_handler_t)(const vendor_model_srv_status_t *val);

/**
 * @brief tid
 */
struct vendor_model_srv_tid
{
    uint8_t trans_tid;
    uint16_t trans_addr;
    uint8_t write_tid;
    uint16_t write_addr;
};

/**
 * @brief Mesh vendor_model_srv Model Context
 */
struct bt_mesh_vendor_model_srv
{
    struct bt_mesh_model          *model;
    uint32_t                       op_req;
    uint32_t                       op_pending;
    struct vendor_model_srv_tid    srv_tid;
    vendor_model_srv_rsp_handler_t handler;
};

extern const struct bt_mesh_model_op vnd_model_srv_op[];

/**
 * @brief   led
 *
 * @param   led_pin - 
 *
 * @return  led
 */
BOOL read_led_state(uint32_t led_pin);

/**
 * @brief   led
 *
 * @param   led_pin - 
 * @param   on      - 
 */
void set_led_state(uint32_t led_pin, BOOL on);

/**
 * @brief   led
 *
 * @param   led_pin - 
 */
void toggle_led_state(uint32_t led_pin);

/**
 * @brief   indicate,
 *
 * @param   param   - .
 * @param   pData   - .
 * @param   len     - ,(APP_MAX_TX_SIZE).
 *
 * @return  Global_Error_Code
 */
int vendor_message_srv_indicate(struct send_param *param, uint8_t *pData, uint16_t len);

/**
 * @brief   send_trans,
 *
 * @param   param   - .
 * @param   pData   - .
 * @param   len     - ,(APP_MAX_TX_SIZE).
 *
 * @return  Global_Error_Code
 */
int vendor_message_srv_send_trans(struct send_param *param, uint8_t *pData, uint16_t len);

/**
 * @brief   TID selection method
 *
 * @return  TID
 */
uint8_t vendor_srv_tid_get(void);

/**
 * @brief   
 */
void vendor_message_srv_trans_reset(void);

/**
 * @brief   
 *
 * @param   model       - 
 *
 * @return  always SUCCESS
 */
int vendor_model_srv_init(struct bt_mesh_model *model);


#ifdef __cplusplus
}
#endif

#endif
