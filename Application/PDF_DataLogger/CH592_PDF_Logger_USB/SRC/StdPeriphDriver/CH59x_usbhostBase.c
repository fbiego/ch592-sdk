/********************************** (C) COPYRIGHT *******************************
 * File Name          : CH59x_usbhost.c
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
#if DISK_LIB_ENABLE
  #include "CHRV3UFI.H"
#endif

uint8_t       UsbDevEndp0Size; // Maximum package size for endpoint 0 of USB device
uint8_t       FoundNewDev;
_RootHubDev   ThisUsbDev;                  // ROOT port
_DevOnHubPort DevOnHubPort[HUB_MAX_PORTS]; // Assuming: no more than 1 external HUB, each external HUB does not exceed HUB_MAX_PORTS ports (no matter if there are too many)

uint8_t *pHOST_RX_RAM_Addr;
uint8_t *pHOST_TX_RAM_Addr;

/*获取设备描述符*/
__attribute__((aligned(4))) const uint8_t SetupGetDevDescr[] = {USB_REQ_TYP_IN, USB_GET_DESCRIPTOR, 0x00,
                                                                USB_DESCR_TYP_DEVICE, 0x00, 0x00, sizeof(USB_DEV_DESCR), 0x00};
/*获取配置描述符*/
__attribute__((aligned(4))) const uint8_t SetupGetCfgDescr[] = {USB_REQ_TYP_IN, USB_GET_DESCRIPTOR, 0x00,
                                                                USB_DESCR_TYP_CONFIG, 0x00, 0x00, 0x04, 0x00};
/*设置USB地址*/
__attribute__((aligned(4))) const uint8_t SetupSetUsbAddr[] = {USB_REQ_TYP_OUT, USB_SET_ADDRESS, USB_DEVICE_ADDR, 0x00,
                                                               0x00, 0x00, 0x00, 0x00};
/*设置USB配置*/
__attribute__((aligned(4))) const uint8_t SetupSetUsbConfig[] = {USB_REQ_TYP_OUT, USB_SET_CONFIGURATION, 0x00, 0x00, 0x00,
                                                                 0x00, 0x00, 0x00};
/* Set up USB interface configuration */
__attribute__((aligned(4))) const uint8_t SetupSetUsbInterface[] = {USB_REQ_RECIP_INTERF, USB_SET_INTERFACE, 0x00, 0x00,
                                                                    0x00, 0x00, 0x00, 0x00};
