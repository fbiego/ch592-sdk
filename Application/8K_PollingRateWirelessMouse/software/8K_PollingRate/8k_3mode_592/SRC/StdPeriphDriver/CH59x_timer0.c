/********************************** (C) COPYRIGHT *******************************
 * File Name          : CH59x_timer0.c
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
* @fn TMR0_TimerInit
*
* @brief timing function initialization
*
* @param t - timing time, based on the current system clock Tsys, maximum timing period 67108864
*
* @return none */
void TMR0_TimerInit(uint32_t t)
{
    R32_TMR0_CNT_END = t;
    R8_TMR0_CTRL_MOD = RB_TMR_ALL_CLEAR;
    R8_TMR0_CTRL_MOD = RB_TMR_COUNT_EN;
}

/* ***************************************************************************
* @fn TMR0_EXTSingleCounterInit
*
* @brief edge counting function initialization
*
* @param cap - Collection count type
*
* @return none */
void TMR0_EXTSingleCounterInit(CapModeTypeDef cap)
{
    R8_TMR0_CTRL_MOD = RB_TMR_ALL_CLEAR;
    R8_TMR0_CTRL_MOD = RB_TMR_COUNT_EN | RB_TMR_CAP_COUNT | RB_TMR_MODE_IN | (cap << 6);
}

/* ***************************************************************************
* @fn TMR0_PWMInit
*
* @brief PWM output initialization
*
* @param pr - select wave polar, refer to PWMX_PolarTypeDef
* @param ts - set pwm repeat times, refer to PWM_RepeatTsTypeDef
*
* @return none */
void TMR0_PWMInit(PWMX_PolarTypeDef pr, PWM_RepeatTsTypeDef ts)
{
    R8_TMR0_CTRL_MOD = RB_TMR_ALL_CLEAR;
    R8_TMR0_CTRL_MOD = (pr << 4) | (ts << 6);
}

/* ***************************************************************************
* @fn TMR0_CapInit
*
* @brief External signal capture function initialization
*
* @param cap - select capture mode, refer to CapModeTypeDef
*
* @return none */
void TMR0_CapInit(CapModeTypeDef cap)
{
    R8_TMR0_CTRL_MOD = RB_TMR_ALL_CLEAR;
    R8_TMR0_CTRL_MOD = RB_TMR_COUNT_EN | RB_TMR_MODE_IN | (cap << 6);
}
