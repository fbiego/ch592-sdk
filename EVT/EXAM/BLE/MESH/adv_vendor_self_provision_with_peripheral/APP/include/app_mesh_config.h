/********************************** (C) COPYRIGHT *******************************
 * File Name          : app_mesh_config.h
 * Author             : WCH
 * Version            : V1.1
 * Date               : 2021/11/18
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#ifndef APP_MESH_CONFIG_H
#define APP_MESH_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif
/* ******************************* Functional configuration, it is recommended to refer to different routines directly ********************************** */

// Relay function
#define CONFIG_BLE_MESH_RELAY                  1
// Agent Function
#define CONFIG_BLE_MESH_PROXY                  0
// GATT layer distribution network function
#define CONFIG_BLE_MESH_PB_GATT                0
// FLASH storage function
#define CONFIG_BLE_MESH_SETTINGS               1
// Friends node function
#define CONFIG_BLE_MESH_FRIEND                 0
// Low power node function
#define CONFIG_BLE_MESH_LOW_POWER              0
// config model client function
#define CONFIG_BLE_MESH_CFG_CLI                1
// health model client functions
#define CONFIG_BLE_MESH_HLTH_CLI               0

/* ************************************************************ */

// Net data cache number
#define CONFIG_MESH_ADV_BUF_COUNT_MIN          (6)
#define CONFIG_MESH_ADV_BUF_COUNT_DEF          (10)
#define CONFIG_MESH_ADV_BUF_COUNT_MAX          (256)

// The number of RPL data caches must not be less than the number of distribution device nodes supported by the distribution initiator.
#define CONFIG_MESH_RPL_COUNT_MIN              (6)
#define CONFIG_MESH_RPL_COUNT_DEF              (20)
#define CONFIG_MESH_RPL_COUNT_MAX              (128)

// RPL cache is used in a loop. After opening, the number of nodes in the network is not restricted by RPL, but NVS will not store RPL data.
#define CONFIG_MESH_ALLOW_RPL_CYCLE            (TRUE)

// IV Update State Timer Frequency Dividing Coefficient Based on 96H
#define CONFIG_MESH_IVU_DIVIDER_MIN            (1)
#define CONFIG_MESH_IVU_DIVIDER_DEF            (96)
#define CONFIG_MESH_IVU_DIVIDER_MAX            (96)

// Number of black and white list functions stored in agent
#define CONFIG_MESH_PROXY_FILTER_MIN           (2)
#define CONFIG_MESH_PROXY_FILTER_DEF           (5)
#define CONFIG_MESH_PROXY_FILTER_MAX           (20)

// Number of messages stored
#define CONFIG_MESH_MSG_CACHE_MIN              (3)
#define CONFIG_MESH_MSG_CACHE_DEF              (20)
#define CONFIG_MESH_MSG_CACHE_MAX              (20)

// Number of subnets
#define CONFIG_MESH_SUBNET_COUNT_MIN           (1)
#define CONFIG_MESH_SUBNET_COUNT_DEF           (1)
#define CONFIG_MESH_SUBNET_COUNT_MAX           (4)

// Number of keys of APP
#define CONFIG_MESH_APPKEY_COUNT_MIN           (1)
#define CONFIG_MESH_APPKEY_COUNT_DEF           (3)
#define CONFIG_MESH_APPKEY_COUNT_MAX           (5)

// Number of model keys that can be stored
#define CONFIG_MESH_MOD_KEY_COUNT_MIN          (1)
#define CONFIG_MESH_MOD_KEY_COUNT_DEF          (1)
#define CONFIG_MESH_MOD_KEY_COUNT_MAX          (3)

// Number of subscription addresses that can be stored
#define CONFIG_MESH_MOD_GROUP_COUNT_MIN        (1)
#define CONFIG_MESH_MOD_GROUP_COUNT_DEF        (6)
#define CONFIG_MESH_MOD_GROUP_COUNT_MAX        (64)

// Whether to allow nodes with the same address in a network (the subcontracting function is not available after enabling)
#define CONFIG_MESH_ALLOW_SAME_ADDR            (FALSE)

// The length supported by non-subcontract messages (this length requires subcontracting, the default value is 7, please note that all devices on the same network must be unified)
#define CONFIG_MESH_UNSEG_LENGTH_MIN           (7)
#define CONFIG_MESH_UNSEG_LENGTH_DEF           (221)
#define CONFIG_MESH_UNSEG_LENGTH_MAX           (221)

// Maximum number of subcontracts per message
#define CONFIG_MESH_TX_SEG_MIN                 (2)
#define CONFIG_MESH_TX_SEG_DEF                 (8)
#define CONFIG_MESH_TX_SEG_MAX                 (32)

// The maximum number of sub-packet messages allowed to be sent simultaneously
#define CONFIG_MESH_TX_SEG_COUNT_MIN           (1)
#define CONFIG_MESH_TX_SEG_COUNT_DEF           (2)
#define CONFIG_MESH_TX_SEG_COUNT_MAX           (4)

// The maximum number of subcontracted messages allowed to be received at the same time
#define CONFIG_MESH_RX_SEG_COUNT_MIN           (1)
#define CONFIG_MESH_RX_SEG_COUNT_DEF           (2)
#define CONFIG_MESH_RX_SEG_COUNT_MAX           (4)

// Maximum number of bytes per received subpackage message
#define CONFIG_MESH_RX_SDU_MIN                 (12)
#define CONFIG_MESH_RX_SDU_DEF                 (192)
#define CONFIG_MESH_RX_SDU_MAX                 (384)

