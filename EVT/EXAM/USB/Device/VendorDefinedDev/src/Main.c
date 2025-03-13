/* ********************************* (C) COPYRIGHT ***************************
* File Name: Main.c
* Author: WCH
* Version: V1.0
* Date: 2020/08/06
* Description: Customize USB device (CH372 device), provides 8 non-0 channels (upload + download), realizes data upload first, and then data content is inverted
************************************************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
********************************************************************************************* */

#include "CH59x_common.h"

#define DevEP0SIZE    0x40
// Device descriptor
const uint8_t MyDevDescr[] = { 0x12, 0x01, 0x10, 0x01, 0xFF, 0x80, 0x55, DevEP0SIZE, 0x48, 0x43, 0x37, 0x55,    // Manufacturer ID and product ID
                             0x00, 0x01, 0x01, 0x02, 0x00, 0x01, };
// Configuration descriptor
const uint8_t MyCfgDescr[] = { 0x09, 0x02, 0x4A, 0x00, 0x01, 0x01, 0x00, 0x80, 0x32, 0x09, 0x04, 0x00, 0x00, 0x08, 0xFF,
                             0x80, 0x55, 0x00, 0x07, 0x05, 0x84, 0x02, 0x40, 0x00, 0x00, 0x07, 0x05, 0x04, 0x02, 0x40,
                             0x00, 0x00, 0x07, 0x05, 0x83, 0x02, 0x40, 0x00, 0x00, 0x07, 0x05, 0x03, 0x02, 0x40, 0x00,
                             0x00, 0x07, 0x05, 0x82, 0x02, 0x40, 0x00, 0x00, 0x07, 0x05, 0x02, 0x02, 0x40, 0x00, 0x00,
                             0x07, 0x05, 0x81, 0x02, 0x40, 0x00, 0x00, 0x07, 0x05, 0x01, 0x02, 0x40, 0x00, 0x00, };
// Language descriptor
const uint8_t MyLangDescr[] = {0x04, 0x03, 0x09, 0x04};
// Manufacturer information
const uint8_t MyManuInfo[] = {0x0E, 0x03, 'w', 0, 'c', 0, 'h', 0, '.', 0, 'c', 0, 'n', 0};
// Product Information
const uint8_t MyProdInfo[] = {0x0C, 0x03, 'C', 0, 'H', 0, '5', 0, '9', 0, 'x', 0};

