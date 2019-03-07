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
* File Name     : r_iic_drv_os.c
* Version       : 1.03
* Device(s)     : RL78/G14
* Tool-Chain    : -
*               : 
* OS            : -
* H/W Platform  : -
* Description   : IIC driver IICA OS module
* Operation     : -
* Limitations   : -
*********************************************************************************************************************
* History       : DD.MM.YYYY    Version Description
*               : 31.05.2012    1.00    Created
*               : 07.03.2013    1.01    Ver.1.01 Release
*               : 31.03.2014    1.02    Supported IAR Embedded Workbench for Renesas RL78.
*               : 31.03.2016    1.03    Supported CC-RL.
*               :                       -----------------------------------------------------------------------------
*********************************************************************************************************************/
#ifdef __CA78K0R__
    #pragma SFR             /* for 78K0R                                                        */  /** SET **/ 
#endif  /* __CA78K0R__ */
#ifdef __CCRL__                                                             /* Renesas CC-RL Compiler               */
    #include "iodefine.h"                                                   /* for RL78                             */
#endif  /* __CCRL__ */
#ifdef __ICCRL78__
    #include <ior5f10wmg.h>                                                 /* for RL78/L13 80pin (R5F10WMG)        */  /** SET **/
    #include <ior5f10wmg_ext.h>                                             /* for RL78/L13 80pin (R5F10WMG)        */  /** SET **/
#endif  /* __ICCRL78__ */


/********************************************************************************************************************
Includes
*********************************************************************************************************************/
#include <stddef.h>

#include "r_iic_drv_api.h"
#include "r_iic_drv_sub.h"
#include "r_iic_drv_int.h"
#include "r_iic_drv_sfr.h"
#include "r_iic_drv_os.h"

/* #include <RTOS.h> */                         /* OS header file                                                   */  /** SET **/

/********************************************************************************************************************
Macro definitions
*********************************************************************************************************************/
/* Specify the ID for semaphore (microITRON-compatible) to be used in the File System Library.                      */
/* If not using a semaphore, ignore this value.                                                                     */
#define R_IIC_SEM_ID             1          /* Semaphore ID for microITRON (Real-Time OS)-compatible                */  /** SET **/
#define R_IIC_EVENT_ID           1          /* Event flag ID for microITRON (Real-Time OS)-compatible               */  /** SET **/

/* If using a semaphore, effective below.                                                                           */
#if 0   /* for microITRON 4.00 */                                                                                                           /** SET **/
    #ifdef R_IIC_OS_USE
#define R_IIC_WAI_SEM(semid)                                wai_sem((ID )(semid))                                                           /** SET **/
#define R_IIC_SIG_SEM(semid)                                sig_sem((ID )(semid))                                                           /** SET **/
#define R_IIC_DLY_TSK(dlytim)                               dly_tsk((RELTIM )(dlytim))                                                      /** SET **/
#define R_IIC_STA_TSK(tskid, stacd)                         sta_tsk((ID )(tskid), (VP_INT )(stacd))                                         /** SET **/
#define R_IIC_SET_FLG(flgid, setptn)                        set_flg((ID )(flgid), (FLGPTN )(setptn))                                        /** SET **/
#define R_IIC_CLR_FLG(flgid, clrptn)                        clr_flg((ID )(flgid), (FLGPTN )(clrptn))                                        /** SET **/
#define R_IIC_WAI_FLG(p_flgptn, flgid, waiptn, wfmode)      wai_flg((ID )(flgid), (FLGPTN )(waiptn), (MODE )(wfmode), (FLGPTN* )(p_flgptn)) /** SET **/
#define R_IIC_POL_FLG(p_flgptn, flgid, waiptn, wfmode)      pol_flg((ID )(flgid), (FLGPTN )(waiptn), (MODE )(wfmode), (FLGPTN* )(p_flgptn)) /** SET **/
    #endif  /* #ifdef R_IIC_OS_USE */

#else
#define R_IIC_WAI_SEM(semid)                                0
#define R_IIC_SIG_SEM(semid)                                0
#define R_IIC_DLY_TSK(dlytim)                               0
#define R_IIC_STA_TSK(tskid, stacd)                         0
#define R_IIC_SET_FLG(flgid, setptn)                        0
#define R_IIC_CLR_FLG(flgid, clrptn)                        0
#define R_IIC_WAI_FLG(p_flgptn, flgid, waiptn, wfmode)      0
#define R_IIC_POL_FLG(p_flgptn, flgid, waiptn, wfmode)      0
#endif  /* for microITRON 4.00 */


/********************************************************************************************************************
Typedef definitions
*********************************************************************************************************************/


/********************************************************************************************************************
Exported global variables (to be accessed by other files)
*********************************************************************************************************************/


