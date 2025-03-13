/* ********************************* (C) COPYRIGHT ***************************
* File Name: Main.c
* Author: WCH
* Version: V1.0
* Date: 2020/08/06
* Description: System sleep mode and wake up demonstration: GPIOA_5 is used as the wake-up source, with a total of 4 sleep levels
************************************************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
********************************************************************************************* */

/* Note: Switch to the HSE clock source, the required waiting stability time is related to the selected external crystal parameters. It is best to choose a new crystal. The crystal provided by the manufacturer and its
Load capacitance parameter value.By configuring the R8_XT32M_TUNE register, different load capacitances and bias currents can be configured to adjust the crystal stability time. */

#include "CH59x_common.h"

/* ***************************************************************************
* @fn DebugInit
*
* @brief debug initialization
*
* @return none */
void DebugInit(void)
{
    GPIOA_SetBits(GPIO_Pin_9);
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
    SetSysClock(CLK_SOURCE_PLL_60MHz);
    PWR_DCDCCfg(ENABLE);
    GPIOA_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);
    GPIOB_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);

    /* Configure serial debugging */
    DebugInit();
    PRINT("Start @ChipID=%02x\n", R8_CHIP_ID);
    DelayMs(200);

#if 1
    /* Configure wakeup source as GPIO - PA5 */
    GPIOA_ModeCfg(GPIO_Pin_5, GPIO_ModeIN_PU);
    GPIOA_ITModeCfg(GPIO_Pin_5, GPIO_ITMode_FallEdge); // Wake up on the falling edge
    PFIC_EnableIRQ(GPIO_A_IRQn);
    PWR_PeriphWakeUpCfg(ENABLE, RB_SLP_GPIO_WAKE, Long_Delay);
#endif

#if 1
    PRINT("IDLE mode sleep \n");
    DelayMs(1);
    LowPower_Idle();
    PRINT("wake.. \n");
    DelayMs(500);
#endif

#if 1
    PRINT("Halt mode sleep \n");
    DelayMs(2);
    LowPower_Halt();
    HSECFG_Current(HSE_RCur_100); // Reduced to rated current (HSE bias current is increased in low power consumption function)
    DelayMs(2);
    PRINT("wake.. \n");
    DelayMs(500);
#endif

#if 1
    PRINT("sleep mode sleep \n");
    DelayMs(2);
    // Note that when the main frequency is 80M, the flash code cannot be called when the Sleep wake-up interrupt is interrupted.
    LowPower_Sleep(RB_PWR_RAM24K | RB_PWR_RAM2K | RB_XT_PRE_EN); // Only 24+2K SRAM power supply is retained
    HSECFG_Current(HSE_RCur_100);                 // Reduced to rated current (HSE bias current is increased in low power consumption function)
    PRINT("wake.. \n");
    DelayMs(500);
#endif

#if 1
    PRINT("shut down mode sleep \n");
    DelayMs(2);
    LowPower_Shutdown(0); // All power off, reset after wake-up
    /* Reset will be performed after this mode wakes up, so the following code will not run.
Be careful to make sure that the system sleeps and wakes up before wakes up, otherwise it may become IDLE level wake-up. */
    HSECFG_Current(HSE_RCur_100); // Reduced to rated current (HSE bias current is increased in low power consumption function)
    PRINT("wake.. \n");
    DelayMs(500);
#endif

    while(1)
        ;
}

/* ***************************************************************************
* @fn GPIOA_IRQHandler
*
* @brief GPIOA interrupt function
*
* @return none */
__INTERRUPT
__HIGH_CODE
void GPIOA_IRQHandler(void)
{
    GPIOA_ClearITFlagBit(GPIO_Pin_6 | GPIO_Pin_5);
}
