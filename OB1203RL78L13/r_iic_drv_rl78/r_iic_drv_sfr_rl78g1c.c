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
* File Name     : r_iic_drv_sfr_rl78g1c.c
* Version       : 1.03
* Device(s)     : RL78/G1C
* Tool-Chain    : -
*               : 
* OS            : -
* H/W Platform  : -
* Description   : IIC driver IICA SFR module
* Operation     : -
* Limitations   : -
*********************************************************************************************************************
* History       : DD.MM.YYYY    Version Description
*               : 31.05.2012    1.00    Created
*               : 07.03.2013    1.01    Ver.1.01 Release
*               : 31.03.2014    1.02    Modified RL78/G14 driver.
*               : 31.03.2014    1.02    Supported IAR Embedded Workbench for Renesas RL78.
*               : 31.03.2016    1.03    Supported CC-RL.
*               :                       Added r_iic_drv_check_scl_level().
*               :                       -----------------------------------------------------------------------------
*********************************************************************************************************************/
#ifdef __CA78K0R__                                                          /* Renesas 78K0R, RL78 Compiler         */
    #pragma sfr
    #pragma DI
    #pragma EI
#endif  /* __CA78K0R__ */
#ifdef __CCRL__                                                             /* Renesas CC-RL Compiler               */
    #include "iodefine.h"                                                   /* for RL78                             */
#endif /* __CCRL__ */
#ifdef __ICCRL78__                                                          /* IAR 78K0R Compiler                   */
    #include <ior5f10jgc.h>                                                 /* for RL78/G1C 48pin (R5F10JGC)        */  /** SET **/
    #include <ior5f10jgc_ext.h>                                             /* for RL78/G1C 48pin (R5F10JGC)        */  /** SET **/
    #include "intrinsics.h"                                                 /* Header file for built in function    */
#endif  /* __ICCRL78__ */


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


/********************************************************************************************************************
* Function Name : r_iic_drv_io_open
* Description   : Sets output latch value setting to Low. Also, sets ports to input mode.
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : None
*********************************************************************************************************************/
void r_iic_drv_io_open(r_iic_drv_info_t *pRIic_Info)
{
    /* Channel number? */
    switch (pRIic_Info->ChNo)
    {
        case 0U:                                                /* Channel 0                                        */
#ifdef IICA0_ENABLE
            R_IIC_P_SCL0    = R_IIC_LOW;                        /* SCL0 output latch value setting to Low           */
            R_IIC_P_SDA0    = R_IIC_LOW;                        /* SDA0 output latch value setting to Low           */

            R_IIC_PM_SCL0   = R_IIC_IN;                         /* SCL0 input mode                                  */
            R_IIC_PM_SDA0   = R_IIC_IN;                         /* SDA0 input mode                                  */
#endif /* #ifdef IICA0_ENABLE */
        break;

        default:
        
            /* Please add a channel as needed. */
        
        break;
    }
}


/********************************************************************************************************************
* Function Name : r_iic_drv_io_init
* Description   : Sets ports to output mode.
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : None
*********************************************************************************************************************/
void r_iic_drv_io_init(r_iic_drv_info_t *pRIic_Info)
{
    /* Channel number? */
    switch (pRIic_Info->ChNo)
    {
        case 0U:                                                /* Channel 0                                        */
#ifdef IICA0_ENABLE
            R_IIC_P_SCL0    = R_IIC_LOW;                        /* SCL0 output latch value setting to Low           */
            R_IIC_P_SDA0    = R_IIC_LOW;                        /* SDA0 output latch value setting to Low           */

            R_IIC_PM_SCL0   = R_IIC_OUT;                        /* SCL0 output mode                                 */
            R_IIC_PM_SDA0   = R_IIC_OUT;                        /* SDA0 output mode                                 */

            R_IIC_P_SCL0    = R_IIC_LOW;                        /* SCL0 output latch value setting to Low           */
            R_IIC_P_SDA0    = R_IIC_LOW;                        /* SDA0 output latch value setting to Low           */
#endif /* #ifdef IICA0_ENABLE */
        break;

        default:
        
            /* Please add a channel as needed. */
        
        break;
    }
}


