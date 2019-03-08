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
* File Name    : lcd_panel.c
* Version      : 1.0
* Device(s)    : R5F10WMG
* Tool-Chain   : GNURL78 V13.02
* Description  : This file contains the lcd panel driver.
* Creation Date: 08/10/2013
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes <System Includes> , �Project Includes
***********************************************************************************************************************/
/* Contains string functions used in this file */
#include "r_cg_macrodriver.h"
#include "r_cg_lcd.h"

/* Header file contains declarations of functions defined in this file */
#include "lcd_panel.h"
#include "r_cg_port.h"

/***********************************************************************************************************************
Global Variables & Defined Constants 
***********************************************************************************************************************/
/* Declare Array Maps */
LCDMAP SECTA_map[20];
LCDMAP SECTB_map[20];
LCDMAP SECTC_map[41];

/***********************************************************************************************************************
User Program Code
***********************************************************************************************************************/
/***********************************************************************************************************************
* Function Name : Init_Display_Panel
* Description   : Calls functions in order to prepare the LCD Panel for use
* Arguments     : None
* Return Value  : None
***********************************************************************************************************************/
void Init_Display_Panel (void)
{    
    /* Load up Segment Maps */
    Init_Maps();
     
    /* Clear LCD Data Registers */
    Clear_Display();
    
    /* Make sure that SEG 48 is not used as this is used for LED 0 */
    PFSEG6 &= (uint8_t)~_02_PFSEG48_SEG;

    /* Make sure that IVERF0 is not used as this is used for LED 1 */
    PFSEG3 &= (uint8_t)~_04_PFDEG_DEFAULT;

    /* Enable the LCD */
    R_LCD_Start();
    
    /* Enable the voltage boost circuit */
    R_LCD_Voltage_On();
    
}
/***********************************************************************************************************************
End of function Init_Display_Panel
***********************************************************************************************************************/


/***********************************************************************************************************************
* Function Name : Validate_Input
* Description   : Takes any char and validates the input for the Update_Display 
*                 function. Capitalises all letters, and remaps symbols. Outputs
*                 to a variable specified by the pointer parameter passed.
* Arguments     : uint8_t input
*                         to be validated
*                 uint8_t * output
*                         pointer to output buffer
* Return Value  : 0: Unsupported Character
*                 1: Number (or space)
*                 2: Letter A-F
*                 3: Letter G-Z
*                 4: Supported Symbol
***********************************************************************************************************************/
int16_t Validate_Input (const uint8_t input, uint8_t * const output)
{
    /* Is space or Carriage Return? */
    if ((input == ' ') || (input == 0x0D))
    {
        *output = 0x29U;
        return 1;
    }

    /* Is Number? */
    else if ((input < 0x3AU) && (input > 0x2FU))
    {
        /* Convert from ASCII */
        *output = (uint8_t)(input - 0x30U);
        return 1;
    }
    
    /* Is Uppercase Alpha A-F ? */
    else if ((input < 0x47U) && (input > 0x40U))
    {
        /* Convert from ASCII */
        *output = (uint8_t)(input - 0x33U);
        return 2;        
    }
    
    /* Is Lowercase Alpha A-F */
    else if ((input < 0x67) && (input > 0x60))
    {
        /* Shift Case & Convert from ASCII */
        *output = (uint8_t)(input - 0x53);
        return 2;
    }
    
    /* Is Uppercase Alpha G-Z ? */
    else if ((input > 0x46) && (input < 0x5B))
    {
        /* Convert from ASCII */
        *output = (uint8_t)(input - 0x33);
        return 3;
    }
    
    /* Is Lowercase Alpha G-Z ? */
    else if ((input > 0x66) && (input < 0x7B))
    {
        /* Shift Case & Convert from ASCII */
        *output = (uint8_t)(input - 0x53);
        return 3;
    }
    
    /* Is Symbol? */
    else
    {
        /* Check input against supported symbols */
        switch (input)
        {
            /* Is Plus */
            case 0x2B:
            {
                *output = 0x0A;
            }
            break;
            
            /* Is Minus */
            case 0x2D:
            {
                *output = 0x0B;
            }
            break;
            
            /* Is Forward Slash */
            case 0x2F:
            {
                *output = 0x0C;
            }
            break;
            
            /* Is Back Slash */
            case 0x5C:
            {
                *output = 0x0D;
            }
            break;
            
            /* Is full stop */
            case 0x2E:
            {
                *output = 0x2E;
            }
            break;
            
            /* Is colon */
            case 0x3A:
            {
                *output = 0x3A;
            }
            break;
            
            case 0x2A:
            {
                *output = 0x28;
            }
            break;

            /* Unsupported Character */
            default:
            {
                return 0;
            }
        }
        return 4;
    }
}
/***********************************************************************************************************************
End of function Validate_Input
***********************************************************************************************************************/