// Number of virtual addresses
#define CONFIG_MESH_LABEL_COUNT_MIN            (1)
#define CONFIG_MESH_LABEL_COUNT_DEF            (2)
#define CONFIG_MESH_LABEL_COUNT_MAX            (4)

// NVS storage uses sectors
#define CONFIG_MESH_SECTOR_COUNT_MIN           (2)
#define CONFIG_MESH_SECTOR_COUNT_DEF           (3)

// NVS storage sector size
#define CONFIG_MESH_SECTOR_SIZE_DEF            (4096)

// NVS storage home address
#define CONFIG_MESH_NVS_ADDR_DEF               (0)

// How many times will RPL be updated to store
#define CONFIG_MESH_RPL_STORE_RATE_MIN         (5)
#define CONFIG_MESH_RPL_STORE_RATE_DEF         (60)
#define CONFIG_MESH_RPL_STORE_RATE_MAX         (3600)

// How many times will SEQ be stored after more than how many times
#define CONFIG_MESH_SEQ_STORE_RATE_MIN         (5)
#define CONFIG_MESH_SEQ_STORE_RATE_DEF         (60)
#define CONFIG_MESH_SEQ_STORE_RATE_MAX         (3600)

// The timeout timeout stored after other information is updated (s)
#define CONFIG_MESH_STORE_RATE_MIN             (2)
#define CONFIG_MESH_STORE_RATE_DEF             (2)
#define CONFIG_MESH_STORE_RATE_MAX             (5)

// The number of subcontracts for each message supported by the friend node
#define CONFIG_MESH_FRIEND_SEG_RX_COUNT_MIN    (1)
#define CONFIG_MESH_FRIEND_SEG_RX_COUNT_DEF    (2)
#define CONFIG_MESH_FRIEND_SEG_RX_COUNT_MAX    (4)

// Number of subscriptions supported by friend nodes
#define CONFIG_MESH_FRIEND_SUB_SIZE_MIN        (1)
#define CONFIG_MESH_FRIEND_SUB_SIZE_DEF        (4)
#define CONFIG_MESH_FRIEND_SUB_SIZE_MAX        (8)

// The number of low-power nodes supported by friend nodes
#define CONFIG_MESH_FRIEND_LPN_COUNT_MIN       (1)
#define CONFIG_MESH_FRIEND_LPN_COUNT_DEF       (1)
#define CONFIG_MESH_FRIEND_LPN_COUNT_MAX       (4)

// Message queue size stored by friend node
#define CONFIG_MESH_QUEUE_SIZE_MIN             (2)
#define CONFIG_MESH_QUEUE_SIZE_DEF             (4)
#define CONFIG_MESH_QUEUE_SIZE_MAX             (30)

// Friend node reception window size (ms)
#define CONFIG_MESH_FRIEND_RECV_WIN_MIN        (1)
#define CONFIG_MESH_FRIEND_RECV_WIN_DEF        (30)
#define CONFIG_MESH_FRIEND_RECV_WIN_MAX        (255)

// Request message queue size for low-power nodes, unit is an index of 2 (range: 2^2 - 2^4)
#define CONFIG_MESH_LPN_REQ_QUEUE_SIZE_MIN     (2)
#define CONFIG_MESH_LPN_REQ_QUEUE_SIZE_DEF     (2)
#define CONFIG_MESH_LPN_REQ_QUEUE_SIZE_MAX     (4)

// Request message interval for low-power nodes (100ms)
#define CONFIG_MESH_LPN_POLLINTERVAL_MIN       (1)
#define CONFIG_MESH_LPN_POLLINTERVAL_DEF       (80)
#define CONFIG_MESH_LPN_POLLINTERVAL_MAX       (3455999)

// Request message timeout timeout for low-power nodes (100ms)
#define CONFIG_MESH_LPN_POLLTIMEOUT_MIN        (10)
#define CONFIG_MESH_LPN_POLLTIMEOUT_DEF        (300)
#define CONFIG_MESH_LPN_POLLTIMEOUT_MAX        (3455999)

// Receive delay supported by low-power nodes (ms)
#define CONFIG_MESH_LPN_RECV_DELAY_MIN         (10)
#define CONFIG_MESH_LPN_RECV_DELAY_DEF         (100)
#define CONFIG_MESH_LPN_RECV_DELAY_MAX         (255)

// Waiting time for rebuilding friends (s)
#define CONFIG_MESH_RETRY_TIMEOUT_MIN          (3)
#define CONFIG_MESH_RETRY_TIMEOUT_DEF          (10)
#define CONFIG_MESH_RETRY_TIMEOUT_MAX          (60)

// Number of distribution network equipment nodes supported by distribution network initiator
#define CONFIG_MESH_PROV_NODE_COUNT_DEF        (0)

// ADV_RF configuration
#define CONFIG_MESH_RF_ACCESSADDRESS           (0x8E89BED6)
#define CONFIG_MESH_RF_CHANNEL_37              (3)
#define CONFIG_MESH_RF_CHANNEL_38              (16)
#define CONFIG_MESH_RF_CHANNEL_39              (34)

#define APP_DBG(X...)                     \
  if(1)                                   \
  {                                       \
    PRINT("APP_DBG %s> ", __FUNCTION__); \
    PRINT(X);                            \
    PRINT("\n");                         \
  }

/******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif
