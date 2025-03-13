/* ********************************* (C) COPYRIGHT *******************************
* File Name          : iap.c
* Author             : WCH
* Version            : V1.0
* Date               : 2022/03/15
* Description        : USB IAPÀý³Ì
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
****************************************************************************** */
#include "iap.h"

#undef pSetupReqPak     /* Resolve conflicts with peripheral library header files */
#define pSetupReqPak          ((PUSB_SETUP_REQ)EP0_Databuf)

void myDevEP2_IN_Deal(uint8_t s);
void myDevEP2_OUT_Deal(uint8_t l);

#define DevEP0SIZE  64
// Device descriptor
const uint8_t MyDevDescr[] =
{
    0x12, 0x01, 0x10, 0x01, 0xFF, 0x80, 0x55,
    DevEP0SIZE, 0x48, 0x43, 0xe0, 0x55,      // Manufacturer ID and product ID
    0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
};
// Configuration descriptor
const uint8_t MyCfgDescr[] =
{
    0x09, 0x02, 0x20, 0x00, 0x01, 0x01, 0x00, 0x80, 0x32, 0x09, 0x04, 0x00, 0x00,
    0x02, 0xFF, 0x80, 0x55, 0x00, 0x07, 0x05, 0x82, 0x02, 0x40, 0x00, 0x00,
    0x07, 0x05, 0x02, 0x02, 0x40, 0x00, 0x00
};
// Language descriptor
const uint8_t MyLangDescr[] =
{ 0x04, 0x03, 0x09, 0x04 };
// Manufacturer information
const uint8_t MyManuInfo[] =
{ 0x0E, 0x03, 'w', 0, 'c', 0, 'h', 0, '.', 0, 'c', 0, 'n', 0 };
// Product Information
const uint8_t MyProdInfo[] =
{ 0x0C, 0x03, 'C', 0, 'H', 0, '5', 0, '9', 0, 'x', 0 };

/**********************************************************/
uint8_t DevConfig;
uint8_t SetupReqCode;
uint16_t SetupReqLen;
const uint8_t *pDescr;

/* ********** User-defined allocation endpoint RAM ********************************* */
__attribute__((aligned(4)))   uint8_t EP0_Databuf[64 + 64 + 64]; //ep0(64)+ep4_out(64)+ep4_in(64)
__attribute__((aligned(4)))   uint8_t EP1_Databuf[64 + 64]; //ep1_out(64)+ep1_in(64)
__attribute__((aligned(4)))   uint8_t EP2_Databuf[64 + 64]; //ep2_out(64)+ep2_in(64)
__attribute__((aligned(4)))   uint8_t EP3_Databuf[64 + 64]; //ep3_out(64)+ep3_in(64)

__attribute__((aligned(4)))   uint8_t g_write_buf[256 + 64]; // Write flash every time you spend 256 bytes to increase the speed
volatile uint16_t g_buf_write_ptr = 0;
volatile uint32_t g_flash_write_ptr = 0;
uint32_t g_tcnt;
__attribute__((aligned(4))) iap_cmd_t g_iap_cmd;

