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
* File Name     : r_iic_drv_api.c
* Version       : 1.03
* Device(s)     : RL78/G14
* Tool-Chain    : -
*               : 
* OS            : -
* H/W Platform  : -
* Description   : IIC driver IICA I/F module
* Operation     : -
* Limitations   : -
*********************************************************************************************************************
* History       : DD.MM.YYYY    Version Description
*               : 31.05.2012    1.00    Created
*               : 07.03.2013    1.01    Ver.1.01 Release
*               : 31.03.2014    1.02    Supported IAR Embedded Workbench for Renesas RL78.
*               : 31.03.2014    1.02    All API functions, the flow of the return value was changed.
*               : 31.03.2016    1.03    Changed version.
*               :                       -----------------------------------------------------------------------------
*********************************************************************************************************************/


/********************************************************************************************************************
Includes
*********************************************************************************************************************/
#include <stddef.h>

#ifdef __ICCRL78__
#include <stdbool.h>
#endif  /* __ICCRL78__ */

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
error_t                         g_iic_ChStatus[MAX_IIC_CH_NUM];     /* Channel status flag                          */
r_iic_drv_internal_event_t      g_iic_Event[MAX_IIC_CH_NUM];        /* Event flag                                   */
r_iic_drv_internal_info_t       g_iic_InternalInfo[MAX_IIC_CH_NUM]; /* Internal status management                   */
uint32_t                        g_iic_ReplyCnt[MAX_IIC_CH_NUM];     /* Non-reply counter                            */
bool                            g_iic_Api[MAX_IIC_CH_NUM];          /* Api flag                                     */

/********************************************************************************************************************
Private global variables and functions
*********************************************************************************************************************/
/* Callback function */
static r_iic_callback R_IIC_Drv_CallBackFunc[MAX_IIC_CH_NUM];

#ifdef R_IIC_OS_USE
/* Please add the user's original flag. */
#endif  /* #ifdef R_IIC_OS_USE */

/********************************************************************************************************************
* Function Name : R_IIC_Drv_Init
* Description   : Initialization processing
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : R_IIC_IDLE                        ;   Successful operation
*               : R_IIC_FINISH / R_IIC_NACK         ;   Already idle state
*               : R_IIC_LOCK_FUNC                   ;   Other task is handling API function
*               : R_IIC_BUS_BUSY                    ;   Bus busy
*               : 
*               : R_IIC_ERR_PARAM                   ;   Parameter error
*               : R_IIC_ERR_AL                      ;   Arbitration lost error
*               : R_IIC_ERR_NON_REPLY               ;   None reply error
*               : R_IIC_ERR_SDA_LOW_HOLD            ;   SDA Low hold error
*               ; R_IIC_ERR_OTHER                   ;   Other error
*********************************************************************************************************************/
error_t R_IIC_Drv_Init(r_iic_drv_info_t *pRIic_Info)
{
    bool        Chk = R_IIC_FALSE;
    error_t     Ret = R_IIC_ERR_OTHER;                          /* Sets R_IIC_ERR_OTHER as initialization of Ret.   */
#ifdef R_IIC_OS_USE
/* Please add internal variable. */
#endif  /* #ifdef R_IIC_OS_USE */

    /* Is "pRIic_Info" setting?  */
    if (NULL == pRIic_Info)
    {
        return R_IIC_ERR_PARAM;
    }

#ifdef R_IIC_OS_USE
/* Please get semaphore resource. */
#endif  /* #ifdef R_IIC_OS_USE */
    
    /* Sets interruption disabled. */
    r_iic_drv_int_maskable_disable();

    /* Checks API flag. */
    Chk = r_iic_drv_lock_api(pRIic_Info);

    /* Sets interruption enabled. */
    r_iic_drv_int_maskable_enable();

    if (R_IIC_FALSE == Chk)
    {
        /* Lock has already been acquired by other task. Need to try again later.   */
        return R_IIC_LOCK_FUNC;
    }

    /* Calls API function. */
    Ret = r_iic_drv_init(pRIic_Info);

    /* Checks API flag. */
    r_iic_drv_unlock_api(pRIic_Info);

#ifdef R_IIC_OS_USE
/* Please release semaphore resource. */
#endif  /* #ifdef R_IIC_OS_USE */
    
    return Ret;

}

