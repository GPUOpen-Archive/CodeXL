//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file atGpuDebuggingTest.h
///
//==================================================================================
#ifndef __ATGPUDEBUGGINGTEST_H
#define __ATGPUDEBUGGINGTEST_H

#include <cstring>
#include "gtest/gtest.h"

#include <inc/atPrintableTestData.h>

using ::testing::TestWithParam;
using ::testing::Values;


/// -----------------------------------------------------------------------------------------------
/// \class Name: atGpuDebuggingTest : public TestWithParam<atTestData*>
/// \brief Description: Test fixture class which uses parameters to generate parameterized test cases
/// \tparam atTestData*: a pointer to the collection of parameters that this test takes
/// -----------------------------------------------------------------------------------------------
class atGpuDebuggingTest : public TestWithParam<atPrintableTestData>
{
public:
    virtual void SetUp();
    virtual void TearDown();

    static bool addInstallFolderToPathEnvVariable();
protected:

};

#endif //__ATGPUDEBUGGINGTEST_H

