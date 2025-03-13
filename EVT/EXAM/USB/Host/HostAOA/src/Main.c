/* ********************************* (C) COPYRIGHT *******************************
* File Name          : Main.c
* Author             : WCH
* Version            : V1.1
* Date               : 2022/01/25
* Description        : USBÉè±¸Ã¶¾Ù
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
****************************************************************************** */

#include "CH59x_common.h"
// Each subroutine returns status code
#define ERR_SUCCESS          0x00  // Operation is successful
#define ERR_USB_CONNECT      0x15  /* USB device connection event was detected and connected */
#define ERR_USB_DISCON       0x16  /* The USB device disconnection event was detected and it has been disconnected */
#define ERR_USB_BUF_OVER     0x17  /* The data transmitted by USB is incorrect or there is too much data and the buffer overflows */
#define ERR_USB_DISK_ERR     0x1F  /* The USB memory operation failed. During initialization, the USB memory may not be supported. During read and write operations, the disk may be damaged or disconnected. */
#define ERR_USB_TRANSFER     0x20  /* NAK/STALL and more error codes are in 0x20~0x2F */
#define ERR_USB_UNSUPPORT    0xFB  /* Unsupported USB devices */
#define ERR_USB_UNKNOWN      0xFE  /* Equipment operation error */
#define ERR_AOA_PROTOCOL     0x41  /* There was an error in the protocol version */

__attribute__((aligned(4))) uint8_t RxBuffer[MAX_PACKET_SIZE]; // IN, must even address
__attribute__((aligned(4))) uint8_t TxBuffer[MAX_PACKET_SIZE]; // OUT, must even address
extern uint8_t                      Com_Buffer[];
// AOA gets protocol version
__attribute__((aligned(4))) const uint8_t GetProtocol[] = {0xc0, 0x33, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00};
// Start Accessories Mode
__attribute__((aligned(4))) const uint8_t TouchAOAMode[] = {0x40, 0x35, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
/* AOA related array definition */
__attribute__((aligned(4))) const uint8_t Sendlen[] = {0, 4, 16, 35, 39, 53, 67};
// String ID, string information related to mobile APP
__attribute__((aligned(4))) uint8_t StringID[] = {
    'W',
    'C',
    'H',
    0x00,                                                                                                             //manufacturer name
    'W', 'C', 'H', 'U', 'A', 'R', 'T', 'D', 'e', 'm', 'o', 0x00,                                                      //model name
    0x57, 0x43, 0x48, 0x20, 0x41, 0x63, 0x63, 0x65, 0x73, 0x73, 0x6f, 0x72, 0x79, 0x20, 0x54, 0x65, 0x73, 0x74, 0x00, //description
    '1', '.', '0', 0x00,                                                                                              //version
    0x68, 0x74, 0x74, 0x70, 0x3a, 0x2f, 0x2f, 0x77, 0x63, 0x68, 0x2e, 0x63, 0x6e, 0,                                  //URI
    0x57, 0x43, 0x48, 0x41, 0x63, 0x63, 0x65, 0x73, 0x73, 0x6f, 0x72, 0x79, 0x31, 0x00                                //serial number
};
// Apply index string command
__attribute__((aligned(4))) const uint8_t SetStringID[] = {0x40, 0x34, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x40, 0x34,
                                                           0x00, 0x00, 0x01, 0x00, 12, 0x00, 0x40, 0x34, 0x00, 0x00, 0x02,
                                                           0x00, 19, 0x00, 0x40, 0x34, 0x00, 0x00, 0x03, 0x00, 4, 0x00,
                                                           0x40, 0x34, 0x00, 0x00, 0x04, 0x00, 0x0E, 0x00, 0x40, 0x34,
                                                           0x00, 0x00, 0x05, 0x00, 0x0E, 0x00};

uint8_t TouchStartAOA(void); // Try to start AOA mode

/* ***************************************************************************
* @fn main
*
* @brief main function
*
* @return none */
int main()
{
    uint8_t s;
    uint8_t touchaoatm = 0;
    SetSysClock(CLK_SOURCE_PLL_60MHz);
    DelayMs(5);
    /* Turn on voltage monitoring */
    PowerMonitor(ENABLE, HALevel_2V1);

    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
    PRINT("Start @ChipID=%02X\n", R8_CHIP_ID);

    pHOST_RX_RAM_Addr = RxBuffer;
    pHOST_TX_RAM_Addr = TxBuffer;
    USB_HostInit();
    PRINT("Wait Device In\n");
    while(1)
    {
        s = ERR_SUCCESS;
        if(R8_USB_INT_FG & RB_UIF_DETECT)
        { // If there is a USB host detection interrupt, it will be handled
            R8_USB_INT_FG = RB_UIF_DETECT;
            s = AnalyzeRootHub();
            if(s == ERR_USB_CONNECT)
                FoundNewDev = 1;
        }

        if(FoundNewDev || s == ERR_USB_CONNECT)
        { // There is a new USB device plugged in
            FoundNewDev = 0;
            mDelaymS(200);        // Since the USB device has just been plugged in, it is not stable yet, so wait for the USB device to be hundreds of milliseconds to eliminate plug-in and unplug jitter
            s = InitRootDevice(); // Initialize USB device
            if((ThisUsbDev.DeviceVID == 0x18D1) && (ThisUsbDev.DevicePID & 0xff00) == 0x2D00)
            {
                PRINT("AOA Mode\n");
                ThisUsbDev.DeviceType = DEF_AOA_DEVICE;
            }
            else
            {                                        // If it is not AOA Accessories mode, try to start Accessories mode.
                SetUsbSpeed(ThisUsbDev.DeviceSpeed); // Set the current USB speed
                s = TouchStartAOA();
                if(s == ERR_SUCCESS)
                {
                    if(touchaoatm < 3) // AOA startup limit
                    {
                        FoundNewDev = 1;
                        touchaoatm++;
                        mDelaymS(500); // Some Android devices are automatically disconnected and reconnected, so it is best to have a delay here
                        continue;      // In fact, there is no need to jump here. The AOA protocol stipulates that the device will automatically reconnect to the bus.
                    }
                    // After this, it may not support AOA, or other devices
                    PRINT("UNKOWN Device\n");
                    SetUsbSpeed(1);
                    while(1);
                }
            }
            //if ( s != ERR_SUCCESS ) 	return( s );
        }
    }
}

/* ***************************************************************************
* @fn TouchStartAOA
*
* @brief Try to start AOA mode
*
* @return status */
uint8_t TouchStartAOA(void)
{
    uint8_t len, s, i, Num;
    // Get the protocol version number
    CopySetupReqPkg(GetProtocol);
    s = HostCtrlTransfer(Com_Buffer, &len); // Perform control transmission
    if(s != ERR_SUCCESS)
    {
        return (s);
    }
    if(Com_Buffer[0] < 2)
        return ERR_AOA_PROTOCOL;

    // Output string
    for(i = 0; i < 6; i++)
    {
        Num = Sendlen[i];
        CopySetupReqPkg(&SetStringID[8 * i]);
        s = HostCtrlTransfer(&StringID[Num], &len); // Perform control transmission
        if(s != ERR_SUCCESS)
        {
            return (s);
        }
    }

    CopySetupReqPkg(TouchAOAMode);
    s = HostCtrlTransfer(Com_Buffer, &len); // Perform control transmission
    if(s != ERR_SUCCESS)
    {
        return (s);
    }
    return ERR_SUCCESS;
}