/*------------------------------------------------------------------------------------------------------------------*/
error_t r_iic_drv_init(r_iic_drv_info_t *pRIic_Info)
{
    error_t     Ret = R_IIC_ERR_OTHER;                          /* Sets R_IIC_ERR_OTHER as initialization of Ret.   */
#ifdef R_IIC_OS_USE
/* Please add internal variable. */
#endif  /* #ifdef R_IIC_OS_USE */

    /* Sets interruption disabled. */
    r_iic_drv_int_maskable_disable();
    
    /* Checks channel status. */
    Ret = r_iic_drv_check_chstatus_init(pRIic_Info);
    if (R_IIC_NO_INIT != Ret)
    {
        /* Sets interruption enabled. */
        r_iic_drv_int_maskable_enable();

        return Ret;
    }

    /* Updates channel status flag. */
    r_iic_drv_set_ch_status(pRIic_Info, R_IIC_IDLE);

    /* Sets interruption enabled. */
    r_iic_drv_int_maskable_enable();

    /* RAM initialization */
    r_iic_drv_ram_init(pRIic_Info);
    
    /* Sets internal status.(Internal information initialization) */
    r_iic_drv_set_internal_status(pRIic_Info, R_IIC_STS_NO_INIT);

    if (NULL != pRIic_Info->CallBackFunc)
    {
        /* Sets callback function. */
        R_IIC_Drv_CallBackFunc[pRIic_Info->ChNo] = pRIic_Info->CallBackFunc;
    }

#ifdef R_IIC_OS_USE
/* Please clear event flag. */
#endif  /* #ifdef R_IIC_OS_USE */

    /* Disables IIC interruption. */
    r_iic_drv_int_disable(pRIic_Info);

    /* Initializes IIC driver. */
    Ret = r_iic_drv_func_table(R_IIC_EV_INIT, pRIic_Info);
    if (R_IIC_IDLE != Ret)
    {
        /* Sets interruption disabled. */
        r_iic_drv_int_maskable_disable();

        /* Updates channel status flag. */
        r_iic_drv_set_ch_status(pRIic_Info, R_IIC_ERR_OTHER);

        /* Sets interruption enabled. */
        r_iic_drv_int_maskable_enable();
        
        return R_IIC_ERR_OTHER;
    }

    return R_IIC_IDLE;

}

/********************************************************************************************************************
* Function Name : R_IIC_Drv_MasterTx
* Description   : Master transmission start processing
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : R_IIC_COMMUNICATION               ;   Successful operation
*               : R_IIC_NO_INIT                     ;   Uninitialized state
*               : R_IIC_LOCK_FUNC                   ;   Other task is handling API function
*               : R_IIC_BUS_BUSY                    ;   Bus busy
*               : 
*               : R_IIC_ERR_PARAM                   ;   Parameter error
*               : R_IIC_ERR_AL                      ;   Arbitration lost error
*               : R_IIC_ERR_NON_REPLY               ;   None reply error
*               : R_IIC_ERR_SDA_LOW_HOLD            ;   SDA Low hold error
*               ; R_IIC_ERR_OTHER                   ;   Other error
*********************************************************************************************************************/
error_t R_IIC_Drv_MasterTx(r_iic_drv_info_t *pRIic_Info)
{
    bool        Chk = R_IIC_FALSE;
    error_t     Ret = R_IIC_ERR_OTHER;                          /* Sets R_IIC_ERR_OTHER as initialization of Ret.   */
#ifdef R_IIC_OS_USE
/* Please add internal variable. */
#endif  /* #ifdef R_IIC_OS_USE */

    /* Is "pRIic_Info" setting?  */
    if (NULL == pRIic_Info)
    {
        return R_IIC_ERR_PARAM;
    }

#ifdef R_IIC_OS_USE
/* Please get semaphore resource. */
#endif  /* #ifdef R_IIC_OS_USE */

    /* Sets interruption disabled. */
    r_iic_drv_int_maskable_disable();

    /* Checks API flag. */
    Chk = r_iic_drv_lock_api(pRIic_Info);

    /* Sets interruption enabled. */
    r_iic_drv_int_maskable_enable();

    if (R_IIC_FALSE == Chk)
    {
        /* Lock has already been acquired by other task. Need to try again later.   */
        return R_IIC_LOCK_FUNC;
    }

    /* Calls API function. */
    Ret = r_iic_drv_mastertx(pRIic_Info);

    /* Checks API flag. */
    r_iic_drv_unlock_api(pRIic_Info);

#ifdef R_IIC_OS_USE
    if (R_IIC_COMMUNICATION != Ret)
    {
/* Please release semaphore resource. */
    }
#endif  /* #ifdef R_IIC_OS_USE */
    
    return Ret;

}

