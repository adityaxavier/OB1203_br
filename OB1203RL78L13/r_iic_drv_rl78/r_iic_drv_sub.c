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
* File Name     : r_iic_drv_sub.c
* Version       : 1.03
* Device(s)     : RL78/G14
* Tool-Chain    : -
*               : 
* OS            : -
* H/W Platform  : -
* Description   : IIC driver IICA internal module
* Operation     : -
* Limitations   : -
*********************************************************************************************************************
* History       : DD.MM.YYYY    Version Description
*               : 31.05.2012    1.00    Created
*               : 07.03.2013    1.01    Ver.1.01 Release
*               : 31.03.2014    1.02    Supported IAR Embedded Workbench for Renesas RL78.
*               : 31.03.2016    1.03    Changed r_iic_drv_send_slvadr().
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


/********************************************************************************************************************
Private global variables and functions
*********************************************************************************************************************/

/* function table */
static const r_iic_drv_mtx_t gc_iic_mtx_tbl[R_IIC_STS_MAX][R_IIC_EV_MAX] =
{
    /* Uninitialized state */
    {
        { R_IIC_EV_INIT                 ,   r_iic_drv_init_driver           } , /* Called function of Initializes IIC driver        */
        { R_IIC_EV_GEN_START_COND       ,   NULL                            } , /* Called function of Start condition generation    */
        { R_IIC_EV_SEND_SLVADR          ,   NULL                            } , /* Called function of Slave address sending         */
        { R_IIC_EV_RE_SEND_SLVADR       ,   NULL                            } , /* Called function of Re-slave address sending      */
        { R_IIC_EV_INT_ADD              ,   NULL                            } , /* Interrupted address sending                      */
        { R_IIC_EV_INT_SEND             ,   NULL                            } , /* Interrupted data sending                         */
        { R_IIC_EV_INT_RECEIVE          ,   NULL                            } , /* Interrupted data receiving                       */
        { R_IIC_EV_INT_STOP             ,   NULL                            } , /* Interrupted stop condition generation            */
        { R_IIC_EV_INT_AL               ,   NULL                            } , /* Interrupted Arbitration-Lost                     */
        { R_IIC_EV_INT_NACK             ,   NULL                            }   /* Interrupted No Acknowledge                       */
    },
    
    /* Idle state */
    {
        { R_IIC_EV_INIT                 ,   NULL                            } , /* Called function of Initializes IIC driver        */
        { R_IIC_EV_GEN_START_COND       ,   r_iic_drv_generate_start_cond   } , /* Called function of Start condition generation    */
        { R_IIC_EV_SEND_SLVADR          ,   NULL                            } , /* Called function of Slave address sending         */
        { R_IIC_EV_RE_SEND_SLVADR       ,   NULL                            } , /* Called function of Re-slave address sending      */
        { R_IIC_EV_INT_ADD              ,   NULL                            } , /* Interrupted address sending                      */
        { R_IIC_EV_INT_SEND             ,   NULL                            } , /* Interrupted data sending                         */
        { R_IIC_EV_INT_RECEIVE          ,   NULL                            } , /* Interrupted data receiving                       */
        { R_IIC_EV_INT_STOP             ,   NULL                            } , /* Interrupted stop condition generation            */
        { R_IIC_EV_INT_AL               ,   NULL                            } , /* Interrupted Arbitration-Lost                     */
        { R_IIC_EV_INT_NACK             ,   NULL                            }   /* Interrupted No Acknowledge                       */
    },
    
    /* Start condition generation completion wait state */
    {
        { R_IIC_EV_INIT                 ,   NULL                            } , /* Called function of Initializes IIC driver        */
        { R_IIC_EV_GEN_START_COND       ,   NULL                            } , /* Called function of Start condition generation    */
        { R_IIC_EV_SEND_SLVADR          ,   r_iic_drv_send_slvadr           } , /* Called function of Slave address sending         */
        { R_IIC_EV_RE_SEND_SLVADR       ,   r_iic_drv_re_send_slvadr        } , /* Called function of Re-slave address sending      */
        { R_IIC_EV_INT_ADD              ,   NULL                            } , /* Interrupted address sending                      */
        { R_IIC_EV_INT_SEND             ,   NULL                            } , /* Interrupted data sending                         */
        { R_IIC_EV_INT_RECEIVE          ,   NULL                            } , /* Interrupted data receiving                       */
        { R_IIC_EV_INT_STOP             ,   NULL                            } , /* Interrupted stop condition generation            */
        { R_IIC_EV_INT_AL               ,   NULL                            } , /* Interrupted Arbitration-Lost                     */
        { R_IIC_EV_INT_NACK             ,   NULL                            }   /* Interrupted No Acknowledge                       */
    },
    
    /* Slave address [Write] transmission completion wait state */
    {
        { R_IIC_EV_INIT                 ,   NULL                            } , /* Called function of Initializes IIC driver        */
        { R_IIC_EV_GEN_START_COND       ,   NULL                            } , /* Called function of Start condition generation    */
        { R_IIC_EV_SEND_SLVADR          ,   NULL                            } , /* Called function of Slave address sending         */
        { R_IIC_EV_RE_SEND_SLVADR       ,   NULL                            } , /* Called function of Re-slave address sending      */
        { R_IIC_EV_INT_ADD              ,   r_iic_drv_after_send_slvadr     } , /* Interrupted address sending                      */
        { R_IIC_EV_INT_SEND             ,   NULL                            } , /* Interrupted data sending                         */
        { R_IIC_EV_INT_RECEIVE          ,   NULL                            } , /* Interrupted data receiving                       */
        { R_IIC_EV_INT_STOP             ,   NULL                            } , /* Interrupted stop condition generation            */
        { R_IIC_EV_INT_AL               ,   r_iic_drv_arbitration_lost      } , /* Interrupted Arbitration-Lost                     */
        { R_IIC_EV_INT_NACK             ,   r_iic_drv_nack                  }   /* Interrupted No Acknowledge                       */
    },
    
    /* Slave address [Read] transmission completion wait state */
    {
        { R_IIC_EV_INIT                 ,   NULL                            } , /* Called function of Initializes IIC driver        */
        { R_IIC_EV_GEN_START_COND       ,   NULL                            } , /* Called function of Start condition generation    */
        { R_IIC_EV_SEND_SLVADR          ,   NULL                            } , /* Called function of Slave address sending         */
        { R_IIC_EV_RE_SEND_SLVADR       ,   NULL                            } , /* Called function of Re-slave address sending      */
        { R_IIC_EV_INT_ADD              ,   r_iic_drv_after_send_slvadr     } , /* Interrupted address sending                      */
        { R_IIC_EV_INT_SEND             ,   NULL                            } , /* Interrupted data sending                         */
        { R_IIC_EV_INT_RECEIVE          ,   NULL                            } , /* Interrupted data receiving                       */
        { R_IIC_EV_INT_STOP             ,   NULL                            } , /* Interrupted stop condition generation            */
        { R_IIC_EV_INT_AL               ,   r_iic_drv_arbitration_lost      } , /* Interrupted Arbitration-Lost                     */
        { R_IIC_EV_INT_NACK             ,   r_iic_drv_nack                  }   /* Interrupted No Acknowledge                       */
    },
    
    /* Data transmission completion wait state */
    {
        { R_IIC_EV_INIT                 ,   NULL                            } , /* Called function of Initializes IIC driver        */
        { R_IIC_EV_GEN_START_COND       ,   NULL                            } , /* Called function of Start condition generation    */
        { R_IIC_EV_SEND_SLVADR          ,   NULL                            } , /* Called function of Slave address sending         */
        { R_IIC_EV_RE_SEND_SLVADR       ,   NULL                            } , /* Called function of Re-slave address sending      */
        { R_IIC_EV_INT_ADD              ,   NULL                            } , /* Interrupted address sending                      */
        { R_IIC_EV_INT_SEND             ,   r_iic_drv_write_data_sending    } , /* Interrupted data sending                         */
        { R_IIC_EV_INT_RECEIVE          ,   NULL                            } , /* Interrupted data receiving                       */
        { R_IIC_EV_INT_STOP             ,   NULL                            } , /* Interrupted stop condition generation            */
        { R_IIC_EV_INT_AL               ,   r_iic_drv_arbitration_lost      } , /* Interrupted Arbitration-Lost                     */
        { R_IIC_EV_INT_NACK             ,   r_iic_drv_nack                  }   /* Interrupted No Acknowledge                       */
    },
    
    /* Data reception completion wait state */
    {
        { R_IIC_EV_INIT                 ,   NULL                            } , /* Called function of Initializes IIC driver        */
        { R_IIC_EV_GEN_START_COND       ,   NULL                            } , /* Called function of Start condition generation    */
        { R_IIC_EV_SEND_SLVADR          ,   NULL                            } , /* Called function of Slave address sending         */
        { R_IIC_EV_RE_SEND_SLVADR       ,   NULL                            } , /* Called function of Re-slave address sending      */
        { R_IIC_EV_INT_ADD              ,   NULL                            } , /* Interrupted address sending                      */
        { R_IIC_EV_INT_SEND             ,   NULL                            } , /* Interrupted data sending                         */
        { R_IIC_EV_INT_RECEIVE          ,   r_iic_drv_read_data_receiving   } , /* Interrupted data receiving                       */
        { R_IIC_EV_INT_STOP             ,   NULL                            } , /* Interrupted stop condition generation            */
        { R_IIC_EV_INT_AL               ,   r_iic_drv_arbitration_lost      } , /* Interrupted Arbitration-Lost                     */
        { R_IIC_EV_INT_NACK             ,   r_iic_drv_nack                  }   /* Interrupted No Acknowledge                       */
    },
    
    /* stop condition generation completion wait state */
    {
        { R_IIC_EV_INIT                 ,   NULL                            } , /* Called function of Initializes IIC driver        */
        { R_IIC_EV_GEN_START_COND       ,   NULL                            } , /* Called function of Start condition generation    */
        { R_IIC_EV_SEND_SLVADR          ,   NULL                            } , /* Called function of Slave address sending         */
        { R_IIC_EV_RE_SEND_SLVADR       ,   NULL                            } , /* Called function of Re-slave address sending      */
        { R_IIC_EV_INT_ADD              ,   NULL                            } , /* Interrupted address sending                      */
        { R_IIC_EV_INT_SEND             ,   NULL                            } , /* Interrupted data sending                         */
        { R_IIC_EV_INT_RECEIVE          ,   NULL                            } , /* Interrupted data receiving                       */
        { R_IIC_EV_INT_STOP             ,   r_iic_drv_release               } , /* Interrupted stop condition generation            */
        { R_IIC_EV_INT_AL               ,   r_iic_drv_arbitration_lost      } , /* Interrupted Arbitration-Lost                     */
        { R_IIC_EV_INT_NACK             ,   r_iic_drv_nack                  }   /* Interrupted No Acknowledge                       */
    },

};


