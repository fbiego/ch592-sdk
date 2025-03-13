/********************************** (C) COPYRIGHT *******************************
 * File Name          : OTA.C
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2018/12/10
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include <rf.h>
#include "rf_device.h"
#include "CH59x_common.h"
#include "ota.h"
#include "usb.h"
#include "peripheral.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
const uint8_t hand_shake_string[] = "WCH@TOOL";
const uint32_t hex_num = 1;


#define IAP_SAFE_FLAG_2_4G       0x30de5820
#define IAP_SAFE_FLAG_BLE        0x30de5821
#define IAP_SAFE_FLAG_MASK       0x30de5820

/* APP */
__attribute__((aligned(4))) uint32_t save_Flag __attribute__((section(".ImageFlag"))) = IAP_SAFE_FLAG_2_4G;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
uint8_t ota_taskID;

// OTA IAP VARIABLES
/* OTA */
OTA_IAP_CMD_t iap_rec_data;

/* OTA */
uint32_t OpParaDataLen = 0;
uint32_t OpAdd = 0;

/* flash */
__attribute__((aligned(8))) uint8_t block_buf[16];

/* Image */
typedef int (*pImageTaskFn)(void);
pImageTaskFn user_image_tasks;

/* Flash  */
uint32_t EraseAdd = 0;      //
uint32_t EraseBlockNum = 0; //
uint32_t EraseBlockCnt = 0; //

/* FLASH  */
uint8_t VerifyStatus = 0;

#define MAX_FLASH_BUFF_LEN      256
__attribute__((aligned(4))) uint8_t flash_buf[MAX_FLASH_BUFF_LEN];
uint8_t flash_buf_uesd_len = 0;
uint8_t flash_verify_flag = 0;
uint8_t flash_erase_flag = 0;

uint32_t flash_offset = 0;  //
uint32_t iap_ok=0;
/*********************************************************************
 * LOCAL FUNCTIONS
 */
uint16_t    OTA_ProcessEvent(uint8_t task_id, uint16_t events);
void        OTA_IAPReadDataComplete(unsigned char index);
void        OTA_IAPWriteData(unsigned char *p_data, unsigned char w_len);
void        Rec_OTA_IAP_DataDeal(void);
void        OTA_IAP_SendCMDDealSta(uint8_t deal_status);
void DisableAllIRQ(void);
void SwitchImageFlag(uint8_t new_flag);
void OTA_IAP_PROM(void);
void OTA_IAP_VERIFY(void);
/*********************************************************************
 * PROFILE CALLBACKS
 */


/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      OTA_Init
 *
 * @brief   OTA_Init
 *
 * @return  none
 */
void OTA_Init()
{
    ota_taskID = TMOS_ProcessEventRegister(OTA_ProcessEvent);
}

/*********************************************************************
 * @fn      OTA_ProcessEvent
 *
 * @brief   OTA_ProcessEvent
 *
 * @param   task_id  - The TMOS assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed
 */
uint16_t OTA_ProcessEvent(uint8_t task_id, uint16_t events)
{
    //OTA_FLASH_ERASE_EVT
    if(events & OTA_FLASH_ERASE_EVT)
    {
        uint8_t status;

        status = FLASH_ROM_ERASE(EraseAdd ,EraseBlockNum * FLASH_BLOCK_SIZE);

        /*  */
        if(status != SUCCESS)
        {
#if(DEBUG_OTA)
        PRINT("ERASE err\r\n");
#endif
            OTA_IAP_SendCMDDealSta(status);
            return (events ^ OTA_FLASH_ERASE_EVT);
        }

#if(DEBUG_OTA)
        PRINT("ERASE Complete\r\n");
#endif
        if(work_mode == MODE_USB)
        {
            OTA_IAP_SendCMDDealSta(status);
        }
        else {
            OTA_IAP_SendCMDDealSta(status);
            flash_erase_flag = 1;
        }

        return (events ^ OTA_FLASH_ERASE_EVT);
    }

    if(events & OTA_IAP_END_EVT)
    {
        /* ImageA */
        /*  */
        DisableAllIRQ();

        /* DataFlashImageIAP */
        SwitchImageFlag(IMAGE_IAP_FLAG);

        SYS_ResetExecute();
        return (events ^ OTA_IAP_END_EVT);
    }

    if(events & OTA_FLASH_PROM_EVT)
    {
        OTA_IAP_PROM();
        return (events ^ OTA_FLASH_PROM_EVT);
    }

    if(events & OTA_FLASH_VRIF_EVT)
    {
        OTA_IAP_VERIFY();
        return (events ^ OTA_FLASH_VRIF_EVT);
    }

    if(events & OTA_RESTART_EVT)
    {
        SYS_ResetExecute();
        return (events ^ OTA_RESTART_EVT);
    }

    // Discard unknown events
    return 0;
}

