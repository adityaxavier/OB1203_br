/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products.
* No other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
* applicable laws, including copyright laws. 
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING THIS SOFTWARE, WHETHER EXPRESS, IMPLIED
* OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY
* LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE FOR ANY DIRECT,
* INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR
* ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability 
* of this software. By using this software, you agree to the additional terms and conditions found by accessing the 
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2013, 2018 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/

/***********************************************************************************************************************
* File Name    : r_cg_main.c
* Version      : Applilet4 for RL78/L13 V1.04.02.03 [24 May 2018]
* Device(s)    : R5F10WMG
* Tool-Chain   : IAR Systems icc78k0r
* Description  : This file implements main function.
* Creation Date: 3/7/2019
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_macrodriver.h"
#include "r_cg_cgc.h"
#include "r_cg_iica.h"
#include "r_cg_intp.h"
/* Start user code for include. Do not edit comment generated here */
#include "OB1203.h"
/* End user code. Do not edit comment generated here */
#include "r_cg_userdefine.h"

/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/
/* Start user code for pragma. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
/* Start user code for global. Do not edit comment generated here */
#if defined(TEST_CODE)
void defaultConfig(void);
OB1203 ob1203;
#endif
/* End user code. Do not edit comment generated here */

/* Set option bytes */
#pragma location = "OPTBYTE"
__root const uint8_t opbyte0 = 0xEFU;
#pragma location = "OPTBYTE"
__root const uint8_t opbyte1 = 0x7FU;
#pragma location = "OPTBYTE"
__root const uint8_t opbyte2 = 0xE0U;
#pragma location = "OPTBYTE"
__root const uint8_t opbyte3 = 0x84U;

/* Set security ID */
#pragma location = "SECUID"
__root const uint8_t secuid[10] = 
    {0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U};

static void R_MAIN_UserInit(void);
/***********************************************************************************************************************
* Function Name: main
* Description  : This function implements main function.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void main(void)
{
    R_MAIN_UserInit();
    /* Start user code. Do not edit comment generated here */
    while (1U)
    {
        ;
    }
    /* End user code. Do not edit comment generated here */
}
/***********************************************************************************************************************
* Function Name: R_MAIN_UserInit
* Description  : This function adds user code before implementing main function.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
static void R_MAIN_UserInit(void)
{
    /* Start user code. Do not edit comment generated here */
    EI();
#if defined(TEST_CODE)
    /* 10ms delay */
    for(volatile uint32_t wait = 0; wait < 9000; wait++)
    {
      __no_operation();
    }
    
    while(ob1203.ready==false)
    {
      ob1203.reset();
    }

    defaultConfig();
#endif
    /* End user code. Do not edit comment generated here */
}

/* Start user code for adding. Do not edit comment generated here */
#if defined(TEST_CODE)
//USER CONFIGURABLE*********
#define PROX_MODE 0 //0 for prox, 1 for bio
#define BIO_MODE 1 //1 for bio mode

bool mode = PROX_MODE; //start in prox mode, then switch to HR mode when we detect proximity
const bool meas_ps = 1;
const bool ppg2 = 1; //0 for HR, 1 for SpO2
const bool meas_temp = 0;
const bool trim_oscillator = 0;

uint8_t control_config[0x3A-0x15+1];

