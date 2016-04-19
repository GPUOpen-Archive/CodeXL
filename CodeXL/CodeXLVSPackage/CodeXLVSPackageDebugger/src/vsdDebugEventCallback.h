//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vsdDebugEventCallback.h
///
//==================================================================================

//------------------------------ vsdDebugEventCallback.h ------------------------------

#ifndef __VSDDEBUGEVENTCALLBACK_H
#define __VSDDEBUGEVENTCALLBACK_H

// Visual Studio:
#include <msdbg.h>

// Local:
#include <CodeXLVSPackageDebugger/Include/vsdPackageDLLBuild.h>
#include <CodeXLVSPackageDebugger/Include/vsdUnknown.h>


// ----------------------------------------------------------------------------------
// Class Name:          vsdCDebugEventCallback : public IDebugEventCallback2, vsdCUnknown
// General Description: Implements IDebugEventCallback2, getting and responding to debug events
// Author:              Uri Shomroni
// Creation Date:       26/12/2011
// ----------------------------------------------------------------------------------
class vsdCDebugEventCallback : public IDebugEventCallback2, vsdCUnknown
{
public:
    vsdCDebugEventCallback();
    virtual ~vsdCDebugEventCallback();

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    ////////////////////////////////////////////////////////////
    // IDebugEventCallback2 methods
    STDMETHOD(Event)(IDebugEngine2* pEngine, IDebugProcess2* pProcess, IDebugProgram2* pProgram, IDebugThread2* pThread, IDebugEvent2* pEvent, REFIID riidEvent, DWORD dwAttrib);
};

#endif //__VSDDEBUGEVENTCALLBACK_H

