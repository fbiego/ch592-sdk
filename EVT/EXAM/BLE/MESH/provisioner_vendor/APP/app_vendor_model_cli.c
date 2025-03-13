/********************************** (C) COPYRIGHT *******************************
 * File Name          : app_vendor_model_cli.c
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
#include "app_mesh_config.h"
#include "MESH_LIB.h"
#include "app_vendor_model_cli.h"
#include "app_vendor_model_srv.h"

/*********************************************************************
 * GLOBAL TYPEDEFS
 */

// The maximum sending length of the application layer is CONFIG_MESH_UNSEG_LENGTH_DEF without subcontracting, and the maximum subcontracting is CONFIG_MESH_TX_SEG_DEF*BLE_MESH_APP_SEG_SDU_MAX-8 (decided based on RAM usage)
#define APP_MAX_TX_SIZE    MAX(CONFIG_MESH_UNSEG_LENGTH_DEF, CONFIG_MESH_TX_SEG_DEF *BLE_MESH_APP_SEG_SDU_MAX - 8)

static uint8_t vendor_model_cli_TaskID = 0; // Task ID for internal task/event processing
static uint8_t cli_send_tid = 0;
static int32_t cli_msg_timeout = K_SECONDS(2); // Responsive data transmission timeout time, default 2 seconds

static struct net_buf       cli_trans_buf;
static struct bt_mesh_trans cli_trans = {
    .buf = &cli_trans_buf,
};

static struct net_buf       cli_write_buf;
static struct bt_mesh_write cli_write = {
    .buf = &cli_write_buf,
};

static struct bt_mesh_vendor_model_cli *vendor_model_cli;

static uint16_t vendor_model_cli_ProcessEvent(uint8_t task_id, uint16_t events);
static void     write_reset(struct bt_mesh_write *write, int err);
static void     adv_cli_trans_send(void);

/*********************************************************************
 * GLOBAL TYPEDEFS
 */

/*********************************************************************
 * GLOBAL TYPEDEFS
 */

/*********************************************************************
 * @fn      vendor_cli_tid_get
 *
 * @brief   TID selection method
 *
 * @return  TID
 */
uint8_t vendor_cli_tid_get(void)
{
    cli_send_tid++;
    if(cli_send_tid > 127)
        cli_send_tid = 0;
    return cli_send_tid;
}

/* ***************************************************************************
* @fn vendor_model_cli_reset
*
* @brief Reset the vendor model service and cancel all sending processes
*
* @return none */
static void vendor_model_cli_reset(void)
{
    vendor_model_cli->op_pending = 0U;
    vendor_model_cli->op_req = 0U;

    if(cli_write.buf->__buf)
    {
        write_reset(&cli_write, -ECANCELED);
    }
    if(cli_trans.buf->__buf)
    {
        vendor_message_cli_trans_reset();
    }

    tmos_stop_task(vendor_model_cli_TaskID, VENDOR_MODEL_CLI_RSP_TIMEOUT_EVT);
    tmos_stop_task(vendor_model_cli_TaskID, VENDOR_MODEL_CLI_WRITE_EVT);
}

/* ***************************************************************************
* @fn vendor_model_cli_rsp_recv
*
* @brief calls the callback passed in the application layer
*
* @param val - Callback parameters, including message type, data content, length, source address
* @param status - Status
*
* @return none */
static void vendor_model_cli_rsp_recv(vendor_model_cli_status_t *val, uint8_t status)
{
    if(vendor_model_cli == NULL || (!vendor_model_cli->op_req))
    {
        return;
    }

    val->vendor_model_cli_Hdr.opcode = vendor_model_cli->op_req;
    val->vendor_model_cli_Hdr.status = status;

    vendor_model_cli_reset();

    if(vendor_model_cli->handler)
    {
        vendor_model_cli->handler(val);
    }
}

/* ***************************************************************************
* @fn vendor_model_cli_wait
*
* @brief The default notification application layer after two seconds is timed out
*
* @return Reference BLE_LIB err code */
static int vendor_model_cli_wait(void)
{
    int err;

    err = tmos_start_task(vendor_model_cli_TaskID,
                          VENDOR_MODEL_CLI_RSP_TIMEOUT_EVT, cli_msg_timeout);

    return err;
}

