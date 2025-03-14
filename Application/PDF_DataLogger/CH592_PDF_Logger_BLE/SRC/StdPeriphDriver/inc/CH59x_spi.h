/********************************** (C) COPYRIGHT *******************************
 * File Name          : CH59x_SPI.h
 * Author             : WCH
 * Version            : V1.2
 * Date               : 2021/11/17
 * Description
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#ifndef __CH59x_SPI_H__
#define __CH59x_SPI_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  SPI0 interrupt bit define
 */
#define SPI0_IT_FST_BYTE    RB_SPI_IE_FST_BYTE  // 从机模式的首字节命令模式下，接收到首字节中断
#define SPI0_IT_FIFO_OV     RB_SPI_IE_FIFO_OV   // FIFO Overflow
#define SPI0_IT_DMA_END     RB_SPI_IE_DMA_END   // DMA transmission ends
#define SPI0_IT_FIFO_HF     RB_SPI_IE_FIFO_HF   // FIFO has been used for more than half
#define SPI0_IT_BYTE_END    RB_SPI_IE_BYTE_END  // Single-byte transmission is completed
#define SPI0_IT_CNT_END     RB_SPI_IE_CNT_END   // All byte transfer is completed

/**
 * @brief  Configuration data mode
 */
typedef enum
{
    Mode0_LowBitINFront = 0, // Mode 0, low position in front
    Mode0_HighBitINFront,    // Mode 0, high position in front
    Mode3_LowBitINFront,     // 模式3，低位在前
    Mode3_HighBitINFront,    // Mode 3, high position in front
} ModeBitOrderTypeDef;

/**
 * @brief  Configuration SPI0 slave mode
 */
typedef enum
{
    Mode_DataStream = 0, // 数据流模式
    Mose_FirstCmd,       // First-byte command mode
} Slave_ModeTypeDef;

/* *
* @brief Host mode default initialization: Mode 0+3 line full duplex + 8MHz */
void SPI0_MasterDefInit(void);

/* *
* @brief SPI0 reference clock configuration, = d*Tsys
*
* @param c - clock frequency division coefficient */
void SPI0_CLKCfg(uint8_t c);

/* *
* @brief Set data flow mode
*
* @param m - Data flow mode refer to ModeBitOrderTypeDef */
void SPI0_DataMode(ModeBitOrderTypeDef m);

/* *
* @brief Send a single byte (buffer)
*
* @param d - Send bytes */
void SPI0_MasterSendByte(uint8_t d);

/* *
* @brief Receive single byte (buffer)
*
* @param none */
uint8_t SPI0_MasterRecvByte(void);

/**
 * @brief   使用FIFO连续发送多字节
 *
 * @param   pbuf    - 待发送的数据内容首地址
 * @param   len     - 请求发送的数据长度，最大4095
 */
void SPI0_MasterTrans(uint8_t *pbuf, uint16_t len);

/* *
* @brief Receive multibytes continuously using FIFO
*
* @param pbuf - The first address of the data to be received
* @param len - The length of data to be received, maximum of 4095 */
void SPI0_MasterRecv(uint8_t *pbuf, uint16_t len);

/* *
* @brief data is sent continuously in DMA mode
*
* @param pbuf - The starting address of data to be sent, four bytes are required to
* @param len - length of data to be sent */
void SPI0_MasterDMATrans(uint8_t *pbuf, uint16_t len);

/* *
* @brief DMA mode continuously receives data
*
* @param pbuf - The starting address of data to be received, four bytes are required to store it
* @param len - length of data to be received */
void SPI0_MasterDMARecv(uint8_t *pbuf, uint16_t len);

void SPI1_MasterDefInit(void);             /* 主机模式默认初始化：模式0+3线全双工+8MHz */
void SPI1_CLKCfg(UINT8 c);                 /* SPI1 reference clock configuration, = d*Tsys */
void SPI1_DataMode(ModeBitOrderTypeDef m); /* 设置数据流模式 */

void  SPI1_MasterSendByte(UINT8 d); /* Send a single byte (buffer) */
UINT8 SPI1_MasterRecvByte(void);    /* Receive single byte (buffer) */

void SPI1_MasterTrans(UINT8 *pbuf, UINT16 len); /* Send multibytes continuously using FIFO */
void SPI1_MasterRecv(UINT8 *pbuf, UINT16 len);  /* Continuous reception of multibytes using FIFO */

/**
 * @brief   设备模式默认初始化，建议设置MISO的GPIO对应为输入模式
 */
void SPI0_SlaveInit(void);

/**
 * @brief   加载首字节数据内容
 *
 * @param   d       - 首字节数据内容
 */
#define SetFirstData(d)    (R8_SPI0_SLAVE_PRE = d)

/* *
* @brief slave mode, send one byte of data
*
* @param d - Data to be sent */
void SPI0_SlaveSendByte(uint8_t d);

/* *
* @brief slave mode, receive one byte of data
*
* @return Received data */
uint8_t SPI0_SlaveRecvByte(void);

/* *
* @brief slave mode, send multibyte data
*
* @param pbuf - The first address of the data content to be sent
* @param len - The length of the data requested to send, maximum 4095 */
void SPI0_SlaveTrans(uint8_t *pbuf, uint16_t len);

/* *
* @brief slave mode, receive multibyte data
*
* @param pbuf - Start address for receiving and receiving data storage
* @param len - Request received data length */
void SPI0_SlaveRecv(uint8_t *pbuf, uint16_t len);

/* *
* @brief data is sent continuously in DMA mode
*
* @param pbuf - The starting address of data to be sent, four bytes are required to
* @param len - length of data to be sent */
void SPI0_SlaveDMATrans(uint8_t *pbuf, uint16_t len);

/**
 * @brief   DMA方式连续接收数据
 *
 * @param   pbuf    - 待接收数据存放起始地址,需要四字节对其
 * @param   len     - 待接收数据长度
 */
void SPI0_SlaveDMARecv(uint8_t *pbuf, uint16_t len);

/* *
* @brief Configure SPI0 interrupt
*
* @param s - Enable/Close
* @param f - refer to SPI0 interrupt bit define */
#define SPI0_ITCfg(s, f)       ((s) ? (R8_SPI0_INTER_EN |= f) : (R8_SPI0_INTER_EN &= ~f))

/* *
* @brief Get interrupt flag status, 0-not set, (!0)-triggered
*
* @param f - refer to SPI0 interrupt bit define */
#define SPI0_GetITFlag(f)      (R8_SPI0_INT_FLAG & f)

/* *
* @brief Clear the current interrupt flag
*
* @param f - refer to SPI0 interrupt bit define */
#define SPI0_ClearITFlag(f)    (R8_SPI0_INT_FLAG = f)

/**
 * @brief   关闭SPI0
 */
#define SPI0_Disable()         (R8_SPI0_CTRL_MOD &= ~(RB_SPI_MOSI_OE | RB_SPI_SCK_OE | RB_SPI_MISO_OE))

#ifdef __cplusplus
}
#endif

#endif // __CH59x_SPI_H__
