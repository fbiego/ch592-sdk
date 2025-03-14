/********************************** (C) COPYRIGHT *******************************
 * File Name          : CH59x_uart1.c
 * Author             : WCH
 * Version            : V1.2
 * Date               : 2021/11/17
 * Description
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#include "CH59x_common.h"

/* ***************************************************************************
* @fn UART1_DefInit
*
* @brief The default initialization configuration of the serial port
*
* @param none
*
* @return none */
void UART1_DefInit(void)
{
    UART1_BaudRateCfg(115200);
    R8_UART1_FCR = (2 << 6) | RB_FCR_TX_FIFO_CLR | RB_FCR_RX_FIFO_CLR | RB_FCR_FIFO_EN; // FIFO打开，触发点4字节
    R8_UART1_LCR = RB_LCR_WORD_SZ;
    R8_UART1_IER = RB_IER_TXD_EN;
    R8_UART1_DIV = 1;
}

/* ***************************************************************************
* @fn UART1_BaudRateCfg
*
* @brief Serial port baud rate configuration
*
* @param baudrate - baudrate
*
* @return none */
void UART1_BaudRateCfg(uint32_t baudrate)
{
    uint32_t x;

    x = 10 * GetSysClock() / 8 / baudrate;
    x = (x + 5) / 10;
    R16_UART1_DL = (uint16_t)x;
}

/* ***************************************************************************
* @fn UART1_ByteTrigCfg
*
* @brief Serial port byte trigger interrupt configuration
*
* @param b - trigger byte count refer to UARTByteTRIGTypeDef
*
* @return none */
void UART1_ByteTrigCfg(UARTByteTRIGTypeDef b)
{
    R8_UART1_FCR = (R8_UART1_FCR & ~RB_FCR_FIFO_TRIG) | (b << 6);
}

/* ***************************************************************************
* @fn UART1_INTCfg
*
* @brief Serial port interrupt configuration
*
* @param s - Interrupt control status, whether corresponding interrupt can be enabled
* @param i - interrupt type
* RB_IER_MODEM_CHG - Modem input state change interrupt enable bit (only supported by UART0)
* RB_IER_LINE_STAT - Receive line status interrupt
* RB_IER_THR_EMPTY - Send hold register air interrupt
* RB_IER_RECV_RDY - Received data interrupt
*
* @return none */
void UART1_INTCfg(FunctionalState s, uint8_t i)
{
    if(s)
    {
        R8_UART1_IER |= i;
        R8_UART1_MCR |= RB_MCR_INT_OE;
    }
    else
    {
        R8_UART1_IER &= ~i;
    }
}

/* ***************************************************************************
* @fn UART1_Reset
*
* @brief serial port software reset
*
* @param none
*
* @return none */
void UART1_Reset(void)
{
    R8_UART1_IER = RB_IER_RESET;
}

/*********************************************************************
 * @fn      UART1_SendString
 *
 * @brief   串口多字节发送
 *
 * @param   buf     - 待发送的数据内容首地址
 * @param   l       - 待发送的数据长度
 *
 * @return  none
 */
void UART1_SendString(uint8_t *buf, uint16_t l)
{
    uint16_t len = l;

    while(len)
    {
        if(R8_UART1_TFC != UART_FIFO_SIZE)
        {
            R8_UART1_THR = *buf++;
            len--;
        }
    }
}

/* ***************************************************************************
* @fn UART1_RecvString
*
* @brief read multibytes on the serial port
*
* @param buf - Read data storage cache area first address
*
* @return Read data length */
uint16_t UART1_RecvString(uint8_t *buf)
{
    uint16_t len = 0;

    while(R8_UART1_RFC)
    {
        *buf++ = R8_UART1_RBR;
        len++;
    }

    return (len);
}
