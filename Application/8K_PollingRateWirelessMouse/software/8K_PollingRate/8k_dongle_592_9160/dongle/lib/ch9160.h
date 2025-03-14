/********************************** (C) COPYRIGHT *******************************
 * File Name          : ch9160.h
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2018/11/12
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#ifndef ch9160_H
#define ch9160_H

#include "stdint.h"
#ifdef __cplusplus
"C" {
#endif

// ACCESS_CMD timeout 10ms
#define ACCESS_CMD_TIMEOUT      16

// ACCESS_STATE
#define STATE_SUCCESS           0x00
#define STATE_PENDING           0xFF

#define INTERFAVE_RECV_BUF_LEN  256
#define INTERFAVE_TRAN_BUF_LEN  256

#define CMD_ACK                 0x80
#define CMD_ACK_LEN             0x07

#define CHK_DATA                0x55
#define CHK_ACK                 0xAA

// Basic data
#define CMD_CHK_CONNECT         0x01
#define CMD_GET_INFO            0x02
#define CMD_SET_INFO            0x03
#define CMD_GET_USB_DESC        0x04
#define CMD_SET_USB_DESC        0x05
#define CMD_SNED_ENDP_DATA1     0x06
#define CMD_SNED_ENDP_DATA2     0x07
#define CMD_RESET               0x08
#define CMD_AUTO_SEND_STATUS    0x89
#define CMD_AUTO_SEND_PC_DATA   0x8A

#define CMD_MRAK_MAX            0x8F
#define CMD_MRAK_ENDP           0x70

#define STATUS_ENUM_SUCCESS         0x00
#define STATUS_SET_REPORT_CHANGE    0x01
#define STATUS_RECV_SET_REPORT      0x02
#define STATUS_RECV_GET_REPORT      0x03
#define STATUS_RECV_BUF_FULL        0x04
#define STATUS_SLEEP                0x05

#define ENTER_SLEEP                 0x55
#define EXIT_SLEEP                  0xAA

#define USB_WEAK_UP_IO              (GPIO_Pin_12)
#define USB_WEAK_UP()               GPIOB_SetBits(USB_WEAK_UP_IO)
#define USB_SLEEP()                 GPIOB_ResetBits(USB_WEAK_UP_IO)

#define  VER_CH9160_FILE            "CH9160_LIB_V1.0"
extern const uint8_t VER_CH9160_LIB[];

typedef void (*access_cmd_cb_t)(uint8_t state, uint8_t cmd, uint8_t *pData, uint16_t len);

/* *
* @brief Register the access layer command to receive callbacks
*
* @param cb - callback function */
void access_register_cmd_cb(access_cmd_cb_t cb);

/* *
* @brief Check USB connection status
*
* @return @ACCESS_STATE. */
uint8_t access_chk_connect(void);

/* *
* @brief Check USB connection status
*
* @return @ACCESS_STATE. */
uint8_t access_get_info(void);

/* *
* @brief Setting USB
*
* @param usb_enable : 1 - Enable USB; 0 - Turn off USB
* io_dir: IO direction 0 means input; 1 means output
* io_pin: IO level 0 means low level; 1 means high level
* endpx_size: 0~9 corresponds to the power of 0~9 with the endpoint length 2, for example, 2 corresponds to the length 4, 6 corresponds to the length 64, 9 corresponds to the length 512
* sleep_off : 2 - USB deep sleep; 1 - Turn off USB sleep; 0 - USB light sleep
*
* @return @ACCESS_STATE. */
uint8_t access_set_info(uint8_t usb_enable, uint8_t io_dir, uint8_t io_pin,
    uint8_t endp1_size, uint8_t endp2_size, uint8_t endp3_size, uint8_t endp4_size, uint8_t sleep_off);

/* *
* @brief Get USB descriptor
*
* @param desc_type : 0x00 USB device descriptor
* 0x01 USB configuration descriptor
* 0x02 USB HID1 report descriptor
* 0x03 USB HID2 report descriptor
* 0x04 USB HID3 report descriptor
* 0x05 USB HID4 report descriptor
* 0x06 USB HID5 report descriptor
* 0x07 USB string 0 (language) descriptor
* 0x08 USB string 1 (manufacturer) descriptor
* 0x09 USB string 2 (product) descriptor
* 0x0A USB string 3 (serial number) descriptor
* 0x0B USB string 4 descriptor
* offset: USB descriptor offset address
* length: USB descriptor data length
*
* @return @ACCESS_STATE. */
uint8_t access_get_usb_desc(uint8_t desc_type, uint16_t offset, uint16_t length);

/* *
* @brief Setting USB descriptor
*
* @param desc_type : 0x00 USB device descriptor
* 0x01 USB configuration descriptor
* 0x02 USB HID1 report descriptor
* 0x03 USB HID2 report descriptor
* 0x04 USB HID3 report descriptor
* 0x05 USB HID4 report descriptor
* 0x06 USB HID5 report descriptor
* 0x07 USB string 0 (language) descriptor
* 0x08 USB string 1 (manufacturer) descriptor
* 0x09 USB string 2 (product) descriptor
* 0x0A USB string 3 (serial number) descriptor
* 0x0B USB string 4 descriptor
* offset: USB descriptor offset address
* length: USB descriptor data length
* pData: USB descriptor data
*
* @return @ACCESS_STATE. */
uint8_t access_set_usb_desc(uint8_t desc_type, uint16_t offset, uint16_t length, uint8_t *pData);

/* *
* @brief The specified endpoint sends data. After sending, you need to wait for a reply to send new data.
*
* @param endp: Endpoint number, supports 1~4
* pData: Data pointer
* length: data length
*
* @return @ACCESS_STATE. */
uint8_t access_send_endp_data( uint8_t endp, uint8_t *pData, uint16_t length);

/* *
* @brief Specifies the endpoint to send data without waiting for a reply
*
* @param endp: Endpoint number, supports 1~4
* pData: Data pointer
* length: data length
*
* @return @ACCESS_STATE. */
uint8_t access_send_endp_data_without_ack( uint8_t endp, uint8_t *pData, uint16_t length);

/* *
* @brief Reset USB
*
* @param reset_type : 0x00: Overall chip reset
* 0x01: Chip USB reset only
*
* @return @ACCESS_STATE. */
uint8_t access_usb_reset(uint8_t reset_type);

/* *
* @brief You need to send a reply after receiving the command actively reported by USB
*
* @param cmd: The received command code that is actively reported by USB
*
* @return @ACCESS_STATE. */
uint8_t access_send_ack(uint8_t cmd);

/* *
* @brief USB data processing */
void trans_process(void);

/* *
* @brief Retransmit the data sent to USB from the previous packet
*
* @return @ACCESS_STATE. */
uint8_t trans_retran_last_data(void );

/* *
* @brief access layer initialization */
void access_Init( void );

/* *
* @brief CH9160 Initialization */
void ch9160_Init( void );


#ifdef __cplusplus
}
#endif

#endif
