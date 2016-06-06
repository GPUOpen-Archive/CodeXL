//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apEnumeratorUsageStatistics.h
///
//==================================================================================

//------------------------------ apEnumeratorUsageStatistics.h ------------------------------

#ifndef __APENUMERATORUSAGESTATISTICS_H
#define __APENUMERATORUSAGESTATISTICS_H

// Infra:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osTransferableObject.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>


// ----------------------------------------------------------------------------------
// Struct Name:          apEnumeratorUsageStatistics :  public osTransferableObject
// General Description:
//   Holds the usage statistics of an OpenGL / extension / etc enumerator.
// Author:  AMD Developer Tools Team
// Creation Date:        29/1/2006
// ----------------------------------------------------------------------------------
struct AP_API apEnumeratorUsageStatistics : public osTransferableObject
{
    // The enum for which we hold the statistics:
    GLenum _enum;

    // The object type for which we are holding the data:
    // While this is actually an enumerator, we differentiate a few kinds
    // (e.g. Primitive type enumerator) in order to display more accurate
    // data to the user (like the calls statistics view)
    osTransferableObjectType _enumType;

    // The amount of times in which the enum was used:
    gtUInt64 _amountOfTimesUsed;

    // The amount of redundant times the function was called:
    gtUInt64 _amountOfRedundantTimesUsed;

public:
    apEnumeratorUsageStatistics();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

private:
    void init();
};


#endif //__APENUMERATORUSAGESTATISTICS_H

