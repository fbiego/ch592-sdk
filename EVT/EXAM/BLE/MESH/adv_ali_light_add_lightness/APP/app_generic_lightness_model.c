/********************************** (C) COPYRIGHT *******************************
 * File Name          : app_generic_onoff_model.c
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
#include "app_generic_onoff_model.h"
#include "app_generic_lightness_model.h"
#include "app_vendor_model.h"

/*********************************************************************
 * GLOBAL TYPEDEFS
 */

#define ALI_DEF_TTL    (10)

// Analog brightness value
uint16_t led_lightness = 100;

/* ***************************************************************************
* @fn read_led_lightness
*
* @brief Get the current light brightness
*
* @param led_pin - LED pin.
*
* @return Brightness */
uint16_t read_led_lightness(uint32_t led_pin)
{
    APP_DBG("led_lightness: %d ", led_lightness);
    return led_lightness;
}

/* ***************************************************************************
* @fn set_led_lightness
*
* @brief Set the current light brightness
*
* @param led_pin - LED pin.
* @param lightness - brightness.
*
* @return none */
void set_led_lightness(uint32_t led_pin, uint16_t lightness)
{
    led_lightness = lightness;
}

/* ***************************************************************************
* @fn gen_lightness_status
*
* @brief reply to Tmall Ghost Brightness
*
* @param model - Model parameters
* @param ctx - Data Parameters
*
* @return none */
static void gen_lightness_status(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx)
{
    NET_BUF_SIMPLE_DEFINE(msg, 32);
    int err;

    bt_mesh_model_msg_init(&msg, BLE_MESH_MODEL_OP_LIGHT_LIGHTNESS_STATUS);
    net_buf_simple_add_le16(&msg, read_led_lightness(MSG_PIN));

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

/* ***************************************************************************
* @fn gen_onoff_get
*
* @brief The brightness command issued by Tmall Genie
*
* @param model - Model parameters
* @param ctx - Data Parameters
* @param buf - Data content
*
* @return none */
static void gen_lightness_get(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx, struct net_buf_simple *buf)
{
    APP_DBG(" ");

    gen_lightness_status(model, ctx);
}

/* ***************************************************************************
* @fn gen_lightness_set
*
* @brief The setting brightness command issued by Tmall Elf. If the brightness is different from the current brightness, you still need to send an ind to Tmall
*
* @param model - Model parameters
* @param ctx - Data Parameters
* @param buf - Data content
*
* @return none */
static void gen_lightness_set(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx, struct net_buf_simple *buf)
{
    struct indicate_param param = {
        .trans_cnt = 0x09,
        .period = K_MSEC(300),
        .rand = (tmos_rand() % 50),
        .tid = als_avail_tid_get(),
    };

    APP_DBG("ttl: 0x%02x dst: 0x%04x rssi: %d lightness: %d",
            ctx->recv_ttl, ctx->recv_dst, ctx->recv_rssi, buf->data[0] | (buf->data[1] << 8));

    set_led_lightness(MSG_PIN, buf->data[0] | (buf->data[1] << 8));

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

    send_lightness_indicate(&param);

    gen_lightness_status(model, ctx);
}

/* ***************************************************************************
* @fn gen_lightness_set_unack
*
* @brief The setting brightness command (no response) issued by Tmall Elf. If the brightness is different from the current brightness, you still need to send an ind to Tmall
*
* @param model - Model parameters
* @param ctx - Data Parameters
* @param buf - Data content
*
* @return none */
static void gen_lightness_set_unack(struct bt_mesh_model   *model,
                                    struct bt_mesh_msg_ctx *ctx,
                                    struct net_buf_simple  *buf)
{
    uint16_t lightness;

    APP_DBG(" ");

    lightness = read_led_lightness(MSG_PIN);
    if(lightness != (buf->data[0] | (buf->data[1] << 8)))
    {
        set_led_lightness(MSG_PIN, buf->data[0] | (buf->data[1] << 8));
    }
}

const struct bt_mesh_model_op gen_lightness_op[] = {
    {BLE_MESH_MODEL_OP_LIGHT_LIGHTNESS_GET, 0, gen_lightness_get},
    {BLE_MESH_MODEL_OP_LIGHT_LIGHTNESS_SET, 3, gen_lightness_set},
    {BLE_MESH_MODEL_OP_LIGHT_LIGHTNESS_SET_UNACK, 3, gen_lightness_set_unack},
    BLE_MESH_MODEL_OP_END,
};

/******************************** endfile @ main ******************************/
