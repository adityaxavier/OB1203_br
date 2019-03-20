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
* File Name    : r_cg_port.h
* Version      : Applilet4 for RL78/L13 V1.04.02.03 [24 May 2018]
* Device(s)    : R5F10WMG
* Tool-Chain   : IAR Systems icc78k0r
* Description  : This file implements device driver for Port module.
* Creation Date: 3/20/2019
***********************************************************************************************************************/
#ifndef PORT_H
#define PORT_H

/***********************************************************************************************************************
Macro definitions (Register bit)
***********************************************************************************************************************/
/*
    Port Mode Register (PMm)
*/
/* Pmn pin I/O mode selection (PMm7 - PMm0) */
#define _01_PMn0_NOT_USE        (0x01U) /* not use Pn0 as digital I/O */
#define _01_PMn0_MODE_INPUT     (0x01U) /* use Pn0 as input mode */
#define _00_PMn0_MODE_OUTPUT    (0x00U) /* use Pn0 as output mode */
#define _02_PMn1_NOT_USE        (0x02U) /* not use Pn1 as digital I/O */
#define _02_PMn1_MODE_INPUT     (0x02U) /* use Pn1 as input mode */
#define _00_PMn1_MODE_OUTPUT    (0x00U) /* use Pn1 as output mode */
#define _04_PMn2_NOT_USE        (0x04U) /* not use Pn2 as digital I/O */
#define _04_PMn2_MODE_INPUT     (0x04U) /* use Pn2 as input mode */
#define _00_PMn2_MODE_OUTPUT    (0x00U) /* use Pn2 as output mode */
#define _08_PMn3_NOT_USE        (0x08U) /* not use Pn3 as digital I/O */
#define _08_PMn3_MODE_INPUT     (0x08U) /* use Pn3 as input mode */
#define _00_PMn3_MODE_OUTPUT    (0x00U) /* use Pn3 as output mode */
#define _10_PMn4_NOT_USE        (0x10U) /* not use Pn4 as digital I/O */
#define _10_PMn4_MODE_INPUT     (0x10U) /* use Pn4 as input mode */
#define _00_PMn4_MODE_OUTPUT    (0x00U) /* use Pn4 as output mode */
#define _20_PMn5_NOT_USE        (0x20U) /* not use Pn5 as digital I/O */
#define _20_PMn5_MODE_INPUT     (0x20U) /* use Pn5 as input mode */
#define _00_PMn5_MODE_OUTPUT    (0x00U) /* use Pn5 as output mode */
#define _40_PMn6_NOT_USE        (0x40U) /* not use Pn6 as digital I/O */
#define _40_PMn6_MODE_INPUT     (0x40U) /* use Pn6 as input mode */
#define _00_PMn6_MODE_OUTPUT    (0x00U) /* use Pn6 as output mode */
#define _80_PMn7_NOT_USE        (0x80U) /* not use Pn7 as digital I/O */
#define _80_PMn7_MODE_INPUT     (0x80U) /* use Pn7 as input mode */
#define _00_PMn7_MODE_OUTPUT    (0x00U) /* use Pn7 as output mode */

/*
    Port Register (Pm)
*/
/* Pmn pin data (Pm0 to Pm7) */
#define _00_Pn0_OUTPUT_0        (0x00U) /* Pn0 output 0 */
#define _01_Pn0_OUTPUT_1        (0x01U) /* Pn0 output 1 */
#define _00_Pn1_OUTPUT_0        (0x00U) /* Pn1 output 0 */
#define _02_Pn1_OUTPUT_1        (0x02U) /* Pn1 output 1 */
#define _00_Pn2_OUTPUT_0        (0x00U) /* Pn2 output 0 */
#define _04_Pn2_OUTPUT_1        (0x04U) /* Pn2 output 1 */
#define _00_Pn3_OUTPUT_0        (0x00U) /* Pn3 output 0 */
#define _08_Pn3_OUTPUT_1        (0x08U) /* Pn3 output 1 */
#define _00_Pn4_OUTPUT_0        (0x00U) /* Pn4 output 0 */
#define _10_Pn4_OUTPUT_1        (0x10U) /* Pn4 output 1 */
#define _00_Pn5_OUTPUT_0        (0x00U) /* Pn5 output 0 */
#define _20_Pn5_OUTPUT_1        (0x20U) /* Pn5 output 1 */
#define _00_Pn6_OUTPUT_0        (0x00U) /* Pn6 output 0 */
#define _40_Pn6_OUTPUT_1        (0x40U) /* Pn6 output 1 */
#define _00_Pn7_OUTPUT_0        (0x00U) /* Pn7 output 0 */
#define _80_Pn7_OUTPUT_1        (0x80U) /* Pn7 output 1 */

/*
    Pull-up Resistor Option Register (PUm)
*/
/* Pmn pin on-chip pull-up resistor selection (PUmn) */
#define _00_PUn0_PULLUP_OFF     (0x00U) /* Pn0 pull-up resistor not connected */
#define _01_PUn0_PULLUP_ON      (0x01U) /* Pn0 pull-up resistor connected */
#define _00_PUn1_PULLUP_OFF     (0x00U) /* Pn1 pull-up resistor not connected */
#define _02_PUn1_PULLUP_ON      (0x02U) /* Pn1 pull-up resistor connected */
#define _00_PUn2_PULLUP_OFF     (0x00U) /* Pn2 Pull-up resistor not connected */
#define _04_PUn2_PULLUP_ON      (0x04U) /* Pn2 pull-up resistor connected */
#define _00_PUn3_PULLUP_OFF     (0x00U) /* Pn3 pull-up resistor not connected */
#define _08_PUn3_PULLUP_ON      (0x08U) /* Pn3 pull-up resistor connected */
#define _00_PUn4_PULLUP_OFF     (0x00U) /* Pn4 pull-up resistor not connected */
#define _10_PUn4_PULLUP_ON      (0x10U) /* Pn4 pull-up resistor connected */
#define _00_PUn5_PULLUP_OFF     (0x00U) /* Pn5 pull-up resistor not connected */
#define _20_PUn5_PULLUP_ON      (0x20U) /* Pn5 pull-up resistor connected */
#define _00_PUn6_PULLUP_OFF     (0x00U) /* Pn6 pull-up resistor not connected */
#define _40_PUn6_PULLUP_ON      (0x40U) /* Pn6 pull-up resistor connected */
#define _00_PUn7_PULLUP_OFF     (0x00U) /* Pn7 pull-up resistor not connected */
#define _80_PUn7_PULLUP_ON      (0x80U) /* Pn7 pull-up resistor connected */

/*
    Port Input Mode Register (PIMm)
*/
/* Pmn pin input buffer selection (PIMmn) */
#define _00_PIMn3_TTL_OFF       (0x00U) /* set Pn3 normal input buffer */
#define _08_PIMn3_TTL_ON        (0x08U) /* set Pn3 TTL input buffer */
#define _00_PIMn4_TTL_OFF       (0x00U) /* set Pn4 normal input buffer */
#define _10_PIMn4_TTL_ON        (0x10U) /* set Pn4 TTL input buffer */
#define _00_PIMn5_TTL_OFF       (0x00U) /* set Pn5 normal input buffer */
#define _20_PIMn5_TTL_ON        (0x20U) /* set Pn5 TTL input buffer */
#define _00_PIMn6_TTL_OFF       (0x00U) /* set Pn6 normal input buffer */
#define _40_PIMn6_TTL_ON        (0x40U) /* set Pn6 TTL input buffer */
#define _00_PIMn7_TTL_OFF       (0x00U) /* set Pn7 normal input buffer */
#define _80_PIMn7_TTL_ON        (0x80U) /* set Pn7 TTL input buffer */

/*
    Port Output Mode Register (POMm)
*/
/* Pmn pin output mode selection (POMmn) */
#define _00_POMn0_NCH_OFF       (0x00U) /* set Pn0 output normal mode */
#define _01_POMn0_NCH_ON        (0x01U) /* set Pn0 output N-ch open-drain mode */
#define _00_POMn2_NCH_OFF       (0x00U) /* set Pn2 output normal mode */
#define _04_POMn2_NCH_ON        (0x04U) /* set Pn2 output N-ch open-drain mode */
#define _00_POMn3_NCH_OFF       (0x00U) /* set Pn3 output normal mode */
#define _08_POMn3_NCH_ON        (0x08U) /* set Pn3 output N-ch open-drain mode */
#define _00_POMn4_NCH_OFF       (0x00U) /* set Pn4 output normal mode */
#define _10_POMn4_NCH_ON        (0x10U) /* set Pn4 output N-ch open-drain mode */
#define _00_POMn5_NCH_OFF       (0x00U) /* set Pn5 output normal mode */
#define _20_POMn5_NCH_ON        (0x20U) /* set Pn5 output N-ch open-drain mode */
#define _00_POMn6_NCH_OFF       (0x00U) /* set Pn6 output normal mode */
#define _40_POMn6_NCH_ON        (0x40U) /* set Pn6 output N-ch open-drain mode */
#define _00_POMn7_NCH_OFF       (0x00U) /* set Pn7 output normal mode */
#define _80_POMn7_NCH_ON        (0x80U) /* set Pn7 output N-ch open-drain mode */

/*
    Port Operation Mode Register (PMCm)
*/
/* Pmn pin digital input buffer selection (PMCmn) */
#define _01_PMCn0_NOT_USE       (0x01U) /* not use Pn0 digital input */
#define _00_PMCn0_DI_ON         (0x00U) /* enable Pn0 digital input */
#define _02_PMCn1_NOT_USE       (0x02U) /* not use Pn1 digital input */
#define _00_PMCn1_DI_ON         (0x00U) /* enable Pn1 digital input */
#define _04_PMCn2_NOT_USE       (0x04U) /* not use Pn2 digital input */
#define _00_PMCn2_DI_ON         (0x00U) /* enable Pn2 digital input */
#define _08_PMCn3_NOT_USE       (0x08U) /* not use Pn3 digital input */
#define _00_PMCn3_DI_ON         (0x00U) /* enable Pn3 digital input */
#define _10_PMCn4_NOT_USE       (0x10U) /* not use Pn4 digital input */
#define _00_PMCn4_DI_ON         (0x00U) /* enable Pn4 digital input */
#define _20_PMCn5_NOT_USE       (0x20U) /* not use Pn5 digital input */
#define _00_PMCn5_DI_ON         (0x00U) /* enable Pn5 digital input */
#define _40_PMCn6_NOT_USE       (0x40U) /* not use Pn6 digital input */
#define _00_PMCn6_DI_ON         (0x00U) /* enable Pn6 digital input */
#define _80_PMCn7_NOT_USE       (0x80U) /* not use Pn7 digital input */
#define _00_PMCn7_DI_ON         (0x00U) /* enable Pn7 digital input */

/*
    AD port configuration register (ADPC)
*/
#define _00_ADPC_DI_OFF         (0x00U) /* use P21, P20 as analog input */
#define _01_ADPC_DI_ON          (0x01U) /* use P21, P20 as digital input */
#define _02_ADPC_DI_ON          (0x02U) /* use P20 as digital input */