/********************************************************************************************************************
* Function Name : r_iic_drv_func_table
* Description   : IIC protocol function
* Arguments     : r_iic_drv_internal_event_t event  ;   Event
*               : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : R_IIC_IDLE                        ;   Successful operation, idle state
*               : R_IIC_COMMUNICATION               ;   Successful operation, communication state
*               : R_IIC_FINISH                      ;   Successful operation, finished communication and idle state
*               : R_IIC_NACK                        ;   Detected NACK, finished communication and idle state
*               :
*               : R_IIC_ERR_AL                      ;   Arbitration lost error
*               : R_IIC_ERR_NON_REPLY               ;   None reply error
*               : R_IIC_ERR_OTHER                   ;   Other error
*********************************************************************************************************************/
error_t r_iic_drv_func_table(
    r_iic_drv_internal_event_t event,
    r_iic_drv_info_t *pRIic_Info
                                            )
{
    error_t     Ret = R_IIC_ERR_OTHER;
    error_t     (*pFunc)( r_iic_drv_info_t *);
    r_iic_drv_internal_status_t     N_status;

    /* Acquires a now state. */
    N_status = g_iic_InternalInfo[pRIic_Info->ChNo].N_status;

    /* Checks parameter. */
    if ((R_IIC_STS_MAX > N_status) && (R_IIC_EV_MAX > event))
    {
        /* Checks appointed function. */
        if (NULL != gc_iic_mtx_tbl[N_status][event].proc)
        {
            /* Sets function. */
            pFunc = gc_iic_mtx_tbl[N_status][event].proc;
            
            /* Event flag initialization. */
            g_iic_Event[pRIic_Info->ChNo] = R_IIC_EV_INIT;
            
            /* Calls status transition operation. */
            Ret = (*pFunc)(pRIic_Info);
        }
        else
        {
            /* Appointed function error */
            Ret = R_IIC_ERR_OTHER;
        }
    }
    else
    {
        /* Appointed function error */
        Ret = R_IIC_ERR_OTHER;
    }

    return Ret;
}