/***********************************************************************************************************************
* Function Name: SECTA_Glyph_Map
* Description  : Takes a validated char input and maps the character to a 
*                segment pattern, to be displayed on Section A of the LCD panel.
* Arguments    : character to be displayed, digit position of character
* Return Value : none
***********************************************************************************************************************/
void SECTA_Glyph_Map (const uint8_t glyph, const uint16_t digit)
{
    volatile uint8_t * PinPtr1 = PDL_NO_PTR;
    volatile uint8_t * PinPtr2 = PDL_NO_PTR;
    
    /* Set Pointers to Correct Segments */
    switch (digit)
    {
        /* First Digit */
        case 0:
        {
            PinPtr1 = &SEG11_DEF;
            PinPtr2 = &SEG10_DEF;
        }
        break;
        
        /* Second Digit */
        case 1:
        {
            PinPtr1 = &SEG13_DEF;
            PinPtr2 = &SEG12_DEF;
        }    
        break;
        
        /* Decimal Point */
        case 2:
        {
            PinPtr1 = &SEG14_DEF;
            PinPtr2 = PDL_NO_PTR;
        }
        break;

        /* Third Digit */
        case 3:
        {
            PinPtr1 = &SEG15_DEF;
            PinPtr2 = &SEG14_DEF;
        }    
        break;
        
        default:
        {
            /* nothing */
        }    
        break;
    }
        
    /* Bit Mask Segments */
    *PinPtr1 &= 0xF0U;
    *PinPtr2 &= 0xF8U;
    
    /* decimal point? */
    if (digit == 2)
    {
        if (glyph == '.')
        {
            /* Turn on Decimal Point */
            *PinPtr1 |= 0x08U;
        }
        else
        {
            /* Turn off Decimal Point */
            *PinPtr1 &= 0xF7U;
        }
    }
    else if (glyph != 0x29)
    { 
        /* Digit-Segment Mapping */
        *PinPtr1 |= (uint8_t)SECTA_map[glyph].BYTE.TWO;
        *PinPtr2 |= (uint8_t)SECTA_map[glyph].BYTE.ONE;
    }
    else
    {
        /* Do nothing */
    }

}
/***********************************************************************************************************************
End of function SECTA_Glyph_Map
***********************************************************************************************************************/
/***********************************************************************************************************************
* Function Name : SECTB_Glyph_Map
* Description   : Takes a validated char input and maps the character to a 
*                 segment pattern, to be displayed on Section B of the LCD panel.
* Arguments     : character to be displayed, digit position of character
* Return Value  : none
***********************************************************************************************************************/
void SECTB_Glyph_Map (const uint8_t glyph, const uint16_t digit)
{
    /* Declare Pointers */
    volatile uint8_t * PinPtr1 = PDL_NO_PTR;
    volatile uint8_t * PinPtr2 = PDL_NO_PTR;
    
    /* Set Pointers to Current Digit Pins */
    switch (digit)
    {
        /* First Digit */    
        case 0:
        {
            PinPtr1 = &SEG28_DEF;
            PinPtr2 = &SEG29_DEF;
        }
        break;

        /* Second Digit */        
        case 1:
        {
            PinPtr1 = &SEG26_DEF;
            PinPtr2 = &SEG27_DEF;
        }    
        break;
        
        /* Third Digit */
        case 2:
        {
            PinPtr1 = &SEG24_DEF;
            PinPtr2 = &SEG25_DEF;
        }    
        break;
        
        /* Colon/Decimal Point */
        case 3:
        {
            PinPtr1 = &SEG23_DEF;
            PinPtr2 = PDL_NO_PTR;
        }    
        break;
        
        /* Fourth Digit */        
        case 4:
        {
            PinPtr1 = &SEG21_DEF;
            PinPtr2 = &SEG22_DEF;
        }    
        break;

        /* Fifth Digit */        
        case 5:
        {
            PinPtr1 = &SEG19_DEF;
            PinPtr2 = &SEG20_DEF;
        }    
        break;
        
        /* AM/PM Indicator */
        case 6:
        {
            /* Nullify Pointers */
            PinPtr1 = PDL_NO_PTR;
            PinPtr2 = PDL_NO_PTR;
            
            /* If last digit is A */
            if (glyph == 0x0E)
            {
               /* AM Indicator On */
                SEG18_DEF |= 0x01U;
                
                /* PM Indicator Off */
                SEG16_DEF &= 0xFDU;
            }

            /* If last digit is P */
            else if (glyph == 0x1DU)
            {
                /* AM Indicator Off */
                SEG18_DEF &= 0xFEU;
                
                /* PM Indicator On */
                SEG16_DEF |= 0x02U;
            }
            
            /* If last digit is other */
            else
            {
                /* AM Indicator Off */
                SEG18_DEF &= 0xFEU;
                
                /* PM Indicator Off */
                SEG16_DEF &= 0xFDU;
            }
        }    
        break;
        
        /* Handles undefined cases */
        default:
        {
            /* Do nothing */
        }
        break;
    }

    /* Bit Mask Segments */
    *PinPtr1 &= 0xF0U;
    *PinPtr2 &= 0xF1U;
        
    /* Is colon/decimal point ? */
    if (digit == 3)
    {    
        /* Is Colon? */
        if (glyph == ':')
        {
            /* Turn on Colon */
            *PinPtr1 |= 0x02U;
        }

        /* Is Decimal Point? */
        else if (glyph == '.')
        {
            /* Turn off Colon */
            *PinPtr1 &= 0xFDU;
        }
        
        /* Is neither? */
        else
        {
           /* Turn off Decimal Point & Colon */
            *PinPtr1 &= 0xFCU;
        }
    }
    else if (glyph != 0x29)
    {
        /* Map Glyph to Segments */
        *PinPtr1 |= (uint8_t)SECTB_map[glyph].BYTE.TWO;
        *PinPtr2 |= (uint8_t)SECTB_map[glyph].BYTE.ONE;
    }
    else
    {
        /* Do nothing */
    }
}
/***********************************************************************************************************************
End of function SECTB_Glyph_Map
***********************************************************************************************************************/

