/* ********************************* (C) COPYRIGHT ***************************
* File Name: Main.c
* Author: WCH
* Version: V1.0
* Date: 2021/03/09
* Description: Adc sampling examples, including temperature detection, single channel detection, differential channel detection, TouchKey detection, and interrupt method sampling.
************************************************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
********************************************************************************************* */

#include "CH59x_common.h"

uint16_t adcBuff[40];

volatile uint8_t adclen;
volatile uint8_t DMA_end = 0;


/* When it is necessary to measure the absolute value of high precision in applications, it is recommended to use differential mode, with one end grounded */


/* ***************************************************************************
* @fn DebugInit
*
* @brief debug initialization
*
* @return none */
void DebugInit(void)
{
    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
}

/* ***************************************************************************
* @fn main
*
* @brief main function
*
* @return none */
int main()
{

    uint8_t      i;
    signed short RoughCalib_Value = 0; // ADC coarse adjustment deviation value

    SetSysClock(CLK_SOURCE_PLL_60MHz);

    /* Configure serial debugging */
    DebugInit();
    PRINT("Start @ChipID=%02X\n", R8_CHIP_ID);
    /* Temperature sampling and output */
    PRINT("\n1.Temperature sampling...\n");
    ADC_InterTSSampInit();
    for(i = 0; i < 20; i++)
    {
        adcBuff[i] = ADC_ExcutSingleConver(); // Sampling 20 times in a row
    }
    for(i = 0; i < 20; i++)
    {
        uint32_t C25 = 0;
        C25 = (*((PUINT32)ROM_CFG_TMP_25C));
        PRINT("%d %d %d \n", adc_to_temperature_celsius(adcBuff[i]),adcBuff[i],C25);
    }

    /* Single channel sampling: select adc channel 0 for sampling, corresponding to PA4 pin, with data calibration function */
    PRINT("\n2.Single channel sampling...\n");
    GPIOA_ModeCfg(GPIO_Pin_4, GPIO_ModeIN_Floating);
    ADC_ExtSingleChSampInit(SampleFreq_3_2, ADC_PGA_0);

    RoughCalib_Value = ADC_DataCalib_Rough(); // Used to calculate the internal deviation of the ADC and record it in the global variable RoughCalib_Value
    PRINT("RoughCalib_Value =%d \n", RoughCalib_Value);

    ADC_ChannelCfg(0);

    for(i = 0; i < 20; i++)
    {
        adcBuff[i] = ADC_ExcutSingleConver() + RoughCalib_Value; // Sampling 20 times in a row
    }
    for(i = 0; i < 20; i++)
    {
        PRINT("%d \n", adcBuff[i]); // Note: Due to the existence of internal deviation of ADC, data overflow may occur when the sampling voltage is near the selected gain range limit.
    }

    /* DMA single channel sampling: select adc channel 0 for sampling, corresponding to PA4 pin */
    PRINT("\n3.Single channel DMA sampling...\n");
    GPIOA_ModeCfg(GPIO_Pin_4, GPIO_ModeIN_Floating);
    ADC_ExtSingleChSampInit(SampleFreq_3_2, ADC_PGA_0);
    ADC_ChannelCfg(0);
    ADC_AutoConverCycle(192); // The sampling period is (256-192)*16 system clocks
    ADC_DMACfg(ENABLE, (uint32_t)&adcBuff[0], (uint32_t)&adcBuff[40], ADC_Mode_Single);
    PFIC_EnableIRQ(ADC_IRQn);
    ADC_StartAutoDMA();
    while(!DMA_end);
    DMA_end = 0;
    ADC_DMACfg(DISABLE, 0, 0, 0);

    for(i = 0; i < 40; i++)
    {
        PRINT("%d \n", adcBuff[i]);
    }

    /* Differential channel sampling: select adc channel 0 for sampling, corresponding to PA4 (AIN0), PA12 (AIN2) */
    PRINT("\n4.Diff channel sampling...\n");
    GPIOA_ModeCfg(GPIO_Pin_4 | GPIO_Pin_12, GPIO_ModeIN_Floating);
    ADC_ExtDiffChSampInit(SampleFreq_3_2, ADC_PGA_0);
    ADC_ChannelCfg(0);
    for(i = 0; i < 20; i++)
    {
        adcBuff[i] = ADC_ExcutSingleConver(); // Sampling 20 times in a row
    }
    for(i = 0; i < 20; i++)
    {
        PRINT("%d \n", adcBuff[i]);
    }

    /* TouchKey sampling: select adc channel 2 for sampling, corresponding to PA12 */
    PRINT("\n5.TouchKey sampling...\n");
    GPIOA_ModeCfg(GPIO_Pin_12, GPIO_ModeIN_Floating);
    TouchKey_ChSampInit();
    ADC_ChannelCfg(2);

    for(i = 0; i < 20; i++)
    {
        adcBuff[i] = TouchKey_ExcutSingleConver(0x10, 0); // Sampling 20 times in a row
    }
    for(i = 0; i < 20; i++)
    {
        PRINT("%d \n", adcBuff[i]);
    }

    /* Single channel sampling: interrupt mode, select adc channel 1 for sampling, corresponding to PA5 pin, without data calibration function */
    PRINT("\n6.Single channel sampling in interrupt mode...\n");
    GPIOA_ModeCfg(GPIO_Pin_5, GPIO_ModeIN_Floating);
    ADC_ExtSingleChSampInit(SampleFreq_3_2, ADC_PGA_0);
    ADC_ChannelCfg(1);
    adclen = 0;
    ADC_ClearITFlag();
    PFIC_EnableIRQ(ADC_IRQn);

    ADC_StartUp();
    while(adclen < 20);
    PFIC_DisableIRQ(ADC_IRQn);
    for(i = 0; i < 20; i++)
    {
        PRINT("%d \n", adcBuff[i]);
    }

    while(1);
}

/* ***************************************************************************
* @fn ADC_IRQHandler
*
* @brief ADC interrupt function
*
* @return none */
__INTERRUPT
__HIGH_CODE
void ADC_IRQHandler(void) // adc interrupt service program
{
    if(ADC_GetDMAStatus())
    {
        ADC_StopAutoDMA();
        R16_ADC_DMA_BEG = ((uint32_t)adcBuff) & 0xffff;
        ADC_ClearDMAFlag();
        DMA_end = 1;
    }
    if(ADC_GetITStatus())
    {
        ADC_ClearITFlag();
        if(adclen < 20)
        {
            adcBuff[adclen] = ADC_ReadConverValue();
            ADC_StartUp(); // Use to clear the interrupt flag and start a new round of sampling
        }
        adclen++;
    }
}
