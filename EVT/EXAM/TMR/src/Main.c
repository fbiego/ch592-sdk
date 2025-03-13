/* ********************************* (C) COPYRIGHT *******************************
* File Name          : Main.c
* Author             : WCH
* Version            : V1.0
* Date               : 2020/08/06
* Description        : ¶¨Ê±Æ÷Àý×Ó
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
****************************************************************************** */

#include "CH59x_common.h"

__attribute__((aligned(4))) uint32_t CapBuf[100];
__attribute__((aligned(4))) uint32_t PwmBuf[100];

volatile uint8_t capFlag = 0;

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
    uint8_t i;

    SetSysClock(CLK_SOURCE_PLL_60MHz);

    /* Configure serial debugging */
    DebugInit();
    PRINT("Start @ChipID=%02X\n", R8_CHIP_ID);

#if 1 /* Timer 0, set the 100ms timer to perform IO port flashing, PB15-LED */

    GPIOB_SetBits(GPIO_Pin_15);
    GPIOB_ModeCfg(GPIO_Pin_15, GPIO_ModeOut_PP_5mA);

    TMR0_TimerInit(FREQ_SYS / 10);         // Set the timing time 100ms
    TMR0_ITCfg(ENABLE, TMR0_3_IT_CYC_END); // Turn on interrupt
    PFIC_EnableIRQ(TMR0_IRQn);
#endif

#if 1 /* Timer 3, PWM output */

    GPIOB_ResetBits(GPIO_Pin_22); // Configure PWM port PB22
    GPIOB_ModeCfg(GPIO_Pin_22, GPIO_ModeOut_PP_5mA);
    TMR3_PWMInit(High_Level, PWM_Times_1);
    TMR3_PWMCycleCfg(60 * 100); // Cycle 100us Maximum 67108864
    TMR3_PWMActDataWidth(3000); // Duty cycle: 50%, the timer must be temporarily turned off when modifying the duty cycle
    TMR3_PWMEnable();
    TMR3_Enable();

#endif

#if 1                                      /* Timer 1, CAP capture, */
    PWR_UnitModCfg(DISABLE, UNIT_SYS_LSE); // Note that this pin is the LSE crystal oscillator pin, and other functions must be turned off before use.
    GPIOA_ResetBits(GPIO_Pin_10);          // Configure PWM port PA10
    GPIOA_ModeCfg(GPIO_Pin_10, GPIO_ModeIN_PU);

    TMR1_CapInit(Edge_To_Edge);
    TMR1_CAPTimeoutCfg(0xFFFFFFFF); // Set the capture timeout
    TMR1_DMACfg(ENABLE, (uint16_t)(uint32_t)&CapBuf[0], (uint16_t)(uint32_t)&CapBuf[100], Mode_Single);
    TMR1_ITCfg(ENABLE, TMR1_2_IT_DMA_END); // Turn on DMA to complete interrupt
    PFIC_EnableIRQ(TMR1_IRQn);

    while(capFlag == 0);
    capFlag = 0;
    for(i = 0; i < 100; i++)
    {
        PRINT("%08ld ", CapBuf[i] & 0x1ffffff); // 26bit, the highest bit indicates whether it is high or low
    }
    PRINT("\n");

#endif

#if 1 /* Timer 2, counter, */
    GPIOB_ModeCfg(GPIO_Pin_11, GPIO_ModeIN_PD);
    GPIOPinRemap(ENABLE, RB_PIN_TMR2);

    TMR2_EXTSingleCounterInit(FallEdge_To_FallEdge);
    TMR2_CountOverflowCfg(1000); // Set the upper limit of counting 1000

    /* Turn on the count overflow interrupt, and the counting is slowed down by 1000 cycles and enters the interrupt */
    TMR2_ClearITFlag(TMR0_3_IT_CYC_END);
    PFIC_EnableIRQ(TMR2_IRQn);
    TMR2_ITCfg(ENABLE, TMR0_3_IT_CYC_END);

    do
    {
        /* Print the current count value once in 1s. If the frequency of the input pulse is high, the counting may overflow quickly and need to be modified according to the actual situation. */
        mDelaymS(1000);
        PRINT("=%ld \n", TMR2_GetCurrentCount());
    } while(1);

#endif

#if 1 /* Timer 2, DMA PWM. */
    GPIOB_ModeCfg(GPIO_Pin_11, GPIO_ModeOut_PP_5mA);
    GPIOPinRemap(ENABLE, RB_PIN_TMR2);

    PRINT("TMR2 DMA PWM\n");
    TMR2_PWMCycleCfg(60 * 2000); // Period 2000us The main frequency is 60Mhz, oscillating 60M times per second, oscillating 60 times is 1 microsecond
    for(i=0; i<50; i++)
    {
      PwmBuf[i]=2400*i;
    }
    for(i=50; i<100; i++)
    {
      PwmBuf[i]=2400*(100-i);
    }
    TMR2_PWMInit(Low_Level, PWM_Times_16);
    TMR2_DMACfg(ENABLE, (uint32_t)&PwmBuf[0], (uint32_t)&PwmBuf[100], Mode_LOOP);
    TMR2_PWMEnable();
    TMR2_Enable();
    /* Turn on the count overflow interrupt, enter the interrupt after 100 cycles */
    TMR2_ClearITFlag(TMR1_2_IT_DMA_END);
    TMR2_ITCfg(ENABLE, TMR1_2_IT_DMA_END);
    PFIC_EnableIRQ(TMR2_IRQn);

#endif

    while(1);
}

/* ***************************************************************************
* @fn TMR0_IRQHandler
*
* @brief TMR0 interrupt function
*
* @return none */
__INTERRUPT
__HIGH_CODE
void TMR0_IRQHandler(void) // TMR0 Timed interrupt
{
    if(TMR0_GetITFlag(TMR0_3_IT_CYC_END))
    {
        TMR0_ClearITFlag(TMR0_3_IT_CYC_END); // Clear the interrupt flag
        GPIOB_InverseBits(GPIO_Pin_15);
    }
}

/* ***************************************************************************
* @fn TMR1_IRQHandler
*
* @brief TMR1 interrupt function
*
* @return none */
__INTERRUPT
__HIGH_CODE
void TMR1_IRQHandler(void) // TMR1 timing interrupt
{
    if(TMR1_GetITFlag(TMR1_2_IT_DMA_END))
    {
        TMR1_ITCfg(DISABLE, TMR1_2_IT_DMA_END); // Use single DMA function + interrupt, please turn off this interrupt enable after completion, otherwise the interrupt will be reported.
        TMR1_ClearITFlag(TMR1_2_IT_DMA_END);    // Clear the interrupt flag
        capFlag = 1;
    }
}

/* ***************************************************************************
* @fn TMR2_IRQHandler
*
* @brief TMR2 interrupt function
*
* @return none */
__INTERRUPT
__HIGH_CODE
void TMR2_IRQHandler(void)
{
    if(TMR2_GetITFlag(TMR0_3_IT_CYC_END))
    {
        TMR2_ClearITFlag(TMR0_3_IT_CYC_END);
        /* The counter is full, the hardware is automatically cleared, and counting is restarted */
        /* Users can add the required processing by themselves */
    }
    if(TMR2_GetITFlag(TMR1_2_IT_DMA_END))
    {
        TMR2_ClearITFlag(TMR1_2_IT_DMA_END);
        PRINT("DMA end\n");
        /* DMA End */
        /* Users can add the required processing by themselves */
    }
}
