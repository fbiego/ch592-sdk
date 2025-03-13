/* ********************************* (C) COPYRIGHT *******************************
* File Name          : Main.c
* Author             : WCH
* Version            : V1.0
* Date               : 2022/03/15
* Description        : USB IAPÀý³Ì
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
****************************************************************************** */

#include "CH59x_common.h"
#include "iap.h"

/* ***************************************************************************
* @fn SetSysClock
*
* @brief Configure the system running clock 60Mhz, 0x48
*
* @param sc - System clock source selection refer to SYS_CLKTypeDef
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
    sys_safe_access_enable();
    R32_CLK_SYS_CFG = (1 << 6) | (CLK_SOURCE_PLL_60MHz & 0x1f) | RB_TX_32M_PWR_EN | RB_PLL_PWR_EN;
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
* @fn Main_Circulation
*
* @brief The main loop of IAP, the program is put into ram to improve the speed.
*
* @param None.
*
* @return None. */
__attribute__((section(".highcode")))
void Main_Circulation()
{
    uint16_t j = 0;
    while (1)
    {
        j++;
        if (j > 5)// 100us processing data once
        {
            j = 0;
            USB_DevTransProcess();// Usb operation is performed using query method without interrupts.
        }
        DelayUs(20);
        g_tcnt++;
        if (g_tcnt > 3000000)
        {
            // No operation in 1 minute, enter the app
            R8_USB_CTRL = RB_UC_RESET_SIE;
            R16_PIN_ANALOG_IE &= ~(RB_PIN_USB_DP_PU | RB_PIN_USB_IE);         // Prevent the USB port from floating and pull-up resistor
            DelayMs(10);
            jumpApp();
        }
    }
}

IAPDataFlashInfo_t p_image_flash;

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

    mySetSysClock(); // In order to streamline the program size, this function has been modified than the initialization function of the ordinary library. You can only set the clock to 60M

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
            // Before starting, determine whether to enter IAP, no button press
            jumpApp();
        }
    }
#endif


    /* USB initialization */
    R8_USB_CTRL = 0x00; // Set the mode first, cancel RB_UC_CLR_ALL

    R8_UEP4_1_MOD = RB_UEP4_RX_EN | RB_UEP4_TX_EN | RB_UEP1_RX_EN | RB_UEP1_TX_EN; // Endpoint 4 OUT+IN, Endpoint 1 OUT+IN
    R8_UEP2_3_MOD = RB_UEP2_RX_EN | RB_UEP2_TX_EN | RB_UEP3_RX_EN | RB_UEP3_TX_EN; // Endpoint 2 OUT+IN, Endpoint 3 OUT+IN

    R16_UEP0_DMA = (uint16_t)(uint32_t)EP0_Databuf;
    R16_UEP1_DMA = (uint16_t)(uint32_t)EP1_Databuf;
    R16_UEP2_DMA = (uint16_t)(uint32_t)EP2_Databuf;
    R16_UEP3_DMA = (uint16_t)(uint32_t)EP3_Databuf;

    R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
    R8_UEP1_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK | RB_UEP_AUTO_TOG;
    R8_UEP2_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK | RB_UEP_AUTO_TOG;
    R8_UEP3_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK | RB_UEP_AUTO_TOG;
    R8_UEP4_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;

    R8_USB_DEV_AD = 0x00;
    R8_USB_CTRL = RB_UC_DEV_PU_EN | RB_UC_INT_BUSY | RB_UC_DMA_EN; // Start the USB device and DMA, and automatically return to NAK before the interrupt flag is not cleared during the interrupt period.
    R16_PIN_ANALOG_IE |= RB_PIN_USB_IE | RB_PIN_USB_DP_PU;         // Prevent the USB port from floating and pull-up resistor
    R8_USB_INT_FG = 0xFF;                                          // Clear interrupt sign
    R8_UDEV_CTRL = RB_UD_PD_DIS | RB_UD_PORT_EN;                   // Allow USB ports
    R8_USB_INT_EN = 0;// Prohibit USB interrupts and use query methods

    /* Enter the highcode main loop */
    Main_Circulation();
}
