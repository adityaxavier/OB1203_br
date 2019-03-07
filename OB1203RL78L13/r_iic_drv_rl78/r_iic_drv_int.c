/********************************************************************************************************************
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
*********************************************************************************************************************/
/* Copyright (C) 2012 (2013-2016) Renesas Electronics Corporation. All rights reserved.                             */
/********************************************************************************************************************
* File Name     : r_iic_drv_int.c
* Version       : 1.03
* Device(s)     : RL78/G14
* Tool-Chain    : -
*               : 
* OS            : -
* H/W Platform  : -
* Description   : IIC driver IICA interrupt module
* Operation     : -
* Limitations   : -
*********************************************************************************************************************
* History       : DD.MM.YYYY    Version Description
*               : 31.05.2012    1.00    Created
*               : 07.03.2013    1.01    Added the comment ** SET ** for #pragma interrupt.
*               : 07.03.2013    1.01    Ver.1.01 Release
*               : 31.03.2014    1.02    Supported IAR Embedded Workbench for Renesas RL78.
*               : 31.03.2016    1.03    Supported CC-RL.
*               :                       -----------------------------------------------------------------------------
*********************************************************************************************************************/
#ifdef __CA78K0R__                                                          /* Renesas 78K0R, RL78 Compiler         */
    #pragma sfr
#endif  /* __CA78K0R__ */
#ifdef __CCRL__                                                             /* Renesas CC-RL Compiler               */
    #include "iodefine.h"                                                   /* for RL78                             */
#endif  /* __CCRL__ */
#ifdef __ICCRL78__
    #include <ior5f10wmg.h>                                                 /* for RL78/L13 80pin (R5F10WMG)        */  /** SET **/
    #include <ior5f10wmg_ext.h>                                             /* for RL78/L13 80pin (R5F10WMG)        */  /** SET **/
#endif  /* __ICCRL78__ */

#ifdef __CA78K0R__
#pragma interrupt INTIICA0 r_iic_drv_intiica0_isr                           /* Uses RL78 channel 0 interrupt IICA.  */ /** SET **/
/* #pragma interrupt INTIICA1 r_iic_drv_intiica1_isr */                     /* Uses RL78 channel 1 interrupt IICA.  */ /** SET **/
#endif  /* __CA78K0R__ */
#ifdef __CCRL__                                                             /* Renesas CC-RL Compiler               */
#pragma interrupt r_iic_drv_intiica0_isr(vect=INTIICA0)                     /* Uses RL78 channel 0 interrupt IICA.  */ /** SET **/
/* #pragma interrupt r_iic_drv_intiica1_isr(vect=INTIICA1) */               /* Uses RL78 channel 1 interrupt IICA.  */ /** SET **/
#endif  /* __CCRL__ */

/********************************************************************************************************************
Includes
*********************************************************************************************************************/
#include <stddef.h>

#include "r_iic_drv_api.h"
#include "r_iic_drv_sub.h"
#include "r_iic_drv_int.h"
#include "r_iic_drv_sfr.h"
#include "r_iic_drv_os.h"


/********************************************************************************************************************
Macro definitions
*********************************************************************************************************************/


/********************************************************************************************************************
Typedef definitions
*********************************************************************************************************************/


/********************************************************************************************************************
Exported global variables (to be accessed by other files)
*********************************************************************************************************************/


/********************************************************************************************************************
Private global variables and functions
*********************************************************************************************************************/


