/********************************** (C) COPYRIGHT *******************************
 * File Name          : app_vendor_model.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2021/03/24
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/******************************************************************************/
#include "app_mesh_config.h"
#include "MESH_LIB.h"
#include "CONFIG.h"
#include "app_vendor_model.h"
#include "app_generic_onoff_model.h"
#include "app_generic_lightness_model.h"
#include "app_generic_color_model.h"

/* ***************************************************************************
* GLOBAL TYPEDEFS
Triple address information */
/* Mac Address Light3   */
#define PID    10135680
#define MAC_ADDR                       \
  {                                    \
    0x50, 0x3d, 0xeb, 0x90, 0x35, 0x17 \
  }
#define ALI_SECRET                                                                                 \
  {                                                                                                \
    0xcb, 0xd1, 0xc1, 0x33, 0xa8, 0x28, 0xc7, 0xdc, 0x7e, 0xbd, 0xef, 0x6f, 0xbc, 0x41, 0x55, 0x0e \
  }

/*********************************************************************
 * GLOBAL TYPEDEFS
 */

uint8_t       tm_uuid[16];
uint8_t       static_key[16];
const uint8_t MacAddr[6] = MAC_ADDR;

static const struct bt_als_cfg cfg = {
    .mac = MAC_ADDR,
    .secret = ALI_SECRET,
    .pid = PID,
    .cid = 0x01a8,
    .version = 0x00060000,
};

static uint8_t als_vendor_model_TaskID = 0; // Task ID for internal task/event processing
static uint8_t als_tid = 0;

static struct net_buf          ind_buf[CONFIG_INDICATE_NUM] = {0};
static struct bt_mesh_indicate indicate[CONFIG_INDICATE_NUM] = {0};

/*********************************************************************
 * GLOBAL TYPEDEFS
 */
static void     ind_reset(struct bt_mesh_indicate *ind, int err);
static uint16_t als_vendor_model_ProcessEvent(uint8_t task_id, uint16_t events);

/* ***************************************************************************
* @fn tm_attr_get
*
* @brief Tmall get attribute command
*
* @param model - Model parameters.
* @param ctx - Data parameters.
* @param buf - Data content.
*
* @return none */
static void tm_attr_get(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx, struct net_buf_simple *buf)
{
    APP_DBG(" ");
}

/* ***************************************************************************
* @fn tm_attr_set
*
* @brief Tmall Setting Properties Command
*
* @param model - Model parameters.
* @param ctx - Data parameters.
* @param buf - Data content.
*
* @return none */
static void tm_attr_set(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx, struct net_buf_simple *buf)
{
    APP_DBG(" ");
}

/* ***************************************************************************
* @fn tm_attr_set_unack
*
* @brief Tmall Setting Attribute Command (No Answer)
*
* @param model - Model parameters.
* @param ctx - Data parameters.
* @param buf - Data content.
*
* @return none */
static void tm_attr_set_unack(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx, struct net_buf_simple *buf)
{
    APP_DBG(" ");
}

/* ***************************************************************************
* @fn tm_attr_status
*
* @brief Tmall status response
*
* @param model - Model parameters.
* @param ctx - Data parameters.
* @param buf - Data content.
*
* @return none */
static void tm_attr_status(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx, struct net_buf_simple *buf)
{
    APP_DBG(" ");
}

/* ***************************************************************************
* @fn tm_attr_confirm
*
* @brief received a confirm sent by Tmall elves - this message is used to reply to the Vendor Model Client to the Vendor Model Server,
Used to indicate that the indication has been received from the Vendor Model Server
*
* @param model - Model parameters.
* @param ctx - Data parameters.
* @param buf - Data content.
*
* @return none */
static void tm_attr_confirm(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx, struct net_buf_simple *buf)
{
    uint8_t recv_tid;

    recv_tid = net_buf_simple_pull_u8(buf);

    APP_DBG("src: 0x%04x dst: 0x%04x tid 0x%02x rssi: %d", ctx->addr, ctx->recv_dst, recv_tid, ctx->recv_rssi);

    for(int i = 0; i < CONFIG_INDICATE_NUM; i++)
    {
        if(indicate[i].param.tid == recv_tid)
        {
            ind_reset(&indicate[i], 0);
            tmos_stop_task(als_vendor_model_TaskID, indicate[i].event);
            continue;
        }
    }
}