void defaultConfig(void) //populate the default settings here
{
    //high accuracy oscillator trim overwrite option
    ob1203.osc_trim = 0x3F; //max trim code =0x3F
    //temperature sensor settings (hidden registers)
    meas_temp ? ob1203.temp_en = TEMP_ON : ob1203.temp_en = TEMP_OFF;
    //LS settings
    ob1203.ls_res = LS_RES(2); //2= 18bit 100ms, 0= max res
    ob1203.ls_rate = LS_RATE(2); //2 =100ms, 4 = 500ms
    ob1203.ls_gain = LS_GAIN(3); //gain 3 default (range)
    ob1203.ls_thres_hi = 0x000FFFFF;
    ob1203.ls_thres_lo = 0;
    ob1203.ls_sai = LS_SAI_OFF;
    ob1203.ls_mode = RGB_MODE;
    ob1203.ls_en = LS_ON;
    //PS and PPG settings
    ob1203.ps_sai_en = PS_SAI_OFF;
//    ob1203.ps_sai_en = PS_SAI_ON;
    if(mode)
    {
        ppg2 ? ob1203.ppg_ps_mode = SPO2_MODE : ob1203.ppg_ps_mode = HR_MODE;    
    }
    else
    {
        ob1203.ppg_ps_mode = PS_MODE;
    }
    ob1203.ps_pulses = PS_PULSES(3);
//    pc.printf("ps_pulses = %02X\r\n",ob1203.ps_pulses);
    ob1203.ps_pwidth = PS_PWIDTH(1);
    ob1203.ps_rate = PS_RATE(5); //5 = 100ms
//    pc.printf("ps_rate = %02X\r\n",ob1203.ps_rate);
    ob1203.ps_avg_en = PS_AVG_OFF;
    ob1203.ps_can_ana = PS_CAN_ANA_0;
    ob1203.ps_digital_can = 0;
    ob1203.ps_hys_level = 0;
    meas_ps ? ob1203.ps_current = 0x1FF : ob1203.ps_current = 0x000;
//    ob1203.ps_current = 0;
    ob1203.ps_thres_hi = 0xFF;
    ob1203.ps_thres_lo = 0x00;
    
    //interrupts
    ob1203.ls_int_sel = LS_INT_SEL_W;
    ob1203.ls_var_mode = LS_THRES_INT_MODE;
    ob1203.ls_int_en = LS_INT_OFF;
    ob1203.ppg_ps_en = PPG_PS_ON;

    ob1203.ps_logic_mode = PS_INT_READ_CLEARS;
    ob1203.ps_int_en = PS_INT_OFF;
    ob1203.ls_persist = LS_PERSIST(2);
    ob1203.ps_persist = PS_PERSIST(2);
    
    
    //BIO SETTINGS
    //int
    ob1203.afull_int_en = AFULL_INT_OFF;
    ob1203.ppg_int_en = PPG_INT_ON;
    //PPG
    ob1203.ir_current = 0x1AF; //max 1023. 3FF
    if (ppg2)
    {
//        ob1203.r_current = 0x0FF;
    ob1203.r_current = 0x1AF; //max 511. 1FF
    }
    else 
    {   
        ob1203.r_current = 0;
    }
    ob1203.ppg_ps_gain = PPG_PS_GAIN_1;
    ob1203.ppg_pow_save = PPG_POW_SAVE_OFF;
    ob1203.led_flip = LED_FLIP_OFF;
    ob1203.ch1_can_ana = PPG_CH1_CAN(0);
    ob1203.ch2_can_ana = PPG_CH2_CAN(0);
    //use rate 1 with pulse width 3 and average 4, or rate 3 with pulse width 4 and average 3 for 100 sps (50Hz basis) or 120 sps sample rate (60Hz basis)
    ob1203.ppg_avg = PPG_AVG(4); //2^n averages
    ob1203.ppg_rate = PPG_RATE(1); 
    ob1203.ppg_pwidth = PPG_PWIDTH(3);
    ob1203.ppg_freq = PPG_FREQ_50HZ;
//    ob1203.ppg_freq = PPG_FREQ_60HZ;
    ob1203.bio_trim = 3; //max 3 --this dims the ADC sensitivity, but reduces noise
    ob1203.led_trim = 0x00; //can use to overwrite trim setting and max out the current 
    ob1203.ppg_LED_settling = PPG_LED_SETTLING(2); //hidden regstier for adjusting LED setting time (not a factor for noise)
    ob1203.ppg_ALC_track = PPG_ALC_TRACK(2); //hidden register for adjusting ALC track and hold time (not a factor for noise)
    ob1203.diff = DIFF_ON; //hidden register for turning off subtraction of residual ambient light after ALC
    ob1203.alc = ALC_ON;  //hidden register for turning off ambient light cancelleation track and hold circuit
    ob1203.sig_out = SIGNAL_OUT; //hidden register for selecting ambient sample or LED sample if DIFF is off
    ob1203.fifo_rollover_en = FIFO_ROLL_ON;
    ob1203.fifo_afull_advance_warning = AFULL_ADVANCE_WARNING(0x0F); //warn as quickly as possible (after 17 samples with 0x0F)
    
    //run initialization according to user compile settings

    if(mode == BIO_MODE)
    {
//        pc.printf("initial setup: bio\r\n");
        ppg2 ? ob1203.init_spo2() : ob1203.init_hr();
    }
    else    
    {
//       pc.printf("initial setup: ps\r\n");
       ob1203.ppg_int_en = PPG_INT_OFF;
       ob1203.ps_int_en = PS_INT_OFF;
       ob1203.init_ps();
    }
    if(trim_oscillator)
        ob1203.setOscTrim();
    
    /* Read out the configuration */
    ob1203.readBlock(OB1203_ADDR, 
                     REG_MAIN_CTRL_0, 
                     (char*)&control_config[0], 
                     sizeof(control_config));
}
#endif

/* End user code. Do not edit comment generated here */
