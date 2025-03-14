/********************************** (C) COPYRIGHT *******************************
 * File Name          : CH59x_SPI0.c
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
* @fn SPI0_MasterDefInit
*
* @brief Host mode default initialization: Mode 0+3 line full duplex + 8MHz
*
* @param none
*
* @return none */
void SPI0_MasterDefInit(void)
{
    R8_SPI0_CLOCK_DIV = 4; // Main frequency clock 4 times
    R8_SPI0_CTRL_MOD = RB_SPI_ALL_CLEAR;
    R8_SPI0_CTRL_MOD = RB_SPI_MOSI_OE | RB_SPI_SCK_OE;
    R8_SPI0_CTRL_CFG |= RB_SPI_AUTO_IF;     // Access BUFFER/FIFO to automatically clear the IF_BYTE_END flag
    R8_SPI0_CTRL_CFG &= ~RB_SPI_DMA_ENABLE; // Don't start DMA mode
}

/* ***************************************************************************
* @fn SPI0_CLKCfg
*
* @brief SPI0 reference clock configuration, = d*Tsys
*
* @param c - clock frequency division coefficient
*
* @return none */
void SPI0_CLKCfg(uint8_t c)
{
    if(c == 2)
    {
        R8_SPI0_CTRL_CFG |= RB_SPI_MST_DLY_EN;
    }
    else
    {
        R8_SPI0_CTRL_CFG &= ~RB_SPI_MST_DLY_EN;
    }
    R8_SPI0_CLOCK_DIV = c;
}

/* ***************************************************************************
* @fn SPI0_DataMode
*
* @brief Set data flow mode
*
* @param m - Data flow mode refer to ModeBitOrderTypeDef
*
* @return none */
void SPI0_DataMode(ModeBitOrderTypeDef m)
{
    switch(m)
    {
        case Mode0_LowBitINFront:
            R8_SPI0_CTRL_MOD &= ~RB_SPI_MST_SCK_MOD;
            R8_SPI0_CTRL_CFG |= RB_SPI_BIT_ORDER;
            break;
        case Mode0_HighBitINFront:
            R8_SPI0_CTRL_MOD &= ~RB_SPI_MST_SCK_MOD;
            R8_SPI0_CTRL_CFG &= ~RB_SPI_BIT_ORDER;
            break;
        case Mode3_LowBitINFront:
            R8_SPI0_CTRL_MOD |= RB_SPI_MST_SCK_MOD;
            R8_SPI0_CTRL_CFG |= RB_SPI_BIT_ORDER;
            break;
        case Mode3_HighBitINFront:
            R8_SPI0_CTRL_MOD |= RB_SPI_MST_SCK_MOD;
            R8_SPI0_CTRL_CFG &= ~RB_SPI_BIT_ORDER;
            break;
        default:
            break;
    }
}

/*********************************************************************
 * @fn      SPI0_MasterSendByte
 *
 * @brief   发送单字节 (buffer)
 *
 * @param   d       - 发送字节
 *
 * @return  none
 */
void SPI0_MasterSendByte(uint8_t d)
{
    R8_SPI0_CTRL_MOD &= ~RB_SPI_FIFO_DIR;
    R8_SPI0_BUFFER = d;
    while(!(R8_SPI0_INT_FLAG & RB_SPI_FREE));
}

/*********************************************************************
 * @fn      SPI0_MasterRecvByte
 *
 * @brief   接收单字节 (buffer)
 *
 * @param   none
 *
 * @return  接收到的字节
 */
uint8_t SPI0_MasterRecvByte(void)
{
    R8_SPI0_CTRL_MOD &= ~RB_SPI_FIFO_DIR;
    R8_SPI0_BUFFER = 0xFF; // Start the transfer
    while(!(R8_SPI0_INT_FLAG & RB_SPI_FREE));
    return (R8_SPI0_BUFFER);
}

/* ***************************************************************************
* @fn SPI0_MasterTrans
*
* @brief sends multibytes continuously using FIFO
*
* @param pbuf - The first address of the data content to be sent
* @param len - The length of the data requested to send, maximum 4095
*
* @return none */
void SPI0_MasterTrans(uint8_t *pbuf, uint16_t len)
{
    uint16_t sendlen;

    sendlen = len;
    R8_SPI0_CTRL_MOD &= ~RB_SPI_FIFO_DIR; // Set the data direction to output
    R16_SPI0_TOTAL_CNT = sendlen;         // Set the length of data to be sent
    R8_SPI0_INT_FLAG = RB_SPI_IF_CNT_END;
    while(sendlen)
    {
        if(R8_SPI0_FIFO_COUNT < SPI_FIFO_SIZE)
        {
            R8_SPI0_FIFO = *pbuf;
            pbuf++;
            sendlen--;
        }
    }
    while(R8_SPI0_FIFO_COUNT != 0); // Wait for all data in FIFO to be sent to complete
}

