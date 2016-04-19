//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscDebugEvents.cpp
///
//==================================================================================

//------------------------------ vspDebugEvents.cpp ------------------------------

#include "stdafx.h"

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/Events/apExceptionEvent.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <CodeXLVSPackage/Include/vspStringConstants.h>
#include <src/vscDebugBreakpoint.h>
#include <src/vscDebugEngine.h>
#include <src/vscDebugEvents.h>
#include <src/vscDebugModule.h>
#include <src/vspExpressionEvaluator.h>

////////////////////////////////////////////////////////////
// vspCDebugEvent
////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// Name:        vspCDebugEvent::vspCDebugEvent
// Description: Constructor
// Author:      Uri Shomroni
// Date:        8/9/2010
// ---------------------------------------------------------------------------
vspCDebugEvent::vspCDebugEvent(REFIID eventInterfaceID)
    : _eventInterfaceID(eventInterfaceID)
{
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugEvent::~vspCDebugEvent
// Description: Destructor
// Author:      Uri Shomroni
// Date:        8/9/2010
// ---------------------------------------------------------------------------
vspCDebugEvent::~vspCDebugEvent()
{
}


// ---------------------------------------------------------------------------
// Name:        vspCDebugEvent::send
// Description: Sends the event to piEventCallback.
// Author:      Uri Shomroni
// Date:        8/9/2010
// ---------------------------------------------------------------------------
bool vspCDebugEvent::send(IDebugEventCallback2* piEventCallback, IDebugEngine2* piEngine, IDebugProgram2* piProgram, IDebugThread2* piThread)
{
    bool retVal = false;

    // Make sure we don't get deleted while this is happening:
    AddRef();

    GT_IF_WITH_ASSERT(piEventCallback != NULL)
    {
        HRESULT hr = S_OK;

#if 0
        IDebugProcess2* piProcess = NULL;

        if (piProgram != NULL)
        {
            hr = piProgram->GetProcess(&piProcess);
        }

#endif

        GT_IF_WITH_ASSERT(hr == S_OK)
        {
            DWORD eventAttribs = eventAttributes();
            hr = piEventCallback->Event(piEngine, NULL /*piProcess*/, piProgram, piThread, this, _eventInterfaceID, eventAttribs);

            retVal = (hr == S_OK);

            if (retVal && ((eventAttribs == EVENT_SYNCHRONOUS) || (eventAttribs == EVENT_SYNC_STOP)))
            {
                // This is a synchronous event, wait for the returned message:
                MSG msg;

                while (GetMessage(&msg, NULL, 0, 0))
                {
                    //TODO: ADD CONTINUE EVENT TEST
                    DispatchMessage(&msg);
                }
            }
        }
    }

    // Restore the event ref count:
    Release();

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugEvent::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        8/9/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugEvent::AddRef(void)
{
    return vspCUnknown::addRef();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugEvent::AddRef
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        8/9/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugEvent::Release(void)
{
    return vspCUnknown::release();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugEvent::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        8/9/2010
// ---------------------------------------------------------------------------
HRESULT vspCDebugEvent::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    HRESULT retVal = S_OK;

    if (ppvObj == NULL)
    {
        retVal = E_POINTER;
    }
    else if (riid == IID_IUnknown)
    {
        *ppvObj = (IUnknown*)this;
        AddRef();
    }
    else if (riid == IID_IDebugEvent2)
    {
        *ppvObj = (IDebugEvent2*)this;
        AddRef();
    }
    else // riid != IID_IUnknown, IID_IDebugEvent2
    {
        retVal = E_NOINTERFACE;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugEvent::GetAttributes
// Description: Returns the event attributes into pdwAttrib.
// Author:      Uri Shomroni
// Date:        8/9/2010
// ---------------------------------------------------------------------------
HRESULT vspCDebugEvent::GetAttributes(DWORD* pdwAttrib)
{
    HRESULT retVal = S_OK;

    if (pdwAttrib != NULL)
    {
        *pdwAttrib = eventAttributes();
    }

    return retVal;
}

////////////////////////////////////////////////////////////
// vspCDebugEngineCreatedEvent
////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// Name:        vspCDebugEngineCreateEvent::vspCDebugEngineCreateEvent
// Description: Constructor
// Author:      Uri Shomroni
// Date:        8/9/2010
// ---------------------------------------------------------------------------
vspCDebugEngineCreateEvent::vspCDebugEngineCreateEvent(IDebugEngine2* piEngine)
    : vspCDebugEvent(IID_IDebugEngineCreateEvent2), _piEngine(piEngine)
{
    if (_piEngine != NULL)
    {
        _piEngine->AddRef();
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugEngineCreateEvent::~vspCDebugEngineCreateEvent
// Description: Destructor
// Author:      Uri Shomroni
// Date:        8/9/2010
// ---------------------------------------------------------------------------
vspCDebugEngineCreateEvent::~vspCDebugEngineCreateEvent()
{
    if (_piEngine != NULL)
    {
        _piEngine->Release();
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugEngineCreateEvent::eventAttributes
// Description: Returns the event attributes
// Author:      Uri Shomroni
// Date:        8/9/2010
// ---------------------------------------------------------------------------
DWORD vspCDebugEngineCreateEvent::eventAttributes() const
{
    // Engine creation events are asynchronous:
    return EVENT_ASYNCHRONOUS;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugEngineCreateEvent::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        12/9/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugEngineCreateEvent::AddRef(void)
{
    // Use the base class's implementation:
    return vspCDebugEvent::AddRef();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugEngineCreateEvent::AddRef
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        12/9/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugEngineCreateEvent::Release(void)
{
    // Use the base class's implementation:
    return vspCDebugEvent::Release();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugEngineCreateEvent::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        8/9/2010
// ---------------------------------------------------------------------------
HRESULT vspCDebugEngineCreateEvent::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    HRESULT retVal = S_OK;

    if ((ppvObj != NULL) && (riid == IID_IDebugEngineCreateEvent2))
    {
        *ppvObj = (IDebugEngineCreateEvent2*)this;
        AddRef();
    }
    else
    {
        retVal = vspCDebugEvent::QueryInterface(riid, ppvObj);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugEngineCreateEvent::GetEngine
// Description: Returns the debug engine whose creation this event represents into ppEngine.
// Author:      Uri Shomroni
// Date:        8/9/2010
// ---------------------------------------------------------------------------
HRESULT vspCDebugEngineCreateEvent::GetEngine(IDebugEngine2** ppEngine)
{
    HRESULT retVal = S_OK;

    if (ppEngine != NULL)
    {
        // Return the engine:
        *ppEngine = _piEngine;
        GT_IF_WITH_ASSERT(_piEngine != NULL)
        {
            _piEngine->AddRef();
        }
        else
        {
            retVal = E_FAIL;
        }
    }
    else
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}

////////////////////////////////////////////////////////////
// vspCDebugProgramCreatedEvent
////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// Name:        vspCDebugProgramCreateEvent::vspCDebugProgramCreateEvent
// Description: Constructor
// Author:      Uri Shomroni
// Date:        8/9/2010
// ---------------------------------------------------------------------------
vspCDebugProgramCreateEvent::vspCDebugProgramCreateEvent()
    : vspCDebugEvent(IID_IDebugProgramCreateEvent2)
{
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugProgramCreateEvent::~vspCDebugProgramCreateEvent
// Description: Destructor
// Author:      Uri Shomroni
// Date:        8/9/2010
// ---------------------------------------------------------------------------
vspCDebugProgramCreateEvent::~vspCDebugProgramCreateEvent()
{
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugProgramCreateEvent::eventAttributes
// Description: Returns the event attributes
// Author:      Uri Shomroni
// Date:        8/9/2010
// ---------------------------------------------------------------------------
DWORD vspCDebugProgramCreateEvent::eventAttributes() const
{
    // Program creation events are asynchronous:
    return EVENT_ASYNCHRONOUS;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugProgramCreateEvent::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        12/9/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugProgramCreateEvent::AddRef(void)
{
    // Use the base class's implementation:
    return vspCDebugEvent::AddRef();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugProgramCreateEvent::AddRef
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        12/9/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugProgramCreateEvent::Release(void)
{
    // Use the base class's implementation:
    return vspCDebugEvent::Release();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugProgramCreateEvent::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        8/9/2010
// ---------------------------------------------------------------------------
HRESULT vspCDebugProgramCreateEvent::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    HRESULT retVal = S_OK;

    if ((ppvObj != NULL) && (riid == IID_IDebugProgramCreateEvent2))
    {
        *ppvObj = (IDebugProgramCreateEvent2*)this;
        AddRef();
    }
    else
    {
        retVal = vspCDebugEvent::QueryInterface(riid, ppvObj);
    }

    return retVal;
}

////////////////////////////////////////////////////////////
// vspCDebugProgramDestroyEvent
////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// Name:        vspCDebugProgramDestroyEvent::vspCDebugProgramDestroyEvent
// Description: Constructor
// Author:      Uri Shomroni
// Date:        8/9/2010
// ---------------------------------------------------------------------------
vspCDebugProgramDestroyEvent::vspCDebugProgramDestroyEvent(int exitCode)
    : vspCDebugEvent(IID_IDebugProgramDestroyEvent2), _exitCode(exitCode)
{
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugProgramDestroyEvent::~vspCDebugProgramDestroyEvent
// Description: Destructor
// Author:      Uri Shomroni
// Date:        8/9/2010
// ---------------------------------------------------------------------------
vspCDebugProgramDestroyEvent::~vspCDebugProgramDestroyEvent()
{
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugProgramDestroyEvent::eventAttributes
// Description: Returns the event attributes
// Author:      Uri Shomroni
// Date:        8/9/2010
// ---------------------------------------------------------------------------
DWORD vspCDebugProgramDestroyEvent::eventAttributes() const
{
    // Program destruction events are asynchronous:
    return EVENT_ASYNCHRONOUS;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugProgramDestroyEvent::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        12/9/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugProgramDestroyEvent::AddRef(void)
{
    // Use the base class's implementation:
    return vspCDebugEvent::AddRef();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugProgramDestroyEvent::AddRef
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        12/9/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugProgramDestroyEvent::Release(void)
{
    // Use the base class's implementation:
    return vspCDebugEvent::Release();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugProgramDestroyEvent::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        8/9/2010
// ---------------------------------------------------------------------------
HRESULT vspCDebugProgramDestroyEvent::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    HRESULT retVal = S_OK;

    if ((ppvObj != NULL) && (riid == IID_IDebugProgramDestroyEvent2))
    {
        *ppvObj = (IDebugProgramDestroyEvent2*)this;
        AddRef();
    }
    else
    {
        retVal = vspCDebugEvent::QueryInterface(riid, ppvObj);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugProgramDestroyEvent::GetExitCode
// Description: Returns the program's exit code into pdwExit
// Author:      Uri Shomroni
// Date:        20/9/2010
// ---------------------------------------------------------------------------
HRESULT vspCDebugProgramDestroyEvent::GetExitCode(DWORD* pdwExit)
{
    HRESULT retVal = S_OK;

    if (pdwExit != NULL)
    {
        // Return the exit code:
        *pdwExit = (DWORD)_exitCode;
    }
    else
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}

////////////////////////////////////////////////////////////
// vspCDebugThreadCreateEvent
////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// Name:        vspCDebugThreadCreateEvent::vspCDebugThreadCreateEvent
// Description: Constructor
// Author:      Uri Shomroni
// Date:        12/9/2010
// ---------------------------------------------------------------------------
vspCDebugThreadCreateEvent::vspCDebugThreadCreateEvent()
    : vspCDebugEvent(IID_IDebugThreadCreateEvent2)
{
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugThreadCreateEvent::~vspCDebugThreadCreateEvent
// Description: Destructor
// Author:      Uri Shomroni
// Date:        12/9/2010
// ---------------------------------------------------------------------------
vspCDebugThreadCreateEvent::~vspCDebugThreadCreateEvent()
{
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugThreadCreateEvent::eventAttributes
// Description: Returns the event attributes
// Author:      Uri Shomroni
// Date:        12/9/2010
// ---------------------------------------------------------------------------
DWORD vspCDebugThreadCreateEvent::eventAttributes() const
{
    // Thread creation events are asynchronous:
    return EVENT_ASYNCHRONOUS;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugThreadCreateEvent::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        12/9/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugThreadCreateEvent::AddRef(void)
{
    // Use the base class's implementation:
    return vspCDebugEvent::AddRef();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugThreadCreateEvent::AddRef
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        12/9/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugThreadCreateEvent::Release(void)
{
    // Use the base class's implementation:
    return vspCDebugEvent::Release();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugThreadCreateEvent::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        12/9/2010
// ---------------------------------------------------------------------------
HRESULT vspCDebugThreadCreateEvent::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    HRESULT retVal = S_OK;

    if ((ppvObj != NULL) && (riid == IID_IDebugThreadCreateEvent2))
    {
        *ppvObj = (IDebugThreadCreateEvent2*)this;
        AddRef();
    }
    else
    {
        retVal = vspCDebugEvent::QueryInterface(riid, ppvObj);
    }

    return retVal;
}

////////////////////////////////////////////////////////////
// vspCDebugThreadDestroydEvent
////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// Name:        vspCDebugThreadDestroyEvent::vspCDebugThreadDestroyEvent
// Description: Constructor
// Author:      Uri Shomroni
// Date:        21/9/2010
// ---------------------------------------------------------------------------
vspCDebugThreadDestroyEvent::vspCDebugThreadDestroyEvent(int exitCode)
    : vspCDebugEvent(IID_IDebugThreadDestroyEvent2), _exitCode(exitCode)
{
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugThreadDestroyEvent::~vspCDebugThreadDestroyEvent
// Description: Destructor
// Author:      Uri Shomroni
// Date:        21/9/2010
// ---------------------------------------------------------------------------
vspCDebugThreadDestroyEvent::~vspCDebugThreadDestroyEvent()
{
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugThreadDestroyEvent::eventAttributes
// Description: Returns the event attributes
// Author:      Uri Shomroni
// Date:        21/9/2010
// ---------------------------------------------------------------------------
DWORD vspCDebugThreadDestroyEvent::eventAttributes() const
{
    // Thread destruction events are asynchronous:
    return EVENT_ASYNCHRONOUS;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugThreadDestroyEvent::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        21/9/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugThreadDestroyEvent::AddRef(void)
{
    // Use the base class's implementation:
    return vspCDebugEvent::AddRef();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugThreadDestroyEvent::AddRef
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        21/9/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugThreadDestroyEvent::Release(void)
{
    // Use the base class's implementation:
    return vspCDebugEvent::Release();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugThreadDestroyEvent::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        21/9/2010
// ---------------------------------------------------------------------------
HRESULT vspCDebugThreadDestroyEvent::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    HRESULT retVal = S_OK;

    if ((ppvObj != NULL) && (riid == IID_IDebugThreadDestroyEvent2))
    {
        *ppvObj = (IDebugThreadDestroyEvent2*)this;
        AddRef();
    }
    else
    {
        retVal = vspCDebugEvent::QueryInterface(riid, ppvObj);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugThreadDestroyEvent::GetExitCode
// Description: Returns the thread's exit code into pdwExit
// Author:      Uri Shomroni
// Date:        20/9/2010
// ---------------------------------------------------------------------------
HRESULT vspCDebugThreadDestroyEvent::GetExitCode(DWORD* pdwExit)
{
    HRESULT retVal = S_OK;

    if (pdwExit != NULL)
    {
        // Return the exit code:
        *pdwExit = (DWORD)_exitCode;
    }
    else
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}

////////////////////////////////////////////////////////////
// vspCDebugModuleLoadEvent
////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// Name:        vspCDebugModuleLoadEvent::vspCDebugModuleLoadEvent
// Description: Constructor
// Arguments: pModule - the module interface
//            debugMessage - the load message
//            wasLoaded - true = this is a module load event.
//                      - false = this is a module unload event.
// Author:      Uri Shomroni
// Date:        26/9/2010
// ---------------------------------------------------------------------------
vspCDebugModuleLoadEvent::vspCDebugModuleLoadEvent(vspCDebugModule* pModule, bool wasLoaded)
    : vspCDebugEvent(IID_IDebugModuleLoadEvent2), _pModule(pModule), _wasLoaded(wasLoaded)
{
    GT_IF_WITH_ASSERT(_pModule != NULL)
    {
        // Retain the module:
        _pModule->AddRef();
    }

    // Construct the debug message:
    _debugMessage = pModule->moduleFilePath().asString();
    _debugMessage.prepend(_wasLoaded ? VSP_STR_ModuleLoadedDebugMessage : VSP_STR_ModuleUnloadedDebugMessage).append('\'');
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugModuleLoadEvent::~vspCDebugModuleLoadEvent
// Description: Destructor
// Author:      Uri Shomroni
// Date:        26/9/2010
// ---------------------------------------------------------------------------
vspCDebugModuleLoadEvent::~vspCDebugModuleLoadEvent()
{
    // Release the module:
    if (_pModule != NULL)
    {
        _pModule->Release();
        _pModule = NULL;
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugModuleLoadEvent::eventAttributes
// Description: Returns the event attributes
// Author:      Uri Shomroni
// Date:        26/9/2010
// ---------------------------------------------------------------------------
DWORD vspCDebugModuleLoadEvent::eventAttributes() const
{
    // Module load / unload events are asynchronous:
    return EVENT_ASYNCHRONOUS;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugModuleLoadEvent::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        26/9/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugModuleLoadEvent::AddRef(void)
{
    // Use the base class's implementation:
    return vspCDebugEvent::AddRef();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugModuleLoadEvent::AddRef
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        26/9/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugModuleLoadEvent::Release(void)
{
    // Use the base class's implementation:
    return vspCDebugEvent::Release();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugModuleLoadEvent::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        26/9/2010
// ---------------------------------------------------------------------------
HRESULT vspCDebugModuleLoadEvent::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    HRESULT retVal = S_OK;

    if ((ppvObj != NULL) && (riid == IID_IDebugModuleLoadEvent2))
    {
        *ppvObj = (IDebugModuleLoadEvent2*)this;
        AddRef();
    }
    else
    {
        retVal = vspCDebugEvent::QueryInterface(riid, ppvObj);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        vspCDebugModuleLoadEvent::GetModule
// Description: Gets the module interface, the load/unload message, and whether
//              this event represents its loading or unloading
// Author:      Uri Shomroni
// Date:        26/9/2010
// ---------------------------------------------------------------------------
HRESULT vspCDebugModuleLoadEvent::GetModule(IDebugModule2** pModule, BSTR* pbstrDebugMessage, BOOL* pbLoad)
{
    HRESULT retVal = S_OK;

    if (pModule != NULL)
    {
        *pModule = _pModule;
        GT_IF_WITH_ASSERT(_pModule != NULL)
        {
            _pModule->AddRef();
        }
        else
        {
            retVal = E_FAIL;
        }
    }

    if (pbstrDebugMessage != NULL)
    {
        *pbstrDebugMessage = (_debugMessage.isEmpty() ? NULL : SysAllocString(_debugMessage.asCharArray()));
    }

    if (pbLoad != NULL)
    {
        *pbLoad = (_wasLoaded ? TRUE : FALSE);
    }

    return retVal;
}

////////////////////////////////////////////////////////////
// vspCDebugLoadCompleteEvent
////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// Name:        vspCDebugLoadCompleteEvent::vspCDebugLoadCompleteEvent
// Description: Constructor
// Author:      Uri Shomroni
// Date:        12/9/2010
// ---------------------------------------------------------------------------
vspCDebugLoadCompleteEvent::vspCDebugLoadCompleteEvent()
    : vspCDebugEvent(IID_IDebugLoadCompleteEvent2)
{
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugLoadCompleteEvent::~vspCDebugLoadCompleteEvent
// Description: Destructor
// Author:      Uri Shomroni
// Date:        12/9/2010
// ---------------------------------------------------------------------------
vspCDebugLoadCompleteEvent::~vspCDebugLoadCompleteEvent()
{
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugLoadCompleteEvent::eventAttributes
// Description: Returns the event attributes
// Author:      Uri Shomroni
// Date:        12/9/2010
// ---------------------------------------------------------------------------
DWORD vspCDebugLoadCompleteEvent::eventAttributes() const
{
    // Load completion events are asynchronous and stopping:
    return EVENT_ASYNC_STOP;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugLoadCompleteEvent::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        12/9/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugLoadCompleteEvent::AddRef(void)
{
    // Use the base class's implementation:
    return vspCDebugEvent::AddRef();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugLoadCompleteEvent::AddRef
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        12/9/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugLoadCompleteEvent::Release(void)
{
    // Use the base class's implementation:
    return vspCDebugEvent::Release();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugLoadCompleteEvent::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        12/9/2010
// ---------------------------------------------------------------------------
HRESULT vspCDebugLoadCompleteEvent::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    HRESULT retVal = S_OK;

    if ((ppvObj != NULL) && (riid == IID_IDebugLoadCompleteEvent2))
    {
        *ppvObj = (IDebugLoadCompleteEvent2*)this;
        AddRef();
    }
    else
    {
        retVal = vspCDebugEvent::QueryInterface(riid, ppvObj);
    }

    return retVal;
}

////////////////////////////////////////////////////////////
// vspCDebugBreakEvent
////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakEvent::vspCDebugBreakEvent
// Description: Constructor
// Author:      Uri Shomroni
// Date:        13/9/2010
// ---------------------------------------------------------------------------
vspCDebugBreakEvent::vspCDebugBreakEvent()
    : vspCDebugEvent(IID_IDebugBreakEvent2)
{
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakEvent::~vspCDebugBreakEvent
// Description: Destructor
// Author:      Uri Shomroni
// Date:        13/9/2010
// ---------------------------------------------------------------------------
vspCDebugBreakEvent::~vspCDebugBreakEvent()
{
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakEvent::eventAttributes
// Description: Returns the event attributes
// Author:      Uri Shomroni
// Date:        13/9/2010
// ---------------------------------------------------------------------------
DWORD vspCDebugBreakEvent::eventAttributes() const
{
    // Debug break events are asynchronous and stopping:
    return EVENT_ASYNC_STOP;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakEvent::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        13/9/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugBreakEvent::AddRef(void)
{
    // Use the base class's implementation:
    return vspCDebugEvent::AddRef();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakEvent::AddRef
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        13/9/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugBreakEvent::Release(void)
{
    // Use the base class's implementation:
    return vspCDebugEvent::Release();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakEvent::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        13/9/2010
// ---------------------------------------------------------------------------
HRESULT vspCDebugBreakEvent::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    HRESULT retVal = S_OK;

    if ((ppvObj != NULL) && (riid == IID_IDebugBreakEvent2))
    {
        *ppvObj = (IDebugBreakEvent2*)this;
        AddRef();
    }
    else
    {
        retVal = vspCDebugEvent::QueryInterface(riid, ppvObj);
    }

    return retVal;
}

////////////////////////////////////////////////////////////
// vspCDebugStepCompleteEvent
////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// Name:        vspCDebugStepCompleteEvent::vspCDebugStepCompleteEvent
// Description: Constructor
// Author:      Uri Shomroni
// Date:        21/9/2010
// ---------------------------------------------------------------------------
vspCDebugStepCompleteEvent::vspCDebugStepCompleteEvent()
    : vspCDebugEvent(IID_IDebugStepCompleteEvent2)
{
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugStepCompleteEvent::~vspCDebugStepCompleteEvent
// Description: Destructor
// Author:      Uri Shomroni
// Date:        21/9/2010
// ---------------------------------------------------------------------------
vspCDebugStepCompleteEvent::~vspCDebugStepCompleteEvent()
{
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugStepCompleteEvent::eventAttributes
// Description: Returns the event attributes
// Author:      Uri Shomroni
// Date:        21/9/2010
// ---------------------------------------------------------------------------
DWORD vspCDebugStepCompleteEvent::eventAttributes() const
{
    // Step complete events are asynchronous and stopping:
    return EVENT_ASYNC_STOP;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugStepCompleteEvent::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        21/9/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugStepCompleteEvent::AddRef(void)
{
    // Use the base class's implementation:
    return vspCDebugEvent::AddRef();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugStepCompleteEvent::AddRef
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        21/9/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugStepCompleteEvent::Release(void)
{
    // Use the base class's implementation:
    return vspCDebugEvent::Release();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugStepCompleteEvent::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        21/9/2010
// ---------------------------------------------------------------------------
HRESULT vspCDebugStepCompleteEvent::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    HRESULT retVal = S_OK;

    if ((ppvObj != NULL) && (riid == IID_IDebugStepCompleteEvent2))
    {
        *ppvObj = (IDebugStepCompleteEvent2*)this;
        AddRef();
    }
    else
    {
        retVal = vspCDebugEvent::QueryInterface(riid, ppvObj);
    }

    return retVal;
}

////////////////////////////////////////////////////////////
// vspCDebugBreakpointBoundEvent
////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpointBoundEvent::vspCDebugBreakpointBoundEvent
// Description: Constructor
// Author:      Uri Shomroni
// Date:        4/10/2010
// ---------------------------------------------------------------------------
vspCDebugBreakpointBoundEvent::vspCDebugBreakpointBoundEvent(vspCDebugBreakpoint* pBreakpoint)
    : vspCDebugEvent(IID_IDebugBreakpointBoundEvent2), _pBreakpoint(pBreakpoint)
{
    // Make sure this is a bound breakpoint and not an error / pending breakpoint:
    GT_IF_WITH_ASSERT(_pBreakpoint != NULL)
    {
        GT_IF_WITH_ASSERT(_pBreakpoint->breakpointStatus() == vspCDebugBreakpoint::VSP_BREAKPOINT_BOUND)
        {
            // Retain the breakpoint:
            _pBreakpoint->AddRef();
        }
        else // _pBreakpoint->breakpointStatus() != vspCDebugBreakpoint::VSP_BREAKPOINT_BOUND
        {
            // Do not keep this breakpoint:
            _pBreakpoint = NULL;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpointBoundEvent::~vspCDebugBreakpointBoundEvent
// Description: Destructor
// Author:      Uri Shomroni
// Date:        4/10/2010
// ---------------------------------------------------------------------------
vspCDebugBreakpointBoundEvent::~vspCDebugBreakpointBoundEvent()
{
    if (_pBreakpoint != NULL)
    {
        _pBreakpoint->Release();
        _pBreakpoint = NULL;
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpointBoundEvent::eventAttributes
// Description: Returns the event attributes
// Author:      Uri Shomroni
// Date:        4/10/2010
// ---------------------------------------------------------------------------
DWORD vspCDebugBreakpointBoundEvent::eventAttributes() const
{
    // Breakpoint binding events are asynchronous:
    return EVENT_ASYNCHRONOUS;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpointBoundEvent::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        4/10/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugBreakpointBoundEvent::AddRef(void)
{
    // Use the base class's implementation:
    return vspCDebugEvent::AddRef();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpointBoundEvent::AddRef
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        4/10/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugBreakpointBoundEvent::Release(void)
{
    // Use the base class's implementation:
    return vspCDebugEvent::Release();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpointBoundEvent::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        4/10/2010
// ---------------------------------------------------------------------------
HRESULT vspCDebugBreakpointBoundEvent::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    HRESULT retVal = S_OK;

    if ((ppvObj != NULL) && (riid == IID_IDebugBreakpointBoundEvent2))
    {
        *ppvObj = (IDebugBreakpointBoundEvent2*)this;
        AddRef();
    }
    else
    {
        retVal = vspCDebugEvent::QueryInterface(riid, ppvObj);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpointBoundEvent::GetPendingBreakpoint
// Description: Returns the pending breakpoint interface to the caller
// Author:      Uri Shomroni
// Date:        4/10/2010
// ---------------------------------------------------------------------------
HRESULT vspCDebugBreakpointBoundEvent::GetPendingBreakpoint(IDebugPendingBreakpoint2** ppPendingBP)
{
    HRESULT retVal = S_OK;

    if (ppPendingBP != NULL)
    {
        // Return the breakpoint (as a pending breakpoint):
        *ppPendingBP = _pBreakpoint;

        if (_pBreakpoint != NULL)
        {
            _pBreakpoint->AddRef();
        }
        else
        {
            retVal = E_FAIL;
        }
    }
    else // ppPendingBP == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpointBoundEvent::EnumBoundBreakpoints
// Description: Returns the bound breakpoint interface(s) for the pending breakpoint
// Author:      Uri Shomroni
// Date:        4/10/2010
// ---------------------------------------------------------------------------
HRESULT vspCDebugBreakpointBoundEvent::EnumBoundBreakpoints(IEnumDebugBoundBreakpoints2** ppEnum)
{
    HRESULT retVal = S_OK;

    if (_pBreakpoint != NULL)
    {
        // Have the breakpoint object perform the needed checks:
        retVal = _pBreakpoint->EnumBoundBreakpoints(ppEnum);
    }
    else // _pBreakpoint == NULL
    {
        if (ppEnum != NULL)
        {
            // We can't give any details:
            retVal = E_FAIL;
        }
        else // ppEnum == NULL
        {
            // Invalid pointer:
            retVal = E_POINTER;
        }
    }

    return retVal;
}

////////////////////////////////////////////////////////////
// vspCDebugBreakpointUnboundEvent
////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpointUnboundEvent::vspCDebugBreakpointUnboundEvent
// Description: Constructor
// Author:      Uri Shomroni
// Date:        15/5/2011
// ---------------------------------------------------------------------------
vspCDebugBreakpointUnboundEvent::vspCDebugBreakpointUnboundEvent(vspCDebugBreakpoint* pBreakpoint, BP_UNBOUND_REASON reason)
    : vspCDebugEvent(IID_IDebugBreakpointUnboundEvent2), _pBreakpoint(pBreakpoint), _reason(reason)
{
    // Make sure this is a bound breakpoint and not an error / pending breakpoint:
    GT_IF_WITH_ASSERT(_pBreakpoint != NULL)
    {
        // Retain the breakpoint:
        _pBreakpoint->AddRef();
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpointUnboundEvent::~vspCDebugBreakpointUnboundEvent
// Description: Destructor
// Author:      Uri Shomroni
// Date:        15/5/2011
// ---------------------------------------------------------------------------
vspCDebugBreakpointUnboundEvent::~vspCDebugBreakpointUnboundEvent()
{
    if (_pBreakpoint != NULL)
    {
        _pBreakpoint->Release();
        _pBreakpoint = NULL;
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpointUnboundEvent::eventAttributes
// Description: Returns the event attributes
// Author:      Uri Shomroni
// Date:        15/5/2011
// ---------------------------------------------------------------------------
DWORD vspCDebugBreakpointUnboundEvent::eventAttributes() const
{
    // Breakpoint binding events are asynchronous:
    return EVENT_ASYNCHRONOUS;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpointUnboundEvent::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        15/5/2011
// ---------------------------------------------------------------------------
ULONG vspCDebugBreakpointUnboundEvent::AddRef(void)
{
    // Use the base class's implementation:
    return vspCDebugEvent::AddRef();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpointUnboundEvent::AddRef
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        15/5/2011
// ---------------------------------------------------------------------------
ULONG vspCDebugBreakpointUnboundEvent::Release(void)
{
    // Use the base class's implementation:
    return vspCDebugEvent::Release();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpointUnboundEvent::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        15/5/2011
// ---------------------------------------------------------------------------
HRESULT vspCDebugBreakpointUnboundEvent::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    HRESULT retVal = S_OK;

    if ((ppvObj != NULL) && (riid == IID_IDebugBreakpointUnboundEvent2))
    {
        *ppvObj = (IDebugBreakpointUnboundEvent2*)this;
        AddRef();
    }
    else
    {
        retVal = vspCDebugEvent::QueryInterface(riid, ppvObj);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpointUnboundEvent::GetBreakpoint
// Description: Returns the unbound breakpoint
// Author:      Uri Shomroni
// Date:        15/5/2011
// ---------------------------------------------------------------------------
HRESULT vspCDebugBreakpointUnboundEvent::GetBreakpoint(IDebugBoundBreakpoint2** ppBP)
{
    HRESULT retVal = S_OK;

    if (ppBP != NULL)
    {
        // Return the breakpoint (as a bound breakpoint):
        *ppBP = _pBreakpoint;

        if (_pBreakpoint != NULL)
        {
            _pBreakpoint->AddRef();
        }
        else
        {
            retVal = E_FAIL;
        }
    }
    else // ppBP == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpointUnboundEvent::GetReason
// Description: Returns the unbinding reason
// Author:      Uri Shomroni
// Date:        15/5/2011
// ---------------------------------------------------------------------------
HRESULT vspCDebugBreakpointUnboundEvent::GetReason(BP_UNBOUND_REASON* pdwUnboundReason)
{
    HRESULT retVal = S_OK;

    if (pdwUnboundReason != NULL)
    {
        // Return the reason:
        *pdwUnboundReason = _reason;
    }
    else // pdwUnboundReason == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}

////////////////////////////////////////////////////////////
// vspCDebugBreakpointErrorEvent
////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpointErrorEvent::vspCDebugBreakpointErrorEvent
// Description: Constructor
// Author:      Uri Shomroni
// Date:        4/10/2010
// ---------------------------------------------------------------------------
vspCDebugBreakpointErrorEvent::vspCDebugBreakpointErrorEvent(vspCDebugBreakpoint* pBreakpoint)
    : vspCDebugEvent(IID_IDebugBreakpointErrorEvent2), _pBreakpoint(pBreakpoint)
{
    // Make sure this is a error breakpoint and not a bound / pending breakpoint:
    GT_IF_WITH_ASSERT(_pBreakpoint != NULL)
    {
        GT_IF_WITH_ASSERT(_pBreakpoint->breakpointStatus() == vspCDebugBreakpoint::VSP_BREAKPOINT_ERROR)
        {
            // Retain the breakpoint:
            _pBreakpoint->AddRef();
        }
        else // _pBreakpoint->breakpointStatus() != vspCDebugBreakpoint::VSP_BREAKPOINT_ERROR
        {
            // Do not keep this breakpoint:
            _pBreakpoint = NULL;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpointErrorEvent::~vspCDebugBreakpointErrorEvent
// Description: Destructor
// Author:      Uri Shomroni
// Date:        4/10/2010
// ---------------------------------------------------------------------------
vspCDebugBreakpointErrorEvent::~vspCDebugBreakpointErrorEvent()
{
    if (_pBreakpoint != NULL)
    {
        _pBreakpoint->Release();
        _pBreakpoint = NULL;
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpointErrorEvent::eventAttributes
// Description: Returns the event attributes
// Author:      Uri Shomroni
// Date:        4/10/2010
// ---------------------------------------------------------------------------
DWORD vspCDebugBreakpointErrorEvent::eventAttributes() const
{
    // Breakpoint error events are asynchronous:
    return EVENT_ASYNCHRONOUS;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpointErrorEvent::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        4/10/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugBreakpointErrorEvent::AddRef(void)
{
    // Use the base class's implementation:
    return vspCDebugEvent::AddRef();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpointErrorEvent::AddRef
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        4/10/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugBreakpointErrorEvent::Release(void)
{
    // Use the base class's implementation:
    return vspCDebugEvent::Release();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpointErrorEvent::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        4/10/2010
// ---------------------------------------------------------------------------
HRESULT vspCDebugBreakpointErrorEvent::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    HRESULT retVal = S_OK;

    if ((ppvObj != NULL) && (riid == IID_IDebugBreakpointErrorEvent2))
    {
        *ppvObj = (IDebugBreakpointErrorEvent2*)this;
        AddRef();
    }
    else
    {
        retVal = vspCDebugEvent::QueryInterface(riid, ppvObj);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpointErrorEvent::GetErrorBreakpoint
// Description: Returns the error breakpoint interface to the caller
// Author:      Uri Shomroni
// Date:        4/10/2010
// ---------------------------------------------------------------------------
HRESULT vspCDebugBreakpointErrorEvent::GetErrorBreakpoint(IDebugErrorBreakpoint2** ppErrorBP)
{
    HRESULT retVal = S_OK;

    if (ppErrorBP != NULL)
    {
        // Return the breakpoint (as a pending breakpoint):
        *ppErrorBP = _pBreakpoint;

        if (_pBreakpoint != NULL)
        {
            _pBreakpoint->AddRef();
        }
        else
        {
            retVal = E_FAIL;
        }
    }
    else // ppErrorBP == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}

////////////////////////////////////////////////////////////
// vspCDebugBreakpointEvent
////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpointEvent::vspCDebugBreakpointEvent
// Description: Constructor
// Author:      Uri Shomroni
// Date:        5/10/2010
// ---------------------------------------------------------------------------
vspCDebugBreakpointEvent::vspCDebugBreakpointEvent(vspCDebugBreakpoint* pBreakpoint)
    : vspCDebugEvent(IID_IDebugBreakpointEvent2), _pBreakpoint(pBreakpoint)
{
    // Make sure this is a bound breakpoint and not an error / pending breakpoint:
    GT_IF_WITH_ASSERT(_pBreakpoint != NULL)
    {
        GT_IF_WITH_ASSERT(_pBreakpoint->breakpointStatus() == vspCDebugBreakpoint::VSP_BREAKPOINT_BOUND)
        {
            // Retain the breakpoint:
            _pBreakpoint->AddRef();
        }
        else // _pBreakpoint->breakpointStatus() != vspCDebugBreakpoint::VSP_BREAKPOINT_BOUND
        {
            // Do not keep this breakpoint:
            _pBreakpoint = NULL;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpointEvent::~vspCDebugBreakpointEvent
// Description: Destructor
// Author:      Uri Shomroni
// Date:        5/10/2010
// ---------------------------------------------------------------------------
vspCDebugBreakpointEvent::~vspCDebugBreakpointEvent()
{
    if (_pBreakpoint != NULL)
    {
        _pBreakpoint->Release();
        _pBreakpoint = NULL;
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpointEvent::eventAttributes
// Description: Returns the event attributes
// Author:      Uri Shomroni
// Date:        5/10/2010
// ---------------------------------------------------------------------------
DWORD vspCDebugBreakpointEvent::eventAttributes() const
{
    // Breakpoint events are asynchronous and stopping:
    return EVENT_ASYNC_STOP;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpointEvent::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        5/10/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugBreakpointEvent::AddRef(void)
{
    // Use the base class's implementation:
    return vspCDebugEvent::AddRef();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpointEvent::AddRef
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        5/10/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugBreakpointEvent::Release(void)
{
    // Use the base class's implementation:
    return vspCDebugEvent::Release();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpointEvent::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        5/10/2010
// ---------------------------------------------------------------------------
HRESULT vspCDebugBreakpointEvent::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    HRESULT retVal = S_OK;

    if ((ppvObj != NULL) && (riid == IID_IDebugBreakpointEvent2))
    {
        *ppvObj = (IDebugBreakpointEvent2*)this;
        AddRef();
    }
    else
    {
        retVal = vspCDebugEvent::QueryInterface(riid, ppvObj);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpointEvent::EnumBreakpoints
// Description: Returns an enumeration of the breakpoints related to this event.
//              In our case - there is only one breakpoint.
// Author:      Uri Shomroni
// Date:        5/10/2010
// ---------------------------------------------------------------------------
HRESULT vspCDebugBreakpointEvent::EnumBreakpoints(IEnumDebugBoundBreakpoints2** ppEnum)
{
    HRESULT retVal = S_OK;

    if (_pBreakpoint != NULL)
    {
        // Have the breakpoint object perform the needed checks:
        retVal = _pBreakpoint->EnumBoundBreakpoints(ppEnum);
    }
    else // _pBreakpoint == NULL
    {
        if (ppEnum != NULL)
        {
            // We can't give any details:
            retVal = E_FAIL;
        }
        else // ppEnum == NULL
        {
            // Invalid pointer:
            retVal = E_POINTER;
        }
    }

    return retVal;
}

////////////////////////////////////////////////////////////
// vspCDebugExceptionEvent
////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// Name:        vspCDebugExceptionEvent::vspCDebugExceptionEvent
// Description: Constructor
// Author:      Uri Shomroni
// Date:        10/10/2010
// ---------------------------------------------------------------------------
vspCDebugExceptionEvent::vspCDebugExceptionEvent(vspCDebugEngine* pDebugEngine, const apExceptionEvent& exceptionEve)
    : vspCDebugEvent(IID_IDebugExceptionEvent2), _pDebugEngine(pDebugEngine), _exceptionReason(exceptionEve.exceptionReason()), _isSecondChanceException(exceptionEve.isSecondChance())
{
    // Retain the debug Engine:
    GT_IF_WITH_ASSERT(_pDebugEngine != NULL)
    {
        _pDebugEngine->AddRef();
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugExceptionEvent::~vspCDebugExceptionEvent
// Description: Destructor
// Author:      Uri Shomroni
// Date:        10/10/2010
// ---------------------------------------------------------------------------
vspCDebugExceptionEvent::~vspCDebugExceptionEvent()
{
    // Release the debug engine:
    if (_pDebugEngine != NULL)
    {
        _pDebugEngine->Release();
        _pDebugEngine = NULL;
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugExceptionEvent::eventAttributes
// Description: Returns the event attributes
// Author:      Uri Shomroni
// Date:        10/10/2010
// ---------------------------------------------------------------------------
DWORD vspCDebugExceptionEvent::eventAttributes() const
{
    // Exception events are asynchronous and stopping:
    return EVENT_ASYNC_STOP;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugExceptionEvent::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        10/10/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugExceptionEvent::AddRef(void)
{
    // Use the base class's implementation:
    return vspCDebugEvent::AddRef();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugExceptionEvent::AddRef
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        10/10/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugExceptionEvent::Release(void)
{
    // Use the base class's implementation:
    return vspCDebugEvent::Release();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugExceptionEvent::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        10/10/2010
// ---------------------------------------------------------------------------
HRESULT vspCDebugExceptionEvent::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    HRESULT retVal = S_OK;

    if ((ppvObj != NULL) && (riid == IID_IDebugExceptionEvent2))
    {
        *ppvObj = (IDebugExceptionEvent2*)this;
        AddRef();
    }
    else
    {
        retVal = vspCDebugEvent::QueryInterface(riid, ppvObj);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugExceptionEvent::GetException
// Description: Fills the exception information struct supplied. The caller is
//              responsible for releasing contained strings and interfaces.
// Author:      Uri Shomroni
// Date:        10/10/2010
// ---------------------------------------------------------------------------
HRESULT vspCDebugExceptionEvent::GetException(EXCEPTION_INFO* pExceptionInfo)
{
    HRESULT retVal = S_OK;

    if (pExceptionInfo != NULL)
    {
        // Clear the struct:
        ::memset(pExceptionInfo, 0, sizeof(EXCEPTION_INFO));

        // If we have the program info:
        if (_pDebugEngine != NULL)
        {
            // Return the program and its name:
            pExceptionInfo->pProgram = _pDebugEngine;
            _pDebugEngine->AddRef();

            // Get the program name:
            _pDebugEngine->GetName(&(pExceptionInfo->bstrProgramName));
        }

        // Get the exception name:
        gtString exceptionReasonAsString;
        osExceptionReasonToString(_exceptionReason, exceptionReasonAsString);
        BSTR exceptionReasonAsBSTR = NULL;

        if (!exceptionReasonAsString.isEmpty())
        {
            exceptionReasonAsBSTR = SysAllocString(exceptionReasonAsString.asCharArray());
        }

        pExceptionInfo->bstrExceptionName = exceptionReasonAsBSTR;

        // Get the exception code:
        pExceptionInfo->dwCode = osExceptionReasonToExceptionCode(_exceptionReason);

        // Get the exception status:
        pExceptionInfo->dwState = _isSecondChanceException ? EXCEPTION_STOP_SECOND_CHANCE : EXCEPTION_STOP_FIRST_CHANCE;

        // We are debugging with our debug engine:
        pExceptionInfo->guidType = __uuidof(vspDebugEngine);
    }
    else // pExceptionInfo == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugExceptionEvent::GetExceptionDescription
// Description: Gets a string describing the exception
// Author:      Uri Shomroni
// Date:        10/10/2010
// ---------------------------------------------------------------------------
HRESULT vspCDebugExceptionEvent::GetExceptionDescription(BSTR* pbstrDescription)
{
    HRESULT retVal = S_OK;

    if (pbstrDescription != NULL)
    {
        gtString exceptionReasonAsString;
        osExceptionReasonToExplanationString(_exceptionReason, exceptionReasonAsString);

        // If we don't have a return string, return NULL:
        BSTR exceptionReasonAsBSTR = NULL;

        if (!exceptionReasonAsString.isEmpty())
        {
            exceptionReasonAsBSTR = SysAllocString(exceptionReasonAsString.asCharArray());
        }

        *pbstrDescription = exceptionReasonAsBSTR;
    }
    else // pbstrDescription == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugExceptionEvent::CanPassToDebuggee
// Description: Returns whether this exception can be passed to the debugged app.
// Author:      Uri Shomroni
// Date:        10/10/2010
// ---------------------------------------------------------------------------
HRESULT vspCDebugExceptionEvent::CanPassToDebuggee(void)
{
    // We can pass any exception to the debugged application:
    HRESULT retVal = S_OK;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugExceptionEvent::PassToDebuggee
// Description: Flags this exception to be passed to the debugged app / stopped
//              from passing to it once the process is resumed.
// Author:      Uri Shomroni
// Date:        10/10/2010
// ---------------------------------------------------------------------------
HRESULT vspCDebugExceptionEvent::PassToDebuggee(BOOL fPass)
{
    HRESULT retVal = S_OK;

    // We currently don't have an API for stopping first-chance exceptions:
    if (fPass == FALSE)
    {
        retVal = E_NOTIMPL;
    }

    return retVal;
}

////////////////////////////////////////////////////////////
// vspCDebugExpressionEvaluationCompleteEvent
////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// Name:        vspCDebugExpressionEvaluationCompleteEvent::vspCDebugExpressionEvaluationCompleteEvent
// Description: Constructor
// Author:      Uri Shomroni
// Date:        10/11/2010
// ---------------------------------------------------------------------------
vspCDebugExpressionEvaluationCompleteEvent::vspCDebugExpressionEvaluationCompleteEvent(vspCDebugExpression* pExpression, vspCDebugProperty* pValueAsProperty)
    : vspCDebugEvent(IID_IDebugExpressionEvaluationCompleteEvent2), _pExpression(pExpression), _pValueAsProperty(pValueAsProperty)
{
    GT_IF_WITH_ASSERT(_pExpression != NULL)
    {
        // Retain the expression:
        _pExpression->AddRef();
    }
    GT_IF_WITH_ASSERT(_pValueAsProperty != NULL)
    {
        // Retain the value:
        _pValueAsProperty->AddRef();
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugExpressionEvaluationCompleteEvent::~vspCDebugExpressionEvaluationCompleteEvent
// Description: Destructor
// Author:      Uri Shomroni
// Date:        10/11/2010
// ---------------------------------------------------------------------------
vspCDebugExpressionEvaluationCompleteEvent::~vspCDebugExpressionEvaluationCompleteEvent()
{
    // Release the expression and the value:
    if (_pExpression != NULL)
    {
        _pExpression->Release();
        _pExpression = NULL;
    }

    if (_pValueAsProperty != NULL)
    {
        _pValueAsProperty->Release();
        _pValueAsProperty = NULL;
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugExpressionEvaluationCompleteEvent::eventAttributes
// Description: Returns the event attributes
// Author:      Uri Shomroni
// Date:        10/11/2010
// ---------------------------------------------------------------------------
DWORD vspCDebugExpressionEvaluationCompleteEvent::eventAttributes() const
{
    // Expression evaluation events are asynchronous:
    return EVENT_ASYNCHRONOUS;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugExpressionEvaluationCompleteEvent::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        10/11/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugExpressionEvaluationCompleteEvent::AddRef(void)
{
    // Use the base class's implementation:
    return vspCDebugEvent::AddRef();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugExpressionEvaluationCompleteEvent::AddRef
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        10/11/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugExpressionEvaluationCompleteEvent::Release(void)
{
    // Use the base class's implementation:
    return vspCDebugEvent::Release();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugExpressionEvaluationCompleteEvent::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        10/11/2010
// ---------------------------------------------------------------------------
HRESULT vspCDebugExpressionEvaluationCompleteEvent::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    HRESULT retVal = S_OK;

    if ((ppvObj != NULL) && (riid == IID_IDebugExpressionEvaluationCompleteEvent2))
    {
        *ppvObj = (IDebugExpressionEvaluationCompleteEvent2*)this;
        AddRef();
    }
    else
    {
        retVal = vspCDebugEvent::QueryInterface(riid, ppvObj);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugExpressionEvaluationCompleteEvent::GetExpression
// Description: Returns the expression
// Author:      Uri Shomroni
// Date:        10/11/2010
// ---------------------------------------------------------------------------
HRESULT vspCDebugExpressionEvaluationCompleteEvent::GetExpression(IDebugExpression2** ppExpr)
{
    HRESULT retVal = S_OK;

    if (ppExpr != NULL)
    {
        // Return the expression:
        *ppExpr = _pExpression;

        if (_pExpression != NULL)
        {
            _pExpression->AddRef();
        }
        else
        {
            retVal = E_FAIL;
        }
    }
    else // ppExpr == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugExpressionEvaluationCompleteEvent::GetResult
// Description: Returns the evaluation result
// Author:      Uri Shomroni
// Date:        10/11/2010
// ---------------------------------------------------------------------------
HRESULT vspCDebugExpressionEvaluationCompleteEvent::GetResult(IDebugProperty2** ppResult)
{
    HRESULT retVal = S_OK;

    if (ppResult != NULL)
    {
        // Return the value:
        *ppResult = _pValueAsProperty;

        if (_pValueAsProperty != NULL)
        {
            _pValueAsProperty->AddRef();
        }
        else
        {
            retVal = E_FAIL;
        }
    }
    else // ppResult == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}