/********************************************************************************************************************
Private global variables and functions
*********************************************************************************************************************/
/* File System common data definitions. */
const uint8_t   RIicSemID   = R_IIC_SEM_ID;                 /* semaphore ID                 */  /*   1      byte    */
const uint8_t   RIicEventID = R_IIC_EVENT_ID;               /* event flag ID                */  /*   1      byte    */


/********************************************************************************************************************
* Function Name : r_iic_drv_waisem
* Description   : Gets the semaphore resource.
*               : If the OS is not used, this function becomes a dummy processing.
* Arguments     : uint16_t      semid           ;   Semaphore ID number
* Return Value  : -
*********************************************************************************************************************/
int r_iic_drv_waisem(short semid)
{
    return R_IIC_WAI_SEM(semid);                            /* Semaphore resource allocation system call            */
}

/********************************************************************************************************************
* Function Name : r_iic_drv_sigsem
* Description   : Returns the semaphore resource.
*               : If the OS is not used, this function becomes a dummy processing.
* Arguments     : uint16_t      semid           ;   Semaphore ID number
* Return Value  : -
*********************************************************************************************************************/
int r_iic_drv_sigsem(short semid)
{
    return R_IIC_SIG_SEM(semid);                            /* Semaphore resource deallocation system call          */
}

/********************************************************************************************************************
* Function Name : r_iic_drv_dlytsk
* Description   : Delays execution of the task.
*               : If the OS is not used, this function becomes a dummy processing.
* Arguments     : int           dlytim          ; Delay
* Return Value  : -
*********************************************************************************************************************/
int r_iic_drv_dlytsk(int dlytim)
{
    return R_IIC_DLY_TSK(dlytim);                               /* Task execution delay system call                 */
}

/********************************************************************************************************************
* Function Name : r_iic_drv_statsk
* Description   : Activates task.
*               : If the OS is not used, this function becomes a dummy processing.
* Arguments     : short         tskid           ; ID number of the task to be activated
*               : int           stacd           ; Start code of the task
* Return Value  : -
*********************************************************************************************************************/
int r_iic_drv_statsk(short tskid, int stacd)
{
    return R_IIC_STA_TSK(tskid, stacd);                         /* Activate task system call                        */
}


/********************************************************************************************************************
* Function Name : r_iic_drv_setflg
* Description   : Event flag setting
*               : If the OS is not used, this function becomes a dummy processing.
* Arguments     : short         flgid           ; ID number of event flag to be set
*               : uint          setptn          ; Bit pattern to set
* Return Value  : -
*********************************************************************************************************************/
int r_iic_drv_setflg(short flgid, uint setptn)
{
    return R_IIC_SET_FLG(flgid, setptn);                        /* Set event flag system call                       */
}

/********************************************************************************************************************
* Function Name : r_iic_drv_clrflg
* Description   : Clears event flag. (substitute system call)
*               : If the OS is not used, this function becomes a dummy processing.
* Arguments     : short         flgid           ; ID number of the event flag to be cleared
*               : uint          clrptn          ; Bits pattern to clear
* Return Value  : -
*********************************************************************************************************************/
int r_iic_drv_clrflg(short flgid, uint clrptn)
{
    return R_IIC_CLR_FLG(flgid, clrptn);                        /* Clear event flag system call                     */
}

/********************************************************************************************************************
* Function Name : r_iic_drv_waiflg
* Description   : Waits the event flag.
*               : If the OS is not used, this function becomes a dummy processing.
* Arguments     : uint          *p_flgptn       ; First address of the area of the bit pattern
*               :                                 which is returned when the wait is released.
*               : short         flgid           ; Event flag ID number
*               : uint          waiptn          ; Wait bit pattern
*               : uint          wfmode          ; Wait mode
* Return Value  : -
*********************************************************************************************************************/
int r_iic_waiflg(uint *p_flgptn, short flgid, uint waiptn, uint wfmode)
{
    return R_IIC_WAI_FLG(p_flgptn, flgid, waiptn, wfmode);  /* Event flag wait system call.                         */
}

/********************************************************************************************************************
* Function Name : r_iic_drv_polflg
* Description   : Gets the event flag. ( No wait)
*               : If the OS is not used, this function becomes a dummy processing.
* Arguments     : uint FAR*     p_flgptn        ; First address of the area of the bit pattern
*               :                                 which is returned when the wait is released.
*               : short         flgid           ; Event flag ID number
*               : uint          waiptn          ; Wait bit pattern
*               : uint          wfmode          ; Wait mode
* Return Value  : -
*********************************************************************************************************************/
int r_iic_polflg(uint *p_flgptn, short flgid, uint waiptn, uint wfmode)
{
    return R_IIC_POL_FLG(p_flgptn, flgid, waiptn, wfmode);  /* System call to get the event flag                    */
}


/* End of File */
