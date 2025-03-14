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

/* Get the device descriptor */
__attribute__((aligned(4))) const uint8_t SetupGetDevDescr[] = {USB_REQ_TYP_IN, USB_GET_DESCRIPTOR, 0x00,
                                                                USB_DESCR_TYP_DEVICE, 0x00, 0x00, sizeof(USB_DEV_DESCR), 0x00};
/* Get the configuration descriptor */
__attribute__((aligned(4))) const uint8_t SetupGetCfgDescr[] = {USB_REQ_TYP_IN, USB_GET_DESCRIPTOR, 0x00,
                                                                USB_DESCR_TYP_CONFIG, 0x00, 0x00, 0x04, 0x00};
/* Setting up the USB address */
__attribute__((aligned(4))) const uint8_t SetupSetUsbAddr[] = {USB_REQ_TYP_OUT, USB_SET_ADDRESS, USB_DEVICE_ADDR, 0x00,
                                                               0x00, 0x00, 0x00, 0x00};
/* Set up USB configuration */
__attribute__((aligned(4))) const uint8_t SetupSetUsbConfig[] = {USB_REQ_TYP_OUT, USB_SET_CONFIGURATION, 0x00, 0x00, 0x00,
                                                                 0x00, 0x00, 0x00};
/* Set up USB interface configuration */
__attribute__((aligned(4))) const uint8_t SetupSetUsbInterface[] = {USB_REQ_RECIP_INTERF, USB_SET_INTERFACE, 0x00, 0x00,
                                                                    0x00, 0x00, 0x00, 0x00};
/* Clear endpoint STALL */
__attribute__((aligned(4))) const uint8_t SetupClrEndpStall[] = {USB_REQ_TYP_OUT | USB_REQ_RECIP_ENDP, USB_CLEAR_FEATURE,
                                                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/* ***************************************************************************
* @fn DisableRootHubPort
*
* @brief Close the ROOT-HUB port, in fact, the hardware has been automatically closed, here is just clearing some structural states
*
* @param none
*
* @return none */
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

/* ***************************************************************************
* @fn AnalyzeRootHub
*
* @brief analyzes the ROOT-HUB status and handles the device plug-in and unplugging events of the ROOT-HUB port
* If the device is unplugged, the DisableRootHubPort() function is called in the function to close the port, insert the event, and set the status bit of the corresponding port.
*
* @param none
*
* @return Return ERR_SUCCESS is no situation, return ERR_USB_CONNECT is a new connection is detected, return ERR_USB_DISCON is a disconnection is detected */
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
            DisableRootHubPort(); // Close the port
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

/* ***************************************************************************
* @fn SetUsbSpeed
*
* @brief Set the current USB speed
*
* @param FullSpeed ​​- USB speed
*
* @return none */
void SetUsbSpeed(uint8_t FullSpeed)
{
#ifndef DISK_BASE_BUF_LEN
    if(FullSpeed) // full speed
    {
        R8_USB_CTRL &= ~RB_UC_LOW_SPEED;  // full speed
        R8_UH_SETUP &= ~RB_UH_PRE_PID_EN; // Prohibit PRE PID
    }
    else
    {
        R8_USB_CTRL |= RB_UC_LOW_SPEED; // Low speed
    }
#endif
    (void)FullSpeed;
}

/* ***************************************************************************
* @fn ResetRootHubPort
*
* @brief After detecting the device, reset the bus, prepare for the enumeration device, and set it to the default full speed
*
* @param none
*
* @return none */
void ResetRootHubPort(void)
{
    UsbDevEndp0Size = DEFAULT_ENDP0_SIZE; // Maximum package size for endpoint 0 of USB device
    SetHostUsbAddr(0x00);
    R8_UHOST_CTRL &= ~RB_UH_PORT_EN;                                      // Turn off the port
    SetUsbSpeed(1);                                                       // Default is full speed
    R8_UHOST_CTRL = (R8_UHOST_CTRL & ~RB_UH_LOW_SPEED) | RB_UH_BUS_RESET; // Default is full speed, start reset
    mDelaymS(15);                                                         // Reset time 10mS to 20mS
    R8_UHOST_CTRL = R8_UHOST_CTRL & ~RB_UH_BUS_RESET;                     // End reset
    mDelayuS(250);
    R8_USB_INT_FG = RB_UIF_DETECT; // Clear interrupt sign
}

/* ***************************************************************************
* @fn EnableRootHubPort
*
* @brief Enable the ROOT-HUB port, and the corresponding bUH_PORT_EN is set to 1 to open the port. The device is disconnected and the return failure may result in the return failure.
*
* @param none
*
* @return Return ERR_SUCCESS is detected as a new connection, return ERR_USB_DISCON is no connection */
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
    { // Have equipment
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
    if(HubPortIndex) // Select the specified port of the external HUB of the ROOT-HUB port specified by the operation
    {
        SetHostUsbAddr(DevOnHubPort[HubPortIndex - 1].DeviceAddress); // Set the USB device address of the current operation of the USB host
        SetUsbSpeed(DevOnHubPort[HubPortIndex - 1].DeviceSpeed);      // Set the current USB speed
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
        { // USB device plug-in event
            // mDelayuS(200); // Wait for the transmission to complete
            R8_USB_INT_FG = RB_UIF_DETECT;
            s = AnalyzeRootHub(); // Analyze ROOT-HUB status

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
            } // USB device connection event
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
            mDelayuS(200); // Wait for the transfer to complete
        }

        if(R8_USB_INT_FG & RB_UIF_TRANSFER) // Transmission completion event
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
                        break; // Timeout and try again
                    default:
                        return (ERR_USB_UNKNOWN); // Impossible situation
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
    if(RemLen && pBuf) // Need to send and receive data
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
                    break; // Short bag
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
                s = USBHostTransact(USB_PID_OUT << 4 | 0x00, R8_UH_TX_CTRL, 200000 / 20); // OUT data
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
            // R8_UH_TX_LEN = 0x01; // The status stage is IN
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
        return (ERR_SUCCESS); // Status IN, check IN status to return data length
    }
    return (ERR_USB_BUF_OVER); // IN status stage error
}

/* ***************************************************************************
* @fn CopySetupReqPkg
*
* @brief Copy control transfer request packet
*
* @param pReqPkt - Control request packet address
*
* @return none */
void CopySetupReqPkg(const uint8_t *pReqPkt) // Replication control transfer request packet
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
    UsbDevEndp0Size = ((PUSB_DEV_DESCR)Com_Buffer)->bMaxPacketSize0; // Endpoint 0 maximum packet length, this is a simplified process. Normally, you should first get the first 8 bytes and then update UsbDevEndp0Size immediately before continuing
    if(len < ((PUSB_SETUP_REQ)SetupGetDevDescr)->wLength)
    {
        return (ERR_USB_BUF_OVER); // Descriptor length error
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
    memcpy(TxBuffer, Com_Buffer, len); // When operating the USB flash drive, you need to copy it to TxBuffer
#endif

    return (ERR_SUCCESS);
}

/* ***************************************************************************
* @fn CtrlSetUsbAddress
*
* @brief Set the USB device address
*
* @param addr - Device address
*
* @return ERR_SUCCESS Success */
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
    mDelaymS(10);         // Wait for the USB device to complete operation
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
    return (HostCtrlTransfer(NULL, NULL)); // Perform control transmission
}

/* ***************************************************************************
* @fn CtrlClearEndpStall
*
* @brief Clear endpoint STALL
*
* @param endp - endpoint address
*
* @return ERR_SUCCESS Success */
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
