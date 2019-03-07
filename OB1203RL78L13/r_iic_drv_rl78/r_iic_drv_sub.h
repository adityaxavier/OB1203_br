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
* File Name     : r_iic_drv_sub.h
* Version       : 1.03
* Device(s)     : RL78/G14
* Tool-Chain    : -
*               : 
* OS            : -
* H/W Platform  : -
* Description   : IIC driver IICA internal module definitions
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
#ifndef __R_IIC_DRV_SUB_H__
#define __R_IIC_DRV_SUB_H__


/********************************************************************************************************
Macro definitions
*********************************************************************************************************/


/********************************************************************************************************
Typedef definitions
*********************************************************************************************************/

/*----- Structure type -----*/
/* Internal state transition structure */
typedef struct
{
    r_iic_drv_internal_event_t  event_type;         /* Event                                            */
    error_t  (*proc)( r_iic_drv_info_t *);          /* handler                                          */
} r_iic_drv_mtx_t;



/********************************************************************************************************
Exported global variables
*********************************************************************************************************/


/********************************************************************************************************
Exported global functions (to be accessed by other files)
*********************************************************************************************************/
/* Driver internal functions. */
error_t r_iic_drv_func_table(r_iic_drv_internal_event_t event, r_iic_drv_info_t *pRIic_Info);
error_t r_iic_drv_init_driver(r_iic_drv_info_t *pRIic_Info);
error_t r_iic_drv_generate_start_cond(r_iic_drv_info_t *pRIic_Info);
error_t r_iic_drv_send_slvadr(r_iic_drv_info_t *pRIic_Info);
error_t r_iic_drv_re_send_slvadr(r_iic_drv_info_t *pRIic_Info);
error_t r_iic_drv_after_send_slvadr(r_iic_drv_info_t *pRIic_Info);
error_t r_iic_drv_write_data_sending(r_iic_drv_info_t *pRIic_Info);
error_t r_iic_drv_read_data_receiving(r_iic_drv_info_t *pRIic_Info);
error_t r_iic_drv_release(r_iic_drv_info_t *pRIic_Info);
error_t r_iic_drv_arbitration_lost(r_iic_drv_info_t *pRIic_Info);
error_t r_iic_drv_nack(r_iic_drv_info_t *pRIic_Info);
void r_iic_drv_ram_init(r_iic_drv_info_t *pRIic_Info);
void r_iic_drv_set_internal_status(r_iic_drv_info_t *pRIic_Info, r_iic_drv_internal_status_t new_status);
void r_iic_drv_set_ch_status(r_iic_drv_info_t *pRIic_Info, error_t status);
error_t r_iic_drv_check_chstatus_init(r_iic_drv_info_t *pRIic_Info);
error_t r_iic_drv_check_chstatus_start(r_iic_drv_info_t *pRIic_Info);
error_t r_iic_drv_check_chstatus_advance(r_iic_drv_info_t *pRIic_Info);
void r_iic_drv_err_fin(r_iic_drv_info_t *pRIic_Info);
bool r_iic_drv_stop_cond_wait(r_iic_drv_info_t *pRIic_Info);
bool r_iic_drv_lock_api(r_iic_drv_info_t *pRIic_Info);
bool r_iic_drv_unlock_api(r_iic_drv_info_t *pRIic_Info);

#endif /* __R_IIC_DRV_SUB_H__ */


/* End of File */
