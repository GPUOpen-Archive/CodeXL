//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdBreakpointsItemData.h
///
//==================================================================================

//------------------------------ gdBreakpointsItemData.h ------------------------------

#ifndef __GDBREAKPOINTSITEMDATA_H
#define __GDBREAKPOINTSITEMDATA_H

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osTransferableObjectType.h>
#include <AMDTOSAPIWrappers/Include/oaOSAPIDefinitions.h>
#include <AMDTAPIClasses/Include/apGenericBreakpoint.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionId.h>



// ----------------------------------------------------------------------------------
// Class Name:          gdBreakpointsItemData
// General Description: Used for a description of a breakpoints
// Author:              Sigal Algranaty
// Creation Date:       7/9/2011
// ----------------------------------------------------------------------------------

class gdBreakpointsItemData
{
public:

    gdBreakpointsItemData();
    gdBreakpointsItemData(const gtAutoPtr<apBreakPoint>& aptrBreakpoint);
    ~gdBreakpointsItemData() {};

    // The index of the breakpoint in the API:
    int breakpointAPIIndex();

    // Is this breakpoint data represents the same breakpoint as aptrBreakpoint:
    bool sameAs(const gtAutoPtr<apBreakPoint>& aptrBreakpoint);

    // Breakpoint type:
    osTransferableObjectType _breakpointType;

    // The breakpoint kernel function name:
    gtString _kernelFunctionName;

    // The id of the wrapped function:
    apMonitoredFunctionId _monitoredFunctionId;

    // The generic breakpoint type:
    apGenericBreakpointType _genericBreakpointType;

    // Kernel source code breakpoint:
    oaCLProgramHandle _clProgramHandle;

    // Source code breakpoint:
    osFilePath _sourceCodeFilePath;
    int _sourceCodeLine;

    // Breakpoint hit count:
    int _hitCount;

    // Is breakpoint enabled:
    bool _isEnabled;
};

#endif //__GDBREAKPOINTSITEMDATA_H

