//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apOpenCLErrorParameters.h
///
//==================================================================================

//------------------------------ apOpenCLErrorParameters.h ------------------------------

#ifndef __APOPENCLERRORPARAMETERS_H
#define __APOPENCLERRORPARAMETERS_H

// Forward decelerations:
class osChannel;
class apFunctionCall;

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osTransferableObject.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>


// ----------------------------------------------------------------------------------
// Struct Name:          apOpenCLErrorParameters : public osTransferableObject
// General Description:
//   Contains the details of an OpenCL error
//
// Author:  AMD Developer Tools Team
// Creation Date:        22/2/2010
// ----------------------------------------------------------------------------------
struct AP_API apOpenCLErrorParameters : public osTransferableObject
{
public:
    // The details of the function that triggered the breakpoint event:
    gtAutoPtr<apFunctionCall> _aptrBreakedOnFunctionCall;

    // OpenCL error code:
    int _openCLErrorCode;

public:
    apOpenCLErrorParameters(const apOpenCLErrorParameters& other);
    apOpenCLErrorParameters();
    apOpenCLErrorParameters& operator=(const apOpenCLErrorParameters& other);
    void clearParameters();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);
};


#endif //__APOPENCLERRORPARAMETERS_H

