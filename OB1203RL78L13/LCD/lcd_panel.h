/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only 
* intended for use with Renesas products. No other uses are authorized. This 
* software is owned by Renesas Electronics Corporation and is protected under 
* all applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING 
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT 
* LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE 
* AND NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.
* TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS 
* ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE 
* FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR 
* ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE 
* BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software 
* and to discontinue the availability of this software.  By using this software, 
* you agree to the additional terms and conditions found by accessing the 
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2012 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/

/***********************************************************************************************************************
* File Name    : lcd_panel.h
* Version      : 1.00
* Device(s)    : R5F10WMG
* Tool-Chain   : GNURL78 V13.02
* Description  : This file contains functions and definitions used in the lcd_panel.c file.
* Creation Date: 08/10/2013
***********************************************************************************************************************/

#ifndef LCD_PANEL_H
#define LCD_PANEL_H

/***********************************************************************************************************************
Includes <System Includes> , �Project Includes
***********************************************************************************************************************/

/***********************************************************************************************************************
Macro Definitions
***********************************************************************************************************************/
/* Set to ensure base delay */
#define PANEL_DELAY_TIMING     0x05

/*  */
#define ENABLE_SW2_SW3         0u
#define DISABLE_SW2_SW3        1u
#define DISABLE_UART           (0u)
#define ENABLE_UART            (1u)
#define JUMPER_SET_SEG         0x60u

/* Unsupported Segments can be Controlled by the RSK when the unconnected 
   pins are connected to the RSK. Set the correct pin addresses to the 
   #defines below. */
#define Aux_pin5               0xFFFFu
#define Aux_pin6               0xFFFFu
#define Aux_pin47              0xFFFFu
#define Aux_pin48              0xFFFFu

/* Use the ROM area to indicate that a data pointer is not to be used */
#define PDL_NO_PTR            (uint8_t * )0xFFFFu

/* Define Section Names */
#define EMPTY                  0
#define SECT1                 '1'
#define SECT2                 '2'
#define SECT3                 '3'
#define SECT4                 '4'
#define CLEAR                  0x23
#define ISOTEST                0x2A

#define PANEL_LCD_LINE1       '3'
#define PANEL_LCD_LINE2       '1'
#define PANEL_LCD_LINE3       '2'
#define PANEL_LCD_SYMBOL      '4'

/* Define Symbol Names */
#define LCD_HEAT_ON            0x3131u
#define LCD_HEAT_OFF           0x3130u
#define LCD_FAN_ON             0x3231u
#define LCD_FAN_OFF            0x3230u
#define LCD_ZONE_ON            0x3331u
#define LCD_ZONE_OFF           0x3330u
#define LCD_MGML_ON            0x3431u
#define LCD_MGML_OFF           0x3430u
#define LCD_MMHG_ON            0x3531u
#define LCD_MMHG_OFF           0x3530u
#define LCD_VOLTS_ON           0x3631u
#define LCD_VOLTS_OFF          0x3630u
#define LCD_HEART_ON           0x3731u
#define LCD_HEART_OFF          0x3730u
#define LCD_DEGREESC_ON        0x3831u
#define LCD_DEGREESC_OFF       0x3830u
#define LCD_DEGREESF_ON        0x3931u
#define LCD_DEGREESF_OFF       0x3930u
#define LCD_ALARM_ON           0x4131u
#define LCD_ALARM_OFF          0x4130u
#define LCD_ALARM_ON_1         0x6131u
#define LCD_ALARM_OFF_1        0x6130u
#define LCD_AM_ON              0x4231u
#define LCD_AM_OFF             0x4230u
#define LCD_AM_ON_1            0x6231u
#define LCD_AM_OFF_1           0x6230u
#define LCD_PM_ON              0x4331u
#define LCD_PM_OFF             0x4330u
#define LCD_PM_ON_1            0x6331u
#define LCD_PM_OFF_1           0x6330u
#define LCD_R_LOGO_ON          0x4431u
#define LCD_R_LOGO_OFF         0x4430u
#define LCD_R_LOGO_ON_1        0x6431u
#define LCD_R_LOGO_OFF_1       0x6430u

/* Battery levels */
#define BATT_LEVEL_0           0u
#define BATT_LEVEL_1           1u
#define BATT_LEVEL_2           2u
#define BATT_LEVEL_3           3u
#define BATT_LEVEL_4           4u
    
