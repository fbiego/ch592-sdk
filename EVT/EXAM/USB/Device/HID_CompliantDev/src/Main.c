/* ********************************* (C) COPYRIGHT ***************************
* File Name: Main.c
* Author: WCH
* Version: V1.1
* Date: 2022/01/25
* Description: Simulate compatible HID devices
************************************************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
********************************************************************************************* */

#include "CH59x_common.h"

#define DevEP0SIZE    0x40
// Device descriptor
const uint8_t MyDevDescr[] = {0x12,0x01,0x10,0x01,0x00,0x00,0x00,DevEP0SIZE,0x3d,0x41,0x07,0x21,0x00,0x00,0x00,0x00,0x00,0x01};
// Configuration descriptor
const uint8_t MyCfgDescr[] = {
    0x09,0x02,0x29,0x00,0x01,0x01,0x04,0xA0,0x23,               // Configuration descriptor
    0x09,0x04,0x00,0x00,0x02,0x03,0x00,0x00,0x05,               // Interface descriptor
    0x09,0x21,0x00,0x01,0x00,0x01,0x22,0x22,0x00,               // HID class descriptor
    0x07,0x05,0x81,0x03,0x40,0x00,0x01,              // Endpoint descriptor
    0x07,0x05,0x01,0x03,0x40,0x00,0x01               // Endpoint descriptor
};
/* String descriptor skip */
/* HID report descriptor */
const uint8_t HIDDescr[] = {  0x06, 0x00,0xff,
                              0x09, 0x01,
                              0xa1, 0x01,                                                   // The collection begins
                              0x09, 0x02,                                                   // Usage Page Usage
                              0x15, 0x00,                                                   //Logical  Minimun
                              0x26, 0x00,0xff,                                              //Logical  Maximun
                              0x75, 0x08,                                                   //Report Size
                              0x95, 0x40,                                                   //Report Counet
                              0x81, 0x06,                                                   //Input
                              0x09, 0x02,                                                   // Usage Page Usage
                              0x15, 0x00,                                                   //Logical  Minimun
                              0x26, 0x00,0xff,                                              //Logical  Maximun
                              0x75, 0x08,                                                   //Report Size
                              0x95, 0x40,                                                   //Report Counet
                              0x91, 0x06,                                                   //Output
                              0xC0};

/**********************************************************/
uint8_t        DevConfig, Ready = 0;
uint8_t        SetupReqCode;
uint16_t       SetupReqLen;
const uint8_t *pDescr;
uint8_t        Report_Value = 0x00;
uint8_t        Idle_Value = 0x00;
uint8_t        USB_SleepStatus = 0x00; /* USB sleep state */

// HID device interrupts 4 bytes of data uploaded to the host during transmission
uint8_t HID_Buf[] = {0,0,0,0};

/* ********** User-defined allocation endpoint RAM ********************************* */
__attribute__((aligned(4))) uint8_t EP0_Databuf[64 + 64 + 64]; //ep0(64)+ep4_out(64)+ep4_in(64)
__attribute__((aligned(4))) uint8_t EP1_Databuf[64 + 64];      //ep1_out(64)+ep1_in(64)
__attribute__((aligned(4))) uint8_t EP2_Databuf[64 + 64];      //ep2_out(64)+ep2_in(64)
__attribute__((aligned(4))) uint8_t EP3_Databuf[64 + 64];      //ep3_out(64)+ep3_in(64)