/*
    LCD port function registers 00 (PFSEG00)
*/
/* Port (other than segment output)/segment outputs specification of Pmn pins (PFSEGxx) */
#define _00_PFSEG04_PORT        (0x00U) /* used the P50 pin as port (other than segment output) */
#define _10_PFSEG04_SEG         (0x10U) /* used the P50 pin as segment output */
#define _00_PFSEG05_PORT        (0x00U) /* used the P51 pin as port (other than segment output) */
#define _20_PFSEG05_SEG         (0x20U) /* used the P51 pin as segment output */
#define _00_PFSEG06_PORT        (0x00U) /* used the P52 pin as port (other than segment output) */
#define _40_PFSEG06_SEG         (0x40U) /* used the P52 pin as segment output */
#define _00_PFSEG07_PORT        (0x00U) /* used the P53 pin as port (other than segment output) */
#define _80_PFSEG07_SEG         (0x80U) /* used the P53 pin as segment output */

/*
    LCD port function registers 01 (PFSEG01)
*/
/* Port (other than segment output)/segment outputs specification of Pmn pins (PFSEGxx) */
#define _00_PFSEG08_PORT        (0x00U) /* used the P54 pin as port (other than segment output) */
#define _01_PFSEG08_SEG         (0x01U) /* used the P54 pin as segment output */
#define _00_PFSEG09_PORT        (0x00U) /* used the P55 pin as port (other than segment output) */
#define _02_PFSEG09_SEG         (0x02U) /* used the P55 pin as segment output */
#define _00_PFSEG10_PORT        (0x00U) /* used the P56 pin as port (other than segment output) */
#define _04_PFSEG10_SEG         (0x04U) /* used the P56 pin as segment output */
#define _00_PFSEG11_PORT        (0x00U) /* used the P57 pin as port (other than segment output) */
#define _08_PFSEG11_SEG         (0x08U) /* used the P57 pin as segment output */
#define _00_PFSEG12_PORT        (0x00U) /* used the P70 pin as port (other than segment output) */
#define _10_PFSEG12_SEG         (0x10U) /* used the P70 pin as segment output */
#define _00_PFSEG13_PORT        (0x00U) /* used the P71 pin as port (other than segment output) */
#define _20_PFSEG13_SEG         (0x20U) /* used the P71 pin as segment output */
#define _00_PFSEG14_PORT        (0x00U) /* used the P72 pin as port (other than segment output) */
#define _40_PFSEG14_SEG         (0x40U) /* used the P72 pin as segment output */
#define _00_PFSEG15_PORT        (0x00U) /* used the P73 pin as port (other than segment output) */
#define _80_PFSEG15_SEG         (0x80U) /* used the P73 pin as segment output */

/*
    LCD port function registers 02 (PFSEG02)
*/
/* Port (other than segment output)/segment outputs specification of Pmn pins (PFSEGxx) */
#define _00_PFSEG16_PORT        (0x00U) /* used the P74 pin as port (other than segment output) */
#define _01_PFSEG16_SEG         (0x01U) /* used the P74 pin as segment output */
#define _00_PFSEG17_PORT        (0x00U) /* used the P75 pin as port (other than segment output) */
#define _02_PFSEG17_SEG         (0x02U) /* used the P75 pin as segment output */
#define _00_PFSEG18_PORT        (0x00U) /* used the P76 pin as port (other than segment output) */
#define _04_PFSEG18_SEG         (0x04U) /* used the P76 pin as segment output */
#define _00_PFSEG19_PORT        (0x00U) /* used the P77 pin as port (other than segment output) */
#define _08_PFSEG19_SEG         (0x08U) /* used the P77 pin as segment output */
#define _00_PFSEG20_PORT        (0x00U) /* used the P30 pin as port (other than segment output) */
#define _10_PFSEG20_SEG         (0x10U) /* used the P30 pin as segment output */
#define _00_PFSEG21_PORT        (0x00U) /* used the P31 pin as port (other than segment output) */
#define _20_PFSEG21_SEG         (0x20U) /* used the P31 pin as segment output */
#define _00_PFSEG22_PORT        (0x00U) /* used the P32 pin as port (other than segment output) */
#define _40_PFSEG22_SEG         (0x40U) /* used the P32 pin as segment output */
#define _00_PFSEG23_PORT        (0x00U) /* used the P33 pin as port (other than segment output) */
#define _80_PFSEG23_SEG         (0x80U) /* used the P33 pin as segment output */

