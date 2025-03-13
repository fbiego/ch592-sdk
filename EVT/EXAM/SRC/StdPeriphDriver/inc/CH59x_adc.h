/********************************** (C) COPYRIGHT *******************************
 * File Name          : CH59x_adc.h
 * Author             : WCH
 * Version            : V1.2
 * Date               : 2021/11/17
 * Description
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#ifndef __CH59x_ADC_H__
#define __CH59x_ADC_H__

#ifdef __cplusplus
extern "C" {
#endif

#define ROM_CFG_TMP_25C    0x7F014

/**
 * @brief  adc single channel define
 */
typedef enum
{
    CH_EXTIN_0 = 0,   // ADC external analog channel 0
    CH_EXTIN_1,       // ADC External Analog Channel 1
    CH_EXTIN_2,       // ADC External Analog Channel 2
    CH_EXTIN_3,       // ADC External Analog Channel 3
    CH_EXTIN_4,       // ADC External Analog Channel 4
    CH_EXTIN_5,       // ADC External Analog Channel 5
    CH_EXTIN_6,       // ADC External Analog Channel 6
    CH_EXTIN_7,       // ADC External Analog Channel 7
    CH_EXTIN_8,       // ADC External Analog Channel 8
    CH_EXTIN_9,       // ADC External Analog Channel 9
    CH_EXTIN_10,      // ADC External Analog Channel 10
    CH_EXTIN_11,      // ADC External Analog Channel 11
    CH_EXTIN_12,      // ADC External Analog Channel 12
    CH_EXTIN_13,      // ADC External Analog Channel 13

    CH_INTE_VBAT = 14,  // ADC internal battery detection channel
    CH_INTE_VTEMP = 15, // ADC internal temperature sensor detection channel

} ADC_SingleChannelTypeDef;

/**
 * @brief  adc differential channel define
 */
typedef enum
{
    CH_DIFF_0_2 = 0, // ADC Differential Channel #0-#2
    CH_DIFF_1_3,     // ADC Differential Channel #1-#3

} ADC_DiffChannelTypeDef;

/**
 * @brief  adc sampling clock
 */
typedef enum
{
    SampleFreq_3_2 = 0, // 3.2M sampling frequency
    SampleFreq_8,       // 8M sampling frequency
    SampleFreq_5_33,    // 5.33M sampling frequency
    SampleFreq_4,       // 4M sampling frequency
} ADC_SampClkTypeDef;

/**
 * @brief  adc signal PGA
 */
typedef enum
{
    ADC_PGA_1_4 = 0,    // -12dB, 1/4 times
    ADC_PGA_1_2,        // -6dB, 1/2 times
    ADC_PGA_0,          // 0dB, ​​1 times, no gain
    ADC_PGA_2,          // 6dB, 2x
    ADC_PGA_2_ = 0x10,  // 6dB, 2x
    ADC_PGA_4,          // 12dB, 4 times
    ADC_PGA_8,          // 18dB, 8 times
    ADC_PGA_16,         // 24dB, 16 times
} ADC_SignalPGATypeDef;

/**
 * @brief  Configuration DMA mode
 */
typedef enum
{
    ADC_Mode_Single = 0, // Single-time mode
    ADC_Mode_LOOP,       // Loop mode
} ADC_DMAModeTypeDef;


/* *
* @brief Setting the ADC sampling channel
*
* @param d - refer to ADC_SingleChannelTypeDef */
#define ADC_ChannelCfg(d)      (R8_ADC_CHANNEL = d)

/* *
* @brief Set the ADC sampling clock
*
* @param d - refer to ADC_SampClkTypeDef */
#define ADC_SampClkCfg(d)      (R8_ADC_CFG = R8_ADC_CFG & (~RB_ADC_CLK_DIV) | (d << 6))

/* *
* @brief Set ADC signal gain
*
* @param d - refer to ADC_SignalPGATypeDef */
#define ADC_PGACfg(d)          (R8_ADC_CFG = R8_ADC_CFG & (~RB_ADC_PGA_GAIN) | (d << 4))

/* *
* @brief Set the internal temperature sensor calibration value
*
* @param d - calibration value */
#define ADC_TempCalibCfg(d)    (R8_TEM_SENSOR = R8_TEM_SENSOR & (~RB_TEM_SEN_CALIB) | d)

/* *
* @brief External signal single channel sampling initialization
*
* @param sp - refer to ADC_SampClkTypeDef
* @param ga - refer to ADC_SignalPGATypeDef */
void ADC_ExtSingleChSampInit(ADC_SampClkTypeDef sp, ADC_SignalPGATypeDef ga);

