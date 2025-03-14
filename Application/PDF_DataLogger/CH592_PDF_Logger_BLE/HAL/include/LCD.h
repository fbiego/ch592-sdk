/********************************** (C) COPYRIGHT *******************************
 * File Name          : LCD.h
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2023/09/01
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/******************************************************************************/

#ifndef __LCD_H_
#define __LCD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "CH59x_lcd.h"

/**************************************************************************************************
 *                                              MACROS
 **************************************************************************************************/
#define LCD_WriteByte0( d )     (R32_LCD_RAM0 = (R32_LCD_RAM0 & 0xffffff00) | ((UINT32)(d)))          /* Fill in SEG0, SEG1 driver value */
#define LCD_WriteByte1( d )     (R32_LCD_RAM0 = (R32_LCD_RAM0 & 0xffff00ff) | ((UINT32)(d)<<8))       /* Fill in SEG2 and SEG3 driver values */
#define LCD_WriteByte2( d )     (R32_LCD_RAM0 = (R32_LCD_RAM0 & 0xff00ffff) | ((UINT32)(d)<<16))      /* Fill in SEG4 and SEG5 driver values */
#define LCD_WriteByte3( d )     (R32_LCD_RAM0 = (R32_LCD_RAM0 & 0x00ffffff) | ((UINT32)(d)<<24))      /* Fill in SEG6 and SEG7 driver values */
#define LCD_WriteByte4( d )     (R32_LCD_RAM1 = (R32_LCD_RAM1 & 0xffffff00) | ((UINT32)(d)))          /* Fill in SEG8 and SEG9 driver values */
#define LCD_WriteByte5( d )     (R32_LCD_RAM1 = (R32_LCD_RAM1 & 0xffff00ff) | ((UINT32)(d)<<8))       /* Fill in SEG10, SEG11 driver values */
#define LCD_WriteByte6( d )     (R32_LCD_RAM1 = (R32_LCD_RAM1 & 0xff00ffff) | ((UINT32)(d)<<16))      /* Fill in SEG12 and SEG13 driver values */
#define LCD_WriteByte7( d )     (R32_LCD_RAM1 = (R32_LCD_RAM1 & 0x00ffffff) | ((UINT32)(d)<<24))      /* Fill in SEG14 and SEG15 driver values */
#define LCD_WriteByte8( d )     (R32_LCD_RAM2 = (R32_LCD_RAM2 & 0xffffff00) | ((UINT32)(d)))          /* Fill in SEG16, SEG17 driver values */
#define LCD_WriteByte9( d )     (R32_LCD_RAM2 = (R32_LCD_RAM2 & 0xffff00ff) | ((UINT32)(d)<<8))       /* Fill in SEG18 and SEG19 driver values */

#define LCD_WriteSeg16( d )     (R32_LCD_RAM2 = (R32_LCD_RAM2 & 0xfffffff0) | ((UINT32)(d)))          /* Fill in SEG16 driver value */
#define LCD_WriteSeg17( d )     (R32_LCD_RAM2 = (R32_LCD_RAM2 & 0xffffff0f) | ((UINT32)(d)<<4))       /* Fill in SEG17 driver value */

/**
  * @brief  LCD display battery voltage
  */
typedef enum
{
    BAT_VOL_0 = 0,          //no power
    BAT_VOL_1,              //1 bar power
    BAT_VOL_2,              //2 bar power
    BAT_VOL_3               //3 bar power
}LCD_BatteryVoltageTypeDef;

/**************************************************************************************************
 *                                             GLOBAL VARIABLES
 **************************************************************************************************/
/* currently displayed value flag, 0:humidity, 1:temperature*/
extern uint8_t LCD_DisplayFlg;

/*********************************************************************
 * FUNCTIONS
 */

/**
 * @brief   Clear the screen of the LCD
 */
void HAL_LCD_ClearScreen(void);

/**
 * @brief   Clear the numbers displayed on the LCD
 */
void HAL_LCD_ClearNumber(void);

/**
 * @brief   Initialize LCD
 */
void HAL_LCD_Init(LCDDutyTypeDef duty, LCDBiasTypeDef bias);

/**
 * @brief   Display humidity data on LCD
 */
void HAL_LCD_ShowHumidity(float humidity);

/**
 * @brief   Display temperature data on LCD
 */
void HAL_LCD_ShowTemperature(float temperature);

/**
 * @brief   Display battery voltage on LCD
 */
void HAL_LCD_ShowBatteryVoltage(LCD_BatteryVoltageTypeDef vol);


/********************************************************************************
********************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* __LCD_H_ */
