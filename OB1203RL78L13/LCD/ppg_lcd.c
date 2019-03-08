/*******************************************************************************
 * DISCLAIMER
 * This software is supplied by Renesas Electronics Corporation and is only
 * intended for use with Renesas products. No other uses are authorized. This
 * software is owned by Renesas Electronics Corporation and is protected under
 * all applicable laws, including copyright laws.
 * THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
 * THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT
 * LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.
 * TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS
 * ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE
 * FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR
 * ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE
 * BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * Renesas reserves the right, without notice, to make changes to this software
 * and to discontinue the availability of this software. By using this software,
 * you agree to the additional terms and conditions found by accessing the
 * following link:
 * http://www.renesas.com/disclaimer
 * Copyright (C) 2019 Renesas Electronics Corporation. All rights reserved.
 ******************************************************************************/
/*******************************************************************************
 * File Name    : ppg_lcd.c
 * Version      : 1.0
 * Description  : This module displays information on the RL78/L12 LCD screen
 ******************************************************************************/
/*******************************************************************************
 * History : DD.MM.YYYY Version  Description
 *         : 15.01.2007 1.00     First Release
 ******************************************************************************/

/*******************************************************************************
 Includes   <System Includes> , "Project Includes"
 ******************************************************************************/
#include "r_cg_macrodriver.h"
#include "r_cg_lcd.h"

#include "r_cg_userdefine.h"

#include "lcd_panel.h"
/*******************************************************************************
 Macro definitions
 ******************************************************************************/

/*******************************************************************************
 Typedef definitions
 ******************************************************************************/

/*******************************************************************************
 Exported global variables (to be accessed by other files)
 ******************************************************************************/

/*******************************************************************************
 Private global variables and functions
 ******************************************************************************/

MD_STATUS R_PPG_LCD_Display_HRM(uint16_t rate)
{
	if (rate > 999)
	{
		return MD_ARGERROR;
	}

	/* Buffer to store the ADC result to be shown on the LCD panel */
	char display_buffer[5] = "00.0";

	/* Turn off the degrees celsius symbol */
	Symbol_Map(LCD_HEART_ON);

    /* Prepare the temperature data */
    display_buffer[0] = (uint8_t)(ASCII_0 + ((rate / DIV_100) % MODULUS_1000));
    display_buffer[1] = (uint8_t)(ASCII_0 + ((rate / DIV_10) % MODULUS_10));
    display_buffer[2] = ' ';
    display_buffer[3] = (uint8_t)(ASCII_0 + (rate % MODULUS_10));

    Display_Panel_String(PANEL_LCD_LINE3, display_buffer);
    Display_Panel_String(PANEL_LCD_LINE1, " BPM");

    return MD_OK;
}

MD_STATUS R_PPG_LCD_Display_SPO2(uint16_t percentX10)
{
	if (percentX10 > 1000)
	{
		return MD_ARGERROR;
	}

	/* Buffer to store the ADC result to be shown on the LCD panel */
	char display_buffer[5] = "00.0";

	/* Turn off the degrees celsius symbol */
	Symbol_Map(LCD_HEART_OFF);

    /* Prepare the temperature data */
    display_buffer[0] = (uint8_t)(ASCII_0 + ((percentX10 / DIV_100) % MODULUS_1000));
    display_buffer[1] = (uint8_t)(ASCII_0 + ((percentX10 / DIV_10) % MODULUS_10));
    display_buffer[2] = '.';
    display_buffer[3] = (uint8_t)(ASCII_0 + (percentX10 % MODULUS_10));

    Display_Panel_String(PANEL_LCD_LINE3, display_buffer);
    Display_Panel_String(PANEL_LCD_LINE1, "SpO2");

    return MD_OK;
}

MD_STATUS R_PPG_LCD_Display_Battery(uint8_t percent)
{
	if (percent > 100)
	{
		return MD_ARGERROR;
	}

	if(percent > 75)
	{
		SECTD_Glyph_Map(BATT_LEVEL_4, BATTERY_OUTLINE_ON);
	}
	else if(percent > 50)
	{
		/* Turn on 1 battery bars */
		SECTD_Glyph_Map(BATT_LEVEL_3, BATTERY_OUTLINE_ON);
	}
	else if(percent > 25)
	{
		/* Turn on 2 battery bars */
		SECTD_Glyph_Map(BATT_LEVEL_2, BATTERY_OUTLINE_ON);
	}
	else if(percent > 5)
	{
		/* Turn on 1 battery bars */
		SECTD_Glyph_Map(BATT_LEVEL_1, BATTERY_OUTLINE_ON);
	}
	else
	{
		SECTD_Glyph_Map(BATT_LEVEL_0, BATTERY_OUTLINE_ON);
	}

	return MD_OK;
}