/*------------------------------------------------------------------------------------------------------------------*/
error_t r_iic_drv_mastertx(r_iic_drv_info_t *pRIic_Info)
{
    error_t     Ret = R_IIC_ERR_OTHER;                          /* Sets R_IIC_ERR_OTHER as initialization of Ret.   */
#ifdef R_IIC_OS_USE
/* Please add internal variable. */
#endif  /* #ifdef R_IIC_OS_USE */

    /* Sets interruption disabled. */
    r_iic_drv_int_maskable_disable();

    /* Checks channel status and Updates channel status flag. */
    Ret = r_iic_drv_check_chstatus_start(pRIic_Info);
    if ((R_IIC_IDLE != Ret) &&
        (R_IIC_FINISH != Ret) &&
        (R_IIC_NACK != Ret)         )
    {
        /* Sets interruption enabled. */
        r_iic_drv_int_maskable_enable();

        return Ret;
    }

    /* Updates channel status flag. */
    r_iic_drv_set_ch_status(pRIic_Info, R_IIC_COMMUNICATION);

    /* Sets interruption enabled. */
    r_iic_drv_int_maskable_enable();
    
    /* Checks bus busy. */
    if (R_IIC_FALSE == r_iic_drv_check_bus_busy(pRIic_Info))
    {
        /* Sets interruption disabled. */
        r_iic_drv_int_maskable_disable();

        /* Updates channel status flag. */
        r_iic_drv_set_ch_status(pRIic_Info, R_IIC_ERR_NON_REPLY);

        /* Sets interruption enabled. */
        r_iic_drv_int_maskable_enable();

        return R_IIC_ERR_NON_REPLY;
    }

    /* RAM initialization */
    r_iic_drv_ram_init(pRIic_Info);

    /* Sets internal mode flag. */
    g_iic_InternalInfo[pRIic_Info->ChNo].Mode = R_IIC_MODE_WRITE;

#ifdef R_IIC_OS_USE
/* Please clear event flag. */
#endif  /* #ifdef R_IIC_OS_USE */
    
    /* Generates start condition.  */
    Ret = r_iic_drv_func_table(R_IIC_EV_GEN_START_COND, pRIic_Info);
    if (R_IIC_COMMUNICATION != Ret)
    {
        /* Terminates processing at error. */
        r_iic_drv_err_fin(pRIic_Info);

        /* Sets interruption disabled. */
        r_iic_drv_int_maskable_disable();

        /* Updates channel status flag. */
        r_iic_drv_set_ch_status(pRIic_Info, R_IIC_ERR_NON_REPLY);

        /* Sets interruption enabled. */
        r_iic_drv_int_maskable_enable();

        return R_IIC_ERR_NON_REPLY;
    }
    
    /* Transmits slave address. */
    Ret = r_iic_drv_func_table(R_IIC_EV_SEND_SLVADR, pRIic_Info);
    if (R_IIC_COMMUNICATION != Ret)
    {
        /* Terminates processing at error. */
        r_iic_drv_err_fin(pRIic_Info);

        /* Sets interruption disabled. */
        r_iic_drv_int_maskable_disable();

        /* Updates channel status flag. */
        r_iic_drv_set_ch_status(pRIic_Info, R_IIC_ERR_OTHER);

        /* Sets interruption enabled. */
        r_iic_drv_int_maskable_enable();

        return R_IIC_ERR_OTHER;
    }

    /* Counter initialization for non-reply error */
    g_iic_ReplyCnt[pRIic_Info->ChNo] = REPLY_CNT;
    
    return R_IIC_COMMUNICATION;
}

/********************************************************************************************************************
* Function Name : R_IIC_Drv_MasterRx
* Description   : Master reception start processing
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : R_IIC_COMMUNICATION               ;   Successful operation
*               : R_IIC_NO_INIT                     ;   Uninitialized state
*               : R_IIC_LOCK_FUNC                   ;   Other task is handling API function
*               : R_IIC_BUS_BUSY                    ;   Bus busy
*               : 
*               : R_IIC_ERR_PARAM                   ;   Parameter error
*               : R_IIC_ERR_AL                      ;   Arbitration lost error
*               : R_IIC_ERR_NON_REPLY               ;   None reply error
*               : R_IIC_ERR_SDA_LOW_HOLD            ;   SDA Low hold error
*               ; R_IIC_ERR_OTHER                   ;   Other error
*********************************************************************************************************************/
error_t R_IIC_Drv_MasterRx(r_iic_drv_info_t *pRIic_Info)
{
    bool        Chk = R_IIC_FALSE;
    error_t     Ret = R_IIC_ERR_OTHER;                          /* Sets R_IIC_ERR_OTHER as initialization of Ret.   */
#ifdef R_IIC_OS_USE
/* Please add internal variable. */
#endif  /* #ifdef R_IIC_OS_USE */

    /* Is "pRIic_Info" setting?  */
    if (NULL == pRIic_Info)
    {
        return R_IIC_ERR_PARAM;
    }

#ifdef R_IIC_OS_USE
/* Please get semaphore resource. */
#endif  /* #ifdef R_IIC_OS_USE */
    
    /* Sets interruption disabled. */
    r_iic_drv_int_maskable_disable();

    /* Checks API flag. */
    Chk = r_iic_drv_lock_api(pRIic_Info);

    /* Sets interruption enabled. */
    r_iic_drv_int_maskable_enable();

    if (R_IIC_FALSE == Chk)
    {
        /* Lock has already been acquired by other task. Need to try again later.   */
        return R_IIC_LOCK_FUNC;
    }

    /* Calls API function. */
    Ret = r_iic_drv_masterrx(pRIic_Info);

    /* Checks API flag. */
    r_iic_drv_unlock_api(pRIic_Info);

#ifdef R_IIC_OS_USE
    if (R_IIC_COMMUNICATION != Ret)
    {
/* Please release semaphore resource. */
    }
#endif  /* #ifdef R_IIC_OS_USE */
    
    return Ret;

}

