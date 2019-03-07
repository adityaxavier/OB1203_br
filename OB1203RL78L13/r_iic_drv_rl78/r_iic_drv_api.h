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
* File Name     : r_iic_drv_api.h
* Version       : 1.03
* Device(s)     : RL78/G14
* Tool-Chain    : -
*               : 
* OS            : -
* H/W Platform  : -
* Description   : IIC driver IICA I/F module definitions
* Operation     : -
* Limitations   : -
*********************************************************************************************************
* History       : DD.MM.YYYY    Version Description
*               : 31.05.2012    1.00    Created
*               : 07.03.2013    1.01    Ver.1.01 Release
*               : 31.03.2014    1.02    Supported IAR Embedded Workbench for Renesas RL78.
*               : 31.03.2016    1.03    Supported CC-RL.
*               :                       Added #define SCLCHK_CNT.
*               :                       -----------------------------------------------------------------
*********************************************************************************************************/
#ifndef __R_IIC_DRV_API_H__
#define __R_IIC_DRV_API_H__

/********************************************************************************************************
Macro definitions
*********************************************************************************************************/
/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
/*                              Please define resources of each user system.                            */
/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
/*------------------------------------------------------------------------------------------------------*/
/*   Select channels to enable.                                                                         */
/*------------------------------------------------------------------------------------------------------*/
#define IICA0_ENABLE                                                                                        /** SET **/
/* #define IICA1_ENABLE */                                                                                  /** SET **/

/*------------------------------------------------------------------------------------------------------*/
/*   Define channel No.(max) + 1                                                                        */
/*------------------------------------------------------------------------------------------------------*/
#define MAX_IIC_CH_NUM          (uint8_t)(2)                                                                /** SET **/

/*------------------------------------------------------------------------------------------------------*/
/*   Define frequency as iic channel. (Please add a channel as needed.)                                 */
/*------------------------------------------------------------------------------------------------------*/
/* Freq = 400KHz at main system clock = 24MHz */
#define R_IIC_CH0_LCLK          (uint8_t)(13)           /* Channel 0 Low level width setting            */  /** SET **/
#define R_IIC_CH0_HCLK          (uint8_t)(13)           /* Channel 0 High level width setting           */  /** SET **/
#define R_IIC_CH1_LCLK          (uint8_t)(20)           /* Channel 1 Low level width setting            */  /** SET **/
#define R_IIC_CH1_HCLK          (uint8_t)(18)           /* Channel 1 High level width setting           */  /** SET **/


/*------------------------------------------------------------------------------------------------------*/
/*   Define counter.                                                                                    */
/*------------------------------------------------------------------------------------------------------*/
#define REPLY_CNT               (uint32_t)(100000)      /* Counter of non-reply errors                  */  /** SET **/
#define START_COND_WAIT         (uint16_t)(100)     /* Counter of waiting start condition generation    */  /** SET **/
#define STOP_COND_WAIT          (uint16_t)(100)         /* Counter of waiting stop condition generation */  /** SET **/
#define BUSCHK_CNT              (uint16_t)(100)         /* Counter of checking bus busy                 */  /** SET **/
#define SDACHK_CNT              (uint16_t)(100)         /* Counter of checking SDA level                */  /** SET **/
#define SCLCHK_CNT              (uint16_t)(100)         /* Counter of checking SCL level                */  /** SET **/
#define SCL_L_WAIT              (uint16_t)(100)         /* Counter of waiting SCL Low clock setting     */  /** SET **/
#define SCL_H_WAIT              (uint16_t)(100)         /* Counter of waiting SCL High clock setting    */  /** SET **/


/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
/*                              Do not change the following data.                                       */
/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
/*------------------------------------------------------------------------------------------------------*/
/*   Define return values and values of channel state flag.                                             */
/*------------------------------------------------------------------------------------------------------*/
#define R_IIC_NO_INIT           (error_t)(0)            /* Uninitialized state                          */
#define R_IIC_IDLE              (error_t)(1)            /* Idle state                                   */
#define R_IIC_FINISH            (error_t)(2)            /* Idle state (Finished communication)          */
#define R_IIC_NACK              (error_t)(3)            /* Idle state (Occured nack)                    */
#define R_IIC_COMMUNICATION     (error_t)(4)            /* Communication state                          */
#define R_IIC_LOCK_FUNC         (error_t)(5)            /* Using other API function                     */
#define R_IIC_BUS_BUSY          (error_t)(6)            /* Bus busy                                     */
#define R_IIC_ERR_PARAM         (error_t)(-1)           /* Error parameter                              */
#define R_IIC_ERR_AL            (error_t)(-2)           /* Error arbitration-lost                       */
#define R_IIC_ERR_NON_REPLY     (error_t)(-3)           /* Error non reply                              */
#define R_IIC_ERR_SDA_LOW_HOLD  (error_t)(-4)           /* Error holding SDA low                        */
#define R_IIC_ERR_OTHER         (error_t)(-5)           /* Error other                                  */