/* ***************************************************************************
* @fn tm_attr_trans
*
* @brief This message is used to transmit data between the Mesh device and the Tmall Genie
*
* @param model - Model parameters.
* @param ctx - Data parameters.
* @param buf - Data content.
*
* @return none */
static void tm_attr_trans(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx, struct net_buf_simple *buf)
{
    APP_DBG(" ");
}

// Opcode corresponding processing function
static const struct bt_mesh_model_op vnd_model_op[] = {
    {OP_VENDOR_MESSAGE_ATTR_GET, 0, tm_attr_get},
    {OP_VENDOR_MESSAGE_ATTR_SET, 0, tm_attr_set},
    {OP_VENDOR_MESSAGE_ATTR_SET_UNACK, 0, tm_attr_set_unack},
    {OP_VENDOR_MESSAGE_ATTR_STATUS, 0, tm_attr_status},
    {OP_VENDOR_MESSAGE_ATTR_CONFIRMATION, 1, tm_attr_confirm},
    {OP_VENDOR_MESSAGE_ATTR_TRANSPARENT_MSG, 0, tm_attr_trans},
    BLE_MESH_MODEL_OP_END,
};

uint16_t vnd_model_srv_keys[CONFIG_MESH_MOD_KEY_COUNT_DEF] = {BLE_MESH_KEY_UNUSED};
uint16_t vnd_model_srv_groups[CONFIG_MESH_MOD_GROUP_COUNT_DEF] = {BLE_MESH_ADDR_UNASSIGNED};

struct bt_mesh_model vnd_models[] = {
    BLE_MESH_MODEL_VND_CB(CID_ALI_GENIE, 0x0000, vnd_model_op, NULL, vnd_model_srv_keys, vnd_model_srv_groups, NULL,
        NULL),
};

/*********************************************************************
 * @fn      als_avail_tid_get
 *
 * @brief   TID selection method
 *
 * @param   none
 *
 * @return  next TID
 */
uint8_t als_avail_tid_get(void)
{
    return als_tid++;
}

/* ***************************************************************************
* @fn uuid_generate
*
* @brief Generate the UUID of Alibaba specification
*
* @param cfg - Configuration information
*
* @return none */
static void uuid_generate(struct bt_als_cfg const *cfg)
{
    /* Company ID */
    tm_uuid[0] = cfg->cid;
    tm_uuid[1] = cfg->cid >> 8;

    /* bit30: Bluetooth broadcast package version number, currently 0x01
bit4 is 1: one machine and one secret
bit5 is 1: Support OTA
bit76: Bluetooth protocol version
00: BLE4.0
01: BLE4.2
10: BLE5.0
11: BLE5.0 or above */
    /* Advertising Verison */
    tm_uuid[2] = BIT(0) | /* adv version */
                 BIT(4) | /* secret */
                 BIT(5) | /* ota */
                 BIT(7);  /* ble verison */

    /* Product Id */
    tm_uuid[2 + 1] = cfg->pid;
    tm_uuid[2 + 2] = cfg->pid >> 8;
    tm_uuid[2 + 3] = cfg->pid >> 16;
    tm_uuid[2 + 4] = cfg->pid >> 24;

    /* Device Mac Address */
    for(int i = 0; i < 6; i++)
    {
        tm_uuid[2 + 1 + 4 + i] = cfg->mac[5 - i];
    }

    /* UUID Verison */
    tm_uuid[2 + 1 + 4 + 6] = BIT(1);

    /* RFU */
    tm_uuid[2 + 1 + 4 + 6 + 1] = 0x00;
    tm_uuid[2 + 1 + 4 + 6 + 2] = 0x00;
}