/* ***************************************************************************
* @fn vendor_model_cli_prepare
*
* @brief Pre-send, record the current message type
*
* @param op_req - type of message sent
* @param op - The type of message you are looking forward to receiving
*
* @return Error code */
static int vendor_model_cli_prepare(uint32_t op_req, uint32_t op)
{
    if(!vendor_model_cli)
    {
        APP_DBG("No available Configuration Client context!");
        return -EINVAL;
    }

    if(vendor_model_cli->op_pending)
    {
        APP_DBG("Another synchronous operation pending");
        return -EBUSY;
    }

    vendor_model_cli->op_req = op_req;
    vendor_model_cli->op_pending = op;

    return 0;
}

/* ***************************************************************************
* @fn vendor_cli_sync_handler
*
* @brief Notify the application layer that the current op_code timeout has expired
*
* @return none */
static void vendor_cli_sync_handler(void)
{
    vendor_model_cli_status_t vendor_model_cli_status;

    tmos_memset(&vendor_model_cli_status, 0, sizeof(vendor_model_cli_status_t));

    write_reset(&cli_write, -ETIMEDOUT);

    vendor_model_cli_rsp_recv(&vendor_model_cli_status, 0xFF);
}

/* ***************************************************************************
* @fn vendor_message_cli_ack
*
* @brief Send vendor_message_cli_ack - This message is used to reply to Vendor Model Client to Vendor Model Server,
* Used to indicate that the indication has been received from the Vendor Model Server
*
* @param model - Model parameters.
* @param ctx - Data parameters.
* @param buf - Data content.
*
* @return none */
static void vendor_message_cli_ack(struct bt_mesh_model   *model,
                                   struct bt_mesh_msg_ctx *ctx, struct net_buf_simple *buf)
{
    NET_BUF_SIMPLE_DEFINE(msg, APP_MAX_TX_SIZE + 8);
    uint8_t recv_tid;
    int     err;

    recv_tid = net_buf_simple_pull_u8(buf);

    APP_DBG("tid 0x%02x ", recv_tid);

    /* Init indication opcode */
    bt_mesh_model_msg_init(&msg, OP_VENDOR_MESSAGE_TRANSPARENT_ACK);

    /* Add tid field */
    net_buf_simple_add_u8(&msg, recv_tid);

    err = bt_mesh_model_send(model, ctx, &msg, NULL, NULL);
    if(err)
    {
        APP_DBG("#mesh-onoff STATUS: send status failed: %d", err);
    }
}

/* ***************************************************************************
* @fn vendor_message_cli_trans
*
* @brief received transparent data
*
* @param model - Model parameters.
* @param ctx - Data parameters.
* @param buf - Data content.
*
* @return none */
static void vendor_message_cli_trans(struct bt_mesh_model   *model,
                                     struct bt_mesh_msg_ctx *ctx, struct net_buf_simple *buf)
{
    vendor_model_cli_status_t vendor_model_cli_status;
    uint8_t                  *pData = buf->data;
    uint16_t                  len = buf->len;

    if((pData[0] != vendor_model_cli->cli_tid.trans_tid) ||
       (ctx->addr != vendor_model_cli->cli_tid.trans_addr))
    {
        vendor_model_cli->cli_tid.trans_tid = pData[0];
        vendor_model_cli->cli_tid.trans_addr = ctx->addr;
        // Start with tid
        pData++;
        len--;
        vendor_model_cli_status.vendor_model_cli_Hdr.opcode =
            OP_VENDOR_MESSAGE_TRANSPARENT_MSG;
        vendor_model_cli_status.vendor_model_cli_Hdr.status = 0;
        vendor_model_cli_status.vendor_model_cli_Event.trans.pdata = pData;
        vendor_model_cli_status.vendor_model_cli_Event.trans.len = len;
        vendor_model_cli_status.vendor_model_cli_Event.trans.addr = ctx->addr;

        if(vendor_model_cli->handler)
        {
            vendor_model_cli->handler(&vendor_model_cli_status);
        }
    }
}

/* ***************************************************************************
* @fn vendor_message_cli_ind
*
* @brief Receive Indication data and then answer vendor_message_cli_ack
*
* @param model - Model parameters.
* @param ctx - Data parameters.
* @param buf - Data content.
*
* @return none */
static void vendor_message_cli_ind(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
                                   struct net_buf_simple *buf)
{
    vendor_model_cli_status_t vendor_model_cli_status;
    uint8_t                  *pData = buf->data;
    uint16_t                  len = buf->len;
    APP_DBG("src: 0x%04x dst: 0x%04x rssi: %d app_idx: 0x%x", ctx->addr, ctx->recv_dst, ctx->recv_rssi, ctx->app_idx);

