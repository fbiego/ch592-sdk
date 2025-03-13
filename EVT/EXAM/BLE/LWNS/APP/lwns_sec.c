/* ********************************* (C) COPYRIGHT *******************************
* File Name          : lwns_sec.c
* Author             : WCH
* Version            : V1.0
* Date               : 2021/09/17
* Description        : lwnsœ˚œ¢º”√‹
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
****************************************************************************** */
#include "lwns_sec.h"

static uint8_t lwns_sec_key[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}; // The user changes to his or her own key, or can be retrieved from the host and stored in eeprom

/* ***************************************************************************
* @fn lwns_msg_encrypt
*
* @brief lwns message encryption
*
* @param src - The data buffer head pointer to be encrypted.
* @param to - The header pointer of the data cache area to be stored.
* @param mlen - length of data to be encrypted.
*
* @return Encrypted data length. */
int lwns_msg_encrypt(uint8_t *src, uint8_t *to, uint8_t mlen)
{
    uint16_t i = 0;
    uint8_t  esrc[16];
    while(1)
    {
        if((mlen - i) < 16)
        {
            tmos_memcpy(esrc, src + i, (mlen - i)); // Expand to 16 bytes, others are 0
            LL_Encrypt(lwns_sec_key, esrc, to + i);
        }
        else
        {
            LL_Encrypt(lwns_sec_key, src + i, to + i);
        }
        i += 16;
        if(i >= mlen)
        {
            break;
        }
    }
    return i; // Returns the encrypted data length
}

/* ***************************************************************************
* @fn lwns_msg_decrypt
*
* @brief lwns message decryption
*
* @param src - The data buffer head pointer to be decrypted.
* @param to - The data cache header pointer to the decrypted data to be stored.
* @param mlen - The length of the data to be decrypted must be a multiple of 16.
*
* @return The decrypted data length. */
int lwns_msg_decrypt(uint8_t *src, uint8_t *to, uint8_t mlen)
{
    unsigned short i = 0;
    while(1)
    {
        LL_Decrypt(lwns_sec_key, src + i, to + i);
        i += 16;
        if(i >= mlen)
        {
            break;
        }
    }
    return i;
}