#define BATTERY_OUTLINE_ON     1u
#define BATTERY_OUTLINE_OFF    0u
/***********************************************************************************************************************
Type definitions
***********************************************************************************************************************/
/* Define Map Structure Used */
typedef union LCD_MAP
{    
    int32_t WORD;
    
    struct
    {
        int8_t ONE   : 4;
        int8_t TWO   : 4;
        int8_t THREE : 4;
        int8_t FOUR  : 4;
    } BYTE;
} LCDMAP;

/***********************************************************************************************************************
Function Prototypes
***********************************************************************************************************************/
/* Clears the screen by the next LCD frame duration */
void Clear_Display (void);

/* Prepares the LCD Panel for use */
void Init_Display_Panel (void);

/* Disables the LCD Peripheral */
void Power_Off_Display(void);

/* Maps the character to a segment pattern, 
   to be displayed on Section A of the LCD panel */
void SECTA_Glyph_Map (const uint8_t glyph, const uint16_t digit);

/* Maps the character to a segment pattern, 
   to be displayed on Section B of the LCD panel */
void SECTB_Glyph_Map (const uint8_t glyph, const uint16_t digit);

/* Maps the character to a segment pattern, 
   to be displayed on Section C of the LCD panel */
void SECTC_Glyph_Map (const uint8_t glyph, const uint16_t digit);

/* Maps the character to a segment pattern, 
   to be displayed on Section D of the LCD panel */
void SECTD_Glyph_Map (const uint8_t  level);

/* Maps the character to a segment pattern, 
   to be displayed on Section E of the LCD panel */
void SECTE_Glyph_Map (const uint16_t  level);

/* Maps the character to a segment pattern, 
   to be displayed on Section F of the LCD panel */
void SECTF_Glyph_Map (const uint8_t  day);

/* Turns on/off LCD Panel Symbols */
int8_t Symbol_Map (const uint16_t  input);

/* Initialises the glyph-segment maps used to display letters, 
   symbols and numbers */
void Init_Maps (void);

/* Validates the input for the Update_Display */
int16_t Validate_Input (const uint8_t input, uint8_t * const output);

/* Turn on RENESAS logo */
void set_logo (void);

/* Switch ON various prats of the LCD Panel */
void LCD_ON (void);

/* Lights up all the segments of the LCD Panel */
void LCD_DISPLAY_ON (void);


/* Switches OFF all the segments of the LCD Panel */
void LCD_DISPLAY_OFF (void);

/* Display string on LCD panel */
void Display_Panel_String (const uint8_t position, const char * const string);

/* Delay routine for LCD */
void Display_Panel_Delay (const uint32_t units);

/***********************************************************************************************************************
Segment Re-definitions
***********************************************************************************************************************/
#define     SEG0_DEF    SEG0
#define     SEG1_DEF    SEG1
#define     SEG2_DEF    SEG2
#define     SEG3_DEF    SEG3
#define     SEG4_DEF    SEG4
#define     SEG5_DEF    SEG5
#define     SEG6_DEF    SEG6
#define     SEG7_DEF    SEG7
#define     SEG8_DEF    SEG8
#define     SEG9_DEF    SEG9
#define     SEG10_DEF   SEG10
#define     SEG11_DEF   SEG11
#define     SEG12_DEF   SEG12
#define     SEG13_DEF   SEG13
#define     SEG14_DEF   SEG14
#define     SEG15_DEF   SEG15
#define     SEG16_DEF   SEG16
#define     SEG17_DEF   SEG17
#define     SEG18_DEF   SEG18
#define     SEG19_DEF   SEG19
#define     SEG20_DEF   SEG20
#define     SEG21_DEF   SEG21
#define     SEG22_DEF   SEG22
#define     SEG23_DEF   SEG23
#define     SEG24_DEF   SEG24
#define     SEG25_DEF   SEG25
#define     SEG26_DEF   SEG26
#define     SEG27_DEF   SEG27
#define     SEG28_DEF   SEG28
#define     SEG29_DEF   SEG29
#define     SEG30_DEF   SEG30
#define     SEG31_DEF   SEG31
#define     SEG32_DEF   SEG32
#define     SEG33_DEF   SEG33
#define     SEG34_DEF   SEG34
#define     SEG35_DEF   SEG35
#define     SEG36_DEF   SEG36
#define     SEG37_DEF   SEG37
#define     SEG38_DEF   SEG38
#define     SEG39_DEF   SEG39

#endif