/**********************************************************/
uint8_t        DevConfig;
uint8_t        SetupReqCode;
uint16_t       SetupReqLen;
const uint8_t *pDescr;

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
void USB_DevTransProcess(void)
{
    uint8_t len, chtype;
    uint8_t intflag, errflag = 0;

    intflag = R8_USB_INT_FG;
    if(intflag & RB_UIF_TRANSFER)
    {
        if((R8_USB_INT_ST & MASK_UIS_TOKEN) != MASK_UIS_TOKEN) // Not idle
        {
            switch(R8_USB_INT_ST & (MASK_UIS_TOKEN | MASK_UIS_ENDP))
            // Analyze operation tokens and endpoint numbers
            {
                case UIS_TOKEN_IN:
                {
                    switch(SetupReqCode)
                    {
                        case USB_GET_DESCRIPTOR:
                            len = SetupReqLen >= DevEP0SIZE ? DevEP0SIZE : SetupReqLen; // The transmission length
                            memcpy(pEP0_DataBuf, pDescr, len);                          /* Load upload data */
                            SetupReqLen -= len;
                            pDescr += len;
                            R8_UEP0_T_LEN = len;
                            R8_UEP0_CTRL ^= RB_UEP_T_TOG; // Flip
                            break;
                        case USB_SET_ADDRESS:
                            R8_USB_DEV_AD = (R8_USB_DEV_AD & RB_UDA_GP_BIT) | SetupReqLen;
                            R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                            break;
                        default:
                            R8_UEP0_T_LEN = 0; // The status phase is interrupted or the forced upload of 0-length packets ends to control transmission
                            R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                            break;
                    }
                }
                break;

                case UIS_TOKEN_OUT:
                    len = R8_USB_RX_LEN;
                    break;

                case UIS_TOKEN_OUT | 1:
                {
                    if(R8_USB_INT_ST & RB_UIS_TOG_OK)
                    { // Out-of-sync packets will be discarded
                        R8_UEP1_CTRL ^= RB_UEP_R_TOG;
                        len = R8_USB_RX_LEN;
                        DevEP1_OUT_Deal(len);
                    }
                }
                break;

                case UIS_TOKEN_IN | 1:
                    R8_UEP1_CTRL ^= RB_UEP_T_TOG;
                    R8_UEP1_CTRL = (R8_UEP1_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK;
                    break;

                case UIS_TOKEN_OUT | 2:
                {
                    if(R8_USB_INT_ST & RB_UIS_TOG_OK)
                    { // Out-of-sync packets will be discarded
                        R8_UEP2_CTRL ^= RB_UEP_R_TOG;
                        len = R8_USB_RX_LEN;
                        DevEP2_OUT_Deal(len);
                    }
                }
                break;

                case UIS_TOKEN_IN | 2:
                    R8_UEP2_CTRL ^= RB_UEP_T_TOG;
                    R8_UEP2_CTRL = (R8_UEP2_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK;
                    break;

                case UIS_TOKEN_OUT | 3:
                {
                    if(R8_USB_INT_ST & RB_UIS_TOG_OK)
                    { // Out-of-sync packets will be discarded
                        R8_UEP3_CTRL ^= RB_UEP_R_TOG;
                        len = R8_USB_RX_LEN;
                        DevEP3_OUT_Deal(len);
                    }
                }
                break;

                case UIS_TOKEN_IN | 3:
                    R8_UEP3_CTRL ^= RB_UEP_T_TOG;
                    R8_UEP3_CTRL = (R8_UEP3_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK;
                    break;

                case UIS_TOKEN_OUT | 4:
                {
                    if(R8_USB_INT_ST & RB_UIS_TOG_OK)
                    {
                        R8_UEP4_CTRL ^= RB_UEP_R_TOG;
                        len = R8_USB_RX_LEN;
                        DevEP4_OUT_Deal(len);
                    }
                }
                break;

                case UIS_TOKEN_IN | 4:
                    R8_UEP4_CTRL ^= RB_UEP_T_TOG;
                    R8_UEP4_CTRL = (R8_UEP4_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK;
                    break;

                default:
                    break;
            }
            R8_USB_INT_FG = RB_UIF_TRANSFER;
        }

        if(R8_USB_INT_ST & RB_UIS_SETUP_ACT) // Setup package processing
        {
            R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_NAK;
            SetupReqLen = pSetupReqPak->wLength;
            SetupReqCode = pSetupReqPak->bRequest;
            chtype = pSetupReqPak->bRequestType;

            len = 0;
            errflag = 0;
            if((pSetupReqPak->bRequestType & USB_REQ_TYP_MASK) != USB_REQ_TYP_STANDARD)
            {
                errflag = 0xFF; /* Non-standard request */
            }
            else /* Standard request */
            {
                switch(SetupReqCode)
                {
                    case USB_GET_DESCRIPTOR:
                    {
                        switch(((pSetupReqPak->wValue) >> 8))
                        {
                            case USB_DESCR_TYP_DEVICE:
                            {
                                pDescr = MyDevDescr;
                                len = MyDevDescr[0];
                            }
                            break;

                            case USB_DESCR_TYP_CONFIG:
                            {
                                pDescr = MyCfgDescr;
                                len = MyCfgDescr[2];
                            }
                            break;

                            case USB_DESCR_TYP_STRING:
                            {
                                switch((pSetupReqPak->wValue) & 0xff)
                                {
                                    case 1:
                                        pDescr = MyManuInfo;
                                        len = MyManuInfo[0];
                                        break;
                                    case 2:
                                        pDescr = MyProdInfo;
                                        len = MyProdInfo[0];
                                        break;
                                    case 0:
                                        pDescr = MyLangDescr;
                                        len = MyLangDescr[0];
                                        break;
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
                            SetupReqLen = len; // The total length needs to be uploaded
                        len = (SetupReqLen >= DevEP0SIZE) ? DevEP0SIZE : SetupReqLen;
                        memcpy(pEP0_DataBuf, pDescr, len);
                        pDescr += len;
                    }
                    break;

                    case USB_SET_ADDRESS:
                        SetupReqLen = (pSetupReqPak->wValue) & 0xff;
                        break;

                    case USB_GET_CONFIGURATION:
                        pEP0_DataBuf[0] = DevConfig;
                        if(SetupReqLen > 1)
                            SetupReqLen = 1;
                        break;

                    case USB_SET_CONFIGURATION:
                        DevConfig = (pSetupReqPak->wValue) & 0xff;
                        break;

                    case USB_CLEAR_FEATURE:
                    {
                        if((pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP) // Endpoint
                        {
                            switch((pSetupReqPak->wIndex) & 0xff)
                            {
                                case 0x82:
                                    R8_UEP2_CTRL = (R8_UEP2_CTRL & ~(RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_NAK;
                                    break;
                                case 0x02:
                                    R8_UEP2_CTRL = (R8_UEP2_CTRL & ~(RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_ACK;
                                    break;
                                case 0x81:
                                    R8_UEP1_CTRL = (R8_UEP1_CTRL & ~(RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_NAK;
                                    break;
                                case 0x01:
                                    R8_UEP1_CTRL = (R8_UEP1_CTRL & ~(RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_ACK;
                                    break;
                                default:
                                    errflag = 0xFF; // Unsupported endpoints
                                    break;
                            }
                        }
                        else
                            errflag = 0xFF;
                    }
                    break;

                    case USB_GET_INTERFACE:
                        pEP0_DataBuf[0] = 0x00;
                        if(SetupReqLen > 1)
                            SetupReqLen = 1;
                        break;

                    case USB_GET_STATUS:
                        pEP0_DataBuf[0] = 0x00;
                        pEP0_DataBuf[1] = 0x00;
                        if(SetupReqLen > 2)
                            SetupReqLen = 2;
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
            }
            else
            {
                if(chtype & 0x80) // Upload
                {
                    len = (SetupReqLen > DevEP0SIZE) ? DevEP0SIZE : SetupReqLen;
                    SetupReqLen -= len;
                }
                else
                    len = 0; // Download
                R8_UEP0_T_LEN = len;
                R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK; // The default packet is DATA1
            }

            R8_USB_INT_FG = RB_UIF_TRANSFER;
        }
    }
    else if(intflag & RB_UIF_BUS_RST)
    {
        R8_USB_DEV_AD = 0;
        R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        R8_UEP1_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        R8_UEP2_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        R8_UEP3_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        R8_USB_INT_FG = RB_UIF_BUS_RST;
    }
    else if(intflag & RB_UIF_SUSPEND)
    {
        if(R8_USB_MIS_ST & RB_UMS_SUSPEND)
        {
            ;
        } // Hang up
        else
        {
            ;
        } // wake
        R8_USB_INT_FG = RB_UIF_SUSPEND;
    }
    else
    {
        R8_USB_INT_FG = intflag;
    }
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
    SetSysClock(CLK_SOURCE_PLL_60MHz);

    DebugInit();
    PRINT("start\n");

    pEP0_RAM_Addr = EP0_Databuf;
    pEP1_RAM_Addr = EP1_Databuf;
    pEP2_RAM_Addr = EP2_Databuf;
    pEP3_RAM_Addr = EP3_Databuf;
    USB_DeviceInit();
    PFIC_EnableIRQ(USB_IRQn);

    while(1);
}

/* ***************************************************************************
* @fn DevEP1_OUT_Deal
*
* @brief Endpoint 1 data processing
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
* @fn DevEP2_OUT_Deal
*
* @brief Endpoint 2 data processing
*
* @return none */
void DevEP2_OUT_Deal(uint8_t l)
{ /* User-customizable */
    uint8_t i;

    for(i = 0; i < l; i++)
    {
        pEP2_IN_DataBuf[i] = ~pEP2_OUT_DataBuf[i];
    }
    DevEP2_IN_Deal(l);
}

/* ***************************************************************************
* @fn DevEP3_OUT_Deal
*
* @brief Endpoint 3 data processing
*
* @return none */
void DevEP3_OUT_Deal(uint8_t l)
{ /* User-customizable */
    uint8_t i;

    for(i = 0; i < l; i++)
    {
        pEP3_IN_DataBuf[i] = ~pEP3_OUT_DataBuf[i];
    }
    DevEP3_IN_Deal(l);
}

/* ***************************************************************************
* @fn DevEP4_OUT_Deal
*
* @brief Endpoint 4 data processing
*
* @return none */
void DevEP4_OUT_Deal(uint8_t l)
{ /* User-customizable */
    uint8_t i;

    for(i = 0; i < l; i++)
    {
        pEP4_IN_DataBuf[i] = ~pEP4_OUT_DataBuf[i];
    }
    DevEP4_IN_Deal(l);
}

/* ***************************************************************************
* @fn USB_IRQHandler
*
* @brief USB interrupt function
*
* @return none */
__INTERRUPT
__HIGH_CODE
void USB_IRQHandler(void) /* USB interrupt service program, use register group 1 */
{
    USB_DevTransProcess();
}