/*------------------------------------------------------------------------------------------------------*/
/*   Define R/W code for slave address.                                                                 */
/*------------------------------------------------------------------------------------------------------*/
#define W_CODE                  (uint8_t)(0x00)         /* Write code for slave address                 */
#define R_CODE                  (uint8_t)(0x01)         /* Read code for slave address                  */

/*------------------------------------------------------------------------------------------------------*/
/*   Define of port control.                                                                            */
/*------------------------------------------------------------------------------------------------------*/
#define R_IIC_HI                (uint8_t)(0x01)         /* Port "H"                                     */
#define R_IIC_LOW               (uint8_t)(0x00)         /* Port "L"                                     */
#define R_IIC_OUT               (uint8_t)(0x00)         /* Port Output                                  */
#define R_IIC_IN                (uint8_t)(0x01)         /* Port Input                                   */

/*------------------------------------------------------------------------------------------------------*/
/*   Define bool type.                                                                                  */
/*------------------------------------------------------------------------------------------------------*/
#ifdef __CA78K0R__
    #define R_IIC_FALSE             (uint8_t)(0x00)     /* False                                        */
    #define R_IIC_TRUE              (uint8_t)(0x01)     /* True                                         */
#endif  /* __CA78K0R__ */
#ifdef __CCRL__
    #define R_IIC_FALSE             (bool)(0)           /* False                                        */
    #define R_IIC_TRUE              (bool)(1)           /* True                                         */
#endif  /* __CCRL__ */
#ifdef __ICCRL78__
    #define R_IIC_FALSE             (bool)(0)           /* False                                        */
    #define R_IIC_TRUE              (bool)(1)           /* True                                         */
#endif  /* __ICCRL78__ */

#ifndef __cplusplus
#ifndef __CCRL__
#define bool                    _Bool
#endif  /* __CCRL__ */
#endif

/********************************************************************************************************
Typedef definitions
*********************************************************************************************************/

/* Data Model. */
typedef signed char     int8_t;
typedef unsigned char   uint8_t;
typedef signed short    int16_t;
typedef unsigned short  uint16_t;
typedef signed long     int32_t;
typedef unsigned long   uint32_t;
typedef signed short    error_t;        /* For CISC MCU.                                                */

/* Definitions bool type. */
#ifdef __CA78K0R__
typedef uint8_t         _Bool;
#endif  /* __CA78K0R__ */
#ifdef __CCRL__
typedef uint8_t         bool;
#endif  /* __CCRL__ */


/*----- Enumeration type. -----*/
/* Internal Status. */
typedef enum
{
    R_IIC_STS_NO_INIT = 0,              /* None initialization state                                    */
    R_IIC_STS_IDLE,                     /* Idle state                                                   */
    R_IIC_STS_ST_COND_WAIT,             /* Start condition generation completion wait state             */
    R_IIC_STS_SEND_SLVADR_W_WAIT,       /* Slave address [Write] transmission completion wait state     */
    R_IIC_STS_SEND_SLVADR_R_WAIT,       /* Slave address [Read] transmission completion wait state      */
    R_IIC_STS_SEND_DATA_WAIT,           /* Data transmission completion wait state                      */
    R_IIC_STS_RECEIVE_DATA_WAIT,        /* Data reception completion wait state                         */
    R_IIC_STS_SP_COND_WAIT,             /* Stop condition generation completion wait state              */
    R_IIC_STS_MAX                       /* Prohibition of setup above here                              */

} r_iic_drv_internal_status_t;

/* Internal Event. */
typedef enum
{
    R_IIC_EV_INIT = 0,                  /* Called function of Initializes IIC driver                    */
    R_IIC_EV_GEN_START_COND,            /* Called function of Start condition generation                */
    R_IIC_EV_SEND_SLVADR,               /* Called function of Slave address sending                     */
    R_IIC_EV_RE_SEND_SLVADR,            /* Called function of Re-slave address sending                  */
    R_IIC_EV_INT_ADD,                   /* Interrupted address sending                                  */
    R_IIC_EV_INT_SEND,                  /* Interrupted data sending                                     */
    R_IIC_EV_INT_RECEIVE,               /* Interrupted data receiving                                   */
    R_IIC_EV_INT_STOP,                  /* Interrupted Stop condition generation                        */
    R_IIC_EV_INT_AL,                    /* Interrupted Arbitration-Lost                                 */
    R_IIC_EV_INT_NACK,                  /* Interrupted No Acknowledge                                   */
    R_IIC_EV_MAX                        /* Prohibition of setup above here                              */

} r_iic_drv_internal_event_t;

