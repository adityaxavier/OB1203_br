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
* File Name     : r_iic_drv_sfr.h.rl78l13
* Version       : 1.03
* Device(s)     : RL78/L13
* Tool-Chain    : -
*               : 
* OS            : -
* H/W Platform  : -
* Description   : IIC driver IICA I/F module SFR definitions
* Operation     : -
* Limitations   : -
*********************************************************************************************************
* History       : DD.MM.YYYY    Version Description
*               : 31.05.2012    1.00    Created
*               : 07.03.2013    1.01    Ver.1.01 Release
*               : 31.03.2014    1.02    Modified RL78/G14 driver.
*               : 31.03.2014    1.02    Supported IAR Embedded Workbench for Renesas RL78.
*               : 31.03.2016    1.03    Supported CC-RL.
*               :                       Added r_iic_drv_check_scl_level().
*               :                       Added #define R_IIC_MSK_CLD.
*               :                       -----------------------------------------------------------------
*********************************************************************************************************/
#ifndef __R_IIC_DRV_SFR_H__
#define __R_IIC_DRV_SFR_H__

/********************************************************************************************************
Macro definitions
*********************************************************************************************************/
/*------------------------------------------------------------------------------------------------------*/
/*   Define channel register.                                                                           */
/*------------------------------------------------------------------------------------------------------*/
#ifdef __CA78K0R__
    #ifdef IICA0_ENABLE

        /* Define port. */
        #define R_IIC_PM_SCL0           PM6.0           /* SCL0 Port mode registers                     */  /** SET **/
        #define R_IIC_PM_SDA0           PM6.1           /* SDA0 Port mode registers                     */  /** SET **/
        #define R_IIC_P_SCL0            P6.0            /* SCL0 Port registers                          */  /** SET **/
        #define R_IIC_P_SDA0            P6.1            /* SDA0 Port registers                          */  /** SET **/

        /* Define peripheral enable register */
        #define R_IIC_IICA0EN           IICA0EN         /* IICA0 peripheral enable register             */  /** SET **/

    #endif /* #ifdef IICA0_ENABLE */
#endif  /* __CA78K0R__ */
#ifdef __CCRL__
    #ifdef IICA0_ENABLE

        /* Define port. */
        #define R_IIC_PM_SCL0           PM6_bit.no0     /* SCL0 Port mode registers                     */  /** SET **/
        #define R_IIC_PM_SDA0           PM6_bit.no1     /* SDA0 Port mode registers                     */  /** SET **/
        #define R_IIC_P_SCL0            P6_bit.no0      /* SCL0 Port registers                          */  /** SET **/
        #define R_IIC_P_SDA0            P6_bit.no1      /* SDA0 Port registers                          */  /** SET **/

        /* Define peripheral enable register */
        #define R_IIC_IICA0EN           IICA0EN         /* IICA0 peripheral enable register             */  /** SET **/

    #endif /* #ifdef IICA0_ENABLE */
#endif  /* __CCRL__ */
#ifdef __ICCRL78__
    #ifdef IICA0_ENABLE

        /* Define port. */
        #define R_IIC_PM_SCL0           PM6_bit.no0     /* SCL0 Port mode registers                     */  /** SET **/
        #define R_IIC_PM_SDA0           PM6_bit.no1     /* SDA0 Port mode registers                     */  /** SET **/
        #define R_IIC_P_SCL0            P6_bit.no0      /* SCL0 Port registers                          */  /** SET **/
        #define R_IIC_P_SDA0            P6_bit.no1      /* SDA0 Port registers                          */  /** SET **/

        /* Define peripheral enable register */
        #define R_IIC_IICA0EN           IICA0EN         /* IICA0 peripheral enable register             */  /** SET **/

    #endif /* #ifdef IICA0_ENABLE */
#endif  /* __ICCRL78__ */


/*------------------------------------------------------------------------------------------------------*/
/*   Define register setting.                                                                           */
/*------------------------------------------------------------------------------------------------------*/
/* IICA register setting */
#define R_IIC_IICCTL0_RESET     (uint8_t)(0x00)         /* IICE = 0                                     */
#define R_IIC_SVA               (uint8_t)(0x00)         /* SAV = 0                                      */
#define R_IIC_IICCTL1_INIT      (uint8_t)(0x0D)         /* SMC = 1(400KHz), DFC = 1, PRS = 1            */  /** SET **/
/* #define R_IIC_IICCTL1_INIT   (uint8_t)(0x01) */      /* SMC = 0(100KHz), DFC = 0, PRS = 1            */  /** SET **/
#define R_IIC_IICF_INIT         (uint8_t)(0x03)         /* STCEN = 1, IICRSV = 1                        */
#define R_IIC_IICCTL0_INIT      (uint8_t)(0x1C)         /* SPIE = 1, WTIM = 1, ACKE = 1                 */
#define R_IIC_IICCTL0_ENABLE    (uint8_t)(0x80)         /* IICE = 1                                     */
#define R_IIC_IICCTL0_ST        (uint8_t)(0x02)         /* STT = 1                                      */
#define R_IIC_IICCTL0_SP        (uint8_t)(0x01)         /* SPT = 1                                      */
#define R_IIC_IICCTL0_REV       (uint8_t)(0x20)         /* WREL = 1                                     */

