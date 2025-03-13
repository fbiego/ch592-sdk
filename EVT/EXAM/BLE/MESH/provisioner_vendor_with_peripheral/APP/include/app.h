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

#define APP_NODE_EVT                    (1 << 0)
#define APP_NODE_TEST_EVT               (1 << 1)
#define APP_DELETE_NODE_TIMEOUT_EVT     (1 << 2)

#define CMD_DELETE_NODE                0xA2
#define CMD_DELETE_NODE_ACK            0x82

#define PERIPHERAL_CMD_LEN             1
#define ADDRESS_LEN                    2

//  1+2
#define DELETE_NODE_DATA_LEN           (PERIPHERAL_CMD_LEN + ADDRESS_LEN)
//  1
#define DELETE_NODE_ACK_DATA_LEN       (PERIPHERAL_CMD_LEN)

/******************************************************************************/

/**
 * @brief 
 */
typedef enum
{
    NODE_INIT = 0,
    NODE_APPKEY_ADD = 1,
    NODE_MOD_BIND_SET = 2,
    NODE_MOD_SUB_SET = 3,

    NODE_CONFIGURATIONED = 4,
} node_stage_t;

/**
 * @brief 
 */
typedef enum
{
    LOCAL_INIT = 0,
    LOCAL_APPKEY_ADD = 1,
    LOCAL_MOD_BIND_SET = 2,

    LOCAL_CONFIGURATIONED = 4,
} local_stage_t;

/**
 * @brief 
 */
typedef union
{
    node_stage_t  node;
    local_stage_t local;
} stage_t;

typedef void (*cfg_rsp_handler_t)(void *node, const void *rsp);
typedef BOOL (*stage_handler_t)(void *node);

/**
 * @brief 
 */
typedef struct
{
    cfg_rsp_handler_t rsp;
    stage_handler_t   stage;
} cfg_cb_t;

/**
 * @brief 
 */
typedef struct
{
    uint16_t node_addr;
    uint16_t elem_count;
    uint16_t net_idx;
    uint16_t retry_cnt : 12,
        fixed : 1,
        blocked : 1;

    stage_t         stage;
    const cfg_cb_t *cb;
} node_t;

typedef union
{
    struct
    {
        uint8_t cmd;                /*  CMD_DELETE_NODE */
        uint8_t addr[ADDRESS_LEN];  /*  */
    } delete_node;                  /*  */
    struct
    {
        uint8_t cmd;                /*  CMD_DELETE_NODE_ACK */
    } delete_node_ack;              /*  */
    struct
    {
        uint8_t buf[20]; /* */
    } data;
}app_mesh_manage_t;

extern node_t         app_nodes[1 + CONFIG_MESH_PROV_NODE_COUNT_DEF];
extern const uint16_t self_prov_net_idx;
extern const uint16_t self_prov_app_idx;
extern const uint32_t self_prov_iv_index;
extern const uint16_t self_prov_addr;

/**
 * @brief   
 */
void App_Init(void);

/******************************************************************************/

/******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif
