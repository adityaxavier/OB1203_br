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
* Copyright (C) 2015 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name	   : test_OB1203_I2C.c
* Description  : Unit tests to check the I2C interface of the OB1203 Class
***********************************************************************************************************************/
/**********************************************************************************************************************
* History : DD.MM.YYYY Version  Description
*         : 06.16.2016 1.00     First Release
***********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @file
 ***********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @ingroup TESTING
 * @defgroup TEST_GROUP1
 * @brief This is a brief description
 * @{
 * Sample Usage:
 *  TEST_GROUP_RUNNER(OB1203_TG1)
 *  {
 *      RUN_TEST_CASE(OB1203_TG1, TC_1_0_GetVersion);
 *      RUN_TEST_CASE(OB1203_TG1, TC_1_1_Open_Invalid_Param_test);
 *      RUN_TEST_CASE(OB1203_TG1, TC_1_2_Open_Close_Handle);
 *      RUN_TEST_CASE(OB1203_TG1, TC_1_3_Scan_Multiple_Handles);
 *      RUN_TEST_CASE(OB1203_TG1, TC_1_4_Read_Results);
 *      RUN_TEST_CASE(OB1203_TG1, TC_1_5_Control_Get_Tests);
 *      RUN_TEST_CASE(OB1203_TG1, TC_1_6_Control_Set_Tests);
 *      RUN_TEST_CASE(OB1203_TG1, TC_1_7_Calibrate_Handle);
 *  }
 *
 ***********************************************************************************************************************/
/***********************************************************************************************************************
* Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
#include <stdio.h>
#include <string.h>
#include "unity_fixture.h"

#include "OB1203.h"

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/


/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Exported global variables (to be accessed by other files)
***********************************************************************************************************************/


/***********************************************************************************************************************
Private global variables and functions
***********************************************************************************************************************/

/* Define test groups. */
TEST_GROUP(OB1203_TG1);

/***********************************************************************************************************************
* Function Name: TEST_OB1203_TG1_SETUP
* Description  : Setup for these unit tests. This will be run before every test case.
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
TEST_SETUP(OB1203_TG1)
{

}
/***********************************************************************************************************************
End of function TEST_OB1203_TG1_SETUP
***********************************************************************************************************************/

/***********************************************************************************************************************
* Function Name: TEST_OB1203_TG1_TEAR_DOWN
* Description  : Tear down for these unit tests. This will be run after each test case.
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
TEST_TEAR_DOWN(OB1203_TG1)
{

}
/***********************************************************************************************************************
End of function TEST_OB1203_TG1_TEAR_DOWN
***********************************************************************************************************************/

/***********************************************************************************************************************
* Function Name: test_Initialization
* Description  : Call the initialization routine to check if the minimum set of variables are set.
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
TEST(OB1203_TG1, test_Initialization)
{
  OB1203 ob1203;
  //TEST_ASSERT_EQUAL_UINT8(OB1203_ADDR, ob1203.i2c_addr);        // Address is a private variable. Ignoring test.
  TEST_ASSERT_EQUAL_UINT(MD_OK, ob1203.i2c_error);
  TEST_ASSERT_FALSE(ob1203.ready);
  TEST_ASSERT_TRUE(ob1203.send_stop);
}
/***********************************************************************************************************************
End of function test_Initialization
***********************************************************************************************************************/
