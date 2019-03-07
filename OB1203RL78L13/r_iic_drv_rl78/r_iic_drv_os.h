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
* File Name     : r_iic_drv_os.h
* Version       : 1.03
* Device(s)     : RL78/G14
* Tool-Chain    : -
*               : 
* OS            : -
* H/W Platform  : -
* Description   : IIC driver IICA OS module definitions
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
#ifndef __R_IIC_DRV_OS_H__
#define __R_IIC_DRV_OS_H__


/********************************************************************************************************
Macro definitions
*********************************************************************************************************/
/* #define R_IIC_OS_USE */                              /* Use OS.                                      */  /** SET **/

#if 0   /* for microITRON 4.00 */                                                                           /** SET **/
    #ifdef R_IIC_OS_USE
#define R_IIC_STARTCODE                                 0                                                   /** SET **/
#define R_IIC_E_OK                                      ( E_OK      )                                       /** SET **/
#define R_IIC_WM_AND_CLR                                ( TWF_ANDW  )                                       /** SET **/
#define R_IIC_WM_AND                                    ( TWF_ANDW  )                                       /** SET **/
    #endif  /* #ifdef R_IIC_OS_USE */

#else
#define R_IIC_STARTCODE                                 0
#define R_IIC_E_OK                                      0
#define R_IIC_WM_AND_CLR                                0
#define R_IIC_WM_AND                                    0
#endif  /* for microITRON 4.00 */


#if 1
typedef unsigned char   uchar;                          /* For Legacy S/W                               */
typedef unsigned short  ushort;                         /* For Legacy S/W                               */
typedef unsigned int    uint;                           /* For Legacy S/W                               */
typedef unsigned long   ulong;                          /* For Legacy S/W                               */
#endif

/* ==== OS information ==== */
#define R_IIC_SETPTN                0x0001              /* Pattern to set in the event flag             */  /** SET **/
#define R_IIC_CLRPTN                0xfffe              /* Pattern to clear in the event flag           */  /** SET **/
#define R_IIC_WAIPTN                0x0001              /* Waiting pattern of the event flag            */  /** SET **/
#define R_IIC_WFMODE                0x00000000          /* TWF_ANDW (Wait AND)                          */  /** SET **/


/********************************************************************************************************
Typedef definitions
*********************************************************************************************************/


/********************************************************************************************************
Exported global variables
*********************************************************************************************************/
extern const uint8_t                RIicSemID;                  /* semaphore ID                         */
extern const uint8_t                RIicEventID;                /* event flag ID                        */


/********************************************************************************************************
Exported global functions (to be accessed by other files)
*********************************************************************************************************/
/* Use OS. */
int r_iic_drv_waisem(short semid);
int r_iic_drv_sigsem(short semid);
int r_iic_drv_dlytsk(int dlytim);
int r_iic_drv_statsk(short tskid, int stacd);
int r_iic_drv_setflg(short flgid, uint setptn);
int r_iic_drv_clrflg(short flgid, uint clrptn);
int r_iic_waiflg(uint *p_flgptn, short flgid, uint waiptn, uint wfmode);
int r_iic_polflg(uint *p_flgptn, short flgid, uint waiptn, uint wfmode);

#endif /* __R_IIC_DRV_OS_H__ */


/* End of File */
