/* Copyright (c) 2010 James Grenning and Contributed to Unity Project
 * ==========================================
 *  Unity Project - A Test Framework for C
 *  Copyright (c) 2007 Mike Karlesky, Mark VanderVoord, Greg Williams
 *  [Released under MIT License. Please refer to license.txt for details]
 * ========================================== */

#include "unity_fixture.h"

static void runAllTests(void)
{
  RUN_TEST_GROUP(OB1203_TG1);
  RUN_TEST_GROUP(SPO2_TG1);
}

int main_all_tests(int argc, const char* argv[])
{
    return UnityMain(argc, argv, runAllTests);
}