/* ***************************************************************************
* @fn USB_DevTransProcess
*
* @brief IAP USB main loop, the program is put into ram to improve the speed.
*
* @param None.
*
* @return None. */
__attribute__((section(".highcode")))
void USB_DevTransProcess(void)
{
    uint8_t len, chtype;
    uint8_t intflag, errflag = 0;

    intflag = R8_USB_INT_FG;
    if (intflag & RB_UIF_TRANSFER)
    {
        g_tcnt = 0; // USB has data, clear the timeout count
        if (intflag & RB_U_IS_NAK)
        {
        }
        else
        {
        	// Analyze operation tokens and endpoint numbers
            switch (R8_USB_INT_ST & (MASK_UIS_TOKEN | MASK_UIS_ENDP))
            {
            case UIS_TOKEN_IN | 2:
                R8_UEP2_CTRL = (R8_UEP2_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK;
                break;
            case UIS_TOKEN_OUT | 2:
            {
                if (R8_USB_INT_ST & RB_UIS_TOG_OK)
                {
                    // Out-of-sync packets will be discarded
                    len = R8_USB_RX_LEN;
                    my_memcpy(g_iap_cmd.other.buf, EP2_Databuf, len);
                    myDevEP2_OUT_Deal(len);
                }
            }
            break;


            case UIS_TOKEN_IN | 1:
                R8_UEP1_CTRL = (R8_UEP1_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK;
                break;

            case UIS_TOKEN_IN:
            {
                switch (SetupReqCode)
                {
                case USB_GET_DESCRIPTOR:
                    len = SetupReqLen >= DevEP0SIZE ? DevEP0SIZE : SetupReqLen;   // The transmission length
                    my_memcpy(EP0_Databuf, pDescr, len);  /* Load upload data */
                    SetupReqLen -= len;
                    pDescr += len;
                    R8_UEP0_T_LEN = len;
                    R8_UEP0_CTRL ^= RB_UEP_T_TOG;                          // Flip
                    break;
                case USB_SET_ADDRESS:
                    R8_USB_DEV_AD = (R8_USB_DEV_AD & RB_UDA_GP_BIT) | SetupReqLen;
                    R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                    break;
                default:
                    R8_UEP0_T_LEN = 0;            // The status phase is interrupted or the forced upload of 0-length packets ends to control transmission
                    R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                    break;
                }
            }
            break;

            case UIS_TOKEN_OUT:
                //len = R8_USB_RX_LEN;
                R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                break;
            default:
                break;
            }
            R8_USB_INT_FG = RB_UIF_TRANSFER;

        }
        if (R8_USB_INT_ST & RB_UIS_SETUP_ACT)                   // Setup package processing
        {
            R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_NAK;
            SetupReqLen = pSetupReqPak->wLength;
            SetupReqCode = pSetupReqPak->bRequest;
            chtype = pSetupReqPak->bRequestType;

            len = 0;
            errflag = 0;
            if ((pSetupReqPak->bRequestType & USB_REQ_TYP_MASK) != USB_REQ_TYP_STANDARD)
            {
                errflag = 0xFF; /* Non-standard request */
            }
            else /* Standard request */
            {
                switch (SetupReqCode)
                {
                case USB_GET_DESCRIPTOR:
                {
                    switch (((pSetupReqPak->wValue) >> 8))
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
                        switch ((pSetupReqPak->wValue) & 0xff)
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
                            errflag = 0xFF;                        // Unsupported string descriptors
                            break;
                        }
                    }
                    break;

                    default:
                        errflag = 0xff;
                        break;
                    }
                    if (SetupReqLen > len)
                    {
                        SetupReqLen = len;      // The total length needs to be uploaded
                    }
                    len = (SetupReqLen >= DevEP0SIZE) ? DevEP0SIZE : SetupReqLen;
                    my_memcpy(EP0_Databuf, pDescr, len);
                    pDescr += len;
                }
                break;

                case USB_SET_ADDRESS:
                    SetupReqLen = (pSetupReqPak->wValue) & 0xff;
                    break;

                case USB_GET_CONFIGURATION:
                    EP0_Databuf[0] = DevConfig;
                    if (SetupReqLen > 1)
                    {
                        SetupReqLen = 1;
                    }
                    break;

                case USB_SET_CONFIGURATION:
                    DevConfig = (pSetupReqPak->wValue) & 0xff;
                    break;

                case USB_CLEAR_FEATURE:
                {
                    if ((pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP)    // Endpoint
                    {
                        switch ((pSetupReqPak->wIndex) & 0xff)
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
                            errflag = 0xFF;                            // Unsupported endpoints
                            break;
                        }
                    }
                    else
                    {
                        errflag = 0xFF;
                    }
                }
                break;

                case USB_GET_INTERFACE:
                    EP0_Databuf[0] = 0x00;
                    if (SetupReqLen > 1)
                    {
                        SetupReqLen = 1;
                    }
                    break;

                case USB_GET_STATUS:
                    EP0_Databuf[0] = 0x00;
                    EP0_Databuf[1] = 0x00;
                    if (SetupReqLen > 2)
                    {
                        SetupReqLen = 2;
                    }
                    break;

                default:
                    errflag = 0xff;
                    break;
                }
            }
            if (errflag == 0xff)        // Error or not supported
            {
//                  SetupReqCode = 0xFF;
                R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL;    // STALL
            }
            else
            {
                if (chtype & 0x80)     // Upload
                {
                    len = (SetupReqLen > DevEP0SIZE) ? DevEP0SIZE : SetupReqLen;
                    SetupReqLen -= len;
                }
                else
                {
                    len = 0;        // Download
                }
                R8_UEP0_T_LEN = len;
                R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;    // The default packet is DATA1
            }
        }
        R8_USB_INT_FG = RB_UIF_TRANSFER;
    }
    else if (intflag & RB_UIF_BUS_RST)
    {
        R8_USB_DEV_AD = 0;
        R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        R8_UEP1_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK | RB_UEP_AUTO_TOG;
        R8_UEP2_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK | RB_UEP_AUTO_TOG;
        R8_UEP3_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK | RB_UEP_AUTO_TOG;
        R8_USB_INT_FG = RB_UIF_BUS_RST;
    }
    else if (intflag & RB_UIF_SUSPEND)
    {
        // wake
        R8_USB_INT_FG = RB_UIF_SUSPEND;
    }
    else
    {
        R8_USB_INT_FG = intflag;
    }
}

