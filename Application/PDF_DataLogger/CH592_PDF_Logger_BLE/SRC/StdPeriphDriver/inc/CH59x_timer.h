/********************************** (C) COPYRIGHT *******************************
 * File Name          : CH59x_timer.h
 * Author             : WCH
 * Version            : V1.2
 * Date               : 2021/11/17
 * Description
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#ifndef __CH59x_TIMER_H__
#define __CH59x_TIMER_H__

#ifdef __cplusplus
extern "C" {
#endif

#define DataBit_25            (1 << 25)

/**
 * @brief  TMR0 interrupt bit define
 */

#define TMR0_3_IT_CYC_END     0x01  // Period end flag: Capture-timeout, timing-period end, PWM-period end
#define TMR0_3_IT_DATA_ACT    0x02  // Data valid flag: Capture - new data, PWM - end of valid level
#define TMR0_3_IT_FIFO_HF     0x04  // FIFO more than half used: Capture - FIFO >=4, PWM- FIFO<4
#define TMR1_2_IT_DMA_END     0x08  // DMA ends, support TMR1 and TMR2
#define TMR0_3_IT_FIFO_OV     0x10  // FIFO Overflow: Capture - FIFO full, PWM - FIFO empty

/**
 * @brief  Configuration PWM effective level repeat times
 */
typedef enum
{
    PWM_Times_1 = 0, // PWM valid output repeats 1 times
    PWM_Times_4,     // PWM valid output repeats 4 times
    PWM_Times_8,     // PWM valid output repeats 8 times
    PWM_Times_16,    // PWM valid output repeats 16 times
} PWM_RepeatTsTypeDef;

/**
 * @brief  Configuration Cap mode
 */
typedef enum
{
    CAP_NULL = 0,         // Don't capture & don't count
    Edge_To_Edge,         // Between any edges & count any edges
    FallEdge_To_FallEdge, // Falling to Falling Edge & Count Falling Edge
    RiseEdge_To_RiseEdge, // Rising edge to Rising edge & Count rising edge
} CapModeTypeDef;

/**
 * @brief  Configuration DMA mode
 */
typedef enum
{
    Mode_Single = 0, // Single-time mode
    Mode_LOOP,       // Loop mode
} DMAModeTypeDef;

/* *
* @brief timing function initialization
*
* @param t - timing time, based on the current system clock Tsys, maximum timing period 67108864 */
void TMR0_TimerInit(uint32_t t);

/* *
* @brief Get the current timer value, maximum 67108864
*
* @return Current timer value */
#define TMR0_GetCurrentTimer()    R32_TMR0_COUNT

/* *
* @brief edge counting function initialization
*
* @param cap - Collection count type */
void TMR0_EXTSingleCounterInit(CapModeTypeDef cap);

/* *
* @brief Set count statistics overflow size, maximum 67108862
*
* @param cyc - Counting statistics overflow size */
#define TMR0_CountOverflowCfg(cyc)    (R32_TMR0_CNT_END = (cyc + 2))

/* *
* @brief Get the current count value, maximum 67108862
*
* @return Current count value */
#define TMR0_GetCurrentCount()        R32_TMR0_COUNT

/* *
* @brief PWM0 channel output waveform period configuration, maximum 67108864
*
* @param cyc - Output waveform period */
#define TMR0_PWMCycleCfg(cyc)         (R32_TMR0_CNT_END = cyc)

/* *
* @brief PWM output initialization
*
* @param pr - select wave polar, refer to PWMX_PolarTypeDef
* @param ts - set pwm repeat times, refer to PWM_RepeatTsTypeDef */
void TMR0_PWMInit(PWMX_PolarTypeDef pr, PWM_RepeatTsTypeDef ts);

/* *
* @brief PWM0 Valid data pulse width, maximum 67108864
*
* @param d - Effective data pulse width */
#define TMR0_PWMActDataWidth(d)    (R32_TMR0_FIFO = d)

/* *
* @brief CAP0 Capture level timeout configuration, maximum 33554432
*
* @param cyc - Capture level timeout */
#define TMR0_CAPTimeoutCfg(cyc)    (R32_TMR0_CNT_END = cyc)

/* *
* @brief External signal capture function initialization
*
* @param cap - select capture mode, refer to CapModeTypeDef */
void TMR0_CapInit(CapModeTypeDef cap);

/* *
* @brief Get pulse data
*
* @return Pulse Data */
#define TMR0_CAPGetData()        R32_TMR0_FIFO

