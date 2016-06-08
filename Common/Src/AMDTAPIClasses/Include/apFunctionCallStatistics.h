//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apFunctionCallStatistics.h
///
//==================================================================================

//------------------------------ apFunctionCallStatistics.h ------------------------------

#ifndef __APFUNCTIONCALLSTATISTICS_H
#define __APFUNCTIONCALLSTATISTICS_H

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

// Local:
#include <AMDTAPIClasses/Include/apEnumeratorUsageStatistics.h>
#include <AMDTAPIClasses/Include/apFunctionDeprecation.h>


// ----------------------------------------------------------------------------------
// Struct Name:          apFunctionCallStatistics : public osTransferableObject
// General Description:
//   Holds the calls statistics of an OpenGL / extension / EGL / etc function.
// Author:  AMD Developer Tools Team
// Creation Date:        29/1/2006
// ----------------------------------------------------------------------------------
struct AP_API apFunctionCallStatistics : public osTransferableObject
{
    // The id of the function for which we hold the statistics:
    apMonitoredFunctionId _functionId;

    // The amount of times in which the function was called:
    gtUInt64 _amountOfTimesCalled;
    gtUInt64 _amountOfRedundantTimesCalled;
    gtUInt64 _amountOfDeprecatedTimesCalled[AP_DEPRECATION_STATUS_AMOUNT];
    gtUInt64 _averageAmountPerFrame;
    gtUInt64 _averageRedundantAmountPerFrame;
    gtUInt64 _averageDeprecatedAmountPerFrame[AP_DEPRECATION_STATUS_AMOUNT];

    // Used enumerators:
    gtVector<apEnumeratorUsageStatistics> _usedEnumerators;

public:
    apFunctionCallStatistics();
    virtual ~apFunctionCallStatistics();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

private:
    void init();
};


#endif //__APFUNCTIONCALLSTATISTICS_H