/********************************************************************************************************************
* Function Name : r_iic_drv_init_driver
* Description   : Initialize IIC driver
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : R_IIC_IDLE                        ;   Successful operation, idle state
*********************************************************************************************************************/
error_t r_iic_drv_init_driver(r_iic_drv_info_t *pRIic_Info)
{
    error_t     Ret = R_IIC_IDLE;

    /* IIC register initialization */
    r_iic_drv_per_enable(pRIic_Info);                       /* Enables IIC Peripheral register.                     */
    r_iic_drv_init_sfr(pRIic_Info);                         /* Initializes IIC register.                            */
    r_iic_drv_set_internal_status(pRIic_Info, R_IIC_STS_IDLE);      /* Sets internal status.                        */
    
    return Ret;
    
}


/********************************************************************************************************************
* Function Name : r_iic_drv_generate_start_cond
* Description   : Generates start condition.
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : R_IIC_COMMUNICATION               ;   Successful operation, communication state
*               :
*               : R_IIC_ERR_NON_REPLY               ;   None repley error
*********************************************************************************************************************/
error_t r_iic_drv_generate_start_cond(r_iic_drv_info_t *pRIic_Info)
{
    error_t     Ret;
    bool        boolret;

    /* Enables interruption. */
    r_iic_drv_int_enable(pRIic_Info);

    /* Initializes port setting. */
    /* Sets the port from input mode to output mode and enable the output of the I2C bus. */
    r_iic_drv_io_init(pRIic_Info);
    
    /* Sets internal status. */
    r_iic_drv_set_internal_status(pRIic_Info, R_IIC_STS_ST_COND_WAIT);

    /* Generates start condition. */
    r_iic_drv_start_cond_generate(pRIic_Info);
    
    /* Checks start condition generation. */
    boolret = r_iic_drv_start_cond_check(pRIic_Info);
    if (R_IIC_TRUE == boolret)
    {
        Ret = R_IIC_COMMUNICATION;
    }
    else
    {
        Ret = R_IIC_ERR_NON_REPLY;
    }

    return Ret;
}


/********************************************************************************************************************
* Function Name : r_iic_drv_send_slvadr
* Description   : Transmits slave address for writing data.
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : R_IIC_COMMUNICATION               ;   Successful operation, communication state
*********************************************************************************************************************/
error_t r_iic_drv_send_slvadr(r_iic_drv_info_t *pRIic_Info)
{
    error_t     Ret = R_IIC_COMMUNICATION;

    /* Is slave address pointer set? */
    if (NULL == pRIic_Info->pSlvAdr)                                /* Pattern 4 of Master Write                    */
    {
        /* Sets internal status. */
        r_iic_drv_set_internal_status(pRIic_Info, R_IIC_STS_SP_COND_WAIT);

        /* Check SCL line. */
        if (r_iic_drv_check_scl_level(pRIic_Info) == R_IIC_FALSE)
        {
            return R_IIC_ERR_OTHER;
        }

        /* Generates stop condition. */
        r_iic_drv_stop_cond_generate(pRIic_Info);
        
        return Ret;
    }

    /* Sets write code. */
    *(pRIic_Info->pSlvAdr) &= ~R_CODE;

    /* Sets internal status. */
    r_iic_drv_set_internal_status(pRIic_Info, R_IIC_STS_SEND_SLVADR_W_WAIT);

    /* Transmits slave address. */
    r_iic_drv_set_sending_data(pRIic_Info, pRIic_Info->pSlvAdr);

    return Ret;
}


/********************************************************************************************************************
* Function Name : r_iic_drv_re_send_slvadr
* Description   : Transmits slave address for reading data.
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : R_IIC_COMMUNICATION               ;   Successful operation, communication state
*********************************************************************************************************************/
error_t r_iic_drv_re_send_slvadr(r_iic_drv_info_t *pRIic_Info)
{
    error_t     Ret = R_IIC_COMMUNICATION;

    /* Sets read code. */
    *(pRIic_Info->pSlvAdr) |= R_CODE;

    /* Sets internal status. */
    r_iic_drv_set_internal_status(pRIic_Info, R_IIC_STS_SEND_SLVADR_R_WAIT);

    /* Transmits slave address. */
    r_iic_drv_set_sending_data(pRIic_Info, pRIic_Info->pSlvAdr);

    return Ret;
}


/********************************************************************************************************************
* Function Name : r_iic_drv_after_send_slvadr
* Description   : After slave address transmission processing
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : R_IIC_COMMUNICATION               ;   Successful operation, communication state
*               :
*               : R_IIC_ERR_OTHER                   ;   Other error
*********************************************************************************************************************/
error_t r_iic_drv_after_send_slvadr(r_iic_drv_info_t *pRIic_Info)
{
    error_t     Ret = R_IIC_COMMUNICATION;

    /* IIC mode? */
    switch (g_iic_InternalInfo[pRIic_Info->ChNo].Mode)
    {
        case R_IIC_MODE_WRITE:
            
            /* Are 1st data and 2nd data pointer set? */
            if ((NULL == pRIic_Info->pData1st) && (NULL == pRIic_Info->pData2nd))   /* Pattern 3 of Master Write    */
            {
                /* Sets internal status. */
                r_iic_drv_set_internal_status(pRIic_Info, R_IIC_STS_SP_COND_WAIT);

                /* Generates stop condition. */
                r_iic_drv_stop_cond_generate(pRIic_Info);
                
                return Ret;
            }
            
            /* Sets internal status. */
            r_iic_drv_set_internal_status(pRIic_Info, R_IIC_STS_SEND_DATA_WAIT);

            /* Is 1st data pointer set? */
            if ((NULL != pRIic_Info->pData1st) && (NULL != pRIic_Info->pData2nd))   /* Pattern 1 of Master Write    */
            {
                /* 1st data counter = 0?  */
                if (0 != pRIic_Info->Cnt1st)
                {
                    /* Transmits 1st data. */
                    r_iic_drv_set_sending_data(pRIic_Info, pRIic_Info->pData1st);
                    
                    /* Decreases 1st data counter. */
                    pRIic_Info->Cnt1st--;
                    
                    /* Increases 1st data buffer. */
                    pRIic_Info->pData1st++;
                }
                else
                {
                    return R_IIC_ERR_OTHER;
                }
            }
            else if ((NULL == pRIic_Info->pData1st) && (NULL != pRIic_Info->pData2nd))  /* Pattern 2 of Master Write. */
            {
                /* Is 2nd data pointer set? */
                if (NULL != pRIic_Info->pData2nd)
                {
                    /* 2nd data counter = 0?  */
                    if (0 != pRIic_Info->Cnt2nd)
                    {
                        /* Transmits 2nd data. */
                        r_iic_drv_set_sending_data(pRIic_Info, pRIic_Info->pData2nd);
                        
                        /* Decreases 2nd data counter. */
                        pRIic_Info->Cnt2nd--;

                        /* Increases 2nd data buffer. */
                        pRIic_Info->pData2nd++;
                        
                    }
                    else
                    {
                        return R_IIC_ERR_OTHER;
                    }
                }
            }
            else
            {
                return R_IIC_ERR_OTHER;
            }
            
        break;
        
        case R_IIC_MODE_READ:

            /* Sets internal status. */
            r_iic_drv_set_internal_status(pRIic_Info, R_IIC_STS_RECEIVE_DATA_WAIT);
            
            /* Reception setting. */
            r_iic_drv_receive_start_setting(pRIic_Info);
            
            /* Cancels wait. */
            /* In this way, releases SCL and starts the reception. */
            r_iic_drv_canceling_wait(pRIic_Info);
            
        break;
        
        case R_IIC_MODE_COMBINED:
            
            /* Now status? */
            switch (g_iic_InternalInfo[pRIic_Info->ChNo].N_status)
            {
                case R_IIC_STS_SEND_SLVADR_W_WAIT:

                    /* Sets internal status. */
                    r_iic_drv_set_internal_status(pRIic_Info, R_IIC_STS_SEND_DATA_WAIT);
                    
                    /* Transmits 1st data. */
                    r_iic_drv_set_sending_data(pRIic_Info, pRIic_Info->pData1st);
                    
                    /* Decreases 1st data counter. */
                    pRIic_Info->Cnt1st--;

                    /* Increases 1st Data buffer. */
                    pRIic_Info->pData1st++;

                break;
                
                case R_IIC_STS_SEND_SLVADR_R_WAIT:
                    
                    /* Sets internal status. */
                    r_iic_drv_set_internal_status(pRIic_Info, R_IIC_STS_RECEIVE_DATA_WAIT);

                    /* Reception setting. */
                    r_iic_drv_receive_start_setting(pRIic_Info);
                    
                    /* Cancels wait. */
                    /* In this way, releases SCL and starts the reception. */
                    r_iic_drv_canceling_wait(pRIic_Info);

                break;
                
                default:
                    /* Does nothing. */
                break;
            }
            
        break;
        
        default:
            /* Does nothing. */
        break;
        
    }

    return Ret;

}


