/********************************** (C) COPYRIGHT *******************************
 * File Name          : CH59x_uart0.c
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
* @fn UART0_DefInit
*
* @brief The default initialization configuration of the serial port
*
* @param none
*
* @return none */
void UART0_DefInit(void)
{
    UART0_BaudRateCfg(115200);
    R8_UART0_FCR = (2 << 6) | RB_FCR_TX_FIFO_CLR | RB_FCR_RX_FIFO_CLR | RB_FCR_FIFO_EN; // FIFO is turned on, trigger point 4 bytes
    R8_UART0_LCR = RB_LCR_WORD_SZ;
    R8_UART0_IER = RB_IER_TXD_EN;
    R8_UART0_DIV = 1;
}

/* ***************************************************************************
* @fn UART0_BaudRateCfg
*
* @brief Serial port baud rate configuration
*
* @param baudrate - baudrate
*
* @return none */
void UART0_BaudRateCfg(uint32_t baudrate)
{
    uint32_t x;

    x = 10 * GetSysClock() / 8 / baudrate;
    x = (x + 5) / 10;
    R16_UART0_DL = (uint16_t)x;
}

/* ***************************************************************************
* @fn UART0_ByteTrigCfg
*
* @brief Serial port byte trigger interrupt configuration
*
* @param b - trigger byte count refer to UARTByteTRIGTypeDef
*
* @return none */
void UART0_ByteTrigCfg(UARTByteTRIGTypeDef b)
{
    R8_UART0_FCR = (R8_UART0_FCR & ~RB_FCR_FIFO_TRIG) | (b << 6);
}

/*********************************************************************
 * @fn      UART0_INTCfg
 *
 * @brief   串口中断配置
 *
 * @param   s       - 中断控制状态，是否使能相应中断
 * @param   i       - 中断类型
 *                    RB_IER_MODEM_CHG  - 调制解调器输入状态变化中断使能位（仅 UART0 支持）
 *                    RB_IER_LINE_STAT  - 接收线路状态中断
 *                    RB_IER_THR_EMPTY  - 发送保持寄存器空中断
 *                    RB_IER_RECV_RDY   - 接收数据中断
 *
 * @return  none
 */
void UART0_INTCfg(FunctionalState s, uint8_t i)
{
    if(s)
    {
        R8_UART0_IER |= i;
        R8_UART0_MCR |= RB_MCR_INT_OE;
    }
    else
    {
        R8_UART0_IER &= ~i;
    }
}

/*********************************************************************
 * @fn      UART0_Reset
 *
 * @brief   串口软件复位
 *
 * @param   none
 *
 * @return  none
 */
void UART0_Reset(void)
{
    R8_UART0_IER = RB_IER_RESET;
}

/* ***************************************************************************
* @fn UART0_SendString
*
* @brief Serial port multibyte send
*
* @param buf - The first address of the data content to be sent
* @param l - length of data to be sent
*
* @return none */
void UART0_SendString(uint8_t *buf, uint16_t l)
{
    uint16_t len = l;

    while(len)
    {
        if(R8_UART0_TFC != UART_FIFO_SIZE)
        {
            R8_UART0_THR = *buf++;
            len--;
        }
    }
}

/* ***************************************************************************
* @fn UART0_RecvString
*
* @brief read multibytes on the serial port
*
* @param buf - Read data storage cache area first address
*
* @return Read data length */
uint16_t UART0_RecvString(uint8_t *buf)
{
    uint16_t len = 0;

    while(R8_UART0_RFC)
    {
        *buf++ = R8_UART0_RBR;
        len++;
    }

    return (len);
}

