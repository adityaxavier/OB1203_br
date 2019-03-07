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
 * File Name    : ob1203.h
 * Version      : 1.0 <- Optional as long as history is shown below
 * Description  : This module solves all the world's problems
 ******************************************************************************/
/*******************************************************************************
 * History : DD.MM.YYYY Version  Description
 *         : 15.01.2007 1.00     First Release
 ******************************************************************************/

/*******************************************************************************
 Includes   <System Includes> , "Project Includes"
 ******************************************************************************/

#ifndef AP_SRC_OB1203_H_
    #define AP_SRC_OB1203_H_
#include <stdbool.h>
#include "r_cg_macrodriver.h"

/*******************************************************************************
 Macro definitions
 ******************************************************************************/
#define OB1203_ADDR 0xA6


/* Register Definitions */
#define REG_STATUS_0        0x00
#define REG_STATUS_1        0x01
#define REG_PS_DATA         0x02
#define REG_LS_W_DATA       0x04
#define REG_LS_G_DATA       0x07
#define REG_LS_B_DATA       0x0A
#define REG_LS_R_DATA       0x0D
#define REG_LS_C_DATA       0x10
#define REG_TEMP_DATA       0x13
#define REG_MAIN_CTRL_0     0x15
#define REG_MAIN_CTRL_1     0x16
#define REG_PS_LED_CURR     0x17
#define REG_PS_CAN_PULSES   0x19
#define REG_PS_PWIDTH_RATE  0x1A
#define REG_PS_CAN_DIG      0x1B
#define REG_PS_MOV_AVG_HYS  0x1D
#define REG_PS_THRES_HI     0x1E
#define REG_PS_THRES_LO     0x20
#define REG_LS_RES_RATE     0x22
#define REG_LS_GAIN         0x23
#define REG_LS_THRES_HI     0x24
#define REG_LS_THRES_LO     0x27
#define REG_LS_THRES_VAR    0x2A
#define REG_INT_CFG_0       0x2B
#define REG_PS_INT_CFG_1    0x2C
#define REG_INT_PST         0x2D
#define REG_PPG_PS_GAIN     0x2E
#define REG_PPG_PS_CFG      0x2F
#define REG_PPG_IRLED_CURR  0x30
#define REG_PPG_RLED_CURR   0x32
#define REG_PPG_CAN_ANA     0x34
#define REG_PPG_AVG         0x35
#define REG_PPG_PWIDTH_RATE 0x36
#define REG_FIFO_CFG        0x37
#define REG_FIFO_WR_PTR     0x38
#define REG_FIFO_RD_PTR     0x39
#define REG_FIFO_OVF_CNT    0x3A
#define REG_FIFO_DATA       0x3B
#define REG_PART_ID         0x3D
#define REG_DEVICE_CFG      0x4D
#define REG_OSC_TRIM        0x3E
#define REG_LED_TRIM        0x3F
#define REG_BIO_TRIM        0x40


//Define settings
//STATUS_0
#define POR_STATUS          0x80
#define LS_INT_STATUS       0x02
#define LS_NEW_DATA         0x01
//STATUS_1
#define LED_DRIVER_STATUS   0x40
#define FIFO_AFULL_STATUS   0x20
#define PPG_DATA_STATUS     0x10
#define PS_LOGIC_STATUS     0x04
#define PS_INT_STATUS       0x02
#define PS_NEW_DATA         0x01
//MAIN_CTRL_0
#define SW_RESET            0x01<<7
#define LS_SAI_ON           0x01<<3
#define LS_SAI_OFF          0x00
#define ALS_MODE            0x00
#define RGB_MODE            0x01<<1
#define LS_OFF              0x00
#define LS_ON               0x01
//MAIN_CTRL_1
#define PS_SAI_ON           0x01<<3
#define PS_SAI_OFF          0x00
#define PS_MODE             0x00
#define HR_MODE             0x01<<1
#define SPO2_MODE           0x02<<1
#define PPG_PS_ON           0x01
#define PPG_PS_OFF          0x00
#define TEMP_ON             0x01<<7
#define TEMP_OFF            0x00
//PS_CAN_PULSES
#define PS_CAN_ANA_0        0x00                //off
#define PS_CAN_ANA_1        0x01<<6             //50% of FS
#define PS_CAN_ANA_2        0x02<<6             //100% of FS
#define PS_CAN_ANA_3        0x03<<6             //150% of FS
#define PS_PULSES(x)    (( (x) & (0x07) )<<3)   //where x = 0..5 and num pulses = 2^x
//PS_PWIDTH_RATE
#define PS_PWIDTH(x)   (x & 0x03)<<4 //where x = 0..3
#define PS_RATE(x)          (x & 0x07) //where x = 0..7
//PS_MOV_AVG_HYS
#define PS_AVG_ON           1<<7
#define PS_AVG_OFF          0
#define PS_HYS_LEVEL(x)     x>>1 //where x=0..256
//LS_RES_RATE
#define LS_RES(x)           (x & 0x07)<<4 //where x=0..7
#define LS_RATE(x)          (x & 0x07) //where x=0..7