/***********************************************************************************************************************
* Function Name : SECTC_Glyph_Map
* Description   : Takes a validated char input and maps the character to a 
*                 segment pattern, to be displayed on Section C of the LCD panel.
* Arguments     : uint8_t glyph 
*                            - character to be displayed, 
                  uint16_t digit 
*                            - position of character
* Return Value  : none
***********************************************************************************************************************/
void SECTC_Glyph_Map (const uint8_t glyph, const uint16_t digit)
{
    /* Declare Pin Address Pointers */
    volatile uint8_t * PinPtr1 = PDL_NO_PTR;
    volatile uint8_t * PinPtr2 = PDL_NO_PTR;
    volatile uint8_t * PinPtr3 = PDL_NO_PTR;
    volatile uint8_t * PinPtr4 = PDL_NO_PTR;
    
    /* Set Pointers to Current Digit Pin Addresses */
    switch (digit)
    {    
        /* First Digit (Specify Aux pins in order to use) */
        case 0:
        {
            PinPtr1 = (uint8_t *)Aux_pin5;
            PinPtr2 = (uint8_t *)Aux_pin6;
            PinPtr3 = (uint8_t *)Aux_pin47;
            PinPtr4 = (uint8_t *)Aux_pin48;
        }
        break;

        /* Second Digit */    
        case 1:
        {
            PinPtr1 = &SEG0_DEF;
            PinPtr2 = &SEG1_DEF;
            PinPtr3 = &SEG38_DEF;
        }
        break;
        
        /* Third Digit */    
        case 2:
        {
            PinPtr1 = &SEG2_DEF;
            PinPtr2 = &SEG3_DEF;
            PinPtr3 = &SEG36_DEF;
            PinPtr4 = &SEG37_DEF;
        }
        break;
        
        /* Four Digit */    
        case 3:
        {
            PinPtr1 = &SEG4_DEF;
            PinPtr2 = &SEG5_DEF;
            PinPtr3 = &SEG34_DEF;
            PinPtr4 = &SEG35_DEF;
        }
        break;
        
        /* Fifth Digit */
        case 4:
        {
            PinPtr1 = &SEG6_DEF;
            PinPtr2 = &SEG7_DEF;
            PinPtr3 = &SEG32_DEF;
            PinPtr4 = &SEG33_DEF;
        }
        break;
        
        /* Sixth Digit */
        case 5:
        {
            PinPtr1 = &SEG8_DEF;
            PinPtr2 = &SEG9_DEF;
            PinPtr3 = &SEG30_DEF;
            PinPtr4 = &SEG31_DEF;
        }
        break;
        
        /* LCD display only has 6 alpha-numeric segments */
        default:
        {
            /* Bad Selection */
        }
        break;
    }
    
    /* Bit Mask All Segments */
    *PinPtr1 &= 0xF0U;
    *PinPtr2 &= 0xF0U;
    *PinPtr3 &= 0xF1U;
    *PinPtr4 &= 0xF0U;
    
    /* Check if glyph is not 0x29 */
    if (glyph != 0x29)   /* LA8A is 0x28 */
    {
        /* Map Glyph To Segments */
        *PinPtr4 |= (uint8_t)SECTC_map[glyph].BYTE.ONE;
        *PinPtr3 |= (uint8_t)SECTC_map[glyph].BYTE.TWO;
        *PinPtr2 |= (uint8_t)SECTC_map[glyph].BYTE.THREE;
        *PinPtr1 |= (uint8_t)SECTC_map[glyph].BYTE.FOUR;
    }
}
/***********************************************************************************************************************
End of function SECTC_Glyph_Map
***********************************************************************************************************************/


/***********************************************************************************************************************
Function Name : SECTD_Glyph_Map
Description   : Takes a number input, and sets the level of the battery indicator (section D)
Argument      : uint8_t level 
*                  - Indicator Level (char)
Return Values : None
***********************************************************************************************************************/
void SECTD_Glyph_Map (const uint8_t level)
{
    /* Switch to segment option */
    switch (level)
    {    
        /* Battery Outline On */    
        case 0:
        {
            SEG17_DEF &= 0xF0U;
            SEG16_DEF |= 0x01U;
        }
        break;
        
        /* Battery Outline & 1 Level */
        case 1:
        {
            SEG17_DEF &= 0xF0U;
            SEG17_DEF |= 0x08U;
            SEG16_DEF |= 0x01U;
        }
        break; 
        
        /* Battery Outline & 2 Levels */
        case 2:
        {
            SEG17_DEF &= 0xF0U;
            SEG17_DEF |= 0x0CU;
            SEG16_DEF |= 0x01U;
        }
        break;
        
        /* Battery Outline & 3 Levels */
        case 3:
        {
            SEG17_DEF &= 0xF0U;
            SEG17_DEF |= 0x0EU;
            SEG16_DEF |= 0x01U;
        }
        break;    
        
        /* Battery Outline & All Levels */
        case 4:
        {
            SEG17_DEF |= 0x0FU;
            SEG16_DEF |= 0x01U;
        }
        break;
        
        default:
        {
            /* Bad Selection */
        }
        break;
    }
}
/***********************************************************************************************************************
End of function SECTD_Glyph_Map
***********************************************************************************************************************/

