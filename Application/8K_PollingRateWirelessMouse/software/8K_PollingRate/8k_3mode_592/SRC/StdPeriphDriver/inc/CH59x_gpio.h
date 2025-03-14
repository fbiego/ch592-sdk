/********************************** (C) COPYRIGHT *******************************
 * File Name          : CH59x_gpio.h
 * Author             : WCH
 * Version            : V1.2
 * Date               : 2021/11/17
 * Description
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#ifndef __CH59x_GPIO_H__
#define __CH59x_GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief	GPIO_pins_define
 */
#define GPIO_Pin_0      (0x00000001) /*!< Pin 0 selected */
#define GPIO_Pin_1      (0x00000002) /*!< Pin 1 selected */
#define GPIO_Pin_2      (0x00000004) /*!< Pin 2 selected */
#define GPIO_Pin_3      (0x00000008) /*!< Pin 3 selected */
#define GPIO_Pin_4      (0x00000010) /*!< Pin 4 selected */
#define GPIO_Pin_5      (0x00000020) /*!< Pin 5 selected */
#define GPIO_Pin_6      (0x00000040) /*!< Pin 6 selected */
#define GPIO_Pin_7      (0x00000080) /*!< Pin 7 selected */
#define GPIO_Pin_8      (0x00000100) /*!< Pin 8 selected */
#define GPIO_Pin_9      (0x00000200) /*!< Pin 9 selected */
#define GPIO_Pin_10     (0x00000400) /*!< Pin 10 selected */
#define GPIO_Pin_11     (0x00000800) /*!< Pin 11 selected */
#define GPIO_Pin_12     (0x00001000) /*!< Pin 12 selected */
#define GPIO_Pin_13     (0x00002000) /*!< Pin 13 selected */
#define GPIO_Pin_14     (0x00004000) /*!< Pin 14 selected */
#define GPIO_Pin_15     (0x00008000) /*!< Pin 15 selected */
#define GPIO_Pin_16     (0x00010000) /*!< Pin 16 selected */
#define GPIO_Pin_17     (0x00020000) /*!< Pin 17 selected */
#define GPIO_Pin_18     (0x00040000) /*!< Pin 18 selected */
#define GPIO_Pin_19     (0x00080000) /*!< Pin 19 selected */
#define GPIO_Pin_20     (0x00100000) /*!< Pin 20 selected */
#define GPIO_Pin_21     (0x00200000) /*!< Pin 21 selected */
#define GPIO_Pin_22     (0x00400000) /*!< Pin 22 selected */
#define GPIO_Pin_23     (0x00800000) /*!< Pin 23 selected */
#define GPIO_Pin_All    (0xFFFFFFFF) /*!< All pins selected */

/**
 * @brief  Configuration GPIO Mode
 */
typedef enum
{
    GPIO_ModeIN_Floating, // Floating input
    GPIO_ModeIN_PU,       // Pull-up input
    GPIO_ModeIN_PD,       // Pull down input
    GPIO_ModeOut_PP_5mA,  // Push-pull output maximum 5mA
    GPIO_ModeOut_PP_20mA, // Push-pull output maximum 20mA

} GPIOModeTypeDef;

/**
 * @brief  Configuration GPIO IT Mode
 */
typedef enum
{
    GPIO_ITMode_LowLevel,  // Low level trigger
    GPIO_ITMode_HighLevel, //高电平触发
    GPIO_ITMode_FallEdge,  // Falling edge trigger
    GPIO_ITMode_RiseEdge,  // Rising edge trigger

} GPIOITModeTpDef;

/* *
* @brief GPIOA port pin mode configuration
*
* @param pin - PA0-PA15
* @param mode - Input and output type */
void GPIOA_ModeCfg(uint32_t pin, GPIOModeTypeDef mode);

/* *
* @brief GPIOB port pin mode configuration
*
* @param pin - PB0-PB23
* @param mode - Input and output type */
void GPIOB_ModeCfg(uint32_t pin, GPIOModeTypeDef mode);

/* *
* @brief GPIOA port pin output is set low
*
* @param pin - PA0-PA15 */
#define GPIOA_ResetBits(pin)      (R32_PA_CLR |= pin)

/**
 * @brief   GPIOA端口引脚输出置高
 *
 * @param   pin     - PA0-PA15
 */
#define GPIOA_SetBits(pin)        (R32_PA_OUT |= pin)

/* *
* @brief GPIOB port pin output is set low
*
* @param pin - PB0-PB23 */
#define GPIOB_ResetBits(pin)      (R32_PB_CLR |= pin)

/* *
* @brief GPIOB port pin output set high
*
* @param pin - PB0-PB23 */
#define GPIOB_SetBits(pin)        (R32_PB_OUT |= pin)

/* *
* @brief GPIOA port pin output level flip
*
* @param pin - PA0-PA15 */
#define GPIOA_InverseBits(pin)    (R32_PA_OUT ^= pin)

/* *
* @brief GPIOB port pin output level flip
*
* @param pin - PB0-PB23 */
#define GPIOB_InverseBits(pin)    (R32_PB_OUT ^= pin)