/*------------------------------------------------------------------------------------------------------------------*/
error_t r_iic_drv_masterrx(r_iic_drv_info_t *pRIic_Info)
{
    error_t     Ret = R_IIC_ERR_OTHER;                          /* Sets R_IIC_ERR_OTHER as initialization of Ret.   */
#ifdef R_IIC_OS_USE
/* Please add internal variable. */
#endif  /* #ifdef R_IIC_OS_USE */

    /* Sets interruption disabled. */
    r_iic_drv_int_maskable_disable();

    /* Checks channel status and Updates channel status flag. */
    Ret = r_iic_drv_check_chstatus_start(pRIic_Info);
    if ((R_IIC_IDLE != Ret) &&
        (R_IIC_FINISH != Ret) &&
        (R_IIC_NACK != Ret)         )
    {
        /* Sets interruption enabled. */
        r_iic_drv_int_maskable_enable();

        return Ret;
    }

    /* Updates channel status flag. */
    r_iic_drv_set_ch_status(pRIic_Info, R_IIC_COMMUNICATION);

    /* Sets interruption enabled. */
    r_iic_drv_int_maskable_enable();
    
    /* Checks bus busy. */
    if (R_IIC_FALSE == r_iic_drv_check_bus_busy(pRIic_Info))
    {
        /* Sets interruption disabled. */
        r_iic_drv_int_maskable_disable();

        /* Updates channel status flag. */
        r_iic_drv_set_ch_status(pRIic_Info, R_IIC_ERR_NON_REPLY);

        /* Sets interruption enabled. */
        r_iic_drv_int_maskable_enable();

        return R_IIC_ERR_NON_REPLY;
    }

    /* RAM initialization */
    r_iic_drv_ram_init(pRIic_Info);

    /* Sets internal mode flag. */
    g_iic_InternalInfo[pRIic_Info->ChNo].Mode = R_IIC_MODE_READ;

#ifdef R_IIC_OS_USE
/* Please clear event flag. */
#endif  /* #ifdef R_IIC_OS_USE */

    /* Generates start condition. */
    Ret = r_iic_drv_func_table(R_IIC_EV_GEN_START_COND, pRIic_Info);
    if (R_IIC_COMMUNICATION != Ret)
    {
        /* Terminates processing at error. */
        r_iic_drv_err_fin(pRIic_Info);

        /* Sets interruption disabled. */
        r_iic_drv_int_maskable_disable();

        /* Updates channel status flag. */
        r_iic_drv_set_ch_status(pRIic_Info, R_IIC_ERR_NON_REPLY);

        /* Sets interruption enabled. */
        r_iic_drv_int_maskable_enable();

        return R_IIC_ERR_NON_REPLY;
    }
    
    /* Transmits slave address. */
    Ret = r_iic_drv_func_table(R_IIC_EV_RE_SEND_SLVADR, pRIic_Info);
    if (R_IIC_COMMUNICATION != Ret)
    {
        /* Terminates processing at error. */
        r_iic_drv_err_fin(pRIic_Info);

        /* Sets interruption disabled. */
        r_iic_drv_int_maskable_disable();

        /* Updates channel status flag. */
        r_iic_drv_set_ch_status(pRIic_Info, R_IIC_ERR_OTHER);

        /* Sets interruption enabled. */
        r_iic_drv_int_maskable_enable();

        return R_IIC_ERR_OTHER;
    }
    
    /* Counter initialization for non-reply error */
    g_iic_ReplyCnt[pRIic_Info->ChNo] = REPLY_CNT;
    
    return R_IIC_COMMUNICATION;
}