/********************************************************************************************************************
* Function Name : r_iic_drv_io_chk_sda_setting
* Description   : Sets ports to input mode.
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : None
*********************************************************************************************************************/
void r_iic_drv_io_chk_sda_setting(r_iic_drv_info_t *pRIic_Info)
{
    /* Channel number? */
    switch (pRIic_Info->ChNo)
    {
        case 0U:                                                /* Channel 0                                        */
#ifdef IICA0_ENABLE
            /* SCL input mode. */
            R_IIC_PM_SCL0   = R_IIC_IN;                         /* SCL0 input mode                                  */

            /* SDA input mode. */
            R_IIC_PM_SDA0   = R_IIC_IN;                         /* SDA0 input mode                                  */
#endif /* #ifdef IICA0_ENABLE */
        break;

        default:
        
            /* Please add a channel as needed. */
        
        break;
    }
}


/********************************************************************************************************************
* Function Name : r_iic_drv_clk_low
* Description   : Sets SCL to Low.
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : None
*********************************************************************************************************************/
void r_iic_drv_clk_low(r_iic_drv_info_t *pRIic_Info)
{
    uint16_t    Cnt;
    
    /* Channel number? */
    switch (pRIic_Info->ChNo)
    {
        case 0U:                                                /* Channel 0                                        */
#ifdef IICA0_ENABLE
            R_IIC_P_SCL0    = R_IIC_LOW;                        /* SCL0 output latch setting to Low                 */
            R_IIC_PM_SCL0   = R_IIC_OUT;                        /* SCL0 output mode                                 */
            R_IIC_P_SCL0    = R_IIC_LOW;                        /* SCL0 output latch setting to Low                 */
#endif /* #ifdef IICA0_ENABLE */
        break;
        
        default:
            /* Please add a channel as needed. */
        break;
    }
    
    /* Waits for setting SCL. */
    Cnt = SCL_L_WAIT;
    do
    {
        Cnt--;
    }
    while(0 != Cnt);

    
}


/********************************************************************************************************************
* Function Name : r_iic_drv_clk_high
* Description   : Sets SCL to High.
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : None
*********************************************************************************************************************/
void r_iic_drv_clk_high(r_iic_drv_info_t *pRIic_Info)
{
    uint16_t    Cnt;

    /* Channel number? */
    switch (pRIic_Info->ChNo)
    {
        case 0U:                                                /* Channel 0                                        */
#ifdef IICA0_ENABLE
            R_IIC_PM_SCL0   = R_IIC_IN;                         /* SCL0 input mode                                  */
#endif /* #ifdef IICA0_ENABLE */
        break;

        default:
        
            /* Please add a channel as needed. */
        
        break;
    }

    /* Waits for setting SCL. */
    Cnt = SCL_H_WAIT;
    do
    {
        Cnt--;
    }
    while(0 != Cnt);

}


