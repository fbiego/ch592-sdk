/* ********************************* (C) COPYRIGHT ***************************
* File Name : main.c
* Author: WCH
* Version: V1.1
* Date: 2024/02/27
* Description: Peripheral slave application master function and task system initialization
* Copyright (c) 2024 Nanjing Qinheng Microelectronics Co., Ltd.
* SPDX-License-Identifier: Apache-2.0
********************************************************************************************* */

/******************************************************************************/
/* The header file contains */
#include "Touch.h"
#include "app.h"

/*********************************************************************
 * GLOBAL TYPEDEFS
 */

/* ***************************************************************************
* @fn DebugInit
*
* @brief serial port printing initialization
*
* @return none
*
* @note Uart1's default mapping pin may conflict with the touch channel,
* In touch applications, other UARTs should be selected or pin remapping */
void DebugInit(void)
{
#ifdef  DEBUG
#if DEBUG == Debug_UART0
	GPIOB_SetBits(GPIO_Pin_7);
	GPIOB_ModeCfg(GPIO_Pin_7, GPIO_ModeOut_PP_5mA);
	UART0_DefInit();
#elif DEBUG == Debug_UART1
	GPIOPinRemap(ENABLE, RB_PIN_UART1);
	GPIOB_SetBits(GPIO_Pin_13);
	GPIOB_ModeCfg(GPIO_Pin_13, GPIO_ModeOut_PP_5mA);
	UART1_DefInit();
#elif DEBUG == Debug_UART2
	GPIOPinRemap(ENABLE, RB_PIN_UART2);
	GPIOB_SetBits(GPIO_Pin_23);
	GPIOB_ModeCfg(GPIO_Pin_23, GPIO_ModeOut_PP_5mA);
	UART2_DefInit();
#elif DEBUG == Debug_UART3
	GPIOPinRemap(ENABLE, RB_PIN_UART3);
	GPIOB_SetBits(GPIO_Pin_21);
	GPIOB_ModeCfg(GPIO_Pin_21, GPIO_ModeOut_PP_5mA);
	UART3_DefInit();
#endif
#endif
}

/* ***************************************************************************
* @fn main
*
* @brief main function
*
* @return none */
int main(void)
{
	SetSysClock(CLK_SOURCE_PLL_60MHz);
	DebugInit();
	PRINT("touch wheel slider\n");
    touch_init();
	while(1)
	{
		touch_dataProcess();
	}
}

/******************************** endfile @ main ******************************/
