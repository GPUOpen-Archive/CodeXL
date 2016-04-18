//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdStateVariablesCompareFunctor.cpp
///
//==================================================================================

//------------------------------ gdStateVariablesCompareFunctor.cpp ------------------------------

// wxWidgets per-compiled header:


// Standard C:
#include <string.h>

// Infra:
#include <AMDTAPIClasses/Include/apOpenGLStateVariablesManager.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/views/gdStateVariablesCompareFunctor.h>


// ---------------------------------------------------------------------------
// Name:        gdStateVariablesCompareFunctor::operator
// Description: Sorts an input indices arrays to according to an alphabetic order
//              of the state variables names.
// Arguments: x, y - Input indices x and y.
// Return Val: bool  - true iff state variable[x] is > from state variable[y]
//                     (in alphabetic order)
// Author:      Avi Shapira
// Date:        2/10/2005
// ---------------------------------------------------------------------------
bool gdStateVariablesCompareFunctor::operator()(const int& x, const int& y)
{
    bool retVal = false;

    // Get the state variables manager:
    apOpenGLStateVariablesManager& theStateVarsManager = apOpenGLStateVariablesManager::instance();
    const wchar_t* pStateVarXName = theStateVarsManager.stateVariableName(x);
    const wchar_t* pStateVarYName = theStateVarsManager.stateVariableName(y);

    // If Y state variable is bigger than X state variable:
    if (wcscmp(pStateVarYName, pStateVarXName) > 0)
    {
        retVal = true;
    }

    return retVal;
}
