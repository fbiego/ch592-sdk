/********************************** (C) COPYRIGHT *******************************
 * File Name          : SLEEP.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2022/03/15
 * Description        :
 * Copyright (c) 2023 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

/******************************************************************************/
/* The header file contains */
#include <RF.h>
#include <SLEEP.h>
#include "wchrf.h"
#include "CH59x_common.h"

/***************************************************
 * Global variables
 */
#ifndef SLEEP_RTC_MIN_TIME
#define SLEEP_RTC_MIN_TIME                  US_TO_RTC(1000)
#endif
#ifndef SLEEP_RTC_MAX_TIME
#define SLEEP_RTC_MAX_TIME                  (RTC_TIMER_MAX_VALUE - 1000 * 1000 * 30)
#endif
#ifndef WAKE_UP_RTC_MAX_TIME
#define WAKE_UP_RTC_MAX_TIME                US_TO_RTC(1600)
#endif

/* *********************************************************************************************
* @fn RF_LowPower
*
* @brief Start sleep
*
* @param time - time of sleep
*
* @return none. */
void RF_LowPower(uint32_t time)
{
#if(defined(HAL_SLEEP)) && (HAL_SLEEP == TRUE)
    uint32_t  time_curr,time_set;
    unsigned long irq_status;

    sys_safe_access_enable();
    R8_RTC_MODE_CTRL &= ~RB_RTC_TRIG_EN;  // Trigger mode
    sys_safe_access_disable();              //
    SYS_DisableAllIrq(&irq_status);

    time_curr = RTC_GetCycle32k();
    if( time >= RTC_TIMER_MAX_VALUE - time_curr )
    {
        time_set = time - (RTC_TIMER_MAX_VALUE - time_curr);
    }
    else
    {
        time_set = time + time_curr;
    }
    RTC_SetTignTime(time_set);
    gSleepFlag = TRUE;
    SYS_RecoverIrq(irq_status);
    sys_safe_access_enable();
    R8_RTC_MODE_CTRL |= RB_RTC_TRIG_EN;  // Trigger mode
    sys_safe_access_disable();              //

    // LOW POWER-SLEEP mode
    LowPower_Sleep( RB_PWR_RAM24K | RB_PWR_RAM2K | RB_PWR_EXTEND ); // Only 24+2K SRAM power supply is retained
    HSECFG_Current(HSE_RCur_100); // Reduced to rated current (HSE bias current is increased in low power consumption function)
#endif
}

/* *********************************************************************************************
* @fn RF_SleepInit
*
* @brief Configure sleep wake-up mode - RTC wake-up, trigger mode
*
* @param None.
*
* @return None. */
void RF_SleepInit(void)
{
#if(defined(HAL_SLEEP)) && (HAL_SLEEP == TRUE)
    sys_safe_access_enable();
    R8_SLP_WAKE_CTRL |= RB_SLP_RTC_WAKE; // RTC wake up
    sys_safe_access_disable();              //
//    sys_safe_access_enable();
// R8_RTC_MODE_CTRL |= RB_RTC_TRIG_EN; // Trigger mode
//    sys_safe_access_disable();              //
//    PFIC_EnableIRQ(RTC_IRQn);
#endif
}