/* *
* @brief External signal differential channel sampling initialization
*
* @param sp - refer to ADC_SampClkTypeDef
* @param ga - refer to ADC_SignalPGATypeDef */
void ADC_ExtDiffChSampInit(ADC_SampClkTypeDef sp, ADC_SignalPGATypeDef ga);

/* *
* @brief Touch key channel sampling initialization */
void TouchKey_ChSampInit(void);

/* *
* @brief Turn off TouchKey power */
#define TouchKey_DisableTSPower()    (R8_TKEY_CFG &= ~RB_TKEY_PWR_ON)

/* *
* @brief Built-in temperature sensor sampling initialization */
void ADC_InterTSSampInit(void);

/* *
* @brief Turn off the temperature sensor */
#define ADC_DisableTSPower()    (R8_TEM_SENSOR = 0)

/* *
* @brief Built-in battery voltage sampling initialization */
void ADC_InterBATSampInit(void);

/* *
* @brief ADC performs a single conversion
*
* @return ADC converted data */
uint16_t ADC_ExcutSingleConver(void);

/* *
* @brief sample data to roughly tune, get the deviation value, you must first configure the ADC and then call this function to get the calibration value
*
* @return deviation */
signed short ADC_DataCalib_Rough(void);

/* *
* @brief TouchKey converted data
*
* @param charg - Touchkey charging time, 5bits valid, t=charg*Tadc
* @param disch - Touchkey discharge time, 3bits valid, t=disch*Tadc
*
* @return Current TouchKey equivalent data */
uint16_t TouchKey_ExcutSingleConver(uint8_t charg, uint8_t disch);

/* *
* @brief Sets the cycle of continuous ADC
*
* @param cycle - Unit is 16 system clocks */
void ADC_AutoConverCycle(uint8_t cycle);

/* *
* @brief Configure DMA function
*
* @param s - Whether to turn on the DMA function
* @param startAddr - DMA Start Address
* @param endAddr - DMA end address
* @param m - Configure DMA mode */
void ADC_DMACfg(uint8_t s, uint32_t startAddr, uint32_t endAddr, ADC_DMAModeTypeDef m);

/**
 * @brief   Convert ADC value to temperature(Celsius)
 *
 * @param   adc_val - adc value
 *
 * @return  temperature (Celsius)
 */
int adc_to_temperature_celsius(uint16_t adc_val);

/* *
* @brief Get ADC conversion value
*
* @return ADC conversion value */
#define ADC_ReadConverValue()     (R16_ADC_DATA)

/* *
* @brief ADC performs a single conversion */
#define ADC_StartUp()             (R8_ADC_CONVERT |= RB_ADC_START)

/* *
* @brief Get ADC interrupt status */
#define ADC_GetITStatus()         (R8_ADC_INT_FLAG & RB_ADC_IF_EOC)

/* *
* @brief Clear ADC interrupt flag */
#define ADC_ClearITFlag()         (R8_ADC_CONVERT = R8_ADC_CONVERT)

/* *
* @brief Get the ADC DMA completion status */
#define ADC_GetDMAStatus()        (R8_ADC_DMA_IF & RB_ADC_IF_DMA_END)

/* *
* @brief Clear ADC DMA completion flag */
#define ADC_ClearDMAFlag()        (R8_ADC_DMA_IF |= RB_ADC_IF_DMA_END)

/* *
* @brief Turn on the automatic continuous ADC at a fixed time interval */
#define ADC_StartAutoDMA()        (R8_ADC_CTRL_DMA |= RB_ADC_AUTO_EN)

/* *
* @brief Stop the time interval automatically continuous ADC */
#define ADC_StopAutoDMA()         (R8_ADC_CTRL_DMA &= ~RB_ADC_AUTO_EN)

/* *
* @brief Enable continuous conversion ADC */
#define ADC_StartContDMA()        (R8_ADC_CTRL_DMA |= RB_ADC_CONT_EN)

/* *
* @brief Stop continuous conversion ADC */
#define ADC_StopContDMA()         (R8_ADC_CTRL_DMA &= ~RB_ADC_CONT_EN)

/* *
* @brief Turn off the ADC */
#define ADC_DisablePower()        (R8_ADC_CFG &= ~RB_ADC_POWER_ON)

#ifdef __cplusplus
}
#endif

#endif // __CH59x_ADC_H__