/********************************************************************************************************************
* Function Name : r_iic_drv_write_data_sending
* Description   : After transmission data processing
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : R_IIC_COMMUNICATION               ;   Successful operation, communication state
*               :
*               : R_IIC_ERR_NON_REPLY               ;   None repley error
*********************************************************************************************************************/
error_t r_iic_drv_write_data_sending(r_iic_drv_info_t *pRIic_Info)
{
    error_t     Ret = R_IIC_COMMUNICATION;
    bool        boolret;

    /* IIC mode? */
    switch (g_iic_InternalInfo[pRIic_Info->ChNo].Mode)
    {
        case R_IIC_MODE_WRITE:

            /* Is 1st data pointer set? */
            if (NULL != pRIic_Info->pData1st)
            {
                /* 1st data counter = 0?  */
                if (0 != pRIic_Info->Cnt1st)                                /* Pattern 1 of Master Write            */
                {
                    /* Sets internal status. */
                    r_iic_drv_set_internal_status(pRIic_Info, R_IIC_STS_SEND_DATA_WAIT);

                    /* Transmits 1st data. */
                    r_iic_drv_set_sending_data(pRIic_Info, pRIic_Info->pData1st);
                    
                    /* Decreases 1st data counter. */
                    pRIic_Info->Cnt1st--;

                    /* Increases 1st data buffer. */
                    pRIic_Info->pData1st++;

                    return Ret;
                }
            }
            
            /* Is 2nd data pointer set? */
            if (NULL != pRIic_Info->pData2nd)
            {
                /* 2nd data counter = 0? */
                if (0 != pRIic_Info->Cnt2nd)                                /* Pattern 2 of Master Write            */
                {
                    /* Sets internal status. */
                    r_iic_drv_set_internal_status(pRIic_Info, R_IIC_STS_SEND_DATA_WAIT);

                    /* Transmits 2nd data. */
                    r_iic_drv_set_sending_data(pRIic_Info, pRIic_Info->pData2nd);
                    
                    /* Decreases 2nd data counter. */
                    pRIic_Info->Cnt2nd--;

                    /* Increases 2nd data buffer. */
                    pRIic_Info->pData2nd++;

                    return Ret;
                }
            }

            /* Sets internal status. */
            r_iic_drv_set_internal_status(pRIic_Info, R_IIC_STS_SP_COND_WAIT);
            
            /* Generates stop condition. */
            r_iic_drv_stop_cond_generate(pRIic_Info);
            
            return Ret;

        break;
        
        case R_IIC_MODE_COMBINED:

            /* Is 1st data pointer set? */
            if (0 != pRIic_Info->Cnt1st)
            {
                /* Sets internal status. */
                r_iic_drv_set_internal_status(pRIic_Info, R_IIC_STS_SEND_DATA_WAIT);

                /* Transmits 1st data. */
                r_iic_drv_set_sending_data(pRIic_Info, pRIic_Info->pData1st);
                
                /* Decreases 1st data counter. */
                pRIic_Info->Cnt1st--;

                /* Increases 1st data buffer. */
                pRIic_Info->pData1st++;

                return Ret;
            }

            /* Sets internal status. */
            r_iic_drv_set_internal_status(pRIic_Info, R_IIC_STS_ST_COND_WAIT);

            /* Re-start condition generation */
            r_iic_drv_start_cond_generate(pRIic_Info);

            /* Checks start condition generation. */
            boolret = r_iic_drv_start_cond_check(pRIic_Info);
            if (R_IIC_FALSE == boolret)
            {
                return R_IIC_ERR_NON_REPLY;
            }

            /* Re-sends slave address. */
            Ret = r_iic_drv_re_send_slvadr(pRIic_Info);
            
        break;
        
        default:
            /* Does nothing. */
        break;
    }
    
    return Ret;
}


/********************************************************************************************************************
* Function Name : r_iic_drv_read_data_receiving
* Description   : After reception data processing
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : R_IIC_COMMUNICATION               ;   Successful operation, communication state
*********************************************************************************************************************/
error_t r_iic_drv_read_data_receiving(r_iic_drv_info_t *pRIic_Info)
{
    error_t     Ret = R_IIC_COMMUNICATION;

    /* Next reception? */
    if (0 == pRIic_Info->Cnt2nd)                                    /* Next stop condition generation               */
    {
        /* Sets internal status. */
        r_iic_drv_set_internal_status(pRIic_Info, R_IIC_STS_SP_COND_WAIT);

        /* Stores the received data from IICA to pRIic_Info->pData2nd buffer. */
        *pRIic_Info->pData2nd = r_iic_drv_get_receiving_data(pRIic_Info);

        /* Increases receiving data buffer pointer. */
        pRIic_Info->pData2nd++;

        /* Generates stop condition. */
        r_iic_drv_stop_cond_generate(pRIic_Info);
    }
    else if (1 == pRIic_Info->Cnt2nd)                               /* last data reception                          */
    {
        /* Decreases receiving data counter. */
        pRIic_Info->Cnt2nd--;

        /* Reception end setting  */
        r_iic_drv_receive_end_setting(pRIic_Info);

        /* Cancels wait. */
        /* In this way, releases SCL and starts the reception. */
        r_iic_drv_canceling_wait(pRIic_Info);
    }
    else                                                            /* Next reception                               */
    {
        /* Sets internal status. */
        r_iic_drv_set_internal_status(pRIic_Info, R_IIC_STS_RECEIVE_DATA_WAIT);

        /* Stores the received data from IICA to pRIic_Info->pData2nd buffer. */
        *pRIic_Info->pData2nd = r_iic_drv_get_receiving_data(pRIic_Info);

        /* Increases receiving data buffer pointer. */
        pRIic_Info->pData2nd++;

        /* Decreases receiving data counter. */
        pRIic_Info->Cnt2nd--;

        /* Cancels wait. */
        /* In this way, releases SCL and starts the reception. */
        r_iic_drv_canceling_wait(pRIic_Info);
    }
    
    return Ret;
}


