/********************************** (C) COPYRIGHT *******************************
 * File Name          : CH59x_SYS.h
 * Author             : WCH
 * Version            : V1.2
 * Date               : 2021/11/17
 * Description
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#ifndef __CH59x_SYS_H__
#define __CH59x_SYS_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  rtc interrupt event define
 */
typedef enum
{
    RST_STATUS_SW = 0, // Software Reset
    RST_STATUS_RPOR,   // Power-on reset
    RST_STATUS_WTR,    // Watchdog timeout reset
    RST_STATUS_MR,     // External manual reset
    RST_STATUS_LRM0,   // Wake-up reset-caused by soft reset
    RST_STATUS_GPWSM,  // Power-off mode wake-up reset
    RST_STATUS_LRM1,   // Wake-up reset-caused by watchdog
    RST_STATUS_LRM2,   // Wake-up reset - manual reset causes

} SYS_ResetStaTypeDef;

/**
 * @brief  rtc interrupt event define
 */
typedef enum
{
    INFO_ROM_READ = 0, // FlashROM code and data area is readable
    INFO_RESET_EN = 2, // Is the RST# external manual reset input function enabled?
    INFO_BOOT_EN,      // System boot program: BootLoader is enabled
    INFO_DEBUG_EN,     // Is the system simulation debugging interface enabled?
    INFO_LOADER,       // Is the current system in the Bootloader area?
    STA_SAFEACC_ACT,   // Is the current system in a secure access state, otherwise the RWA attribute area is not accessible

} SYS_InfoStaTypeDef;

/* *
* @brief Get the chip ID class, generally a fixed value */
#define SYS_GetChipID()      R8_CHIP_ID

/* *
* @brief Get the secure access ID, generally a fixed value */
#define SYS_GetAccessID()    R8_SAFE_ACCESS_ID

/* *
* @brief Configure the system running clock
*
* @param sc - System clock source selection refer to SYS_CLKTypeDef */
void SetSysClock(SYS_CLKTypeDef sc);

/* *
* @brief Get the current system clock
*
* @return Hz */
uint32_t GetSysClock(void);

/* *
* @brief Get the current system information status
*
* @param i - refer to SYS_InfoStaTypeDef
*
* @return is enabled */
uint8_t SYS_GetInfoSta(SYS_InfoStaTypeDef i);

/* *
* @brief Get the last reset status of the system
*
* @return refer to SYS_ResetStaTypeDef */
#define SYS_GetLastResetSta()    (R8_RESET_STATUS & RB_RESET_FLAG)

/* *
* @brief Execute system software reset */
void SYS_ResetExecute(void);

/* *
* @brief Set the value of the reset save register, not affected by manual reset, software reset, watchdog reset or normal wake-up reset
*
* @param i - refer to SYS_InfoStaTypeDef */
#define SYS_ResetKeepBuf(d)    (R8_GLOB_RESET_KEEP = d)

/* *
* @brief Close all interrupts and keep the current interrupt value
*
* @param pirqv - Currently reserved interrupt value */
void SYS_DisableAllIrq(uint32_t *pirqv);

/* *
* @brief restores the interrupt value that was closed before
*
* @param irq_status - currently retained interrupt value */
void SYS_RecoverIrq(uint32_t irq_status);

/* *
* @brief Get the current system (SYSTICK) count value
*
* @return Current count value */
uint32_t SYS_GetSysTickCnt(void);

/* *
* @brief Load the initial value of the watchdog count, incremental
*
* @param c - Watchdog count initial value */
#define WWDG_SetCounter(c)    (R8_WDOG_COUNT = c)

/* *
* @brief Watchdog timer overflow interrupt enable
*
* @param s - whether the overflow is interrupted */
void WWDG_ITCfg(FunctionalState s);

/* *
* @brief Watchdog timer reset function
*
* @param s - Whether to reset overflow */
void WWDG_ResetCfg(FunctionalState s);

/* *
* @brief Get the current watchdog timer overflow flag
*
* @return Watchdog timer overflow flag */
#define WWDG_GetFlowFlag()    (R8_RST_WDOG_CTRL & RB_WDOG_INT_FLAG)

/* *
* @brief Clear the watchdog interrupt flag, and reload the count value can also be cleared */
void WWDG_ClearFlag(void);

/* *
* @brief uS Delay
*
* @param t - Time parameters */
void mDelayuS(uint16_t t);

/* *
* @brief mS Delay
*
* @param t - Time parameters */
void mDelaymS(uint16_t t);

extern volatile uint32_t IRQ_STA;

/**
 * @brief Enter safe access mode.
 * 
 * @NOTE: After enter safe access mode, about 16 system frequency cycles 
 * are in safe mode, and one or more secure registers can be rewritten 
 * within the valid period. The safe mode will be automatically 
 * terminated after the above validity period is exceeded.
 */
 __attribute__((always_inline)) static inline void sys_safe_access_enable(void)
{
    if(read_csr(0x800)&0x08)
    {
        IRQ_STA = read_csr(0x800);
        write_csr(0x800, (IRQ_STA&(~0x08)));
    }
    SAFEOPERATE;
    R8_SAFE_ACCESS_SIG = SAFE_ACCESS_SIG1;
    R8_SAFE_ACCESS_SIG = SAFE_ACCESS_SIG2;
    SAFEOPERATE;
}

__attribute__((always_inline)) static inline void sys_safe_access_disable(void)
{
    R8_SAFE_ACCESS_SIG = 0;
    write_csr(0x800, read_csr(0x800)|(IRQ_STA&0x08));
    IRQ_STA = 0;
    SAFEOPERATE;
}

#ifdef __cplusplus
}
#endif

#endif // __CH59x_SYS_H__
