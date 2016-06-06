//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apDetectedErrorParameters.h
///
//==================================================================================

//------------------------------ apDetectedErrorParameters.h ------------------------------

#ifndef __APDETECTEDERRORPARAMETERS_H
#define __APDETECTEDERRORPARAMETERS_H

// Forward decelerations:
class osChannel;

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osTransferableObject.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionId.h>
#include <AMDTAPIClasses/Include/apErrorCode.h>

// ----------------------------------------------------------------------------------
// Struct Name:          apDetectedErrorParameters : public osTransferableObject
// General Description:
//   Contains the details of a detected error
//
// Author:  AMD Developer Tools Team
// Creation Date:        12/4/2005
// ----------------------------------------------------------------------------------
struct AP_API apDetectedErrorParameters : public osTransferableObject
{
public:
    // The detected error code (see apErrorCode enumeration):
    apErrorCode _detectedErrorCode;

    // The id of a function that is associated with this error code
    // (Or -1 if there is no associated function):
    apMonitoredFunctionId _detectedErrorAssociatedFunction;

    // The detected error description:
    gtString _detectedErrorDescription;

public:
    apDetectedErrorParameters(const apDetectedErrorParameters& other);
    apDetectedErrorParameters();
    apDetectedErrorParameters& operator=(const apDetectedErrorParameters& other);
    void clearParameters();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);
};


#endif //__APDETECTEDERRORPARAMETERS_H