    if((pData[0] != vendor_model_cli->cli_tid.ind_tid) ||
       (ctx->addr != vendor_model_cli->cli_tid.trans_addr))
    {
        vendor_model_cli->cli_tid.ind_tid = pData[0];
        vendor_model_cli->cli_tid.trans_addr = ctx->addr;
        // Start with tid
        pData++;
        len--;
        vendor_model_cli_status.vendor_model_cli_Hdr.opcode =
            OP_VENDOR_MESSAGE_TRANSPARENT_IND;
        vendor_model_cli_status.vendor_model_cli_Hdr.status = 0;
        vendor_model_cli_status.vendor_model_cli_Event.ind.pdata = pData;
        vendor_model_cli_status.vendor_model_cli_Event.ind.len = len;
        vendor_model_cli_status.vendor_model_cli_Event.ind.addr = ctx->addr;

        if(vendor_model_cli->handler)
        {
            vendor_model_cli->handler(&vendor_model_cli_status);
        }
    }
    vendor_message_cli_ack(model, ctx, buf);
}

/* ***************************************************************************
* @fn vendor_message_cli_cfm
*
* @brief Received vendor_message_cli_cfm - This message is used to reply to Vendor Model Server to Vendor Model Client,
* Used to indicate that the Write has been received from the Vendor Model Client
*
* @param model - Model parameters.
* @param ctx - Data parameters.
* @param buf - Data content.
*
* @return none */
static void vendor_message_cli_cfm(struct bt_mesh_model   *model,
                                   struct bt_mesh_msg_ctx *ctx, struct net_buf_simple *buf)
{
    uint8_t                   recv_tid;
    vendor_model_cli_status_t vendor_model_cli_status;

    recv_tid = net_buf_simple_pull_u8(buf);

    APP_DBG("src: 0x%04x dst: 0x%04x tid 0x%02x rssi: %d", ctx->addr,
            ctx->recv_dst, recv_tid, ctx->recv_rssi);

    if(cli_write.param.tid == recv_tid)
    {
        write_reset(&cli_write, 0);
        vendor_model_cli_rsp_recv(&vendor_model_cli_status, 0);
    }
}

// Opcode corresponding processing function
const struct bt_mesh_model_op vnd_model_cli_op[] = {
    {OP_VENDOR_MESSAGE_TRANSPARENT_MSG, 0, vendor_message_cli_trans},
    {OP_VENDOR_MESSAGE_TRANSPARENT_IND, 0, vendor_message_cli_ind},
    {OP_VENDOR_MESSAGE_TRANSPARENT_CFM, 0, vendor_message_cli_cfm},
    BLE_MESH_MODEL_OP_END,
};

/* ***************************************************************************
* @fn vendor_message_cli_send_trans
*
* @brief send_trans ,trans data transmission channel
*
* @param param - Send parameters.
* @param pData - Data pointer.
* @param len - Data length, maximum is (APP_MAX_TX_SIZE).
*
* @return Reference Global_Error_Code */
int vendor_message_cli_send_trans(struct send_param *param, uint8_t *pData,
                                  uint16_t len)
{
    if(!param->addr)
        return -EINVAL;
    if(cli_trans.buf->__buf)
        return -EBUSY;
    if(len > (APP_MAX_TX_SIZE))
        return -EINVAL;

    cli_trans.buf->__buf = tmos_msg_allocate(len + 8);
    if(!(cli_trans.buf->__buf))
    {
        APP_DBG("No enough space!");
        return -ENOMEM;
    }
    cli_trans.buf->size = len + 4;
    /* Init indication opcode */
    bt_mesh_model_msg_init(&(cli_trans.buf->b), OP_VENDOR_MESSAGE_TRANSPARENT_MSG);

    /* Add tid field */
    net_buf_simple_add_u8(&(cli_trans.buf->b), param->tid);

    net_buf_simple_add_mem(&(cli_trans.buf->b), pData, len);

    memcpy(&cli_trans.param, param, sizeof(struct send_param));

    if(param->rand)
    {
        // Delayed send
        tmos_start_task(vendor_model_cli_TaskID, VENDOR_MODEL_CLI_TRANS_EVT, param->rand);
    }
    else
    {
        // Send directly
        adv_cli_trans_send();
    }
    return 0;
}

