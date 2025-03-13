/********************************** (C) COPYRIGHT *******************************
 * File Name          : app_generic_color_model.h
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2018/11/12
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#ifndef app_generic_color_model_H
#define app_generic_colormodel_H

#ifdef __cplusplus
extern "C" {
#endif

#include "MESH_LIB.h"

/* *
* @brief The structure array of processing functions stored in the received command code */
extern const struct bt_mesh_model_op gen_color_op[];

/* *
* @brief Get the current light color temperature
*
* @param led_pin - LED pin.
*
* @return Color temperature */
uint16_t read_led_color(uint32_t led_pin);

/* *
* @brief Set the current light color temperature
*
* @param led_pin - LED pin.
* @param color - Color temperature. */
void set_led_color(uint32_t led_pin, uint16_t color);

#ifdef __cplusplus
}
#endif

#endif