/* *
* @brief Get the current number of captured data
*
* @return The number of data captured currently */
#define TMR0_CAPDataCounter()    R8_TMR0_FIFO_COUNT

/* *
* @brief Close TMR0 PWM output */
#define TMR0_PWMDisable()           (R8_TMR0_CTRL_MOD &= ~RB_TMR_OUT_EN)

/* *
* @brief enable TMR0 PWM output */
#define TMR0_PWMEnable()           (R8_TMR0_CTRL_MOD |= RB_TMR_OUT_EN)

/* *
* @brief Close TMR0 */
#define TMR0_Disable()           (R8_TMR0_CTRL_MOD &= ~RB_TMR_COUNT_EN)

/* *
* @brief enable TMR0 */
#define TMR0_Enable()            (R8_TMR0_CTRL_MOD |= RB_TMR_COUNT_EN)

/* *
* @brief interrupt configuration
*
* @param s - Enable/Close
* @param f - refer to TMR interrupt bit define */
#define TMR0_ITCfg(s, f)         ((s) ? (R8_TMR0_INTER_EN |= f) : (R8_TMR0_INTER_EN &= ~f))

/* *
* @brief Clear interrupt flag
*
* @param f - refer to TMR interrupt bit define */
#define TMR0_ClearITFlag(f)      (R8_TMR0_INT_FLAG = f)

/* *
* @brief query interrupt flag status
*
* @param f - refer to TMR interrupt bit define */
#define TMR0_GetITFlag(f)        (R8_TMR0_INT_FLAG & f)

/* *
* @brief timing function initialization
*
* @param t - timing time, based on the current system clock Tsys, maximum timing period 67108864 */
void TMR1_TimerInit(uint32_t t);

/* *
* @brief Get the current timer value, maximum 67108864
*
* @return Current timer value */
#define TMR1_GetCurrentTimer()    R32_TMR1_COUNT

/* *
* @brief edge counting function initialization
*
* @param cap - Collection count type */
void TMR1_EXTSingleCounterInit(CapModeTypeDef cap);

/* *
* @brief Set count statistics overflow size, maximum 67108862
*
* @param cyc - Counting statistics overflow size */
#define TMR1_CountOverflowCfg(cyc)    (R32_TMR1_CNT_END = (cyc + 2))

/* *
* @brief Get the current count value, maximum 67108862
*
* @return Current count value */
#define TMR1_GetCurrentCount()        R32_TMR1_COUNT

/* *
* @brief PWM1 channel output waveform period configuration, maximum 67108864
*
* @param cyc - Output waveform period */
#define TMR1_PWMCycleCfg(cyc)         (R32_TMR1_CNT_END = cyc)

/* *
* @brief PWM output initialization
*
* @param pr - select wave polar, refer to PWMX_PolarTypeDef
* @param ts - set pwm repeat times, refer to PWM_RepeatTsTypeDef */
void TMR1_PWMInit(PWMX_PolarTypeDef pr, PWM_RepeatTsTypeDef ts);

/* *
* @brief PWM1 Valid data pulse width, maximum 67108864
*
* @param d - Effective data pulse width */
#define TMR1_PWMActDataWidth(d)    (R32_TMR1_FIFO = d)

/* *
* @brief CAP1 Capture level timeout configuration, maximum 33554432
*
* @param cyc - Capture level timeout */
#define TMR1_CAPTimeoutCfg(cyc)    (R32_TMR1_CNT_END = cyc)

/* *
* @brief External signal capture function initialization
*
* @param cap - select capture mode, refer to CapModeTypeDef */
void TMR1_CapInit(CapModeTypeDef cap);

/* *
* @brief Get pulse data
*
* @return Pulse Data */
#define TMR1_CAPGetData()        R32_TMR1_FIFO

/* *
* @brief Get the current number of captured data
*
* @return The number of data captured currently */
#define TMR1_CAPDataCounter()    R8_TMR1_FIFO_COUNT

/* *
* @brief Configure DMA function
*
* @param s - Whether to turn on the DMA function
* @param startAddr - DMA Start Address
* @param endAddr - DMA end address
* @param m - Configure DMA mode */
void TMR1_DMACfg(uint8_t s, uint16_t startAddr, uint16_t endAddr, DMAModeTypeDef m);

/* *
* @brief Close TMR1 PWM output */
#define TMR1_PWMDisable()           (R8_TMR1_CTRL_MOD &= ~RB_TMR_OUT_EN)

/* *
* @brief Enable TMR1 PWM output */
#define TMR1_PWMEnable()           (R8_TMR1_CTRL_MOD |= RB_TMR_OUT_EN)