/* *
* @brief GPIOA port 32-bit data returns, the lower 16-bit valid
*
* @return GPIOA port 32-bit data */
#define GPIOA_ReadPort()          (R32_PA_PIN)

/* *
* @brief GPIOB port 32-bit data returns, the lower 24-bit valid
*
* @return GPIOB port 32-bit data */
#define GPIOB_ReadPort()          (R32_PB_PIN)

/* *
* @brief GPIOA port pin status, 0-pin low level, (!0)-pin high level
*
* @param pin - PA0-PA15
*
* @return GPIOA port pin status */
#define GPIOA_ReadPortPin(pin)    (R32_PA_PIN & (pin))

/* *
* @brief GPIOB port pin status, 0-pin low level, (!0)-pin high level
*
* @param pin - PB0-PB23
*
* @return GPIOB port pin status */
#define GPIOB_ReadPortPin(pin)    (R32_PB_PIN & (pin))

/* *
* @brief GPIOA pin interrupt mode configuration
*
* @param pin - PA0-PA15
* @param mode - Trigger type */
void GPIOA_ITModeCfg(uint32_t pin, GPIOITModeTpDef mode);

/* *
* @brief GPIOB pin interrupt mode configuration
*
* @param pin - PB0-PB23
* @param mode - Trigger type */
void GPIOB_ITModeCfg(uint32_t pin, GPIOITModeTpDef mode);

/* *
* @brief Read the status of the GPIOA port interrupt flag
*
* @return GPIOA port interrupt flag status */
#define GPIOA_ReadITFlagPort()       (R16_PA_INT_IF)

/* *
* @brief Read the status of the GPIOB port interrupt flag
*
* @return GPIOB port interrupt flag status */
#define GPIOB_ReadITFlagPort()       ((R16_PB_INT_IF & (~((GPIO_Pin_22 | GPIO_Pin_23) >> 14))) | ((R16_PB_INT_IF << 14) & (GPIO_Pin_22 | GPIO_Pin_23)))

/* *
* @brief Read the status of the GPIOA port pin interrupt flag
*
* @param pin - PA0-PA15
*
* @return GPIOA port pin interrupt flag status */
#define GPIOA_ReadITFlagBit(pin)     (R16_PA_INT_IF & (pin))

/* *
* @brief Read the status of the GPIOB port pin interrupt flag
*
* @param pin - PB0-PB23
*
* @return GPIOB port pin interrupt flag status */
#define GPIOB_ReadITFlagBit(pin)     (R16_PB_INT_IF & ((pin) | (((pin) & (GPIO_Pin_22 | GPIO_Pin_23)) >> 14)))

/* *
* @brief Clear GPIOA port pin interrupt flag status
*
* @param pin - PA0-PA15 */
#define GPIOA_ClearITFlagBit(pin)    (R16_PA_INT_IF = pin)

/* *
* @brief Clear GPIOB port pin interrupt flag status
*
* @param pin - PB0-PB23 */
#define GPIOB_ClearITFlagBit(pin)    (R16_PB_INT_IF = ((pin) | (((pin) & (GPIO_Pin_22 | GPIO_Pin_23)) >> 14)))

/* *
* @brief Peripheral Function Pin Mapping
*
* @param s - Whether to enable mapping
* @param perph - RB_RF_ANT_SW_EN - RF antenna switch control output on PB16/PB17/PB18/PB19/PB20/PB21
* RB_PIN_U0_INV - RXD0/RXD0_/TXD0/TXD0_ invert input/output
* RB_PIN_INTX - INTX: INT24/INT25 PB8/PB9 -> INT24_/INT25_ PB22/PB23
* RB_PIN_MODEM - MODEM: PB1/PB5 -> PB14/PB15
* RB_PIN_I2C - I2C: PB13/PB12 -> PB21/PB20
* RB_PIN_PWMX - PWMX: PA12/PA13/PB4/PB6/PB7 -> PA6/PA7/PB1/PB2/PB3
* RB_PIN_SPI0 - SPI0: PA12/PA13/PA14/PA15 -> PB12/PB13/PB14/PB15
* RB_PIN_UART3 - UART3: PA4/PA5 -> PB20/PB21
* RB_PIN_UART2 - UART2: PA6/PA7 -> PB22/PB23
* RB_PIN_UART1 - UART1: PA8/PA9 -> PB12/PB13
* RB_PIN_UART0 - UART0: PB4/PB7 -> PA15/PA14
* RB_PIN_TMR3 - TMR2: PA9 -> PB23
* RB_PIN_TMR2 - TMR2: PA11 -> PB11
* RB_PIN_TMR1 - TMR1: PA10 -> PB10
* RB_PIN_TMR0 - TMR0: PA9 -> PB23 */
void GPIOPinRemap(FunctionalState s, uint16_t perph);

/* *
* @brief Analog peripheral GPIO pin function control
*
* @param s - Whether to enable the analog peripheral function
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
* RB_PIN_ADC4_5_IE - ADC/TKEY 4/5 Channel */
void GPIOAGPPCfg(FunctionalState s, uint16_t perph);

#ifdef __cplusplus
}
#endif

#endif // __CH59x_GPIO_H__