/* ***************************************************************************
* @fn vendor_message_cli_write
*
* @brief write, there is a reply transmission data channel
*
* @param param - Send parameters.
* @param pData - Data pointer.
* @param len - Data length, maximum is (APP_MAX_TX_SIZE).
*
* @return Reference Global_Error_Code */
int vendor_message_cli_write(struct send_param *param, uint8_t *pData, uint16_t len)
{
    if(!param->addr)
        return -EINVAL;
    if(cli_write.buf->__buf)
        return -EBUSY;
    if(len > (APP_MAX_TX_SIZE))
        return -EINVAL;

    cli_write.buf->__buf = tmos_msg_allocate(len + 8);
    if(!(cli_write.buf->__buf))
    {
        APP_DBG("No enough space!");
        return -ENOMEM;
    }
    cli_write.buf->size = len + 4;
    /* Init indication opcode */
    bt_mesh_model_msg_init(&(cli_write.buf->b),
                           OP_VENDOR_MESSAGE_TRANSPARENT_WRT);

    /* Add tid field */
    net_buf_simple_add_u8(&(cli_write.buf->b), param->tid);

    net_buf_simple_add_mem(&(cli_write.buf->b), pData, len);

    memcpy(&cli_write.param, param, sizeof(struct send_param));

    vendor_model_cli_prepare(OP_VENDOR_MESSAGE_TRANSPARENT_WRT,
                             OP_VENDOR_MESSAGE_TRANSPARENT_CFM);

    tmos_start_task(vendor_model_cli_TaskID, VENDOR_MODEL_CLI_WRITE_EVT,
                    param->rand);

    vendor_model_cli_wait();
    return 0;
}

/* ***************************************************************************
* @fn adv_cli_trans_send
*
* @brief send Transmission cli_trans
*
* @return none */
static void adv_cli_trans_send(void)
{
    int err;
    NET_BUF_SIMPLE_DEFINE(msg, APP_MAX_TX_SIZE + 8);

    struct bt_mesh_msg_ctx ctx = {
        .net_idx = cli_trans.param.net_idx ? cli_trans.param.net_idx : BLE_MESH_KEY_ANY,
        .app_idx = cli_trans.param.app_idx ? cli_trans.param.app_idx : vendor_model_cli->model->keys[0],
        .addr = cli_trans.param.addr,
    };

    if(cli_trans.buf->__buf == NULL)
    {
        APP_DBG("NULL buf");
        return;
    }

    if(cli_trans.param.trans_cnt == 0)
    {
        //		APP_DBG("cli_trans.buf.trans_cnt over");
        tmos_msg_deallocate(cli_trans.buf->__buf);
        cli_trans.buf->__buf = NULL;
        return;
    }

    cli_trans.param.trans_cnt--;

    ctx.send_ttl = cli_trans.param.send_ttl;

    /** TODO */
    net_buf_simple_add_mem(&msg, cli_trans.buf->data, cli_trans.buf->len);

    err = bt_mesh_model_send(vendor_model_cli->model, &ctx, &msg, NULL, NULL);
    if(err)
    {
        APP_DBG("Unable send model message (err:%d)", err);
        tmos_msg_deallocate(cli_trans.buf->__buf);
        cli_trans.buf->__buf = NULL;
        return;
    }

    if(cli_trans.param.trans_cnt == 0)
    {
        //    APP_DBG("cli_trans.buf.trans_cnt over");
        tmos_msg_deallocate(cli_trans.buf->__buf);
        cli_trans.buf->__buf = NULL;
        return;
    }
    // Retransmit
    tmos_start_task(vendor_model_cli_TaskID, VENDOR_MODEL_CLI_TRANS_EVT,
                    cli_trans.param.period);
}

/* ***************************************************************************
* @fn vendor_message_cli_trans_reset
*
* @brief Cancel the task of sending trans data and release the cache
*
* @return none */
void vendor_message_cli_trans_reset(void)
{
    tmos_msg_deallocate(cli_trans.buf->__buf);
    cli_trans.buf->__buf = NULL;
    tmos_stop_task(vendor_model_cli_TaskID, VENDOR_MODEL_CLI_TRANS_EVT);
}

/* ***************************************************************************
* @fn write_reset
*
* @brief Call write to send a complete callback, freeing the cache
*
* @param write - write that needs to be released.
* @param err - Error status.
*
* @return none */
static void write_reset(struct bt_mesh_write *write, int err)
{
    if(write->param.cb && write->param.cb->end)
    {
        write->param.cb->end(err, write->param.cb_data);
    }

    tmos_msg_deallocate(write->buf->__buf);
    write->buf->__buf = NULL;
}

