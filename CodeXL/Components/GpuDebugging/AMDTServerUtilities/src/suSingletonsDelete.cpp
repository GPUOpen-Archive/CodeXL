//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suSingletonsDelete.cpp
///
//==================================================================================

//------------------------------ suSingletonsDelete.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Local:
#include <AMDTServerUtilities/Include/suGlobalVariables.h>
#include <AMDTServerUtilities/Include/suContextMonitor.h>
#include <src/suSingletonsDelete.h>
#include <AMDTServerUtilities/Include/suTechnologyMonitorsManager.h>
#include <AMDTServerUtilities/Include/suMemoryAllocationMonitor.h>

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    // Mac-only headers:
    #include <AMDTServerUtilities/Include/suMacOSXInterception.h>
    #ifdef _GR_IPHONE_DEVICE_BUILD
        #include <AMDTServerUtilities/Include/suSpyBreakpointImplementation.h>
    #endif
#endif

// ---------------------------------------------------------------------------
// Name:        suSingletonsDelete::suSingletonsDelete
// Description: Constructor - does nothing.
// Author:      Yaki Tebeka
// Date:        8/11/2009
// ---------------------------------------------------------------------------
suSingletonsDelete::suSingletonsDelete()
{
}


// ---------------------------------------------------------------------------
// Name:        suSingletonsDelete::~suSingletonsDelete
// Description:
//  Destructor - deletes GRSpiesUtilities singelton instances and other
//  GRSpiesUtilities memory that should be released.
// Author:      Yaki Tebeka
// Date:        8/11/2009
// ---------------------------------------------------------------------------
suSingletonsDelete::~suSingletonsDelete()
{
    // Delete the technology monitors manager:
    delete suTechnologyMonitorsManager::_pMySingleInstance;
    suTechnologyMonitorsManager::_pMySingleInstance = NULL;

    // Delete the default context's single instance:
    delete suContextMonitor::_pDefaultContextSingleInstance;
    suContextMonitor::_pDefaultContextSingleInstance = NULL;

    // Delete the interoperability helper:
    delete suInteroperabilityHelper::_pMySingleInstance;
    suInteroperabilityHelper::_pMySingleInstance = NULL;

    // delete the memory allocation monitor:
    delete suMemoryAllocationMonitor::m_spMySingleInstance;
    suMemoryAllocationMonitor::m_spMySingleInstance = NULL;

    // Terminate global variables:
    suTerminateGlobalVariables();

    // On Mac OS X only:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    {
        // Mac-specific items:
        delete[] su_stat_functionInterceptionInfo;
        su_stat_functionInterceptionInfo = NULL;

        // On iPhone only:
#ifdef _GR_IPHONE_DEVICE_BUILD
        {
            // iPhone on-device-specific items:
            delete[] su_stat_armv6InterceptionIslands;
            su_stat_armv6InterceptionIslands = NULL;
            delete suSpyBreakpointImplementation::_pMySingleInstance;
            suSpyBreakpointImplementation::_pMySingleInstance = NULL;
        }
#endif // _GR_IPHONE_DEVICE_BUILD
    }
#endif // ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
}

