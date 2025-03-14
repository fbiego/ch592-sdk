/********************************** (C) COPYRIGHT *******************************
 * File Name          : CH59x_lcd.h
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2022/12/05
 * Description        : 
 ********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/


#ifndef __CH59x_LCD_H__
#define __CH59x_LCD_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include "CH592SFR.h"

/**
  * @brief  Configuration LCD driver power
  */
typedef enum
{
	LCD_PS_3V3 = 0,					// 3.3V driver
	LCD_PS_2V5,						// 2.5V driver
}LCDDrvPowerTypeDef; 

/**
  * @brief  Configuration LCD bias
  */
typedef enum
{
	LCD_1_2_Bias = 0,				// 2-stage pressure
	LCD_1_3_Bias,					// 3-stage pressure
}LCDBiasTypeDef;

/**
  * @brief  Configuration LCD duty
  */
typedef enum
{
	LCD_1_2_Duty = 0,				// COM0-COM1
	LCD_1_3_Duty,					// COM0-COM2
	LCD_1_4_Duty,					// COM0-COM3
}LCDDutyTypeDef;

/**
  * @brief  Configuration LCD scan clk
  */
typedef enum
{
	LCD_CLK_256 = 0,				// 256Hz
	LCD_CLK_512,					// 512Hz
	LCD_CLK_1000,					// 1KHz
	LCD_CLK_128						// 128Hz
}LCDSCANCLKTypeDef;
	 
/* LCD segment screen driver initialization configuration */
void LCD_Init(LCDDutyTypeDef duty, LCDBiasTypeDef bias);

#define	LCD_PowerDown()			(R32_LCD_CMD &= ~(RB_LCD_ON | RB_LCD_SYS_EN))		/* LCD function module is turned off */
#define	LCD_PowerOn()			(R32_LCD_CMD |= (RB_LCD_ON | RB_LCD_SYS_EN))		/* LCD function module is enabled */

// Input value reference LCDDrvPowerTypeDef
#define LCD_PowerCfg( d )		(R32_LCD_CMD = (R32_LCD_CMD & ~RB_LCD_VLCD_SEL) | (d<<7))			/* Configure the power supply voltage selection of LCD */
// Input value reference LCDSCANCLKTypeDef
#define LCD_ScanCLKCfg( d )		(R32_LCD_CMD = (R32_LCD_CMD & ~RB_LCD_SCAN_CLK) | (d<<5))			/* Configure the scan clock selection of LCD */
// Input value reference LCDDutyTypeDef
#define LCD_DutyCfg( d )		(R32_LCD_CMD = (R32_LCD_CMD & ~RB_LCD_DUTY) | (d<<3))				/* Configure the duty selection of LCD */
// Input value reference LCDBiasTypeDef
#define LCD_BiasCfg( d )		(R32_LCD_CMD = (R32_LCD_CMD & ~RB_LCD_BIAS) | (d<<2))				/* Configure the bias selection of LCD */
	 
#define LCD_WriteData0( d )		(R32_LCD_RAM0 = (R32_LCD_RAM0 & 0xffffff00) | ((UINT32)d))			/* Fill in SEG0 driver value */
#define LCD_WriteData1( d )		(R32_LCD_RAM0 = (R32_LCD_RAM0 & 0xffff00ff) | ((UINT32)d<<8))		/* Fill in SEG1 driver value */
#define LCD_WriteData2( d )		(R32_LCD_RAM0 = (R32_LCD_RAM0 & 0xff00ffff) | ((UINT32)d<<16))		/* Fill in SEG2 driver values */
#define LCD_WriteData3( d )		(R32_LCD_RAM0 = (R32_LCD_RAM0 & 0x00ffffff) | ((UINT32)d<<24))		/* Fill in SEG3 driver values */
	 
#define LCD_WriteData4( d )		(R32_LCD_RAM1 = (R32_LCD_RAM1 & 0xffffff00) | ((UINT32)d))			/* Fill in SEG4 driver values */
#define LCD_WriteData5( d )		(R32_LCD_RAM1 = (R32_LCD_RAM1 & 0xffff00ff) | ((UINT32)d<<8))		/* Fill in SEG5 driver values */
#define LCD_WriteData6( d )		(R32_LCD_RAM1 = (R32_LCD_RAM1 & 0xff00ffff) | ((UINT32)d<<16))		/* Fill in SEG6 driver values */
#define LCD_WriteData7( d )		(R32_LCD_RAM1 = (R32_LCD_RAM1 & 0x00ffffff) | ((UINT32)d<<24))		/* Fill in SEG7 driver values */
	 
#define LCD_WriteData8( d )		(R32_LCD_RAM2 = (R32_LCD_RAM2 & 0xffffff00) | ((UINT32)d))			/* Fill in SEG8 driver value */
#define LCD_WriteData9( d )		(R32_LCD_RAM2 = (R32_LCD_RAM2 & 0xffff00ff) | ((UINT32)d<<8))		/* Fill in SEG9 driver values */
#define LCD_WriteData10( d )	(R32_LCD_RAM2 = (R32_LCD_RAM2 & 0xff00ffff) | ((UINT32)d<<16))		/* Fill in SEG10 driver value */



#ifdef __cplusplus
}
#endif

#endif  // __CH59x_LCD_H__	

