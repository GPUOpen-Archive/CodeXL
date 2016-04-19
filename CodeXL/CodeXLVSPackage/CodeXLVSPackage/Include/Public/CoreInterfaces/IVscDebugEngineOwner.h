//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file IVscDebugEngineOwner.h
///
//==================================================================================

#ifndef IVscDebugEngineOwner_h__
#define IVscDebugEngineOwner_h__

class IVscDebugEngineOwner
{
public:
    virtual ~IVscDebugEngineOwner() {}
    virtual bool IsAnyOpenedFileModified() const = 0;
    virtual void ClearOpenFiles() const = 0;
    virtual void InformPackageOfNewDebugEngine(void* pNewEngine) const  = 0;
};

#endif // IVscPackageCommandHandlerOwner_h__
