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
#include "app.h"
#include "HAL.h"

/*********************************************************************
 * GLOBAL TYPEDEFS
 */
#define ADV_TIMEOUT       K_MINUTES(10)

#define SELENCE_ADV_ON    0x01
#define SELENCE_ADV_OF    0x00

#define APP_WAIT_ADD_APPKEY_DELAY     1600*10

#define APP_DELETE_LOCAL_NODE_DELAY   3200
// shall not less than APP_DELETE_LOCAL_NODE_DELAY
#define APP_DELETE_NODE_INFO_DELAY    3200
/*********************************************************************
 * GLOBAL TYPEDEFS
 */

static uint8_t MESH_MEM[1024 * 2] = {0};

extern const ble_mesh_cfg_t app_mesh_cfg;
extern const struct device  app_dev;

static uint8_t App_TaskID = 0; // Task ID for internal task/event processing

static uint16_t App_ProcessEvent(uint8_t task_id, uint16_t events);

static uint8_t dev_uuid[16] = {0}; // UUID
uint8_t        MACAddr[6];         // mac

#if(!CONFIG_BLE_MESH_PB_GATT)
NET_BUF_SIMPLE_DEFINE_STATIC(rx_buf, 65);
#endif /* !PB_GATT */

/*********************************************************************
 * LOCAL FUNCION
 */

static void cfg_srv_rsp_handler( const cfg_srv_status_t *val );
static void link_open(bt_mesh_prov_bearer_t bearer);
static void link_close(bt_mesh_prov_bearer_t bearer, uint8_t reason);
static void prov_complete(uint16_t net_idx, uint16_t addr, uint8_t flags, uint32_t iv_index);
static void vendor_model_srv_rsp_handler(const vendor_model_srv_status_t *val);
static int  vendor_model_srv_send(uint16_t addr, uint8_t *pData, uint16_t len);
static void prov_reset(void);

static struct bt_mesh_cfg_srv cfg_srv = {
    .relay = BLE_MESH_RELAY_DISABLED,
    .beacon = BLE_MESH_BEACON_DISABLED,
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
    .handler = cfg_srv_rsp_handler,
};

/* Attention on */
void app_prov_attn_on(struct bt_mesh_model *model)
{
    APP_DBG("app_prov_attn_on");
}

/* Attention off */
void app_prov_attn_off(struct bt_mesh_model *model)
{
    APP_DBG("app_prov_attn_off");
}

const struct bt_mesh_health_srv_cb health_srv_cb = {
    .attn_on = app_prov_attn_on,
    .attn_off = app_prov_attn_off,
};

static struct bt_mesh_health_srv health_srv = {
    .cb = &health_srv_cb,
};

BLE_MESH_HEALTH_PUB_DEFINE(health_pub, 8);

uint16_t cfg_srv_keys[CONFIG_MESH_MOD_KEY_COUNT_DEF] = {BLE_MESH_KEY_UNUSED};
uint16_t cfg_srv_groups[CONFIG_MESH_MOD_GROUP_COUNT_DEF] = {BLE_MESH_ADDR_UNASSIGNED};

uint16_t health_srv_keys[CONFIG_MESH_MOD_KEY_COUNT_DEF] = {BLE_MESH_KEY_UNUSED};
uint16_t health_srv_groups[CONFIG_MESH_MOD_GROUP_COUNT_DEF] = {BLE_MESH_ADDR_UNASSIGNED};

// root
static struct bt_mesh_model root_models[] = {
    BLE_MESH_MODEL_CFG_SRV(cfg_srv_keys, cfg_srv_groups, &cfg_srv),
    BLE_MESH_MODEL_HEALTH_SRV(health_srv_keys, health_srv_groups, &health_srv, &health_pub),
};

struct bt_mesh_vendor_model_srv vendor_model_srv = {
    .srv_tid.trans_tid = 0xFF,
    .handler = vendor_model_srv_rsp_handler,
};

uint16_t vnd_model_srv_keys[CONFIG_MESH_MOD_KEY_COUNT_DEF] = {BLE_MESH_KEY_UNUSED};
uint16_t vnd_model_srv_groups[CONFIG_MESH_MOD_GROUP_COUNT_DEF] = {BLE_MESH_ADDR_UNASSIGNED};

// 
struct bt_mesh_model vnd_models[] = {
    BLE_MESH_MODEL_VND_CB(CID_WCH, BLE_MESH_MODEL_ID_WCH_SRV, vnd_model_srv_op, NULL, vnd_model_srv_keys,
                          vnd_model_srv_groups, &vendor_model_srv, NULL),
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
    .reset = prov_reset,
};

app_mesh_manage_t app_mesh_manage;

uint16_t delete_node_info_address=0;
uint8_t settings_load_over = FALSE;

/*********************************************************************
 * GLOBAL TYPEDEFS
 */

/*********************************************************************
 * @fn      prov_enable
 *
 * @brief   
 *
 * @return  none
 */
