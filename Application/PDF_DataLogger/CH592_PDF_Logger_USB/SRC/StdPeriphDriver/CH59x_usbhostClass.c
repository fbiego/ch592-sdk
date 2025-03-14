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

/* 设置HID上传速率 */
__attribute__((aligned(4))) const uint8_t SetupSetHIDIdle[] = {0x21, HID_SET_IDLE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
/* 获取HID设备报表描述符 */
__attribute__((aligned(4))) const uint8_t SetupGetHIDDevReport[] = {0x81, USB_GET_DESCRIPTOR, 0x00, USB_DESCR_TYP_REPORT,
                                                                    0x00, 0x00, 0x41, 0x00};
/* Get the HUB descriptor */
__attribute__((aligned(4))) const uint8_t SetupGetHubDescr[] = {HUB_GET_HUB_DESCRIPTOR, HUB_GET_DESCRIPTOR, 0x00,
                                                                USB_DESCR_TYP_HUB, 0x00, 0x00, sizeof(USB_HUB_DESCR), 0x00};

__attribute__((aligned(4))) uint8_t Com_Buffer[128]; // Define user temporary buffers, which are used to process descriptors during enumeration, and the end of enumeration can also be used as a normal temporary buffer.

/*********************************************************************
 * @fn      AnalyzeHidIntEndp
 *
 * @brief   从描述符中分析出HID中断端点的地址,如果HubPortIndex是0保存到ROOTHUB，如果是非零值则保存到HUB下结构体
 *
 * @param   buf     - 待分析数据缓冲区地址 HubPortIndex：0表示根HUB，非0表示外部HUB下的端口号
 *
 * @return  端点数
 */
uint8_t AnalyzeHidIntEndp(uint8_t *buf, uint8_t HubPortIndex)
{
    uint8_t i, s, l;
    s = 0;

    if(HubPortIndex)
    {
        memset(DevOnHubPort[HubPortIndex - 1].GpVar, 0, sizeof(DevOnHubPort[HubPortIndex - 1].GpVar)); // Clear the array
    }
    else
    {
        memset(ThisUsbDev.GpVar, 0, sizeof(ThisUsbDev.GpVar)); //清空数组
    }

    for(i = 0; i < ((PUSB_CFG_DESCR)buf)->wTotalLength; i += l) // Search for interrupt endpoint descriptors, skip configuration descriptors and interface descriptors
    {
        if(((PUSB_ENDP_DESCR)(buf + i))->bDescriptorType == USB_DESCR_TYP_ENDP                         // 是端点描述符
           && (((PUSB_ENDP_DESCR)(buf + i))->bmAttributes & USB_ENDP_TYPE_MASK) == USB_ENDP_TYPE_INTER // 是中断端点
           && (((PUSB_ENDP_DESCR)(buf + i))->bEndpointAddress & USB_ENDP_DIR_MASK))                    // It's an IN endpoint
        {                                                                                              // Save the address of the interrupt endpoint, bit 7 is used to synchronize the flag bit, clear 0
            if(HubPortIndex)
            {
                DevOnHubPort[HubPortIndex - 1].GpVar[s] = ((PUSB_ENDP_DESCR)(buf + i))->bEndpointAddress & USB_ENDP_ADDR_MASK;
            }
            else
            {
                ThisUsbDev.GpVar[s] = ((PUSB_ENDP_DESCR)(buf + i))->bEndpointAddress & USB_ENDP_ADDR_MASK; // 中断端点的地址，可以根据需要保存wMaxPacketSize和bInterval
            }
            PRINT("%02x ", (uint16_t)ThisUsbDev.GpVar[s]);
            s++;
            if(s >= 4)
            {
                break; //只分析4个端点
            }
        }
        l = ((PUSB_ENDP_DESCR)(buf + i))->bLength; // Current descriptor length, skip
        if(l > 16)
        {
            break;
        }
    }
    PRINT("\n");
    return (s);
}

/* ***************************************************************************
* @fn AnalyzeBulkEndp
*
* @brief analyzes batch endpoints, and GpVar[0] and GpVar[1] store upload endpoints.GpVar[2] and GpVar[3] store the downward transmission endpoint
*
* @param buf - Data buffer address to be analyzed HubPortIndex: 0 represents the root HUB, non-0 represents the port number under the external HUB
*
* @return 0 */
uint8_t AnalyzeBulkEndp(uint8_t *buf, uint8_t HubPortIndex)
{
    uint8_t i, s1, s2, l;
    s1 = 0;
    s2 = 2;

    if(HubPortIndex)
    {
        memset(DevOnHubPort[HubPortIndex - 1].GpVar, 0, sizeof(DevOnHubPort[HubPortIndex - 1].GpVar)); //清空数组
    }
    else
    {
        memset(ThisUsbDev.GpVar, 0, sizeof(ThisUsbDev.GpVar)); // Clear the array
    }

    for(i = 0; i < ((PUSB_CFG_DESCR)buf)->wTotalLength; i += l) // 搜索中断端点描述符,跳过配置描述符和接口描述符
    {
        if((((PUSB_ENDP_DESCR)(buf + i))->bDescriptorType == USB_DESCR_TYP_ENDP)                         // is an endpoint descriptor
           && ((((PUSB_ENDP_DESCR)(buf + i))->bmAttributes & USB_ENDP_TYPE_MASK) == USB_ENDP_TYPE_BULK)) // It's the interrupt endpoint

        {
            if(HubPortIndex)
            {
                if(((PUSB_ENDP_DESCR)(buf + i))->bEndpointAddress & USB_ENDP_DIR_MASK)
                {
                    DevOnHubPort[HubPortIndex - 1].GpVar[s1++] = ((PUSB_ENDP_DESCR)(buf + i))->bEndpointAddress & USB_ENDP_ADDR_MASK;
                }
                else
                {
                    DevOnHubPort[HubPortIndex - 1].GpVar[s2++] = ((PUSB_ENDP_DESCR)(buf + i))->bEndpointAddress & USB_ENDP_ADDR_MASK;
                }
            }
            else
            {
                if(((PUSB_ENDP_DESCR)(buf + i))->bEndpointAddress & USB_ENDP_DIR_MASK)
                {
                    ThisUsbDev.GpVar[s1++] = ((PUSB_ENDP_DESCR)(buf + i))->bEndpointAddress & USB_ENDP_ADDR_MASK;
                }
                else
                {
                    ThisUsbDev.GpVar[s2++] = ((PUSB_ENDP_DESCR)(buf + i))->bEndpointAddress & USB_ENDP_ADDR_MASK;
                }
            }

            if(s1 == 2)
            {
                s1 = 1;
            }
            if(s2 == 4)
            {
                s2 = 3;
            }
        }
        l = ((PUSB_ENDP_DESCR)(buf + i))->bLength; // Current descriptor length, skip
        if(l > 16)
        {
            break;
        }
    }
    return (0);
}

/* ***************************************************************************
* @fn InitRootDevice
*
* @brief Initialize the USB device with the specified ROOT-HUB port
*
* @param none
*
* @return Error code */
uint8_t InitRootDevice(void)
{
    uint8_t i, s;
    uint8_t cfg, dv_cls, if_cls;

    PRINT("Reset host port\n");
    ResetRootHubPort(); // After detecting the device, reset the USB bus of the corresponding port
    for(i = 0, s = 0; i < 100; i++)
    { // Wait for the USB device to reset and reconnect, 100mS timeout
        mDelaymS(1);
        if(EnableRootHubPort() == ERR_SUCCESS)
        { // Enable port
            i = 0;
            s++;
            if(s > 100)
            {
                break; // It has been stably connected to 100mS
            }
        }
    }
    if(i)
    { // The device is not connected after reset
        DisableRootHubPort();
        PRINT("Disable host port because of disconnect\n");
        return (ERR_USB_DISCON);
    }
    SetUsbSpeed(ThisUsbDev.DeviceSpeed); // Set the current USB speed

    PRINT("GetDevDescr: ");
    s = CtrlGetDeviceDescr(); // Get the device descriptor
    if(s == ERR_SUCCESS)
    {
        for(i = 0; i < ((PUSB_SETUP_REQ)SetupGetDevDescr)->wLength; i++)
        {
            PRINT("x%02X ", (uint16_t)(Com_Buffer[i]));
        }
        PRINT("\n");

        ThisUsbDev.DeviceVID = ((PUSB_DEV_DESCR)Com_Buffer)->idVendor; // Save VID PID information
        ThisUsbDev.DevicePID = ((PUSB_DEV_DESCR)Com_Buffer)->idProduct;
        dv_cls = ((PUSB_DEV_DESCR)Com_Buffer)->bDeviceClass;

        s = CtrlSetUsbAddress(((PUSB_SETUP_REQ)SetupSetUsbAddr)->wValue);
        if(s == ERR_SUCCESS)
        {
            ThisUsbDev.DeviceAddress = ((PUSB_SETUP_REQ)SetupSetUsbAddr)->wValue; // Save USB address

            PRINT("GetCfgDescr: ");
            s = CtrlGetConfigDescr();
            if(s == ERR_SUCCESS)
            {
                for(i = 0; i < ((PUSB_CFG_DESCR)Com_Buffer)->wTotalLength; i++)
                {
                    PRINT("x%02X ", (uint16_t)(Com_Buffer[i]));
                }
                PRINT("\n");
                /* Analyze configuration descriptors, get endpoint data/endpoint addresses/endpoint sizes, etc., update variables endp_addr and endp_size, etc. */
                cfg = ((PUSB_CFG_DESCR)Com_Buffer)->bConfigurationValue;
                if_cls = ((PUSB_CFG_DESCR_LONG)Com_Buffer)->itf_descr.bInterfaceClass; // Interface code

                if((dv_cls == 0x00) && (if_cls == USB_DEV_CLASS_STORAGE))
                { // It is a USB storage device, basically confirmed it is a USB drive
#ifdef FOR_ROOT_UDISK_ONLY
                    CHRV3DiskStatus = DISK_USB_ADDR;
                    return (ERR_SUCCESS);
                }
                else
                {
                    return (ERR_USB_UNSUPPORT);
                }
#else
                    s = CtrlSetUsbConfig(cfg); // 设置USB设备配置
                    if(s == ERR_SUCCESS)
                    {
                        ThisUsbDev.DeviceStatus = ROOT_DEV_SUCCESS;
                        ThisUsbDev.DeviceType = USB_DEV_CLASS_STORAGE;
                        PRINT("USB-Disk Ready\n");
                        SetUsbSpeed(1); // Default is full speed
                        return (ERR_SUCCESS);
                    }
                }
                else if((dv_cls == 0x00) && (if_cls == USB_DEV_CLASS_PRINTER) && ((PUSB_CFG_DESCR_LONG)Com_Buffer)->itf_descr.bInterfaceSubClass == 0x01)
                {                              // It is a printer device
                    s = CtrlSetUsbConfig(cfg); // Set up USB device configuration
                    if(s == ERR_SUCCESS)
                    {
                        // Endpoint information needs to be saved for the main program to perform USB transmission
                        ThisUsbDev.DeviceStatus = ROOT_DEV_SUCCESS;
                        ThisUsbDev.DeviceType = USB_DEV_CLASS_PRINTER;
                        PRINT("USB-Print Ready\n");
                        SetUsbSpeed(1); // Default is full speed
                        return (ERR_SUCCESS);
                    }
                }
                else if((dv_cls == 0x00) && (if_cls == USB_DEV_CLASS_HID) && ((PUSB_CFG_DESCR_LONG)Com_Buffer)->itf_descr.bInterfaceSubClass <= 0x01)
                { // It is a HID device, keyboard/mouse, etc.
                    // Analyze the address of the HID interrupt endpoint from the descriptor
                    s = AnalyzeHidIntEndp(Com_Buffer, 0); // 从描述符中分析出HID中断端点的地址
                    PRINT("AnalyzeHidIntEndp %02x\n", (uint16_t)s);
                    // Save the address of the interrupt endpoint, bit 7 is used to synchronize the flag bit, clear 0
                    if_cls = ((PUSB_CFG_DESCR_LONG)Com_Buffer)->itf_descr.bInterfaceProtocol;
                    s = CtrlSetUsbConfig(cfg); // Set up USB device configuration
                    if(s == ERR_SUCCESS)
                    {
                        //	    					Set_Idle( );
                        // Endpoint information needs to be saved for the main program to perform USB transmission
                        ThisUsbDev.DeviceStatus = ROOT_DEV_SUCCESS;
                        if(if_cls == 1)
                        {
                            ThisUsbDev.DeviceType = DEV_TYPE_KEYBOARD;
                            //	进一步初始化,例如设备键盘指示灯LED等
                            PRINT("USB-Keyboard Ready\n");
                            SetUsbSpeed(1); // Default is full speed
                            return (ERR_SUCCESS);
                        }
                        else if(if_cls == 2)
                        {
                            ThisUsbDev.DeviceType = DEV_TYPE_MOUSE;
                            // In order to query the mouse status in the future, the descriptor should be analyzed and the address, length and other information of the interrupt port should be obtained.
                            PRINT("USB-Mouse Ready\n");
                            SetUsbSpeed(1); // Default is full speed
                            return (ERR_SUCCESS);
                        }
                        s = ERR_USB_UNSUPPORT;
                    }
                }
                else if(dv_cls == USB_DEV_CLASS_HUB)
                { // It is HUB equipment, hubs, etc.
                    s = CtrlGetHubDescr();
                    if(s == ERR_SUCCESS)
                    {
                        PRINT("Max Port:%02X ", (((PXUSB_HUB_DESCR)Com_Buffer)->bNbrPorts));
                        ThisUsbDev.GpHUBPortNum = ((PXUSB_HUB_DESCR)Com_Buffer)->bNbrPorts; // Save the number of ports to HUB
                        if(ThisUsbDev.GpHUBPortNum > HUB_MAX_PORTS)
                        {
                            ThisUsbDev.GpHUBPortNum = HUB_MAX_PORTS; // Because when defining the structure DevOnHubPort, it is artificially assumed that each HUB does not exceed HUB_MAX_PORTS ports per HUB_MAX_PORTS
                        }
                        s = CtrlSetUsbConfig(cfg); // Set up USB device configuration
                        if(s == ERR_SUCCESS)
                        {
                            ThisUsbDev.DeviceStatus = ROOT_DEV_SUCCESS;
                            ThisUsbDev.DeviceType = USB_DEV_CLASS_HUB;
                            // The endpoint information needs to be saved for the main program to perform USB transmission. Originally, the interrupt endpoint can be used for HUB event notification, but this program uses query status control transmission instead.
                            // Power on each port of HUB, query the status of each port, initialize the HUB port connected to the device, and initialize the device
                            for(i = 1; i <= ThisUsbDev.GpHUBPortNum; i++) // Power on all ports of HUB
                            {
                                DevOnHubPort[i - 1].DeviceStatus = ROOT_DEV_DISCONNECT; // 清外部HUB端口上设备的状态
                                s = HubSetPortFeature(i, HUB_PORT_POWER);
                                if(s != ERR_SUCCESS)
                                {
                                    PRINT("Ext-HUB Port_%1d# power on error\n", (uint16_t)i); // 端口上电失败
                                }
                            }
                            PRINT("USB-HUB Ready\n");
                            SetUsbSpeed(1); // Default is full speed
                            return (ERR_SUCCESS);
                        }
                    }
                }
                else
                {                              // 可以进一步分析
                    s = CtrlSetUsbConfig(cfg); // 设置USB设备配置
                    if(s == ERR_SUCCESS)
                    {
                        // Endpoint information needs to be saved for the main program to perform USB transmission
                        ThisUsbDev.DeviceStatus = ROOT_DEV_SUCCESS;
                        ThisUsbDev.DeviceType = DEV_TYPE_UNKNOW;
                        SetUsbSpeed(1);       // 默认为全速
                        return (ERR_SUCCESS); /* Unknown device initialization successfully */
                    }
                }
#endif
            }
        }
    }

    PRINT("InitRootDev Err = %02X\n", (uint16_t)s);
#ifdef FOR_ROOT_UDISK_ONLY
    CHRV3DiskStatus = DISK_CONNECT;
#else
    ThisUsbDev.DeviceStatus = ROOT_DEV_FAILED;
#endif
    SetUsbSpeed(1); // Default is full speed
    return (s);
}

