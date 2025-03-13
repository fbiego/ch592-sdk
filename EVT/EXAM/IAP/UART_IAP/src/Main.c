/* ********************************* (C) COPYRIGHT *******************************
* File Name          : Main.c
* Author             : WCH
* Version            : V1.0
* Date               : 2020/08/06
* Description        : FALSH¶ÁÐ´Àý³Ì
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
****************************************************************************** */

#include "CH59x_common.h"
#include "iap.h"

IAPDataFlashInfo_t p_image_flash;

/* ***************************************************************************
* @fn mySetSysClock
*
* @brief Configure the system running clock 60Mhz, 0x48
*
* @param none
*
* @return none */
__HIGH_CODE
void mySetSysClock()
{
    uint32_t i;
    sys_safe_access_enable();
    R8_PLL_CONFIG &= ~(1 << 5); //
    sys_safe_access_disable();
    // PLL div
    if(!(R32_CLK_SYS_CFG & RB_PLL_PWR_EN))
    {
        sys_safe_access_enable();
        R32_CLK_SYS_CFG |= RB_PLL_PWR_EN; // PLL power on
        sys_safe_access_disable();
        for(i = 0; i < 2000; i++)
        {
            __nop();
            __nop();
        }
    }
    sys_safe_access_enable();
    R32_CLK_SYS_CFG = (R32_CLK_SYS_CFG & 0xFF0000) | (1 << 6) | (CLK_SOURCE_PLL_60MHz & 0x1f);
    __nop();
    __nop();
    __nop();
    __nop();
    sys_safe_access_disable();
    sys_safe_access_enable();
    R8_FLASH_CFG = 0X52;
    sys_safe_access_disable();
    // Change the drive capability of FLASH clk
    sys_safe_access_enable();
    R8_PLL_CONFIG |= 1 << 7;
    sys_safe_access_disable();
}

/* ***************************************************************************
* @fn main
*
* @brief main function
*
* @return none */
int main()
{
    uint16_t i;
    uint8_t  s;

    mySetSysClock();

#if USE_EEPROM_FLAG
    EEPROM_READ(IAP_FLAG_DATAFLASH_ADD, &p_image_flash, 4);
    if ((p_image_flash.ImageFlag != FLAG_USER_CALL_IAP))
    {
        jumpApp();
    }
#else
    // The initialization pin is a pull-up input.To reduce the size of the program, it is written in registers.
    R32_PB_PD_DRV &= ~GPIO_Pin_4;
    R32_PB_PU |= GPIO_Pin_4;
    R32_PB_DIR &= ~GPIO_Pin_4;
    //
    DelayMs(10);
    if (GPIOB_ReadPortPin(GPIO_Pin_4))
    {
        DelayMs(5);
        if (GPIOB_ReadPortPin(GPIO_Pin_4))
        {
            // Before starting, determine whether to enter IAP
            jumpApp();
        }
    }
#endif

    /* Initialize uart and change it to your own uart as needed */
    GPIOA_SetBits( bTXD1 );

    /* To save code space, use registers as much as possible initing initialization */
    R32_PA_PD_DRV &= ((~bTXD1) & (~bRXD1));
    /* GPIOA_ModeCfg( bTXD1, GPIO_ModeOut_PP_5mA ); */
    //R32_PA_PD_DRV &= ~bTXD1;
    R32_PA_DIR    |= bTXD1;

    /* GPIOA_ModeCfg(bRXD1, GPIO_ModeIN_PU); */
    //R32_PA_PD_DRV &= ~bRXD1;
    R32_PA_PU     |= bRXD1;
    R32_PA_DIR    &= ~bRXD1;

    UART1_BaudRateCfg( 115200 );
    R8_UART1_FCR = (2<<6) | RB_FCR_TX_FIFO_CLR | RB_FCR_RX_FIFO_CLR | RB_FCR_FIFO_EN;   // FIFO is turned on, trigger point 4 bytes
    R8_UART1_LCR = RB_LCR_WORD_SZ;
    R8_UART1_IER = RB_IER_TXD_EN;
    R8_UART1_DIV = 1;

    Main_Circulation();
}
