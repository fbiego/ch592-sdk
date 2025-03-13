/********************************** (C) COPYRIGHT *******************************
 * File Name          : app.c
 * Author             : WCH
 * Version            : V1.1
 * Date               : 2022/01/18
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/******************************************************************************/
#include "CONFIG.h"
#include "MESH_LIB.h"
#include "app_vendor_model_srv.h"
#include "app_vendor_model_cli.h"
#include "app.h"
#include "HAL.h"

/*********************************************************************
 * GLOBAL TYPEDEFS
 */
#define ADV_TIMEOUT       K_MINUTES(10)

#define SELENCE_ADV_ON    0x01
#define SELENCE_ADV_OF    0x00

/*********************************************************************
 * GLOBAL TYPEDEFS
 */

static uint8_t MESH_MEM[1024 * 2 + 30 * CONFIG_MESH_PROV_NODE_COUNT_DEF] = {0};

extern const ble_mesh_cfg_t app_mesh_cfg;
extern const struct device  app_dev;

static uint8_t App_TaskID = 0; // Task ID for internal task/event processing

static uint16_t App_ProcessEvent(uint8_t task_id, uint16_t events);

static uint8_t dev_uuid[16] = {0}; // UUID
uint8_t        MACAddr[6];         // mac

static const uint8_t self_prov_net_key[16] = {
    0x00, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
    0x00, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
};

static const uint8_t self_prov_dev_key[16] = {
    0x00, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
    0x00, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
};

static const uint8_t self_prov_app_key[16] = {
    0x00, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
    0x00, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
};

/*  The key indexes are 12-bit values ranging from 0x000 to 0xFFF
    inclusive. A network key at index 0x000 is called the primary NetKey*/
const uint16_t self_prov_net_idx = 0x0000;      // net key
const uint16_t self_prov_app_idx = 0x0001;      // app key
const uint32_t self_prov_iv_index = 0x00000000; // iv_index
const uint16_t self_prov_addr = 0x0001;         // 
const uint8_t  self_prov_flags = 0x00;          // key
const uint16_t vendor_sub_addr = 0xC000;        // group

#if(!CONFIG_BLE_MESH_PB_GATT)
NET_BUF_SIMPLE_DEFINE_STATIC(rx_buf, 65);
#endif /* !PB_GATT */

/*********************************************************************
 * LOCAL FUNCION
 */

static void link_open(bt_mesh_prov_bearer_t bearer);
static void link_close(bt_mesh_prov_bearer_t bearer, uint8_t reason);
static void prov_complete(uint16_t net_idx, uint16_t addr, uint8_t flags, uint32_t iv_index);
static void unprov_recv(bt_mesh_prov_bearer_t bearer,
                        const uint8_t uuid[16], bt_mesh_prov_oob_info_t oob_info,
                        const unprivison_info_t *info);
static void node_added(uint16_t net_idx, uint16_t addr, uint8_t num_elem);
static node_t *node_get(uint16_t node_addr);
static void cfg_cli_rsp_handler(const cfg_cli_status_t *val);
static void vendor_model_cli_rsp_handler(const vendor_model_cli_status_t *val);
static int  vendor_model_cli_send(uint16_t addr, uint8_t *pData, uint16_t len);
static void node_init(void);

static struct bt_mesh_cfg_srv cfg_srv = {
#if(CONFIG_BLE_MESH_RELAY)
    .relay = BLE_MESH_RELAY_ENABLED,
#endif
    .beacon = BLE_MESH_BEACON_ENABLED,
#if(CONFIG_BLE_MESH_FRIEND)
    .frnd = BLE_MESH_FRIEND_ENABLED,
#endif
#if(CONFIG_BLE_MESH_PROXY)
    .gatt_proxy = BLE_MESH_GATT_PROXY_ENABLED,
#endif
    /* TTL3 */
    .default_ttl = 3,
    /* 710ms */
    .net_transmit = BLE_MESH_TRANSMIT(7, 10),
    /* 710ms */
    .relay_retransmit = BLE_MESH_TRANSMIT(7, 10),
};

static struct bt_mesh_health_srv health_srv;

BLE_MESH_HEALTH_PUB_DEFINE(health_pub, 8);

struct bt_mesh_cfg_cli cfg_cli = {
    .handler = cfg_cli_rsp_handler,
};