/********************************************************************************************************************
* Function Name : r_iic_drv_intiica0_isr
* Description   : Interruption handler for channel 0
* Arguments     : None
* Return Value  : None
*********************************************************************************************************************/
#ifdef     __CA78K0R__
__interrupt void r_iic_drv_intiica0_isr(void)
#endif  /* __CA78K0R__ */
#ifdef     __CCRL__                                                         /* Renesas CC-RL Compiler               */
static void __near r_iic_drv_intiica0_isr(void)
#endif  /* __CCRL__ */
#ifdef     __ICCRL78__
#pragma vector=INTIICA0_vect
__interrupt __root void r_iic_drv_intiica0_isr(void)
#endif  /* __ICCRL78__ */
{
#ifdef IICA0_ENABLE

    uint8_t     iics   = R_IIC_FALSE;

    iics = IICS0;

    if (R_IIC_IICS_AL == (iics & R_IIC_IICS_AL))                /* Arbitration-Lost interruption                    */
    {
        g_iic_Event[0] = R_IIC_EV_INT_AL;
    }
    else if (R_IIC_IICS_STOP == (iics & R_IIC_IICS_STOP))       /* Stop condition issue interruption                */
    {
        g_iic_Event[0] = R_IIC_EV_INT_STOP;
    }

    switch (iics & R_IIC_IICS_MASK)
    {
        case R_IIC_IICS_ADRW:                                   /* Address interruption(Write)                      */
        case R_IIC_IICS_ADRR:                                   /* Address interruption(Read)                       */
            g_iic_Event[0] = R_IIC_EV_INT_ADD;                  /* MSTS = 1 ,ACKD = 1, STD = 1, TRC = 1/0           */
        break;

        case R_IIC_IICS_SEND:                                   /* Transmission data interruption                   */
            g_iic_Event[0] = R_IIC_EV_INT_SEND;                 /* MSTS = 1, ACKD = 1, STD = 0, TRC = 1             */
        break;

        case R_IIC_IICS_RECEIVE:                                /* Reception data interruption                      */
            g_iic_Event[0] = R_IIC_EV_INT_RECEIVE;              /* MSTS = 1, ACKD = 0, STD = 0, TRC = 0             */
        break;

        case R_IIC_IICS_NACK_ADRW:                              /* NACK (Address interruption(Write))               */
        case R_IIC_IICS_NACK_ADRR:                              /* NACK (Address interruption(Read))                */
        case R_IIC_IICS_NACK_SEND:                              /* NACK (Sends data interruption)                   */
            g_iic_Event[0] = R_IIC_EV_INT_NACK;                 /* MSTS = 1, ACKD = 0, STD = 0/1, TRC = 1           */
        break;

        default:                                                /* None case                                        */
            /* Do Nothing. */
        break;
    }

#ifdef R_IIC_OS_USE
/* Please set event flag. */

#else
    /* Make the state machine move ahead */
    extern r_iic_drv_info_t i2c;
    error_t i2c_error = R_IIC_Drv_Advance(&i2c);    
#endif  /* #ifdef R_IIC_OS_USE */

#endif /* #ifdef IICA0_ENABLE */
}



/********************************************************************************************************************
* Function Name : r_iic_drv_intiica1_isr
* Description   : Interruption handler for channel 1
* Arguments     : None
* Return Value  : None
*********************************************************************************************************************/
#ifdef     __CA78K0R__
__interrupt void r_iic_drv_intiica1_isr(void)
#endif  /* __CA78K0R__ */
#ifdef     __CCRL__                                                         /* Renesas CC-RL Compiler               */
static void __near r_iic_drv_intiica1_isr(void)
#endif  /* __CCRL__ */
#ifdef     __ICCRL78__
/* #pragma vector=INTIICA1_vect */                                                                                     /** SET **/
__interrupt __root void r_iic_drv_intiica1_isr(void)
#endif  /* __ICCRL78__ */
{
#ifdef IICA1_ENABLE

    uint8_t     iics   = R_IIC_FALSE;

    iics = IICS1;

    if (R_IIC_IICS_AL == (iics & R_IIC_IICS_AL))                /* Arbitration-Lost interruption                    */
    {
        g_iic_Event[1] = R_IIC_EV_INT_AL;
    }
    else if (R_IIC_IICS_STOP == (iics & R_IIC_IICS_STOP))       /* Stop condition issue interruption                */
    {
        g_iic_Event[1] = R_IIC_EV_INT_STOP;
    }

    switch (iics & R_IIC_IICS_MASK)
    {
        case R_IIC_IICS_ADRW:                                   /* Address interruption(Write)                      */
        case R_IIC_IICS_ADRR:                                   /* Address interruption(Read)                       */
            g_iic_Event[1] = R_IIC_EV_INT_ADD;                  /* MSTS = 1 ,ACKD = 1, STD = 1, TRC = 1/0           */
        break;

        case R_IIC_IICS_SEND:                                   /* Transmission data interruption                   */
            g_iic_Event[1] = R_IIC_EV_INT_SEND;                 /* MSTS = 1, ACKD = 1, STD = 0, TRC = 1             */
        break;

        case R_IIC_IICS_RECEIVE:                                /* Reception data interruption                      */
            g_iic_Event[1] = R_IIC_EV_INT_RECEIVE;              /* MSTS = 1, ACKD = 0, STD = 0, TRC = 0             */
        break;

        case R_IIC_IICS_NACK_ADRW:                              /* NACK (Address interruption(Write))               */
        case R_IIC_IICS_NACK_ADRR:                              /* NACK (Address interruption(Read))                */
        case R_IIC_IICS_NACK_SEND:                              /* NACK (Sends data interruption)                   */
            g_iic_Event[1] = R_IIC_EV_INT_NACK;                 /* MSTS = 1, ACKD = 0, STD = 0/1, TRC = 1           */
        break;

        default:                                                /* None case                                        */
            /* Do Nothing. */
        break;
    }

#ifdef R_IIC_OS_USE
/* Please set event flag. */
#endif  /* #ifdef R_IIC_OS_USE */

#endif /* #ifdef IICA1_ENABLE */
}


/* End of File */