#define R_IIC_MSK_IICBSY        (uint8_t)(0x40)         /* Mask IICBSY bit                              */
#define R_IIC_MSK_CLDDAD        (uint8_t)(0x30)         /* Mask CLD bit and DLD bit                     */
#define R_IIC_MSK_CLD           (uint8_t)(0x20)         /* Mask CLD bit                                 */
#define R_IIC_MSK_STD           (uint8_t)(0x02)         /* Mask STD bit                                 */
#define R_IIC_WTIM_CLR          (uint8_t)(0xF7)         /* WTIM = 0                                     */
#define R_IIC_WTIM_SET          (uint8_t)(0x08)         /* WTIM = 1                                     */
#define R_IIC_ACKE_CLR          (uint8_t)(0xFB)         /* ACKE = 0                                     */
#define R_IIC_ACKE_SET          (uint8_t)(0x04)         /* ACKE = 1                                     */


/* Interrupt register setting */
#ifdef IICA0_ENABLE                                     /* Channel 0                                    */
#define R_IIC_CH0_PR0_INIT      (uint8_t)(0x01)         /* Priority level 1                             */  /** SET **/
#define R_IIC_CH0_PR1_INIT      (uint8_t)(0x00)         /* Priority level 1                             */  /** SET **/
#define R_IIC_CH0_IF_INIT       (uint8_t)(0x00)         /* Clears interruption request flag.            */
#define R_IIC_CH0_MK_DISABLE    (uint8_t)(0x01)         /* Disables interruption.                       */
#define R_IIC_CH0_MK_ENABLE     (uint8_t)(0x00)         /* Enables interruption.                        */
#endif  /* #ifdef IICA0_ENABLE */

/* Registers address defines */
#define IICA_ADR(n)             ( (volatile uint8_t *)&IICA0    + ((4 * n) / sizeof(uint8_t)) )
#define SVA_ADR(n)              ( (volatile uint8_t *)&SVA0     + ((8 * n) / sizeof(uint8_t)) )

/* Control registers address defines */
#define PER0_ADR(n)             ( (volatile uint8_t *)&PER0 )
#define IICCTL0_ADR(n)          ( (volatile uint8_t *)&IICCTL00 + ((8 * n) / sizeof(uint8_t)) )
#define IICS_ADR(n)             ( (volatile uint8_t *)&IICS0    + ((4 * n) / sizeof(uint8_t)) )
#define IICF_ADR(n)             ( (volatile uint8_t *)&IICF0    + ((4 * n) / sizeof(uint8_t)) )
#define IICCTL1_ADR(n)          ( (volatile uint8_t *)&IICCTL01 + ((8 * n) / sizeof(uint8_t)) )
#define IICWL_ADR(n)            ( (volatile uint8_t *)&IICWL0   + ((8 * n) / sizeof(uint8_t)) )
#define IICWH_ADR(n)            ( (volatile uint8_t *)&IICWH0   + ((8 * n) / sizeof(uint8_t)) )


/********************************************************************************************************
Typedef definitions
*********************************************************************************************************/


/********************************************************************************************************
Exported global variables
*********************************************************************************************************/


/********************************************************************************************************
Exported global functions (to be accessed by other files)
*********************************************************************************************************/
/* SFR setting functions */
void r_iic_drv_io_open(r_iic_drv_info_t *pRIic_Info);
void r_iic_drv_io_init(r_iic_drv_info_t *pRIic_Info);
void r_iic_drv_io_chk_sda_setting(r_iic_drv_info_t *pRIic_Info);
void r_iic_drv_clk_low(r_iic_drv_info_t *pRIic_Info);
void r_iic_drv_clk_high(r_iic_drv_info_t *pRIic_Info);
void r_iic_drv_init_sfr(r_iic_drv_info_t *pRIic_Info);
void r_iic_drv_int_disable(r_iic_drv_info_t *pRIic_Info);
void r_iic_drv_int_enable(r_iic_drv_info_t *pRIic_Info);
void r_iic_drv_int_maskable_disable(void);
void r_iic_drv_int_maskable_enable(void);
void r_iic_drv_set_frequency(r_iic_drv_info_t *pRIic_Info);
bool r_iic_drv_check_sda_level(r_iic_drv_info_t *pRIic_Info);
bool r_iic_drv_check_scl_level(r_iic_drv_info_t *pRIic_Info);
bool r_iic_drv_check_bus_busy(r_iic_drv_info_t *pRIic_Info);
void r_iic_drv_per_enable(r_iic_drv_info_t *pRIic_Info);
void r_iic_drv_start_cond_generate(r_iic_drv_info_t *pRIic_Info);
void r_iic_drv_stop_cond_generate(r_iic_drv_info_t *pRIic_Info);
bool r_iic_drv_start_cond_check(r_iic_drv_info_t *pRIic_Info);
void r_iic_drv_set_sending_data(r_iic_drv_info_t *pRIic_Info, uint8_t *pData);
uint8_t r_iic_drv_get_receiving_data(r_iic_drv_info_t *pRIic_Info);
void r_iic_drv_receive_start_setting(r_iic_drv_info_t *pRIic_Info);
void r_iic_drv_receive_end_setting(r_iic_drv_info_t *pRIic_Info);
void r_iic_drv_canceling_wait(r_iic_drv_info_t *pRIic_Info);
void r_iic_drv_reset_iic(r_iic_drv_info_t *pRIic_Info);


#endif /* __R_IIC_DRV_SFR_H__ */


/* End of File */