uint16_t cfg_srv_keys[CONFIG_MESH_MOD_KEY_COUNT_DEF] = {BLE_MESH_KEY_UNUSED};
uint16_t cfg_srv_groups[CONFIG_MESH_MOD_GROUP_COUNT_DEF] = {BLE_MESH_ADDR_UNASSIGNED};

uint16_t cfg_cli_keys[CONFIG_MESH_MOD_KEY_COUNT_DEF] = {BLE_MESH_KEY_UNUSED};
uint16_t cfg_cli_groups[CONFIG_MESH_MOD_GROUP_COUNT_DEF] = {BLE_MESH_ADDR_UNASSIGNED};

uint16_t health_srv_keys[CONFIG_MESH_MOD_KEY_COUNT_DEF] = {BLE_MESH_KEY_UNUSED};
uint16_t health_srv_groups[CONFIG_MESH_MOD_GROUP_COUNT_DEF] = {BLE_MESH_ADDR_UNASSIGNED};

// root
static struct bt_mesh_model root_models[] = {
    BLE_MESH_MODEL_CFG_SRV(cfg_srv_keys, cfg_srv_groups, &cfg_srv),
    BLE_MESH_MODEL_CFG_CLI(cfg_cli_keys, cfg_cli_groups, &cfg_cli),
    BLE_MESH_MODEL_HEALTH_SRV(health_srv_keys, health_srv_groups, &health_srv, &health_pub),
};

struct bt_mesh_vendor_model_cli vendor_model_cli = {
    .cli_tid.trans_tid = 0xFF,
    .cli_tid.ind_tid = 0xFF,
    .handler = vendor_model_cli_rsp_handler,
};

uint16_t vnd_model_cli_keys[CONFIG_MESH_MOD_KEY_COUNT_DEF] = {BLE_MESH_KEY_UNUSED};
uint16_t vnd_model_cli_groups[CONFIG_MESH_MOD_GROUP_COUNT_DEF] = {BLE_MESH_ADDR_UNASSIGNED};

// 
struct bt_mesh_model vnd_models[] = {
    BLE_MESH_MODEL_VND_CB(CID_WCH, BLE_MESH_MODEL_ID_WCH_CLI, vnd_model_cli_op, NULL, vnd_model_cli_keys,
                          vnd_model_cli_groups, &vendor_model_cli, NULL),
};

//  elements
static struct bt_mesh_elem elements[] = {
    {
        /* Location Descriptor (GATT Bluetooth Namespace Descriptors) */
        .loc = (0),
        .model_count = ARRAY_SIZE(root_models),
        .models = (root_models),
        .vnd_model_count = ARRAY_SIZE(vnd_models),
        .vnd_models = (vnd_models),
    }
};

// elements  Node Composition
const struct bt_mesh_comp app_comp = {
    .cid = 0x07D7, // WCH id
    .elem = elements,
    .elem_count = ARRAY_SIZE(elements),
};

// 
static const struct bt_mesh_prov app_prov = {
    .uuid = dev_uuid,
    .link_open = link_open,
    .link_close = link_close,
    .complete = prov_complete,
    .unprovisioned_beacon = unprov_recv,
    .node_added = node_added,
};

// 012
node_t app_nodes[1 + CONFIG_MESH_PROV_NODE_COUNT_DEF] = {0};

app_mesh_manage_t app_mesh_manage;

uint16_t reset_node_addr = BLE_MESH_ADDR_UNASSIGNED;
uint8_t settings_load_over = FALSE;
/*********************************************************************
 * GLOBAL TYPEDEFS
 */

/*********************************************************************
 * @fn      link_open
 *
 * @brief   link
 *
 * @param   bearer  - linkPB_ADVPB_GATT
 *
 * @return  none
 */
static void link_open(bt_mesh_prov_bearer_t bearer)
{
    APP_DBG("");
}

/*********************************************************************
 * @fn      link_close
 *
 * @brief   link
 *
 * @param   bearer  - linkPB_ADVPB_GATT
 * @param   reason  - link
 *
 * @return  none
 */
static void link_close(bt_mesh_prov_bearer_t bearer, uint8_t reason)
{
    APP_DBG("reason %x", reason);
    if(reason == CLOSE_REASON_RESOURCES)
    {
        //      ()
        bt_mesh_provisioner_disable(BLE_MESH_PROV_ADV, TRUE);
        //bt_mesh_node_clear();node_init();
        //bt_mesh_node_del_by_addr(app_nodes[1].node_addr);
    }
    else
    {
        bt_mesh_provisioner_enable(BLE_MESH_PROV_ADV);
    }
}