/********************************************************************************************************************
* Function Name : r_iic_drv_init_sfr
* Description   : IIC driver initialization
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : None
*********************************************************************************************************************/
void r_iic_drv_init_sfr(r_iic_drv_info_t *pRIic_Info)
{
    /* Creates the register pointer for the selected IICA channel. */
    volatile uint8_t * const    pcSVA       = SVA_ADR(pRIic_Info->ChNo);
    volatile uint8_t * const    pcIICCTL0   = IICCTL0_ADR(pRIic_Info->ChNo);
    volatile uint8_t * const    pcIICF      = IICF_ADR(pRIic_Info->ChNo);
    volatile uint8_t * const    pcIICCTL1   = IICCTL1_ADR(pRIic_Info->ChNo);

    /* Ports open setting. */
    r_iic_drv_io_open(pRIic_Info);                              /* Port : Input, Output latch : Low                 */

    /* Initializes IICA register. */
    *pcIICCTL0 = R_IIC_IICCTL0_RESET;                           /* IICE = 0                                         */
    
    /* Sets a transfer clock. */
    r_iic_drv_set_frequency(pRIic_Info);

    /* Sets a local address. */
    *pcSVA = R_IIC_SVA;
    
    /* Sets IICCTL1. */
    *pcIICCTL1 |= R_IIC_IICCTL1_INIT;                           /* Sets SMC, DFC and PRS.                           */

    /* Sets a start condition. */
    *pcIICF |= R_IIC_IICF_INIT;                                 /* STCEN = 1, IICRSV = 1                            */

    /* Sets IICCTL0. */
    *pcIICCTL0 |= R_IIC_IICCTL0_INIT;                           /* SPIE = 1, WTIM = 1, ACKE = 1                     */

    /* Enables IIC. */
    *pcIICCTL0 |= R_IIC_IICCTL0_ENABLE;                         /* IICE = 1                                         */

}


/********************************************************************************************************************
* Function Name : r_iic_drv_int_disable
* Description   : Disables interruption
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : None
*********************************************************************************************************************/
void r_iic_drv_int_disable(r_iic_drv_info_t *pRIic_Info)
{
    /* Channel number? */
    switch (pRIic_Info->ChNo)
    {
        case 0U:                                                /* Channel 0                                        */
#ifdef IICA0_ENABLE
            IICAMK0 = R_IIC_CH0_MK_DISABLE;                     /* Disables interruption.                           */
            IICAIF0 = R_IIC_CH0_IF_INIT;                        /* Clears interruption flag.                        */

            /* Sets interruption level. */
            IICAPR00 = R_IIC_CH0_PR0_INIT;
            IICAPR10 = R_IIC_CH0_PR1_INIT;
#endif /* #ifdef IICA0_ENABLE */
        break;

        default:
        
            /* Please add a channel as needed. */
        
        break;
    }
}


/********************************************************************************************************************
* Function Name : r_iic_drv_int_enable
* Description   : Enables interruption
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : None
*********************************************************************************************************************/
void r_iic_drv_int_enable(r_iic_drv_info_t *pRIic_Info)
{
    /* Channel number? */
    switch (pRIic_Info->ChNo)
    {
        case 0U:                                                /* Channel 0                                        */
#ifdef IICA0_ENABLE
            IICAIF0 = R_IIC_CH0_IF_INIT;                        /* Clears interruption flag.                        */
            IICAMK0 = R_IIC_CH0_MK_ENABLE;                      /* Enables interruption.                            */
#endif /* #ifdef IICA0_ENABLE */
        break;

        default:
        
            /* Please add a channel as needed. */
        
        break;
    }
}


/********************************************************************************************************************
* Function Name : r_iic_drv_int_maskable_disable
* Description   : Sets interruption disabled.
* Arguments     : None
* Return Value  : None
*********************************************************************************************************************/
void r_iic_drv_int_maskable_disable(void)
{
    /* Sets interruption disabled. */
#ifdef __CA78K0R__
    DI();
#endif  /* __CA78K0R__ */
#ifdef __CCRL__
    __DI();
#endif  /* __CCRL__ */
#ifdef __ICCRL78__
    __disable_interrupt();
#endif  /* __ICCRL78__ */
}


/********************************************************************************************************************
* Function Name : r_iic_drv_int_maskable_enable
* Description   : Sets interruption enabled.
* Arguments     : None
* Return Value  : None
*********************************************************************************************************************/
void r_iic_drv_int_maskable_enable(void)
{
    /* Sets interruption enabled. */
#ifdef __CA78K0R__
    EI();
#endif  /* __CA78K0R__ */
#ifdef __CCRL__
    __EI();
#endif  /* __CCRL__ */
#ifdef __ICCRL78__
    __enable_interrupt();
#endif  /* __ICCRL78__ */
}


