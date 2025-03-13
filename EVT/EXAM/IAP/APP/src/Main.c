/* ********************************* (C) COPYRIGHT *******************************
* File Name          : Main.c
* Author             : WCH
* Version            : V1.0
* Date               : 2022/03/15
* Description        : USB IAP APPÀý³Ì
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
****************************************************************************** */

#include "CH59x_common.h"
#include "app_flag.h"

/* ***************************************************************************
* @fn DebugInit
*
* @brief debug initialization
*
* @return none */
void DebugInit(void)
{
    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
}

/* ***************************************************************************
* @fn main
*
* @brief main function
*
* @return none */
int main()
{
    uint16_t i = 0;
    uint8_t  s = 0;

    SetSysClock(CLK_SOURCE_PLL_60MHz);

    /* Configure serial debugging */
    DebugInit();
    PRINT("Start @ChipID=%02x\n", R8_CHIP_ID);
    /* The app program must execute this statement to ensure that when the app update fails, the IAP will still be run next time. */
    SwitchImageFlag(FLAG_USER_CALL_APP);

    GPIOB_ModeCfg(GPIO_Pin_4, GPIO_ModeIN_PU);
    while (1)
    {
		PRINT("i:%d\n",i);
		i++;
		DelayMs(10);
		if (GPIOB_ReadPortPin(GPIO_Pin_4) == 0)
		{
			s++;
			// Two consecutive key presses are detected and jump to IAP
			if(s >= 2)
			{
				jumpToIap();
			}
		}
		else
		{
			s = 0;
		}
		DelayMs(100);
    }
}