/*********************************************************************
 * @fn      node_unblock_get
 *
 * @brief     node
 *
 * @return  node_t / NULL
 */
static node_t *node_unblock_get(void)
{
    for(int i = 0; i < ARRAY_SIZE(app_nodes); i++)
    {
        if(app_nodes[i].node_addr != BLE_MESH_ADDR_UNASSIGNED &&
           app_nodes[i].fixed != TRUE &&
           app_nodes[i].blocked == FALSE)
        {
            return &app_nodes[i];
        }
    }
    return NULL;
}

/*********************************************************************
 * @fn      node_block_get
 *
 * @brief    node
 *
 * @return  node_t / NULL
 */
static node_t *node_block_get(void)
{
    for(int i = 0; i < ARRAY_SIZE(app_nodes); i++)
    {
        if(app_nodes[i].node_addr != BLE_MESH_ADDR_UNASSIGNED &&
           app_nodes[i].fixed != TRUE &&
           app_nodes[i].blocked == TRUE)
        {
            return &app_nodes[i];
        }
    }
    return NULL;
}

/*********************************************************************
 * @fn      node_work_handler
 *
 * @brief   node 
 *
 * @return  TRUE    
 *          FALSE   
 */
static BOOL node_work_handler(void)
{
    node_t *node;

    node = node_unblock_get();
    if(!node)
    {
        APP_DBG("Unable find Unblocked Node");
        return FALSE;
    }

    if(node->retry_cnt-- == 0)
    {
        APP_DBG("Ran Out of Retransmit");
        // 
        bt_mesh_node_del_by_addr(node->node_addr);
        node = node_get(node->node_addr);
        node->stage.node = NODE_INIT;
        node->node_addr = BLE_MESH_ADDR_UNASSIGNED;
        node->fixed = FALSE;
        APP_DBG("Delete node complete");
        goto unblock;
    }

    if(!node->cb->stage(node))
    {
        return FALSE;
    }

    node->fixed = TRUE;

unblock:

    node = node_block_get();
    if(node)
    {
        node->blocked = FALSE;
        return TRUE;
    }
    return FALSE;
}

/*********************************************************************
 * @fn      node_init
 *
 * @brief   node 
 *
 * @return  none
 */
static void node_init(void)
{
    int i;

    for(i = 0; i < ARRAY_SIZE(app_nodes); i++)
    {
        app_nodes[i].stage.node = NODE_INIT;
        app_nodes[i].node_addr = BLE_MESH_ADDR_UNASSIGNED;
    }
}

/*********************************************************************
 * @fn      free_node_get
 *
 * @brief   node
 *
 * @return  node_t / NULL
 */
static node_t *free_node_get(void)
{
    for(int i = 0; i < ARRAY_SIZE(app_nodes); i++)
    {
        if(app_nodes[i].node_addr == BLE_MESH_ADDR_UNASSIGNED)
        {
            return &app_nodes[i];
        }
    }
    return NULL;
}

/*********************************************************************
 * @fn      node_get
 *
 * @brief   node
 *
 * @param   node_addr   - node
 *
 * @return  node_t / NULL
 */
static node_t *node_get(uint16_t node_addr)
{
    for(int i = 0; i < ARRAY_SIZE(app_nodes); i++)
    {
        if(app_nodes[i].node_addr == node_addr)
        {
            return &app_nodes[i];
        }
    }
    return NULL;
}

/*********************************************************************
 * @fn      node_should_blocked
 *
 * @brief   node
 *
 * @param   node_addr   - node
 *
 * @return  TRUE    node
 *          FALSE   node
 */
static BOOL node_should_blocked(uint16_t node_addr)
{
    for(int i = 0; i < ARRAY_SIZE(app_nodes); i++)
    {
        if(app_nodes[i].node_addr != BLE_MESH_ADDR_UNASSIGNED &&
           app_nodes[i].node_addr != node_addr &&
           app_nodes[i].fixed != TRUE &&
           app_nodes[i].blocked == FALSE)
        {
            return TRUE;
        }
    }
    return FALSE;
}

/*********************************************************************
 * @fn      node_cfg_process
 *
 * @brief   
 *
 * @param   node        - 
 * @param   net_idx     - key
 * @param   addr        - 
 * @param   num_elem    - 
 *
 * @return  node_t / NULL
 */
