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

__attribute__((aligned(4))) uint8_t RxBuffer[MAX_PACKET_SIZE]; // IN, must even address
__attribute__((aligned(4))) uint8_t TxBuffer[MAX_PACKET_SIZE]; // OUT, must even address

/* ***************************************************************************
* @fn main
*
* @brief main function
*
* @return none */
int main()
{
    uint8_t i, s, k, len, endp;
    uint16_t  loc;

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
            if(s != ERR_SUCCESS)
            {
                PRINT("EnumAllRootDev err = %02X\n", (uint16_t)s);
            }
        }

        /* If the HUB is connected to the lower end, first enumerate the HUB */
        s = EnumAllHubPort(); // Enumerate all secondary USB devices after external HUB under ROOT-HUB ports
        if(s != ERR_SUCCESS)
        { // Maybe HUB is disconnected
            PRINT("EnumAllHubPort err = %02X\n", (uint16_t)s);
        }

        /* If the device is a mouse */
        loc = SearchTypeDevice(DEV_TYPE_MOUSE); // Search the port number of the specified type of device on each port of ROOT-HUB and external HUB
        if(loc != 0xFFFF)
        { // Found, what if there are two MOUSEs?
            i = (uint8_t)(loc >> 8);
            len = (uint8_t)loc;
            SelectHubPort(len);                                                // Select the ROOT-HUB port specified in the operation, set the current USB speed and the USB address of the operating device
            endp = len ? DevOnHubPort[len - 1].GpVar[0] : ThisUsbDev.GpVar[0]; // The address of the interrupt endpoint, bit 7 is used to synchronize the flag bit
            if(endp & USB_ENDP_ADDR_MASK)
            {                                                                                                       // Endpoint valid
                s = USBHostTransact(USB_PID_IN << 4 | endp & 0x7F, endp & 0x80 ? RB_UH_R_TOG | RB_UH_T_TOG : 0, 0); // Transfer transactions, get data, NAK does not try again
                if(s == ERR_SUCCESS)
                {
                    endp ^= 0x80; // Synchronous flag flip
                    if(len)
                        DevOnHubPort[len - 1].GpVar[0] = endp; // Save the synchronization flag bit
                    else
                        ThisUsbDev.GpVar[0] = endp;
                    len = R8_USB_RX_LEN; // The length of data received
                    if(len)
                    {
                        PRINT("Mouse data: ");
                        for(i = 0; i < len; i++)
                        {
                            PRINT("x%02X ", (uint16_t)(RxBuffer[i]));
                        }
                        PRINT("\n");
                    }
                }
                else if(s != (USB_PID_NAK | ERR_USB_TRANSFER))
                {
                    PRINT("Mouse error %02x\n", (uint16_t)s); // Maybe it's disconnected
                }
            }
            else
            {
                PRINT("Mouse no interrupt endpoint\n");
            }
            SetUsbSpeed(1); // Default is full speed
        }

        /* If the device is a keyboard */
        loc = SearchTypeDevice(DEV_TYPE_KEYBOARD); // Search the port number of the specified type of device on each port of ROOT-HUB and external HUB
        if(loc != 0xFFFF)
        { // Found, what if there are two KeyBoards?
            i = (uint8_t)(loc >> 8);
            len = (uint8_t)loc;
            SelectHubPort(len);                                                // Select the ROOT-HUB port specified in the operation, set the current USB speed and the USB address of the operating device
            endp = len ? DevOnHubPort[len - 1].GpVar[0] : ThisUsbDev.GpVar[0]; // The address of the interrupt endpoint, bit 7 is used to synchronize the flag bit
            if(endp & USB_ENDP_ADDR_MASK)
            {                                                                                                       // Endpoint valid
                s = USBHostTransact(USB_PID_IN << 4 | endp & 0x7F, endp & 0x80 ? RB_UH_R_TOG | RB_UH_T_TOG : 0, 0); // CH554 transmits transactions, obtains data, NAK does not try again
                if(s == ERR_SUCCESS)
                {
                    endp ^= 0x80; // Synchronous flag flip
                    if(len)
                        DevOnHubPort[len - 1].GpVar[0] = endp; // Save the synchronization flag bit
                    else
                        ThisUsbDev.GpVar[0] = endp;
                    len = R8_USB_RX_LEN; // The length of data received
                    if(len)
                    {
                        SETorOFFNumLock(RxBuffer);
                        PRINT("keyboard data: ");
                        for(i = 0; i < len; i++)
                        {
                            PRINT("x%02X ", (uint16_t)(RxBuffer[i]));
                        }
                        PRINT("\n");
                    }
                }
                else if(s != (USB_PID_NAK | ERR_USB_TRANSFER))
                {
                    PRINT("keyboard error %02x\n", (uint16_t)s); // Maybe it's disconnected
                }
            }
            else
            {
                PRINT("keyboard no interrupt endpoint\n");
            }
            SetUsbSpeed(1); // Default is full speed
        }
    }
}