/* ***************************************************************************
* @fn myDevEP2_OUT_Deal
*
* @brief IAP USB data processing function, put it in ram to run to improve speed.
*
* @param None.
*
* @return None. */
__attribute__((section(".highcode")))
void myDevEP2_OUT_Deal(uint8_t l)
{
    /* User-customizable */
    uint8_t s = 0;
    uint32_t addr;
    switch (g_iap_cmd.other.buf[0])
    {
    case CMD_IAP_PROM:
        if (g_iap_cmd.program.len == 0)
        {
            if (g_buf_write_ptr != 0)
            {
            	g_buf_write_ptr = ((g_buf_write_ptr + 3) & (~3)); // Four byte alignment
                s = FLASH_ROM_WRITE(g_flash_write_ptr, (PUINT32)g_write_buf, g_buf_write_ptr);
                g_buf_write_ptr = 0;
            }
        }
        else
        {
            my_memcpy(g_write_buf + g_buf_write_ptr, g_iap_cmd.program.buf, g_iap_cmd.program.len);
            g_buf_write_ptr += g_iap_cmd.program.len;
            if (g_buf_write_ptr >= 256)
            {
                s = FLASH_ROM_WRITE(g_flash_write_ptr, (PUINT32)g_write_buf, 256);
                g_flash_write_ptr += 256;
                g_buf_write_ptr = g_buf_write_ptr - 256;    // Exceeded length
                my_memcpy(g_write_buf, g_write_buf + 256, g_buf_write_ptr); // Save the remaining iap_cmd.program.buf + g_iap_cmd.program.len - g_buf_write_ptr
            }
        }
        myDevEP2_IN_Deal(s);
        break;
    case CMD_IAP_ERASE:
    	// Here you can add address judgment or directly erase the specified location
    	addr = (g_iap_cmd.erase.addr[0]
				| (uint32_t) g_iap_cmd.erase.addr[1] << 8
				| (uint32_t) g_iap_cmd.erase.addr[2] << 16
				| (uint32_t) g_iap_cmd.erase.addr[3] << 24);
    	if(addr == APP_CODE_START_ADDR)
    	{
			s = FLASH_ROM_ERASE(APP_CODE_START_ADDR, APP_CODE_END_ADDR - APP_CODE_START_ADDR);
			g_buf_write_ptr = 0;    // Count clear
			g_flash_write_ptr = APP_CODE_START_ADDR;
    	}
    	else
    	{
    		s = 0xfe;
		}
        myDevEP2_IN_Deal(s);
        break;
    case CMD_IAP_VERIFY:
		my_memcpy(g_write_buf, g_iap_cmd.verify.buf, g_iap_cmd.verify.len);
		addr = (g_iap_cmd.verify.addr[0]
				| (uint32_t) g_iap_cmd.verify.addr[1] << 8
				| (uint32_t) g_iap_cmd.verify.addr[2] << 16
				| (uint32_t) g_iap_cmd.verify.addr[3] << 24);
		s = FLASH_ROM_VERIFY(addr, g_write_buf, g_iap_cmd.verify.len);
        myDevEP2_IN_Deal(s);
        break;
    case CMD_IAP_END:
        /* End the upgrade, reset the USB, and jump to the app */
        R8_USB_CTRL = RB_UC_RESET_SIE;
        R16_PIN_ANALOG_IE &= ~(RB_PIN_USB_DP_PU | RB_PIN_USB_IE);
        DelayMs(10);
        jumpApp();
        break;
    default:
        myDevEP2_IN_Deal(0xfe);
        break;
    }

}

/* *********************************************************************************************
* Function Name: myDevEP2_IN_Deal
* Description: Endpoint 2 data upload
* Input: l: Upload data length (<64B)
* Return : None
********************************************************************************************* */
__attribute__((section(".highcode")))
void myDevEP2_IN_Deal(uint8_t s)
{
    EP2_Databuf[64] = s;
    EP2_Databuf[65] = 0;
    R8_UEP2_T_LEN = 2;
    R8_UEP2_CTRL = (R8_UEP2_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_ACK; //enable send
}

/* ***************************************************************************
* @fn my_memcpy
*
* @brief data copy function, program is put into ram to improve speed
*
* @param None.
*
* @return None. */
__attribute__((section(".highcode")))
void my_memcpy(void *dst, const void *src, uint32_t l)
{
    uint32_t len = l;
    PUINT8 pdst = (PUINT8) dst;
    PUINT8 psrc = (PUINT8) src;
    while (len)
    {
        *pdst++ = *psrc++;
        len--;
    }
}