static node_t *node_cfg_process(node_t *node, uint16_t net_idx, uint16_t addr, uint8_t num_elem)
{
    if(!node)
    {
        node = free_node_get();
        if(!node)
        {
            APP_DBG("No Free Node Object Available");
            return NULL;
        }
        node->net_idx = net_idx;
        node->node_addr = addr;
        node->elem_count = num_elem;
    }

    node->blocked = node_should_blocked(addr);

    if(!node->blocked)
    {
        tmos_start_task(App_TaskID, APP_NODE_EVT, 1600);
    }
    return node;
}

/*********************************************************************
 * @fn      node_stage_set
 *
 * @brief   node
 *
 * @param   node        - 
 * @param   new_stage   - 
 *
 * @return  none
 */
static void node_stage_set(node_t *node, node_stage_t new_stage)
{
    node->retry_cnt = 5;
    node->stage.node = new_stage;
}

/*********************************************************************
 * @fn      local_stage_set
 *
 * @brief   node
 *
 * @param   node        - 
 * @param   new_stage   - 
 *
 * @return  none
 */
static void local_stage_set(node_t *node, local_stage_t new_stage)
{
    node->retry_cnt = 1;
    node->stage.local = new_stage;
}

/*********************************************************************
 * @fn      node_rsp
 *
 * @brief   
 *
 * @param   p1      - node
 * @param   p2      - 
 *
 * @return  none
 */
static void node_rsp(void *p1, const void *p2)
{
    node_t                 *node = p1;
    const cfg_cli_status_t *val = p2;

    switch(val->cfgHdr.opcode)
    {
        case OP_APP_KEY_ADD:
            APP_DBG("node Application Key Added");
            node_stage_set(node, NODE_MOD_BIND_SET);
            break;
        case OP_MOD_APP_BIND:
            APP_DBG("node vendor Model Binded");
            node_stage_set(node, NODE_MOD_SUB_SET);
            break;
        case OP_MOD_SUB_ADD:
            APP_DBG("node vendor Model Subscription Set");
            node_stage_set(node, NODE_CONFIGURATIONED);
            break;
        default:
            APP_DBG("Unknown Opcode (0x%04x)", val->cfgHdr.opcode);
            return;
    }
}

/*********************************************************************
 * @fn      local_rsp
 *
 * @brief   
 *
 * @param   p1      - node
 * @param   p2      - 
 *
 * @return  none
 */
static void local_rsp(void *p1, const void *p2)
{
    node_t                 *node = p1;
    const cfg_cli_status_t *val = p2;

    switch(val->cfgHdr.opcode)
    {
        case OP_APP_KEY_ADD:
            APP_DBG("local Application Key Added");
            local_stage_set(node, LOCAL_MOD_BIND_SET);
            break;
        case OP_MOD_APP_BIND:
            APP_DBG("local vendor Model Binded");
            local_stage_set(node, LOCAL_CONFIGURATIONED);
            break;
        default:
            APP_DBG("Unknown Opcode (0x%04x)", val->cfgHdr.opcode);
            return;
    }
}

/*********************************************************************
 * @fn      node_stage
 *
 * @brief   app keyapp key
 *
 * @param   p1      - node
 *
 * @return  TRUE    
 *          FALSE   
 */
static BOOL node_stage(void *p1)
{
    int     err;
    BOOL    ret = FALSE;
    node_t *node = p1;

    switch(node->stage.node)
    {
        case NODE_APPKEY_ADD:
            err = bt_mesh_cfg_app_key_add(node->net_idx, node->node_addr, self_prov_net_idx, self_prov_app_idx, self_prov_app_key);
            if(err)
            {
                APP_DBG("Unable to adding Application key (err %d)", err);
                ret = TRUE;
            }
            break;

        case NODE_MOD_BIND_SET:
            err = bt_mesh_cfg_mod_app_bind_vnd(node->net_idx, node->node_addr, node->node_addr, self_prov_app_idx, BLE_MESH_MODEL_ID_WCH_SRV, CID_WCH);
            if(err)
            {
                APP_DBG("Unable to Binding vendor Model (err %d)", err);
                ret = TRUE;
            }
            break;

            // 
        case NODE_MOD_SUB_SET:
            err = bt_mesh_cfg_mod_sub_add_vnd(node->net_idx, node->node_addr, node->node_addr, vendor_sub_addr, BLE_MESH_MODEL_ID_WCH_SRV, CID_WCH);
            if(err)
            {
                APP_DBG("Unable to Set vendor Model Subscription (err %d)", err);
                ret = TRUE;
            }
            break;

        default:
            ret = TRUE;
            break;
    }

    return ret;
}

