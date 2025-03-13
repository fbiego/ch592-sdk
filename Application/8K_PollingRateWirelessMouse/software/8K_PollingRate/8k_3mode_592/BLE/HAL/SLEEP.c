/* ********************************* (C) COPYRIGHT *******************************
* File Name          : SLEEP.c
* Author             : WCH
* Version            : V1.2
* Date               : 2022/01/18
* Description        : ÀØ√ﬂ≈‰÷√º∞∆‰≥ı ºªØ
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
****************************************************************************** */

/******************************************************************************/
/* The header file contains */
#include "HAL.h"
#include "access.h"

/* *********************************************************************************************
* @fn CH58X_LowPower
*
* @brief Start sleep
*
* @param time - Wake-up time point (RTC absolute value)
*
* @return state. */
__HIGH_CODE
uint32_t CH59x_LowPower(uint32_t time)
{
#if(defined(HAL_SLEEP)) && (HAL_SLEEP == TRUE)
    volatile uint32_t i;
    uint32_t time_sleep, time_curr;
    unsigned long irq_status;
    
    if(!access_state.sleep_en)
        return 0;
    // Wait for the serial port to be sent
    if(R8_UART1_TFC)
        return 3;
    // Sleep shut down the serial port, and then turn it on after waking up
    PFIC_DisableIRQ(UART1_IRQn);
	
    // Wake up in advance
    if (time <= WAKE_UP_RTC_MAX_TIME) {
        time = time + (RTC_MAX_COUNT - WAKE_UP_RTC_MAX_TIME);
    } else {
        time = time - WAKE_UP_RTC_MAX_TIME;
    }
	
    SYS_DisableAllIrq(&irq_status);
    time_curr = RTC_GetCycle32k();
    // Detection of sleep time
    if (time < time_curr) {
        time_sleep = time + (RTC_MAX_COUNT - time_curr);
    } else {
        time_sleep = time - time_curr;
    }
    
    // If the sleep time is less than the minimum sleep time or greater than the maximum sleep time, then no sleep will occur.
    if ((time_sleep < SLEEP_RTC_MIN_TIME) || 
        (time_sleep > SLEEP_RTC_MAX_TIME)) {
        SYS_RecoverIrq(irq_status);
        return 2;
    }
    RTC_SetTignTime(time);
    SYS_RecoverIrq(irq_status);
    peripheral_enter_sleep();
  #if(DEBUG == Debug_UART1) // Use other serial ports to output printing information to modify this line of code to output the code
    while((R8_UART1_LSR & RB_LSR_TX_ALL_EMP) == 0)
    {
        __nop();
    }
  #endif
    // LOW POWER-sleep mode
    if((!RTCTigFlag))
    {
        LowPower_Sleep(RB_PWR_RAM2K | RB_PWR_RAM24K | RB_PWR_EXTEND | RB_XT_PRE_EN );
//        LowPower_Idle();

        if(RTCTigFlag) // Note that if you use wake-up method other than RTC, you need to note that the 32M crystal oscillator is not stable at this time.
        {
            time += WAKE_UP_RTC_MAX_TIME;
            if(time > 0xA8C00000)
            {
                time -= 0xA8C00000;
            }
            RTC_SetTignTime(time);
            LowPower_Idle();
        }
        PFIC_DisableIRQ( GPIO_A_IRQn );
        HSECFG_Current(HSE_RCur_100); // Reduced to rated current (HSE bias current is increased in low power consumption function)
		i = RTC_GetCycle32k();
        while(i == RTC_GetCycle32k());
    }
    else
    {
        return 3;
    }
#endif
    return 0;
}

/* *********************************************************************************************
* @fn HAL_SleepInit
*
* @brief Configure sleep wake-up mode - RTC wake-up, trigger mode
*
* @param None.
*
* @return None. */
void HAL_SleepInit(void)
{
#if(defined(HAL_SLEEP)) && (HAL_SLEEP == TRUE)
    sys_safe_access_enable();
    R8_SLP_WAKE_CTRL |= RB_SLP_RTC_WAKE; // RTCªΩ–—
    sys_safe_access_disable();              //
    sys_safe_access_enable();
    R8_RTC_MODE_CTRL |= RB_RTC_TRIG_EN;  // Trigger mode
    sys_safe_access_disable();              //
    PFIC_EnableIRQ(RTC_IRQn);
#endif
}