/***********************************************************************************************************************
* Function Name : SECTE_Glyph_Map
* Description    : Takes a number input, and sets the level of the bar graph (Section E)
* Argument      : int16_t Bar Graph Level
* Return Values : None
***********************************************************************************************************************/
void SECTE_Glyph_Map (const uint16_t level)
{
    /* Initialise Missing Pin Pointers */
    uint8_t * const Pin47Ptr = (uint8_t *)Aux_pin47;
    uint8_t * const Pin48Ptr = (uint8_t *)Aux_pin48;
    
    /* Switch to segment option */
    switch (level)
    {
        /* All Bars Off */
        case 0:
        {
            *Pin47Ptr &= 0xFEU;
            *Pin48Ptr &= 0xFEU;
            SEG38_DEF &= 0xFEU;
            SEG37_DEF &= 0xFEU;
            SEG36_DEF &= 0xFEU;
        }
        break;
        
        /* First Bar Only */
        case 1:
        {
            *Pin47Ptr |= 0x01U;
            *Pin48Ptr &= 0xFEU;
            SEG38_DEF &= 0xFEU;
            SEG37_DEF &= 0xFEU;
            SEG36_DEF &= 0xFEU;
        }
        break;
        
        /* Bars 1 - 2 */
        case 2:
        {
            *Pin47Ptr |= 0x01U;
            *Pin48Ptr |= 0x01U;
            SEG38_DEF &= 0xFEU;
            SEG37_DEF &= 0xFEU;
            SEG36_DEF &= 0xFEU;
        }
        break;
        
        /* Bars 1 - 3 */
        case 3:
        {
            *Pin47Ptr |= 0x01U;
            *Pin48Ptr |= 0x01U;
            SEG38_DEF &= 0xFEU;
            SEG37_DEF &= 0xFEU;
            SEG36_DEF &= 0xFEU;
        }
        break;
        
        /* Bars 1 - 4 */
        case 4:
        {
            *Pin47Ptr |= 0x01U;
            *Pin48Ptr |= 0x01U;
            SEG38_DEF |= 0x01U;
            SEG37_DEF &= 0xFEU;
            SEG36_DEF &= 0xFEU;
        }
        break;
        
        /* Bars 1 - 5 */
        case 5:
        {
            *Pin47Ptr |= 0x01U;
            *Pin48Ptr |= 0x01U;
            SEG38_DEF |= 0x01U;
            SEG37_DEF |= 0x01U;
            SEG36_DEF &= 0xFEU;
        }
        break; 
        
        /* All Levels */
        case 6:
        {
            *Pin47Ptr |= 0x01U;
            *Pin48Ptr |= 0x01U;
            SEG38_DEF |= 0x01U;
            SEG37_DEF |= 0x01U;
            SEG36_DEF |= 0x01U;
        }
        break;
        
        default:
        {
            /* Bad Selection */
        }
        break;
    }
}
/***********************************************************************************************************************
End of function SECTE_Glyph_Map
***********************************************************************************************************************/

/***********************************************************************************************************************
* Function Name : SECTF_Glyph_Map
* Description   : Sets the week calendar (Section F) based on a number input
*                 0: OFF            4: Thursday
*                 1: Monday         5: Friday    
*                 2: Tuesday        6: Saturday
*                 3: Wednesday      7: Sunday
* Arguments     : uint8_t day - character to be displayed, digit position of character
* Return Value  : None
***********************************************************************************************************************/
void SECTF_Glyph_Map (const uint8_t day)
{    
   /* Switch to segment option */
    switch (day)
    {
         /* All Days Off */
        case 0:
        {    
            SEG25_DEF &= 0x0EU;
            SEG23_DEF &= 0x03U;
            SEG22_DEF &= 0x0EU;
            SEG20_DEF &= 0x0EU;
            SEG18_DEF &= 0x03U;
        }    
        break;
        
        /* Monday Lit */
        case 1:
        {    
            SEG25_DEF &= 0x0EU;
            SEG23_DEF &= 0x03U;
            SEG23_DEF |= 0x08U;
            SEG22_DEF &= 0x0EU;
            SEG20_DEF &= 0x0EU;
            SEG18_DEF &= 0x03U;
        }    
        break;
        
        /* Tuesday Lit */
        case 2:
        {   
            SEG25_DEF &= 0x0EU;
            SEG23_DEF &= 0x03U;
            SEG23_DEF |= 0x04U;
            SEG22_DEF &= 0x0EU;
            SEG20_DEF &= 0x0EU;
            SEG18_DEF &= 0x03U;
        }   
        break;
        
        /* Wednesday Lit */
        case 3:
        {    
            SEG25_DEF &= 0x0EU;
            SEG31_DEF &= 0x03U;
            SEG22_DEF |= 0x01U;
            SEG20_DEF &= 0x0EU;
            SEG18_DEF &= 0x03U;
        }    
        break;

        /* Thursday Lit */
        case 4:
        {   
            SEG25_DEF &= 0x0EU;
            SEG23_DEF &= 0x03U;
            SEG22_DEF &= 0x0EU;
            SEG20_DEF |= 0x01U;
            SEG18_DEF &= 0x03U;
        }    
        break;
    
        /* Friday Lit */
        case 5:
        {    
            SEG25_DEF &= 0x0EU;
            SEG23_DEF &= 0x03U;
            SEG22_DEF &= 0x0EU;
            SEG20_DEF &= 0x0EU;
            SEG18_DEF &= 0x03U;
            SEG18_DEF |= 0x08U;
        }
        break;
        
        /* Saturday Lit */
        case 6:
        {    
            SEG25_DEF &= 0x0EU;
            SEG23_DEF &= 0x03U;
            SEG22_DEF &= 0x0EU;
            SEG20_DEF &= 0x0EU;
            SEG18_DEF &= 0x03U;
            SEG18_DEF |= 0x04U;
        }    
        break;
        
        /* Sunday Lit */
        case 7:
        {    
            SEG25_DEF |= 0x01U;
            SEG23_DEF &= 0x03U;
            SEG22_DEF &= 0x0EU;
            SEG20_DEF &= 0x0EU;
            SEG18_DEF &= 0x03U;
        }
        break;
        
        default:
        {
            /* Bad Selection */
        }
        break;
    }
}
/***********************************************************************************************************************
End of function SECTF_Glyph_Map
***********************************************************************************************************************/