/*********************************************************************
 * @fn      SPI0_MasterRecv
 *
 * @brief   使用FIFO连续接收多字节
 *
 * @param   pbuf    - 待接收的数据首地址
 * @param   len     - 待接收的数据长度，最大4095
 *
 * @return  none
 */
void SPI0_MasterRecv(uint8_t *pbuf, uint16_t len)
{
    uint16_t readlen;

    readlen = len;
    R8_SPI0_CTRL_MOD |= RB_SPI_FIFO_DIR; // Set the data direction to input
    R16_SPI0_TOTAL_CNT = len;            // Set the length of data to be received. If the FIFO direction is input length not 0, the transmission will be started */
    R8_SPI0_INT_FLAG = RB_SPI_IF_CNT_END;
    while(readlen)
    {
        if(R8_SPI0_FIFO_COUNT)
        {
            *pbuf = R8_SPI0_FIFO;
            pbuf++;
            readlen--;
        }
    }
}

/* ***************************************************************************
* @fn SPI0_MasterDMATrans
*
* @brief data is sent continuously in DMA mode
*
* @param pbuf - The starting address of data to be sent, four bytes are required to
* @param len - length of data to be sent
*
* @return none */
void SPI0_MasterDMATrans(uint8_t *pbuf, uint16_t len)
{
    R8_SPI0_CTRL_MOD &= ~RB_SPI_FIFO_DIR;
    R16_SPI0_DMA_BEG = (uint32_t)pbuf;
    R16_SPI0_DMA_END = (uint32_t)(pbuf + len);
    R16_SPI0_TOTAL_CNT = len;
    R8_SPI0_INT_FLAG = RB_SPI_IF_CNT_END | RB_SPI_IF_DMA_END;
    R8_SPI0_CTRL_CFG |= RB_SPI_DMA_ENABLE;
    while(!(R8_SPI0_INT_FLAG & RB_SPI_IF_CNT_END));
    R8_SPI0_CTRL_CFG &= ~RB_SPI_DMA_ENABLE;
}

/*********************************************************************
 * @fn      SPI0_MasterDMARecv
 *
 * @brief   DMA方式连续接收数据
 *
 * @param   pbuf    - 待接收数据存放起始地址,需要四字节对其
 * @param   len     - 待接收数据长度
 *
 * @return  none
 */
void SPI0_MasterDMARecv(uint8_t *pbuf, uint16_t len)
{
    R8_SPI0_CTRL_MOD |= RB_SPI_FIFO_DIR;
    R16_SPI0_DMA_BEG = (uint32_t)pbuf;
    R16_SPI0_DMA_END = (uint32_t)(pbuf + len);
    R16_SPI0_TOTAL_CNT = len;
    R8_SPI0_INT_FLAG = RB_SPI_IF_CNT_END | RB_SPI_IF_DMA_END;
    R8_SPI0_CTRL_CFG |= RB_SPI_DMA_ENABLE;
    while(!(R8_SPI0_INT_FLAG & RB_SPI_IF_CNT_END));
    R8_SPI0_CTRL_CFG &= ~RB_SPI_DMA_ENABLE;
}

/* ***************************************************************************
* @fn SPI0_SlaveInit
*
* @brief The device mode is initialized by default. It is recommended to set the GPIO of MISO to the input mode.
*
* @return none */
void SPI0_SlaveInit(void)
{
    R8_SPI0_CTRL_MOD = RB_SPI_ALL_CLEAR;
    R8_SPI0_CTRL_MOD = RB_SPI_MISO_OE | RB_SPI_MODE_SLAVE;
    R8_SPI0_CTRL_CFG |= RB_SPI_AUTO_IF;
}

/*********************************************************************
 * @fn      SPI0_SlaveRecvByte
 *
 * @brief   从机模式，接收一字节数据
 *
 * @return  接收到数据
 */
uint8_t SPI0_SlaveRecvByte(void)
{
    R8_SPI0_CTRL_MOD |= RB_SPI_FIFO_DIR;
    while(R8_SPI0_FIFO_COUNT == 0);
    return R8_SPI0_FIFO;
}

