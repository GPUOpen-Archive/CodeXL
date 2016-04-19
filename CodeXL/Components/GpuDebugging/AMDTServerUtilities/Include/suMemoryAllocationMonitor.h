//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suMemoryAllocationMonitor.h
///
//==================================================================================

//------------------------------ suMemoryAllocationMonitor.h ------------------------------

#ifndef __SUMEMORYALLOCATIONMONITOR_H
#define __SUMEMORYALLOCATIONMONITOR_H

// Local:
#include <AMDTServerUtilities/Include/suSpiesUtilitiesDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:          SU_API suMemoryAllocationMonitor
// General Description: A class used to Monitor memory allocation
// Author:              Gilad Yarnitzky
// Creation Date:       6/1/2015
// ----------------------------------------------------------------------------------
class SU_API suMemoryAllocationMonitor
{
private:
    friend class suSingletonsDelete;

public:
    static suMemoryAllocationMonitor& instance();
    ~suMemoryAllocationMonitor();

    static void HandlerMemoryAllocationFailure();

private:
    // The constructor should only be called by the instance() function:
    suMemoryAllocationMonitor();

private:
    // My single instance:
    static suMemoryAllocationMonitor* m_spMySingleInstance;
};

#endif //__SUMEMORYALLOCATIONMONITOR_H

