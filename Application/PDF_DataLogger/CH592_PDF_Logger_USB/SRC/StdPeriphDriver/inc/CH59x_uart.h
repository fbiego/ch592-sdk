/********************************** (C) COPYRIGHT *******************************
 * File Name          : CH59x_uart.h
 * Author             : WCH
 * Version            : V1.2
 * Date               : 2021/11/17
 * Description
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#ifndef __CH59x_UART_H__
#define __CH59x_UART_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief	LINE error and status define
 */
#define STA_ERR_BREAK     RB_LSR_BREAK_ERR    // Data interval error
#define STA_ERR_FRAME     RB_LSR_FRAME_ERR    // Data frame error
#define STA_ERR_PAR       RB_LSR_PAR_ERR      // 奇偶校验位出错
#define STA_ERR_FIFOOV    RB_LSR_OVER_ERR     // Received data overflow

#define STA_TXFIFO_EMP    RB_LSR_TX_FIFO_EMP  // The current sending FIFO is empty, and the sending data can be continued to be filled.
#define STA_TXALL_EMP     RB_LSR_TX_ALL_EMP   // All currently sent data are sent
#define STA_RECV_DATA     RB_LSR_DATA_RDY     // Data is currently received

/**
 * @brief  Configuration UART TrigByte num
 */
typedef enum
{
    UART_1BYTE_TRIG = 0, // 1 byte trigger
    UART_2BYTE_TRIG,     // 2 byte trigger
    UART_4BYTE_TRIG,     // 4 byte trigger
    UART_7BYTE_TRIG,     // 7字节触发

} UARTByteTRIGTypeDef;

/* *
* @brief The default initialization configuration of the serial port */
void UART0_DefInit(void);

/**
 * @brief   串口波特率配置
 *
 * @param   baudrate    - 波特率
 */
void UART0_BaudRateCfg(uint32_t baudrate);

/* *
* @brief Serial port byte trigger interrupt configuration
*
* @param b - trigger byte count refer to UARTByteTRIGTypeDef */
void UART0_ByteTrigCfg(UARTByteTRIGTypeDef b);

/**
 * @brief   串口中断配置
 *
 * @param   s       - 中断控制状态，是否使能相应中断
 * @param   i       - 中断类型
 *                    RB_IER_MODEM_CHG  - 调制解调器输入状态变化中断使能位（仅 UART0 支持）
 *                    RB_IER_LINE_STAT  - 接收线路状态中断
 *                    RB_IER_THR_EMPTY  - 发送保持寄存器空中断
 *                    RB_IER_RECV_RDY   - 接收数据中断
 */
void UART0_INTCfg(FunctionalState s, uint8_t i);

/* *
* @brief serial port software reset */
void UART0_Reset(void);

/* *
* @brief Clear the currently received FIFO */
#define UART0_CLR_RXFIFO()    (R8_UART0_FCR |= RB_FCR_RX_FIFO_CLR)

/* *
* @brief Clear the currently sent FIFO */
#define UART0_CLR_TXFIFO()    (R8_UART0_FCR |= RB_FCR_TX_FIFO_CLR)

/* *
* @brief Get the current interrupt flag
*
* @return Current interrupt flag */
#define UART0_GetITFlag()     (R8_UART0_IIR & RB_IIR_INT_MASK)

/**
 * @brief   获取当前通讯状态
 *
 * @return  refer to LINE error and status define
 */
#define UART0_GetLinSTA()     (R8_UART0_LSR)

/**
 * @brief   串口单字节发送
 *
 * @param   b       待发送的字节
 */
#define UART0_SendByte(b)     (R8_UART0_THR = b)

/**
 * @brief   串口多字节发送
 *
 * @param   buf     - 待发送的数据内容首地址
 * @param   l       - 待发送的数据长度
 */
void UART0_SendString(uint8_t *buf, uint16_t l);

/* *
* @brief read single byte on the serial port
*
* @return Read single byte */
#define UART0_RecvByte()    (R8_UART0_RBR)

