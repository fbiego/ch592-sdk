/* ********************************* (C) COPYRIGHT *******************************
* File Name          : iap.c
* Author             : WCH
* Version            : V1.0
* Date               : 2022/03/15
* Description        : UART IAP例程
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
****************************************************************************** */
#include "iap.h"

/* Update permissions must be erased first, and the erase address must be consistent with the corresponding address of the APP before the update permission can be granted */
uint8_t g_update_permition = 0;

uint8_t iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_SOP1, all_data_rec_cnt = 0, part_rec_cnt = 0, uart_rec_sign = 0;

iap_cmd_t iap_rec_data;

__attribute__((aligned(4)))                    uint8_t g_write_buf[256 + 64];

uint16_t g_buf_write_ptr = 0;

uint32_t g_flash_write_ptr = 0;

uint32_t g_tcnt;

uint32_t g_addr;

__attribute__((aligned(4)))   uint8_t iap_rsp_data[6] = {IAP_DATA_SOP1, IAP_DATA_SOP2, 0, 0, IAP_DATA_EOP1, IAP_DATA_EOP2};

/* ***************************************************************************
* @fn Main_Circulation
*
* @brief The main loop of IAP, the program is put into ram to improve the speed.
*
* @param None.
*
* @return None. */
__attribute__((section(".highcode")))
void Main_Circulation()
{
    while (1)
    {
        /* Adopt query mode without using interrupts to reduce code occupancy of flash */
        if (R8_UART1_RFC)
        {
            /* There is data on the serial port, and the serial port is in the receiving process flag bit */
            uart_rec_sign = 1;
            /* There is data on the serial port, clear the timeout time */
            g_tcnt = 0;
            /* Receive count plus one, store it in buf */
            all_data_rec_cnt++;
            /* buf cannot cross the line */
            if (all_data_rec_cnt >= sizeof(iap_rec_data))
            {
                all_data_rec_cnt = 0;
            }
            /* Read serial port register data and directly assign value to buf. There is no need to continue to assign values ​​in the future, which can save program flash occupation */
            iap_rec_data.other.buf[all_data_rec_cnt] = R8_UART1_RBR;
            /* * In order to save the code footprint and code running speed, corresponding code cutting has been done here.
* Otherwise, the check value and packet tail information should be stored in the checksum and eop member variables of the corresponding structure according to the command code.
* The processing method of this program is to store it directly in buf in order.
* Therefore, when the data length is not full packet data length, it will be incorrect to access the member checksum and eop variables through the structure.
* Users need to pay attention to when modifying by themselves.
* Compared to the judgment of the package header every time, the judgment time and code flash space of the command are saved every time. */
            switch (iap_rec_data_state) /* Reading and storing data according to the received state */
            {
            /* When the status is waiting for packet header 1, determine whether the received byte is IAP_DATA_SOP1 */
            case IAP_DATA_REC_STATE_WAIT_SOP1:
                if (iap_rec_data.other.buf[all_data_rec_cnt] == IAP_DATA_SOP1)
                {
                    /* When there is only the first byte, it is not necessarily stored in the correct location of the buf, and it is restored. */
                    iap_rec_data.other.buf[0] = iap_rec_data.other.buf[all_data_rec_cnt];
                    /* Receive count initialization */
                    all_data_rec_cnt = 0;
                    iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_SOP2;
                }
                break;
            /* When the status is waiting for packet header 2, determine whether the received byte is IAP_DATA_SOP2 */
            case IAP_DATA_REC_STATE_WAIT_SOP2:
                if (iap_rec_data.other.buf[all_data_rec_cnt] == IAP_DATA_SOP2)
                {
                    iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_CMD;
                }
                else
                {
                    iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_SOP1;
                }
                break;
            /* When the status is waiting for the command code, determine whether the received byte is legal cmd */
            case IAP_DATA_REC_STATE_WAIT_CMD:
                if ((iap_rec_data.other.buf[all_data_rec_cnt] < CMD_IAP_PROM) || (iap_rec_data.other.buf[all_data_rec_cnt] > CMD_IAP_END))
                {
                    /* error does not have this cmd */
                    iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_SOP1;
                }
                else
                {
                    iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_LEN;
                }
                break;
            /* When the state is waiting for data length, determine whether the received byte is the legal length. */
            case IAP_DATA_REC_STATE_WAIT_LEN:
                if (iap_rec_data.other.buf[all_data_rec_cnt] <= IAP_LEN)
                {
                    /* Clear some structure variables receive byte count */
                    part_rec_cnt = 0;
                    if ((iap_rec_data.other.buf[2] == CMD_IAP_ERASE) || (iap_rec_data.other.buf[2] == CMD_IAP_VERIFY))
                    {
                        iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_ADDR;
                    }
                    else
                    {
                        /* Determine whether the data length is 0. If it is 0, the checksum will be received directly. */
                        if (iap_rec_data.other.buf[3] > 0)
                        {
                            iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_DATA;
                        }
                        else
                        {
                            iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_CHECKNUM;
                        }
                    }
                }
                else
                {
                    iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_SOP1;
                }
                break;
            /* When the status is waiting for the address */
            case IAP_DATA_REC_STATE_WAIT_ADDR:
                part_rec_cnt++;
                /* The address is 4 bytes. After receiving 4, it will jump to receive the next state. */
                if (part_rec_cnt >= 4)
                {
                    /* Some structure variables receive byte count */
                    part_rec_cnt = 0;
                    if (iap_rec_data.other.buf[3] > 0)
                    {
                        iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_DATA;
                    }
                    else
                    {
                        iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_CHECKNUM;
                    }
                }
                break;
            /* When the status is waiting for data */
            case IAP_DATA_REC_STATE_WAIT_DATA:
                part_rec_cnt++;
                if (part_rec_cnt >= iap_rec_data.other.buf[3])
                {
                    /* Determine whether the data is received and completed */
                    iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_CHECKNUM;
                    /* Some structure variables receive byte count */
                    part_rec_cnt = 0;
                }
                break;
            /* When the status is waiting for verification */
            case IAP_DATA_REC_STATE_WAIT_CHECKNUM:
                part_rec_cnt++;
                if (part_rec_cnt >= 2)
                {
                    /* Determine whether the verification is received and the verification is 2 bytes and */
                    iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_EOP1;
                }
                break;
            /* When the status is waiting for packet tail 1, determine whether the received byte is IAP_DATA_EOP1 */
            case IAP_DATA_REC_STATE_WAIT_EOP1:
                if (iap_rec_data.other.buf[all_data_rec_cnt] == IAP_DATA_EOP1)
                {
                    iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_EOP2;
                }
                else
                {
                    iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_SOP1;
                }
                break;
            /* When the status is waiting for packet tail 2, determine whether the received byte is IAP_DATA_EOP2 */
            case IAP_DATA_REC_STATE_WAIT_EOP2:
                if (iap_rec_data.other.buf[all_data_rec_cnt] == IAP_DATA_EOP2)
                {
                    iap_rec_data_state = IAP_DATA_REC_STATE_OK;
                }
                else
                {
                    iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_SOP1;
                }
                break;
            default:
                /* This is generally impossible */
                break;
            }

            if (iap_rec_data_state == IAP_DATA_REC_STATE_OK)
            {
                /* Calculate the checksum */
                uint16_t   check_num = 0, check_num_rec;
                /* Used for checksum calculation */
                uint16_t   check_num_i;
                /* The reported error code is cleared as the default error-free state */
                iap_rsp_data[2] = 0x00;
                iap_rsp_data[3] = 0x00;
                /* Restore the default state */
                iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_SOP1;
                /* parsed to a complete packet, freeing idle timer */
                uart_rec_sign = 0;
                g_tcnt = 0;
                /* Calculate the checksum verification */
                for (check_num_i = 2; check_num_i < all_data_rec_cnt - 3; check_num_i++)
                {
                    check_num += iap_rec_data.other.buf[check_num_i];
                }
                check_num_rec = iap_rec_data.other.buf[check_num_i] | (iap_rec_data.other.buf[check_num_i + 1] << 8);
                /* Packet checksum passes */
                if (check_num_rec == check_num)
                {
                    /* Judgment command */
                    switch (iap_rec_data.other.buf[2])
                    {
                    /* Write command */
                    case CMD_IAP_PROM:
                        /* Determine whether the chip has been erased first */
                        if (g_update_permition == 1)
                        {
                            if (iap_rec_data.program.len == 0)
                            {
                                /* The last time is an empty package, declared as the last package */
                                if (g_buf_write_ptr != 0)
                                {
                                    if (FLASH_ROM_WRITE(g_flash_write_ptr, (PUINT32)g_write_buf, g_buf_write_ptr))
                                    {
                                        iap_rsp_data[2] = 0xfe;
                                        iap_rsp_data[3] = IAP_ERR_WRITE_FAIL;
                                    }
                                    g_buf_write_ptr = 0;
                                }
                            }
                            else
                            {
                                my_memcpy(g_write_buf + g_buf_write_ptr, iap_rec_data.program.data, iap_rec_data.program.len);
                                g_buf_write_ptr += iap_rec_data.program.len;
                                if (g_buf_write_ptr >= 256)
                                {
                                    /* Write once for 256 bytes */
                                    if (FLASH_ROM_WRITE(g_flash_write_ptr, (PUINT32)g_write_buf, 256))
                                    {
                                        iap_rsp_data[2] = 0xfe;
                                        iap_rsp_data[3] = IAP_ERR_WRITE_FAIL;
                                        break;
                                    }
                                    /* Move the pointer */
                                    g_flash_write_ptr += 256;
                                    /* Recalculate the exceeded length and copy the exceeded data to the first part of the array */
                                    g_buf_write_ptr = g_buf_write_ptr - 256;
                                    my_memcpy(g_write_buf, g_write_buf + 256, g_buf_write_ptr);
                                }
                            }
                        }
                        else
                        {
                            /* There is no erase step, cannot be updated, and an error is reported */
                            iap_rsp_data[2] = 0xfe;
                            iap_rsp_data[3] = IAP_ERR_PROG_NO_ERASE;
                        }
                        break;
                    /* Erase command */
                    case CMD_IAP_ERASE:
                        if (iap_rec_data.erase.addr == APP_CODE_START_ADDR)
                        {
                            /* If there are special needs, you can modify the erase length and exclude the flash part of the special needs. */
                            if (FLASH_ROM_ERASE(APP_CODE_START_ADDR, APP_CODE_END_ADDR - APP_CODE_START_ADDR) == 0)
                            {
                                /* The starting address is correct, and the update permission is granted, otherwise it will be considered to be failed. */
                                g_update_permition = 1;
                                /* Count clear, flash write pointer reset */
                                g_buf_write_ptr = 0;
                                g_flash_write_ptr = APP_CODE_START_ADDR;
                            }
                            else
                            {
                                /* Erase failed */
                                iap_rsp_data[2] = 0xfe;
                                iap_rsp_data[3] = IAP_ERR_ERASE_FAIL;
                            }
                        }
                        else
                        {
                            /* Erase address error */
                            iap_rsp_data[2] = 0xfe;
                            iap_rsp_data[3] = IAP_ERR_ADDR;
                        }
                        break;
                    /* Verification command */
                    case CMD_IAP_VERIFY:
                        if (((iap_rec_data.verify.addr % 4) == 0) && (iap_rec_data.verify.addr >= APP_CODE_START_ADDR) && (iap_rec_data.verify.addr < APP_CODE_END_ADDR))
                        {
                            my_memcpy(g_write_buf, iap_rec_data.verify.data, iap_rec_data.verify.len);
                            if (FLASH_ROM_VERIFY(iap_rec_data.verify.addr, g_write_buf, iap_rec_data.verify.len))
                            {
                                /* Check failed, error reported */
                                iap_rsp_data[2] = 0xfe;
                                iap_rsp_data[3] = IAP_ERR_VERIFY;
                            }
                        }
                        else
                        {
                            /* The verification address is incorrect */
                            iap_rsp_data[2] = 0xfe;
                            iap_rsp_data[3] = IAP_ERR_ADDR;
                        }
                        break;
                    /* End jump command */
                    case CMD_IAP_END:
                        jumpApp();
                        break;
                    default:
                        /* It was judged that the command could not be another value during reception, so this situation will not occur. */
                        iap_rsp_data[2] = 0xfe;
                        iap_rsp_data[3] = IAP_ERR_UNKNOWN;
                        break;
                    }
                    if (iap_rsp_data[2] != 0)
                    {
                        /* After the checksum is passed, clear the update permission after other errors. If you want to update, follow the steps to start again. */
                        g_update_permition = 0;
                    }
                }
                else
                {
                    /* The packet checksum fails, and the host can choose to resend the packet without any impact. */
                    iap_rsp_data[2] = 0xfe;
                    iap_rsp_data[3] = IAP_ERR_CHECK;
                }
                /* Each time the data packet is processed, clear the cache of other data to prevent some serial port modules from occasionally sending one or two characters signals, keeping one packet and one reply */
                while (R8_UART1_RFC)
                {
                    iap_rec_data.other.buf[all_data_rec_cnt] = R8_UART1_RBR;
                }
                /* Reply to data */
                UART1_SendString(iap_rsp_data, sizeof(iap_rsp_data));
            }
        }
        else
        {
            /* Delay the time of about one quarter byte at 115200 baud rate, reduce the frequency and time of the read register, and facilitate timeout counting. If the baud rate is modified, the relevant time parameters must also be changed. */
            DelayUs(20);
            g_tcnt++;
            if (uart_rec_sign)
            {
                if (g_tcnt >= 43)
                {
                    /* If there are more than 10 bytes, no new bytes will arrive, and there is no complete data packet, the timeout will be reported and the baud rate will be modified. */
                    /* Currently the baud rate is 115200, and one byte time is 1s/11520 = 87us, 87us*10 / 20us = 43.5 */
                    /* The host can choose to resend the packet without any impact */
                    iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_SOP1;
                    uart_rec_sign = 0;
                    iap_rsp_data[2] = 0xfe;
                    iap_rsp_data[3] = IAP_ERR_OVERTIME;
                    UART1_SendString(iap_rsp_data, sizeof(iap_rsp_data));
                }
            }
            else
            {
                if (g_tcnt > 6000000)
                {
                    /* There is no data in 120 seconds. If you think it is timed out, jump to the app and modify it yourself according to the situation. */
                    jumpApp();
                }
            }
        }
    }
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