/* ***************************************************************************
* @fn InitDevOnHub
*
* @brief The secondary USB device after initializing the enumeration of external HUBs
*
* @param HubPortIndex - Specify external HUB
*
* @return Error code */
uint8_t InitDevOnHub(uint8_t HubPortIndex)
{
    uint8_t i, s, cfg, dv_cls, if_cls;
    uint8_t ifc;
    PRINT("Init dev @ExtHub-port_%1d ", (uint16_t)HubPortIndex);
    if(HubPortIndex == 0)
    {
        return (ERR_USB_UNKNOWN);
    }
    SelectHubPort(HubPortIndex); // 选择操作指定的ROOT-HUB端口的外部HUB的指定端口,选择速度
    PRINT("GetDevDescr: ");
    s = CtrlGetDeviceDescr(); // Get the device descriptor
    if(s != ERR_SUCCESS)
    {
        return (s);
    }
    DevOnHubPort[HubPortIndex - 1].DeviceVID = ((uint16_t)((PUSB_DEV_DESCR)Com_Buffer)->idVendor); // Save VID PID information
    DevOnHubPort[HubPortIndex - 1].DevicePID = ((uint16_t)((PUSB_DEV_DESCR)Com_Buffer)->idProduct);

    dv_cls = ((PUSB_DEV_DESCR)Com_Buffer)->bDeviceClass; // Device class code
    cfg = (1 << 4) + HubPortIndex;                       // Calculate a USB address to avoid overlapping addresses
    s = CtrlSetUsbAddress(cfg);                          // Set the USB device address
    if(s != ERR_SUCCESS)
    {
        return (s);
    }
    DevOnHubPort[HubPortIndex - 1].DeviceAddress = cfg; // 保存分配的USB地址
    PRINT("GetCfgDescr: ");
    s = CtrlGetConfigDescr(); // Get the configuration descriptor
    if(s != ERR_SUCCESS)
    {
        return (s);
    }
    cfg = ((PUSB_CFG_DESCR)Com_Buffer)->bConfigurationValue;
    for(i = 0; i < ((PUSB_CFG_DESCR)Com_Buffer)->wTotalLength; i++)
    {
        PRINT("x%02X ", (uint16_t)(Com_Buffer[i]));
    }
    PRINT("\n");
    /* 分析配置描述符,获取端点数据/各端点地址/各端点大小等,更新变量endp_addr和endp_size等 */
    if_cls = ((PXUSB_CFG_DESCR_LONG)Com_Buffer)->itf_descr.bInterfaceClass; // Interface code
    if(dv_cls == 0x00 && if_cls == USB_DEV_CLASS_STORAGE)                   // It is a USB storage device, basically confirmed it is a USB drive
    {
        AnalyzeBulkEndp(Com_Buffer, HubPortIndex);
        for(i = 0; i != 4; i++)
        {
            PRINT("%02x ", (uint16_t)DevOnHubPort[HubPortIndex - 1].GpVar[i]);
        }
        PRINT("\n");
        s = CtrlSetUsbConfig(cfg); // Set up USB device configuration
        if(s == ERR_SUCCESS)
        {
            DevOnHubPort[HubPortIndex - 1].DeviceStatus = ROOT_DEV_SUCCESS;
            DevOnHubPort[HubPortIndex - 1].DeviceType = USB_DEV_CLASS_STORAGE;
            PRINT("USB-Disk Ready\n");
            SetUsbSpeed(1); // Default is full speed
            return (ERR_SUCCESS);
        }
    }
    else if((dv_cls == 0x00) && (if_cls == USB_DEV_CLASS_HID) && (((PXUSB_CFG_DESCR_LONG)Com_Buffer)->itf_descr.bInterfaceSubClass <= 0x01)) // It is a HID device, keyboard/mouse, etc.
    {
        ifc = ((PXUSB_CFG_DESCR_LONG)Com_Buffer)->cfg_descr.bNumInterfaces;
        s = AnalyzeHidIntEndp(Com_Buffer, HubPortIndex); // Analyze the address of the HID interrupt endpoint from the descriptor
        PRINT("AnalyzeHidIntEndp %02x\n", (uint16_t)s);
        if_cls = ((PXUSB_CFG_DESCR_LONG)Com_Buffer)->itf_descr.bInterfaceProtocol;
        s = CtrlSetUsbConfig(cfg); // 设置USB设备配置
        if(s == ERR_SUCCESS)
        {
            for(dv_cls = 0; dv_cls < ifc; dv_cls++)
            {
                s = CtrlGetHIDDeviceReport(dv_cls); // Get report descriptor
                if(s == ERR_SUCCESS)
                {
                    for(i = 0; i < 64; i++)
                    {
                        PRINT("x%02X ", (uint16_t)(Com_Buffer[i]));
                    }
                    PRINT("\n");
                }
            }
            //需保存端点信息以便主程序进行USB传输
            DevOnHubPort[HubPortIndex - 1].DeviceStatus = ROOT_DEV_SUCCESS;
            if(if_cls == 1)
            {
                DevOnHubPort[HubPortIndex - 1].DeviceType = DEV_TYPE_KEYBOARD;
                //进一步初始化,例如设备键盘指示灯LED等
                if(ifc > 1)
                {
                    PRINT("USB_DEV_CLASS_HID Ready\n");
                    DevOnHubPort[HubPortIndex - 1].DeviceType = USB_DEV_CLASS_HID; //复合HID设备
                }
                PRINT("USB-Keyboard Ready\n");
                SetUsbSpeed(1); // Default is full speed

                return (ERR_SUCCESS);
            }
            else if(if_cls == 2)
            {
                DevOnHubPort[HubPortIndex - 1].DeviceType = DEV_TYPE_MOUSE;
                // In order to query the mouse status in the future, the descriptor should be analyzed and the address, length and other information of the interrupt port should be obtained.
                if(ifc > 1)
                {
                    PRINT("USB_DEV_CLASS_HID Ready\n");
                    DevOnHubPort[HubPortIndex - 1].DeviceType = USB_DEV_CLASS_HID; // Composite HID devices
                }
                PRINT("USB-Mouse Ready\n");
                SetUsbSpeed(1); // Default is full speed

                return (ERR_SUCCESS);
            }
            s = ERR_USB_UNSUPPORT;
        }
    }
    else if(dv_cls == USB_DEV_CLASS_HUB) // It is HUB equipment, hubs, etc.
    {
        DevOnHubPort[HubPortIndex - 1].DeviceType = USB_DEV_CLASS_HUB;
        PRINT("This program don't support Level 2 HUB\n"); // If you need to support multi-level HUB cascade, please refer to this program for extension.
        s = HubClearPortFeature(i, HUB_PORT_ENABLE);       // 禁止HUB端口
        if(s != ERR_SUCCESS)
        {
            return (s);
        }
        s = ERR_USB_UNSUPPORT;
    }
    else // Other equipment
    {
        AnalyzeBulkEndp(Com_Buffer, HubPortIndex); //分析出批量端点
        for(i = 0; i != 4; i++)
        {
            PRINT("%02x ", (uint16_t)DevOnHubPort[HubPortIndex - 1].GpVar[i]);
        }
        PRINT("\n");
        s = CtrlSetUsbConfig(cfg); // Set up USB device configuration
        if(s == ERR_SUCCESS)
        {
            // Endpoint information needs to be saved for the main program to perform USB transmission
            DevOnHubPort[HubPortIndex - 1].DeviceStatus = ROOT_DEV_SUCCESS;
            DevOnHubPort[HubPortIndex - 1].DeviceType = dv_cls ? dv_cls : if_cls;
            SetUsbSpeed(1);       // Default is full speed
            return (ERR_SUCCESS); //未知设备初始化成功
        }
    }
    PRINT("InitDevOnHub Err = %02X\n", (uint16_t)s);
    DevOnHubPort[HubPortIndex - 1].DeviceStatus = ROOT_DEV_FAILED;
    SetUsbSpeed(1); // Default is full speed
    return (s);
}