/* *
* @brief read multibytes on the serial port
*
* @param buf - Read data storage cache area first address
*
* @return Read data length */
uint16_t UART0_RecvString(uint8_t *buf);

/**
 * @brief   串口默认初始化配置
 */
void UART1_DefInit(void);

/* *
* @brief Serial port baud rate configuration
*
* @param baudrate - baudrate */
void UART1_BaudRateCfg(uint32_t baudrate);

/* *
* @brief Serial port byte trigger interrupt configuration
*
* @param b - trigger byte count refer to UARTByteTRIGTypeDef */
void UART1_ByteTrigCfg(UARTByteTRIGTypeDef b);

/* *
* @brief Serial port interrupt configuration
*
* @param s - Interrupt control status, whether corresponding interrupt can be enabled
* @param i - interrupt type
* RB_IER_MODEM_CHG - Modem input state change interrupt enable bit (only supported by UART0)
* RB_IER_LINE_STAT - Receive line status interrupt
* RB_IER_THR_EMPTY - Send hold register air interrupt
* RB_IER_RECV_RDY - Received data interrupt */
void UART1_INTCfg(FunctionalState s, uint8_t i);

/**
 * @brief   串口软件复位
 */
void UART1_Reset(void);

/* *
* @brief Clear the currently received FIFO */
#define UART1_CLR_RXFIFO()    (R8_UART1_FCR |= RB_FCR_RX_FIFO_CLR)

/* *
* @brief Clear the currently sent FIFO */
#define UART1_CLR_TXFIFO()    (R8_UART1_FCR |= RB_FCR_TX_FIFO_CLR)

/**
 * @brief   获取当前中断标志
 *
 * @return  当前中断标志
 */
#define UART1_GetITFlag()     (R8_UART1_IIR & RB_IIR_INT_MASK)

/**
 * @brief   获取当前通讯状态
 *
 * @return  refer to LINE error and status define
 */
#define UART1_GetLinSTA()     (R8_UART1_LSR)

/**
 * @brief   串口单字节发送
 *
 * @param   b       待发送的字节
 */
#define UART1_SendByte(b)     (R8_UART1_THR = b)

/**
 * @brief   串口多字节发送
 *
 * @param   buf     - 待发送的数据内容首地址
 * @param   l       - 待发送的数据长度
 */
void UART1_SendString(uint8_t *buf, uint16_t l);

/**
 * @brief   串口读取单字节
 *
 * @return  读取到的单字节
 */
#define UART1_RecvByte()    (R8_UART1_RBR)

/* *
* @brief read multibytes on the serial port
*
* @param buf - Read data storage cache area first address
*
* @return Read data length */
uint16_t UART1_RecvString(uint8_t *buf);

/* *
* @brief The default initialization configuration of the serial port */
void UART2_DefInit(void);

/* *
* @brief Serial port baud rate configuration
*
* @param baudrate - baudrate */
void UART2_BaudRateCfg(uint32_t baudrate);

/* *
* @brief Serial port byte trigger interrupt configuration
*
* @param b - trigger byte count refer to UARTByteTRIGTypeDef */
void UART2_ByteTrigCfg(UARTByteTRIGTypeDef b);

/* *
* @brief Serial port interrupt configuration
*
* @param s - Interrupt control status, whether corresponding interrupt can be enabled
* @param i - interrupt type
* RB_IER_MODEM_CHG - Modem input state change interrupt enable bit (only supported by UART0)
* RB_IER_LINE_STAT - Receive line status interrupt
* RB_IER_THR_EMPTY - Send hold register air interrupt
* RB_IER_RECV_RDY - Received data interrupt */
void UART2_INTCfg(FunctionalState s, uint8_t i);

/* *
* @brief serial port software reset */
void UART2_Reset(void);

/* *
* @brief Clear the currently received FIFO */
#define UART2_CLR_RXFIFO()    (R8_UART2_FCR |= RB_FCR_RX_FIFO_CLR)