/* ***************************************************************************
* @fn SPI0_SlaveSendByte
*
* @brief slave mode, send one byte of data
*
* @param d - Data to be sent
*
* @return none */
void SPI0_SlaveSendByte(uint8_t d)
{
    R8_SPI0_CTRL_MOD &= ~RB_SPI_FIFO_DIR;
    R8_SPI0_FIFO = d;
    while(R8_SPI0_FIFO_COUNT != 0); // Wait for the sending to complete
}

/*********************************************************************
 * @fn      SPI0_SlaveRecv
 *
 * @brief   从机模式，接收多字节数据
 *
 * @param   pbuf    - 接收收数据存放起始地址
 * @param   len     - 请求接收数据长度
 *
 * @return  none
 */
__HIGH_CODE
void SPI0_SlaveRecv(uint8_t *pbuf, uint16_t len)
{
    uint16_t revlen;

    revlen = len;
    R8_SPI0_CTRL_MOD |= RB_SPI_FIFO_DIR;
    R8_SPI0_INT_FLAG = RB_SPI_IF_CNT_END;
    while(revlen)
    {
        if(R8_SPI0_FIFO_COUNT)
        {
            *pbuf = R8_SPI0_FIFO;
            pbuf++;
            revlen--;
        }
    }
}

/* ***************************************************************************
* @fn SPI0_SlaveTrans
*
* @brief slave mode, send multibyte data
*
* @param pbuf - The first address of the data content to be sent
* @param len - The length of the data requested to send, maximum 4095
*
* @return none */
__HIGH_CODE
void SPI0_SlaveTrans(uint8_t *pbuf, uint16_t len)
{
    uint16_t sendlen;

    sendlen = len;
    R8_SPI0_CTRL_MOD &= ~RB_SPI_FIFO_DIR; // Set the data direction to output
    R8_SPI0_INT_FLAG = RB_SPI_IF_CNT_END;
    while(sendlen)
    {
        if(R8_SPI0_FIFO_COUNT < SPI_FIFO_SIZE)
        {
            R8_SPI0_FIFO = *pbuf;
            pbuf++;
            sendlen--;
        }
    }
    while(R8_SPI0_FIFO_COUNT != 0); // Wait for all data in FIFO to be sent to complete
}

/* ***************************************************************************
* @fn SPI0_SlaveDMARecv
*
* @brief DMA mode continuously receives data
*
* @param pbuf - The starting address of data to be received, four bytes are required to store it
* @param len - length of data to be received
*
* @return none */
void SPI0_SlaveDMARecv(uint8_t *pbuf, uint16_t len)
{
    R8_SPI0_CTRL_MOD |= RB_SPI_FIFO_DIR;
    R16_SPI0_DMA_BEG = (uint32_t)pbuf;
    R16_SPI0_DMA_END = (uint32_t)(pbuf + len);
    R16_SPI0_TOTAL_CNT = len;
    R8_SPI0_INT_FLAG = RB_SPI_IF_CNT_END | RB_SPI_IF_DMA_END;
    R8_SPI0_CTRL_CFG |= RB_SPI_DMA_ENABLE;
    while(!(R8_SPI0_INT_FLAG & RB_SPI_IF_CNT_END));
    R8_SPI0_CTRL_CFG &= ~RB_SPI_DMA_ENABLE;
}

/* ***************************************************************************
* @fn SPI0_SlaveDMATrans
*
* @brief data is sent continuously in DMA mode
*
* @param pbuf - The starting address of data to be sent, four bytes are required to
* @param len - length of data to be sent
*
* @return none */
void SPI0_SlaveDMATrans(uint8_t *pbuf, uint16_t len)
{
    R8_SPI0_CTRL_MOD &= ~RB_SPI_FIFO_DIR;
    R16_SPI0_DMA_BEG = (uint32_t)pbuf;
    R16_SPI0_DMA_END = (uint32_t)(pbuf + len);
    R16_SPI0_TOTAL_CNT = len;
    R8_SPI0_INT_FLAG = RB_SPI_IF_CNT_END | RB_SPI_IF_DMA_END;
    R8_SPI0_CTRL_CFG |= RB_SPI_DMA_ENABLE;
    while(!(R8_SPI0_INT_FLAG & RB_SPI_IF_CNT_END));
    R8_SPI0_CTRL_CFG &= ~RB_SPI_DMA_ENABLE;
}