/* ***************************************************************************
* @fn USB_DevTransProcess
*
* @brief USB transfer processing function
*
* @return none */
void USB_DevTransProcess(void)  // USB device transmission interrupt processing
{
    uint8_t len, chtype;        // len is used to copy functions, and chtype is used to store information such as data transmission direction, command type, and received objects.
    uint8_t intflag, errflag = 0;   // intflag is used to store flag register values, and errflag is used to mark whether the host is supported.

    intflag = R8_USB_INT_FG;        // Get the value of the interrupt identification register

    if(intflag & RB_UIF_TRANSFER)   // Determines the USB transmission completion interrupt flag in _INT_FG.If the transmission is interrupted, enter the if statement
    {
        if((R8_USB_INT_ST & MASK_UIS_TOKEN) != MASK_UIS_TOKEN) // Non-idle //Judge 5:4 bits in the interrupt status register and view the PID ID of the token.If these two digits are not 11 (meaning idle), enter the if statement
        {
            switch(R8_USB_INT_ST & (MASK_UIS_TOKEN | MASK_UIS_ENDP))    // Get the PID ID of the token and the endpoint number of the 3:0 bit in device mode.In host mode, 3:0 bit is the answer PID identification bit
            // Analyze operation tokens and endpoint numbers
            {                           // Endpoint 0 is used to control transmission.The following IN and OUT tokens of endpoint 0 correspond to the corresponding program, corresponding to the data stage and status stage of the control transmission.
                case UIS_TOKEN_IN:      // The PID of the token package is IN, and the 5:4 bit is 10. The endpoint number of the 3:0 bit is 0.IN token: The device sends data to the host._UIS_: USB interrupt status
                {                       // Endpoint 0 is a bidirectional endpoint, used as control transmission."|0" operation is omitted
                    switch(SetupReqCode)// This value will be assigned when the SETUP packet is received.There will be a SETUP packet handler later, corresponding to the setting stage of control transmission.
                    {
                        case USB_GET_DESCRIPTOR:    // USB standard command, the host obtains description from the USB device
                            len = SetupReqLen >= DevEP0SIZE ? DevEP0SIZE : SetupReqLen; // The packet transmission length is.The maximum length is 64 bytes, and more than 64 bytes are processed in multiple times, and the first few times will be full.
                            memcpy(pEP0_DataBuf, pDescr, len);// memcpy: Memory copy function, copy (number 2) string length from (number 2) address to (number 1) address to (number 1) address
                            // DMA is directly connected to memory and will detect memory rewriting, and then the data in memory can be sent out without the control of the microcontroller.If only two arrays are assigned to each other and do not involve physical memory matching DMA, DMA cannot be triggered.
                            SetupReqLen -= len;     // Record the remaining length of data to be sent
                            pDescr += len;          // Update the starting address of the data to be sent next, and use the copy function to
                            R8_UEP0_T_LEN = len;    // Endpoint 0 send length register writes the packet transmission length
                            R8_UEP0_CTRL ^= RB_UEP_T_TOG;   // Synchronous switch.Switch DATA0 and DATA1 in PID of IN direction (for microcontrollers, the T direction)
                            break;                  // The handshake packet response (ACK, NAK, STALL) of the endpoint control register is packaged into a package that complies with the specifications by the hardware, and DMA will automatically send it.
                        case USB_SET_ADDRESS:       // USB standard command, the host sets a unique address for the device, the range 0 to 127, and 0 is the default address
                            R8_USB_DEV_AD = (R8_USB_DEV_AD & RB_UDA_GP_BIT) | SetupReqLen;
                                    // 7-bit address + the user-defined address of the highest bit (default is 1), or the "packet transmission length" on the top (the "packet transmission length" here is assigned as the address bit later)
                            R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                                    // R responds to OUT transaction ACK, T responds to IN transaction NAK.This CASE branch is in the IN direction. When the DMA corresponding memory is in the microcontroller, it returns to the NAK handshake package.
                            break;                                                  // Generally, the device will return the OUT transaction in the program to the host and will not respond to NAK.

                        case USB_SET_FEATURE:       // USB standard command, the host requires a feature on the device, interface, or endpoint
                            break;

                        default:
                            R8_UEP0_T_LEN = 0;      // The status stage is interrupted or forced upload of 0-length packets to end control transmission (data packets with data field length 0, all SYNC, PID, and EOP fields are included in the packet)
                            R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                                    // R responds to OUT transaction ACK, T responds to IN transaction NAK.This CASE branch is OUT direction. When the data is updated in the corresponding DMA memory and the microcontroller acceptance is normal, return to the ACK handshake bag.
                            Ready = 1;
                            PRINT("Ready_STATUS = %d\n",Ready);
                            break;
                    }
                }
                break;

                case UIS_TOKEN_OUT:     // The PID of the token package is OUT, and the 5:4 bit is 00. The endpoint number of the 3:0 bit is 0.OUT token: The host sends data to the device.
                {                       // Endpoint 0 is a bidirectional endpoint, used as control transmission."|0" operation is omitted
                    len = R8_USB_RX_LEN;    // Read the number of received data bytes stored in the current USB receiving length register //The receiving length register is shared by each endpoint, and the sending length register has its own
                }
                break;

                case UIS_TOKEN_OUT | 1: // The PID of the token package is OUT and the endpoint number is 1
                {
                    if(R8_USB_INT_ST & RB_UIS_TOG_OK)   // The hardware will determine whether the synchronization switches packets are correct. If the synchronization switch is correct, this bit will automatically be set.
                    { // Out-of-sync packets will be discarded
                        R8_UEP1_CTRL ^= RB_UEP_R_TOG;   // DATA synchronization switching of OUT transactions.Set an expected value.
                        len = R8_USB_RX_LEN;        // Read the number of bytes received data
                        DevEP1_OUT_Deal(len);       // Send bytes of length len and automatically return to the ACK handshake packet.Customized program.
                    }
                }
                break;

                case UIS_TOKEN_IN | 1: // The PID of the token package is IN and the endpoint number is 1
                    R8_UEP1_CTRL ^= RB_UEP_T_TOG;       // Switch the DATA of the IN transaction.Sets the PID of the packet to be sent.
                    R8_UEP1_CTRL = (R8_UEP1_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK;    // When the data is not updated by the microcontroller in the DMA, the T response IN transaction is set to NAK.Publish data after update.
                    Ready = 1;
                    PRINT("Ready_IN_EP1 = %d\n",Ready);
                    break;
            }
            R8_USB_INT_FG = RB_UIF_TRANSFER;    // Write 1 Clear interrupt flag
        }

        if(R8_USB_INT_ST & RB_UIS_SETUP_ACT) // Setup package processing
        {
            R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_NAK;
                        // R responds to OUT transaction expectation to be DATA1 (the PID of the data packet received by DMA must be DATA1, otherwise the calculation data error will be retransmitted) and ACK (the data received in the corresponding memory of DMA, and the acceptance of the microcontroller is normal)
                        // The T response IN transaction is set to DATA1 (the microcontroller has data sent to the corresponding DMA memory and is sent out with DATA1) and NAK (the microcontroller has not prepared data).
            SetupReqLen = pSetupReqPak->wLength;    // Number of bytes in the data stage //pSetupReqPak: casts the RAM address of endpoint 0 into an address that stores the structure, and the structure members are arranged in order.
            SetupReqCode = pSetupReqPak->bRequest;  // The sequence number of the command
            chtype = pSetupReqPak->bRequestType;    // Contains information such as data transmission direction, command type, received object, etc.

            len = 0;
            errflag = 0;
            if((pSetupReqPak->bRequestType & USB_REQ_TYP_MASK) != USB_REQ_TYP_STANDARD) // Determine the type of the command, if it is not a standard request, enter the if statement
            {
                /* Non-standard request */
                /* Other requests, such as class requests, manufacturer requests, etc. */
                if(pSetupReqPak->bRequestType & 0x40)   // Get a certain one of the commands and determine whether it is 0, and enter the if statement without zero
                {
                    /* Manufacturer request */
                }
                else if(pSetupReqPak->bRequestType & 0x20)  // Get a certain one of the commands and determine whether it is 0, and enter the if statement without zero
                {   // Determined as HID class request
                    switch(SetupReqCode)    // Determine the sequence number of the command
                    {
                        case DEF_USB_SET_IDLE: /* 0x0A: SET_IDLE */         // The host wants to set the idle time interval for the specific input report of the HID device
                            Idle_Value = EP0_Databuf[3];
                            break; // This must have

                        case DEF_USB_SET_REPORT: /* 0x09: SET_REPORT */     // The host wants to set the report descriptor for the HID device
                            break;

                        case DEF_USB_SET_PROTOCOL: /* 0x0B: SET_PROTOCOL */ // The host wants to set the protocol currently used by the HID device
                            Report_Value = EP0_Databuf[2];
                            break;

                        case DEF_USB_GET_IDLE: /* 0x02: GET_IDLE */         // The host wants to read the current idle ratio of the HID device-specific input report
                            EP0_Databuf[0] = Idle_Value;
                            len = 1;
                            break;

                        case DEF_USB_GET_PROTOCOL: /* 0x03: GET_PROTOCOL */     // The host wants to obtain the protocol currently used by the HID device
                            EP0_Databuf[0] = Report_Value;
                            len = 1;
                            break;

                        default:
                            errflag = 0xFF;
                    }
                }
            }
            else    // Determined as a standard request
            {
                switch(SetupReqCode)    // Determine the sequence number of the command
                {
                    case USB_GET_DESCRIPTOR:    // The host wants to obtain the standard descriptor
                    {
                        switch(((pSetupReqPak->wValue) >> 8))   // Move the right 8 bits to see if the original high 8 bits are 0. If it is 1, it means that the direction is IN. Then enter the s-case statement
                        {
                            case USB_DESCR_TYP_DEVICE:  // Different values ​​represent different commands.The host wants to obtain the device descriptor
                            {
                                pDescr = MyDevDescr;    // Put the device descriptor string in the pDescr address, and the end of the case "Get standard descriptor" will be sent with a copy function.
                                len = MyDevDescr[0];    // The protocol specifies the length of the first byte of the device descriptor.The copy function will use the len parameter
                            }
                            break;

                            case USB_DESCR_TYP_CONFIG:  // The host wants to obtain the configuration descriptor
                            {
                                pDescr = MyCfgDescr;    // Place the configuration descriptor string in the pDescr address and will be sent later
                                len = MyCfgDescr[2];    // The protocol specifies the total length of the configuration information stored in the third byte of the configuration descriptor.
                            }
                            break;

                            case USB_DESCR_TYP_HID:     // The host wants to obtain the human-computer interface class descriptor.The wIndex in the structure here is different from the configuration descriptor, meaning the interface number.
                                switch((pSetupReqPak->wIndex) & 0xff)       // Take the lower eight digits and wipe off the higher eight digits
                                {
                                    /* Select an interface */
                                    case 0:
                                        pDescr = (uint8_t *)(&MyCfgDescr[18]);  // The class descriptor storage location of interface 1, to be sent
                                        len = 9;
                                        break;

                                    default:
                                        /* Unsupported string descriptors */
                                        errflag = 0xff;
                                        break;
                                }
                                break;

                            case USB_DESCR_TYP_REPORT:  // The host wants to obtain the device report descriptor
                            {
                                if(((pSetupReqPak->wIndex) & 0xff) == 0) // Interface 0 report descriptor
                                {
                                    pDescr = HIDDescr; // Data ready to be uploaded
                                    len = sizeof(HIDDescr);
                                }
                                else
                                    len = 0xff; // This program has only 2 interfaces, so this sentence is not possible to be executed normally
                            }
                            break;

                            case USB_DESCR_TYP_STRING:  // The host wants to obtain the device string descriptor
                            {
                                switch((pSetupReqPak->wValue) & 0xff)   // Pass string information according to the value of wValue
                                {
                                    default:
                                        errflag = 0xFF; // Unsupported string descriptors
                                        break;
                                }
                            }
                            break;

                            default:
                                errflag = 0xff;
                                break;
                        }
                        if(SetupReqLen > len)
                            SetupReqLen = len;      // The total length needs to be uploaded
                        len = (SetupReqLen >= DevEP0SIZE) ? DevEP0SIZE : SetupReqLen;   // Maximum length is 64 bytes
                        memcpy(pEP0_DataBuf, pDescr, len);  // Copy functions
                        pDescr += len;
                    }
                    break;

                    case USB_SET_ADDRESS:       // The host wants to set the device address
                        SetupReqLen = (pSetupReqPak->wValue) & 0xff;    // The bit device address distributed by the host is temporarily stored in SetupReqLen
                        break;                                          // The control phase will be assigned to the device address parameters

                    case USB_GET_CONFIGURATION: // The host wants to obtain the current configuration of the device
                        pEP0_DataBuf[0] = DevConfig;    // Put device configuration into RAM
                        if(SetupReqLen > 1)
                            SetupReqLen = 1;    // Set the number of bytes in the data stage by 1.Because DevConfig has only one byte
                        break;

                    case USB_SET_CONFIGURATION: // The host wants to set the current configuration of the device
                        DevConfig = (pSetupReqPak->wValue) & 0xff;  // Take the lower eight digits and wipe off the higher eight digits
                        break;

                    case USB_CLEAR_FEATURE:     // Turn off the features/functions of the USB device.It can be at the device or endpoint level.
                    {
                        if((pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP) // Determine whether it is an endpoint feature (clear the state where the endpoint stops working)
                        {
                            switch((pSetupReqPak->wIndex) & 0xff)   // Take the lower eight digits and erase the higher eight digits.Judge index
                            {       // The highest bit of 16 bits determines the data transmission direction, 0 is OUT and 1 is IN.The low position is the endpoint number.
                                case 0x81:      // Clear the three bits of _TOG and _T_RES, and write the latter as _NAK, and respond to IN transaction NAK that means no data is returned.
                                    R8_UEP1_CTRL = (R8_UEP1_CTRL & ~(RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_NAK;
                                    break;
                                case 0x01:      // Clear the three bits of _TOG and _R_RES, and write the latter as _ACK, and respond to OUT transaction ACK that indicates normal reception.
                                    R8_UEP1_CTRL = (R8_UEP1_CTRL & ~(RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_ACK;
                                    break;
                                default:
                                    errflag = 0xFF; // Unsupported endpoints
                                    break;
                            }
                        }
                        else if((pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_DEVICE)  // Determine whether it is a device feature (used to wake up the device)
                        {
                            if(pSetupReqPak->wValue == 1)   // The wake-up flag is 1
                            {
                                USB_SleepStatus &= ~0x01;   // Clear the lowest position
                            }
                        }
                        else
                        {
                            errflag = 0xFF;
                        }
                    }
                    break;

                    case USB_SET_FEATURE:       // Turn on the features/functions of the USB device.It can be at the device or endpoint level.
                        if((pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP) // Determine whether it is an endpoint feature (make the endpoint stop working)
                        {
                            /* Endpoint */
                            switch(pSetupReqPak->wIndex)    // Judge index
                            {       // The highest bit of 16 bits determines the data transmission direction, 0 is OUT and 1 is IN.The low position is the endpoint number.
                                case 0x81:      // Clear _TOG and _T_RES, and write the latter as _STALL to stop the endpoint's work according to the host instruction.
                                    R8_UEP1_CTRL = (R8_UEP1_CTRL & ~(RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_STALL;
                                    break;
                                case 0x01:      // Clear _TOG and _R_RES, and write the latter as _STALL to stop the endpoint's work according to the host instruction.
                                    R8_UEP1_CTRL = (R8_UEP1_CTRL & ~(RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_STALL;
                                    break;
                                default:
                                    /* Unsupported endpoints */
                                    errflag = 0xFF; // Unsupported endpoints
                                    break;
                            }
                        }
                        else if((pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_DEVICE)  // Determine whether it is a device feature (make the device sleepy)
                        {
                            if(pSetupReqPak->wValue == 1)
                            {
                                USB_SleepStatus |= 0x01;    // Setting up sleep
                            }
                        }
                        else
                        {
                            errflag = 0xFF;
                        }
                        break;

                    case USB_GET_INTERFACE:     // The host wants to obtain the selection setting value for the interface currently working
                        pEP0_DataBuf[0] = 0x00;
                        if(SetupReqLen > 1)
                            SetupReqLen = 1;    // Set the number of bytes in the data stage by 1.Because there is only one byte of data to be transmitted
                        break;

                    case USB_SET_INTERFACE:     // The host wants to activate an interface of the device
                        break;

                    case USB_GET_STATUS:        // The host wants to obtain the status of the device, interface, or endpoint
                        if((pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP) // Determine whether it is endpoint status
                        {
                            /* Endpoint */
                            pEP0_DataBuf[0] = 0x00;
                            switch(pSetupReqPak->wIndex)
                            {       // The highest bit of 16 bits determines the data transmission direction, 0 is OUT and 1 is IN.The low position is the endpoint number.
                                case 0x81:      // Judgment _TOG and _T_RES, if in STALL state, enter if statement
                                    if((R8_UEP1_CTRL & (RB_UEP_T_TOG | MASK_UEP_T_RES)) == UEP_T_RES_STALL)
                                    {
                                        pEP0_DataBuf[0] = 0x01; // Returning D0 is 1, indicating that the endpoint has stopped working.This bit is configured by the SET_FEATURE and CLEAR_FEATURE commands.
                                    }
                                    break;

                                case 0x01:      // Judge the three digits _TOG and _R_RES, if in STALL state, enter the if statement
                                    if((R8_UEP1_CTRL & (RB_UEP_R_TOG | MASK_UEP_R_RES)) == UEP_R_RES_STALL)
                                    {
                                        pEP0_DataBuf[0] = 0x01;
                                    }
                                    break;
                            }
                        }
                        else if((pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_DEVICE)  // Determine whether it is the device status
                        {
                            pEP0_DataBuf[0] = 0x00;
                            if(USB_SleepStatus)     // If the device is sleeping
                            {
                                pEP0_DataBuf[0] = 0x02;     // The lowest bit D0 is 0, which means the device is powered by the bus, and 1 means the device is powered by the device.A D1 bit of 1 means that remote wake-up is supported, and a 0 means that it is not supported.
                            }
                            else
                            {
                                pEP0_DataBuf[0] = 0x00;
                            }
                        }
                        pEP0_DataBuf[1] = 0;    // The format of the return status information is 16 digits, and the high eight digits are reserved as 0
                        if(SetupReqLen >= 2)
                        {
                            SetupReqLen = 2;    // Set the number of bytes in the data stage by 2.Because there are only 2 bytes of data to be transferred
                        }
                        break;

                    default:
                        errflag = 0xff;
                        break;
                }
            }
            if(errflag == 0xff) // Error or not supported
            {
                //                  SetupReqCode = 0xFF;
                R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL; // STALL
                Ready = 1;
                PRINT("Ready_Stall = %d\n",Ready);
            }
            else
            {
                if(chtype & 0x80)   // Upload.The highest bit is 1, and the data transmission direction is the transmission of the device to the host.
                {
                    len = (SetupReqLen > DevEP0SIZE) ? DevEP0SIZE : SetupReqLen;
                    SetupReqLen -= len;
                }
                else
                    len = 0;        // Download.The highest bit is 0, and the data transmission direction is the transmission of the host to the device.
                R8_UEP0_T_LEN = len;
                R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;     // The default packet is DATA1
            }

            R8_USB_INT_FG = RB_UIF_TRANSFER;    // Write 1 clear interrupt flag
        }
    }


    else if(intflag & RB_UIF_BUS_RST)   // Determine the bus reset flag bit in _INT_FG, triggered by 1.
    {
        R8_USB_DEV_AD = 0;      // The device address is written as 0, and the host will reassign a new address to the device.
        R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;   // Write the control register of endpoint 0 as: the reception response ACK means normal reception, and the transmission response NAK means no data to be returned.
        R8_UEP1_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        R8_UEP2_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        R8_UEP3_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        R8_USB_INT_FG = RB_UIF_BUS_RST; // Write 1 clear interrupt flag
    }
    else if(intflag & RB_UIF_SUSPEND)   // Determines the bus suspend or wake-up event interrupt flag in _INT_FG.Both hang and wake up trigger this interrupt
    {
        if(R8_USB_MIS_ST & RB_UMS_SUSPEND)  // Get the suspend status bit in the miscellaneous status register. It is 1 that indicates that the USB bus is in a suspended state, and it is 0 that indicates that the bus is in a non-hanged state.
        {
            Ready = 0;
            PRINT("Ready_Sleep = %d\n",Ready);
        } // Hang // When the device is idle for more than 3ms, the host requires the device to hang (similar to computer hibernation)
        else    // The suspend or wake-up interrupt is triggered and is not judged to be suspended
        {
            Ready = 1;
            PRINT("Ready_WeakUp = %d\n",Ready);
        } // wake
        R8_USB_INT_FG = RB_UIF_SUSPEND; // Write 1 clear interrupt sign
    }
    else
    {
        R8_USB_INT_FG = intflag;    // There is no interrupt flag in _INT_FG, and then write the original value back to the original register
    }
}

/* ***************************************************************************
* @fn DevHIDReport
*
* @brief Report HID data
*
* @return 0: Success
* 1: An error occurred */
void DevHIDReport(uint8_t data0,uint8_t data1,uint8_t data2,uint8_t data3)
{
    HID_Buf[0] = data0;
    HID_Buf[1] = data1;
    HID_Buf[2] = data2;
    HID_Buf[3] = data3;
    memcpy(pEP1_IN_DataBuf, HID_Buf, sizeof(HID_Buf));
    DevEP1_IN_Deal(sizeof(HID_Buf));
}

/* ***************************************************************************
* @fn DevWakeup
*
* @brief wake up the host in device mode
*
* @return none */
void DevWakeup(void)
{
    R16_PIN_ANALOG_IE &= ~(RB_PIN_USB_DP_PU);
    R8_UDEV_CTRL |= RB_UD_LOW_SPEED;
    mDelaymS(2);
    R8_UDEV_CTRL &= ~RB_UD_LOW_SPEED;
    R16_PIN_ANALOG_IE |= RB_PIN_USB_DP_PU;
}

/* ***************************************************************************
* @fn DebugInit
*
* @brief debug initialization
*
* @return none */
void DebugInit(void)
{
    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
}

/* ***************************************************************************
* @fn main
*
* @brief main function
*
* @return none */
int main()
{
    uint8_t s;
    SetSysClock(CLK_SOURCE_PLL_60MHz);

    DebugInit();        // Configure serial port 1 to print to debug
    printf("start\n");

    pEP0_RAM_Addr = EP0_Databuf;    // Configure the cache area 64 bytes.
    pEP1_RAM_Addr = EP1_Databuf;

    USB_DeviceInit();

    PFIC_EnableIRQ(USB_IRQn);       // Enable interrupt vector
    mDelaymS(100);

    while(1)
    {// Simulate the transmission of 4 bytes of data, and the actual transmission is modified by itself according to user needs.
        if(Ready)
        {
            Ready = 0;
            DevHIDReport(0x05, 0x10, 0x20, 0x11);
        }
        mDelaymS(100);

        if(Ready)
        {
            Ready = 0;
            DevHIDReport(0x0A, 0x15, 0x25, 0x22);
        }
        mDelaymS(100);

        if(Ready)
        {
            Ready = 0;
            DevHIDReport(0x0E, 0x1A, 0x2A, 0x44);
        }
        mDelaymS(100);

        if(Ready)
        {
            Ready = 0;
            DevHIDReport(0x10, 0x1E, 0x2E, 0x88);
        }
        mDelaymS(100);
    }
}

/* ***************************************************************************
* @fn DevEP1_OUT_Deal
*
* @brief Endpoint 1 data processing, and then invert the data after receiving it.User changes it by themselves.
*
* @return none */
void DevEP1_OUT_Deal(uint8_t l)
{ /* User-customizable */
    uint8_t i;

    for(i = 0; i < l; i++)
    {
        pEP1_IN_DataBuf[i] = ~pEP1_OUT_DataBuf[i];
    }
    DevEP1_IN_Deal(l);
}


/* ***************************************************************************
* @fn USB_IRQHandler
*
* @brief USB interrupt function
*
* @return none */
__attribute__((interrupt("WCH-Interrupt-fast")))
__attribute__((section(".highcode")))
void USB_IRQHandler(void) /* USB interrupt service program, use register group 1 */
{
    USB_DevTransProcess();
}