/***********************************************************************************************************************
* Function Name : Symbol_Map
* Description   : Takes a symbol code and turns on/off LCD Panel Symbols. The 
*                 first digit should be which symbol to control (see symbols 
*                 listed below) 
*                 The second should be either a 1 or a 0. 
*                 (1 - Turn on, 0 - Turn Off)
*                 1 - Heat         5 - mmHg             9 - Degrees F
*                 2 - Fan          6 - Volts            A - Alarm
*                 3 - Zone         7 - Heart            B - AM
*                 4 - mg/ml        8 - Degrees C        C - PM
*                 For example '61' would turn on the Volts symbol and 
*                 C0 would turn off the PM symbol.
* Arguments     : int16_t input 
*                           0 - valid input
*                          -1 - invalid input
* Return Value  : int8_t
***********************************************************************************************************************/
int8_t Symbol_Map (const uint16_t input)
{
    /* Declare a status variable */
    int8_t status = 0;
    
    switch (input)
    {
        /* Heat Symbol On/Off */
        case LCD_HEAT_ON:
        {
            SEG27_DEF |= 0x01U;
        }
        break;
        
        case LCD_HEAT_OFF:
        {
            SEG27_DEF &= 0xFEU;
        }    
        break;

        /* Fan Symbol On/Off */
        case LCD_FAN_ON:
        {
            SEG33_DEF |= 0x01U;
        }    
        break;
        
        case LCD_FAN_OFF:
        {
            SEG33_DEF &= 0xFEU;
        }    
        break;
        
        /* Zone Symbol On/Off */
        case LCD_ZONE_ON:
        {
            SEG32_DEF |= 0x01U;
        }   
        break;
        
        case LCD_ZONE_OFF:
        {
            SEG32_DEF &= 0xFEU;
        }    
        break;
        
        /* MGML Symbol On/Off */
        case LCD_MGML_ON:
        {
            SEG34_DEF |= 0x01U;
        }    
        break;

        case LCD_MGML_OFF:
        {
            SEG34_DEF &= 0xFEU;
        }    
        break;    
        
        /* MMHG Symbol On/Off */
        case LCD_MMHG_ON:
        {
            SEG35_DEF |= 0x01U;
        }    
        break;
        
        case LCD_MMHG_OFF:
        {
            SEG35_DEF &= 0xFEU;
        }    
        break;
        
        /* Volts Symbol On/Off */
        case LCD_VOLTS_ON:
        {
            SEG31_DEF |= 0x01U;
        }    
        break;
        
        case LCD_VOLTS_OFF:
        {
            SEG31_DEF &= 0xFEU;
        }    
        break;
                
        /* Heart Symbol On/Off */
        case LCD_HEART_ON:
        {
            SEG30_DEF |= 0x01U;
        }    
        break;
        
        case LCD_HEART_OFF:
        {
            SEG30_DEF &= 0xFEU;
        }    
        break;
        
        /* Degrees C Symbol On/Off */
        case LCD_DEGREESC_ON:
            SEG16_DEF |= 0x04U;
        break;

        case LCD_DEGREESC_OFF:
        {
            SEG16_DEF &= 0xFBU;
        }    
        break;
        
        /* Degrees F Symbol On/Off */
        case LCD_DEGREESF_ON:
        {
            SEG16_DEF |= 0x08U;
        }    
        break;
 
        case LCD_DEGREESF_OFF:
        {
            SEG16_DEF &= 0xF7U;
        }    
        break;            
        
        /* Alarm Symbol On/Off */
        case LCD_ALARM_ON:
        {
            SEG18_DEF |= 0x02U;
        }    
        break;

        case LCD_ALARM_OFF:
        {
            SEG18_DEF &= 0xFDU;
        }
        break;

        /* Alarm Symbol On/Off */
        case LCD_ALARM_ON_1:
            SEG18_DEF |= 0x02U;
        break;
 
        case LCD_ALARM_OFF_1:
        {
            SEG18_DEF &= 0xFDU;
        }    
       break;    
        
        /* AM Symbol On/Off */
        case LCD_AM_ON:
        {
            SEG18_DEF |= 0x01U;
        }
        break;
 
        case LCD_AM_OFF:
        {
            SEG18_DEF &= 0xFEU;
        }    
        break;    

        /* AM Symbol On/Off */
        case LCD_AM_ON_1:
        {
            SEG18_DEF |= 0x01U;
        }
        break;
        
        case LCD_AM_OFF_1:
        {
            SEG27_DEF &= 0xFEU;
        }    
        break;    
        
        /* PM Symbol On/Off */
        case LCD_PM_ON:
        {
            SEG16_DEF |= 0x02U;
        }    
        break;
        
        case LCD_PM_OFF:
        {
            SEG16_DEF &= 0xFDU;
        }    
        break;
        
        /* PM Symbol On/Off */
        case LCD_PM_ON_1:
        {
           SEG16_DEF |= 0x02U;
        }    
        break;
        
        case LCD_PM_OFF_1:
        {
            SEG16_DEF &= 0xFDU;
        }
        break;
        
        /* Renesas Logo On/Off */
        case LCD_R_LOGO_ON:
        {
            SEG12_DEF |= 0x08U;
        }    
        break;
        
        case LCD_R_LOGO_OFF:
        {
            SEG12_DEF &= 0xF7U;
        }
        break;
        
        /* Renesas Logo On/Off */
        case LCD_R_LOGO_ON_1:
        {
            SEG12_DEF |= 0x08U;
        }    
        break;
        
        case LCD_R_LOGO_OFF_1:
        {
            SEG12_DEF &= 0xF7U;
        }    
        break;
        
        default:
        {
            /* Bad Selection */
            status = 1;
        }    
        break;
    }
    return status;
}
/***********************************************************************************************************************
End of function Symbol_Map
***********************************************************************************************************************/