/*********************************************************************
 * @fn      local_stage
 *
 * @brief   app keyapp key
 *
 * @param   p1      - node
 *
 * @return  TRUE    
 *          FALSE   
 */
static BOOL local_stage(void *p1)
{
    int     err;
    BOOL    ret = FALSE;
    node_t *node = p1;

    switch(node->stage.local)
    {
        case LOCAL_APPKEY_ADD:
            err = bt_mesh_cfg_app_key_add(node->net_idx, node->node_addr, self_prov_net_idx, self_prov_app_idx, self_prov_app_key);
            if(err)
            {
                APP_DBG("Unable to adding Application key (err %d)", err);
                ret = 1;
            }
            break;

        case LOCAL_MOD_BIND_SET:
            err = bt_mesh_cfg_mod_app_bind_vnd(node->net_idx, node->node_addr, node->node_addr, self_prov_app_idx, BLE_MESH_MODEL_ID_WCH_CLI, CID_WCH);
            if(err)
            {
                APP_DBG("Unable to Binding vendor Model (err %d)", err);
                ret = 1;
            }
            break;

        default:
            ret = 1;
            break;
    }

    return ret;
}

static const cfg_cb_t node_cfg_cb = {
    node_rsp,
    node_stage,
};

static const cfg_cb_t local_cfg_cb = {
    local_rsp,
    local_stage,
};

/*********************************************************************
 * @fn      prov_complete
 *
 * @brief   
 *
 * @param   net_idx     - keyindex
 * @param   addr        - link
 * @param   flags       - key refresh
 * @param   iv_index    - ivindex
 *
 * @return  none
 */
static void prov_complete(uint16_t net_idx, uint16_t addr, uint8_t flags, uint32_t iv_index)
{
    int     err;
    node_t *node;

    APP_DBG("");

    err = bt_mesh_provisioner_enable(BLE_MESH_PROV_ADV);
    if(err)
    {
        APP_DBG("Unabled Enable Provisoner (err:%d)", err);
    }

    node = node_get(addr);
    if(!node || !node->fixed)
    {
        node = node_cfg_process(node, net_idx, addr, ARRAY_SIZE(elements));
        if(!node)
        {
            APP_DBG("Unable allocate node object");
            return;
        }

        node->cb = &local_cfg_cb;
        // 
        if( settings_load_over )
        {
            local_stage_set(node, LOCAL_APPKEY_ADD);
        }
        else
        {
            local_stage_set(node, LOCAL_CONFIGURATIONED);
        }
    }
}

/*********************************************************************
 * @fn      unprov_recv
 *
 * @brief   
 *
 * @param   bearer      - PB_ADV/PB_GATT
 * @param   uuid        - UUID
 * @param   oob_info    - 
 * @param   info        - 
 *
 * @return  none
 */
static void unprov_recv(bt_mesh_prov_bearer_t bearer,
                        const uint8_t uuid[16], bt_mesh_prov_oob_info_t oob_info,
                        const unprivison_info_t *info)
{
    APP_DBG("");
    int err;

    if(bearer & BLE_MESH_PROV_ADV)
    {
        err = bt_mesh_provision_adv(uuid, self_prov_net_idx, BLE_MESH_ADDR_UNASSIGNED, 5);
        if(err)
        {
            APP_DBG("Unable Open PB-ADV Session (err:%d)", err);
        }
    }
}

/*********************************************************************
 * @fn      node_added
 *
 * @brief   
 *
 * @param   net_idx     - key
 * @param   addr        - 
 * @param   num_elem    - 
 *
 * @return  none
 */
static void node_added(uint16_t net_idx, uint16_t addr, uint8_t num_elem)
{
    node_t *node;
    APP_DBG("");

    node = node_get(addr);
    if(!node || !node->fixed)
    {
        node = node_cfg_process(node, net_idx, addr, num_elem);
        if(!node)
        {
            APP_DBG("Unable allocate node object");
            return;
        }

        node->cb = &node_cfg_cb;
        // 
        if( settings_load_over )
        {
            node_stage_set(node, NODE_APPKEY_ADD);
        }
        else
        {
            node_stage_set(node, NODE_CONFIGURATIONED);
        }
    }
}

