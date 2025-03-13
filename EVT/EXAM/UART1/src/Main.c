/* ********************************* (C) COPYRIGHT *******************************
* File Name          : Main.c
* Author             : WCH
* Version            : V1.0
* Date               : 2020/08/06
* Description        : 串口1收发演示
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
****************************************************************************** */

#include "CH59x_common.h"

uint8_t TxBuff[] = "This is a tx exam\r\n";
uint8_t RxBuff[100];
uint8_t trigB;

/* ***************************************************************************
* @fn main
*
* @brief main function
*
* @return none */
int main()
{
    uint8_t len;

    SetSysClock(CLK_SOURCE_PLL_60MHz);

    /* Configure serial port 1: first configure IO port mode, then configure serial port */
    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);      // RXD-Configure pull-up input
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA); // TXD-Configure push-pull output, be careful to let the IO port output high level first
    UART1_DefInit();

#if 1 // Test serial send string
    UART1_SendString(TxBuff, sizeof(TxBuff));

#endif

#if 1 // Query method: send it out after receiving the data
    while(1)
    {
        len = UART1_RecvString(RxBuff);
        if(len)
        {
            UART1_SendString(RxBuff, len);
        }
    }

#endif

#if 0 // Interrupt method: send it out after receiving the data
    UART1_ByteTrigCfg(UART_7BYTE_TRIG);
    trigB = 7;
    UART1_INTCfg(ENABLE, RB_IER_RECV_RDY | RB_IER_LINE_STAT);
    PFIC_EnableIRQ(UART1_IRQn);
#endif

    while(1);
}

/* ***************************************************************************
* @fn UART1_IRQHandler
*
* @brief UART1 interrupt function
*
* @return none */
__INTERRUPT
__HIGH_CODE
void UART1_IRQHandler(void)
{
    volatile uint8_t i;

    switch(UART1_GetITFlag())
    {
        case UART_II_LINE_STAT: // Line status error
        {
            UART1_GetLinSTA();
            break;
        }

        case UART_II_RECV_RDY: // The data reaches the setting trigger point
            for(i = 0; i != trigB; i++)
            {
                RxBuff[i] = UART1_RecvByte();
                UART1_SendByte(RxBuff[i]);
            }
            break;

        case UART_II_RECV_TOUT: // Receive timeout, temporary data reception is completed
            i = UART1_RecvString(RxBuff);
            UART1_SendString(RxBuff, i);
            break;

        case UART_II_THR_EMPTY: // The sending buffer area is empty, and you can continue to send
            break;

        case UART_II_MODEM_CHG: // Only support serial port 0
            break;

        default:
            break;
    }
}