/* ***************************************************************************
* @fn EnumHubPort
*
* @brief Enumerate the ports of the external HUB hub on the specified ROOT-HUB port, check whether each port has connections or remove events and initialize the secondary USB device
*
* @param RootHubIndex - ROOT_HUB0 and ROOT_HUB1
*
* @return Error code */
uint8_t EnumHubPort()
{
    uint8_t i, s;

    for(i = 1; i <= ThisUsbDev.GpHUBPortNum; i++) // Check whether the port of the hub has changed
    {
        SelectHubPort(0);        // Select the ROOT-HUB port specified in the operation, set the current USB speed and the USB address of the operating device
        s = HubGetPortStatus(i); // Get the port status
        if(s != ERR_SUCCESS)
        {
            return (s); // Maybe the HUB is disconnected
        }
        if(((Com_Buffer[0] & (1 << (HUB_PORT_CONNECTION & 0x07))) && (Com_Buffer[2] & (1 << (HUB_C_PORT_CONNECTION & 0x07)))) || (Com_Buffer[2] == 0x10))
        {                                                          // Found a device connected
            DevOnHubPort[i - 1].DeviceStatus = ROOT_DEV_CONNECTED; // Connected with equipment
            DevOnHubPort[i - 1].DeviceAddress = 0x00;
            s = HubGetPortStatus(i); // Get the port status
            if(s != ERR_SUCCESS)
            {
                return (s); // Maybe the HUB is disconnected
            }
            DevOnHubPort[i - 1].DeviceSpeed = Com_Buffer[1] & (1 << (HUB_PORT_LOW_SPEED & 0x07)) ? 0 : 1; // Low or full speed
            if(DevOnHubPort[i - 1].DeviceSpeed)
            {
                PRINT("Found full speed device on port %1d\n", (uint16_t)i);
            }
            else
            {
                PRINT("Found low speed device on port %1d\n", (uint16_t)i);
            }
            mDelaymS(200);                            // Wait for the device to power on and stabilize
            s = HubSetPortFeature(i, HUB_PORT_RESET); // Reset the port connected to the device
            if(s != ERR_SUCCESS)
            {
                return (s); // Maybe the HUB is disconnected
            }
            PRINT("Reset port and then wait in\n");
            do // 查询复位端口,直到复位完成,把完成后的状态显示出来
            {
                mDelaymS(1);
                s = HubGetPortStatus(i);
                if(s != ERR_SUCCESS)
                {
                    return (s); // Maybe the HUB is disconnected
                }
            } while(Com_Buffer[0] & (1 << (HUB_PORT_RESET & 0x07))); // The port is resetting and waiting
            mDelaymS(100);
            s = HubClearPortFeature(i, HUB_C_PORT_RESET);      // Clear the reset completion flag
                                                               // s = HubSetPortFeature( i, HUB_PORT_ENABLE ); // Enable HUB port
            s = HubClearPortFeature(i, HUB_C_PORT_CONNECTION); // Clear connection or remove change flags
            if(s != ERR_SUCCESS)
            {
                return (s);
            }
            s = HubGetPortStatus(i); // Read the status again and check whether the device is still there
            if(s != ERR_SUCCESS)
            {
                return (s);
            }
            if((Com_Buffer[0] & (1 << (HUB_PORT_CONNECTION & 0x07))) == 0)
            {
                DevOnHubPort[i - 1].DeviceStatus = ROOT_DEV_DISCONNECT; // 设备不在了
            }
            s = InitDevOnHub(i); // Initialize a secondary USB device
            if(s != ERR_SUCCESS)
            {
                return (s);
            }
            SetUsbSpeed(1); // Default is full speed
        }
        else if(Com_Buffer[2] & (1 << (HUB_C_PORT_ENABLE & 0x07))) // Device connection error
        {
            HubClearPortFeature(i, HUB_C_PORT_ENABLE); // Clear the connection error flag
            PRINT("Device on port error\n");
            s = HubSetPortFeature(i, HUB_PORT_RESET); // Reset the port connected to the device
            if(s != ERR_SUCCESS)
                return (s); // 可能是该HUB断开了
            do              // Query the reset port until the reset is completed and display the completed status
            {
                mDelaymS(1);
                s = HubGetPortStatus(i);
                if(s != ERR_SUCCESS)
                    return (s);                                      // 可能是该HUB断开了
            } while(Com_Buffer[0] & (1 << (HUB_PORT_RESET & 0x07))); // The port is resetting and waiting
        }
        else if((Com_Buffer[0] & (1 << (HUB_PORT_CONNECTION & 0x07))) == 0) // The device has been disconnected
        {
            if(DevOnHubPort[i - 1].DeviceStatus >= ROOT_DEV_CONNECTED)
            {
                PRINT("Device on port %1d removed\n", (uint16_t)i);
            }
            DevOnHubPort[i - 1].DeviceStatus = ROOT_DEV_DISCONNECT; // 有设备连接
            if(Com_Buffer[2] & (1 << (HUB_C_PORT_CONNECTION & 0x07)))
            {
                HubClearPortFeature(i, HUB_C_PORT_CONNECTION); // Clear Remove Change Flag
            }
        }
    }
    return (ERR_SUCCESS); // 返回操作成功
}

