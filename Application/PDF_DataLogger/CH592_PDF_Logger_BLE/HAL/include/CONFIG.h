/* ********************************* (C) COPYRIGHT ***************************
* File Name : CONFIG.h
* Author: WCH
* Version: V1.2
* Date: 2022/01/18
* Description: Configuration description and default value. It is recommended to modify the current value in preprocessing in the project configuration.
************************************************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
********************************************************************************************* */

/******************************************************************************/
#ifndef __CONFIG_H
#define __CONFIG_H

#define	ID_CH592							0x92

#define CHIP_ID								ID_CH592

#ifdef CH59xBLE_ROM
#include "CH59xBLE_ROM.h"
#else
#include "CH59xBLE_LIB.h"
#endif

#include "CH59x_common.h"

/* ***************************************************************************
【MAC】
BLE_MAC - Whether to customize the Bluetooth Mac address (default: FALSE - use the chip Mac address), you need to modify the Mac address definition in main.c

【DCDC】
DCDC_ENABLE - Whether to enable DCDC (default: FALSE)

【SLEEP】
HAL_SLEEP - Whether to enable sleep function (default: FALSE)
SLEEP_RTC_MIN_TIME - Minimum time to sleep in non-idle mode (unit: one RTC cycle)
SLEEP_RTC_MAX_TIME - Maximum time for sleep in non-idle mode (unit: one RTC cycle)
WAKE_UP_RTC_MAX_TIME - Waiting for the 32M crystal oscillator stabilization time (unit: one RTC cycle)
Values ​​can be divided into: Sleep mode/power-down mode - 45 (default)
Pause mode - 45
Idle mode - 5
【TEMPERATION】
TEM_SAMPLE - Whether to turn on the calibration function according to temperature changes, the single calibration takes less than 10ms (default: TRUE)

【CALIBRATION】
BLE_CALIBRATION_ENABLE - Whether to turn on the timing calibration function, the single calibration takes less than 10ms (default: TRUE)
BLE_CALIBRATION_PERIOD - The period of timed calibration, unit ms (default: 120000)

【SNV】
BLE_SNV - Whether to enable SNV function, it is used to store binding information (default: TRUE)
BLE_SNV_ADDR - SNV information save address, use data flash last 512 bytes (default: 0x77E00)
BLE_SNV_BLOCK - SNV information saving block size (default: 256)
BLE_SNV_NUM - Number of SNV information saved (default: 1)

【RTC】
CLK_OSC32K - RTC clock selection. If the host role is included, you must use external 32K (0 external (32768Hz), default: 1: internal (32000Hz), 2: internal (32768Hz))

【MEMORY】
BLE_MEMHEAP_SIZE - The RAM size used by the Bluetooth protocol stack is not less than 6K (default: (1024*6))

【DATA】
BLE_BUFF_MAX_LEN - Maximum packet length for a single connection (default: 27 (ATT_MTU=23), value range [27~516])
BLE_BUFF_NUM - Number of packets cached by the controller (default: 5)
BLE_TX_NUM_EVENT - How many packets can be sent for a single connection event (default: 1)
BLE_TX_POWER - Transmission power (default: LL_TX_POWEER_0_DBM (0dBm) )

【MULTICONN】
PERIPHERAL_MAX_CONNECTION - Up to how many slave roles can be done at the same time (default: 1)
CENTRAL_MAX_CONNECTION - Up to how many host roles can be used at the same time (default: 3)

************************************************************************ */

/* ***************************************************************************
* Default configuration value */
#ifndef BLE_MAC
#define BLE_MAC                             FALSE
#endif
#ifndef DCDC_ENABLE
#define DCDC_ENABLE                         TRUE
#endif
#ifndef HAL_SLEEP
#define HAL_SLEEP                           FALSE
#endif
#ifndef SLEEP_RTC_MIN_TIME                   
#define SLEEP_RTC_MIN_TIME                  US_TO_RTC(1000)
#endif
#ifndef SLEEP_RTC_MAX_TIME                   
#define SLEEP_RTC_MAX_TIME                  (RTC_MAX_COUNT - 1000 * 1000 * 30)
#endif
#ifndef WAKE_UP_RTC_MAX_TIME
#define WAKE_UP_RTC_MAX_TIME                US_TO_RTC(1600)
#endif
#ifndef HAL_KEY
#define HAL_KEY                             FALSE
#endif
#ifndef HAL_LED
#define HAL_LED                             FALSE
#endif
#ifndef TEM_SAMPLE
#define TEM_SAMPLE                          TRUE
#endif
#ifndef BLE_CALIBRATION_ENABLE
#define BLE_CALIBRATION_ENABLE              TRUE
#endif
#ifndef BLE_CALIBRATION_PERIOD
#define BLE_CALIBRATION_PERIOD              120000
#endif
#ifndef BLE_SNV
#define BLE_SNV                             TRUE
#endif
#ifndef BLE_SNV_ADDR
#define BLE_SNV_ADDR                        0x77E00-FLASH_ROM_MAX_SIZE
#endif
#ifndef BLE_SNV_BLOCK
#define BLE_SNV_BLOCK                       256
#endif
#ifndef BLE_SNV_NUM
#define BLE_SNV_NUM                         1
#endif
#ifndef CLK_OSC32K
#define CLK_OSC32K                          1   // Do not modify this item here. It must be modified in preprocessing in the project configuration. If the host role is included, it must use external 32K.
#endif
#ifndef BLE_MEMHEAP_SIZE
#define BLE_MEMHEAP_SIZE                    (1024*6)
#endif
#ifndef BLE_BUFF_MAX_LEN
#define BLE_BUFF_MAX_LEN                    27
#endif
#ifndef BLE_BUFF_NUM
#define BLE_BUFF_NUM                        5
#endif
#ifndef BLE_TX_NUM_EVENT
#define BLE_TX_NUM_EVENT                    1
#endif
#ifndef BLE_TX_POWER
#define BLE_TX_POWER                        LL_TX_POWEER_0_DBM
#endif
#ifndef PERIPHERAL_MAX_CONNECTION
#define PERIPHERAL_MAX_CONNECTION           1
#endif
#ifndef CENTRAL_MAX_CONNECTION
#define CENTRAL_MAX_CONNECTION              3
#endif


extern uint32_t MEM_BUF[BLE_MEMHEAP_SIZE / 4];
extern const uint8_t MacAddr[6];

#endif

