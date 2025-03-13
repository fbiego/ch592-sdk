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

/*********************************************************************
 MAC
 BLE_MAC                                    - Mac ( :FALSE - Mac )main.cMac

 DCDC
 DCDC_ENABLE                                - DCDC ( :FALSE )

 SLEEP
 HAL_SLEEP                                  -  ( :FALSE )
 SLEEP_RTC_MIN_TIME                         - RTC
 SLEEP_RTC_MAX_TIME                         - RTC
 WAKE_UP_RTC_MAX_TIME                       - 32MRTC
                                                                                                                             /  - 45 ()
                                                                                                                                                                                                      - 45
                                                                                                                                                                                                      - 5
 TEMPERATION
 TEM_SAMPLE                                 - 10ms( :TRUE )
 
 CALIBRATION
 BLE_CALIBRATION_ENABLE                     - 10ms( :TRUE )
 BLE_CALIBRATION_PERIOD                     - ms( :120000 )
 
 SNV
 BLE_SNV                                    - SNV( :TRUE )
 BLE_SNV_ADDR                               - SNVdata flash512( :0x77E00 )
 BLE_SNV_BLOCK                              - SNV( :256 )
 BLE_SNV_NUM                                - SNV( :1 )

 RTC
 CLK_OSC32K                                 - RTC32K( 0 (32768Hz):1(32000Hz)2(32768Hz) )

 MEMORY
 BLE_MEMHEAP_SIZE                           - RAM6K ( :(1024*6) )

 DATA
 BLE_BUFF_MAX_LEN                           - ( :27 (ATT_MTU=23)[27~516] )
 BLE_BUFF_NUM                               - ( :5 )
 BLE_TX_NUM_EVENT                           - ( :1 )
 BLE_TX_POWER                               - ( :LL_TX_POWEER_0_DBM (0dBm) )
 
 MULTICONN
 PERIPHERAL_MAX_CONNECTION                  - ( :1 )
 CENTRAL_MAX_CONNECTION                     - ( :3 )

 **********************************************************************/

/* ***************************************************************************
* Default configuration value */
#ifndef BLE_MAC
#define BLE_MAC                             FALSE
#endif
#ifndef DCDC_ENABLE
#define DCDC_ENABLE                         FALSE
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
#define BLE_SNV_ADDR                        0x77000-FLASH_ROM_MAX_SIZE
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