/***********************************************************************************************************************
* Function Name : Clear_Display
* Description   : Clears all the segment data registers, thereby clearing the screen by the next LCD frame duration.
* Arguments     : none
* Return Value  : none
***********************************************************************************************************************/
void Clear_Display (void)
{
    /* Declare a loop count variable */
    unsigned char      i;
     
     /* Initialise pointer to start of registers */
    volatile uint8_t * RegPtr = &SEG0_DEF;

    /* Execute the instructions in the loop 40 times */
    for (i = 0; i < 40; i++)
    {
        /* Write 0 to the register being pointed to.*/
        *RegPtr = 0;
        
        /* Increment the pointer */
        RegPtr++;
    }
}
/***********************************************************************************************************************
End of function Clear_Display
***********************************************************************************************************************/

/***********************************************************************************************************************
* Function Name : Init_Maps
* Description   : Initialises the glyph-segment maps used to display letters, symbols and numbers.
* Arguments     : none
* Return Value  : none
***********************************************************************************************************************/
void Init_Maps (void)
 {
     /* Section A maps START */
     SECTA_map[0x0].WORD = 0x00F5u; /* 0 */
     SECTA_map[0x1].WORD = 0x0060u; /* 1 */
     SECTA_map[0x2].WORD = 0x00B6u; /* 2 */
     SECTA_map[0x3].WORD = 0x00F2u; /* 3 */ 
     SECTA_map[0x4].WORD = 0x0063u; /* 4 */
     SECTA_map[0x5].WORD = 0x00D3u; /* 5 */
     SECTA_map[0x6].WORD = 0x00D7u; /* 6 */
     SECTA_map[0x7].WORD = 0x0070u; /* 7 */
     SECTA_map[0x8].WORD = 0x00F7u; /* 8 */
     SECTA_map[0x9].WORD = 0x00F3u; /* 9 */
     SECTB_map[0xA].WORD = 0x0000u; /* dummy */
     SECTB_map[0xB].WORD = 0x0000u; /* dummy */
     SECTB_map[0xC].WORD = 0x0000u; /* dummy */
     SECTB_map[0xD].WORD = 0x0000u; /* dummy */
     SECTA_map[0xE].WORD = 0x0077u; /* A */
     SECTA_map[0xF].WORD = 0x00C7u; /* B */
     SECTA_map[0x10].WORD = 0x0095u; /* C */
     SECTA_map[0x11].WORD = 0x00E6u; /* D */
     SECTA_map[0x12].WORD = 0x0097u; /* E */
     SECTA_map[0x13].WORD = 0x0017u; /* F */
     /* Section A maps END */

     /* Section B maps START */
     SECTB_map[0x0].WORD = 0x00FAu; /* 0 */
     SECTB_map[0x1].WORD = 0x0060u; /* 1 */
     SECTB_map[0x2].WORD = 0x00D6u; /* 2 */
     SECTB_map[0x3].WORD = 0x00F4u; /* 3 */
     SECTB_map[0x4].WORD = 0x006Cu; /* 4 */
     SECTB_map[0x5].WORD = 0x00BCu; /* 5 */
     SECTB_map[0x6].WORD = 0x00BEu; /* 6 */
     SECTB_map[0x7].WORD = 0x00E0u; /* 7 */
     SECTB_map[0x8].WORD = 0x00FEu; /* 8 */
     SECTB_map[0x9].WORD = 0x00FCu; /* 9 */
     SECTB_map[0xA].WORD = 0x0000u; /* dummy */
     SECTB_map[0xB].WORD = 0x0000u; /* dummy */
     SECTB_map[0xC].WORD = 0x0000u; /* dummy */
     SECTB_map[0xD].WORD = 0x0000u; /* dummy */
     SECTB_map[0xE].WORD = 0x00EEu; /* A */
     SECTB_map[0xF].WORD = 0x003Eu; /* B */
     SECTB_map[0x10].WORD = 0x009Au; /* C */
     SECTB_map[0x11].WORD = 0x0076u; /* D */
     SECTB_map[0x12].WORD = 0x009Eu; /* E */
     SECTB_map[0x13].WORD = 0x008Eu; /* F */
     /* Section B maps END */

     /* Section C maps START */
     /* Numbers */
     SECTC_map[0].WORD = 0xC3AAu; /* 0 */
     SECTC_map[1].WORD = 0x00A0u; /* 1 */
     SECTC_map[2].WORD = 0x816Cu; /* 2 */
     SECTC_map[3].WORD = 0x81E4u; /* 3 */
     SECTC_map[4].WORD = 0x00E6u; /* 4 */
     SECTC_map[5].WORD = 0x81C6u; /* 5 */
     SECTC_map[6].WORD = 0x81CEu; /* 6 */
     SECTC_map[7].WORD = 0x01A0u; /* 7 */
     SECTC_map[8].WORD = 0x81EEu; /* 8 */
     SECTC_map[9].WORD = 0x81E6u; /* 9 */
     
     /* Symbols */
     SECTC_map[10].WORD = 0x2444u; /* + */
     SECTC_map[11].WORD = 0x0044u; /* - */
     SECTC_map[12].WORD = 0x4200u; /* / */
     SECTC_map[13].WORD = 0x1800u; /* \ */
     SECTC_map[40].WORD = 0x7e44u; /* * */
     
     /* Letters */
     SECTC_map[14].WORD = 0x01EEu; /* A */
     SECTC_map[15].WORD = 0xA5E0u; /* B */
     SECTC_map[16].WORD = 0x810Au; /* C */
     SECTC_map[17].WORD = 0xA5A0u; /* D */
     SECTC_map[18].WORD = 0x814Eu; /* E */
     SECTC_map[19].WORD = 0x014Eu; /* F */
     SECTC_map[20].WORD = 0x81CAu; /* G */
     SECTC_map[21].WORD = 0x00EEu; /* H */
     SECTC_map[22].WORD = 0xA500u; /* I */
     SECTC_map[23].WORD = 0x80A0u; /* J */
     SECTC_map[24].WORD = 0x0A0Eu; /* K */
     SECTC_map[25].WORD = 0X800Au; /* L */
     SECTC_map[26].WORD = 0x12AAu; /* M */
     SECTC_map[27].WORD = 0x18AAu; /* N */
     SECTC_map[28].WORD = 0x81AAu; /* O */
     SECTC_map[29].WORD = 0x016Eu; /* P */
     SECTC_map[30].WORD = 0x89AAu; /* Q */
     SECTC_map[31].WORD = 0x096Eu; /* R */
     SECTC_map[32].WORD = 0x81C6u; /* S */
     SECTC_map[33].WORD = 0x2500u; /* T */
     SECTC_map[34].WORD = 0x80AAu; /* U */
     SECTC_map[35].WORD = 0x420Au; /* V */
     SECTC_map[36].WORD = 0x48AAu; /* W */
     SECTC_map[37].WORD = 0x5A00u; /* X */ 
     SECTC_map[38].WORD = 0x1600u; /* Y */
     SECTC_map[39].WORD = 0xC300u; /* Z */

     /* Section C maps END */
 }