/********************************************************************************************************************
* Function Name : R_IIC_Drv_MasterTRx
* Description   : Master combination start processing
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : R_IIC_COMMUNICATION               ;   Successful operation
*               : R_IIC_NO_INIT                     ;   Uninitialized state
*               : R_IIC_LOCK_FUNC                   ;   Other task is handling API function
*               : R_IIC_BUS_BUSY                    ;   Bus busy
*               : 
*               : R_IIC_ERR_PARAM                   ;   Parameter error
*               : R_IIC_ERR_AL                      ;   Arbitration lost error
*               : R_IIC_ERR_NON_REPLY               ;   None reply error
*               : R_IIC_ERR_SDA_LOW_HOLD            ;   SDA Low hold error
*               ; R_IIC_ERR_OTHER                   ;   Other error
*********************************************************************************************************************/
error_t R_IIC_Drv_MasterTRx(r_iic_drv_info_t *pRIic_Info)
{
    bool        Chk = R_IIC_FALSE;
    error_t     Ret = R_IIC_ERR_OTHER;                          /* Sets R_IIC_ERR_OTHER as initialization of Ret.   */
#ifdef R_IIC_OS_USE
/* Please add internal variable. */
#endif  /* #ifdef R_IIC_OS_USE */

    /* Is "pRIic_Info" setting?  */
    if (NULL == pRIic_Info)
    {
        return R_IIC_ERR_PARAM;
    }

#ifdef R_IIC_OS_USE
/* Please get semaphore resource. */
#endif  /* #ifdef R_IIC_OS_USE */
    
    /* Sets interruption disabled. */
    r_iic_drv_int_maskable_disable();

    /* Checks API flag. */
    Chk = r_iic_drv_lock_api(pRIic_Info);

    /* Sets interruption enabled. */
    r_iic_drv_int_maskable_enable();

    if (R_IIC_FALSE == Chk)
    {
        /* Lock has already been acquired by other task. Need to try again later.   */
        return R_IIC_LOCK_FUNC;
    }

    /* Calls API function. */
    Ret = r_iic_drv_mastertrx(pRIic_Info);

    /* Checks API flag. */
    r_iic_drv_unlock_api(pRIic_Info);
    
#ifdef R_IIC_OS_USE
    if (R_IIC_COMMUNICATION != Ret)
    {
/* Please release semaphore resource. */
    }
#endif  /* #ifdef R_IIC_OS_USE */

    return Ret;

}

/*------------------------------------------------------------------------------------------------------------------*/
error_t r_iic_drv_mastertrx(r_iic_drv_info_t *pRIic_Info)
{
    error_t     Ret = R_IIC_ERR_OTHER;                          /* Sets R_IIC_ERR_OTHER as initialization of Ret.   */
#ifdef R_IIC_OS_USE
/* Please add internal variable. */
#endif  /* #ifdef R_IIC_OS_USE */

    /* Sets interruption disabled. */
    r_iic_drv_int_maskable_disable();

    /* Checks channel status and Updates channel status flag. */
    Ret = r_iic_drv_check_chstatus_start(pRIic_Info);
    if ((R_IIC_IDLE != Ret) &&
        (R_IIC_FINISH != Ret) &&
        (R_IIC_NACK != Ret)         )
    {
        /* Sets interruption enabled. */
        r_iic_drv_int_maskable_enable();

        return Ret;
    }

    /* Updates channel status flag. */
    r_iic_drv_set_ch_status(pRIic_Info, R_IIC_COMMUNICATION);

    /* Sets interruption enabled. */
    r_iic_drv_int_maskable_enable();

    /* Checks bus busy. */
    if (R_IIC_FALSE == r_iic_drv_check_bus_busy(pRIic_Info))
    {
        /* Sets interruption disabled. */
        r_iic_drv_int_maskable_disable();

        /* Updates channel status flag. */
        r_iic_drv_set_ch_status(pRIic_Info, R_IIC_ERR_NON_REPLY);

        /* Sets interruption enabled. */
        r_iic_drv_int_maskable_enable();

        return R_IIC_ERR_NON_REPLY;
    }
    
    /* RAM initialization */
    r_iic_drv_ram_init(pRIic_Info);

    /* Sets internal mode flag. */
    g_iic_InternalInfo[pRIic_Info->ChNo].Mode = R_IIC_MODE_COMBINED;

#ifdef R_IIC_OS_USE
/* Please clear event flag. */
#endif  /* #ifdef R_IIC_OS_USE */
    
    /* Generates start condition. */
    Ret = r_iic_drv_func_table(R_IIC_EV_GEN_START_COND, pRIic_Info);
    if (R_IIC_COMMUNICATION != Ret)
    {
        /* Terminates processing at error. */
        r_iic_drv_err_fin(pRIic_Info);

        /* Sets interruption disabled. */
        r_iic_drv_int_maskable_disable();

        /* Updates channel status flag. */
        r_iic_drv_set_ch_status(pRIic_Info, R_IIC_ERR_NON_REPLY);

        /* Sets interruption enabled. */
        r_iic_drv_int_maskable_enable();

        return R_IIC_ERR_NON_REPLY;
    }
    
    /* Transmits slave address. */
    Ret = r_iic_drv_func_table(R_IIC_EV_SEND_SLVADR, pRIic_Info);
    if (R_IIC_COMMUNICATION != Ret)
    {
        /* Terminates processing at error. */
        r_iic_drv_err_fin(pRIic_Info);

        /* Sets interruption disabled. */
        r_iic_drv_int_maskable_disable();

        /* Updates channel status flag. */
        r_iic_drv_set_ch_status(pRIic_Info, R_IIC_ERR_OTHER);

        /* Sets interruption enabled. */
        r_iic_drv_int_maskable_enable();

        return R_IIC_ERR_OTHER;
    }

    /* Counter initialization for non-reply error */
    g_iic_ReplyCnt[pRIic_Info->ChNo] = REPLY_CNT;
    
    return R_IIC_COMMUNICATION;
}

