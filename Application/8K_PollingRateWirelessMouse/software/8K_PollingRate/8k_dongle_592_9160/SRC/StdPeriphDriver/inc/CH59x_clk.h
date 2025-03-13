/********************************** (C) COPYRIGHT *******************************
 * File Name          : CH59x_clk.h
 * Author             : WCH
 * Version            : V1.2
 * Date               : 2021/11/17
 * Description
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#ifndef __CH59x_CLK_H__
#define __CH59x_CLK_H__

#ifdef __cplusplus
extern "C" {
#endif

/* *
* @brief System main frequency definition */
typedef enum
{
    CLK_SOURCE_LSI = 0x00,
    CLK_SOURCE_LSE,

    CLK_SOURCE_HSE_16MHz = 0x22,
    CLK_SOURCE_HSE_8MHz = 0x24,
    CLK_SOURCE_HSE_6_4MHz = 0x25,
    CLK_SOURCE_HSE_4MHz = 0x28,

    CLK_SOURCE_PLL_60MHz = 0x48,
    CLK_SOURCE_PLL_48MHz = (0x40 | 10),
    CLK_SOURCE_PLL_32MHz = (0x40 | 15),
    CLK_SOURCE_PLL_24MHz = (0x40 | 20),
} SYS_CLKTypeDef;

/* *
* @brief 32K clock selection */
typedef enum
{
    Clk32K_LSI = 0,
    Clk32K_LSE,

} LClk32KTypeDef;

/* *
* @brief 32M crystal oscillator current gear */
typedef enum
{
    HSE_RCur_75 = 0,
    HSE_RCur_100,
    HSE_RCur_125,
    HSE_RCur_150

} HSECurrentTypeDef;

/* *
* @brief 32M crystal oscillator internal capacitor gear */
typedef enum
{
    HSECap_10p = 0,
    HSECap_12p,
    HSECap_14p,
    HSECap_16p,
    HSECap_18p,
    HSECap_20p,
    HSECap_22p,
    HSECap_24p

} HSECapTypeDef;

/* *
* @brief 32K crystal oscillator current gear */
typedef enum
{
    LSE_RCur_70 = 0,
    LSE_RCur_100,
    LSE_RCur_140,
    LSE_RCur_200

} LSECurrentTypeDef;

/* *
* @brief 32K crystal oscillator internal capacitor gear */
typedef enum
{
    LSECap_2p = 0,
    LSECap_13p,
    LSECap_14p,
    LSECap_15p,
    LSECap_16p,
    LSECap_17p,
    LSECap_18p,
    LSECap_19p,
    LSECap_20p,
    LSECap_21p,
    LSECap_22p,
    LSECap_23p,
    LSECap_24p,
    LSECap_25p,
    LSECap_26p,
    LSECap_27p

} LSECapTypeDef;

#define RTC_MAX_COUNT             0xA8C00000

#define MAX_DAY                   0x00004000
#define MAX_2_SEC                 0x0000A8C0
//#define	 MAX_SEC		0x545FFFFF

#define BEGYEAR                   2020
#define IsLeapYear(yr)            (!((yr) % 400) || (((yr) % 100) && !((yr) % 4)))
#define YearLength(yr)            (IsLeapYear(yr) ? 366 : 365)
#define monthLength(lpyr, mon)    (((mon) == 1) ? (28 + (lpyr)) : (((mon) > 6) ? (((mon) & 1) ? 31 : 30) : (((mon) & 1) ? 30 : 31)))

/**
 * @brief  rtc timer mode period define
 */
typedef enum
{
    Period_0_125_S = 0, // 0.125s cycle
    Period_0_25_S,      // 0.25s cycle
    Period_0_5_S,       // 0.5s cycle
    Period_1_S,         // 1s cycle
    Period_2_S,         // 2s cycle
    Period_4_S,         // 4s cycle
    Period_8_S,         // 8s cycle
    Period_16_S,        // 16s ÖÜÆÚ
} RTC_TMRCycTypeDef;

/**
 * @brief  rtc interrupt event define
 */
typedef enum
{
    RTC_TRIG_EVENT = 0, // RTC trigger event
    RTC_TMR_EVENT,      // RTC periodic timing events

} RTC_EVENTTypeDef;

/**
 * @brief  rtc interrupt event define
 */
typedef enum
{
    RTC_TRIG_MODE = 0, // RTC Trigger Mode
    RTC_TMR_MODE,      // RTC periodic timing mode

} RTC_MODETypeDef;

