/********************************** (C) COPYRIGHT *******************************
 * File Name          : CH59x_pwm.h
 * Author             : WCH
 * Version            : V1.2
 * Date               : 2021/11/17
 * Description
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#ifndef __CH59x_PWM_H__
#define __CH59x_PWM_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  channel of PWM define
 */
#define CH_PWM4     0x01  // PWM4 Channel
#define CH_PWM5     0x02  // PWM5 Channel
#define CH_PWM6     0x04  // PWM6 Channel
#define CH_PWM7     0x08  // PWM7 Channel
#define CH_PWM8     0x10  // PWM8 Channel
#define CH_PWM9     0x20  // PWM9 Channel
#define CH_PWM10    0x40  // PWM10 Channel
#define CH_PWM11    0x80  // PWM11 Channel

/**
 * @brief  channel of PWM define
 */
typedef enum
{
    High_Level = 0, // Default low level, active high level
    Low_Level,      // Default high level, active low level
} PWMX_PolarTypeDef;

/**
 * @brief  Configuration PWM4_11 Cycle size
 */
typedef enum
{
    PWMX_Cycle_256 = 0, // 256 PWMX cycles
    PWMX_Cycle_255,     // 255 PWMX cycles
    PWMX_Cycle_128,     // 128 PWMX cycles
    PWMX_Cycle_127,     // 127 PWMX cycles
    PWMX_Cycle_64,      // 64 PWMX cycles
    PWMX_Cycle_63,      // 63 PWMX cycles
} PWMX_CycleTypeDef;

/* *
* @brief PWM4-PWM11 Channel Reference Clock Configuration
*
* @param d - Channel reference clock = d*Tsys */
#define PWMX_CLKCfg(d)    (R8_PWM_CLOCK_DIV = d)

/* *
* @brief PWM4-PWM11 cycle configuration
*
* @param cyc - refer to PWMX_CycleTypeDef */
void PWMX_CycleCfg(PWMX_CycleTypeDef cyc);

/* *
* @brief PWM4-PWM9 16-bit cycle configuration
*
* @param cyc - 16-bit cycle */
void PWMX_16bit_CycleCfg(uint16_t cyc);

/* *
* @brief Set PWM4 valid data pulse width
*
* @param d - Effective data pulse width */
#define PWM4_ActDataWidth(d)     (R8_PWM4_DATA = d)

/* *
* @brief Set PWM5 valid data pulse width
*
* @param d - Effective data pulse width */
#define PWM5_ActDataWidth(d)     (R8_PWM5_DATA = d)

/* *
* @brief Set PWM6 valid data pulse width
*
* @param d - Effective data pulse width */
#define PWM6_ActDataWidth(d)     (R8_PWM6_DATA = d)

/* *
* @brief Set PWM7 valid data pulse width
*
* @param d - Effective data pulse width */
#define PWM7_ActDataWidth(d)     (R8_PWM7_DATA = d)

/* *
* @brief Set PWM8 valid data pulse width
*
* @param d - Effective data pulse width */
#define PWM8_ActDataWidth(d)     (R8_PWM8_DATA = d)

/* *
* @brief Set PWM9 valid data pulse width
*
* @param d - Effective data pulse width */
#define PWM9_ActDataWidth(d)     (R8_PWM9_DATA = d)

/* *
* @brief Set PWM10 valid data pulse width
*
* @param d - Effective data pulse width */
#define PWM10_ActDataWidth(d)    (R8_PWM10_DATA = d)

/* *
* @brief Set PWM11 valid data pulse width
*
* @param d - Effective data pulse width */
#define PWM11_ActDataWidth(d)    (R8_PWM11_DATA = d)

/* *
* @brief   PWM4-PWM11通道输出波形配置
*
* @param   ch      - select channel of pwm, refer to channel of PWM define
* @param   da      - effective pulse width
* @param   pr      - select wave polar, refer to PWMX_PolarTypeDef
* @param   s       - control pwmx function, ENABLE or DISABLE */
void PWMX_ACTOUT(uint8_t ch, uint8_t da, PWMX_PolarTypeDef pr, FunctionalState s);

/* *
* @brief   PWM4-PWM9 通道16位输出波形配置
*
* @param   ch      - select channel of pwm, refer to channel of PWM define
* @param   da      - effective pulse width
* @param   pr      - select wave polar, refer to PWMX_PolarTypeDef
* @param   s       - control pwmx function, ENABLE or DISABLE */
void PWMX_16bit_ACTOUT(uint8_t ch, uint16_t da, PWMX_PolarTypeDef pr, FunctionalState s);

/* *
* @brief PWM Alternating Output Mode Configuration
*
* @param ch - select group of PWM alternate output
* RB_PWM4_5_STAG_EN - Alternate output of PWM4 and PWM5 channels
* RB_PWM6_7_STAG_EN - Alternate output of PWM6 and PWM7 channels
* RB_PWM8_9_STAG_EN - Alternate output of PWM8 and PWM9 channels
* RB_PWM10_11_STAG_EN - Alternate output of PWM10 and PWM11 channels
* @param s - control pwmx function, ENABLE or DISABLE */
void PWMX_AlterOutCfg(uint8_t ch, FunctionalState s);

#ifdef __cplusplus
}
#endif

#endif // __CH59x_PWM_H__