/* ***************************************************************************
* @fn EnumAllHubPort
*
* @brief Enumerate all secondary USB devices after external HUB under ROOT-HUB ports
*
* @return Error code */
uint8_t EnumAllHubPort(void)
{
    uint8_t s;

    if((ThisUsbDev.DeviceStatus >= ROOT_DEV_SUCCESS) && (ThisUsbDev.DeviceType == USB_DEV_CLASS_HUB)) // HUB enumeration succeeded
    {
        SelectHubPort(0);    // Select the ROOT-HUB port specified in the operation, set the current USB speed and the USB address of the operating device
        s = EnumHubPort();   // Enumerate the ports of the external HUB hub on the specified ROOT-HUB port, check whether each port has connection or remove events
        if(s != ERR_SUCCESS) // Maybe HUB is disconnected
        {
            PRINT("EnumAllHubPort err = %02X\n", (uint16_t)s);
        }
        SetUsbSpeed(1); // Default is full speed
    }
    return (ERR_SUCCESS);
}

/* ***************************************************************************
* @fn SearchTypeDevice
*
* @brief Search for the port number of the device of the specified type on each port of ROOT-HUB and external HUB. If the output port number is 0xFFFF, no search was found.
* Of course, you can also search based on the USB manufacturer's VID product PID (the VID and PID of each device must be recorded in advance), and the search serial number can be specified.
*
* @param type - the type of device to search
*
* @return The output is the ROOT-HUB port number, the lower 8 bits are the port number of the external HUB, and the lower 8 bits are 0, the device is directly on the ROOT-HUB port. */
uint16_t SearchTypeDevice(uint8_t type)
{
    uint8_t RootHubIndex; //CH554只有一个USB口,RootHubIndex = 0,只需看返回值的低八位即可
    uint8_t HubPortIndex;

    RootHubIndex = 0;
    if((ThisUsbDev.DeviceType == USB_DEV_CLASS_HUB) && (ThisUsbDev.DeviceStatus >= ROOT_DEV_SUCCESS)) // 外部集线器HUB且枚举成功
    {
        for(HubPortIndex = 1; HubPortIndex <= ThisUsbDev.GpHUBPortNum; HubPortIndex++) // 搜索外部HUB的各个端口
        {
            if(DevOnHubPort[HubPortIndex - 1].DeviceType == type && DevOnHubPort[HubPortIndex - 1].DeviceStatus >= ROOT_DEV_SUCCESS)
            {
                return (((uint16_t)RootHubIndex << 8) | HubPortIndex); // Type matching and enumeration succeeds
            }
        }
    }
    if((ThisUsbDev.DeviceType == type) && (ThisUsbDev.DeviceStatus >= ROOT_DEV_SUCCESS))
    {
        return ((uint16_t)RootHubIndex << 8); // Type matching and enumeration is successful, on the ROOT-HUB port
    }

    return (0xFFFF);
}

/* ***************************************************************************
* @fn SETorOFFNumLock
*
* @brief NumLock's lighting judgment
*
* @param buf - Lighting key value
*
* @return Error code */
uint8_t SETorOFFNumLock(uint8_t *buf)
{
    uint8_t tmp[] = {0x21, 0x09, 0x00, 0x02, 0x00, 0x00, 0x01, 0x00};
    uint8_t len, s;
    if((buf[2] == 0x53) & ((buf[0] | buf[1] | buf[3] | buf[4] | buf[5] | buf[6] | buf[7]) == 0))
    {
        for(s = 0; s != sizeof(tmp); s++)
        {
            ((uint8_t *)pSetupReq)[s] = tmp[s];
        }
        s = HostCtrlTransfer(Com_Buffer, &len); // Perform control transmission
        if(s != ERR_SUCCESS)
        {
            return (s);
        }
    }
    return (ERR_SUCCESS);
}

/* ***************************************************************************
* @fn CtrlGetHIDDeviceReport
*
* @brief Get the HID device report descriptor, return it in TxBuffer
*
* @param none
*
* @return Error code */
uint8_t CtrlGetHIDDeviceReport(uint8_t infc)
{
    uint8_t s;
    uint8_t len;

    CopySetupReqPkg(SetupSetHIDIdle);
    pSetupReq->wIndex = infc;
    s = HostCtrlTransfer(Com_Buffer, &len); // Perform control transmission
    if(s != ERR_SUCCESS)
    {
        return (s);
    }

    CopySetupReqPkg(SetupGetHIDDevReport);
    pSetupReq->wIndex = infc;
    s = HostCtrlTransfer(Com_Buffer, &len); // Perform control transmission
    if(s != ERR_SUCCESS)
    {
        return (s);
    }

    return (ERR_SUCCESS);
}

/* ***************************************************************************
* @fn CtrlGetHubDescr
*
* @brief Get the HUB descriptor, return it in Com_Buffer
*
* @param none
*
* @return Error code */
uint8_t CtrlGetHubDescr(void)
{
    uint8_t s;
    uint8_t len;

    CopySetupReqPkg(SetupGetHubDescr);
    s = HostCtrlTransfer(Com_Buffer, &len); // Perform control transmission
    if(s != ERR_SUCCESS)
    {
        return (s);
    }
    if(len < ((PUSB_SETUP_REQ)SetupGetHubDescr)->wLength)
    {
        return (ERR_USB_BUF_OVER); // Descriptor length error
    }
    //  if ( len < 4 ) return( ERR_USB_BUF_OVER );                                 // 描述符长度错误
    return (ERR_SUCCESS);
}

/* ***************************************************************************
* @fn HubGetPortStatus
*
* @brief query the HUB port status, return to Com_Buffer
*
* @param HubPortIndex - Port number
*
* @return Error code */
uint8_t HubGetPortStatus(uint8_t HubPortIndex)
{
    uint8_t s;
    uint8_t len;

    pSetupReq->bRequestType = HUB_GET_PORT_STATUS;
    pSetupReq->bRequest = HUB_GET_STATUS;
    pSetupReq->wValue = 0x0000;
    pSetupReq->wIndex = 0x0000 | HubPortIndex;
    pSetupReq->wLength = 0x0004;
    s = HostCtrlTransfer(Com_Buffer, &len); // 执行控制传输
    if(s != ERR_SUCCESS)
    {
        return (s);
    }
    if(len < 4)
    {
        return (ERR_USB_BUF_OVER); // Descriptor length error
    }
    return (ERR_SUCCESS);
}

/* ***************************************************************************
* @fn HubSetPortFeature
*
* @brief Set HUB port characteristics
*
* @param HubPortIndex - Port number
* @param FeatureSelt - Port Features
*
* @return Error code */
uint8_t HubSetPortFeature(uint8_t HubPortIndex, uint8_t FeatureSelt)
{
    pSetupReq->bRequestType = HUB_SET_PORT_FEATURE;
    pSetupReq->bRequest = HUB_SET_FEATURE;
    pSetupReq->wValue = 0x0000 | FeatureSelt;
    pSetupReq->wIndex = 0x0000 | HubPortIndex;
    pSetupReq->wLength = 0x0000;
    return (HostCtrlTransfer(NULL, NULL)); // Perform control transmission
}

/*********************************************************************
 * @fn      HubClearPortFeature
 *
 * @brief   清除HUB端口特性
 *
 * @param   HubPortIndex    - 端口号
 * @param   FeatureSelt     - 端口特性
 *
 * @return  错误码
 */
uint8_t HubClearPortFeature(uint8_t HubPortIndex, uint8_t FeatureSelt)
{
    pSetupReq->bRequestType = HUB_CLEAR_PORT_FEATURE;
    pSetupReq->bRequest = HUB_CLEAR_FEATURE;
    pSetupReq->wValue = 0x0000 | FeatureSelt;
    pSetupReq->wIndex = 0x0000 | HubPortIndex;
    pSetupReq->wLength = 0x0000;
    return (HostCtrlTransfer(NULL, NULL)); // 执行控制传输
}
