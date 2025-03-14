/********************************** (C) COPYRIGHT *******************************
 * File Name          : CH59x_pwm.c
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
* @fn PWMX_CycleCfg
*
* @brief PWM4-PWM11 reference clock configuration
*
* @param cyc - refer to PWMX_CycleTypeDef
*
* @return none */
void PWMX_CycleCfg(PWMX_CycleTypeDef cyc)
{
    switch(cyc)
    {
        case PWMX_Cycle_256:
            R8_PWM_CONFIG = R8_PWM_CONFIG & 0xf0;
            break;

        case PWMX_Cycle_255:
            R8_PWM_CONFIG = (R8_PWM_CONFIG & 0xf0) | 0x01;
            break;

        case PWMX_Cycle_128:
            R8_PWM_CONFIG = (R8_PWM_CONFIG & 0xf0) | (1 << 2);
            break;

        case PWMX_Cycle_127:
            R8_PWM_CONFIG = (R8_PWM_CONFIG & 0xf0) | (1 << 2) | 0x01;
            break;

        case PWMX_Cycle_64:
            R8_PWM_CONFIG = (R8_PWM_CONFIG & 0xf0) | (2 << 2);
            break;

        case PWMX_Cycle_63:
            R8_PWM_CONFIG = (R8_PWM_CONFIG & 0xf0) | (2 << 2) | 0x01;
            break;

        case PWMX_Cycle_32:
            R8_PWM_CONFIG = (R8_PWM_CONFIG & 0xf0) | (3 << 2);
            break;

        case PWMX_Cycle_31:
            R8_PWM_CONFIG = (R8_PWM_CONFIG & 0xf0) | (3 << 2) | 0x01;
            break;

        default:
            break;
    }
}

/* ********************************************************************
* @fn      PWMX_ACTOUT
*
* @brief   PWM4-PWM11通道输出波形配置
*
* @param   ch      - select channel of pwm, refer to channel of PWM define
* @param   da      - effective pulse width
* @param   pr      - select wave polar, refer to PWMX_PolarTypeDef
* @param   s       - control pwmx function, ENABLE or DISABLE
*
* @return  none */
void PWMX_ACTOUT(uint8_t ch, uint8_t da, PWMX_PolarTypeDef pr, FunctionalState s)
{
    uint8_t i;

    if(s == DISABLE)
    {
        R8_PWM_OUT_EN &= ~(ch);
    }
    else
    {
        (pr) ? (R8_PWM_POLAR |= (ch)) : (R8_PWM_POLAR &= ~(ch));
        for(i = 0; i < 8; i++)
        {
            if((ch >> i) & 1)
            {
                *((volatile uint8_t *)((&R8_PWM4_DATA) + i)) = da;
            }
        }
        R8_PWM_OUT_EN |= (ch);
    }
}

/* ***************************************************************************
* @fn PWMX_AlterOutCfg
*
* @brief PWM Alternating Output Mode Configuration
*
* @param ch - select group of PWM alternate output
* RB_PWM4_5_STAG_EN - Alternate output of PWM4 and PWM5 channels
* RB_PWM6_7_STAG_EN - Alternate output of PWM6 and PWM7 channels
* RB_PWM8_9_STAG_EN - Alternate output of PWM8 and PWM9 channels
* RB_PWM10_11_STAG_EN - Alternate output of PWM10 and PWM11 channels
* @param s - control pwmx function, ENABLE or DISABLE
*
* @return none */
void PWMX_AlterOutCfg(uint8_t ch, FunctionalState s)
{
    if(s == DISABLE)
    {
        R8_PWM_CONFIG &= ~(ch);
    }
    else
    {
        R8_PWM_CONFIG |= (ch);
    }
}