static void prov_enable(void)
{
    if(bt_mesh_is_provisioned())
    {
        return;
    }

    // Make sure we're scanning for provisioning inviations
    bt_mesh_scan_enable();
    // Enable unprovisioned beacon sending
    bt_mesh_beacon_enable();

    if(CONFIG_BLE_MESH_PB_GATT)
    {
        bt_mesh_proxy_prov_enable();
    }
}

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
    APP_DBG("");
    if(reason != CLOSE_REASON_SUCCESS)
        APP_DBG("reason %x", reason);

    // LPNappkey
    tmos_start_task(App_TaskID, APP_LPN_ENABLE_EVT, 3200);
}

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
    APP_DBG("net_idx %x, addr %x", net_idx, addr);
    if(settings_load_over || (vnd_models[0].keys[0]==BLE_MESH_KEY_UNUSED))
    {
        tmos_start_task(App_TaskID, APP_DELETE_LOCAL_NODE_EVT, APP_WAIT_ADD_APPKEY_DELAY);
    }
}

/*********************************************************************
 * @fn      prov_reset
 *
 * @brief   
 *
 * @param   none
 *
 * @return  none
 */
static void prov_reset(void)
{
    APP_DBG("");

    prov_enable();
}

/*********************************************************************
 * @fn      cfg_srv_rsp_handler
 *
 * @brief   config 
 *
 * @param   val     - 
 *
 * @return  none
 */
static void cfg_srv_rsp_handler( const cfg_srv_status_t *val )
{
    if(val->cfgHdr.status)
    {
        // 
        APP_DBG("warning opcode 0x%02x", val->cfgHdr.opcode);
        return;
    }
    if(val->cfgHdr.opcode == OP_APP_KEY_ADD)
    {
        APP_DBG("App Key Added");
        // 
        tmos_start_task(App_TaskID, APP_DELETE_LOCAL_NODE_EVT, APP_WAIT_ADD_APPKEY_DELAY);
    }
    else if(val->cfgHdr.opcode == OP_MOD_APP_BIND)
    {
        APP_DBG("Vendor Model Binded");
        // 
        tmos_start_task(App_TaskID, APP_DELETE_LOCAL_NODE_EVT, APP_WAIT_ADD_APPKEY_DELAY);
    }
    else if(val->cfgHdr.opcode == OP_MOD_SUB_ADD)
    {
        APP_DBG("Vendor Model Subscription Set");
        // 
        tmos_stop_task(App_TaskID, APP_DELETE_LOCAL_NODE_EVT);
    }
    else
    {
        APP_DBG("Unknow opcode 0x%02x", val->cfgHdr.opcode);
    }
}

/*********************************************************************
 * @fn      lpn_state
 *
 * @brief   
 *
 * @param   frd_addr    - 
 *          state       - 
 *
 * @return  none
 */
static void lpn_state(uint16_t frd_addr, uint8_t state)
{
    if(state == LPN_FRIENDSHIP_ESTABLISHED)
    {
        APP_DBG("lpn friendship established %d",frd_addr);
    }
    else if(state == LPN_FRIENDSHIP_TERMINATED)
    {
        APP_DBG("lpn friendship terminated");
    }
    else
    {
        APP_DBG("unknow state %x", state);
    }
}

/*********************************************************************
 * @fn      vendor_model_srv_rsp_handler
 *
 * @brief   
 *
 * @param   val     - 
 *
 * @return  none
 */
static void vendor_model_srv_rsp_handler(const vendor_model_srv_status_t *val)
{
    if(val->vendor_model_srv_Hdr.status)
    {
        //  
        APP_DBG("Timeout opcode 0x%02x", val->vendor_model_srv_Hdr.opcode);
        return;
    }
    if(val->vendor_model_srv_Hdr.opcode == OP_VENDOR_MESSAGE_TRANSPARENT_MSG)
    {
        // 
        APP_DBG("len %d, data 0x%02x from 0x%04x", val->vendor_model_srv_Event.trans.len,
                val->vendor_model_srv_Event.trans.pdata[0],
                val->vendor_model_srv_Event.trans.addr);
        tmos_memcpy(&app_mesh_manage, val->vendor_model_srv_Event.trans.pdata, val->vendor_model_srv_Event.trans.len);
        switch(app_mesh_manage.data.buf[0])
        {
            // 
            case CMD_DELETE_NODE:
            {
                if(val->vendor_model_srv_Event.trans.len != DELETE_NODE_DATA_LEN)
                {
                    APP_DBG("Delete node data err!");
                    return;
                }
                uint8_t status;
                APP_DBG("receive delete cmd, send ack and start delete node delay");
                app_mesh_manage.delete_node_ack.cmd = CMD_DELETE_NODE_ACK;
                status = vendor_model_srv_send(val->vendor_model_srv_Event.trans.addr,
                                                app_mesh_manage.data.buf, DELETE_NODE_ACK_DATA_LEN);
                if(status)
                {
                    APP_DBG("send ack failed %d", status);
                }
                // CMD_DELETE_NODE_INFO
                APP_DBG("send to all node to let them delete stored info ");
                app_mesh_manage.delete_node_info.cmd = CMD_DELETE_NODE_INFO;
                status = vendor_model_srv_send(BLE_MESH_ADDR_ALL_NODES,
                                                app_mesh_manage.data.buf, DELETE_NODE_INFO_DATA_LEN);
                if(status)
                {
                    APP_DBG("send ack failed %d", status);
                }
                tmos_start_task(App_TaskID, APP_DELETE_LOCAL_NODE_EVT, APP_DELETE_LOCAL_NODE_DELAY);
                break;
            }

            // 
            case CMD_DELETE_NODE_INFO:
            {
                if(val->vendor_model_srv_Event.trans.len != DELETE_NODE_INFO_DATA_LEN)
                {
                    APP_DBG("Delete node info data err!");
                    return;
                }
                delete_node_info_address = val->vendor_model_srv_Event.trans.addr;
                tmos_start_task(App_TaskID, APP_DELETE_NODE_INFO_EVT, APP_DELETE_NODE_INFO_DELAY);
                break;
            }
        }
    }
    else if(val->vendor_model_srv_Hdr.opcode == OP_VENDOR_MESSAGE_TRANSPARENT_WRT)
    {
        // write
        APP_DBG("len %d, data 0x%02x from 0x%04x", val->vendor_model_srv_Event.write.len,
                val->vendor_model_srv_Event.write.pdata[0],
                val->vendor_model_srv_Event.write.addr);
    }
    else if(val->vendor_model_srv_Hdr.opcode == OP_VENDOR_MESSAGE_TRANSPARENT_IND)
    {
        // indicate
    }
    else
    {
        APP_DBG("Unknow opcode 0x%02x", val->vendor_model_srv_Hdr.opcode);
    }
}

