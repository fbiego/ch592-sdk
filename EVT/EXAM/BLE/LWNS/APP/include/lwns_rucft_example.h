/* ********************************* (C) COPYRIGHT ***************************
* File Name: lwns_rucft_example.h
* Author: WCH
* Version: V1.0
* Date: 2021/06/30
* Description: reliable unicast file transfer, reliable unicast file transfer example
************************************************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
********************************************************************************************* */
#ifndef _LWNS_RUCFT_EXAMPLE_H_
#define _LWNS_RUCFT_EXAMPLE_H_

#include "lwns_config.h"

#define RUCFT_EXAMPLE_TX_PERIOD_EVT    1 << (0)

void lwns_rucft_process_init(void);

#endif /* _LWNS_RUCFT_EXAMPLE_H_ */