/* ***************************************************************************
* @fn num_to_str
*
* @brief number to character
*
* @param out - Output
* @param in - Input
* @param in_len - length
*
* @return none */
static void num_to_str(uint8_t *out, const uint8_t *in, uint16_t in_len)
{
    uint16_t          i;
    static const char hex[] = "0123456789abcdef";

    for(i = 0; i < in_len; i++)
    {
        out[i * 2] = hex[in[i] >> 4];
        out[i * 2 + 1] = hex[in[i] & 0xf];
    }
}

/* ***************************************************************************
* @fn oob_key_generate
*
* @brief Generate OOB key
*
* @param cfg - Configuration information
*
* @return none */
static void oob_key_generate(struct bt_als_cfg const *cfg)
{
    int                           err;
    uint32_t                      pid;
    uint8_t                       out[8 + 1 + 12 + 1 + 32], dig[32];
    struct tc_sha256_state_struct s;

    tc_sha256_init(&s);

    /** pid value */
    pid = ((uint32_t)((((cfg->pid) >> 24) & 0xff) | (((cfg->pid) >> 8) & 0xff00) | (((cfg->pid) & 0xff00) << 8) | (((cfg->pid) & 0xff) << 24)));
    num_to_str(out, (void *)&pid, 4);

    /** Separator */
    strcpy((void *)(out + 8), ",");
    /** mac value */
    num_to_str(out + 8 + 1, (void *)cfg->mac, 6);

    /** Separator */
    strcpy((void *)(out + 8 + 1 + 12), ",");
    /** secret value */
    num_to_str(out + 8 + 1 + 12 + 1, (void *)cfg->secret, 16);

    err = tc_sha256_update(&s, out, sizeof(out));
    if(err != TC_CRYPTO_SUCCESS)
    {
        APP_DBG("Unable Update Sha256");
        return;
    }

    err = tc_sha256_final(dig, &s);
    if(err != TC_CRYPTO_SUCCESS)
    {
        APP_DBG("Unable Generate sha256 value");
        return;
    }

    memcpy(static_key, dig, 16);
}

/* ***************************************************************************
* @fn ind_reset
*
* @brief Remove the list, call the send completion callback, and release the cache
*
* @param ind - Notifications that need to be reset
* @param err - Error code
*
* @return none */
static void ind_reset(struct bt_mesh_indicate *ind, int err)
{
    if(ind->param.cb && ind->param.cb->end)
    {
        ind->param.cb->end(err, ind->param.cb_data);
    }

    tmos_msg_deallocate(ind->buf->__buf);
    ind->buf->__buf = NULL;
}

/* ***************************************************************************
* @fn bt_mesh_indicate_reset
*
* @brief Release all unsent notifications
*
* @param none
*
* @return none */
void bt_mesh_indicate_reset(void)
{
    uint8_t i;
    for(i = 0; i < CONFIG_INDICATE_NUM; i++)
    {
        if(indicate[i].buf->__buf != NULL)
        {
            ind_reset(&indicate[i], -ECANCELED);
        }
    }
}

/* ***************************************************************************
* @fn ind_start
*
* @brief send indication start callback
*
* @param duration - How long will this send last
* @param err - Error code
* @param cb_data - callback parameters
*
* @return none */
static void ind_start(uint16_t duration, int err, void *cb_data)
{
    struct bt_mesh_indicate *ind = cb_data;

    if(ind->buf->__buf == NULL)
    {
        return;
    }

    if(err)
    {
        APP_DBG("Unable send indicate (err:%d)", err);
        tmos_start_task(als_vendor_model_TaskID, ind->event, K_MSEC(100));
        return;
    }
}

/* ***************************************************************************
* @fn ind_end
*
* @brief send indication end callback
*
* @param err - Error code
* @param cb_data - callback parameters
*
* @return none */
static void ind_end(int err, void *cb_data)
{
    struct bt_mesh_indicate *ind = cb_data;

    if(ind->buf->__buf == NULL)
    {
        return;
    }
    tmos_start_task(als_vendor_model_TaskID, ind->event, ind->param.period);
}

