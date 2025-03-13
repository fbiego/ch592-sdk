/********************************** (C) COPYRIGHT *******************************
 * File Name          : CH59x_gpio.c
 * Author             : WCH
 * Version            : V1.2
 * Date               : 2021/11/17
 * Description
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#include "CH59x_common.h"

/* ***************************************************************************
* @fn GPIOA_ModeCfg
*
* @brief GPIOA port pin mode configuration
*
* @param pin - PA0-PA15
* @param mode - Input and output type
*
* @return none */
void GPIOA_ModeCfg(uint32_t pin, GPIOModeTypeDef mode)
{
    switch(mode)
    {
        case GPIO_ModeIN_Floating:
            R32_PA_PD_DRV &= ~pin;
            R32_PA_PU &= ~pin;
            R32_PA_DIR &= ~pin;
            break;

        case GPIO_ModeIN_PU:
            R32_PA_PD_DRV &= ~pin;
            R32_PA_PU |= pin;
            R32_PA_DIR &= ~pin;
            break;

        case GPIO_ModeIN_PD:
            R32_PA_PD_DRV |= pin;
            R32_PA_PU &= ~pin;
            R32_PA_DIR &= ~pin;
            break;

        case GPIO_ModeOut_PP_5mA:
            R32_PA_PD_DRV &= ~pin;
            R32_PA_DIR |= pin;
            break;

        case GPIO_ModeOut_PP_20mA:
            R32_PA_PD_DRV |= pin;
            R32_PA_DIR |= pin;
            break;

        default:
            break;
    }
}

/* ***************************************************************************
* @fn GPIOB_ModeCfg
*
* @brief GPIOB port pin mode configuration
*
* @param pin - PB0-PB23
* @param mode - Input and output type
*
* @return none */
void GPIOB_ModeCfg(uint32_t pin, GPIOModeTypeDef mode)
{
    switch(mode)
    {
        case GPIO_ModeIN_Floating:
            R32_PB_PD_DRV &= ~pin;
            R32_PB_PU &= ~pin;
            R32_PB_DIR &= ~pin;
            break;

        case GPIO_ModeIN_PU:
            R32_PB_PD_DRV &= ~pin;
            R32_PB_PU |= pin;
            R32_PB_DIR &= ~pin;
            break;

        case GPIO_ModeIN_PD:
            R32_PB_PD_DRV |= pin;
            R32_PB_PU &= ~pin;
            R32_PB_DIR &= ~pin;
            break;

        case GPIO_ModeOut_PP_5mA:
            R32_PB_PD_DRV &= ~pin;
            R32_PB_DIR |= pin;
            break;

        case GPIO_ModeOut_PP_20mA:
            R32_PB_PD_DRV |= pin;
            R32_PB_DIR |= pin;
            break;

        default:
            break;
    }
}

/* ***************************************************************************
* @fn GPIOA_ITModeCfg
*
* @brief GPIOA pin interrupt mode configuration
*
* @param pin - PA0-PA15
* @param mode - Trigger type
*
* @return none */
void GPIOA_ITModeCfg(uint32_t pin, GPIOITModeTpDef mode)
{
    switch(mode)
    {
        case GPIO_ITMode_LowLevel: // Low level trigger
            R16_PA_INT_MODE &= ~pin;
            R32_PA_CLR |= pin;
            break;

        case GPIO_ITMode_HighLevel: // High level trigger
            R16_PA_INT_MODE &= ~pin;
            R32_PA_OUT |= pin;
            break;

        case GPIO_ITMode_FallEdge: // Falling edge trigger
            R16_PA_INT_MODE |= pin;
            R32_PA_CLR |= pin;
            break;

        case GPIO_ITMode_RiseEdge: // Rising edge trigger
            R16_PA_INT_MODE |= pin;
            R32_PA_OUT |= pin;
            break;

        default:
            break;
    }
    R16_PA_INT_IF = pin;
    R16_PA_INT_EN |= pin;
}

