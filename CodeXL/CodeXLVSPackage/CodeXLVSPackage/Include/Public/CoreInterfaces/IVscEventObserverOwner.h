//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file IVscEventObserverOwner.h
///
//==================================================================================

#ifndef IVscEventObserverOwner_h__
#define IVscEventObserverOwner_h__
#include <wchar.h>

class IVscEventObserverOwner
{
public:
    virtual ~IVscEventObserverOwner() {}

    virtual void CloseDisassemblyWindow() const = 0;

    virtual void ForceVariablesReevaluation() const = 0;

    virtual void ClearMessagePane() const = 0;

    virtual void OutputMessage(const wchar_t* pMsgBuffer, bool outputOnlyToLog) = 0;

    virtual void DeleteWCharStringBuffer(wchar_t*& pBuffer) = 0;
};

#endif // IVscEventObserverOwner_h__
