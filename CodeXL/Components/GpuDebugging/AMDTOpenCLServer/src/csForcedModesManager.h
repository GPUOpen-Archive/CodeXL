//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csForcedModesManager.h
///
//==================================================================================

//------------------------------ csForcedModesManager.h ------------------------------

#ifndef __CSFORCEDMODESMANAGER
#define __CSFORCEDMODESMANAGER

// Infra:
#include <AMDTAPIClasses/Include/apApplicationModesEventsType.h>

// ----------------------------------------------------------------------------------
// Class Name:           csForcedModesManager
// General Description:  Manages, applies and removed forces modes for OpenCL applications
// Author:               Sigal Algranaty
// Creation Date:        6/5/2010
// ----------------------------------------------------------------------------------
class csForcedModesManager
{
public:
    csForcedModesManager();
    ~csForcedModesManager();

public:

    // Set operation execution mode:
    void setExecutionMode(apOpenCLExecutionType operationExecutionType, bool isOn) {_isModeForced[operationExecutionType] = isOn;};

    // Query "is mode on":
    bool isOperationExecutionOn(apOpenCLExecutionType operationExecutionType) const { return _isModeForced[operationExecutionType]; };

private:

    // Are forced modes "on":
    bool _isModeForced[AP_OPENCL_AMOUNT_OF_EXECUTIONS];
};

#endif  // __CSFORCEDMODESMANAGER