/* Internal Mode. */
typedef enum
{
    R_IIC_MODE_NONE = 0,                /* Non-communication mode                                       */
    R_IIC_MODE_WRITE,                   /* Single master transmission mode                              */
    R_IIC_MODE_READ,                    /* Single master reception mode                                 */
    R_IIC_MODE_COMBINED,                /* Single master combined mode                                  */

} r_iic_drv_internal_mode_t;

/*----- Callback function type. -----*/
typedef void (*r_iic_callback)(void);                   /* Callback function type                       */


/*----- Structure type. -----*/
/* IIC Information structure. */
typedef struct
{
    uint8_t         *pSlvAdr;                           /* Pointer for Slave address buffer             */
    uint8_t         *pData1st;                          /* Pointer for 1st Data buffer                  */
    uint8_t         *pData2nd;                          /* Pointer for 2nd Data buffer                  */
    error_t         *pDevStatus;                        /* Device status flag                           */
    uint32_t        Cnt1st;                             /* 1st Data Counter                             */
    uint32_t        Cnt2nd;                             /* 2nd Data Counter                             */
    r_iic_callback  CallBackFunc;                       /* Callback function                            */
    uint8_t         ChNo;                               /* Channel No.                                  */
    uint8_t         rsv1;
    uint8_t         rsv2;
    uint8_t         rsv3;
} r_iic_drv_info_t;


/* Internal IIC Information structer. */
typedef struct
{
    r_iic_drv_internal_mode_t       Mode;               /* Internal mode of control protocol            */
    r_iic_drv_internal_status_t     N_status;           /* Internal Status of Now                       */
    r_iic_drv_internal_status_t     B_status;           /* Internal Status of Before                    */

} r_iic_drv_internal_info_t;


/********************************************************************************************************
Exported global variables
*********************************************************************************************************/
extern error_t g_iic_ChStatus[MAX_IIC_CH_NUM];                          /* Channel state flag           */
extern r_iic_drv_internal_event_t g_iic_Event[MAX_IIC_CH_NUM];          /* Event flag                   */
extern r_iic_drv_internal_info_t g_iic_InternalInfo[MAX_IIC_CH_NUM];    /* Internal status management   */
extern uint32_t g_iic_ReplyCnt[MAX_IIC_CH_NUM];                         /* Counter of non-reply         */
extern bool g_iic_Api[MAX_IIC_CH_NUM];                                  /* Api flag                     */

/********************************************************************************************************
Exported global functions (to be accessed by other files)
*********************************************************************************************************/
/* IIC Driver API functions */
error_t R_IIC_Drv_Init(r_iic_drv_info_t *pRIic_Info);
error_t r_iic_drv_init(r_iic_drv_info_t *pRIic_Info);
error_t R_IIC_Drv_MasterTx(r_iic_drv_info_t *pRIic_Info);
error_t r_iic_drv_mastertx(r_iic_drv_info_t *pRIic_Info);
error_t R_IIC_Drv_MasterRx(r_iic_drv_info_t *pRIic_Info);
error_t r_iic_drv_masterrx(r_iic_drv_info_t *pRIic_Info);
error_t R_IIC_Drv_MasterTRx(r_iic_drv_info_t *pRIic_Info);
error_t r_iic_drv_mastertrx(r_iic_drv_info_t *pRIic_Info);
error_t R_IIC_Drv_Advance(r_iic_drv_info_t *pRIic_Info);
error_t r_iic_drv_advance(r_iic_drv_info_t *pRIic_Info);
error_t R_IIC_Drv_GenClk(r_iic_drv_info_t *pRIic_Info, uint8_t ClkCnt);
error_t r_iic_drv_genclk(r_iic_drv_info_t *pRIic_Info, uint8_t ClkCnt);
error_t R_IIC_Drv_Reset(r_iic_drv_info_t *pRIic_Info);
error_t r_iic_drv_reset(r_iic_drv_info_t *pRIic_Info);


#endif /* __R_IIC_DRV_API_H__ */


/* End of File */