#define LS_RES_20          0x00
#define LS_RES_19          0x01
#define LS_RES_18          0x02
#define LS_RES_17          0x03
#define LS_RES_16         0x04
#define LS_RES_13          0x05

//LS_GAIN
#define LS_GAIN(x)          (x & 0x03)
#define LS_GAIN_1           0x00
#define LS_GAIN_3           0x01
#define LS_GAIN_6           0x10
#define LS_GAIN_20          0x11

//LS_THRES_VAR
#define LS_THRES_VAR(x)     (x & 0x07)
//INT_CFG_0
#define LS_INT_SEL_W        0
#define LS_INT_SEL_G        1<<4
#define LS_INT_SEL_R        2<<4
#define LS_INT_SEL_B        3<<4
#define LS_THRES_INT_MODE  0
#define LS_VAR_INT_MODE     1<<1
#define LS_INT_ON           1
#define LS_INT_OFF          0
//INT_CFG_1
#define AFULL_INT_ON        1<<5
#define AFULL_INT_OFF       0
#define PPG_INT_ON          1<<4
#define PPG_INT_OFF          0
#define PS_INT_READ_CLEARS  0<<1
#define PS_INT_LOGIC 1
#define PS_INT_ON           1
#define PS_INT_OFF          0

//INT_PST
#define LS_PERSIST(x)       (x & 0x0F)<<4
#define PS_PERSIST(x)       (x & 0x0F)
//PPG_PS_GAIN
#define PPG_PS_GAIN_1       0
#define PPG_PS_GAIN_1P5     1<<4
#define PPG_PS_GAIN_2       2<<4
#define PPG_PS_GAIN_4       3<<4
#define PPG_LED_SETTLING(x) (x & 0x03)<<2 // 0=0us, 1 = 5us, 2 = 10us (Default), 3=20us
#define PPG_ALC_TRACK(x)    (x & 0x03) // 0 = 10us, 1 (20us) DEFAULT ,2 = 30us, 3 = 60us
//PPG_PS_CFG
#define PPG_POW_SAVE_ON     1<<6
#define PPG_POW_SAVE_OFF    0
#define LED_FLIP_ON         1<<3
#define LED_FLIP_OFF        0
#define DIFF_OFF            2
#define ALC_OFF             1
#define DIFF_ON             0
#define ALC_ON              0
#define SIGNAL_OUT          1<<2
#define OFFSET_OUT          0
//PPG_CAN_ANA
#define PPG_CH1_CAN(x)      (x & 0x03)<<2
#define PPG_CH2_CAN(x)      (x & 0x03)
//PPG_AVG
#define PPG_AVG(x)          (x & 0x07)<<4
//PPG_PWIDTH_RATE
#define PPG_PWIDTH(x)       (x & 0x07)<<4
#define PPG_FREQ_60HZ       0
#define PPG_FREQ_50HZ       1<<3
#define PPG_RATE(x)         (x & 0x07)
//FIFO_CFG
#define FIFO_ROLL_ON        1<<4
#define FIFO_ROLL_OFF       0
#define AFULL_ADVANCE_WARNING(x)   (x & 0x0F)
/*******************************************************************************
 Typedef definitions
 ******************************************************************************/
