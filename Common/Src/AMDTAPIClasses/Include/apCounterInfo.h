//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCounterInfo.h
///
//==================================================================================

//------------------------------ apCounterInfo.h ------------------------------

#ifndef __APCOUNTERINFO
#define __APCOUNTERINFO

// Standard C:
#include "float.h"

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osTransferableObject.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTAPIClasses/Include/apCounterScope.h>
enum apCounterType;

// Pre declaration:
struct apCounterID;


// Infinite counter values:
#define AP_INFINITE_COUNTER_VALUE DBL_MAX

// An auto scaled counter:
#define AP_AUTO_SCALED_COUNTER DBL_MAX

enum apCounterDataType
{
    AP_COUNTER_DATA_NUMBER,
    AP_COUNTER_DATA_PERCENT
};

// ----------------------------------------------------------------------------------
// Class Name:           apCounterInfo : public osTransferableObject
// General Description: Holds a single counter information.
// Author:  AMD Developer Tools Team
// Creation Date:        30/6/2005
// ----------------------------------------------------------------------------------
class AP_API apCounterInfo : public osTransferableObject
{
public:
    // The counter name:
    gtString _name;

    // The counter OS id:
    gtString _osID;

    // The counter type:
    apCounterType _counterType;

    // Contains the counter instantiation type (global / per context / per queue):
    apCounterScope::apCounterScopeType _counterScopeType;

    // The index of the GPU that exposes the counter:
    // (-1 if unknown)
    int _GPUIndex;

    // The counter description:
    gtString _description;

    // The counter min value:
    double _minValue;

    // The counter max value:
    // (can also be AP_INFINITE_COUNTER_VALUE)
    double _maxValue;

    // The counter default scale:
    // (can also be AP_AUTO_SCALED_COUNTER)
    double _defaultScale;

    // The counter data type (number / percent):
    apCounterDataType _counterDataType;

    // Builds a counter description for a counter ID:
    void getDescriptionWithScope(const apCounterScope& counterScope, gtString& counterDescription) const;
    void getNameWithScope(const apCounterScope& counterScope, gtString& counterName) const;


public:
    apCounterInfo();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);
};


#endif  // __APCOUNTERINFO