/*********************************************************************
 * @fn      cfg_cli_rsp_handler
 *
 * @brief   cfg
 *          1
 *
 * @param   val     - 
 *
 * @return  none
 */
static void cfg_cli_rsp_handler(const cfg_cli_status_t *val)
{
    node_t *node;
    APP_DBG("");

    // ,
    if(val->cfgHdr.opcode == OP_NODE_RESET)
    {
        if(reset_node_addr != BLE_MESH_ADDR_UNASSIGNED)
        {
            bt_mesh_node_del_by_addr(reset_node_addr);
            node = node_get(reset_node_addr);
            node->stage.node = NODE_INIT;
            node->node_addr = BLE_MESH_ADDR_UNASSIGNED;
            node->fixed = FALSE;
            APP_DBG("node reset complete");
        }
        return;
    }

    node = node_unblock_get();
    if(!node)
    {
        APP_DBG("Unable find Unblocked Node");
        return;
    }

    if(val->cfgHdr.status == 0xFF)
    {
        APP_DBG("Opcode 0x%04x, timeout", val->cfgHdr.opcode);
        goto end;
    }

    node->cb->rsp(node, val);

end:
    tmos_start_task(App_TaskID, APP_NODE_EVT, K_SECONDS(1));
}

/*********************************************************************
 * @fn      vendor_model_cli_rsp_handler
 *
 * @brief   
 *
 * @param   val     - 
 *
 * @return  none
 */
static void vendor_model_cli_rsp_handler(const vendor_model_cli_status_t *val)
{
    if(val->vendor_model_cli_Hdr.status)
    {
        //  
        APP_DBG("Timeout opcode 0x%02x", val->vendor_model_cli_Hdr.opcode);
        return;
    }
    if(val->vendor_model_cli_Hdr.opcode == OP_VENDOR_MESSAGE_TRANSPARENT_MSG)
    {
        // 
        APP_DBG("trans len %d, data 0x%02x from 0x%04x", val->vendor_model_cli_Event.trans.len,
                val->vendor_model_cli_Event.trans.pdata[0],
                val->vendor_model_cli_Event.trans.addr);
        tmos_memcpy(&app_mesh_manage, val->vendor_model_cli_Event.trans.pdata, val->vendor_model_cli_Event.trans.len);
        switch(app_mesh_manage.data.buf[0])
        {
            // 
            case CMD_DELETE_NODE_ACK:
            {
                if(val->vendor_model_cli_Event.trans.len != DELETE_NODE_ACK_DATA_LEN)
                {
                    APP_DBG("Delete node ack data err!");
                    return;
                }
                node_t *node;
                tmos_stop_task(App_TaskID, APP_DELETE_NODE_TIMEOUT_EVT);
                bt_mesh_node_del_by_addr(val->vendor_model_cli_Event.trans.addr);
                node = node_get(val->vendor_model_cli_Event.trans.addr);
                node->stage.node = NODE_INIT;
                node->node_addr = BLE_MESH_ADDR_UNASSIGNED;
                node->fixed = FALSE;
                APP_DBG("Delete node complete");
                break;
            }
        }
    }
    else if(val->vendor_model_cli_Hdr.opcode == OP_VENDOR_MESSAGE_TRANSPARENT_IND)
    {
        // indicate
        APP_DBG("ind len %d, data 0x%02x from 0x%04x", val->vendor_model_cli_Event.ind.len,
                val->vendor_model_cli_Event.ind.pdata[0],
                val->vendor_model_cli_Event.ind.addr);
    }
    else if(val->vendor_model_cli_Hdr.opcode == OP_VENDOR_MESSAGE_TRANSPARENT_WRT)
    {
        // write
    }
    else
    {
        APP_DBG("Unknow opcode 0x%02x", val->vendor_model_cli_Hdr.opcode);
    }
}

/*********************************************************************
 * @fn      vendor_model_cli_send
 *
 * @brief   
 *
 * @param   addr    - 
 *          pData   - 
 *          len     - 
 *
 * @return  Global_Error_Code
 */