/********************************************************************************************************************
* Function Name : r_iic_drv_set_frequency
* Description   : Sets IIC Frequency.
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : None
*********************************************************************************************************************/
void r_iic_drv_set_frequency(r_iic_drv_info_t *pRIic_Info)
{
    /* Creates the register pointer for the selected IICA channel. */
    volatile uint8_t * const    pcIICWL     = IICWL_ADR(pRIic_Info->ChNo);
    volatile uint8_t * const    pcIICWH     = IICWH_ADR(pRIic_Info->ChNo);

    /* Channel number? */
    switch (pRIic_Info->ChNo)
    {
        case 0U:                                                /* Channel 0                                        */
#ifdef IICA0_ENABLE
            *pcIICWL = R_IIC_CH0_LCLK;                          /* Low-level width setting                          */
            *pcIICWH = R_IIC_CH0_HCLK;                          /* High-level width setting                         */
#endif /* #ifdef IICA0_ENABLE */
        break;

        default:
        
            /* Please add a channel as needed. */
        
        break;
    }
}


/********************************************************************************************************************
* Function Name : r_iic_drv_check_sda_level
* Description   : Checks SDA level.
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : R_IIC_TRUE                        ;   SDA = High
*               : R_IIC_FALSE                       ;   SDA = Low
*********************************************************************************************************************/
bool r_iic_drv_check_sda_level(r_iic_drv_info_t *pRIic_Info)
{
    bool        Ret = R_IIC_TRUE;
    uint16_t    Cnt;

    /* Channel number? */
    switch (pRIic_Info->ChNo)
    {
        case 0U:                                                /* Channel 0                                        */
#ifdef IICA0_ENABLE

            /* Checks SDA level. */
            Cnt = SDACHK_CNT;
            do
            {
                /* SDA = High? */
                if (R_IIC_HI == R_IIC_P_SDA0)
                {
                    break;                                      /* SDA = High                                       */
                }
                Cnt--;
            }
            while(Cnt != 0);

            /* Counter limit? */
            if(0 == Cnt)
            {
                Ret = R_IIC_FALSE;                              /* SDA = Low                                        */
            }
#endif /* #ifdef IICA0_ENABLE */
        break;

        default:
        
            /* Please add a channel as needed. */

        break;
    }

    return Ret;
}


/********************************************************************************************************************
* Outline       : Check SCL Level Processing
* Function Name : r_iic_drv_check_scl_level
* Description   : Checks SCL level from high to low.
* Arguments     : r_iic_drv_info_t * pRIic_Info     ;   IIC Information
* Return Value  : R_IIC_TRUE                        ;   SDA = High
*               : R_IIC_FALSE                       ;   SDA = Low
*********************************************************************************************************************/
bool r_iic_drv_check_scl_level(r_iic_drv_info_t * pRIic_Info)
{
    uint16_t cnt = 0;
    
    /* Creates the register pointer for the selected IICA channel. */
    volatile uint8_t * const    pcIICCTL1   = IICCTL1_ADR(pRIic_Info->ChNo);
    
    for (cnt = SCLCHK_CNT; cnt > 0; cnt--)
    {
        if (R_IIC_MSK_CLD != (*pcIICCTL1 & R_IIC_MSK_CLD))
        {
            return R_IIC_TRUE;
        }
    }
    
    return R_IIC_FALSE;
}


