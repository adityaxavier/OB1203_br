/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products.
* No other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
* applicable laws, including copyright laws. 
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING THIS SOFTWARE, WHETHER EXPRESS, IMPLIED
* OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY
* LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE FOR ANY DIRECT,
* INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR
* ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability 
* of this software. By using this software, you agree to the additional terms and conditions found by accessing the 
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2013, 2018 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/

/***********************************************************************************************************************
* File Name    : r_cg_port.c
* Version      : Applilet4 for RL78/L13 V1.04.02.03 [24 May 2018]
* Device(s)    : R5F10WMG
* Tool-Chain   : IAR Systems icc78k0r
* Description  : This file implements device driver for Port module.
* Creation Date: 3/26/2019
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_macrodriver.h"
#include "r_cg_port.h"
/* Start user code for include. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
#include "r_cg_userdefine.h"

/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/
/* Start user code for pragma. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
/* Start user code for global. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
* Function Name: R_PORT_Create
* Description  : This function initializes the Port I/O.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_PORT_Create(void)
{
    PFSEG0 = _80_PFSEG07_SEG | _40_PFSEG06_SEG | _20_PFSEG05_SEG | _10_PFSEG04_SEG;
    PFSEG1 = _80_PFSEG15_SEG | _40_PFSEG14_SEG | _20_PFSEG13_SEG | _10_PFSEG12_SEG | _08_PFSEG11_SEG | 
             _04_PFSEG10_SEG | _02_PFSEG09_SEG | _01_PFSEG08_SEG;
    PFSEG2 = _80_PFSEG23_SEG | _40_PFSEG22_SEG | _20_PFSEG21_SEG | _10_PFSEG20_SEG | _08_PFSEG19_SEG | 
             _04_PFSEG18_SEG | _02_PFSEG17_SEG | _01_PFSEG16_SEG;
    PFSEG3 = _80_PFSEG30_SEG | _40_PFSEG29_SEG | _20_PFSEG28_SEG | _10_PFSEG27_SEG | _08_PFSEG26_SEG | 
             _02_PFSEG25_SEG | _01_PFSEG24_SEG | _00_PFDEG_PORT;
    PFSEG4 = _80_PFSEG38_SEG | _40_PFSEG37_SEG | _20_PFSEG36_SEG | _10_PFSEG35_SEG | _08_PFSEG34_SEG | 
             _04_PFSEG33_SEG | _02_PFSEG32_SEG | _01_PFSEG31_SEG;
    PFSEG5 = _00_PFSEG46_PORT | _40_PFSEG45_SEG | _20_PFSEG44_SEG | _10_PFSEG43_SEG | _08_PFSEG42_SEG | 
             _04_PFSEG41_SEG | _02_PFSEG40_SEG | _01_PFSEG39_SEG;
    PFSEG6 = _08_PFSEG50_SEG | _04_PFSEG49_SEG | _00_PFSEG48_PORT | _01_PFSEG47_SEG;
    P0 = _00_Pn3_OUTPUT_0 | _20_Pn5_OUTPUT_1;
    P4 = _02_Pn1_OUTPUT_1 | _20_Pn5_OUTPUT_1;
    PM0 = _01_PMn0_NOT_USE | _02_PMn1_NOT_USE | _04_PMn2_NOT_USE | _00_PMn3_MODE_OUTPUT | _10_PMn4_NOT_USE | 
          _00_PMn5_MODE_OUTPUT | _40_PMn6_NOT_USE | _80_PMn7_NOT_USE;
    PM4 = _01_PMn0_NOT_USE | _00_PMn1_MODE_OUTPUT | _04_PMn2_NOT_USE | _08_PMn3_NOT_USE | _10_PMn4_NOT_USE | 
          _00_PMn5_MODE_OUTPUT | _40_PMn6_NOT_USE | _80_PMn7_NOT_USE;
}

/* Start user code for adding. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
