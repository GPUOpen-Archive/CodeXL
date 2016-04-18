//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suInteroperabilityHelper.h
///
//==================================================================================

//------------------------------ suInteroperabilityHelper.h ------------------------------

#ifndef __SUINTEROPERABILITYHELPER_H
#define __SUINTEROPERABILITYHELPER_H

// Local:
#include <AMDTServerUtilities/Include/suSpiesUtilitiesDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:          SU_API suInteroperabilityHelper
// General Description: A class used to help interoperability between different APIs
//                      Any sort of information or flags that need to be passed between
//                      two different API servers (also without knowing which ones are
//                      already loaded) should be placed here).
// Author:              Uri Shomroni
// Creation Date:       9/11/2011
// ----------------------------------------------------------------------------------
class SU_API suInteroperabilityHelper
{
private:
    friend class suSingletonsDelete;

public:
    static suInteroperabilityHelper& instance();
    ~suInteroperabilityHelper();

    // Nested functions:
    void onNestedFunctionEntered() {_nestedFunctionCount++;};
    void onNestedFunctionExited();
    bool isInNestedFunction() {return (_nestedFunctionCount > 0);};

private:
    // The constructor should only be called by the instance() function:
    suInteroperabilityHelper();

private:
    // My single instance:
    static suInteroperabilityHelper* _pMySingleInstance;

    // This integer contain the amount of certain API calls (such as WGL function calls).
    // Some API functions call other nested functions through dynamic linking instead of static linking.
    // This variable is used preventing the API servers from logging the function call and doing other
    // redundant works in such cases.
    int _nestedFunctionCount;
};

#endif //__SUINTEROPERABILITYHELPER_H