/* ***************************************************************************
* @fn write_start
*
* @brief send write start callback
*
* @param duration - The duration of this sending (ms).
* @param err - Error status.
* @param cb_data - Callback parameters filled in when the error status is sent.
*
* @return none */
static void write_start(uint16_t duration, int err, void *cb_data)
{
    struct bt_mesh_write *write = cb_data;

    if(write->buf->__buf == NULL)
    {
        return;
    }

    if(err)
    {
        APP_DBG("Unable send indicate (err:%d)", err);
        tmos_start_task(vendor_model_cli_TaskID, VENDOR_MODEL_CLI_WRITE_EVT,
                        K_MSEC(100));
        return;
    }
}

/* ***************************************************************************
* @fn write_end
*
* @brief send write end callback
*
* @param err - Error status.
* @param cb_data - Callback parameters filled in when the error status is sent.
*
* @return none */
static void write_end(int err, void *cb_data)
{
    struct bt_mesh_write *write = cb_data;
    APP_DBG("write_end (err:%d)", err);

    if(write->buf->__buf == NULL)
    {
        return;
    }

    tmos_start_task(vendor_model_cli_TaskID, VENDOR_MODEL_CLI_WRITE_EVT,
                    write->param.period);
}

// Send indicated callback structure
const struct bt_mesh_send_cb write_cb = {
    .start = write_start,
    .end = write_end,
};

/* ********************************************************************
* @fn      adv_cli_write_send
*
* @brief    cli_write
*
* @return  none */
static void adv_cli_write_send(void)
{
    int err;
    NET_BUF_SIMPLE_DEFINE(msg, APP_MAX_TX_SIZE + 8);

    struct bt_mesh_msg_ctx ctx = {
        .net_idx = cli_write.param.net_idx ? cli_write.param.net_idx : BLE_MESH_KEY_ANY,
        .app_idx = cli_write.param.app_idx ? cli_write.param.app_idx : vendor_model_cli->model->keys[0],
        .addr = cli_write.param.addr,
    };

    if(cli_write.buf->__buf == NULL)
    {
        APP_DBG("NULL buf");
        return;
    }

    if(cli_write.param.trans_cnt == 0)
    {
        //		APP_DBG("cli_write.buf.trans_cnt over");
        write_reset(&cli_write, -ETIMEDOUT);
        return;
    }

    cli_write.param.trans_cnt--;

    ctx.send_ttl = cli_write.param.send_ttl;

    /** TODO */
    net_buf_simple_add_mem(&msg, cli_write.buf->data, cli_write.buf->len);

    err = bt_mesh_model_send(vendor_model_cli->model, &ctx, &msg, &write_cb,
                             &cli_write);
    if(err)
    {
        APP_DBG("Unable send model message (err:%d)", err);

        write_reset(&cli_write, err);
        return;
    }
}

/* ***************************************************************************
* @fn vendor_model_cli_init
*
* @brief vendor model initialization
*
* @param model - Point to the manufacturer model structure
*
* @return always SUCCESS */
int vendor_model_cli_init(struct bt_mesh_model *model)
{
    vendor_model_cli = model->user_data;
    vendor_model_cli->model = model;

    vendor_model_cli_TaskID = TMOS_ProcessEventRegister(vendor_model_cli_ProcessEvent);
    return 0;
}

/* ********************************************************************
* @fn      vendor_model_cli_ProcessEvent
*
* @brief   
*
* @param   task_id  - The TMOS assigned task ID.
* @param   events - events to process.  This is a bit map and can
*                   contain more than one event.
*
* @return  events not processed */
static uint16_t vendor_model_cli_ProcessEvent(uint8_t task_id, uint16_t events)
{
    if(events & VENDOR_MODEL_CLI_TRANS_EVT)
    {
        adv_cli_trans_send();
        return (events ^ VENDOR_MODEL_CLI_TRANS_EVT);
    }
    if(events & VENDOR_MODEL_CLI_RSP_TIMEOUT_EVT)
    {
        vendor_cli_sync_handler();
        return (events ^ VENDOR_MODEL_CLI_RSP_TIMEOUT_EVT);
    }
    if(events & VENDOR_MODEL_CLI_WRITE_EVT)
    {
        adv_cli_write_send();
        return (events ^ VENDOR_MODEL_CLI_WRITE_EVT);
    }
    // Discard unknown events
    return 0;
}

/******************************** endfile @ main ******************************/
