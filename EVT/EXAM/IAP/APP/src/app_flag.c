/* ********************************* (C) COPYRIGHT *******************************
* File Name          : app_flag.c
* Author             : WCH
* Version            : V1.0
* Date               : 2022/03/15
* Description        : USB IAP APPÀý³Ì
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
****************************************************************************** */
#include "app_flag.h"

/* ***************************************************************************
* @fn SwitchImageFlag
*
* @brief Switch to the Flag in dataflash. If you want to switch to IAP, set the flag to ImageFlag_USER_PROGRAM_CALL_IAP and jump to the 0 position
*
* @param new_flag - Switched Flag
*
* @return none */
void SwitchImageFlag(uint8_t new_flag)
{
    UINT16 i;
    UINT32 ver_flag;
    __attribute__((aligned(4)))   IAPDataFlashInfo_t imgFlag;
    /* Read the first block */
    EEPROM_READ(IAP_FLAG_DATAFLASH_ADD, (PUINT32) &imgFlag, 4);
    if (imgFlag.ImageFlag != new_flag)
    {
        /* Erase the first piece */
        EEPROM_ERASE(IAP_FLAG_DATAFLASH_ADD, EEPROM_PAGE_SIZE);

        /* Update Image Information */
        imgFlag.ImageFlag = new_flag;

        /* Programming DataFlash */
        EEPROM_WRITE(IAP_FLAG_DATAFLASH_ADD, (PUINT32) &imgFlag, 4);
    }
}

/* ***************************************************************************
* @fn jumpToIap
*
* @brief Jump to IAP
*
* @return none */
void jumpToIap(void)
{
    uint32_t irq_status;
    SwitchImageFlag(FLAG_USER_CALL_IAP);// If IAP uses key detection, comment out the sentence
    SYS_DisableAllIrq(&irq_status);
    SYS_ResetExecute();
}
