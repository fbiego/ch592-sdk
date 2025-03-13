/********************************** (C) COPYRIGHT *******************************
 * File Name          : app_trans_process.h
 * Author             : WCH
 * Version            : V1.1
 * Date               : 2022/03/31
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#ifndef app_trans_process_H
#define app_trans_process_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#define LED_PIN    GPIO_Pin_18

/******************************************************************************/
/* *
* @brief reads the LED status
*
* @param led_pin - pin
*
* @return led status */
BOOL read_led_state(uint32_t led_pin);

/* *
* @brief Setting the LED status
*
* @param led_pin - pin
* @param on - Status */
void set_led_state(uint32_t led_pin, BOOL on);

/* *
* @brief flipped LED status
*
* @param led_pin - pin */
void toggle_led_state(uint32_t led_pin);

/* *
* @brief process trans data
*
* @param pValue - Data pointer.
* len - Data length.
* src_Addr - Data source address.
* dst_Addr - Data destination address. */
extern void app_trans_process(uint8_t *pValue, uint8_t len, uint16_t src_Addr, uint16_t dst_Addr);

/******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif
