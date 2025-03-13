/********************************** (C) COPYRIGHT *******************************
 * File Name          : CH59x_usbhost.h
 * Author             : WCH
 * Version            : V1.2
 * Date               : 2021/11/17
 * Description
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#ifndef __CH59x_USBHOST_H__
#define __CH59x_USBHOST_H__

#ifdef __cplusplus
extern "C" {
#endif

#if DISK_LIB_ENABLE
  #if DISK_WITHOUT_USB_HUB
  /* Do not use the USB disk file system library or USB disk mount the USB hub, you need to close the following definition */
    #define FOR_ROOT_UDISK_ONLY
  #endif
  /* Use the USB disk file system library, you need to enable the following definition. Please close if you do not use it. */
  #define DISK_BASE_BUF_LEN    512  /* The default disk data buffer size is 512 bytes. It is recommended to select USB disks with 2048 or even 4096 to support certain large sectors. If 0 is, the buffer is prohibited from being defined in the .H file and specified by the application in pDISK_BASE_BUF. */
#endif

// Each subroutine returns status code
#define ERR_SUCCESS            0x00  // Operation is successful
#define ERR_USB_CONNECT        0x15  /* USB device connection event was detected and connected */
#define ERR_USB_DISCON         0x16  /* The USB device disconnection event was detected and it has been disconnected */
#define ERR_USB_BUF_OVER       0x17  /* The data transmitted by USB is incorrect or there is too much data and the buffer overflows */
#define ERR_USB_DISK_ERR       0x1F  /* The USB memory operation failed. During initialization, the USB memory may not be supported. During read and write operations, the disk may be damaged or disconnected. */
#define ERR_USB_TRANSFER       0x20  /* NAK/STALL and more error codes are in 0x20~0x2F */
#define ERR_USB_UNSUPPORT      0xFB  /* 不支持的USB设备*/
#define ERR_USB_UNKNOWN        0xFE  /* Equipment operation error */
#define ERR_AOA_PROTOCOL       0x41  /* There was an error in the protocol version */

/* USB device related information table, support up to 1 device */
#define ROOT_DEV_DISCONNECT    0
#define ROOT_DEV_CONNECTED     1
#define ROOT_DEV_FAILED        2
#define ROOT_DEV_SUCCESS       3
#define DEV_TYPE_KEYBOARD      (USB_DEV_CLASS_HID | 0x20)
#define DEV_TYPE_MOUSE         (USB_DEV_CLASS_HID | 0x30)
#define DEF_AOA_DEVICE         0xF0
#define DEV_TYPE_UNKNOW        0xFF

/* Convention: USB device address allocation rules (refer to USB_DEVICE_ADDR)
Address Value Device Location
0x02 USB device or external HUB under built-in Root-HUB
0x1x The USB device under the port x of the external HUB under the built-in Root-HUB, x is 1~n */
#define HUB_MAX_PORTS          4
#define WAIT_USB_TOUT_200US    800   // Waiting for USB interrupt timeout

typedef struct
{
    uint8_t  DeviceStatus;  // Device status, 0-No device, 1-There is a device but has not been initialized yet, 2-There is a device but the initialization enumeration failed, 3-There is a device and the initialization enumeration is successful
    uint8_t  DeviceAddress; // The USB address assigned by the device
    uint8_t  DeviceSpeed;   // 0 is low speed, non-0 is full speed
    uint8_t  DeviceType;    // 设备类型
    uint16_t DeviceVID;
    uint16_t DevicePID;
    uint8_t  GpVar[4];     // Common variables, storing endpoints
    uint8_t  GpHUBPortNum; // General variable, if it is HUB, it indicates the number of HUB ports
} _RootHubDev;

typedef struct
{
    UINT8  DeviceStatus;  // Device status, 0-No device, 1-There is a device but has not been initialized yet, 2-There is a device but the initialization enumeration failed, 3-There is a device and the initialization enumeration is successful
    UINT8  DeviceAddress; // The USB address assigned by the device
    UINT8  DeviceSpeed;   // 0 is low speed, non-0 is full speed
    UINT8  DeviceType;    // 设备类型
    UINT16 DeviceVID;
    UINT16 DevicePID;
    UINT8  GpVar[4]; // General variables
} _DevOnHubPort;     // Assuming: no more than 1 external HUB, each external HUB does not exceed HUB_MAX_PORTS ports (no matter if there are too many)

extern _RootHubDev   ThisUsbDev;
extern _DevOnHubPort DevOnHubPort[HUB_MAX_PORTS]; // 假定:不超过1个外部HUB,每个外部HUB不超过HUB_MAX_PORTS个端口(多了不管)
extern uint8_t       UsbDevEndp0Size;             // Maximum package size for endpoint 0 of USB device */
extern uint8_t       FoundNewDev;

extern uint8_t *pHOST_RX_RAM_Addr;
extern uint8_t *pHOST_TX_RAM_Addr;

extern _RootHubDev   ThisUsb2Dev;
extern _DevOnHubPort DevOnU2HubPort[HUB_MAX_PORTS]; // 假定:不超过1个外部HUB,每个外部HUB不超过HUB_MAX_PORTS个端口(多了不管)
extern uint8_t       Usb2DevEndp0Size;              // USB设备的端点0的最大包尺寸 */
extern uint8_t       FoundNewU2Dev;

extern uint8_t *pU2HOST_RX_RAM_Addr;
extern uint8_t *pU2HOST_TX_RAM_Addr;

#define pSetupReq      ((PUSB_SETUP_REQ)pHOST_TX_RAM_Addr)
#define pU2SetupReq    ((PUSB_SETUP_REQ)pU2HOST_TX_RAM_Addr)
extern uint8_t Com_Buffer[];
extern uint8_t U2Com_Buffer[];

/* The following is the USB host request package */
extern const uint8_t SetupGetDevDescr[];     // Get device descriptor*/
extern const uint8_t SetupGetCfgDescr[];     // Get the configuration descriptor*/
extern const uint8_t SetupSetUsbAddr[];      // Set USB address*/
extern const uint8_t SetupSetUsbConfig[];    // 设置USB配置*/
extern const uint8_t SetupSetUsbInterface[]; // Set USB interface configuration*/
extern const uint8_t SetupClrEndpStall[];    // Clear endpoint STALL*/

extern const uint8_t SetupGetU2DevDescr[];    // 获取设备描述符*/
extern const uint8_t SetupGetU2CfgDescr[];    // Get the configuration descriptor*/
extern const uint8_t SetupSetUsb2Addr[];      // Set USB address*/
extern const uint8_t SetupSetUsb2Config[];    // 设置USB配置*/
extern const uint8_t SetupSetUsb2Interface[]; // Set USB interface configuration*/
extern const uint8_t SetupClrU2EndpStall[];   // Clear endpoint STALL*/

/**
 * @brief   关闭ROOT-HUB端口,实际上硬件已经自动关闭,此处只是清除一些结构状态
 */
void DisableRootHubPort(void);

/* *
* @brief analyzes the ROOT-HUB status and handles the device plug-in and unplugging events of the ROOT-HUB port
* If the device is unplugged, the DisableRootHubPort() function is called in the function to close the port, insert the event, and set the status bit of the corresponding port.
*
* @return Return ERR_SUCCESS is no situation, return ERR_USB_CONNECT is a new connection is detected, return ERR_USB_DISCON is a disconnection is detected */
uint8_t AnalyzeRootHub(void);

/* *
* @brief Set the USB device address of the current operation of the USB host
*
* @param addr - USB device address */
void SetHostUsbAddr(uint8_t addr);

/**
 * @brief   设置当前USB速度
 *
 * @param   FullSpeed   - USB速度
 */
void SetUsbSpeed(uint8_t FullSpeed);

/* *
* @brief After detecting the device, reset the bus, prepare for the enumeration device, and set it to the default full speed */
void ResetRootHubPort(void);

/* *
* @brief Enable the ROOT-HUB port, and the corresponding bUH_PORT_EN is set to 1 to open the port. The device is disconnected and the return failure may result in the return failure.
*
* @return Return ERR_SUCCESS is detected as a new connection, return ERR_USB_DISCON is no connection */
uint8_t EnableRootHubPort(void);

/* *
* @brief Waiting for USB interruption
*
* @return Return ERR_SUCCESS Data received or sent successfully, Return ERR_USB_UNKNOWN Data received or sent failed */
uint8_t WaitUSB_Interrupt(void);

