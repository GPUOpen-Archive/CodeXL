//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apKernelFunctionNameBreakpoint.h
///
//==================================================================================

//------------------------------ apKernelFunctionNameBreakpoint.h ------------------------------

#ifndef __APKERNELFUNCTIONNAMEBREAKPOINT_H
#define __APKERNELFUNCTIONNAMEBREAKPOINT_H

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTAPIClasses/Include/apBreakPoint.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apKernelFunctionNameBreakpoint
// General Description:
//   Represents a breakpoint on a kernel function. When a kernel with this function name
//   is enqueued, the debugged process execution will be suspended
// Author:  AMD Developer Tools Team
// Creation Date:        23/2/2011
// ----------------------------------------------------------------------------------
class AP_API apKernelFunctionNameBreakpoint : public apBreakPoint
{
public:
    apKernelFunctionNameBreakpoint(const gtString& kernelFunctionName)
        : _kernelFunctionName(kernelFunctionName) {};
    apKernelFunctionNameBreakpoint() {};
    virtual ~apKernelFunctionNameBreakpoint() {};

    // Overrides apBreakPoint:
    virtual bool compareToOther(const apBreakPoint& otherBreakpoint) const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    void setKernelFunctionName(const gtString& kernelFunctionName) {_kernelFunctionName = kernelFunctionName;};
    const gtString& kernelFunctionName() const {return _kernelFunctionName;};

private:
    // The monitored function id:
    gtString _kernelFunctionName;
};

#endif //__APKERNELFUNCTIONNAMEBREAKPOINT_H