// Send indicated callback structure
const struct bt_mesh_send_cb ind_cb =
    {
        .start = ind_start,
        .end = ind_end,
};

/* ***************************************************************************
* @fn adv_ind_send
*
* @brief send indicated
*
* @param ind - Notifications to be sent
*
* @return none */
static void adv_ind_send(struct bt_mesh_indicate *ind)
{
    int err;
    NET_BUF_SIMPLE_DEFINE(msg, 32);

    struct bt_mesh_msg_ctx ctx = {
        .app_idx = vnd_models[0].keys[0],
        .addr = ALI_TM_SUB_ADDRESS,
    };

    if(ind->buf->__buf == NULL)
    {
        APP_DBG("NULL buf");
        return;
    }

    if(ind->param.trans_cnt == 0)
    {
        ind_reset(ind, -ETIMEDOUT);
        return;
    }

    ind->param.trans_cnt--;

    ctx.send_ttl = ind->param.send_ttl;

    net_buf_simple_add_mem(&msg, ind->buf->data, ind->buf->len);

    err = bt_mesh_model_send(vnd_models, &ctx, &msg, &ind_cb, ind);
    if(err)
    {
        APP_DBG("Unable send model message (err:%d)", err);
        ind_reset(ind, -EIO);
        return;
    }
}

/* ***************************************************************************
* @fn bt_mesh_ind_alloc
*
* @brief Find an empty indicator and allocate memory
*
* @param len - The length of data to be allocated
*
* @return indicated structure pointer */
struct bt_mesh_indicate *bt_mesh_ind_alloc(uint16_t len)
{
    uint8_t i;
    for(i = 0; i < CONFIG_INDICATE_NUM; i++)
    {
        if(indicate[i].buf->__buf == NULL)
            break;
    }
    if(i == CONFIG_INDICATE_NUM)
    {
        return NULL;
    }

    indicate[i].buf->__buf = tmos_msg_allocate(len);
    indicate[i].buf->size = len;

    if(indicate[i].buf->__buf == NULL)
    {
        return NULL;
    }

    return &indicate[i];
}

/* ***************************************************************************
* @fn bt_mesh_indicate_send
*
* @brief starts the event to send notifications
*
* @param ind - indicated structure pointer
*
* @return none */
void bt_mesh_indicate_send(struct bt_mesh_indicate *ind)
{
    tmos_start_task(als_vendor_model_TaskID, ind->event, ind->param.rand);
}

/* ***************************************************************************
* @fn send_led_indicate
*
* @brief sends the current LED switch status. This function needs to be called when the LED switch status is updated.
*
* @param param - Send parameters for sending notifications
*
* @return none */
void send_led_indicate(struct indicate_param *param)
{
    struct bt_mesh_indicate *ind;

    ind = bt_mesh_ind_alloc(16);
    if(!ind)
    {
        APP_DBG("Unable allocate buffers");
        return;
    }
    memcpy(&(ind->param), param, sizeof(struct indicate_param));

    /* Init indication opcode */
    bt_mesh_model_msg_init(&(ind->buf->b), OP_VENDOR_MESSAGE_ATTR_INDICATION);

    /* Add tid field */
    net_buf_simple_add_u8(&(ind->buf->b), param->tid);

    /* Add generic onoff attrbute op */
    net_buf_simple_add_le16(&(ind->buf->b), ALI_GEN_ATTR_TYPE_POWER_STATE);

    /* Add current generic onoff status */
    net_buf_simple_add_u8(&(ind->buf->b), read_led_state(MSG_PIN));

    bt_mesh_indicate_send(ind);
}