/* *
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
uint8_t USBHostTransact(uint8_t endp_pid, uint8_t tog, uint32_t timeout);

/**
 * @brief   执行控制传输,8字节请求码在pSetupReq中,DataBuf为可选的收发缓冲区
 *
 * @param   DataBuf     - 如果需要接收和发送数据,那么DataBuf需指向有效缓冲区用于存放后续数据
 * @param   RetLen      - 实际成功收发的总长度保存在RetLen指向的字节变量中
 *
 * @return  ERR_USB_BUF_OVER    IN状态阶段出错
 *          ERR_SUCCESS         数据交换成功
 */
uint8_t HostCtrlTransfer(uint8_t *DataBuf, uint8_t *RetLen);

/* *
* @brief Copy control transfer request packet
*
* @param pReqPkt - Control request packet address */
void CopySetupReqPkg(const uint8_t *pReqPkt);

/* *
* @brief Get the device descriptor, return it in pHOST_TX_RAM_Addr
*
* @return ERR_USB_BUF_OVER Descriptor length error
* ERR_SUCCESS Success */
uint8_t CtrlGetDeviceDescr(void);

/* *
* @brief Get the configuration descriptor, return it in pHOST_TX_RAM_Addr
*
* @return ERR_USB_BUF_OVER Descriptor length error
* ERR_SUCCESS Success */
uint8_t CtrlGetConfigDescr(void);

/* *
* @brief Set the USB device address
*
* @param addr - Device address
*
* @return ERR_SUCCESS Success */
uint8_t CtrlSetUsbAddress(uint8_t addr);

/* *
* @brief Setting USB device configuration
*
* @param cfg - Configuration value
*
* @return ERR_SUCCESS Success */
uint8_t CtrlSetUsbConfig(uint8_t cfg);

/* *
* @brief Clear endpoint STALL
*
* @param endp - endpoint address
*
* @return ERR_SUCCESS Success */
uint8_t CtrlClearEndpStall(uint8_t endp);

/* *
* @brief Setting up USB device interface
*
* @param cfg - Configuration value
*
* @return ERR_SUCCESS Success */
uint8_t CtrlSetUsbIntercace(uint8_t cfg);

/* *
* @brief USB host function initialization */
void USB_HostInit(void);
uint8_t EnumAllHubPort(void);// Enumerate all secondary USB devices after external HUB under ROOT-HUB ports
void SelectHubPort(uint8_t HubPortIndex); // HubPortIndex=0 Select the ROOT-HUB port specified by the operation, otherwise select the specified port of the external HUB of the ROOT-HUB port specified by the operation.
uint16_t SearchTypeDevice(uint8_t type); // Search for the port number of the device of the specified type on each port of ROOT-HUB and external HUB. If the output port number is 0xFFFF, no search is found.
uint8_t SETorOFFNumLock(uint8_t *buf); // NumLock's lighting judgment

/*************************************************************/

/**
 * @brief   初始化指定ROOT-HUB端口的USB设备
 *
 * @return  错误码
 */
uint8_t InitRootDevice(void);

/**
 * @brief   获取HID设备报表描述符,返回在TxBuffer中
 *
 * @return  错误码
 */
uint8_t CtrlGetHIDDeviceReport(uint8_t infc);

/* *
* @brief Get the HUB descriptor, return it in Com_Buffer
*
* @return Error code */
uint8_t CtrlGetHubDescr(void);

/* *
* @brief query the HUB port status, return to Com_Buffer
*
* @param HubPortIndex - Port number
*
* @return Error code */
uint8_t HubGetPortStatus(uint8_t HubPortIndex);

/* *
* @brief Set HUB port characteristics
*
* @param HubPortIndex - Port number
* @param FeatureSelt - Port Features
*
* @return Error code */
uint8_t HubSetPortFeature(uint8_t HubPortIndex, uint8_t FeatureSelt);

/* *
* @brief Clear HUB port features
*
* @param HubPortIndex - Port number
* @param FeatureSelt - Port Features
*
* @return Error code */
uint8_t HubClearPortFeature(uint8_t HubPortIndex, uint8_t FeatureSelt);

#ifdef __cplusplus
}
#endif

#endif // __CH59x_USBHOST_H__