/********************************************************************************************************************
* Function Name : R_IIC_Drv_Advance
* Description   : Advance processing
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : R_IIC_COMMUNICATION               ;   Successful operation ,not finished communication
*               : R_IIC_FINISH                      ;   Successful operation ,finished communication
*               : R_IIC_NACK                        ;   Detected NACK and finished communication
*               : R_IIC_NO_INIT                     ;   Uninitialized state
*               : R_IIC_IDLE                        ;   Not started state
*               : R_IIC_LOCK_FUNC                   ;   Other task is handling API function
*               : R_IIC_BUS_BUSY                    ;   Bus busy
*               : 
*               : R_IIC_ERR_PARAM                   ;   Parameter error
*               : R_IIC_ERR_AL                      ;   Arbitration lost error
*               : R_IIC_ERR_NON_REPLY               ;   None reply error
*               : R_IIC_ERR_SDA_LOW_HOLD            ;   SDA Low hold error
*               ; R_IIC_ERR_OTHER                   ;   Other error
*********************************************************************************************************************/
error_t R_IIC_Drv_Advance(r_iic_drv_info_t *pRIic_Info)
{
    bool        Chk = R_IIC_FALSE;
    error_t     Ret = R_IIC_ERR_OTHER;                          /* Sets R_IIC_ERR_OTHER as initialization of Ret.   */
#ifdef R_IIC_OS_USE
/* Please add internal variable. */
#endif  /* #ifdef R_IIC_OS_USE */

    /* Is "pRIic_Info" setting?  */
    if (NULL == pRIic_Info)
    {
        return R_IIC_ERR_PARAM;
    }
    
#ifdef R_IIC_OS_USE
/* Please clear the user's original flag. */
#endif  /* #ifdef R_IIC_OS_USE */

    /* Sets interruption disabled. */
    r_iic_drv_int_maskable_disable();

    /* Checks API flag. */
    Chk = r_iic_drv_lock_api(pRIic_Info);

    /* Sets interruption enabled. */
    r_iic_drv_int_maskable_enable();

    if (R_IIC_FALSE == Chk)
    {
        /* Lock has already been acquired by other task. Need to try again later.   */
        return R_IIC_LOCK_FUNC;
    }

    /* Calls API function. */
    Ret = r_iic_drv_advance(pRIic_Info);

    /* Checks API flag. */
    r_iic_drv_unlock_api(pRIic_Info);

#ifdef R_IIC_OS_USE
/* Please check the user's original flag.
   If set the flag, please clear user's original flag and release semaphore resource. */
#endif  /* #ifdef R_IIC_OS_USE */
    
    return Ret;

}