/********************************************************************************************************************
* Function Name : r_iic_drv_check_bus_busy
* Description   : Checks bus busy.
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : R_IIC_TRUE                        ;   Bus Ready
*               : R_IIC_FALSE                       ;   Bus Busy
*********************************************************************************************************************/
bool r_iic_drv_check_bus_busy(r_iic_drv_info_t *pRIic_Info)
{
    bool        Ret = R_IIC_TRUE;
    uint32_t    Cnt;

    /* Creates the register pointer for the selected IICA channel. */
    volatile uint8_t * const    pcIICCTL1   = IICCTL1_ADR(pRIic_Info->ChNo);
    volatile uint8_t * const    pcIICF      = IICF_ADR(pRIic_Info->ChNo);

    /* Checks bus busy. */
    for(Cnt = BUSCHK_CNT; Cnt > 0; Cnt--)
    {
        /* Bus busy? */
        if ((R_IIC_MSK_IICBSY != (*pcIICF & R_IIC_MSK_IICBSY))  &&
            (R_IIC_MSK_CLDDAD == (*pcIICCTL1 & R_IIC_MSK_CLDDAD)))              /* IICBSY = 0, CLD = 1, DAD = 1     */
        {
            break;                                              /* Bus ready                                        */
        }
    }

    /* Counter limit? */
    if (0 == Cnt)
    {
        Ret = R_IIC_FALSE;                                      /* Bus busy                                         */
    }

    return Ret;

}


/********************************************************************************************************************
* Function Name : r_iic_drv_per_enable
* Description   : Clock is supplied permission to IIC peripheral hardware.
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : None
*********************************************************************************************************************/
void r_iic_drv_per_enable(r_iic_drv_info_t *pRIic_Info)
{
    /* Channel number? */
    switch (pRIic_Info->ChNo)
    {
        case 0U:
#ifdef IICA0_ENABLE
            R_IIC_IICA0EN = 1U;                                 /* IICA0 enables input clock supply.                */
#endif /* #ifdef IICA0_ENABLE */
        break;
    
        default:
        
            /* Please add a channel as needed. */
        
        break;
    }
}

/********************************************************************************************************************
* Function Name : r_iic_drv_start_cond_generate
* Description   : Start condition generation
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : None
*********************************************************************************************************************/
void r_iic_drv_start_cond_generate(r_iic_drv_info_t *pRIic_Info)
{
    /* Creates the register pointer for the selected IICA channel. */
    volatile uint8_t * const    pcIICCTL0   = IICCTL0_ADR(pRIic_Info->ChNo);

    /* Start condition generation. */
    *pcIICCTL0 |= R_IIC_IICCTL0_ST;                             /* Sets a STT bit.                                  */
}

/********************************************************************************************************************
* Function Name : r_iic_drv_stop_cond_generate
* Description   : Stop condition generation
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : None
*********************************************************************************************************************/
void r_iic_drv_stop_cond_generate(r_iic_drv_info_t *pRIic_Info)
{
    /* Creates the register pointer for the selected IICA channel. */
    volatile uint8_t * const    pcIICCTL0   = IICCTL0_ADR(pRIic_Info->ChNo);
    
    /* Stop condition generation. */
    *pcIICCTL0 |= R_IIC_IICCTL0_SP;                             /* Set a SPT bit.                                   */

}


/********************************************************************************************************************
* Function Name : r_iic_drv_start_cond_check
* Description   : Checks start condition generation.
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : R_IIC_TRUE                        ;   Detects start condition
*               : R_IIC_FALSE                       ;   Not detects start condition
*********************************************************************************************************************/
bool r_iic_drv_start_cond_check(r_iic_drv_info_t *pRIic_Info)
{
    bool        Ret = R_IIC_TRUE;
    uint16_t    Cnt;
    
    /* Creates the register pointer for the selected IICA channel. */
    volatile uint8_t * const    pcIICS      = IICS_ADR(pRIic_Info->ChNo);
    
    /* Waits Start Condition generation. */
    for (Cnt = START_COND_WAIT; Cnt > 0; Cnt--)
    {
        /* Sets STD bit? */
        if (R_IIC_MSK_STD == (*pcIICS & R_IIC_MSK_STD))
        {
            break;                                              /* Successful                                       */
        }
    }

    /* Counter limit? */
    if (0 == Cnt)                                               /* Failed start condition generation                */
    {
        Ret = R_IIC_FALSE;                                      /* Error                                            */
    }

    return Ret;
}