/********************************************************************************************************************
* Function Name : r_iic_drv_release
* Description   : After stop condition generation processing
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
*               : R_IIC_FINISH                      ;   Successful operation, finished communication and idle state
*********************************************************************************************************************/
error_t r_iic_drv_release(r_iic_drv_info_t *pRIic_Info)
{
    error_t     Ret = R_IIC_FINISH;
    
    /* Disables Port. */
    /* Sets the port to input mode and the output latch to 0. */
    r_iic_drv_io_open(pRIic_Info);

    /* Disables interruption. */
    r_iic_drv_int_disable(pRIic_Info);

    /* Sets internal status. */
    r_iic_drv_set_internal_status(pRIic_Info, R_IIC_STS_IDLE);

    return Ret;
}


/********************************************************************************************************************
* Function Name : r_iic_drv_arbitration_lost
* Description   : Arbitration lost error processing
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
*               : R_IIC_ERR_AL                      ;   Arbitration lost error
*********************************************************************************************************************/
error_t r_iic_drv_arbitration_lost(r_iic_drv_info_t *pRIic_Info)
{
    error_t     Ret = R_IIC_ERR_AL;
    
    return Ret;
}


/********************************************************************************************************************
* Function Name : r_iic_drv_nack
* Description   : Nack error processing
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
*               : R_IIC_NACK                        ;   Detected NACK, finished communication and idle state
*********************************************************************************************************************/
error_t r_iic_drv_nack(r_iic_drv_info_t *pRIic_Info)
{
    error_t     Ret = R_IIC_NACK;

    /* Sets internal status. */
    r_iic_drv_set_internal_status(pRIic_Info, R_IIC_STS_SP_COND_WAIT);

    /* Generates stop condition. */
    r_iic_drv_stop_cond_generate(pRIic_Info);
    
    return Ret;
}


/********************************************************************************************************************
* Function Name : r_iic_drv_ram_init
* Description   : RAM initialization
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : None
*********************************************************************************************************************/
void r_iic_drv_ram_init(r_iic_drv_info_t *pRIic_Info)
{
    /* Clears event flag. */
    g_iic_Event[pRIic_Info->ChNo]               = R_IIC_EV_INIT;

    /* Sets mode flag to R_IIC_MODE_NONE. */
    g_iic_InternalInfo[pRIic_Info->ChNo].Mode   = R_IIC_MODE_NONE;

}


/********************************************************************************************************************
* Function Name : r_iic_drv_set_internal_status
* Description   : Updates now status and previous status.
* Arguments     : r_iic_drv_info_t * pRIic_Info     ;   IIC Information
                : r_iic_drv_internal_status_t new_status    ;   New status
* Return Value  : None
*********************************************************************************************************************/
void r_iic_drv_set_internal_status(r_iic_drv_info_t *pRIic_Info, r_iic_drv_internal_status_t new_status)
{
    /* Sets previous status. */
    g_iic_InternalInfo[pRIic_Info->ChNo].B_status = g_iic_InternalInfo[pRIic_Info->ChNo].N_status;

    /* Sets now status. */
    g_iic_InternalInfo[pRIic_Info->ChNo].N_status = new_status;
}


/********************************************************************************************************************
* Function Name : r_iic_drv_set_ch_status
* Description   : Updates channel status flag.
* Arguments     : r_iic_drv_info_t * pRIic_Info     ;   IIC Information
* Return Value  : None
*********************************************************************************************************************/
void r_iic_drv_set_ch_status(r_iic_drv_info_t *pRIic_Info, error_t status)
{
    /* Sets channel status flag. */
    g_iic_ChStatus[pRIic_Info->ChNo] = status;
    
    /* Sets device status flag. */
    *(pRIic_Info->pDevStatus) = status;
}