/*------------------------------------------------------------------------------------------------------------------*/
error_t r_iic_drv_advance(r_iic_drv_info_t *pRIic_Info)
{
    error_t     Ret = R_IIC_ERR_OTHER;                          /* Sets R_IIC_ERR_OTHER as initialization of Ret.   */
    bool        boolRet = R_IIC_FALSE;
#ifdef R_IIC_OS_USE
/* Please add internal variable. */
#endif  /* #ifdef R_IIC_OS_USE */

    /* Checks channel status. */
    Ret = r_iic_drv_check_chstatus_advance(pRIic_Info);
    if (R_IIC_COMMUNICATION != Ret)
    {
        return Ret;
    }
    
#ifdef R_IIC_OS_USE
/* Please set waiting event flag. */
#endif  /* #ifdef R_IIC_OS_USE */
    
    /* Event happened? */
    if (R_IIC_EV_INIT != g_iic_Event[pRIic_Info->ChNo])
    {
        /* Counter initialization for non-reply error */
        g_iic_ReplyCnt[pRIic_Info->ChNo] = REPLY_CNT;
        
        /* IIC communication processing. */
        Ret = r_iic_drv_func_table(g_iic_Event[pRIic_Info->ChNo], pRIic_Info);
        
        /* Return value? */
        switch(Ret)
        {
            /* Advanced communication. (Not finished) */
            case R_IIC_COMMUNICATION:
                return Ret;
            break;
            
            /* Finished communication. */
            case R_IIC_FINISH:
                
                /* Sets interruption disabled. */
                r_iic_drv_int_maskable_disable();

                /* Updates channel status flag. */
                r_iic_drv_set_ch_status(pRIic_Info, R_IIC_FINISH);

                /* Sets interruption enabled. */
                r_iic_drv_int_maskable_enable();

#ifdef R_IIC_OS_USE
/* Please set the user's original flag. */
#endif  /* #ifdef R_IIC_OS_USE */

                /* Checks callback function. */
                if (NULL != R_IIC_Drv_CallBackFunc[pRIic_Info->ChNo])
                {
                    /* Calls callback function. */
                    R_IIC_Drv_CallBackFunc[pRIic_Info->ChNo]();
                }

                /* IIC finish. */
                return Ret;
                
            break;
            
            /* NACK is occurred. */
            case R_IIC_NACK:
                
                /* Waits stop condition generation. */
                boolRet = r_iic_drv_stop_cond_wait(pRIic_Info);
                
                /* Terminates processing at error. */
                r_iic_drv_err_fin(pRIic_Info);

                /* Check stop condition generation. */
                if (R_IIC_TRUE == boolRet)
                {
                    /* Sets interruption disabled. */
                    r_iic_drv_int_maskable_disable();

                    /* Updates channel status flag. */
                    r_iic_drv_set_ch_status(pRIic_Info, R_IIC_NACK);

                    /* Sets interruption enabled. */
                    r_iic_drv_int_maskable_enable();
                    
                    Ret = R_IIC_NACK;
                }
                else
                {
                    /* Sets interruption disabled. */
                    r_iic_drv_int_maskable_disable();

                    /* Updates channel status flag. */
                    r_iic_drv_set_ch_status(pRIic_Info, R_IIC_ERR_NON_REPLY);

                    /* Sets interruption enabled. */
                    r_iic_drv_int_maskable_enable();
                    
                    Ret = R_IIC_ERR_NON_REPLY;
                }

#ifdef R_IIC_OS_USE
/* Please set the user's original flag. */
#endif  /* #ifdef R_IIC_OS_USE */

                /* Checks callback function. */
                if (NULL != R_IIC_Drv_CallBackFunc[pRIic_Info->ChNo])
                {
                    /* Calls callback function. */
                    R_IIC_Drv_CallBackFunc[pRIic_Info->ChNo]();
                }

                return Ret;

            break;

            default:
            
                /* Terminates processing at error. */
                r_iic_drv_err_fin(pRIic_Info);

                /* Checks return value. */
                if (R_IIC_ERR_AL == Ret)
                {
                    /* Sets interruption disabled. */
                    r_iic_drv_int_maskable_disable();

                    /* Updates channel status flag. */
                    r_iic_drv_set_ch_status(pRIic_Info, R_IIC_ERR_AL);

                    /* Sets interruption enabled. */
                    r_iic_drv_int_maskable_enable();
                }
                else if (R_IIC_ERR_OTHER == Ret)
                {
                    /* Sets interruption disabled. */
                    r_iic_drv_int_maskable_disable();

                    /* Updates channel status flag. */
                    r_iic_drv_set_ch_status(pRIic_Info, R_IIC_ERR_OTHER);

                    /* Sets interruption enabled. */
                    r_iic_drv_int_maskable_enable();
                }
                else
                {
                    /* Does nothing. */
                }

#ifdef R_IIC_OS_USE
/* Please set the user's original flag. */
#endif  /* #ifdef R_IIC_OS_USE */

                /* Checks callback function. */
                if (NULL != R_IIC_Drv_CallBackFunc[pRIic_Info->ChNo])
                {
                    /* Calls callback function. */
                    R_IIC_Drv_CallBackFunc[pRIic_Info->ChNo]();
                }

                return Ret;
                
            break;
        }
    }
    else    /* Event nothing. */
    {
        /* Decreases non-reply counter. */
        g_iic_ReplyCnt[pRIic_Info->ChNo]--;
        
        /* Counter limitation? */
        if (0 == g_iic_ReplyCnt[pRIic_Info->ChNo])
        {
            /* Terminates processing at error. */
            r_iic_drv_err_fin(pRIic_Info);

            /* Sets interruption disabled. */
            r_iic_drv_int_maskable_disable();

            /* Updates channel status flag. */
            r_iic_drv_set_ch_status(pRIic_Info, R_IIC_ERR_NON_REPLY);

            /* Sets interruption enabled. */
            r_iic_drv_int_maskable_enable();

#ifdef R_IIC_OS_USE
/* Please set the user's original flag. */
#endif  /* #ifdef R_IIC_OS_USE */

            /* Checks callback function. */
            if (NULL != R_IIC_Drv_CallBackFunc[pRIic_Info->ChNo])
            {
                /* Calls callback function. */
                R_IIC_Drv_CallBackFunc[pRIic_Info->ChNo]();
            }

            /* Non-reply error and stop condition generation failure */
            return R_IIC_ERR_NON_REPLY;
        }
        
        /* During communication */
        return R_IIC_COMMUNICATION;
    }
    
}