static int vendor_model_cli_send(uint16_t addr, uint8_t *pData, uint16_t len)
{
    struct send_param param = {
        .app_idx = self_prov_app_idx,     // app key
        .addr = addr,                     // 1
        .trans_cnt = 0x01,                // 
        .period = K_MSEC(400),            // (200+50*TTL)ms
        .rand = (0),                      // 
        .tid = vendor_cli_tid_get(),      // tidcli0~127
        .send_ttl = BLE_MESH_TTL_DEFAULT, // ttl
    };
//    return vendor_message_cli_write(&param, pData, len);  // 2s
    return vendor_message_cli_send_trans(&param, pData, len); // 
}

/*********************************************************************
 * @fn      keyPress
 *
 * @brief   
 *
 * @param   keys    - 
 *
 * @return  none
 */
void keyPress(uint8_t keys)
{
    APP_DBG("%d", keys);

    switch(keys)
    {
        default:
        {
            if(0)
            {
                // 
                if(app_nodes[1].node_addr)
                {
                    uint8_t status;
                    APP_DBG("node1_addr %x", app_nodes[1].node_addr);
                    uint8_t data[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14, 15, 16};
                    status = vendor_model_cli_send(app_nodes[1].node_addr, data, 16);
                    if(status)
                    {
                        APP_DBG("write failed %d", status);
                    }
                }
            }
            if(1)
            {
                // 
                if(app_nodes[1].node_addr)
                {
                    uint8_t status;
                    APP_DBG("node1_addr %x", app_nodes[1].node_addr);
                    if(0)
                    {
                        // 
                        status = bt_mesh_cfg_node_reset(self_prov_net_idx, app_nodes[1].node_addr);
                        if(status)
                        {
                            APP_DBG("reset failed %d", status);
                        }
                        else
                        {
                            reset_node_addr = app_nodes[1].node_addr;
                        }
                    }
                    if(1)
                    {
                        // 
                        app_mesh_manage.delete_node.cmd = CMD_DELETE_NODE;
                        app_mesh_manage.delete_node.addr[0] = app_nodes[1].node_addr&0xFF;
                        app_mesh_manage.delete_node.addr[1] = (app_nodes[1].node_addr>>8)&0xFF;
                        status = vendor_model_cli_send(app_nodes[1].node_addr, app_mesh_manage.data.buf, DELETE_NODE_DATA_LEN);
                        if(status)
                        {
                            APP_DBG("delete failed %d", status);
                        }
                        else
                        {
                            // 
                            tmos_start_task(App_TaskID, APP_DELETE_NODE_TIMEOUT_EVT, 4800);
                        }
                    }
                }
            }
            break;
        }
    }
}

/*********************************************************************
 * @fn      blemesh_on_sync
 *
 * @brief   mesh
 *
 * @param   none
 *
 * @return  none
 */
void blemesh_on_sync(void)
{
    int        err;
    mem_info_t info;

    if(tmos_memcmp(VER_MESH_LIB, VER_MESH_FILE, strlen(VER_MESH_FILE)) == FALSE)
    {
        PRINT("head file error...\n");
        while(1);
    }

    info.base_addr = MESH_MEM;
    info.mem_len = ARRAY_SIZE(MESH_MEM);
#if(CONFIG_BLE_MESH_FRIEND)
    friend_init_register(bt_mesh_friend_init);
#endif /* FRIEND */
#if(CONFIG_BLE_MESH_LOW_POWER)
    lpn_init_register(bt_mesh_lpn_init);
#endif /* LPN */
    GetMACAddress(MACAddr);
    tmos_memcpy(dev_uuid, MACAddr, 6);
    err = bt_mesh_cfg_set(&app_mesh_cfg, &app_dev, MACAddr, &info);
    if(err)
    {
        APP_DBG("Unable set configuration (err:%d)", err);
        return;
    }
    hal_rf_init();
    err = bt_mesh_comp_register(&app_comp);
    node_init();

#if(CONFIG_BLE_MESH_RELAY)
    bt_mesh_relay_init();
#endif /* RELAY  */
#if(CONFIG_BLE_MESH_PROXY || CONFIG_BLE_MESH_PB_GATT)
  #if(CONFIG_BLE_MESH_PROXY)
    bt_mesh_proxy_beacon_init_register((void *)bt_mesh_proxy_beacon_init);
    gatts_notify_register(bt_mesh_gatts_notify);
    proxy_gatt_enable_register(bt_mesh_proxy_gatt_enable);
  #endif /* PROXY  */
  #if(CONFIG_BLE_MESH_PB_GATT)
    proxy_prov_enable_register(bt_mesh_proxy_prov_enable);
  #endif /* PB_GATT  */
    bt_mesh_proxy_init();
#endif /* PROXY || PB-GATT */

#if(CONFIG_BLE_MESH_PROXY_CLI)
    bt_mesh_proxy_client_init(cli); //
#endif                              /* PROXY_CLI */

    bt_mesh_prov_retransmit_init();

#if(!CONFIG_BLE_MESH_PB_GATT)
    adv_link_rx_buf_register(&rx_buf);
#endif /* !PB_GATT */
    err = bt_mesh_prov_init(&app_prov);

    bt_mesh_mod_init();
    bt_mesh_net_init();
    bt_mesh_trans_init();
    bt_mesh_beacon_init();
    bt_mesh_adv_init();

#if((CONFIG_BLE_MESH_PB_GATT) || (CONFIG_BLE_MESH_PROXY) || (CONFIG_BLE_MESH_OTA))
    bt_mesh_conn_adv_init();
#endif /* PROXY || PB-GATT || OTA */

#if(CONFIG_BLE_MESH_SETTINGS)
    bt_mesh_settings_init();
#endif /* SETTINGS */

#if(CONFIG_BLE_MESH_PROXY_CLI)
    bt_mesh_proxy_cli_adapt_init();
#endif /* PROXY_CLI */

#if((CONFIG_BLE_MESH_PROXY) || (CONFIG_BLE_MESH_PB_GATT) || \
    (CONFIG_BLE_MESH_PROXY_CLI) || (CONFIG_BLE_MESH_OTA))
    bt_mesh_adapt_init();
