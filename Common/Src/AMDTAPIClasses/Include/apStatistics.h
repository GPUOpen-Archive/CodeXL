//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apStatistics.h
///
//==================================================================================

//------------------------------ apStatistics.h ------------------------------

#ifndef __APSTATISTICS_H
#define __APSTATISTICS_H

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>

// Local:
// Infra:
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionId.h>
#include <AMDTOSWrappers/Include/osTransferableObject.h>
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTAPIClasses/Include/apFunctionType.h>
#include <AMDTAPIClasses/Include/apFunctionCallStatistics.h>
// ----------------------------------------------------------------------------------
// Struct Name:          apStatistics : public osTransferableObject
// General Description:  Contain the statistic data in a debugged process
// Author:  AMD Developer Tools Team
// Creation Date:        15/7/2008
// ----------------------------------------------------------------------------------
class AP_API apStatistics : public osTransferableObject
{
public:
    apStatistics();
    virtual ~apStatistics();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    void clearFunctionCallsStatistics();
    int amountOfFunctionCallsStatistics() const;
    bool getFunctionCallStatistics(int functionCallIndex, const apFunctionCallStatistics*& pFunctionStatistics) const;
    void addFunctionCallStatistics(apFunctionCallStatistics* pFunctionCallStatistics);

    void setAmountOfFullFrames(gtUInt64 amountOfFullFrames) {_amountOfFullFrames = amountOfFullFrames;};
    void setAmountOfFunctionCallsInFullFrames(gtUInt64 amountOfFunctionCalls) {_amountOfFunctionCallsInFullFrames = amountOfFunctionCalls;};
    gtUInt64 amountOfFullFrames() const;
    gtUInt64 amountOfFunctionCallsInFullFrames() const;

private:
    // Vector that contain the function calls statistics:
    gtPtrVector<apFunctionCallStatistics*> _functionStatisticsCallsVec;

    // Number of full frames since the last clear:
    gtUInt64 _amountOfFullFrames;

    // Number of function calls in the full frames since the last clear;
    gtUInt64 _amountOfFunctionCallsInFullFrames;
};


#endif //__APSTATISTICS_H