/*
    LCD port function registers 03 (PFSEG03)
*/
/* Port (other than segment output)/segment outputs specification of Pmn pins (PFSEGxx) */
#define _00_PFSEG24_PORT        (0x00U) /* used the P34 pin as port (other than segment output) */
#define _01_PFSEG24_SEG         (0x01U) /* used the P34 pin as segment output */
#define _00_PFSEG25_PORT        (0x00U) /* used the P35 pin as port (other than segment output) */
#define _02_PFSEG25_SEG         (0x02U) /* used the P35 pin as segment output */
#define _00_PFSEG26_PORT        (0x00U) /* used the P46 pin as port (other than segment output) */
#define _08_PFSEG26_SEG         (0x08U) /* used the P46 pin as segment output */
#define _00_PFSEG27_PORT        (0x00U) /* used the P47 pin as port (other than segment output) */
#define _10_PFSEG27_SEG         (0x10U) /* used the P47 pin as segment output */
#define _00_PFSEG28_PORT        (0x00U) /* used the P130 pin as port (other than segment output) */
#define _20_PFSEG28_SEG         (0x20U) /* used the P130 pin as segment output */
#define _00_PFSEG29_PORT        (0x00U) /* used the P22 pin as port (other than segment output) */
#define _40_PFSEG29_SEG         (0x40U) /* used the P22 pin as segment output */
#define _00_PFSEG30_PORT        (0x00U) /* used the P23 pin as port (other than segment output) */
#define _80_PFSEG30_SEG         (0x80U) /* used the P23 pin as segment output */
#define _00_PFDEG_PORT          (0x00U) /* used the P45 pin as port */
#define _04_PFDEG_IVREF0        (0x04U) /* used the P45 pin as analog input (IVREF0) */

/*
    LCD port function registers 04 (PFSEG04)
*/
/* Port (other than segment output)/segment outputs specification of Pmn pins (PFSEGxx) */
#define _00_PFSEG31_PORT        (0x00U) /* used the P24 pin as port (other than segment output) */
#define _01_PFSEG31_SEG         (0x01U) /* used the P24 pin as segment output */
#define _00_PFSEG32_PORT        (0x00U) /* used the P25 pin as port (other than segment output) */
#define _02_PFSEG32_SEG         (0x02U) /* used the P25 pin as segment output */
#define _00_PFSEG33_PORT        (0x00U) /* used the P26 pin as port (other than segment output) */
#define _04_PFSEG33_SEG         (0x04U) /* used the P26 pin as segment output */
#define _00_PFSEG34_PORT        (0x00U) /* used the P27 pin as port (other than segment output) */
#define _08_PFSEG34_SEG         (0x08U) /* used the P27 pin as segment output */
#define _00_PFSEG35_PORT        (0x00U) /* used the P10 pin as port (other than segment output) */
#define _10_PFSEG35_SEG         (0x10U) /* used the P10 pin as segment output */
#define _00_PFSEG36_PORT        (0x00U) /* used the P11 pin as port (other than segment output) */
#define _20_PFSEG36_SEG         (0x20U) /* used the P11 pin as segment output */
#define _00_PFSEG37_PORT        (0x00U) /* used the P12 pin as port (other than segment output) */
#define _40_PFSEG37_SEG         (0x40U) /* used the P12 pin as segment output */
#define _00_PFSEG38_PORT        (0x00U) /* used the P13 pin as port (other than segment output) */
#define _80_PFSEG38_SEG         (0x80U) /* used the P13 pin as segment output */

