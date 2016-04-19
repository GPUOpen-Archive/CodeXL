//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file atKernelDebuggingStepTest.h
///
//==================================================================================

//------------------------------ atKernelDebuggingStepTest.h ------------------------------

#ifndef __ATKERNELDEBUGGINGSTEPTEST_H
#define __ATKERNELDEBUGGINGSTEPTEST_H
#include "gtest/gtest.h"
#include <AMDTAPIClasses/Include/apDebugProjectSettings.h>

// Local:
#include <inc/atLoadTestsCommand.h>
//#include <inc/atTestsLogCommand.h>

// Forward declaration:
class atTestData;

// ----------------------------------------------------------------------------------
// Class Name:           atKernelDebuggingStepTest
// General Description:
//   Initializes the infrastructure to be used by the automatic tester executable.
//
// Author:      Merav Zanany
// Date:        29/11/2011
// ----------------------------------------------------------------------------------
class atKernelDebuggingStepTest
{
public:
    atKernelDebuggingStepTest(const atTestData* pTestData);
    ~atKernelDebuggingStepTest();

    void execute();

protected:

    bool setKernelSourceFilePaths(const apDebugProjectSettings& processCreationData);
    void putBreakpointsOnKernels();
    void launchExecutableForDebugSession();

    void writeBreakpointsIntoResultsFile();
    void checkTestResults();
    bool diffFiles(const gtString& newLogFile, const gtString& compareTo);

    const atTestData* m_pTestData;

};


#endif //__ATKERNELDEBUGGINGSTEPTEST_H