/*********************************************************************
 * @fn      OTA_get_checksum
 *
 * @brief   OTA_get_checksum
 *
 * @return  none
 */
uint8_t OTA_get_checksum( uint8_t *pData, uint8_t len )
{
    int i;
    uint8_t checksum = 0;
    for( i = 0; i < len; i++ )
    {
        checksum += pData[i];
    }
    return checksum;
}

/*********************************************************************
 * @fn      OTA_IAP_SendCMDDealSta
 *
 * @brief   OTA IAP
 *
 * @param   deal_status - 
 *
 * @return  none
 */
__HIGH_CODE
void OTA_IAP_SendCMDDealSta(uint8_t deal_status)
{
    uint8_t send_buf[6];

    send_buf[0] = RF_DATA_IAP;
    send_buf[1] = CMD_STATE_ACK;
    send_buf[2] = 0x01;
    send_buf[3] = deal_status;
    send_buf[4] = OTA_get_checksum(&send_buf[1],3);
    if(work_mode == MODE_USB)
    {
        USB_IAP_report(&send_buf[1], 5-1);
    }
    else
    {
        if(rf_send_data( send_buf, 5 ))
        {
            PRINT("rf err\n");
        }
    }
}

/*********************************************************************
 * @fn      OTA_IAP_CMDErrDeal
 *
 * @brief   OTA IAP
 *
 * @return  none
 */
void OTA_IAP_CMDErrDeal(void)
{
    OTA_IAP_SendCMDDealSta(0xf8);
}

/*********************************************************************
 * @fn      SwitchImageFlag
 *
 * @brief   dataflashImageFlag
 *
 * @param   new_flag    - ImageFlag
 *
 * @return  none
 */
void SwitchImageFlag(uint8_t new_flag)
{
    /*  */
    EEPROM_READ(OTA_DATAFLASH_ADD, (uint32_t *)&block_buf[0], 4);

    /*  */
    EEPROM_ERASE(OTA_DATAFLASH_ADD, EEPROM_PAGE_SIZE);

    /* Image */
    block_buf[0] = new_flag;

    /* DataFlash */
    EEPROM_WRITE(OTA_DATAFLASH_ADD, (uint32_t *)&block_buf[0], 4);
}

/*********************************************************************
 * @fn      DisableAllIRQ
 *
 * @brief   
 *
 * @return  none
 */
void DisableAllIRQ(void)
{
    SYS_DisableAllIrq(NULL);
}

/*********************************************************************
 * @fn      OTA_IAP_PROM
 *
 * @brief   OTA_IAP_PROM
 *
 * @return  none
 */
void OTA_IAP_PROM(void)
{
    uint8_t  status = 0;
    /* ImageA */
    status = FLASH_ROM_WRITE(OpAdd, flash_buf, flash_buf_uesd_len);
    if(status)
    {
#if(DEBUG_OTA)
        PRINT("IAP_PROM err \r\n");
#endif
        iap_ok = 0;
    }
    flash_buf_uesd_len = 0;
    OTA_IAP_SendCMDDealSta(status);
}

/*********************************************************************
 * @fn      OTA_IAP_VERIFY
 *
 * @brief   OTA_IAP_VERIFY
 *
 * @return  none
 */