/* ***************************************************************************
* @fn GPIOB_ITModeCfg
*
* @brief GPIOB pin interrupt mode configuration
*
* @param pin - PB0-PB23
* @param mode - Trigger type
*
* @return none */
void GPIOB_ITModeCfg(uint32_t pin, GPIOITModeTpDef mode)
{
    uint32_t Pin = pin | ((pin & (GPIO_Pin_22 | GPIO_Pin_23)) >> 14);
    switch(mode)
    {
        case GPIO_ITMode_LowLevel: // Low level trigger
            R16_PB_INT_MODE &= ~Pin;
            R32_PB_CLR |= pin;
            break;

        case GPIO_ITMode_HighLevel: // High level trigger
            R16_PB_INT_MODE &= ~Pin;
            R32_PB_OUT |= pin;
            break;

        case GPIO_ITMode_FallEdge: // 
            R16_PB_INT_MODE |= Pin;
            R32_PB_CLR |= pin;
            break;

        case GPIO_ITMode_RiseEdge: // Rising edge trigger
            R16_PB_INT_MODE |= Pin;
            R32_PB_OUT |= pin;
            break;

        default:
            break;
    }
    R16_PB_INT_IF = Pin;
    R16_PB_INT_EN |= Pin;
}

/* ***************************************************************************
* @fn GPIOPinRemap
*
* @brief Peripheral Function Pin Mapping
*
* @param s - Whether to enable mapping
* @param perph - RB_RF_ANT_SW_EN - RF antenna switch control output on PA4/PA5/PA12/PA13/PA14/PA15
* RB_PIN_U0_INV - RXD0/RXD0_/TXD0/TXD0_ invert input/output
* RB_PIN_INTX - INTX: INT24/INT25 PB8/PB9 -> INT24_/INT25_ PB22/PB23
* RB_PIN_MODEM - MODEM: PA6/PA7 -> PB12/PB13
* RB_PIN_I2C - I2C: PB14/PB15 -> PB14/PB15
* RB_PIN_PWMX - PWMX: PA12/PA13 -> PA6/PA7
* RB_PIN_SPI0 - SPI0: PA12/PA13/PA14/PA15 -> PB12/PB13/PB14/PB15
* RB_PIN_UART3 - UART3: PA4/PA5 -> PA4/PA5
* RB_PIN_UART2 - UART2: PB22/PB23 -> PA6/PA7
* RB_PIN_UART1 - UART1: PA8/PA9 -> PB12/PB13
* RB_PIN_UART0 - UART0: PB4/PB7 -> PA15/PA14
* RB_PIN_TMR3 - TMR2: PB22 -> PB22
* RB_PIN_TMR2 - TMR2: PA11 -> PB11
* RB_PIN_TMR1 - TMR1: PA10 -> PB10
* RB_PIN_TMR0 - TMR0: PA9 -> PB23
*
* @return none */
void GPIOPinRemap(FunctionalState s, uint16_t perph)
{
    if(s)
    {
        R16_PIN_ALTERNATE |= perph;
    }
    else
    {
        R16_PIN_ALTERNATE &= ~perph;
    }
}

/* ***************************************************************************
* @fn GPIOAGPPCfg
*
* @brief Analog peripheral GPIO pin function control
*
* @param s - ENABLE - Turn on analog peripheral function and turn off digital function
* DISABLE - Enable digital function and turn off analog peripheral function
* @param perph - RB_PIN_ADC8_9_IE - ADC/TKEY 9/8 channel
* RB_PIN_ADC6_7_IE - ADC/TKEY 7/6 channel
* RB_PIN_ADC10_IE - ADC/TKEY 10 channel
* RB_PIN_ADC11_IE - ADC/TKEY 11 channel
* RB_PIN_USB2_DP_PU - USB2 U2D+ pin internal pull-up resistor
* RB_PIN_USB2_IE - USB2 pin
* RB_PIN_USB_DP_PU - USB UD+ pin internal pull-up resistor
* RB_PIN_USB_IE - USB pin
* RB_PIN_ADC0_IE - ADC/TKEY 0 Channel
* RB_PIN_ADC1_IE - ADC/TKEY 1 channel
* RB_PIN_ADC12_IE - ADC/TKEY 12 channel
* RB_PIN_ADC13_IE - ADC/TKEY 13 Channel
* RB_PIN_XT32K_IE - 32KHz crystal oscillator LSE pin
* RB_PIN_ADC2_3_IE - ADC/TKEY 2/3 Channel
* RB_PIN_ADC4_5_IE - ADC/TKEY 4/5 Channel
*
* @return none */
void GPIOAGPPCfg(FunctionalState s, uint16_t perph)
{
    if(s)
    {
        R16_PIN_ANALOG_IE |= perph;
    }
    else
    {
        R16_PIN_ANALOG_IE &= ~perph;
    }
}