/* *
* @brief Close TMR1 */
#define TMR1_Disable()         (R8_TMR1_CTRL_MOD &= ~RB_TMR_COUNT_EN)

/* *
* @brief activate TMR1 */
#define TMR1_Enable()          (R8_TMR1_CTRL_MOD |= RB_TMR_COUNT_EN)

/* *
* @brief interrupt configuration
*
* @param s - Enable/Close
* @param f - refer to TMR interrupt bit define */
#define TMR1_ITCfg(s, f)       ((s) ? (R8_TMR1_INTER_EN |= f) : (R8_TMR1_INTER_EN &= ~f))

/* *
* @brief Clear interrupt flag
*
* @param f - refer to TMR interrupt bit define */
#define TMR1_ClearITFlag(f)    (R8_TMR1_INT_FLAG = f)

/* *
* @brief query interrupt flag status
*
* @param f - refer to TMR interrupt bit define */
#define TMR1_GetITFlag(f)      (R8_TMR1_INT_FLAG & f)

/* *
* @brief timing function initialization
*
* @param t - timing time, based on the current system clock Tsys, maximum timing period 67108864 */
void TMR2_TimerInit(uint32_t t);

/* *
* @brief Get the current timer value, maximum 67108864
*
* @return Current timer value */
#define TMR2_GetCurrentTimer()    R32_TMR2_COUNT

/* *
* @brief edge counting function initialization
*
* @param cap - Collection count type */
void TMR2_EXTSingleCounterInit(CapModeTypeDef cap);

/* *
* @brief Set count statistics overflow size, maximum 67108862
*
* @param cyc - Counting statistics overflow size */
#define TMR2_CountOverflowCfg(cyc)    (R32_TMR2_CNT_END = (cyc + 2))

/* *
* @brief Get the current count value, maximum 67108862
*
* @return Current count value */
#define TMR2_GetCurrentCount()        R32_TMR2_COUNT

/* *
* @brief PWM2 channel output waveform period configuration, maximum 67108864
*
* @param cyc - Output waveform period */
#define TMR2_PWMCycleCfg(cyc)         (R32_TMR2_CNT_END = cyc)

/* *
* @brief PWM output initialization
*
* @param pr - select wave polar, refer to PWMX_PolarTypeDef
* @param ts - set pwm repeat times, refer to PWM_RepeatTsTypeDef */
void TMR2_PWMInit(PWMX_PolarTypeDef pr, PWM_RepeatTsTypeDef ts);

/* *
* @brief PWM2 valid data pulse width, maximum 67108864
*
* @param d - Effective data pulse width */
#define TMR2_PWMActDataWidth(d)    (R32_TMR2_FIFO = d)

/* *
* @brief CAP2 Capture level timeout configuration, maximum 33554432
*
* @param cyc - Capture level timeout */
#define TMR2_CAPTimeoutCfg(cyc)    (R32_TMR2_CNT_END = cyc)

/* *
* @brief External signal capture function initialization
*
* @param cap - select capture mode, refer to CapModeTypeDef */
void TMR2_CapInit(CapModeTypeDef cap);

/* *
* @brief Get pulse data
*
* @return Pulse Data */
#define TMR2_CAPGetData()        R32_TMR2_FIFO

/* *
* @brief Get the current number of captured data
*
* @return The number of data captured currently */
#define TMR2_CAPDataCounter()    R8_TMR2_FIFO_COUNT

/* *
* @brief Configure DMA function
*
* @param s - Whether to turn on the DMA function
* @param startAddr - DMA Start Address
* @param endAddr - DMA end address
* @param m - Configure DMA mode */
void TMR2_DMACfg(uint8_t s, uint32_t startAddr, uint32_t endAddr, DMAModeTypeDef m);

/* *
* @brief Close TMR2 PWM output */
#define TMR2_PWMDisable()           (R8_TMR2_CTRL_MOD &= ~RB_TMR_OUT_EN)

/* *
* @brief enables TMR2 PWM output */
#define TMR2_PWMEnable()           (R8_TMR2_CTRL_MOD |= RB_TMR_OUT_EN)

/* *
* @brief Close TMR2 */
#define TMR2_Disable()         (R8_TMR2_CTRL_MOD &= ~RB_TMR_COUNT_EN)

/* *
* @brief enable TMR2 */
#define TMR2_Enable()          (R8_TMR2_CTRL_MOD |= RB_TMR_COUNT_EN)