void OTA_IAP_VERIFY(void)
{
    uint8_t  status = 0;
    /* ImageAImageB */
    status = FLASH_ROM_VERIFY(OpAdd, flash_buf, flash_buf_uesd_len);
    if(status)
    {
#if(DEBUG_OTA)
        PRINT("IAP_VERIFY err \r\n");
        for(uint8_t i=0; i<flash_buf_uesd_len; i++)
        {
            if(flash_buf[i]!=((uint8_t*)OpAdd)[i])
            {
                PRINT("%d : %x %x \r\n",i,flash_buf[i],((uint8_t*)OpAdd)[i]);
            }
        }
#endif
        iap_ok = 0;
    }
    flash_buf_uesd_len = 0;
    VerifyStatus |= status;
    OTA_IAP_SendCMDDealSta(VerifyStatus);
}

/*********************************************************************
 * @fn      Rec_OTA_IAP_DataDeal
 *
 * @brief   OTA
 *
 * @return  none
 */
void Rec_OTA_IAP_DataDeal(void)
{
    switch(iap_rec_data.other.buf[0])
    {
        /*  */
        case CMD_HAND_SHAKE:
        {
            if( tmos_memcmp(iap_rec_data.handshake.string, hand_shake_string, 7))
            {
                uint8_t send_buf[20];
                PRINT("CMD_HAND_SHAKE \r\n");

                send_buf[0] = CMD_HAND_SHAKE_ACK;
                send_buf[1] = 0x04;
                send_buf[2] = (uint8_t)(hex_num & 0xff);
                send_buf[3] = (uint8_t)((hex_num >> 8) & 0xff);
                send_buf[4] = (uint8_t)((hex_num >> 16) & 0xff);
                send_buf[5] = (uint8_t)((hex_num >> 24) & 0xff);

                send_buf[6] = OTA_get_checksum(send_buf,6);

                if(work_mode == MODE_USB)
                {
                    USB_IAP_report(send_buf, 7);
                }
                else
                {
                    PRINT("work_mode err\n");
                }
            }
            else
            {
                PRINT("%s : %s\r\n",hand_shake_string,iap_rec_data.handshake.string);
                OTA_IAP_SendCMDDealSta(IAP_STATE_FAILURE);
            }
            break;
        }
        /*  */
        case CMD_IAP_PROM:
        {
            if(flash_buf_uesd_len==0)
            {
                OpParaDataLen = iap_rec_data.program.len;
                OpAdd = (uint32_t)(iap_rec_data.program.addr[0]);
                OpAdd |= ((uint32_t)(iap_rec_data.program.addr[1]) << 8);
#if(DEBUG_OTA)
                PRINT("P1: %x l:%d\n", (int)OpAdd, (int)OpParaDataLen);
#endif
                OpAdd = OpAdd * 16;
                if( OpAdd == 0x1000 )
                {
                    if(((iap_rec_data.program.buf[4]|(iap_rec_data.program.buf[5]<<8)|
                        (iap_rec_data.program.buf[6]<<16)|(iap_rec_data.program.buf[7]<<24))&0xFFFFFFF0) != IAP_SAFE_FLAG_MASK)
                    {
#if(DEBUG_OTA)
                        PRINT("IAP_SAFE_FLAG err %x\r\n",(iap_rec_data.program.buf[4]|(iap_rec_data.program.buf[5]<<8)|
                            (iap_rec_data.program.buf[6]<<16)|(iap_rec_data.program.buf[7]<<24)));
#endif
                        iap_ok = 0;
                        OTA_IAP_SendCMDDealSta(0xFF);
                        break;
                    }
                }

                OpAdd += flash_offset;

                tmos_memcpy(flash_buf, iap_rec_data.program.buf, OpParaDataLen);
                flash_buf_uesd_len += OpParaDataLen;
                if( flash_buf_uesd_len>(MAX_FLASH_BUFF_LEN-OpParaDataLen) )
                {
                    // flash
                    tmos_set_event(ota_taskID, OTA_FLASH_PROM_EVT);
                    break;
                }
            }
            else {
                OpParaDataLen = iap_rec_data.program.len;
#if(DEBUG_OTA)
                PRINT("P2: %x l:%d\n", (int)(iap_rec_data.program.addr[0]|((iap_rec_data.program.addr[1]) << 8)), (int)OpParaDataLen);
#endif
                tmos_memcpy(&flash_buf[flash_buf_uesd_len], iap_rec_data.program.buf, OpParaDataLen);
                flash_buf_uesd_len += OpParaDataLen;
                if( flash_buf_uesd_len>(MAX_FLASH_BUFF_LEN-OpParaDataLen) )
                {
                    // flash
                    tmos_set_event(ota_taskID, OTA_FLASH_PROM_EVT);
                    break;
                }
            }
            OTA_IAP_SendCMDDealSta(VerifyStatus);
            break;
        }
        /*  --  */
        case CMD_IAP_ERASE:
        {
            OpAdd = (uint32_t)(iap_rec_data.erase.addr[0]);
            OpAdd |= ((uint32_t)(iap_rec_data.erase.addr[1]) << 8);
            OpAdd = OpAdd * 16;

            if(OpAdd == IMAGE_A_2_4G_START_ADD)
            {
                flash_offset = IMAGE_A_2_4G_SIZE + IMAGE_A_BLE_SIZE;
                OpAdd += IMAGE_A_2_4G_SIZE + IMAGE_A_BLE_SIZE;
            }
            else if(OpAdd == IMAGE_A_BLE_START_ADD)
            {
                flash_offset = IMAGE_A_BLE_SIZE;
                OpAdd += IMAGE_A_BLE_SIZE;
            }
            else {
#if(DEBUG_OTA)
                PRINT("EraseAdd err %x \r\n",OpAdd);
#endif
                flash_offset = 0;
                OTA_IAP_SendCMDDealSta(0xFF);
            }

            EraseBlockNum = (uint32_t)(iap_rec_data.erase.block_num[0]);
            EraseBlockNum |= ((uint32_t)(iap_rec_data.erase.block_num[1]) << 8);
            EraseAdd = OpAdd;
            EraseBlockCnt = 0;

            /* 0 */
            VerifyStatus = 0;

            iap_ok = 1;
#if(DEBUG_OTA)
            PRINT("IAP_ERASE start:%08x num:%d\r\n", (int)OpAdd, (int)EraseBlockNum);
#endif

            if((EraseAdd < IMAGE_B_START_ADD) || ((EraseAdd + (EraseBlockNum) * FLASH_BLOCK_SIZE) > IMAGE_IAP_START_ADD))
            {
#if(DEBUG_OTA)
            PRINT("EraseAdd err\r\n");
#endif
                OTA_IAP_SendCMDDealSta(0xFF);
            }
            else
            {
                if(flash_erase_flag == 0)
                {
                    flash_verify_flag = 0;
                    tmos_set_event(ota_taskID, OTA_FLASH_ERASE_EVT);
                }
                else {
                    flash_erase_flag = 0;
                    OTA_IAP_SendCMDDealSta(0);
                }

            }
            break;
        }
        /*  */
        case CMD_IAP_VERIFY:
        {
            uint8_t  status=0;
            if(flash_verify_flag ==0)
            {
                if(flash_buf_uesd_len)
                {
                    /* ImageA */
                    status = FLASH_ROM_WRITE(OpAdd, flash_buf, flash_buf_uesd_len);
                    if(status)
                    {
#if(DEBUG_OTA)
                        PRINT("IAP_VERIFY WRITE err \r\n");
#endif
                        iap_ok = 0;
                    }
                    flash_buf_uesd_len = 0;
                }
            }
            flash_verify_flag = 1;
            if(flash_buf_uesd_len==0)
            {
                OpParaDataLen = iap_rec_data.verify.len;
                OpAdd = (uint32_t)(iap_rec_data.verify.addr[0]);
                OpAdd |= ((uint32_t)(iap_rec_data.verify.addr[1]) << 8);
#if(DEBUG_OTA)
                PRINT("V1: %x l:%d\n", (int)OpAdd, (int)OpParaDataLen);
#endif
                OpAdd = OpAdd * 16;

                OpAdd += flash_offset;

                tmos_memcpy(flash_buf, iap_rec_data.verify.buf, OpParaDataLen);
                flash_buf_uesd_len += OpParaDataLen;
                if( flash_buf_uesd_len>(MAX_FLASH_BUFF_LEN-OpParaDataLen) )
                {
                    // flash
                    tmos_set_event(ota_taskID, OTA_FLASH_VRIF_EVT);
                    break;
                }
            }
            else {
                OpParaDataLen = iap_rec_data.verify.len;
#if(DEBUG_OTA)
                PRINT("V2: %x l:%d\n", (int)(iap_rec_data.program.addr[0]|((iap_rec_data.program.addr[1]) << 8)), (int)OpParaDataLen);
#endif
                tmos_memcpy(&flash_buf[flash_buf_uesd_len], iap_rec_data.verify.buf, OpParaDataLen);
                flash_buf_uesd_len += OpParaDataLen;
                if( flash_buf_uesd_len>(MAX_FLASH_BUFF_LEN-OpParaDataLen) )
                {
                    // flash
                    tmos_set_event(ota_taskID, OTA_FLASH_VRIF_EVT);
                    break;
                }
            }
            VerifyStatus |= status;
            OTA_IAP_SendCMDDealSta(VerifyStatus);
            break;
        }
        /*  */
        case CMD_IAP_END:
        {
#if(DEBUG_OTA)
            PRINT("IAP_END \r\n");
#endif
            if(flash_buf_uesd_len)
            {
                uint8_t  status = 0;
                /* ImageAImageB */
                status = FLASH_ROM_VERIFY(OpAdd, flash_buf, flash_buf_uesd_len);
                if(status)
                {
#if(DEBUG_OTA)
                    PRINT("IAP_VERIFY err \r\n");
#endif
                    iap_ok = 0;
                }
                flash_buf_uesd_len = 0;
                VerifyStatus |= status;
                OTA_IAP_SendCMDDealSta(VerifyStatus);
                if(VerifyStatus)
                {
                    break;
                }
            }
            else {
                OTA_IAP_SendCMDDealSta(IAP_STATE_SUCCESS);
            }
            if(iap_ok)
            {
                /*  */
                tmos_start_task(ota_taskID, OTA_IAP_END_EVT, 16);
            }
            else {
#if(DEBUG_OTA)
                PRINT("IAP_end err \r\n");
#endif
            }
            break;
        }
        case CMD_IAP_INFO:
        {
            uint8_t send_buf[20];

#if(DEBUG_OTA)
            PRINT("IAP_INFO \r\n");
#endif
            send_buf[0] = RF_DATA_IAP;
            send_buf[1] = CMD_IAP_INFO_ACK;
            send_buf[2] = 0x08;
            send_buf[3] = (uint8_t)(IMAGE_B_SIZE & 0xff);
            send_buf[4] = (uint8_t)((IMAGE_B_SIZE >> 8) & 0xff);
            send_buf[5] = (uint8_t)((IMAGE_B_SIZE >> 16) & 0xff);
            send_buf[6] = (uint8_t)((IMAGE_B_SIZE >> 24) & 0xff);

            /* BLOCK SIZE */
            send_buf[7] = (uint8_t)(FLASH_BLOCK_SIZE & 0xff);
            send_buf[8] = (uint8_t)((FLASH_BLOCK_SIZE >> 8) & 0xff);

            send_buf[9] = CHIP_ID&0xFF;
            send_buf[10] = (CHIP_ID>>8)&0xFF;
            send_buf[11] = OTA_get_checksum(&send_buf[1],10);
            if(work_mode == MODE_USB)
            {
                USB_IAP_report(&send_buf[1], 12-1);
            }
            else
            {
                if(rf_send_data( send_buf, 12 ))
                {
                    PRINT("rf err\n");
                }
            }
            break;
        }
        /*  */
        case CMD_SINGLE_CHANNEL:
        {
            if(iap_rec_data.other.buf[1]==(unsigned char)(1-2))
            {
                PRINT("SINGLE_CHANNEL %d\r\n",iap_rec_data.other.buf[2]);
                RFIP_SingleChannel(iap_rec_data.other.buf[2]);
                RFIP_SetTxPower(LL_TX_POWEER_0_DBM);
                OTA_IAP_SendCMDDealSta(IAP_STATE_SUCCESS);
            }
            else
            {
                OTA_IAP_CMDErrDeal();
            }
            break;
        }

        /*  */
        case CMD_SINGLE_POWER:
        {
            if(iap_rec_data.other.buf[1]==(unsigned char)(1-2))
            {
                PRINT("SINGLE_POWER %d\r\n",iap_rec_data.other.buf[2]);
                RFIP_SetTxPower(iap_rec_data.other.buf[2]);
                OTA_IAP_SendCMDDealSta(IAP_STATE_SUCCESS);
            }
            else
            {
                OTA_IAP_CMDErrDeal();
            }
            break;
        }

        /*  */
        case CMD_CAPACITANCE:
        {
            if(iap_rec_data.other.buf[1]==(unsigned char)(1-2))
            {
                PRINT("CAPACITANCE %d\r\n",iap_rec_data.other.buf[2]);
                nvs_flash_info.capacitance = iap_rec_data.other.buf[2];
                nvs_flash_store();
                HSECFG_Capacitance(nvs_flash_info.capacitance);
                OTA_IAP_SendCMDDealSta(IAP_STATE_SUCCESS);
            }
            else
            {
                OTA_IAP_CMDErrDeal();
            }
            break;
        }

        /* CMD_RESTART */
        case CMD_RESTART:
        {
            if(iap_rec_data.other.buf[1]==(unsigned char)(0-2))
            {
                PRINT("CMD_RESTART\n");
                OTA_IAP_SendCMDDealSta(IAP_STATE_SUCCESS);
                tmos_start_task(ota_taskID, OTA_RESTART_EVT, 160);
            }
            else
            {
                OTA_IAP_CMDErrDeal();
            }
            break;
        }

        /* CMD_CONFIG_VID_PID */
        case CMD_CONFIG_VID_PID:
        {
            if(iap_rec_data.other.buf[1]==(unsigned char)(4-2))
            {
                PRINT("CFG_VID_PID\n");
                tmos_memcpy(nvs_flash_info.usb_vid_pid, &iap_rec_data.other.buf[2], 4);
                nvs_flash_store();
                OTA_IAP_SendCMDDealSta(IAP_STATE_SUCCESS);
            }
            else
            {
                OTA_IAP_CMDErrDeal();
            }
            break;
        }

        /* CMD_CONFIG_PROD_INFO */
        case CMD_CONFIG_PROD_INFO:
        {
            nvs_flash_info.usb_prod_info_len = iap_rec_data.other.buf[1]-2;
            PRINT("CMD_CONFIG_PROD_INFO %d\n",nvs_flash_info.usb_prod_info_len);
            tmos_memcpy(nvs_flash_info.usb_prod_info, &iap_rec_data.other.buf[2], nvs_flash_info.usb_prod_info_len);
            nvs_flash_store();
            OTA_IAP_SendCMDDealSta(IAP_STATE_SUCCESS);
            break;
        }

        default:
        {
            PRINT("cmd err ! %x \r\n",iap_rec_data.other.buf[0]);

            OTA_IAP_CMDErrDeal();
            break;
        }
    }
}

/*********************************************************************
 * @fn      OTA_USB_IAPWriteData
 *
 * @brief   OTA 
 *
 * @param   index   - OTA 
 * @param   p_data  - 
 * @param   w_len   - 
 *
 * @return  none
 */
void OTA_USB_IAPWriteData(unsigned char *p_data, unsigned char w_len)
{
    if(p_data[p_data[1]+2] != OTA_get_checksum( p_data, p_data[1]+2 ))
    {
        OTA_IAP_SendCMDDealSta(0xF3);
    }
    else
    {
        tmos_memcpy((unsigned char *)&iap_rec_data, p_data, p_data[1]+3);
        // IAP
        iap_rec_data.program.len -= 2;
        Rec_OTA_IAP_DataDeal();
    }
}
/*********************************************************************
*********************************************************************/