#endif /* PROXY || PB-GATT || PROXY_CLI || OTA */

#if(CONFIG_BLE_MESH_LOW_POWER)
    bt_mesh_lpn_set(TRUE);
#endif /* LPN */

    if(err)
    {
        APP_DBG("Initializing mesh failed (err %d)", err);
        return;
    }
    APP_DBG("Bluetooth initialized");

#if(CONFIG_BLE_MESH_SETTINGS)
    settings_load();
    settings_load_over = TRUE;
#endif /* SETTINGS */

    if(bt_mesh_is_provisioned())
    {
        APP_DBG("Mesh network restored from flash");
    }
    else
    {
        err = bt_mesh_provision(self_prov_net_key, self_prov_net_idx, self_prov_flags,
                                self_prov_iv_index, self_prov_addr, self_prov_dev_key);
        if(err)
        {
            APP_DBG("Self Privisioning (err %d)", err);
            return;
        }
    }
    APP_DBG("Mesh initialized");
}

/*********************************************************************
 * @fn      App_Init
 *
 * @brief   
 *
 * @return  none
 */
void App_Init(void)
{
    App_TaskID = TMOS_ProcessEventRegister(App_ProcessEvent);

    vendor_model_cli_init(vnd_models);
    blemesh_on_sync();

    HAL_KeyInit();
    HalKeyConfig(keyPress);

    // 
    tmos_start_task(App_TaskID, APP_NODE_TEST_EVT, 4800);
}

/*********************************************************************
 * @fn      App_ProcessEvent
 *
 * @brief   
 *
 * @param   task_id  - The TMOS assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed
 */
static uint16_t App_ProcessEvent(uint8_t task_id, uint16_t events)
{
    // 
    if(events & APP_NODE_EVT)
    {
        if(node_work_handler())
            return (events);
        else
            return (events ^ APP_NODE_EVT);
    }

    // 
    if(events & APP_NODE_TEST_EVT)
    {
        if(app_nodes[1].node_addr)
        {
            uint8_t status;
            APP_DBG("app_nodes[1] ADDR %x", app_nodes[1].node_addr);
            uint8_t data[4] = {0, 1, 2, 3};
            status = vendor_model_cli_send(app_nodes[1].node_addr, data, 4); // 
            if(status)
                APP_DBG("trans failed %d", status);
        }
        tmos_start_task(App_TaskID, APP_NODE_TEST_EVT, 6400);
        return (events ^ APP_NODE_TEST_EVT);
    }

    if(events & APP_DELETE_NODE_TIMEOUT_EVT)
    {
        // 
        APP_DBG("Delete node failed ");
        return (events ^ APP_DELETE_NODE_TIMEOUT_EVT);
    }

    // Discard unknown events
    return 0;
}

/******************************** endfile @ main ******************************/