/*清除端点STALL*/
__attribute__((aligned(4))) const uint8_t SetupClrEndpStall[] = {USB_REQ_TYP_OUT | USB_REQ_RECIP_ENDP, USB_CLEAR_FEATURE,
                                                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/*********************************************************************
 * @fn      DisableRootHubPort
 *
 * @brief   关闭ROOT-HUB端口,实际上硬件已经自动关闭,此处只是清除一些结构状态
 *
 * @param   none
 *
 * @return  none
 */
void DisableRootHubPort(void)
{
#ifdef FOR_ROOT_UDISK_ONLY
    CHRV3DiskStatus = DISK_DISCONNECT;
#endif
#ifndef DISK_BASE_BUF_LEN
    ThisUsbDev.DeviceStatus = ROOT_DEV_DISCONNECT;
    ThisUsbDev.DeviceAddress = 0x00;
#endif
}

/*********************************************************************
 * @fn      AnalyzeRootHub
 *
 * @brief   分析ROOT-HUB状态,处理ROOT-HUB端口的设备插拔事件
 *          如果设备拔出,函数中调用DisableRootHubPort()函数,将端口关闭,插入事件,置相应端口的状态位
 *
 * @param   none
 *
 * @return  返回ERR_SUCCESS为没有情况,返回ERR_USB_CONNECT为检测到新连接,返回ERR_USB_DISCON为检测到断开
 */
uint8_t AnalyzeRootHub(void)
{
    uint8_t s;

    s = ERR_SUCCESS;

    if(R8_USB_MIS_ST & RB_UMS_DEV_ATTACH)
    { // The device exists
#ifdef DISK_BASE_BUF_LEN
        if(CHRV3DiskStatus == DISK_DISCONNECT
#else
        if(ThisUsbDev.DeviceStatus == ROOT_DEV_DISCONNECT // Device insertion was detected
#endif
           || (R8_UHOST_CTRL & RB_UH_PORT_EN) == 0x00)
        {                         // The device was detected to be inserted, but it has not been allowed yet, which means it has just been inserted.
            DisableRootHubPort(); // 关闭端口
#ifdef DISK_BASE_BUF_LEN
            CHRV3DiskStatus = DISK_CONNECT;
#else
            ThisUsbDev.DeviceSpeed = R8_USB_MIS_ST & RB_UMS_DM_LEVEL ? 0 : 1;
            ThisUsbDev.DeviceStatus = ROOT_DEV_CONNECTED; // Set connection flag
#endif
            PRINT("USB dev in\n");
            s = ERR_USB_CONNECT;
        }
    }

#ifdef DISK_BASE_BUF_LEN
    else if(CHRV3DiskStatus >= DISK_CONNECT)
    {
#else
    else if(ThisUsbDev.DeviceStatus >= ROOT_DEV_CONNECTED)
    { // Device unplugged detected
#endif
        DisableRootHubPort(); // Close the port
        PRINT("USB dev out\n");
        if(s == ERR_SUCCESS)
        {
            s = ERR_USB_DISCON;
        }
    }
    // R8_USB_INT_FG = RB_UIF_DETECT; // Clear the interrupt flag
    return (s);
}

/* ***************************************************************************
* @fn SetHostUsbAddr
*
* @brief Set the USB device address of the current operation of the USB host
*
* @param addr - USB device address
*
* @return none */
void SetHostUsbAddr(uint8_t addr)
{
    R8_USB_DEV_AD = (R8_USB_DEV_AD & RB_UDA_GP_BIT) | (addr & MASK_USB_ADDR);
}

/*********************************************************************
 * @fn      SetUsbSpeed
 *
 * @brief   设置当前USB速度
 *
 * @param   FullSpeed   - USB速度
 *
 * @return  none
 */
void SetUsbSpeed(uint8_t FullSpeed)
{
#ifndef DISK_BASE_BUF_LEN
    if(FullSpeed) // full speed
    {
        R8_USB_CTRL &= ~RB_UC_LOW_SPEED;  // 全速
        R8_UH_SETUP &= ~RB_UH_PRE_PID_EN; // 禁止PRE PID
    }
    else
    {
        R8_USB_CTRL |= RB_UC_LOW_SPEED; // 低速
    }
#endif
    (void)FullSpeed;
}

/*********************************************************************
 * @fn      ResetRootHubPort
 *
 * @brief   检测到设备后,复位总线,为枚举设备准备,设置为默认为全速
 *
 * @param   none
 *
 * @return  none
 */
void ResetRootHubPort(void)
{
    UsbDevEndp0Size = DEFAULT_ENDP0_SIZE; // Maximum package size for endpoint 0 of USB device
    SetHostUsbAddr(0x00);
    R8_UHOST_CTRL &= ~RB_UH_PORT_EN;                                      // Turn off the port
    SetUsbSpeed(1);                                                       // Default is full speed
    R8_UHOST_CTRL = (R8_UHOST_CTRL & ~RB_UH_LOW_SPEED) | RB_UH_BUS_RESET; // 默认为全速,开始复位
    mDelaymS(15);                                                         // Reset time 10mS to 20mS
    R8_UHOST_CTRL = R8_UHOST_CTRL & ~RB_UH_BUS_RESET;                     // 结束复位
    mDelayuS(250);
    R8_USB_INT_FG = RB_UIF_DETECT; // Clear interrupt sign
}

/*********************************************************************
 * @fn      EnableRootHubPort
 *
 * @brief   使能ROOT-HUB端口,相应的bUH_PORT_EN置1开启端口,设备断开可能导致返回失败
 *
 * @param   none
 *
 * @return  返回ERR_SUCCESS为检测到新连接,返回ERR_USB_DISCON为无连接
 */
uint8_t EnableRootHubPort(void)
{
#ifdef DISK_BASE_BUF_LEN
    if(CHRV3DiskStatus < DISK_CONNECT)
        CHRV3DiskStatus = DISK_CONNECT;
#else
    if(ThisUsbDev.DeviceStatus < ROOT_DEV_CONNECTED)
        ThisUsbDev.DeviceStatus = ROOT_DEV_CONNECTED;
#endif
    if(R8_USB_MIS_ST & RB_UMS_DEV_ATTACH)
    { // 有设备
#ifndef DISK_BASE_BUF_LEN
        if((R8_UHOST_CTRL & RB_UH_PORT_EN) == 0x00)
        { // Not enabled yet
            ThisUsbDev.DeviceSpeed = (R8_USB_MIS_ST & RB_UMS_DM_LEVEL) ? 0 : 1;
            if(ThisUsbDev.DeviceSpeed == 0)
            {
                R8_UHOST_CTRL |= RB_UH_LOW_SPEED; // Low speed
            }
        }
#endif
        R8_UHOST_CTRL |= RB_UH_PORT_EN; // Enable HUB port
        return (ERR_SUCCESS);
    }
    return (ERR_USB_DISCON);
}

#ifndef DISK_BASE_BUF_LEN
/* ***************************************************************************
* @fn SelectHubPort
*
* @brief Select the HUB port to operate
*
* @param HubPortIndex - Select the specified port of the external HUB of the ROOT-HUB port specified by the operation
*
* @return None */
void SelectHubPort(uint8_t HubPortIndex)
{
    if(HubPortIndex) // 选择操作指定的ROOT-HUB端口的外部HUB的指定端口
    {
        SetHostUsbAddr(DevOnHubPort[HubPortIndex - 1].DeviceAddress); // 设置USB主机当前操作的USB设备地址
        SetUsbSpeed(DevOnHubPort[HubPortIndex - 1].DeviceSpeed);      // 设置当前USB速度
        if(DevOnHubPort[HubPortIndex - 1].DeviceSpeed == 0)           // Communication with low-speed USB devices through external HUB requires a pre-ID
        {
            R8_UEP1_CTRL |= RB_UH_PRE_PID_EN; // Enable PRE PID
            mDelayuS(100);
        }
    }
    else
    {
        SetHostUsbAddr(ThisUsbDev.DeviceAddress); // Set the USB device address of the current operation of the USB host
        SetUsbSpeed(ThisUsbDev.DeviceSpeed);      // Set the speed of the USB device
    }
}
#endif

/* ***************************************************************************
* @fn WaitUSB_Interrupt
*
* @brief Waiting for USB interruption
*
* @param none
*
* @return Return ERR_SUCCESS Data received or sent successfully, Return ERR_USB_UNKNOWN Data received or sent failed */
uint8_t WaitUSB_Interrupt(void)
{
    uint16_t i;
    for(i = WAIT_USB_TOUT_200US; i != 0 && (R8_USB_INT_FG & RB_UIF_TRANSFER) == 0; i--)
    {
        ;
    }
    return ((R8_USB_INT_FG & RB_UIF_TRANSFER) ? ERR_SUCCESS : ERR_USB_UNKNOWN);
}

/* ***************************************************************************
* @fn USBHostTransact
*
* @brief Transfer transactions, enter the destination endpoint address/PID token, synchronization flag, total time for NAK retry in units of 20uS (no retry if 0 is not retry, 0xFFFFF infinite retry), return 0 successful, timeout/error retry
* This subprogram focuses on easy understanding, but in actual applications, in order to provide running speed, the subprogram code should be optimized.
*
* @param endp_pid - token and address, the upper 4 bits are token_pid token, the lower 4 bits are endpoint address
* @param tog - Synchronize flag
* @param timeout - Timeout
*
* @return ERR_USB_UNKNOWN Timeout, possible hardware exception
* ERR_USB_DISCON The device is disconnected
* ERR_USB_CONNECT device connection
* ERR_SUCCESS Transmission Completed */
uint8_t USBHostTransact(uint8_t endp_pid, uint8_t tog, uint32_t timeout)
{
    uint8_t TransRetry;

    uint8_t  s, r;
    uint16_t i;

    R8_UH_RX_CTRL = R8_UH_TX_CTRL = tog;
    TransRetry = 0;

    do
    {
        R8_UH_EP_PID = endp_pid; // Specify the token PID and destination endpoint number
        R8_USB_INT_FG = RB_UIF_TRANSFER;
        for(i = WAIT_USB_TOUT_200US; i != 0 && (R8_USB_INT_FG & RB_UIF_TRANSFER) == 0; i--)
        {
            ;
        }
        R8_UH_EP_PID = 0x00; // Stop USB transmission
        if((R8_USB_INT_FG & RB_UIF_TRANSFER) == 0)
        {
            return (ERR_USB_UNKNOWN);
        }

        if(R8_USB_INT_FG & RB_UIF_DETECT)
        { // USB设备插拔事件
            // mDelayuS(200); // Wait for the transmission to complete
            R8_USB_INT_FG = RB_UIF_DETECT;
            s = AnalyzeRootHub(); // 分析ROOT-HUB状态

            if(s == ERR_USB_CONNECT)
                FoundNewDev = 1;
#ifdef DISK_BASE_BUF_LEN
            if(CHRV3DiskStatus == DISK_DISCONNECT)
            {
                return (ERR_USB_DISCON);
            } // USB device disconnect event
            if(CHRV3DiskStatus == DISK_CONNECT)
            {
                return (ERR_USB_CONNECT);
            } // USB设备连接事件
#else
            if(ThisUsbDev.DeviceStatus == ROOT_DEV_DISCONNECT)
            {
                return (ERR_USB_DISCON);
            } // USB device disconnect event
            if(ThisUsbDev.DeviceStatus == ROOT_DEV_CONNECTED)
            {
                return (ERR_USB_CONNECT);
            } // USB device connection event
#endif
            mDelayuS(200); // 等待传输完成
        }

        if(R8_USB_INT_FG & RB_UIF_TRANSFER) // 传输完成事件
        {
            if(R8_USB_INT_ST & RB_UIS_TOG_OK)
            {
                return (ERR_SUCCESS);
            }
            r = R8_USB_INT_ST & MASK_UIS_H_RES; // USB device answer status
            if(r == USB_PID_STALL)
            {
                return (r | ERR_USB_TRANSFER);
            }
            if(r == USB_PID_NAK)
            {
                if(timeout == 0)
                {
                    return (r | ERR_USB_TRANSFER);
                }
                if(timeout < 0xFFFFFFFF)
                {
                    timeout--;
                }
                --TransRetry;
            }
            else
                switch(endp_pid >> 4)
                {
                    case USB_PID_SETUP:
                    case USB_PID_OUT:
                        if(r)
                        {
                            return (r | ERR_USB_TRANSFER);
                        }      // Not a timeout/error, unexpected response
                        break; // Timeout and try again
                    case USB_PID_IN:
                        if(r == USB_PID_DATA0 || r == USB_PID_DATA1)
                        { // If not synchronized, you need to discard it and try again
                        } // Try again if synchronously
                        else if(r)
                        {
                            return (r | ERR_USB_TRANSFER);
                        }      // Not a timeout/error, unexpected response
                        break; // 超时重试
                    default:
                        return (ERR_USB_UNKNOWN); // 不可能的情况
                        break;
                }
        }
        else
        {                         // Other interruptions, situations that should not happen
            R8_USB_INT_FG = 0xFF; /* Clear interrupt sign */
        }
        mDelayuS(15);
    } while(++TransRetry < 3);
    return (ERR_USB_TRANSFER); // Response timeout
}

/* ***************************************************************************
* @fn HostCtrlTransfer
*
* @brief executes control transmission, 8 byte request code in pSetupReq, DataBuf is an optional sending and receiving buffer
*
* @param DataBuf - If you need to receive and send data, then DataBuf needs to point to a valid buffer to store subsequent data
* @param RetLen - The total length of the actual successful sending and receiving is saved in the byte variable pointed to by RetLen
*
* @return ERR_USB_BUF_OVER IN status stage error
* ERR_SUCCESS Data exchange was successful */
uint8_t HostCtrlTransfer(uint8_t *DataBuf, uint8_t *RetLen)
{
    uint16_t RemLen = 0;
    uint8_t  s, RxLen, RxCnt, TxCnt;
    uint8_t *pBuf;
    uint8_t *pLen;

    pBuf = DataBuf;
    pLen = RetLen;
    mDelayuS(200);
    if(pLen)
    {
        *pLen = 0; // Total length of actual successful sending and receiving
    }

    R8_UH_TX_LEN = sizeof(USB_SETUP_REQ);
    s = USBHostTransact(USB_PID_SETUP << 4 | 0x00, 0x00, 200000 / 20); // SETUP stage, 200mS timeout
    if(s != ERR_SUCCESS)
    {
        return (s);
    }
    R8_UH_RX_CTRL = R8_UH_TX_CTRL = RB_UH_R_TOG | RB_UH_R_AUTO_TOG | RB_UH_T_TOG | RB_UH_T_AUTO_TOG; // Default DATA1
    R8_UH_TX_LEN = 0x01;                                                                             // The default is no data, so the status stage is IN
    RemLen = pSetupReq->wLength;
    PRINT("wLength: %x\n", RemLen);
    if(RemLen && pBuf) // 需要收发数据
    {
        PRINT("bRequestType: %x\n", pSetupReq->bRequestType);
        if(pSetupReq->bRequestType & USB_REQ_TYP_IN) // æ”¶
        {
            while(RemLen)
            {
                mDelayuS(200);
                s = USBHostTransact(USB_PID_IN << 4 | 0x00, R8_UH_RX_CTRL, 200000 / 20); // IN data
                if(s != ERR_SUCCESS)
                {
                    return (s);
                }
                RxLen = R8_USB_RX_LEN < RemLen ? R8_USB_RX_LEN : RemLen;
                RemLen -= RxLen;
                if(pLen)
                {
                    *pLen += RxLen; // Total length of actual successful sending and receiving
                }
                for(RxCnt = 0; RxCnt != RxLen; RxCnt++)
                {
                    *pBuf = pHOST_RX_RAM_Addr[RxCnt];
                    pBuf++;
                }
                if(R8_USB_RX_LEN == 0 || (R8_USB_RX_LEN & (UsbDevEndp0Size - 1)))
                {
                    break; // 短包
                }
            }
            R8_UH_TX_LEN = 0x00; // The status phase is OUT
        }
        else // hair
        {
            while(RemLen)
            {
                mDelayuS(200);
                R8_UH_TX_LEN = RemLen >= UsbDevEndp0Size ? UsbDevEndp0Size : RemLen;
                for(TxCnt = 0; TxCnt != R8_UH_TX_LEN; TxCnt++)
                {
                    pHOST_TX_RAM_Addr[TxCnt] = *pBuf;
                    pBuf++;
                }
                s = USBHostTransact(USB_PID_OUT << 4 | 0x00, R8_UH_TX_CTRL, 200000 / 20); // OUT数据
                if(s != ERR_SUCCESS)
                {
                    return (s);
                }
                RemLen -= R8_UH_TX_LEN;
                if(pLen)
                {
                    *pLen += R8_UH_TX_LEN; // Total length of actual successful sending and receiving
                }
            }
            PRINT("Send: %d\n", *pLen);
            //          R8_UH_TX_LEN = 0x01;                                                     // 状态阶段为IN
        }
    }
    mDelayuS(200);
    s = USBHostTransact((R8_UH_TX_LEN ? USB_PID_IN << 4 | 0x00 : USB_PID_OUT << 4 | 0x00), RB_UH_R_TOG | RB_UH_T_TOG, 200000 / 20); // STATUS stage
    if(s != ERR_SUCCESS)
    {
        return (s);
    }
    if(R8_UH_TX_LEN == 0)
    {
        return (ERR_SUCCESS); // Status OUT
    }
    if(R8_USB_RX_LEN == 0)
    {
        return (ERR_SUCCESS); // 状态IN,检查IN状态返回数据长度
    }
    return (ERR_USB_BUF_OVER); // IN状态阶段错误
}

/* ***************************************************************************
* @fn CopySetupReqPkg
*
* @brief Copy control transfer request packet
*
* @param pReqPkt - Control request packet address
*
* @return none */
void CopySetupReqPkg(const uint8_t *pReqPkt) // 复制控制传输的请求包
{
    uint8_t i;
    for(i = 0; i != sizeof(USB_SETUP_REQ); i++)
    {
        ((uint8_t *)pSetupReq)[i] = *pReqPkt;
        pReqPkt++;
    }
}

/* ***************************************************************************
* @fn CtrlGetDeviceDescr
*
* @brief Get the device descriptor, return it in pHOST_TX_RAM_Addr
*
* @param none
*
* @return ERR_USB_BUF_OVER Descriptor length error
* ERR_SUCCESS Success */
uint8_t CtrlGetDeviceDescr(void)
{
    uint8_t s;
    uint8_t len;

    UsbDevEndp0Size = DEFAULT_ENDP0_SIZE;
    CopySetupReqPkg(SetupGetDevDescr);
    s = HostCtrlTransfer(Com_Buffer, &len); // Perform control transmission
    if(s != ERR_SUCCESS)
    {
        return (s);
    }
    UsbDevEndp0Size = ((PUSB_DEV_DESCR)Com_Buffer)->bMaxPacketSize0; // 端点0最大包长度,这是简化处理,正常应该先获取前8字节后立即更新UsbDevEndp0Size再继续
    if(len < ((PUSB_SETUP_REQ)SetupGetDevDescr)->wLength)
    {
        return (ERR_USB_BUF_OVER); // 描述符长度错误
    }
    return (ERR_SUCCESS);
}

/* ***************************************************************************
* @fn CtrlGetConfigDescr
*
* @brief Get the configuration descriptor, return it in pHOST_TX_RAM_Addr
*
* @param none
*
* @return ERR_USB_BUF_OVER Descriptor length error
* ERR_SUCCESS Success */
uint8_t CtrlGetConfigDescr(void)
{
    uint8_t s;
    uint8_t len;

    CopySetupReqPkg(SetupGetCfgDescr);
    s = HostCtrlTransfer(Com_Buffer, &len); // Perform control transmission
    if(s != ERR_SUCCESS)
    {
        return (s);
    }
    if(len < ((PUSB_SETUP_REQ)SetupGetCfgDescr)->wLength)
    {
        return (ERR_USB_BUF_OVER); // Returns a length error
    }

    len = ((PUSB_CFG_DESCR)Com_Buffer)->wTotalLength;
    CopySetupReqPkg(SetupGetCfgDescr);
    pSetupReq->wLength = len;               // Total length of full configuration descriptor
    s = HostCtrlTransfer(Com_Buffer, &len); // Perform control transmission
    if(s != ERR_SUCCESS)
    {
        return (s);
    }

#ifdef DISK_BASE_BUF_LEN
    if(len > 64)
        len = 64;
    memcpy(TxBuffer, Com_Buffer, len); //U盘操作时，需要拷贝到TxBuffer
#endif

    return (ERR_SUCCESS);
}

/*********************************************************************
 * @fn      CtrlSetUsbAddress
 *
 * @brief   设置USB设备地址
 *
 * @param   addr    - 设备地址
 *
 * @return  ERR_SUCCESS     成功
 */
uint8_t CtrlSetUsbAddress(uint8_t addr)
{
    uint8_t s;

    CopySetupReqPkg(SetupSetUsbAddr);
    pSetupReq->wValue = addr;         // USB device address
    s = HostCtrlTransfer(NULL, NULL); // Perform control transmission
    if(s != ERR_SUCCESS)
    {
        return (s);
    }
    SetHostUsbAddr(addr); // Set the USB device address of the current operation of the USB host
    mDelaymS(10);         // 等待USB设备完成操作
    return (ERR_SUCCESS);
}

/* ***************************************************************************
* @fn CtrlSetUsbConfig
*
* @brief Setting USB device configuration
*
* @param cfg - Configuration value
*
* @return ERR_SUCCESS Success */
uint8_t CtrlSetUsbConfig(uint8_t cfg)
{
    CopySetupReqPkg(SetupSetUsbConfig);
    pSetupReq->wValue = cfg;               // USB device configuration
    return (HostCtrlTransfer(NULL, NULL)); // 执行控制传输
}

/*********************************************************************
 * @fn      CtrlClearEndpStall
 *
 * @brief   清除端点STALL
 *
 * @param   endp    - 端点地址
 *
 * @return  ERR_SUCCESS     成功
 */
uint8_t CtrlClearEndpStall(uint8_t endp)
{
    CopySetupReqPkg(SetupClrEndpStall); // Clear endpoint error
    pSetupReq->wIndex = endp;                     // Endpoint address
    return (HostCtrlTransfer(NULL, NULL));        // Perform control transmission
}

/* ***************************************************************************
* @fn CtrlSetUsbIntercace
*
* @brief Setting up USB device interface
*
* @param cfg - Configuration value
*
* @return ERR_SUCCESS Success */
uint8_t CtrlSetUsbIntercace(uint8_t cfg)
{
    CopySetupReqPkg(SetupSetUsbInterface);
    pSetupReq->wValue = cfg;               // USB device configuration
    return (HostCtrlTransfer(NULL, NULL)); // Perform control transmission
}

/* ***************************************************************************
* @fn USB_HostInit
*
* @brief USB host function initialization
*
* @param none
*
* @return none */
void USB_HostInit(void)
{
    R8_USB_CTRL = RB_UC_HOST_MODE;
    R8_UHOST_CTRL = 0;
    R8_USB_DEV_AD = 0x00;

    R8_UH_EP_MOD = RB_UH_EP_TX_EN | RB_UH_EP_RX_EN;
    R16_UH_RX_DMA = (uint16_t)(uint32_t)pHOST_RX_RAM_Addr;
    R16_UH_TX_DMA = (uint16_t)(uint32_t)pHOST_TX_RAM_Addr;

    R8_UH_RX_CTRL = 0x00;
    R8_UH_TX_CTRL = 0x00;
    R8_USB_CTRL = RB_UC_HOST_MODE | RB_UC_INT_BUSY | RB_UC_DMA_EN;
    R8_UH_SETUP = RB_UH_SOF_EN;
    R8_USB_INT_FG = 0xFF;
    DisableRootHubPort();
    R8_USB_INT_EN = RB_UIE_TRANSFER | RB_UIE_DETECT;

    FoundNewDev = 0;
}
