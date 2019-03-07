/********************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only
* intended for use with Renesas products. No other uses are authorized.
* This software is owned by Renesas Electronics Corporation and is protected under
* all applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES
* REGARDING THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY,
* INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
* PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY
* DISCLAIMED.
* TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS
* ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE
* FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES
* FOR ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS
* AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this
* software and to discontinue the availability of this software.
* By using this software, you agree to the additional terms and
* conditions found by accessing the following link:
* http://www.renesas.com/disclaimer
*********************************************************************************************************/
/* Copyright (C) 2012 (2013-2016) Renesas Electronics Corporation. All rights reserved.                 */
/********************************************************************************************************
* File Name     : r_iic_drv_int.h
* Version       : 1.03
* Device(s)     : RL78/G14
* Tool-Chain    : -
*               : 
* OS            : -
* H/W Platform  : -
* Description   : IIC driver IICA interrupt module definitions
* Operation     : -
* Limitations   : -
*********************************************************************************************************
* History       : DD.MM.YYYY    Version Description
*               : 31.05.2012    1.00    Created
*               : 07.03.2013    1.01    Ver.1.01 Release
*               : 31.03.2014    1.02    Supported IAR Embedded Workbench for Renesas RL78.
*               : 31.03.2016    1.03    Changed version.
*               :                       -----------------------------------------------------------------
*********************************************************************************************************/
#ifndef __R_IIC_DRV_INT_H__
#define __R_IIC_DRV_INT_H__


/********************************************************************************************************
Macro definitions
*********************************************************************************************************/


/********************************************************************************************************
Typedef definitions
*********************************************************************************************************/
#define R_IIC_IICS_AL           (uint8_t)(0x40)     /* Arbitration-Lost interruption                    */
#define R_IIC_IICS_STOP         (uint8_t)(0x01)     /* Stop condition issue interruption                */
#define R_IIC_IICS_ADRW         (uint8_t)(0x8E)     /* Address interruption(Write)                      */
#define R_IIC_IICS_ADRR         (uint8_t)(0x86)     /* Address interruption(Read)                       */
#define R_IIC_IICS_SEND         (uint8_t)(0x8C)     /* Transmission data interruption                   */
#define R_IIC_IICS_RECEIVE      (uint8_t)(0x80)     /* Reception data interruption                      */
#define R_IIC_IICS_NACK_ADRW    (uint8_t)(0x8A)     /* NACK (Address interruption(Write))               */
#define R_IIC_IICS_NACK_ADRR    (uint8_t)(0x82)     /* NACK (Address interruption(Read))                */
#define R_IIC_IICS_NACK_SEND    (uint8_t)(0x88)     /* NACK (Sends data interruption)                   */
#define R_IIC_IICS_MASK         (uint8_t)(0x8F)     /* Mask ALD bit, EXC bit and CIO bit                */

/********************************************************************************************************
Exported global variables
*********************************************************************************************************/


/********************************************************************************************************
Exported global functions (to be accessed by other files)
*********************************************************************************************************/
/* Driver interrupt functions */
#ifdef     __CA78K0R__
__interrupt void r_iic_drv_intiica0_isr(void);
__interrupt void r_iic_drv_intiica1_isr(void);
#endif  /* __CA78K0R__ */
#ifdef     __ICCRL78__
__interrupt __root void r_iic_drv_intiica0_isr(void);
__interrupt __root void r_iic_drv_intiica1_isr(void);
#endif  /* __ICCRL78__ */


#endif /* __R_IIC_DRV_INT_H__ */


/* End of File */