/********************************************************************************************************************
* Function Name : r_iic_drv_check_chstatus_init
* Description   : When calls initialization function, checks channel status flag.
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : R_IIC_NO_INIT                     ;   Successful operation
*               : R_IIC_IDLE / R_IIC_FINISH / R_IIC_NACK    ;   Already idle state
*               : R_IIC_BUS_BUSY                    ;   Bus busy
*               : 
*               : R_IIC_ERR_AL                      ;   Arbitration lost error
*               : R_IIC_ERR_NON_REPLY               ;   None reply error
*               : R_IIC_ERR_SDA_LOW_HOLD            ;   SDA Low hold error
*               ; R_IIC_ERR_OTHER                   ;   Other error
*********************************************************************************************************************/
error_t r_iic_drv_check_chstatus_init(r_iic_drv_info_t *pRIic_Info)
{
    error_t     Ret = R_IIC_ERR_OTHER;

    /* Checks channel status. */
    switch (g_iic_ChStatus[pRIic_Info->ChNo])
    {
        case R_IIC_NO_INIT:
            if (0 > *(pRIic_Info->pDevStatus))
            {
                /* The target device is error status. */
                Ret = *(pRIic_Info->pDevStatus);
            }
            else
            {
                /* The target device is not initialized. */
                Ret = g_iic_ChStatus[pRIic_Info->ChNo];
            }
        break;

        case R_IIC_IDLE:
        case R_IIC_FINISH:
        case R_IIC_NACK:
            if (0 > *(pRIic_Info->pDevStatus))
            {
                /* The target device is error status. */
                Ret = *(pRIic_Info->pDevStatus);
            }
            else
            {
                /* The channel has been already initialized. */
                if (R_IIC_IDLE == g_iic_ChStatus[pRIic_Info->ChNo])
                {
                    /* If channel status flag is "R_IIC_IDLE", sets "R_IIC_IDLE" to a device status flag. */
                    *(pRIic_Info->pDevStatus) = R_IIC_IDLE;
                }
                Ret = g_iic_ChStatus[pRIic_Info->ChNo];
            }
        break;

        case R_IIC_COMMUNICATION:
            if (R_IIC_NO_INIT == *(pRIic_Info->pDevStatus))
            {
                /* The channel has been already initialized. */
                /* Sets "R_IIC_IDLE" to a device status flag. */
                *(pRIic_Info->pDevStatus) = R_IIC_IDLE;
                
                /* Other device is communicating.  */
                Ret = R_IIC_IDLE;
            }
            else if ((R_IIC_IDLE == *(pRIic_Info->pDevStatus)) ||
                     (R_IIC_FINISH == *(pRIic_Info->pDevStatus)) ||
                     (R_IIC_NACK == *(pRIic_Info->pDevStatus))       )
            {
                /* Other device is communicating. */
                Ret = R_IIC_IDLE;
            }
            else if (R_IIC_COMMUNICATION == *(pRIic_Info->pDevStatus))
            {
                /* The target device is communicating. */
                Ret = R_IIC_BUS_BUSY;
            }
            else
            {
                /* The target device is error status. */
                Ret = *(pRIic_Info->pDevStatus);
            }
        break;

        case R_IIC_ERR_AL:
        case R_IIC_ERR_NON_REPLY:
        case R_IIC_ERR_SDA_LOW_HOLD:
        case R_IIC_ERR_OTHER:
            Ret = g_iic_ChStatus[pRIic_Info->ChNo];
        break;

        default:
            /* Does nothing */
        break;
    }

    return Ret;
}

/********************************************************************************************************************
* Function Name : r_iic_drv_check_chstatus_start
* Description   : When calls starting function, checks channel status flag.
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : R_IIC_IDLE / R_IIC_FINISH / R_IIC_NACK    ;   Successful operation
*               : R_IIC_NO_INIT                     ;   Uninitialized state
*               : R_IIC_BUS_BUSY                    ;   Bus busy
*               : 
*               : R_IIC_ERR_AL                      ;   Arbitration lost error
*               : R_IIC_ERR_NON_REPLY               ;   None reply error
*               : R_IIC_ERR_SDA_LOW_HOLD            ;   SDA Low hold error
*               ; R_IIC_ERR_OTHER                   ;   Other error
*********************************************************************************************************************/
error_t r_iic_drv_check_chstatus_start(r_iic_drv_info_t *pRIic_Info)
{
    error_t     Ret = R_IIC_ERR_OTHER;

    /* Checks channel status. */
    switch (g_iic_ChStatus[pRIic_Info->ChNo])
    {
        case R_IIC_NO_INIT:
            if (0 > *(pRIic_Info->pDevStatus))
            {
                /* The target device is error status. */
                Ret = *(pRIic_Info->pDevStatus);
            }
            else
            {
                /* The target device is not initialized. */
                Ret = g_iic_ChStatus[pRIic_Info->ChNo];
            }
        break;

        case R_IIC_IDLE:
        case R_IIC_FINISH:
        case R_IIC_NACK:
            if (0 > *(pRIic_Info->pDevStatus))
            {
                /* The target device is error status. */
                Ret = *(pRIic_Info->pDevStatus);
            }
            else
            {
                /* The target device is not communicating. */
                Ret = g_iic_ChStatus[pRIic_Info->ChNo];
            }
        break;

        case R_IIC_COMMUNICATION:
            if (R_IIC_NO_INIT == *(pRIic_Info->pDevStatus))
            {
                /* Other device is communicating. The target device is not initialized. */
                Ret = R_IIC_NO_INIT;
            }
            else if ((R_IIC_IDLE == *(pRIic_Info->pDevStatus)) ||
                     (R_IIC_FINISH == *(pRIic_Info->pDevStatus)) ||
                     (R_IIC_NACK == *(pRIic_Info->pDevStatus)) ||
                     (R_IIC_COMMUNICATION == *(pRIic_Info->pDevStatus)))
            {
                /* The target device or other device is communicating. */
                Ret = R_IIC_BUS_BUSY;
            }
            else
            {
                /* The target device is error status. */
                Ret = *(pRIic_Info->pDevStatus);
            }
        break;

        case R_IIC_ERR_AL:
        case R_IIC_ERR_NON_REPLY:
        case R_IIC_ERR_SDA_LOW_HOLD:
        case R_IIC_ERR_OTHER:
            Ret = g_iic_ChStatus[pRIic_Info->ChNo];
        break;

        default:
            /* Does nothing. */
        break;
    }
    
    return Ret;
}

