//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscDebugThread.cpp
///
//==================================================================================

//------------------------------ vspDebugThread.cpp ------------------------------

#include "stdafx.h"

// Windows:
#include <intsafe.h>

// Infra:
#include <AMDTAPIClasses/Include/apAIDFunctions.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>
#include <AMDTOSWrappers/Include/osCallStack.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afProjectManager.h>

// AMDTGpuDebuggingComponents:
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdThreadsEventObserver.h>

// Local:
#include <CodeXLVSPackage/Include/vspStringConstants.h>
#include <src/vscDebugEngine.h>
#include <src/vscDebugThread.h>
#include <src/vscCallsStack.h>

// ---------------------------------------------------------------------------
// Name:        vspCDebugThread::vspCDebugThread
// Description: Constructor
// Author:      Uri Shomroni
// Date:        12/9/2010
// ---------------------------------------------------------------------------
vspCDebugThread::vspCDebugThread(vspCDebugEngine* pDebugEngine, DWORD threadId, bool isKernelDebuggingThread, bool isMainThread)
    : _pDebugEngine(pDebugEngine), _threadId(threadId), m_isKernelDebuggingThread(isKernelDebuggingThread), _isMainThread(isMainThread), _threadIndex(0), _threadState(THREADSTATE_RUNNING)
{
    if (!m_isKernelDebuggingThread)
    {
        _threadIndex = gdThreadsEventObserver::getThreadInternalID(_threadId);
    }
    else // m_isKernelDebuggingThread
    {
        _threadIndex = DWORD_MAX - _threadId;
    }

    // Retain the debug engine:
    GT_IF_WITH_ASSERT(_pDebugEngine != NULL)
    {
        _pDebugEngine->AddRef();
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugThread::~vspCDebugThread
// Description: Destructor
// Author:      Uri Shomroni
// Date:        12/9/2010
// ---------------------------------------------------------------------------
vspCDebugThread::~vspCDebugThread()
{
    // Retain the debug engine:
    GT_IF_WITH_ASSERT(_pDebugEngine != NULL)
    {
        _pDebugEngine->Release();
        _pDebugEngine = NULL;
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugThread::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        12/9/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugThread::AddRef(void)
{
    return vspCUnknown::addRef();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugThread::Release
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        12/9/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugThread::Release(void)
{
    return vspCUnknown::release();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugThread::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        12/9/2010
// ---------------------------------------------------------------------------
HRESULT vspCDebugThread::QueryInterface(REFIID riid, LPVOID* ppvObj)
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
    else if (riid == IID_IDebugThread2)
    {
        *ppvObj = (IDebugThread2*)this;
        AddRef();
    }
    else // riid != IID_IUnknown, IID_IDebugThread2
    {
        retVal = E_NOINTERFACE;
    }

    return retVal;
}

//////////////////////////////////////////////////////////////////////////////
// IDebugThread2 methods
HRESULT vspCDebugThread::SetThreadName(LPCOLESTR pszName)
{
    GT_UNREFERENCED_PARAMETER(pszName);

    return E_NOTIMPL;
}

HRESULT vspCDebugThread::GetName(BSTR* pbstrName)
{
    GT_UNREFERENCED_PARAMETER(pbstrName);

    return E_NOTIMPL;
}

HRESULT vspCDebugThread::GetProgram(IDebugProgram2** ppProgram)
{
    GT_UNREFERENCED_PARAMETER(ppProgram);

    return E_NOTIMPL;
}

HRESULT vspCDebugThread::CanSetNextStatement(IDebugStackFrame2* pStackFrame,
                                             IDebugCodeContext2* pCodeContext)
{
    GT_UNREFERENCED_PARAMETER(pStackFrame);
    GT_UNREFERENCED_PARAMETER(pCodeContext);

    return S_FALSE;
}

HRESULT vspCDebugThread::SetNextStatement(IDebugStackFrame2* pStackFrame,
                                          IDebugCodeContext2* pCodeContext)
{
    GT_UNREFERENCED_PARAMETER(pStackFrame);
    GT_UNREFERENCED_PARAMETER(pCodeContext);

    return E_NOTIMPL;
}

HRESULT vspCDebugThread::Suspend(DWORD* pdwSuspendCount)
{
    GT_UNREFERENCED_PARAMETER(pdwSuspendCount);

    return E_NOTIMPL;
}

HRESULT vspCDebugThread::Resume(DWORD* pdwSuspendCount)
{
    GT_UNREFERENCED_PARAMETER(pdwSuspendCount);

    return E_NOTIMPL;
}

HRESULT vspCDebugThread::GetThreadProperties(THREADPROPERTY_FIELDS dwFields,
                                             THREADPROPERTIES* ptp)
{
    HRESULT retVal = S_OK;

    if (ptp != NULL)
    {
        if ((dwFields & TPF_ID) != 0)
        {
            ptp->dwThreadId = _threadId;
            ptp->dwFields |= TPF_ID;
        }

        if ((dwFields & TPF_SUSPENDCOUNT) != 0)
        {
            // We currently do not support thread suspension:
            ptp->dwSuspendCount = 0;
            ptp->dwFields |= TPF_SUSPENDCOUNT;
        }

        if ((dwFields & TPF_STATE) != 0)
        {
            // We currently do not support thread diagnostics or thread suspension, so stopped, frozen, etc are irrelevant:
            ptp->dwThreadState = _threadState;
            ptp->dwFields |= TPF_STATE;
        }

        if ((dwFields & TPF_PRIORITY) != 0)
        {
            // TO_DO: Get a real value
            ptp->bstrPriority = SysAllocString(L"Normal");
            ptp->dwFields |= TPF_PRIORITY;
        }

        if ((dwFields & TPF_NAME) != 0)
        {
            // Get the thread's name:
            gtString threadName;

            if (_isMainThread)
            {
                threadName = VSP_STR_ThreadMainThreadName;
            }
            else if (!m_isKernelDebuggingThread)
            {
                threadName.appendFormattedString(VSP_STR_ThreadNormalThreadName, _threadIndex);
            }
            else // !_isMainThread && m_isKernelDebuggingThread && 0 != _threadIndex
            {
                threadName.appendFormattedString(VSP_STR_ThreadKernelDebuggingWavefrontThreadName, _threadIndex + 1);
            }

            ptp->bstrName = SysAllocString(threadName.asCharArray());
            ptp->dwFields |= TPF_NAME;
        }

        if ((dwFields & TPF_LOCATION) != 0)
        {
            // Calculate the location:
            gtString location = VSP_STR_ThreadLocationUnknown;

            // Get the thread's calls stack:
            osCallStack threadCallsStack;
            bool rcStack = false;

            if (!m_isKernelDebuggingThread)
            {
                rcStack = gaGetThreadCallStack(_threadId, threadCallsStack);
            }
            else // m_isKernelDebuggingThread
            {
                // We assume the active wavefront is selected:
                rcStack = gaGetCurrentlyDebuggedKernelCallStack(threadCallsStack);
            }

            GT_IF_WITH_ASSERT(rcStack)
            {
                // Get the top frame:
                const osCallStackFrame* pStackTopFrame = threadCallsStack.stackFrame(0);
                GT_IF_WITH_ASSERT(pStackTopFrame != NULL)
                {
                    // If we have a function name, use it. Otherwise, use the instruction address:
                    const gtString& topFrameFuncName = pStackTopFrame->functionName();

                    if (topFrameFuncName.isEmpty())
                    {
                        gdUserApplicationAddressToDisplayString((osProcedureAddress64)pStackTopFrame->instructionCounterAddress(), location);
                    }
                    else // !topFrameFuncName.isEmpty()
                    {
                        location = topFrameFuncName;
                    }
                }
            }

            ptp->bstrLocation = SysAllocString(location.asCharArray());
            ptp->dwFields |= TPF_LOCATION;
        }
    }
    else // ptp == NULL
    {
        retVal = E_POINTER;
    }

    return retVal;
}

HRESULT vspCDebugThread::GetLogicalThread(IDebugStackFrame2* pStackFrame,
                                          IDebugLogicalThread2** ppLogicalThread)
{
    GT_UNREFERENCED_PARAMETER(pStackFrame);
    GT_UNREFERENCED_PARAMETER(ppLogicalThread);

    return E_NOTIMPL;
}

HRESULT vspCDebugThread::GetThreadId(DWORD* pdwThreadId)
{
    HRESULT retVal = S_OK;

    if (pdwThreadId != NULL)
    {
        // Return the thread Id:
        *pdwThreadId = _threadId;
    }
    else
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}

HRESULT vspCDebugThread::EnumFrameInfo(FRAMEINFO_FLAGS dwFieldSpec,
                                       UINT nRadix,
                                       IEnumDebugFrameInfo2** ppEnum)
{
    HRESULT retVal = S_OK;

    if (ppEnum != NULL)
    {
        // Determine the base to be used for numeric values:
        bool useHexStrings = false;

        if (nRadix == 16)
        {
            useHexStrings = true;
        }
        else
        {
            GT_ASSERT(nRadix == 10);
        }

        // Get the thread's calls stack:
        osCallStack threadCallStack;
        bool rcStack = false;

        if (!m_isKernelDebuggingThread)
        {
            rcStack = gaGetThreadCallStack(_threadId, threadCallStack);
        }
        else // m_isKernelDebuggingThread
        {
            // We assume the active wavefront is selected:
            rcStack = gaGetCurrentlyDebuggedKernelCallStack(threadCallStack);
        }

        GT_IF_WITH_ASSERT(rcStack)
        {
            // if there is a osFilePath and file does not exists look in the additional directories:
            int stackSize = threadCallStack.amountOfStackFrames();

            for (int stackFrameNum = 0 ; stackFrameNum < stackSize ; stackFrameNum ++)
            {
                const osCallStackFrame* pCurrentFrame = threadCallStack.stackFrame(stackFrameNum);
                GT_IF_WITH_ASSERT(NULL != pCurrentFrame)
                {
                    // check if there is a source code at all and does not exists:
                    osFilePath originalFilePath = pCurrentFrame->sourceCodeFilePath();

                    if (!originalFilePath.fileDirectoryAsString().isEmpty() && !originalFilePath.exists())
                    {
                        osFilePath filePathToOpen;

                        // If it is a source file look at addition directory if defined and if the file is source file:
                        gtString additionalSourceCodeDir = afProjectManager::instance().currentProjectSettings().SourceFilesDirectories();

                        // Mark termination of process that the file was found:
                        bool fileExistsOnDisk = false;

                        gtStringTokenizer sourceFoldersTokenizer(additionalSourceCodeDir, L";");
                        gtString currentSourceDir;

                        while (sourceFoldersTokenizer.getNextToken(currentSourceDir))
                        {
                            fileExistsOnDisk = apLookForFileInAdditionalDirectories(originalFilePath, currentSourceDir, filePathToOpen);

                            if (fileExistsOnDisk)
                            {
                                break;
                            }
                        }

                        if (!fileExistsOnDisk)
                        {
                            // Check with a new source code root directory
                            gtString additionalRootDir = afProjectManager::instance().currentProjectSettings().SourceCodeRootLocation();

                            if (!additionalRootDir.isEmpty())
                            {
                                fileExistsOnDisk = apLookForFileInAdditionalDirectories(originalFilePath, additionalRootDir, filePathToOpen);
                            }
                        }

                        if (fileExistsOnDisk)
                        {
                            osCallStackFrame updatedStackFrame = *pCurrentFrame;
                            updatedStackFrame.setSourceCodeFilePath(filePathToOpen);
                            threadCallStack.setStackFrame(updatedStackFrame, stackFrameNum);
                        }
                    }
                }
            }

            osThreadId stackThreadId = m_isKernelDebuggingThread ? OS_NO_THREAD_ID : _threadId;
            vspCEnumDebugFrameInfo* pEnumFrames = new vspCEnumDebugFrameInfo(stackThreadId, threadCallStack, dwFieldSpec, useHexStrings, *_pDebugEngine);
            *ppEnum = (IEnumDebugFrameInfo2*)pEnumFrames;
        }
        else // !rcStack
        {
            // We could not get the calls stack, so we let VS know the stack not available:
            retVal = E_FAIL;
        }
    }
    else // ppEnum == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspCEnumDebugThreads::vspCEnumDebugThreads
// Description: Constructor
// Author:      Uri Shomroni
// Date:        13/9/2010
// ---------------------------------------------------------------------------
vspCEnumDebugThreads::vspCEnumDebugThreads(const gtVector<vspCDebugThread*>& currentThreads)
    : _currentPosition(0)
{
    unsigned int numberOfThreads = (unsigned int)currentThreads.size();

    for (unsigned int i = 0; i < numberOfThreads; i++)
    {
        vspCDebugThread* pCurrentThread = currentThreads[i];
        GT_IF_WITH_ASSERT(pCurrentThread != NULL)
        {
            // Add the thread to our vector and retain it.
            _enumThreads.push_back(pCurrentThread);
            pCurrentThread->AddRef();

        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCEnumDebugThreads::~vspCEnumDebugThreads
// Description: Destructor
// Author:      Uri Shomroni
// Date:        13/9/2010
// ---------------------------------------------------------------------------
vspCEnumDebugThreads::~vspCEnumDebugThreads()
{
    // Reduce the threads' reference counts:
    unsigned int amountOfThreads = (unsigned int)_enumThreads.size();

    for (unsigned int i = 0; i < amountOfThreads; i++)
    {
        // Sanity check:
        vspCDebugThread* pCurrentThread = _enumThreads[i];
        GT_IF_WITH_ASSERT(pCurrentThread != NULL)
        {
            // Release the current thread and set the vector item to NULL:
            pCurrentThread->Release();
            _enumThreads[i] = NULL;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCEnumDebugThreads::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        13/9/2010
// ---------------------------------------------------------------------------
ULONG vspCEnumDebugThreads::AddRef(void)
{
    return vspCUnknown::addRef();
}

// ---------------------------------------------------------------------------
// Name:        vspCEnumDebugThreads::AddRef
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        13/9/2010
// ---------------------------------------------------------------------------
ULONG vspCEnumDebugThreads::Release(void)
{
    return vspCUnknown::release();
}

// ---------------------------------------------------------------------------
// Name:        vspCEnumDebugThreads::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        13/9/2010
// ---------------------------------------------------------------------------
HRESULT vspCEnumDebugThreads::QueryInterface(REFIID riid, LPVOID* ppvObj)
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
    else if (riid == IID_IEnumDebugThreads2)
    {
        *ppvObj = (IEnumDebugThreads2*)this;
        AddRef();
    }
    else // riid != IID_IUnknown, IID_IEnumDebugThreads2
    {
        retVal = E_NOINTERFACE;
    }

    return retVal;
}

//////////////////////////////////////////////////////////////////////////////
// IEnumDebugThreads2 methods
HRESULT vspCEnumDebugThreads::Next(ULONG celt, IDebugThread2** rgelt, ULONG* pceltFetched)
{
    HRESULT retVal = S_OK;

    // Will get the amount of threads we successfully returned:
    ULONG fetchedItems = 0;

    if (rgelt != NULL)
    {
        unsigned int amountOfThreads = (unsigned int)_enumThreads.size();

        // Try to fill as many items as the caller requested:
        for (ULONG i = 0; i < celt; i++)
        {
            // If we are overflowing
            if (_currentPosition >= amountOfThreads)
            {
                retVal = S_FALSE;
                break;
            }

            // Get the current item:
            vspCDebugThread* pCurrentThread = _enumThreads[_currentPosition];
            GT_IF_WITH_ASSERT(pCurrentThread != NULL)
            {
                // Return it and increment its reference count and the amount of items returned:
                rgelt[fetchedItems] = pCurrentThread;
                pCurrentThread->AddRef();
                fetchedItems++;
            }

            // Advance the current position:
            _currentPosition++;
        }
    }
    else // rgelt == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    // If the caller requested the fetched amount, return it:
    if (pceltFetched != NULL)
    {
        *pceltFetched = fetchedItems;
    }

    return retVal;
}
HRESULT vspCEnumDebugThreads::Skip(ULONG celt)
{
    HRESULT retVal = S_OK;

    // Get the amount of threads:
    unsigned int amountOfThreads = (unsigned int)_enumThreads.size();

    // Advance the current position:
    _currentPosition += (unsigned int)celt;

    // If we moved past the end, return S_FALSE and reset the position to the end:
    if (_currentPosition > amountOfThreads)
    {
        retVal = S_FALSE;
        _currentPosition = amountOfThreads;
    }

    return retVal;
}
HRESULT vspCEnumDebugThreads::Reset(void)
{
    HRESULT retVal = S_OK;

    // Reset the position to the beginning:
    _currentPosition = 0;

    return retVal;
}
HRESULT vspCEnumDebugThreads::Clone(IEnumDebugThreads2** ppEnum)
{
    HRESULT retVal = S_OK;

    if (ppEnum != NULL)
    {
        // Create a duplicate of this item (note that this will increment the threads' reference counts:
        vspCEnumDebugThreads* pClone = new vspCEnumDebugThreads(_enumThreads);

        // Set its position to equal ours:
        pClone->_currentPosition = _currentPosition;

        // Return it:
        *ppEnum = (IEnumDebugThreads2*)pClone;
    }
    else // ppEnum == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}
HRESULT vspCEnumDebugThreads::GetCount(ULONG* pcelt)
{
    HRESULT retVal = S_OK;

    if (pcelt != NULL)
    {
        // Return the count:
        *pcelt = (ULONG)_enumThreads.size();
    }
    else
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}