/*********************************************************************
 * @fn      vendor_model_srv_send
 *
 * @brief   
 *
 * @param   addr    - 
 *          pData   - 
 *          len     - 
 *
 * @return  Global_Error_Code
 */
static int vendor_model_srv_send(uint16_t addr, uint8_t *pData, uint16_t len)
{
    struct send_param param = {
        .app_idx = vnd_models[0].keys[0], // app key0key
        .addr = addr,          // 
        .trans_cnt = 0x01,                // 
        .period = K_MSEC(400),            // (200+50*TTL)ms
        .rand = (0),                      // 
        .tid = vendor_srv_tid_get(),      // tidsrv128~191
        .send_ttl = BLE_MESH_TTL_DEFAULT, // ttl
    };
//    return vendor_message_srv_indicate(&param, pData, len);  // 2s
    return vendor_message_srv_send_trans(&param, pData, len); // 
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
            int status;
            uint8_t data[8] = {0, 1, 2, 3, 4, 5, 6, 7};
            // 
            status = vendor_model_srv_send(0x0001, data, 8);
            if(status)
            {
                APP_DBG("send failed %d", status);
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
    friend_init_register(bt_mesh_friend_init, friend_state);
#endif /* FRIEND */
#if(CONFIG_BLE_MESH_LOW_POWER)
    lpn_init_register(bt_mesh_lpn_init, lpn_state);
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

        // flashLPN
        tmos_set_event(App_TaskID, APP_LPN_ENABLE_EVT);
    }
    else
    {
        prov_enable();
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
void App_Init()
{
    App_TaskID = TMOS_ProcessEventRegister(App_ProcessEvent);

    vendor_model_srv_init(vnd_models);
    blemesh_on_sync();
#if(HAL_KEY)
    HAL_KeyInit();
    HalKeyConfig(keyPress);
#endif /* HAL_Key */
//    tmos_start_task(App_TaskID, APP_NODE_TEST_EVT, 1600);
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
    if(events & APP_NODE_TEST_EVT)
    {
        tmos_start_task(App_TaskID, APP_NODE_TEST_EVT, 2400);
        return (events ^ APP_NODE_TEST_EVT);
    }
    if(events & APP_LPN_ENABLE_EVT)
    {
        if(bt_mesh_app_key_find(vnd_models[0].keys[0]) != NULL)
        {
            //  lpnfriend
#if(CONFIG_BLE_MESH_LOW_POWER)
            bt_mesh_lpn_set(TRUE);
            APP_DBG("Low power enable");
#endif /* LPN */
        }
        else
        {
            tmos_start_task(App_TaskID, APP_LPN_ENABLE_EVT, 1600);
        }
        return (events ^ APP_LPN_ENABLE_EVT);
    }

    if(events & APP_DELETE_LOCAL_NODE_EVT)
    {
        // 
        APP_DBG("Delete local node");
        // 
        bt_mesh_reset();
        return (events ^ APP_DELETE_LOCAL_NODE_EVT);
    }

    if(events & APP_DELETE_NODE_INFO_EVT)
    {
        // 
        bt_mesh_delete_node_info(delete_node_info_address,app_comp.elem_count);
        APP_DBG("Delete stored node info complete");
        return (events ^ APP_DELETE_NODE_INFO_EVT);
    }

    // Discard unknown events
    return 0;
}

/******************************** endfile @ main ******************************/
