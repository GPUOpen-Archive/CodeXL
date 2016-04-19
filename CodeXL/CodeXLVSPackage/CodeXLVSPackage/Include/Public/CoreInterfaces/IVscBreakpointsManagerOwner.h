//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file IVscBreakpointsManagerOwner.h
///
//==================================================================================

#ifndef IVscBreakpointsManagerOwner_h__
#define IVscBreakpointsManagerOwner_h__

class IVscBreakpointsManagerOwner
{
public:
    virtual ~IVscBreakpointsManagerOwner() {}
    virtual bool IsSrcLocationBreakpointEnabled(const wchar_t* filePath, int lineNumber, bool& isEnabled) const = 0;
    virtual bool IsFuncBreakpointEnabled(const wchar_t* functionName, bool& isEnabled) const = 0;
    virtual bool DisableFuncBreakpoint(const wchar_t* functionName) const = 0;
    virtual bool AddBreakpointInSourceLocation(const wchar_t* filePath, int lineNumber, bool enabled) const = 0;
    virtual bool RemoveBreakpointsInSourceLocation(const wchar_t* filePath, int lineNumber) const = 0;
};

#endif // IVscBreakpointsManagerOwner_h__