/**
 * @brief   清除当前发送FIFO
 */
#define UART2_CLR_TXFIFO()    (R8_UART2_FCR |= RB_FCR_TX_FIFO_CLR)

/* *
* @brief Get the current interrupt flag
*
* @return Current interrupt flag */
#define UART2_GetITFlag()     (R8_UART2_IIR & RB_IIR_INT_MASK)

/**
 * @brief   获取当前通讯状态
 *
 * @return  refer to LINE error and status define
 */
#define UART2_GetLinSTA()     (R8_UART2_LSR)

/* *
* @brief Serial port single byte send
*
* @param b Bytes to be sent */
#define UART2_SendByte(b)     (R8_UART2_THR = b)

/**
 * @brief   串口多字节发送
 *
 * @param   buf     - 待发送的数据内容首地址
 * @param   l       - 待发送的数据长度
 */
void UART2_SendString(uint8_t *buf, uint16_t l);

/**
 * @brief   串口读取单字节
 *
 * @return  读取到的单字节
 */
#define UART2_RecvByte()    (R8_UART2_RBR)

/* *
* @brief read multibytes on the serial port
*
* @param buf - Read data storage cache area first address
*
* @return Read data length */
uint16_t UART2_RecvString(uint8_t *buf);

/* *
* @brief The default initialization configuration of the serial port */
void UART3_DefInit(void);

/**
 * @brief   串口波特率配置
 *
 * @param   baudrate    - 波特率
 */
void UART3_BaudRateCfg(uint32_t baudrate);

/* *
* @brief Serial port byte trigger interrupt configuration
*
* @param b - trigger byte count refer to UARTByteTRIGTypeDef */
void UART3_ByteTrigCfg(UARTByteTRIGTypeDef b);

/* *
* @brief Serial port interrupt configuration
*
* @param s - Interrupt control status, whether corresponding interrupt can be enabled
* @param i - interrupt type
* RB_IER_MODEM_CHG - Modem input state change interrupt enable bit (only supported by UART0)
* RB_IER_LINE_STAT - Receive line status interrupt
* RB_IER_THR_EMPTY - Send hold register air interrupt
* RB_IER_RECV_RDY - Received data interrupt */
void UART3_INTCfg(FunctionalState s, uint8_t i);

/* *
* @brief serial port software reset */
void UART3_Reset(void);

/* *
* @brief Clear the currently received FIFO */
#define UART3_CLR_RXFIFO()    (R8_UART3_FCR |= RB_FCR_RX_FIFO_CLR)

/* *
* @brief Clear the currently sent FIFO */
#define UART3_CLR_TXFIFO()    (R8_UART3_FCR |= RB_FCR_TX_FIFO_CLR)

/**
 * @brief   获取当前中断标志
 *
 * @return  当前中断标志
 */
#define UART3_GetITFlag()     (R8_UART3_IIR & RB_IIR_INT_MASK)

/**
 * @brief   获取当前通讯状态
 *
 * @return  refer to LINE error and status define
 */
#define UART3_GetLinSTA()     (R8_UART3_LSR)

/**
 * @brief   串口单字节发送
 *
 * @param   b       待发送的字节
 */
#define UART3_SendByte(b)     (R8_UART3_THR = b)

/* *
* @brief Serial port multibyte send
*
* @param buf - The first address of the data content to be sent
* @param l - length of data to be sent */
void UART3_SendString(uint8_t *buf, uint16_t l);

/* *
* @brief read single byte on the serial port
*
* @return Read single byte */
#define UART3_RecvByte()    (R8_UART3_RBR)

/**
 * @brief   串口读取多字节
 *
 * @param   buf     - 读取数据存放缓存区首地址
 *
 * @return  读取数据长度
 */
uint16_t UART3_RecvString(uint8_t *buf);

#ifdef __cplusplus
}
#endif

#endif // __CH59x_UART_H__