/* ***************************************************************************
* @fn send_lightness_indicate
*
* @brief sends the current LED brightness. This function needs to be called when there is an update to the LED brightness.
*
* @param param - Send parameters for sending notifications
*
* @return none */
void send_lightness_indicate(struct indicate_param *param)          // Addition
{
    struct bt_mesh_indicate *ind;

    ind = bt_mesh_ind_alloc(16);
    if(!ind)
    {
        APP_DBG("Unable allocate buffers");
        return;
    }
    memcpy(&(ind->param), param, sizeof(struct indicate_param));

    /* Init indication opcode */
    bt_mesh_model_msg_init(&(ind->buf->b), OP_VENDOR_MESSAGE_ATTR_INDICATION);

    /* Add tid field */
    net_buf_simple_add_u8(&(ind->buf->b), param->tid);

    /* Add brightness attrbute opcode */
    net_buf_simple_add_le16(&(ind->buf->b), ALI_GEN_ATTR_TYPE_BRIGHTNESS);

    /* Add brightness status (655~65535 corresponds to Tmall control 1~100) */
    net_buf_simple_add_le16(&(ind->buf->b), read_led_lightness(MSG_PIN));           // Addition (modify)

    bt_mesh_indicate_send(ind);
}

/* ***************************************************************************
* @fn send_lightness_indicate
*
* @brief Send the current LED color temperature. This function needs to be called when there is an update of the LED color temperature.
*
* @param param - Send parameters for sending notifications
*
* @return none */
void send_color_indicate(struct indicate_param *param)          // Addition
{
    struct bt_mesh_indicate *ind;

    ind = bt_mesh_ind_alloc(16);
    if(!ind)
    {
        APP_DBG("Unable allocate buffers");
        return;
    }
    memcpy(&(ind->param), param, sizeof(struct indicate_param));

    /* Init indication opcode */
    bt_mesh_model_msg_init(&(ind->buf->b), OP_VENDOR_MESSAGE_ATTR_INDICATION);

    /* Add tid field */
    net_buf_simple_add_u8(&(ind->buf->b), param->tid);

    /* Add brightness attrbute opcode */
    net_buf_simple_add_le16(&(ind->buf->b), ALI_GEN_ATTR_TYPE_COLOR);

    /* Add brightness status (992~20000 corresponds to Tmall control 3000~6400) */
    net_buf_simple_add_le16(&(ind->buf->b), read_led_color(MSG_PIN));           // Addition (modify)

    bt_mesh_indicate_send(ind);
}

/* ***************************************************************************
* @fn als_vendor_init
*
* @brief Alibaba Manufacturer Model Initialization
*
* @param model - callback model parameters
*
* @return always success */
int als_vendor_init(struct bt_mesh_model *model)
{
    uint32_t ran;

    uuid_generate(&cfg);
    oob_key_generate(&cfg);

    /** Random Local TID Value
     *  @Caution Don't use single octer only.
     */
    ran = tmos_rand();
    als_tid += ((uint8_t *)&ran)[0];
    als_tid += ((uint8_t *)&ran)[1];
    als_tid += ((uint8_t *)&ran)[2];
    als_tid += ((uint8_t *)&ran)[3];

    for(int i = 0; i < CONFIG_INDICATE_NUM; i++)
    {
        indicate[i].buf = &ind_buf[i];
        indicate[i].event = (1 << i);
    }

    als_vendor_model_TaskID = TMOS_ProcessEventRegister(als_vendor_model_ProcessEvent);
    return 0;
}

/* ********************************************************************
* @fn      als_vendor_model_ProcessEvent
*
* @brief   
*
* @param   task_id  - The TMOS assigned task ID.
* @param   events - events to process.  This is a bit map and can
*                   contain more than one event.
*
* @return  events not processed */
static uint16_t als_vendor_model_ProcessEvent(uint8_t task_id, uint16_t events)
{
    for(int i = 0; i < CONFIG_INDICATE_NUM; i++)
    {
        if(events & indicate[i].event)
        {
            adv_ind_send(&indicate[i]);
            return (events ^ indicate[i].event);
        }
    }

    // Discard unknown events
    return 0;
}


/******************************** endfile @ main ******************************/
