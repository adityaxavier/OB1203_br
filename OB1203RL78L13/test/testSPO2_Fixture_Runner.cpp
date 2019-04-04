/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
* other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
* applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
* EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
* SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS
* SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
* this software. By using this software, you agree to the additional terms and conditions found by accessing the
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2014 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : r_ctsu_synergy_common_test_runner.c
* Description  : Unity unit tests for <Module Name>
***********************************************************************************************************************/
/**********************************************************************************************************************
* History : DD.MM.YYYY Version  Description
*           04.05.2015 1.00     Initial Release
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
#include "unity_fixture.h"

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define RUN_TG1     (false)
#define RUN_TG2     (false)
#define RUN_TG3     (false)
#define RUN_TG4     (false)

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Exported global variables (to be accessed by other files)
***********************************************************************************************************************/

/***********************************************************************************************************************
Private global variables and functions
***********************************************************************************************************************/
static void runAllTests(void);

TEST_GROUP_RUNNER(SPO2_TG1)
{
  //RUN_TEST_CASE(SPO2_TG1, test_uint_sqrt32);
  //RUN_TEST_CASE(SPO2_TG1, test_uint_sqrt64);
  RUN_TEST_CASE(SPO2_TG1, test_get_std);
  RUN_TEST_CASE(SPO2_TG1, test_get_avg);
  
  
  RUN_TEST_CASE(SPO2_TG1, test_Initialization);
}

/***********************************************************************************************************************
* Function Name: runAllTests
* Description  : Call test groups
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
static void runAllTests(void)
{
	/* Get module software versions */
	RUN_TEST_GROUP(SPO2_TG1);

}
/***********************************************************************************************************************
End of function runAllTests
***********************************************************************************************************************/

/***********************************************************************************************************************
* Function Name: test_SPO2
* Description  : Tear down for these unit tests
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
int test_SPO2_fixture_runner(void)
{
    char const * args[2] = {"", "-v"};
    UnityBegin("testSPO2.cpp");
    UnityMain(2, args, runAllTests);
    return UnityEnd();
}
/***********************************************************************************************************************
End of function test_ctsu
***********************************************************************************************************************/