/********************************************************************************************************************
* Function Name : r_iic_drv_set_sending_data
* Description   : Writes data to IICA in order to transmit.
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
*               : uint8_t *pData                    ;   Sending data buffer pointer
* Return Value  : None
*********************************************************************************************************************/
void r_iic_drv_set_sending_data(r_iic_drv_info_t *pRIic_Info, uint8_t *pData)
{
    /* Creates the register pointer for the selected IICA channel. */
    volatile uint8_t * const    pcIICA      = IICA_ADR(pRIic_Info->ChNo);

    /* Sets Sending Data. */
    *pcIICA = *pData;                                           /* Writes data to IICA in order to transmit.        */

}


/********************************************************************************************************************
* Function Name : r_iic_drv_get_receiving_data
* Description   : Stores the received data from IICA to "pRIic_Info->pData buffer".
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
*               : uint8_t *pData                    ;   Sending data buffer pointer
* Return Value  : None
*********************************************************************************************************************/
uint8_t r_iic_drv_get_receiving_data(r_iic_drv_info_t *pRIic_Info)
{
    /* Creates the register pointer for the selected IICA channel. */
    volatile uint8_t * const    pcIICA      = IICA_ADR(pRIic_Info->ChNo);

    /* Stores the received data. */
    return *pcIICA;
}


/********************************************************************************************************************
* Function Name : r_iic_drv_receive_start_setting
* Description   : Setting when starts the master reception
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : None
*********************************************************************************************************************/
void r_iic_drv_receive_start_setting(r_iic_drv_info_t *pRIic_Info)
{
    /* Creates the register pointer for the selected IICA channel. */
    volatile uint8_t * const    pcIICCTL0   = IICCTL0_ADR(pRIic_Info->ChNo);
    
    /* Clears WTIM bit = 0 & Sets ACKE bit = 1. */
    *pcIICCTL0 = ((*pcIICCTL0 & R_IIC_WTIM_CLR) | R_IIC_ACKE_SET);
}


/********************************************************************************************************************
* Function Name : r_iic_drv_receive_end_setting
* Description   : Setting when stops the master reception
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : None
*********************************************************************************************************************/
void r_iic_drv_receive_end_setting(r_iic_drv_info_t *pRIic_Info)
{
    /* Creates the register pointer for the selected IICA channel. */
    volatile uint8_t * const    pcIICCTL0   = IICCTL0_ADR(pRIic_Info->ChNo);

    /* Sets WTIM bit = 1 & Clears ACKE bit = 0. */
    *pcIICCTL0 = ((*pcIICCTL0 & R_IIC_ACKE_CLR) | R_IIC_WTIM_SET);
}


/********************************************************************************************************************
* Function Name : r_iic_drv_canceling_wait
* Description   : Cancels wait.
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : None
*********************************************************************************************************************/
void r_iic_drv_canceling_wait(r_iic_drv_info_t *pRIic_Info)
{
    /* Creates the register pointer for the selected IICA channel. */
    volatile uint8_t * const    pcIICCTL0   = IICCTL0_ADR(pRIic_Info->ChNo);

    /* Cancels wait.(Starts reception.) */
    *pcIICCTL0 |= R_IIC_IICCTL0_REV;
}


/********************************************************************************************************************
* Function Name : r_iic_drv_reset_iic
* Description   : Resets iic register.
* Arguments     : r_iic_drv_info_t *pRIic_Info      ;   IIC Information
* Return Value  : None
*********************************************************************************************************************/
void r_iic_drv_reset_iic(r_iic_drv_info_t *pRIic_Info)
{
    /* Creates the register pointer for the selected IICA channel. */
    volatile uint8_t * const    pcIICCTL0   = IICCTL0_ADR(pRIic_Info->ChNo);

    /* Resets IICA register. */
    *pcIICCTL0 = R_IIC_IICCTL0_RESET;
    
}


/* End of File */