/*
    LCD port function registers 05 (PFSEG05)
*/
/* Port (other than segment output)/segment outputs specification of Pmn pins (PFSEGxx) */
#define _00_PFSEG39_PORT        (0x00U) /* used the P14 pin as port (other than segment output) */
#define _01_PFSEG39_SEG         (0x01U) /* used the P14 pin as segment output */
#define _00_PFSEG40_PORT        (0x00U) /* used the P15 pin as port (other than segment output) */
#define _02_PFSEG40_SEG         (0x02U) /* used the P15 pin as segment output */
#define _00_PFSEG41_PORT        (0x00U) /* used the P16 pin as port (other than segment output) */
#define _04_PFSEG41_SEG         (0x04U) /* used the P16 pin as segment output */
#define _00_PFSEG42_PORT        (0x00U) /* used the P17 pin as port (other than segment output) */
#define _08_PFSEG42_SEG         (0x08U) /* used the P17 pin as segment output */
#define _00_PFSEG43_PORT        (0x00U) /* used the P00 pin as port (other than segment output) */
#define _10_PFSEG43_SEG         (0x10U) /* used the P00 pin as segment output */
#define _00_PFSEG44_PORT        (0x00U) /* used the P01 pin as port (other than segment output) */
#define _20_PFSEG44_SEG         (0x20U) /* used the P01 pin as segment output */
#define _00_PFSEG45_PORT        (0x00U) /* used the P02 pin as port (other than segment output) */
#define _40_PFSEG45_SEG         (0x40U) /* used the P02 pin as segment output */
#define _00_PFSEG46_PORT        (0x00U) /* used the P03 pin as port (other than segment output) */
#define _80_PFSEG46_SEG         (0x80U) /* used the P03 pin as segment output */

/*
    LCD port function registers 06 (PFSEG06)
*/
/* Port (other than segment output)/segment outputs specification of Pmn pins (PFSEGxx) */
#define _00_PFSEG47_PORT        (0x00U) /* used the P04 pin as port (other than segment output) */
#define _01_PFSEG47_SEG         (0x01U) /* used the P04 pin as segment output */
#define _00_PFSEG48_PORT        (0x00U) /* used the P05 pin as port (other than segment output) */
#define _02_PFSEG48_SEG         (0x02U) /* used the P05 pin as segment output */
#define _00_PFSEG49_PORT        (0x00U) /* used the P06 pin as port (other than segment output) */
#define _04_PFSEG49_SEG         (0x04U) /* used the P06 pin as segment output */

/*
    LCD port function registers 06 (PFSEG06)
*/
/* Port (other than segment output)/segment outputs specification of Pmn pins (PFSEGxx) */
#define _00_PFSEG50_PORT        (0x00U) /* used the P07 pin as port (other than segment output) */
#define _08_PFSEG50_SEG         (0x08U) /* used the P07 pin as segment output */

/*
    LCD input switch control register (ISCLCD)
*/
/* Control of schmitt trigger buffer of CAPL/P126 and CAPH/P127 pins (ISCCAP) */
#define _00_ISCCAP_INVALID      (0x00U) /* makes digital input invalid */
#define _01_ISCCAP_VALID        (0x01U) /* makes digital input valid */
/* Control of schmitt trigger buffer of VL3/P125 pin (ISCVL3) */
#define _00_ISCVL3_INVALID      (0x00U) /* makes digital input invalid */
#define _02_ISCVL3_VALID        (0x02U) /* makes digital input valid */

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define _C0_PM3_DEFAULT         (0xC0U) /* PM3 default value */
#define _FC_PM6_DEFAULT         (0xFCU) /* PM6 default value */
#define _1F_PM12_DEFAULT        (0x1FU) /* PM12 default value */
#define _FE_PM13_DEFAULT        (0xFEU) /* PM13 default value */
#define _F0_PMC1_DEFAULT        (0xF0U) /* PMC1 default value */
#define _03_PMC2_DEFAULT        (0x03U) /* PMC2 default value */

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Global functions
***********************************************************************************************************************/
void R_PORT_Create(void);

/* Start user code for function. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
#endif