/***********************************************************************************************************************
End of function Init_Maps
***********************************************************************************************************************/

/***********************************************************************************************************************
* Function Name : Display_Panel_String
* Description     This function writes to line 1, line 2 or 3 of the LCD. You need to use the defines LCD_LINE1, 
                  LCD_LINE2 and LCD_LINE3 in order to specify the starting position. 
* Arguments     : (uint8_t) position - LCD row (symbol) number.
*                 (char*) string - pointer to data to be written to display (up to 8 chars).
* Return Value  : None
***********************************************************************************************************************/
void Display_Panel_String (const uint8_t position, const char * const string)
{
    /* Declare variable to hold the output data */
    uint8_t     output_buf = 0;
    
    /* Declare a variable flag to hold the input validation result */
    int16_t     flag = 0;
    
    /* Declare a loop count variable */
    uint16_t    i = 0;
    
    /* Declare a symbol select variable */
    uint16_t    symbsel = 0;

    /* Display input onto LCD */
    switch (position)
    {        
        /* Start Section 1, Numerical */
        case PANEL_LCD_LINE2:
        {
             /* Cycle Through Each Digit */
            for (i = 0; i < 6; i++)
            {
                /* Validate Input */
                flag = Validate_Input((const uint8_t) string[i], &output_buf);
                
                /* Is Hex Number? */
                if ((flag == 1) || (flag == 2) || ((i == 3) && (flag == 4)) || ((i == 6) && (flag == 3)))
                {
                    SECTB_Glyph_Map (output_buf, i);
                }
            }
        }    
        break;

        /* End Section 1 */

        /* Start Section C, Alphanumeric */
        case PANEL_LCD_LINE1:
        {
            /* Cycle through Each Section */
            for (i = 0; i < 5; i++)
            {
                flag = Validate_Input((const uint8_t)string[i], &output_buf);
                
                /* Is Alphanumeric or Supported Symbol? */
                if (flag)
                {
                    /* Map Glyph to Segment Pattern */
                    SECTC_Glyph_Map(output_buf, i + 1);
                }
            }
        }    
        break;
        
        /* Start Section 2, Signed Numerical */
        case PANEL_LCD_LINE3:
        {
            /* Turn off minus symbol */
            SEG10_DEF &= 0xF7U;
            
            /* Cycle through each digit */
            for (i = 0; i < 4; i++)
            {
                /* Validate Input */
                flag = Validate_Input((const uint8_t)string[i], &output_buf);
            
                /* Is Number? */
                if ((flag == 1) || (flag == 2) || (flag == 4))
                {
                    if ((i > 1) && (string[2] != '.'))
                    {
                        /* Map Glyph to Segment Pattern */
                        SECTA_Glyph_Map(output_buf, i + 1);
                    }
                    else
                    {
                        /* Map Glyph to Segment Pattern */
                        SECTA_Glyph_Map(output_buf, i);
                    }
                }
            }
        }
        break;

        /* End Section 2 */

        /* Start Symbol Section */
        case PANEL_LCD_SYMBOL:
        {
            if ((string[0] == 'F') || (string[0] == 'f'))
            {
                /* Validate Input */
                flag = Validate_Input((const uint8_t)string[1], &output_buf);
            
                /* Is Number ? */
                if (flag == 1)
                {        
                    /* Map Glyph to Segment Pattern */
                    SECTF_Glyph_Map(output_buf);
                }
            }
            else if (string[0] ==  '-')
            {
                if (string[1])
                {
                    /* Turn on minus symbol */
                    SEG10_DEF/*lra10l*/ |= 0x08U;
                }
                else
                {
                    /* Turn off minus symbol */
                    SEG10_DEF/*lra10l*/ &= 0xF7U;
                }
            }
            else
            {
                /* Convert To Symbol Code */
                symbsel = (uint8_t)string[0];
                symbsel = symbsel << 8;
                symbsel |= (uint8_t)string[1];
            
                /* Map Symbol to Segment */
                Symbol_Map(symbsel);
            }
        }    
        break;
        
        /* End Symbol Section */

        /* Do nothing for undefined selections */
        default:
        {
            /* Bad Selection */
        }    
        break;
    }
}
/***********************************************************************************************************************
End of function Display_Panel_String
***********************************************************************************************************************/

