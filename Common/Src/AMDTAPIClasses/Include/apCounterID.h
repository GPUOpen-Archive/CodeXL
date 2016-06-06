//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCounterID.h
///
//==================================================================================

//------------------------------ apCounterID.h ------------------------------

#ifndef __APCOUNTERID
#define __APCOUNTERID

// Standard C:
#include "float.h"

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osTransferableObject.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTAPIClasses/Include/apContextID.h>
#include <AMDTAPIClasses/Include/apCounterScope.h>
enum apCounterType;


// ----------------------------------------------------------------------------------
// Class Name:           apCounterID : public osTransferableObject
// General Description: Holds a performance counter's Identification information
// Author:  AMD Developer Tools Team
// Creation Date:        20/2/2008
// ----------------------------------------------------------------------------------
struct AP_API apCounterID : public osTransferableObject
{
    // The counter type:
    apCounterType _counterType;

    // The local index (the counter's index within the specific counter type reader)
    int _counterLocalIndex;

    // The Context / Queue ID (if applicable):
    apCounterScope _counterScope;

    bool operator==(const apCounterID& other)const ;
    apCounterID& operator=(const apCounterID& other);

public:
    apCounterID();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);
};


// ----------------------------------------------------------------------------------
// Struct Name:         apCounterActivationInfo: public osTransferableObject
// General Description: The struct is used for passing counter activation information
//                      We want the API to pass a vector of counters and activation information,
//                      so that the spy can activate / deactivate them all together
// Author:  AMD Developer Tools Team
// Creation Date:        31/1/2010
// ----------------------------------------------------------------------------------
struct AP_API apCounterActivationInfo: public osTransferableObject
{
    apCounterID _counterId;
    bool _shouldBeActivated;

public:
    apCounterActivationInfo();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);
};

#endif  // __APCOUNTERID
