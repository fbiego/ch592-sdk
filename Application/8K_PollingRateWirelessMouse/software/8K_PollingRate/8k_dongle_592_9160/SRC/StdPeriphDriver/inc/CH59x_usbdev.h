/********************************** (C) COPYRIGHT *******************************
 * File Name          : CH59x_usbdev.h
 * Author             : WCH
 * Version            : V1.2
 * Date               : 2021/11/17
 * Description
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#ifndef __CH59x_USBDEV_H__
#define __CH59x_USBDEV_H__

#ifdef __cplusplus
extern "C" {
#endif

/* HID class request */
#define DEF_USB_GET_IDLE           0x02                                         /* get idle for key or mouse */
#define DEF_USB_GET_PROTOCOL       0x03                                         /* get protocol for bios type */
#define DEF_USB_SET_REPORT         0x09                                         /* set report for key */
#define DEF_USB_SET_IDLE           0x0A                                         /* set idle for key or mouse */
#define DEF_USB_SET_PROTOCOL       0x0B                                         /* set protocol for bios type */

/* The following cache area is the data buffer used by USB module to send and receive, with a total of 9 channels (9 caches). Users can define the corresponding cache area based on the actual number of channels used. */
extern uint8_t *pEP0_RAM_Addr; //ep0(64)+ep4_out(64)+ep4_in(64)
extern uint8_t *pEP1_RAM_Addr; //ep1_out(64)+ep1_in(64)
extern uint8_t *pEP2_RAM_Addr; //ep2_out(64)+ep2_in(64)
extern uint8_t *pEP3_RAM_Addr; //ep3_out(64)+ep3_in(64)
extern uint8_t *pEP5_RAM_Addr; //ep5_out(64)+ep5_in(64)
extern uint8_t *pEP6_RAM_Addr; //ep6_out(64)+ep6_in(64)

#define pSetupReqPak          ((PUSB_SETUP_REQ)pEP0_RAM_Addr)
#define pEP0_DataBuf          (pEP0_RAM_Addr)
#define pEP1_OUT_DataBuf      (pEP1_RAM_Addr)
#define pEP1_IN_DataBuf       (pEP1_RAM_Addr + 64)
#define pEP2_OUT_DataBuf      (pEP2_RAM_Addr)
#define pEP2_IN_DataBuf       (pEP2_RAM_Addr + 64)
#define pEP3_OUT_DataBuf      (pEP3_RAM_Addr)
#define pEP3_IN_DataBuf       (pEP3_RAM_Addr + 64)
#define pEP4_OUT_DataBuf      (pEP0_RAM_Addr + 64)
#define pEP4_IN_DataBuf       (pEP0_RAM_Addr + 128)
#define pEP5_OUT_DataBuf      (pEP5_RAM_Addr)
#define pEP5_IN_DataBuf       (pEP5_RAM_Addr + 64)
#define pEP6_OUT_DataBuf      (pEP6_RAM_Addr)
#define pEP6_IN_DataBuf       (pEP6_RAM_Addr + 64)

/* *
* @brief USB device function initialization, 4 endpoints, 8 channels. */
void USB_DeviceInit(void);

/* *
* @brief USB device reply transmission processing */
void USB_DevTransProcess(void);

/* *
* @brief endpoint 1 download data processing
*
* @param l - Pending data length (<64B) */
void DevEP1_OUT_Deal(uint8_t l);

/* *
* @brief endpoint 2 download data processing
*
* @param l - Pending data length (<64B) */
void DevEP2_OUT_Deal(uint8_t l);

/* *
* @brief endpoint 3 download data processing
*
* @param l - Pending data length (<64B) */
void DevEP3_OUT_Deal(uint8_t l);

/* *
* @brief endpoint 4 download data processing
*
* @param l - Pending data length (<64B) */
void DevEP4_OUT_Deal(uint8_t l);

/* *
* @brief Endpoint 1 data upload
*
* @param l - Upload data length (<64B) */
void DevEP1_IN_Deal(uint8_t l);

/* *
* @brief Endpoint 2 data upload
*
* @param l - Upload data length (<64B) */
void DevEP2_IN_Deal(uint8_t l);

/* *
* @brief Endpoint 3 data upload
*
* @param l - Upload data length (<64B) */
void DevEP3_IN_Deal(uint8_t l);

/* *
* @brief Endpoint 4 data upload
*
* @param l - Upload data length (<64B) */
void DevEP4_IN_Deal(uint8_t l);

/* *
* @brief endpoint 5 data upload
*
* @param l - Upload data length (<64B) */
void DevEP5_IN_Deal(uint8_t l);

/* *
* @brief Endpoint 6 data upload
*
* @param l - Upload data length (<64B) */
void DevEP6_IN_Deal(uint8_t l);

/* *
* @brief query whether endpoint 1 is uploaded
*
* @return 0-not completed (!0)-completed */
#define EP1_GetINSta()    (R8_UEP1_CTRL & UEP_T_RES_NAK)

/* *
* @brief query whether endpoint 2 is uploaded
*
* @return 0-not completed (!0)-completed */
#define EP2_GetINSta()    (R8_UEP2_CTRL & UEP_T_RES_NAK)

/* *
* @brief query whether endpoint 3 is uploaded
*
* @return 0-not completed (!0)-completed */
#define EP3_GetINSta()    (R8_UEP3_CTRL & UEP_T_RES_NAK)

/* *
* @brief query whether endpoint 4 is uploaded
*
* @return 0-not completed (!0)-completed */
#define EP4_GetINSta()    (R8_UEP4_CTRL & UEP_T_RES_NAK)

/* *
* @brief Turn off USB pull-up resistor */
#define USB_DisablePin()  (R16_PIN_ANALOG_IE &= ~(RB_PIN_USB_IE | RB_PIN_USB_DP_PU))

/* *
* @brief Close USB */
#define USB_Disable()     (R32_USB_CONTROL = 0)

#ifdef __cplusplus
}
#endif

#endif // __CH59x_USBDEV_H__