/********************************************************************************************************************
* Function Name : r_iic_drv_check_chstatus_advance
* Description   : When calls advance function, checks channel status flag.
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : R_IIC_COMMUNICATION               ;   Successful operation
*               : R_IIC_NO_INIT                     ;   Uninitialized state
*               : R_IIC_IDLE / R_IIC_FINISH / R_IIC_NACK    ;   Not started state
*               : R_IIC_BUS_BUSY                    ;   Bus busy
*               : 
*               : R_IIC_ERR_AL                      ;   Arbitration lost error
*               : R_IIC_ERR_NON_REPLY               ;   None reply error
*               : R_IIC_ERR_SDA_LOW_HOLD            ;   SDA Low hold error
*               ; R_IIC_ERR_OTHER                   ;   Other error
*********************************************************************************************************************/
error_t r_iic_drv_check_chstatus_advance(r_iic_drv_info_t *pRIic_Info)
{
    error_t     Ret = R_IIC_ERR_OTHER;
    
    /* Checks channel status. */
    switch (g_iic_ChStatus[pRIic_Info->ChNo])
    {
        case R_IIC_NO_INIT:
            if (0 > *(pRIic_Info->pDevStatus))
            {
                /* The target device is error status. */
                Ret = *(pRIic_Info->pDevStatus);
            }
            else
            {
                /* The target device is not initialized. */
                Ret = g_iic_ChStatus[pRIic_Info->ChNo];
            }
        break;

        case R_IIC_IDLE:
        case R_IIC_FINISH:
        case R_IIC_NACK:
            if (0 > *(pRIic_Info->pDevStatus))
            {
                /* The target device is error status. */
                Ret = *(pRIic_Info->pDevStatus);
            }
            else
            {
                /* The target device is not communicating. */
                Ret = g_iic_ChStatus[pRIic_Info->ChNo];
            }
        break;

        case R_IIC_COMMUNICATION:
            if (R_IIC_NO_INIT == *(pRIic_Info->pDevStatus))
            {
                /* Other device is communicating. The target device is not initialized. */
                Ret = R_IIC_NO_INIT;
            }
            else if ((R_IIC_IDLE == *(pRIic_Info->pDevStatus)) ||
                     (R_IIC_FINISH == *(pRIic_Info->pDevStatus)) ||
                     (R_IIC_NACK == *(pRIic_Info->pDevStatus))       )
            {
                /* Other device is communicating. The target device is not communicating. */
                Ret = R_IIC_BUS_BUSY;
            }
            else
            {
                /* The target device is communicating or error status. */
                Ret = *(pRIic_Info->pDevStatus);
            }
        break;

        case R_IIC_ERR_AL:
        case R_IIC_ERR_NON_REPLY:
        case R_IIC_ERR_SDA_LOW_HOLD:
        case R_IIC_ERR_OTHER:
            Ret = g_iic_ChStatus[pRIic_Info->ChNo];
        break;

        default:
            /* Does nothing. */
        break;
    }
    
    return Ret;
}

/********************************************************************************************************************
* Function Name : r_iic_drv_err_fin
* Description   : Completion processing at error
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : None
*********************************************************************************************************************/
void r_iic_drv_err_fin(r_iic_drv_info_t *pRIic_Info)
{
    /* Disables Port. */
    /* Sets the port to input mode and the output latch to 0. */
    r_iic_drv_io_open(pRIic_Info);

    /* Disables interruption. */
    r_iic_drv_int_disable(pRIic_Info);

    /* Sets internal status. */
    r_iic_drv_set_internal_status(pRIic_Info, R_IIC_STS_IDLE);
}


/********************************************************************************************************************
* Function Name : r_iic_drv_stop_cond_wait
* Description   : Waits stop condition generation.
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : R_IIC_TRUE                        ;   Successful operation ,stop condition generation
*               : R_IIC_FALSE                       ;   Failed operation ,stop condition generation
*********************************************************************************************************************/
bool r_iic_drv_stop_cond_wait(r_iic_drv_info_t *pRIic_Info)
{
    uint32_t    Cnt;
    bool        boolRet = R_IIC_TRUE;
    
    Cnt = STOP_COND_WAIT;
    do
    {
        /* Checks stop condition generation. */
        if (R_IIC_EV_INT_STOP == g_iic_Event[pRIic_Info->ChNo])
        {
            break;
        }
        Cnt--;
    }
    while(0 != Cnt);
    
    /* Stop Cond generation error? */
    if (0 == Cnt)
    {
        boolRet = R_IIC_FALSE;
    }
    
    return boolRet;
}

/********************************************************************************************************************
* Function Name : r_iic_drv_lock_api
* Description   : Confirms whether API is active and sets a flag if it is not active.
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : R_IIC_TRUE                        ;   Lock was acquired.
*               : R_IIC_FALSE                       ;   Lock was not acquired.
*********************************************************************************************************************/
bool r_iic_drv_lock_api(r_iic_drv_info_t *pRIic_Info)
{
    bool        Ret = R_IIC_FALSE;
    
    /* Checks api flag. */
    if (R_IIC_FALSE == g_iic_Api[pRIic_Info->ChNo])
    {
        /* Sets api flag. */
        g_iic_Api[pRIic_Info->ChNo] = R_IIC_TRUE;
        
        /* Sets return value. */
        Ret = R_IIC_TRUE;
    }
    
    return Ret;
    
}


/********************************************************************************************************************
* Function Name : r_iic_drv_unlock_api
* Description   : Clears api flag.
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : R_IIC_TRUE                        ;   Lock was released.
*********************************************************************************************************************/
bool r_iic_drv_unlock_api(r_iic_drv_info_t *pRIic_Info)
{
    bool        Ret = R_IIC_TRUE;

    /* Clears api flag. */
    g_iic_Api[pRIic_Info->ChNo] = R_IIC_FALSE;
    
    return Ret;
    
}

/* End of File */