/********************************************************************************************************************
* Function Name : R_IIC_Drv_GenClk
* Description   : SCL clock generation processing
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
*               : uint8_t ClkCnt                    ;   SCL clock counter
* Return Value  : R_IIC_NO_INIT                     ;   Successful operation
*               : R_IIC_LOCK_FUNC                   ;   Other task is handling API function
*               : 
*               : R_IIC_ERR_PARAM                   ;   Parameter error
*               : R_IIC_ERR_SDA_LOW_HOLD            ;   SDA Low hold error
*               ; R_IIC_ERR_OTHER                   ;   Other error
*********************************************************************************************************************/
error_t R_IIC_Drv_GenClk(r_iic_drv_info_t *pRIic_Info, uint8_t ClkCnt)
{
    bool        Chk = R_IIC_FALSE;
    error_t     Ret = R_IIC_ERR_OTHER;                          /* Sets R_IIC_ERR_OTHER as initialization of Ret.   */

    /* Is "pRIic_Info" setting? */
    if (NULL == pRIic_Info)
    {
        return R_IIC_ERR_PARAM;
    }

    /* Sets interruption disabled. */
    r_iic_drv_int_maskable_disable();

    /* Checks API flag. */
    Chk = r_iic_drv_lock_api(pRIic_Info);

    /* Sets interruption enabled. */
    r_iic_drv_int_maskable_enable();

    if (R_IIC_FALSE == Chk)
    {
        /* Lock has already been acquired by other task. Need to try again later.   */
        return R_IIC_LOCK_FUNC;
    }

    /* Calls API function. */
    Ret = r_iic_drv_genclk(pRIic_Info, ClkCnt);

    /* Checks API flag. */
    r_iic_drv_unlock_api(pRIic_Info);
    
    return Ret;

}

/*------------------------------------------------------------------------------------------------------------------*/
error_t r_iic_drv_genclk(r_iic_drv_info_t *pRIic_Info, uint8_t ClkCnt)
{
    /* Sets port for checking SDA. */
    r_iic_drv_io_chk_sda_setting(pRIic_Info);

    /* Checks SDA level. */
    do
    {
        /* SDA = High? */
        if (R_IIC_TRUE == r_iic_drv_check_sda_level(pRIic_Info))
        {
            /* SDA = High */
            break;
        }
        else
        {
            /* SCL clock generation.(Low->High) */
            r_iic_drv_clk_low(pRIic_Info);                      /* Port = Low                                   */
            r_iic_drv_clk_high(pRIic_Info);                     /* Port = Hi-z                                  */
        }
        ClkCnt--;
    }
    while(ClkCnt != 0);

    /* SDA = Low? */
    if (0 != ClkCnt)
    {
        /* Sets interruption disabled. */
        r_iic_drv_int_maskable_disable();

        /* Updates channel status flag. */
        r_iic_drv_set_ch_status(pRIic_Info, R_IIC_NO_INIT);

        /* Sets interruption enabled. */
        r_iic_drv_int_maskable_enable();

        return R_IIC_NO_INIT;                                   /* Successful operation.                        */
    }
    else
    {
        /* Sets interruption disabled. */
        r_iic_drv_int_maskable_disable();

        /* Updates channel status flag. */
        r_iic_drv_set_ch_status(pRIic_Info, R_IIC_ERR_SDA_LOW_HOLD);

        /* Sets interruption enabled. */
        r_iic_drv_int_maskable_enable();

        return R_IIC_ERR_SDA_LOW_HOLD;                          /* Failed operation.                            */
    }

}


/********************************************************************************************************************
* Function Name : R_IIC_Drv_Reset
* Description   : Driver reset processing
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : R_IIC_NO_INIT                     ;   Successful operation
*               : R_IIC_LOCK_FUNC                   ;   Other task is handling API function
*               : 
*               : R_IIC_ERR_PARAM                   ;   Parameter error
*********************************************************************************************************************/
error_t R_IIC_Drv_Reset(r_iic_drv_info_t *pRIic_Info)
{
    bool        Chk = R_IIC_FALSE;
    error_t     Ret = R_IIC_ERR_OTHER;                          /* Sets R_IIC_ERR_OTHER as initialization of Ret.   */
    
    /* Is "pRIic_Info" setting?  */
    if (NULL == pRIic_Info)
    {
        return R_IIC_ERR_PARAM;
    }

    /* Sets interruption disabled. */
    r_iic_drv_int_maskable_disable();

    /* Checks API flag. */
    Chk = r_iic_drv_lock_api(pRIic_Info);

    /* Sets interruption enabled. */
    r_iic_drv_int_maskable_enable();

    if (R_IIC_FALSE == Chk)
    {
        /* Lock has already been acquired by other task. Need to try again later.   */
        return R_IIC_LOCK_FUNC;
    }

    /* Calls API function. */
    Ret = r_iic_drv_reset(pRIic_Info);

    /* Checks API flag. */
    r_iic_drv_unlock_api(pRIic_Info);
    
    return Ret;

}

/*------------------------------------------------------------------------------------------------------------------*/
error_t r_iic_drv_reset(r_iic_drv_info_t *pRIic_Info)
{
    /* Sets interruption disabled. */
    r_iic_drv_int_maskable_disable();

    /* Updates channel status flag. */
    r_iic_drv_set_ch_status(pRIic_Info, R_IIC_NO_INIT);

    /* Sets interruption enabled. */
    r_iic_drv_int_maskable_enable();

    /* Terminates processing at error. */
    r_iic_drv_err_fin(pRIic_Info);

    /* Resets IIC Interface. */
    r_iic_drv_reset_iic(pRIic_Info);

    return R_IIC_NO_INIT;

}



/* End of File */