typedef struct st_ob1203_data
{
	char osc_trim;
	char ls_res;
	char ls_rate;
	char ls_gain;
	uint32_t ls_thres_hi;
	uint32_t ls_thres_lo;
	char ls_sai;
	char ls_mode;
	char ls_en;
	char ls_int_sel;
	char ls_var_mode;
	char ps_sai_en;
	char temp_en;
	char ppg_ps_mode;
	char ppg_ps_en;
	char ps_can_ana;
	char afull_int_en;
	char ppg_int_en;
	char ps_logic_mode;
	uint16_t ps_digital_can;
	char ps_int_en;
	char ls_persist;
	char ps_persist;
	uint16_t ps_thres_hi;
	uint16_t ps_thres_lo;
	uint16_t ps_current;
	uint16_t ir_current;
	uint16_t r_current;
	char ppg_ps_gain;
	char ppg_pow_save;
	char led_flip;
	char ch1_can_ana;
	char ch2_can_ana;
	char ppg_avg;
	char ppg_pwidth;
	char ppg_freq;
	char ppg_rate;
	char ppg_LED_settling;
	char ppg_ALC_track;
	char ps_pulses;
	char ps_pwidth;
	char ps_rate;
	char ps_avg_en;
	char ps_hys_level;
	char ls_int_en;
	char fifo_rollover_en;
	char fifo_afull_advance_warning;
	char writePointer;
	char readPointer;
	char fifoOverflow;
	char bio_trim;
	char led_trim;
	char diff;
	char alc;
	char sig_out;
}ob1203_data_t;

/* Low-level operations */
typedef struct st_ob1203_low_level_fp
{
  void          (__far_func * reset)                    (void);
  uint16_t 	(__far_func * get_status)		(void);
  void 		(__far_func * writeRegister)		(uint8_t, char, char);
  void 		(__far_func * writeBlock)		(uint8_t, char, char *, char);
  void 		(__far_func * readBlock)		(uint8_t, char, char *, int32_t);

  uint32_t 	(__far_func * bytes2uint32)		(char *, int);
  uint32_t 	(__far_func * twoandhalfBytes2uint32)	(char *, int);
}ob1203_ll_fp_t;
/*******************************************************************************
 Exported global variables
 ******************************************************************************/

/* High-level operations */
typedef struct st_ob1203_high_level_fp
{
    void    (__far_func * setOscTrim)			(void);
    bool    (__far_func * dataIsNew)			(void);
    bool    (__far_func * lsIsNew)			(void);
    bool    (__far_func * psIsNew)			(void);
    bool    (__far_func * tempIsNew)			(void);
    bool    (__far_func * bioIsNew)			(void);
    void    (__far_func * setMainConfig)		(void);
    void    (__far_func * setIntConfig)			(void);
    void    (__far_func * setLSthresh)			(void);
    void    (__far_func * setPSthresh)			(void);
    void    (__far_func * setPScurrent)			(void);
    void    (__far_func * setPPGcurrent)		(void);
    void    (__far_func * setPPG_PSgain_cfg)		(void);
    void    (__far_func * setPPGana_can)		(void);
    void    (__far_func * setDigitalCan)		(void);
    void    (__far_func * setPPGavg_and_rate)		(void);
    void    (__far_func * setFifoConfig)		(void);
    void    (__far_func * setBioTrim)			(void);
    void    (__far_func * setLEDTrim)			(void);
    char    (__far_func * get_ps_data)			(uint32_t *);
    char    (__far_func * get_ls_data)			(uint32_t *);
    char    (__far_func * get_ps_ls_data)		(uint32_t *);
    void    (__far_func * resetFIFO)			(void);
    void    (__far_func * init_ps)			(void);
    void    (__far_func * init_rgb)			(void);
    void    (__far_func * init_ps_rgb)			(void);
    void    (__far_func * init_hr)			(void);
    void    (__far_func * init_spo2)			(void);
    void    (__far_func * getFifoInfo)			(char *fifo_info);
    uint8_t (__far_func * getNumFifoSamplesAvailable)	(void);
    void    (__far_func * getFifoSamples)		(uint8_t, char *);
    void    (__far_func * parseFifoSamples)		(char, char *, uint32_t *);
    char    (__far_func * get_part_ID)			(char *);
}ob1203_hl_fp_t;


typedef struct st_ob1203
{
  const __near ob1203_data_t * p_data;
  const __far  ob1203_ll_fp_t * p_ll;
  const __far  ob1203_hl_fp_t * p_hl;
}ob1203_t1;
/*******************************************************************************
 Exported global functions (to be accessed by other files)
 ******************************************************************************/

#endif /* AP_SRC_OB1203_H_ */
