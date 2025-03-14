/********************************** (C) COPYRIGHT *******************************
 * File Name          : CH59x_flash.c
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

/* RESET_EN */
#define RESET_Enable           0x00000008
#define RESET_Disable          0xFFFFFFF7

/* LOCKUP_RST_EN */
#define UART_NO_KEY_Enable     0x00000100
#define UART_NO_KEY_Disable    0xFFFFFEFF

/* BOOT_PIN */
#define BOOT_PIN_PB22          0x00000200
#define BOOT_PIN_PB11          0xFFFFFDFF

/* FLASH_WRProt */
#define FLASH_WRProt           0xFFF003FF

/*********************************************************************
 * @fn      FLASH_ROM_READ
 *
 * @brief   Read Flash
 *
 * @param   StartAddr   - read address
 * @param   Buffer      - read buffer
 * @param   len         - read len
 *
 * @return  none
 */
void FLASH_ROM_READ(uint32_t StartAddr, void *Buffer, uint32_t len)
{
    uint32_t  i, Length = (len + 3) >> 2;
    uint32_t *pCode = (uint32_t *)StartAddr;
    uint32_t *pBuf = (uint32_t *)Buffer;

    for(i = 0; i < Length; i++)
    {
        *pBuf++ = *pCode++;
    }
}

/* ***************************************************************************
* @fn UserOptionByteConfig
*
* @brief Configure User Option Byte. It needs to be effective after calling the user configuration word effect function, and can only be modified once each time it is burned.
* (With this function, you must use the .S file provided by the official. After calling the function at the same time, after powering on twice, the two-wire debugging interface will be closed by default)
*
* @param RESET_EN - External reset pin enable
* @param BOOT_PIN - ENABLE-Use default boot pin-PB22, DISABLE-Use boot pin-PB11
* @param UART_NO_KEY_EN - Serial port key-free download enable
* @param FLASHProt_Size - Write-protect size (unit 4K)
*
* @return 0-Success, 1-Err */
uint8_t UserOptionByteConfig(FunctionalState RESET_EN, FunctionalState BOOT_PIN, FunctionalState UART_NO_KEY_EN,
                           uint32_t FLASHProt_Size)
{
    uint32_t s, t;

    FLASH_ROM_READ(0x14, &s, 4);

    if(s == 0xF5F9BDA9)
    {
        s = 0;
        FLASH_EEPROM_CMD(CMD_GET_ROM_INFO, 0x7EFFC, &s, 4);
        s &= 0xFF;

        if(RESET_EN == ENABLE)
            s |= RESET_Enable;
        else
            s &= RESET_Disable;

        /* bit[7:0]-bit[31-24] */
        s |= ((~(s << 24)) & 0xFF000000); // High 8 bits configuration information inverted;

        if(BOOT_PIN == ENABLE)
            s |= BOOT_PIN_PB22;
        if(UART_NO_KEY_EN == ENABLE)
            s |= UART_NO_KEY_Enable;

        /* bit[23-10] */
        s &= 0xFF0003FF;
        s |= ((FLASHProt_Size << 10) | (5 << 20)) & 0x00FFFC00;

        /*Write user option byte*/
        FLASH_ROM_WRITE(0x14, &s, 4);

        /* Verify user option byte */
        FLASH_ROM_READ(0x14, &t, 4);

        if(s == t)
            return 0;
        else
            return 1;
    }

    return 1;
}

/* ***************************************************************************
* @fn UserOptionByteClose_SWD
*
* @brief Close the two-wire debugging interface, and the remaining configuration values ​​remain unchanged. It needs to be effective after calling the user configuration word effect function, and can only be modified once each time it is burned.
* (With this function, you must use the .S file provided by the official. After calling the function at the same time, after powering on twice, the two-wire debugging interface will be closed by default)
*
* @return 0-Success, 1-Err */
uint8_t UserOptionByteClose_SWD(void)
{
    uint32_t s, t;

    FLASH_ROM_READ(0x14, &s, 4);

    if(s == 0xF5F9BDA9)
    {
        FLASH_EEPROM_CMD(CMD_GET_ROM_INFO, 0x7EFFC, &s, 4);

        s &= ~((1 << 4) | (1 << 7)); // Disable debugging function, disable SPI read and write FLASH

        /* bit[7:0]-bit[31-24] */
        s &= 0x00FFFFFF;
        s |= ((~(s << 24)) & 0xFF000000); // High 8 bits configuration information inverted;

        /*Write user option byte*/
        FLASH_ROM_WRITE(0x14, &s, 4);

        /* Verify user option byte */
        FLASH_ROM_READ(0x14, &t, 4);

        if(s == t)
            return 0;
        else
            return 1;
    }

    return 1;
}

/* ***************************************************************************
* @fn UserOptionByte_Active
*
* @brief The user configures the word effective function and automatically reset after execution
*
* @return 0-Success, 1-Err */
void UserOptionByte_Active(void)
{
    FLASH_ROM_SW_RESET();
    sys_safe_access_enable();
    R16_INT32K_TUNE = 0xFFFF;
    sys_safe_access_disable();
    sys_safe_access_enable();
    R8_RST_WDOG_CTRL |= RB_SOFTWARE_RESET;
    sys_safe_access_disable();
    while(1);
}
