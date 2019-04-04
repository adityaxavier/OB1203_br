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
* File Name    : r_cg_userdefine.h
* Version      : Applilet4 for RL78/L13 V1.04.02.03 [24 May 2018]
* Device(s)    : R5F10WMG
* Tool-Chain   : IAR Systems icc78k0r
* Description  : This file includes user definition.
* Creation Date: 3/26/2019
***********************************************************************************************************************/
#ifndef _USER_DEF_H
#define _USER_DEF_H

/***********************************************************************************************************************
User definitions
***********************************************************************************************************************/

/* Start user code for function. Do not edit comment generated here */
#define ASCII_0			 ('0')

#define DIV_10                   (10)
#define DIV_100                  (100)
#define DIV_1000                 (1000)

#define MULT_10 		 (10)
#define MULT_100                 (100)
#define MULT_1000                (1000)

#define MODULUS_10 		 (10)
#define MODULUS_100              (100)
#define MODULUS_1000             (1000)

#define LOG_FATAL    (1)
#define LOG_ERR      (2)
#define LOG_WARN     (4)
#define LOG_INFO     (8)
#define LOG_DEBUG      (16)
#define LOG_DEBUG_MEAN  (32)
#define LOG_DEBUG_NEED  (64)
#define LOG_DEBUG_RAW   (128)

#if !defined(DEBUG_STREAM)
#define DEBUG_STREAM    stdout
#endif

#if defined(UNITY_TESTING)
#define DEBUG_LVL       (0)
#elif defined(stdout) && !defined(DEBUG_LVL)
#define DEBUG_LVL       (LOG_INFO)
#endif

#if defined(DEBUG_LVL)
#define LOG(level, ...) do {  \
                            if ((level & DEBUG_LVL)>0) { \
                              if(level <= LOG_ERR) { fprintf(DEBUG_STREAM,"%s:%d:", __FILE__, __LINE__); }\
                                fprintf(DEBUG_STREAM, __VA_ARGS__); \
                                fflush(DEBUG_STREAM); \
                            } \
                        } while (0)
#else
#define LOG(level, ...)           
#endif
                          
void OB1203_callback_tx_complete(void);
void OB1203_callback_rx_complete(void);
void OB1203_callback_error(MD_STATUS status);
/* End user code. Do not edit comment generated here */
#endif