/***********************************************************************************************************************
* Function Name : Display_Panel_Delay
* Description   : Delay routine for LCD or any other devices.
* Arguments     :(uint32_t) units - time in, approximately, microsec
* Return Value  : None
***********************************************************************************************************************/
void Display_Panel_Delay (const uint32_t units)
{
    /* Declare a delay count */
    uint32_t counter = units * PANEL_DELAY_TIMING;
    
    /* Execute a NOP instruction */
    while (counter--)
    {
        asm("nop");
    }
}
/***********************************************************************************************************************
End of function Display_Panel_Delay
***********************************************************************************************************************/

/***********************************************************************************************************************
Function Name : LCD_ON
Description   : Lights up all the segments of the Renesas LCD Panel.
Parameters    : None
Return Value  : None
***********************************************************************************************************************/
void LCD_ON (void)
{
    Display_Panel_String(PANEL_LCD_LINE2, "888:88");
    Display_Panel_String(PANEL_LCD_LINE3, "888");
    Display_Panel_String(PANEL_LCD_LINE1, "88888");
    
    
    Display_Panel_String(PANEL_LCD_SYMBOL, "-1");
    Display_Panel_String(PANEL_LCD_SYMBOL, "11");
    Display_Panel_String(PANEL_LCD_SYMBOL, "21");
    Display_Panel_String(PANEL_LCD_SYMBOL, "31");
    Display_Panel_String(PANEL_LCD_SYMBOL, "41");
    Display_Panel_String(PANEL_LCD_SYMBOL, "51");
    Display_Panel_String(PANEL_LCD_SYMBOL, "61");
    Display_Panel_String(PANEL_LCD_SYMBOL, "71");
    Display_Panel_String(PANEL_LCD_SYMBOL, "81");
    Display_Panel_String(PANEL_LCD_SYMBOL, "91");
    Display_Panel_String(PANEL_LCD_SYMBOL, "a1");
    Display_Panel_String(PANEL_LCD_SYMBOL, "b1");
    Display_Panel_String(PANEL_LCD_SYMBOL, "c1");
    Display_Panel_String(PANEL_LCD_SYMBOL, "d1");
    Display_Panel_String(PANEL_LCD_SYMBOL, "e1");
    
    SECTD_Glyph_Map(1);
    SECTD_Glyph_Map(2);
    SECTD_Glyph_Map(3);
    SECTD_Glyph_Map(4);
    SECTE_Glyph_Map(3);
    SECTE_Glyph_Map(4);
    SECTE_Glyph_Map(5);
    SECTE_Glyph_Map(6);
}
/***********************************************************************************************************************
End of function LCD_ON
***********************************************************************************************************************/

/***********************************************************************************************************************
* Function Name: LCD_DISPLAY_ON
* Description  : Turns on all LCD segments.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void LCD_DISPLAY_ON (void)
{
    SEG0 = 0x0f;
    SEG1 = 0x0f;
    SEG2 = 0x0f;
    SEG3 = 0x0f;
    SEG4 = 0x0f;
    SEG5 = 0x0f;
    SEG6 = 0x0f;
    SEG7 = 0x0f;
    SEG8 = 0x0f;
    SEG9 = 0x0f;
    SEG10 = 0x0f;
    SEG11 = 0x0f;
    SEG12 = 0x0f;
    SEG13 = 0x0f;
    SEG14 = 0x0f;
    SEG15 = 0x0f;
    SEG16 = 0x0f;
    SEG17 = 0x0f;
    SEG18 = 0x0f;
    SEG19 = 0x0f;
    SEG20 = 0x0f;
    SEG21 = 0x0f;
    SEG22 = 0x0f;
    SEG23 = 0x0f;
    SEG24 = 0x0f;
    SEG25 = 0x0f;
    SEG26 = 0x0f;
    SEG27 = 0x0f;
    SEG28 = 0x0f;
    SEG29 = 0x0f;
    SEG30 = 0x0f;
    SEG31 = 0x0f;
    SEG32 = 0x0f;
    SEG33 = 0x0f;
    SEG34 = 0x0f;
    SEG35 = 0x0f;
    SEG36 = 0x0f;
    SEG37 = 0x0f;
    SEG38 = 0x0f;
}
/***********************************************************************************************************************
End of function LCD_DISPLAY_ON
***********************************************************************************************************************/

/***********************************************************************************************************************
* Function Name: LCD_DISPLAY_OFF
* Description  : Turns off all segments.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void LCD_DISPLAY_OFF (void)
{
    SEG0 = 0x00;
    SEG1 = 0x00;
    SEG2 = 0x00;
    SEG3 = 0x00;
    SEG4 = 0x00;
    SEG5 = 0x00;
    SEG6 = 0x00;
    SEG7 = 0x00;
    SEG8 = 0x00;
    SEG9 = 0x00;
    SEG10 = 0x00;
    SEG11 = 0x00;
    SEG12 = 0x00;
    SEG13 = 0x00;
    SEG14 = 0x00;
    SEG15 = 0x00;
    SEG16 = 0x00;
    SEG17 = 0x00;
    SEG18 = 0x00;
    SEG19 = 0x00;
    SEG20 = 0x00;
    SEG21 = 0x00;
    SEG22 = 0x00;
    SEG23 = 0x00;
    SEG24 = 0x00;
    SEG25 = 0x00;
    SEG26 = 0x00;
    SEG27 = 0x00;
    SEG28 = 0x00;
    SEG29 = 0x00;
    SEG30 = 0x00;
    SEG31 = 0x00;
    SEG32 = 0x00;
    SEG33 = 0x00;
    SEG34 = 0x00;
    SEG35 = 0x00;
    SEG36 = 0x00;
    SEG37 = 0x00;
    SEG38 = 0x00;
}
/***********************************************************************************************************************
End of function LCD_DISPLAY_OFF
***********************************************************************************************************************/