typedef enum
{
    /* The higher the calibration accuracy, the longer the time it takes */
    Level_32 = 3, // Time taken 1.2ms 1000ppm (32M main frequency) 1100ppm (60M main frequency)
    Level_64,     // Time 2.2ms 800ppm (32M main frequency) 1000ppm (60M main frequency)
    Level_128,    // Time 4.2ms 600ppm (32M main frequency) 800ppm (60M main frequency)

} Cali_LevelTypeDef;

/* *
* @brief 32K low frequency clock source
*
* @param hc - Choose 32K to use internal or external */
void LClk32K_Select(LClk32KTypeDef hc);

/* *
* @brief HSE crystal Bias current configuration
*
* @param c - 75%, 100%, 125%, 150% */
void HSECFG_Current(HSECurrentTypeDef c);

/* *
* @brief HSE crystal Load capacitor configuration
*
* @param c - refer to HSECapTypeDef */
void HSECFG_Capacitance(HSECapTypeDef c);

/* *
* @brief LSE crystal Bias current configuration
*
* @param c - 70%, 100%, 140%, 200% */
void LSECFG_Current(LSECurrentTypeDef c);

/* *
* @brief LSE crystal Load capacitor configuration
*
* @param c - refer to LSECapTypeDef */
void LSECFG_Capacitance(LSECapTypeDef c);

void Calibration_LSI(Cali_LevelTypeDef cali_Lv); /* Calibrate the internal 32K clock with main frequency */

/* *
* @brief RTC clock initialization current time
*
* @param y - Configuration year, MAX_Y = BEGYEAR + 44
* @param mon - Configure month, MAX_MON = 12
* @param d - Configuration day, MAX_D = 31
* @param h - Configuration hours, MAX_H = 23
* @param m - Configuration minutes, MAX_M = 59
* @param s - Configure seconds, MAX_S = 59 */
void RTC_InitTime(uint16_t y, uint16_t mon, uint16_t d, uint16_t h, uint16_t m, uint16_t s);

/* *
* @brief Get the current time
*
* @param py - year obtained, MAX_Y = BEGYEAR + 44
* @param pmon - month obtained, MAX_MON = 12
* @param pd - the day obtained, MAX_D = 31
* @param ph - The hour obtained, MAX_H = 23
* @param pm - minute obtained, MAX_M = 59
* @param ps - the seconds obtained, MAX_S = 59 */
void RTC_GetTime(uint16_t *py, uint16_t *pmon, uint16_t *pd, uint16_t *ph, uint16_t *pm, uint16_t *ps);

/* *
* @brief Configure the current number of RTC cycles based on the LSE/LSI clock
*
* @param cyc - Configure the initial value of the cycle count, MAX_CYC = 0xA8BFFFF = 2831155199 */
void RTC_SetCycle32k(uint32_t cyc);

/* *
* @brief Get the current number of RTC cycles based on the LSE/LSI clock
*
* @return Current cycle number, MAX_CYC = 0xA8BFFFF = 2831155199 */
uint32_t RTC_GetCycle32k(void);

/* *
* @brief RTC timing mode configuration (note that the timing reference is fixed to 32768Hz)
*
* @param t - refer to RTC_TMRCycTypeDef */
void RTC_TRIGFunCfg(uint32_t cyc);

/* *
* @brief RTC timing mode configuration (note that the timing reference is fixed to 32768Hz)
*
* @param t - refer to RTC_TMRCycTypeDef */
void RTC_TMRFunCfg(RTC_TMRCycTypeDef t);

/* *
* @brief RTC mode function is turned off
*
* @param m - Current mode that needs to be turned off */
void RTC_ModeFunDisable(RTC_MODETypeDef m);

/* *
* @brief Get RTC interrupt flag
*
* @param f - refer to RTC_EVENTTypeDef
*
* @return Interrupt flag status */
uint8_t RTC_GetITFlag(RTC_EVENTTypeDef f);

/* *
* @brief Clear RTC interrupt flag
*
* @param f - refer to RTC_EVENTTypeDef */
void RTC_ClearITFlag(RTC_EVENTTypeDef f);

/* *
* @brief 32K low frequency clock power supply configuration */
void LClk32K_Cfg(LClk32KTypeDef hc, FunctionalState s);


#ifdef __cplusplus
}
#endif

#endif // __CH59x_CLK_H__
