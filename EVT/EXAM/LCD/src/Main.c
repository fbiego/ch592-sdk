/* ********************************* (C) COPYRIGHT *******************************
* File Name          : Main.c
* Author             : WCH
* Version            : V1.0
* Date               : 2023/02/24
* Description        : LCDÑÝÊ¾
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
****************************************************************************** */

#include "CH59x_common.h"
#include "CH59x_lcd.h"

uint16_t aux_power;
unsigned char const lcd[10]={0x7d, 0x60, 0x3e, 0x7a, 0x63, 0x5b, 0x5f, 0x70, 0x03, 0x9b};
/*     A
     |----|
    F|    |B
     |-G--|
    E|    |C
     |----| .P
       D
*/
/* Note: Using this routine, the external manual reset function must be turned off when downloading. */
int main()
{
    uint32_t VER = 0;

    SetSysClock(CLK_SOURCE_PLL_60MHz);
    LCD_Init(LCD_1_4_Duty, LCD_1_3_Bias);

    LCD_WriteData0( lcd[0] );
    LCD_WriteData1( lcd[1] );
    LCD_WriteData2( lcd[2] );
    LCD_WriteData3( lcd[3] );
    LCD_WriteData4( lcd[4] );
    LCD_WriteData5( lcd[5] );
    LCD_WriteData6( lcd[6] );
    LCD_WriteData7( lcd[7] );

    LCD_WriteData8( lcd[8] );


    /* LCD + sleep example */
#if 1
    /* Configure wakeup source as GPIO - PB0 */
    GPIOB_ModeCfg(GPIO_Pin_0, GPIO_ModeIN_PU);
    GPIOB_ITModeCfg(GPIO_Pin_0, GPIO_ITMode_FallEdge); // Wake up on the falling edge
    PFIC_EnableIRQ(GPIO_B_IRQn);
    PWR_PeriphWakeUpCfg(ENABLE, RB_SLP_GPIO_WAKE, Long_Delay);
    VER = (*((PUINT32)ROM_CFG_VERISON));
    if((VER&0xFF000000) == 0xFF000000)
    {
        aux_power = R16_AUX_POWER_ADJ;
        sys_safe_access_enable();
        R16_AUX_POWER_ADJ |= RB_ULPLDO_ADJ;      // This code must be added before sleeping
        sys_safe_access_disable();
    }
    // Note that when the main frequency is 80M, the flash code cannot be called when the Sleep wake-up interrupt is interrupted.
    LowPower_Sleep(RB_PWR_RAM24K | RB_PWR_RAM2K | RB_XT_PRE_EN); // Only 24+2K SRAM power supply is retained
    HSECFG_Current(HSE_RCur_100);                 // Reduced to rated current (HSE bias current is increased in low power consumption function)
    if((VER&0xFF000000) == 0xFF000000)
    {
        sys_safe_access_enable();
        R16_AUX_POWER_ADJ = aux_power;
        sys_safe_access_disable();
    }
#endif

    while(1);

}

/* ***************************************************************************
* @fn GPIOB_IRQHandler
*
* @brief GPIOB interrupt function
*
* @return none */
__INTERRUPT
__HIGH_CODE
void GPIOB_IRQHandler(void)
{
    GPIOB_ClearITFlagBit(GPIO_Pin_0);
}
