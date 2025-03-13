/********************************** (C) COPYRIGHT *******************************
 * File Name          : app_als_fan_attr.c
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
#include "CONFIG.h"
#include "MESH_LIB.h"
#include "app_mesh_config.h"
#include "app_als_fan_attr.h"
#include "app_vendor_model.h"
#include "app_vendor2_model.h"

/*********************************************************************
 * GLOBAL TYPEDEFS
 */

#define ALI_DEF_TTL    (10)

// Simulate LED_mode value
uint8_t device_angle_auto_LR = 0;

/* *********************************************************************************************
* Function Name: read_angle_auto_LR
* Description: Get the current angle_auto_LR
* Input: None
* Return : None
********************************************************************************************* */
uint8_t read_angle_auto_LR(void)
{
    APP_DBG("device_led_mode: %d ", device_angle_auto_LR);
    return device_angle_auto_LR;
}

/* *********************************************************************************************
* Function Name: set_angle_auto_LR
* Description: Set the current angle_auto_LR
* Input: None
* Return : None
********************************************************************************************* */
void set_angle_auto_LR(uint8_t angle_auto_LR)
{
    device_angle_auto_LR = angle_auto_LR;
}

/* *********************************************************************************************
* Function Name : gen_angle_auto_LR_status
* Description: Reply to Tmall Elf Angle_auto_LR
* Input: model: Model parameters
* ctx: Data parameters
* Return : None
********************************************************************************************* */
static void gen_angle_auto_LR_status(struct bt_mesh_model   *model,
                                     struct bt_mesh_msg_ctx *ctx)
{
    NET_BUF_SIMPLE_DEFINE(msg, 32);
    int err;

    ////////////////////////////////////////////////////////////////////////
    //		0xD3	0xA8	0x01	|	0x##	|	0x0a	0x01			|	0x##	0x##				//
    //		Opcode						|	TID		|	Attribute Type	|	Attribute Value		//
    ////////////////////////////////////////////////////////////////////////
    bt_mesh_model_msg_init(&msg, OP_VENDOR_MESSAGE_ATTR_STATUS);
    net_buf_simple_add_u8(&msg, als_avail_tid_get());
    net_buf_simple_add_le16(&msg, ALI_GEN_ATTR_TYPE_ANGLEAUTO_LR_ONOFF);
    net_buf_simple_add_u8(&msg, read_angle_auto_LR());

    APP_DBG("ttl: 0x%02x dst: 0x%04x", ctx->recv_ttl, ctx->recv_dst);

    if(ctx->recv_ttl != ALI_DEF_TTL)
    {
        ctx->send_ttl = BLE_MESH_TTL_DEFAULT;
    }
    else
    {
        ctx->send_ttl = 0;
    }

    err = bt_mesh_model_send(model, ctx, &msg, NULL, NULL);
    if(err)
    {
        APP_DBG("send status failed: %d", err);
    }
}

/* *********************************************************************************************
* Function Name: gen_angle_auto_LR_get
* Description: The angle_auto_LR command issued by Tmall elves
* Input: model: Model parameters
* ctx: Data parameters
* buf: Data content
* Return : None
********************************************************************************************* */
void gen_angle_auto_LR_get(struct bt_mesh_model   *model,
                           struct bt_mesh_msg_ctx *ctx,
                           struct net_buf_simple  *buf)
{
    APP_DBG(" ");
    gen_angle_auto_LR_status(model, ctx);
}

/* *********************************************************************************************
* Function Name : gen_fan_set
* Description: Setting fan command issued by Tmall elves
Need to send an ind to Tmall
* Input: model: Model parameters
* ctx: Data parameters
* buf: Data content
* Return : None
********************************************************************************************* */
void gen_fan_set(struct bt_mesh_model   *model,
                 struct bt_mesh_msg_ctx *ctx,
                 struct net_buf_simple  *buf)
{
    struct indicate_param param = {
        .trans_cnt = 0x09,
        .period = K_MSEC(300),
        .rand = (tmos_rand() % 50),
        .tid = als_avail_tid_get(),
    };

    APP_DBG("ttl: 0x%02x dst: 0x%04x rssi: %d ",
            ctx->recv_ttl, ctx->recv_dst, ctx->recv_rssi);

    if(ctx->recv_ttl != ALI_DEF_TTL)
    {
        param.send_ttl = BLE_MESH_TTL_DEFAULT;
    }

    /* Overwrite default configuration */
    if(BLE_MESH_ADDR_IS_UNICAST(ctx->recv_dst))
    {
        param.rand = 0;
        param.send_ttl = BLE_MESH_TTL_DEFAULT;
        param.period = K_MSEC(100);
    }

    if((buf->data[1] | (buf->data[2] << 8)) == ALI_GEN_ATTR_TYPE_ANGLEAUTO_LR_ONOFF)
    {
        // The command is the set value
        set_angle_auto_LR(buf->data[3]);

        send_angle_auto_LR_indicate(&param);

        gen_angle_auto_LR_status(model, ctx);
    }
    else if((buf->data[1] | (buf->data[2] << 8)) == ALI_GEN_ATTR_TYPE_ANGLEAUTO_LR_ONOFF)
    {
    }
}

/* *********************************************************************************************
* Function Name: gen_fan_set_unack
* Description: Setting fan command issued by Tmall elves (no response)
* Input: model: Model parameters
* ctx: Data parameters
* buf: Data content
* Return : None
********************************************************************************************* */
void gen_fan_set_unack(struct bt_mesh_model   *model,
                       struct bt_mesh_msg_ctx *ctx,
                       struct net_buf_simple  *buf)
{
    APP_DBG(" ");

    if((buf->data[1] | (buf->data[2] << 8)) == ALI_GEN_ATTR_TYPE_ANGLEAUTO_LR_ONOFF)
    {
        // The command is the set value
        set_angle_auto_LR(buf->data[3]);
    }
}

/******************************** endfile @ main ******************************/