/* *
* @brief interrupt configuration
*
* @param s - Enable/Close
* @param f - refer to TMR interrupt bit define */
#define TMR2_ITCfg(s, f)       ((s) ? (R8_TMR2_INTER_EN |= f) : (R8_TMR2_INTER_EN &= ~f))

/* *
* @brief Clear interrupt flag
*
* @param f - refer to TMR interrupt bit define */
#define TMR2_ClearITFlag(f)    (R8_TMR2_INT_FLAG = f)

/* *
* @brief query interrupt flag status
*
* @param f - refer to TMR interrupt bit define */
#define TMR2_GetITFlag(f)      (R8_TMR2_INT_FLAG & f)

/* *
* @brief timing function initialization
*
* @param t - timing time, based on the current system clock Tsys, maximum timing period 67108864 */
void TMR3_TimerInit(uint32_t t);

/* *
* @brief Get the current timer value, maximum 67108864
*
* @return Current timer value */
#define TMR3_GetCurrentTimer()    R32_TMR3_COUNT

/* *
* @brief edge counting function initialization
*
* @param cap - Collection count type */
void TMR3_EXTSingleCounterInit(CapModeTypeDef cap);

/* *
* @brief Set count statistics overflow size, maximum 67108862
*
* @param cyc - Counting statistics overflow size */
#define TMR3_CountOverflowCfg(cyc)    (R32_TMR3_CNT_END = (cyc + 2))

/* *
* @brief Get the current count value, maximum 67108862
*
* @return Current count value */
#define TMR3_GetCurrentCount()        R32_TMR3_COUNT

/* *
* @brief PWM3 channel output waveform period configuration, maximum 67108864
*
* @param cyc - Output waveform period */
#define TMR3_PWMCycleCfg(cyc)         (R32_TMR3_CNT_END = cyc)

/* *
* @brief   PWM 输出初始化
*
* @param   pr      - select wave polar, refer to PWMX_PolarTypeDef
* @param   ts      - set pwm repeat times, refer to PWM_RepeatTsTypeDef */
void TMR3_PWMInit(PWMX_PolarTypeDef pr, PWM_RepeatTsTypeDef ts);

/* *
* @brief PWM3 Valid data pulse width, maximum 67108864
*
* @param d - Effective data pulse width */
#define TMR3_PWMActDataWidth(d)    (R32_TMR3_FIFO = d)

/* *
* @brief CAP3 Capture level timeout configuration, maximum 33554432
*
* @param cyc - Capture level timeout */
#define TMR3_CAPTimeoutCfg(cyc)    (R32_TMR3_CNT_END = cyc)

/* *
* @brief External signal capture function initialization
*
* @param cap - select capture mode, refer to CapModeTypeDef */
void TMR3_CapInit(CapModeTypeDef cap);

/* *
* @brief Get pulse data
*
* @return Pulse Data */
#define TMR3_CAPGetData()        R32_TMR3_FIFO

/* *
* @brief Get the current number of captured data
*
* @return The number of data captured currently */
#define TMR3_CAPDataCounter()    R8_TMR3_FIFO_COUNT

/* *
* @brief Close TMR3 PWM output */
#define TMR3_PWMDisable()           (R8_TMR3_CTRL_MOD &= ~RB_TMR_OUT_EN)

/* *
* @brief enables TMR3 PWM output */
#define TMR3_PWMEnable()           (R8_TMR3_CTRL_MOD |= RB_TMR_OUT_EN)

/* *
* @brief Close TMR3 */
#define TMR3_Disable()           (R8_TMR3_CTRL_MOD &= ~RB_TMR_COUNT_EN)

/* *
* @brief enable TMR3 */
#define TMR3_Enable()            (R8_TMR3_CTRL_MOD |= RB_TMR_COUNT_EN)

/* *
* @brief interrupt configuration
*
* @param s - Enable/Close
* @param f - refer to TMR interrupt bit define */
#define TMR3_ITCfg(s, f)         ((s) ? (R8_TMR3_INTER_EN |= f) : (R8_TMR3_INTER_EN &= ~f))

/* *
* @brief Clear interrupt flag
*
* @param f - refer to TMR interrupt bit define */
#define TMR3_ClearITFlag(f)      (R8_TMR3_INT_FLAG = f)

/* *
* @brief query interrupt flag status
*
* @param f - refer to TMR interrupt bit define */
#define TMR3_GetITFlag(f)        (R8_TMR3_INT_FLAG & f)

#ifdef __cplusplus
}
#endif

#endif // __CH59x_TIMER_H__
