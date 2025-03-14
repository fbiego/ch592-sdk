/********************************** (C) COPYRIGHT *******************************
 * File Name          : CH59x_pwr.h
 * Author             : WCH
 * Version            : V1.2
 * Date               : 2021/11/17
 * Description
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#ifndef __CH59x_PWR_H__
#define __CH59x_PWR_H__

#ifdef __cplusplus
extern "C" {
#endif

#define ROM_CFG_ADR_HW       0x7F00C            // config address for hardware config for LDO&OSC and etc

/**
 * @brief	Peripher CLK control bit define
 */
#define BIT_SLP_CLK_TMR0     (0x00000001) /*!< TMR0 peripher clk bit */
#define BIT_SLP_CLK_TMR1     (0x00000002) /*!< TMR1 peripher clk bit */
#define BIT_SLP_CLK_TMR2     (0x00000004) /*!< TMR2 peripher clk bit */
#define BIT_SLP_CLK_TMR3     (0x00000008) /*!< TMR3 peripher clk bit */
#define BIT_SLP_CLK_UART0    (0x00000010) /*!< UART0 peripher clk bit */
#define BIT_SLP_CLK_UART1    (0x00000020) /*!< UART1 peripher clk bit */
#define BIT_SLP_CLK_UART2    (0x00000040) /*!< UART2 peripher clk bit */
#define BIT_SLP_CLK_UART3    (0x00000080) /*!< UART3 peripher clk bit */
#define BIT_SLP_CLK_SPI0     (0x00000100) /*!< SPI0 peripher clk bit */
//#define BIT_SLP_CLK_SPI1     (0x00000200)  /*!< SPI1 peripher clk bit */
#define BIT_SLP_CLK_PWMX     (0x00000400) /*!< PWMX peripher clk bit */
//#define BIT_SLP_CLK_LCD      (0x00000800)  /*!< LCD peripher clk bit */
#define BIT_SLP_CLK_USB      (0x00001000) /*!< USB peripher clk bit */
//#define BIT_SLP_CLK_ETH      (0x00002000)  /*!< ETH peripher clk bit */
//#define BIT_SLP_CLK_LED      (0x00004000)  /*!< LED peripher clk bit */
#define BIT_SLP_CLK_BLE      (0x00008000) /*!< BLE peripher clk bit */

#define BIT_SLP_CLK_RAMX     (0x10000000) /*!< main SRAM RAM16K peripher clk bit */
#define BIT_SLP_CLK_RAM2K    (0x20000000) /*!< RAM2K peripher clk bit */
#define BIT_SLP_CLK_ALL      (0x3000FFFF) /*!< All peripher clk bit */

/**
 * @brief  unit of controllable power supply
 */
#define UNIT_SYS_LSE         RB_CLK_XT32K_PON   // 外部32K 时钟振荡
#define UNIT_SYS_LSI         RB_CLK_INT32K_PON  // Internal 32K clock oscillation
#define UNIT_SYS_HSE         RB_CLK_XT32M_PON   // External 32M clock oscillation
#define UNIT_SYS_PLL         RB_CLK_PLL_PON     // PLL clock oscillation

/**
 * @brief  wakeup mode define
 */
typedef enum
{
    Short_Delay = 0,
    Long_Delay,

} WakeUP_ModeypeDef;

/**
 * @brief  wakeup mode define
 */
typedef enum
{
    /* The following levels will use high-precision monitoring, 210uA consumption */
    HALevel_1V9 = 0, // 1.7-1.9
    HALevel_2V1,     // 1.9-2.1
    HALevel_2V3,     // 2.1-2.3
    HALevel_2V5,     // 2.3-2.5

    /* The following level will use low power monitoring, 1uA consumption */
    LPLevel_1V8 = 0x80,
    LPLevel_1V9,
    LPLevel_2V0,
    LPLevel_2V1,
    LPLevel_2V2,
    LPLevel_2V3,
    LPLevel_2V4,
    LPLevel_2V5,

} VolM_LevelypeDef;

/* *
* @brief Enable internal DC/DC power supply to save system power consumption
*
* @param s - Whether to turn on DCDC power */
void PWR_DCDCCfg(FunctionalState s);

/* *
* @brief Power control of controllable unit module
*
* @param s - Whether to power on
* @param unit - please refer to unit of controlled power supply */
void PWR_UnitModCfg(FunctionalState s, uint8_t unit);

/* *
* @brief Peripheral clock control bit
*
* @param s - Whether to turn on the corresponding peripheral clock
* @param perph - please refer to Peripher CLK control bit define */
void PWR_PeriphClkCfg(FunctionalState s, uint16_t perph);

/* *
* @brief Sleep wake source configuration
*
* @param s - Whether to turn on the sleep wake-up function of this peripheral
* @param perph - Wake source that needs to be set
* RB_SLP_USB_WAKE - USB is the wake-up source
* RB_SLP_RTC_WAKE - RTC is the wake-up source
* RB_SLP_GPIO_WAKE - GPIO is the wake-up source
* RB_SLP_BAT_WAKE - BAT is the wake-up source
* @param mode - refer to WakeUP_ModeypeDef */
void PWR_PeriphWakeUpCfg(FunctionalState s, uint8_t perph, WakeUP_ModeypeDef mode);

/* *
* @brief Power Monitoring
*
* @param s - Whether to turn on this feature
* @param vl - refer to VolM_LevelypeDef */
void PowerMonitor(FunctionalState s, VolM_LevelypeDef vl);

/**
 * @brief   低功耗-Idle模式
 */
void LowPower_Idle(void);

/* *
* @brief Low power consumption - Halt mode, this low power consumption cuts to the HSI/5 clock operation, and after wake-up, the user needs to re-select the system clock source by himself */
void LowPower_Halt(void);

/* *
* @brief Low power consumption - Sleep mode, this low power consumption cuts to the HSI/5 clock operation, and after wake-up, the user needs to re-select the system clock source by himself
* @note Note: Call this function, the DCDC function is forced to be closed, and it can be manually turned on again after wake-up.
*
* @param rm - Power supply module selection
* RB_PWR_RAM2K - 2K retention SRAM powered
* RB_PWR_RAM16K - 16K main SRAM powered
* RB_PWR_EXTEND - Reserved area power supply for USB and BLE units
* RB_PWR_XROM - FlashROM Powered
* NULL - All the above units are powered off */
void LowPower_Sleep(uint16_t rm);

/* *
* @brief Low power consumption - Shutdown mode, this low power consumption cuts to the HSI/5 clock operation, and after wake-up, the user needs to re-select the system clock source by himself
* @note Note: Call this function, the DCDC function is forced to be closed, and it can be manually turned on again after wake-up.
*
* @param rm - Power supply module selection
* RB_PWR_RAM2K - 2K retention SRAM powered
* RB_PWR_RAM16K - 16K main SRAM powered
* NULL - All the above units are powered off */
void LowPower_Shutdown(uint16_t rm);

#ifdef __cplusplus
}
#endif

#endif // __CH59x_PWR_H__
