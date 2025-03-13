/* ********************************* (C) COPYRIGHT *******************************
* File Name          : lwns_config.h
* Author             : WCH
* Version            : V1.0
* Date               : 2021/11/17
* Description        : lwns≈‰÷√—°œÓ
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
****************************************************************************** */
#ifndef _LWNS_CONFIG_H_
#define _LWNS_CONFIG_H_

#include "config.h"
#include "CH59x_common.h"
#include "WCH_LWNS_LIB.h"

#define LWNS_ADDR_USE_BLE_MAC     1   // Whether to use the mac address of Bluetooth hardware as the default lwns address

#define LWNS_ENCRYPT_ENABLE       1   // Whether to enable encryption

#define QBUF_MANUAL_NUM           4   // qbuf cache quantity configuration

#define ROUTE_ENTRY_MANUAL_NUM    32  // Number of routing entries configuration

#define LWNS_NEIGHBOR_MAX_NUM     8   // Maximum number of neighbors

#endif /* _LWNS_CONFIG_H_ */
