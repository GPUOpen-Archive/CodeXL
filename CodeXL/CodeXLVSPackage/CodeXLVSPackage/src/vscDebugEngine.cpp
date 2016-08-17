//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscDebugEngine.cpp
///
//==================================================================================

//------------------------------ vspDebugEngine.cpp ------------------------------

// src/vspDebugEngine.cpp : Implementation of vspCDebugEngine

#include "stdafx.h"

// Windows:
#include <intsafe.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/apKernelDebuggingCommand.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTProcessDebugger/Include/pdProcessDebugger.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>

// CodeXLVSPackageDebugger:
#include <CodeXLVSPackageDebugger/Include/vsdPackageConnector.h>

// Local:
#include <CodeXLVSPackage/Include/vspStringConstants.h>
#include <src/vscBreakpointsManager.h>
#include <src/vscDebugContext.h>
#include <src/vscDebugEngine.h>
#include <src/vscDebugModule.h>
#include <src/vscDebugThread.h>
#include <src/vspExpressionEvaluator.h>
#include <src/vspWindowsManager.h>

#define VSP_PATH_ENV_VARIABLE_NAME L"PATH"

// Static members initialization:
int vspCDebugEngine::_static_amountOfAllocatedEngines = 0;
IVscDebugEngineOwner* vspCDebugEngine::_pOwner = NULL;


class PackageModule :
    public CAtlDllModuleT<PackageModule>
{
public:
    BOOL APIENTRY DllMain(DWORD reason, LPVOID reserved, HINSTANCE hInstance)
    {
        GT_UNREFERENCED_PARAMETER(reason);
        GT_UNREFERENCED_PARAMETER(reserved);
        GT_UNREFERENCED_PARAMETER(hInstance);

        return TRUE;
    }
};
PackageModule _AtlModule;

HRESULT vscRegisterDebugEngine()
{
    return _AtlModule.RegisterServer(TRUE);
}
HRESULT vscUnregisterDebugEngine()
{
    return _AtlModule.UnregisterServer(TRUE);
}

// This macro is used as default registry root when a NULL parameter is passed to VSDllRegisterServer
// or VSDllUnregisterServer. For sample code we set as default the experimental instance, but for production
// code you should change it to the standard VisualStudio instance that is LREGKEY_VISUALSTUDIOROOT.
#define DEFAULT_REGISTRY_ROOT LREGKEY_VISUALSTUDIOROOT
#pragma warning(push)
#pragma warning(disable : 4702)
// Since this project defines an oleautomation interface, the typelib needs to be registered.
#define VSL_REGISTER_TYPE_LIB TRUE
extern "C" {

    // Returns a class factory to create an object of the requested type
    STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
    {
        HRESULT result = S_OK;
        bool done = false;
        VSL_STDMETHODTRY
        {

            result = _AtlModule.GetClassObject(rclsid, riid, ppv);
            done = true;

        } VSL_STDMETHODCATCH()

        if (!done)
        {
            result = VSL_GET_STDMETHOD_HRESULT();
        }

        return result;
    }
};

// Used by COM to determine whether the DLL can be unloaded
STDAPI DllCanUnloadNow()
{
    HRESULT result = S_OK;
    bool done = false;
    VSL_STDMETHODTRY
    {
        result = _AtlModule.DllCanUnloadNow();
        done = true;
    } VSL_STDMETHODCATCH()

    if (!done)
    {
        result = VSL_GET_STDMETHOD_HRESULT();
    }

    return result;
}
#pragma warning(pop)

// vspCDebugEngine

// ---------------------------------------------------------------------------
// Name:        vspCDebugEngine::vspCDebugEngine
// Description: Constructor
// Author:      Uri Shomroni
// Date:        4/10/2010
// ---------------------------------------------------------------------------
vspCDebugEngine::vspCDebugEngine() : _piDebuggedProcess(NULL), _piDebugPortNotify(NULL), _eventObserver(*this, NULL),
    _isCurrentlyKernelDebugging(false)
{
    // Increase the amount of engines allocated:
    _static_amountOfAllocatedEngines++;

    // Make sure that there is only one instance of the debug engine:
    if (_static_amountOfAllocatedEngines != 1)
    {
        gtString dbg;
        dbg.appendFormattedString(L"Error: There are %d allocated debug engine objects", _static_amountOfAllocatedEngines);
        GT_ASSERT_EX(false, dbg.asCharArray());
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugEngine::~vspCDebugEngine
// Description: Destructor
// Author:      Uri Shomroni
// Date:        20/9/2010
// ---------------------------------------------------------------------------
vspCDebugEngine::~vspCDebugEngine()
{
    // Decrease the amount of engines allocated:
    _static_amountOfAllocatedEngines--;

    // Release all our threads:
    int numberOfThreads = (int)_debugProcessThreads.size();

    for (int i = 0; i < numberOfThreads; i++)
    {
        vspCDebugThread* pCurrentThread = _debugProcessThreads[i];
        GT_IF_WITH_ASSERT(pCurrentThread != NULL)
        {
            pCurrentThread->Release();
            _debugProcessThreads[i] = NULL;
        }
    }

    // Release all our modules:
    int numberOfModules = (int)_debugProcessModules.size();

    for (int i = 0; i < numberOfModules; i++)
    {
        vspCDebugModule* pCurrentModule = _debugProcessModules[i];
        GT_IF_WITH_ASSERT(pCurrentModule != NULL)
        {
            pCurrentModule->Release();
            _debugProcessModules[i] = NULL;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
//IDebugEngine2 methods
HRESULT vspCDebugEngine::EnumPrograms(IEnumDebugPrograms2** ppEnum)
{
    GT_UNREFERENCED_PARAMETER(ppEnum);

    return E_NOTIMPL;
}

HRESULT vspCDebugEngine::SetException(EXCEPTION_INFO* pException)
{
    GT_UNREFERENCED_PARAMETER(pException);

    return E_NOTIMPL;
}

HRESULT vspCDebugEngine::RemoveSetException(EXCEPTION_INFO* pException)
{
    GT_UNREFERENCED_PARAMETER(pException);

    return E_NOTIMPL;
}

HRESULT vspCDebugEngine::RemoveAllSetExceptions(REFGUID guidType)
{
    GT_UNREFERENCED_PARAMETER(guidType);

    return E_NOTIMPL;
}

HRESULT vspCDebugEngine::GetEngineId(GUID* pguidEngine)
{
    HRESULT retVal = S_OK;

    if (pguidEngine != NULL)
    {
        // Return our GUID
        *pguidEngine = __uuidof(vspDebugEngine);
    }
    else
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}
HRESULT vspCDebugEngine::DestroyProgram(IDebugProgram2* pProgram)
{
    GT_UNREFERENCED_PARAMETER(pProgram);

    return E_NOTIMPL;
}

HRESULT vspCDebugEngine::SetLocale(WORD wLangID)
{
    GT_UNREFERENCED_PARAMETER(wLangID);

    return E_NOTIMPL;
}

HRESULT vspCDebugEngine::SetRegistryRoot(LPCOLESTR pszRegistryRoot)
{
    GT_UNREFERENCED_PARAMETER(pszRegistryRoot);

    return E_NOTIMPL;
}

HRESULT vspCDebugEngine::SetMetric(LPCOLESTR pszMetric, VARIANT varValue)
{
    GT_UNREFERENCED_PARAMETER(pszMetric);
    GT_UNREFERENCED_PARAMETER(varValue);

    return E_NOTIMPL;
}

HRESULT vspCDebugEngine::CauseBreak(void)
{
    HRESULT retVal = S_OK;

    bool rcBrk = gaSuspendDebuggedProcess();
    GT_ASSERT(rcBrk);

    if (!rcBrk)
    {
        retVal = E_FAIL;
    }

    return retVal;
}

HRESULT vspCDebugEngine::Attach(
    IDebugProgram2** rgpPrograms,
    IDebugProgramNode2** rgpProgramNodes,
    DWORD celtPrograms,
    IDebugEventCallback2* pCallback,
    ATTACH_REASON dwReason)
{
    GT_UNREFERENCED_PARAMETER(rgpProgramNodes);

    HRESULT retVal = S_OK;

    // Verify expected parameter values
    GT_ASSERT(celtPrograms == 1);
    GT_ASSERT(pCallback == _eventObserver.debugEventCallbackInterface());
    GT_ASSERT(dwReason == ATTACH_REASON_LAUNCH);

    // If there are programs to attach to:
    if (celtPrograms > 0)
    {
        // Get the program GUID:
        HRESULT hr = rgpPrograms[0]->GetProgramId(&_programGUID);

        GT_IF_WITH_ASSERT(hr == S_OK)
        {
            // Start running the process:
            bool rcCont = gaContinueDebuggedProcessFromSuspendedCreation();

            if (!rcCont)
            {
                retVal = E_FAIL;
            }
        }
    }

    return retVal;
}

HRESULT vspCDebugEngine::ContinueFromSynchronousEvent(IDebugEvent2* pEvent)
{
    GT_UNREFERENCED_PARAMETER(pEvent);

    //TODO: IMPLEMENT CONTINUE
    return E_NOTIMPL ;
}

HRESULT vspCDebugEngine::CreatePendingBreakpoint(
    IDebugBreakpointRequest2* pBPRequest,
    IDebugPendingBreakpoint2** ppPendingBP)
{
    HRESULT retVal = S_OK;

    if (ppPendingBP != NULL)
    {
        vspCDebugBreakpoint* pBreakpoint = vscBreakpointsManager::instance().respondToBreakpointRequest(pBPRequest);

        if (pBreakpoint != NULL)
        {
            // Return the breakpoint:
            *ppPendingBP = pBreakpoint;
            pBreakpoint->AddRef();
        }
        else // pBreakpoint == NULL
        {
            // Could not create breakpoint:
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


//////////////////////////////////////////////////////////////////////////////
// IDebugEngineLaunch2 methods
HRESULT vspCDebugEngine::LaunchSuspended(
    LPCOLESTR pszMachine,
    IDebugPort2* pPort,
    LPCOLESTR pszExe,
    LPCOLESTR pszArgs,
    LPCOLESTR pszDir,
    BSTR bstrEnv,
    LPCOLESTR pszOptions,
    LAUNCH_FLAGS dwLaunchFlags,
    DWORD hStdInput,
    DWORD hStdOutput,
    DWORD hStdError,
    IDebugEventCallback2* pCallback,
    IDebugProcess2** ppDebugProcess)
{
    GT_UNREFERENCED_PARAMETER(pszMachine);
    GT_UNREFERENCED_PARAMETER(pszOptions);
    GT_UNREFERENCED_PARAMETER(dwLaunchFlags);
    GT_UNREFERENCED_PARAMETER(hStdInput);
    GT_UNREFERENCED_PARAMETER(hStdOutput);
    GT_UNREFERENCED_PARAMETER(hStdError);

    HRESULT retVal = S_OK;

    GT_IF_WITH_ASSERT(_pOwner != NULL)
    {
        // Inform the package of this class's activation:
        _pOwner->InformPackageOfNewDebugEngine(this);

        if (ppDebugProcess != NULL)
        {
            GT_IF_WITH_ASSERT(pCallback != NULL)
            {
                _eventObserver.setDebugEventCallbackInterface(pCallback);
                vscBreakpointsManager::instance().setDebugEventCallback(pCallback);
                vspExpressionEvaluator::instance().setDebugEventCallback(pCallback);
            }

            *ppDebugProcess = NULL;

            // Get the process from a processId:
            GT_IF_WITH_ASSERT(pPort != NULL)
            {
                // Fill the process creation data:
                gtString executablePathAsString = pszExe;
                osFilePath executablePath(executablePathAsString);
                gtString workingDirPathAsString = pszDir;
                gtString argumentsAsString = pszArgs != NULL ? pszArgs : L"";
                gtString environmentAsString = bstrEnv != NULL ? bstrEnv : L"";
                osFilePath defaultLogFilesPath(osFilePath::OS_TEMP_DIRECTORY);

                // Get the "debugger install directory", which is actually the directory where the DLLs are stored:
                gtString debuggerInstallDirAsString;
                osFilePath directoryPath;
                bool rcBin = directoryPath.SetInstallRelatedPath(osFilePath::OS_CODEXL_BINARIES_PATH);
                GT_IF_WITH_ASSERT(rcBin)
                {
                    debuggerInstallDirAsString = directoryPath.clearFileExtension().clearFileName().asString();
                }

                // Get the spies directory as a string:
                gtString spiesDirAsString = debuggerInstallDirAsString;
                bool rcSpy = directoryPath.SetInstallRelatedPath(osFilePath::OS_CODEXL_SERVERS_PATH);
                GT_IF_WITH_ASSERT(rcSpy)
                {
                    spiesDirAsString = directoryPath.clearFileExtension().clearFileName().asString();

                    bool is64Bit = false;
                    bool rc64 = osIs64BitModule(executablePath, is64Bit);

                    if (rc64 && is64Bit)
                    {
                        spiesDirAsString.replace(OS_SPIES_SUB_DIR_NAME, OS_SPIES_64_SUB_DIR_NAME);
                    }
                }

                // Get the global variables manager:
                gdGDebuggerGlobalVariablesManager& globalVarsManager = gdGDebuggerGlobalVariablesManager::instance();

                _processCreationData.copyFrom(afProjectManager::instance().currentProjectSettings());

                // CODEXL-2300 - Disable the VS debug engine for 64-bit targets:
                bool is64Bit = false;
                bool rc64Bit = osIs64BitModule(_processCreationData.executablePath(), is64Bit);

                if (rc64Bit && is64Bit)
                {
                    // US- 15/8/16 - comment out this line to re-enable 64-bit host debugging, as the hang on startup seems to be fixed now.
                    _processCreationData.setShouldDisableVSDebugEngine(true);
                }

                // Set the process created data parameters:
                _processCreationData.setExecutablePath(executablePath);
                _processCreationData.setCommandLineArguments(argumentsAsString);
                _processCreationData.setWorkDirectory(workingDirPathAsString);
                _processCreationData.clearEnvironmentVariables();
                _processCreationData.addEnvironmentVariablesString(environmentAsString, AF_STR_newProjectEnvironmentVariablesDelimitersVS);
                _processCreationData.setSpiesDirectory(spiesDirAsString);
                _processCreationData.setDebuggerInstallDir(debuggerInstallDirAsString);
                _processCreationData.setFrameTerminators(globalVarsManager.currentDebugProjectSettings().frameTerminatorsMask());
                _processCreationData.setShouldInitializePerformanceCounters(false);
                _processCreationData.setLoggedImagesFileType(globalVarsManager.imagesFileFormat());
                _processCreationData.setLogFilesFolder(afGlobalVariablesManager::instance().logFilesDirectoryPath());

                // Get max logged calls numbers, and set in process creation data:
                unsigned int maxLoggedOpenGLCallsPerContext = AP_DEFAULT_OPENGL_CONTEXT_CALLS_LOG_MAX_SIZE;
                unsigned int maxLoggedOpenCLCalls = AP_DEFAULT_OPENCL_CONTEXT_CALLS_LOG_MAX_SIZE;
                unsigned int maxLoggedOpenCLCommandsPerQueue = AP_DEFAULT_OPENCL_QUEUE_COMMANDS_LOG_MAX_SIZE;
                globalVarsManager.getLoggingLimits(maxLoggedOpenGLCallsPerContext, maxLoggedOpenCLCalls, maxLoggedOpenCLCommandsPerQueue);
                _processCreationData.setMaxLoggedOpenGLCallsPerContext(maxLoggedOpenGLCallsPerContext);
                _processCreationData.setMaxLoggedOpenCLCallsPerContext(maxLoggedOpenCLCalls);
                _processCreationData.setMaxLoggedOpenCLCommandsPerQueue(maxLoggedOpenCLCommandsPerQueue);


                // Add the debugger install dir to the path, since pdWin32ProcessDebugger::addDebuggedProcessWordDirToPath will add
                // the IDE's path, not the debugger install dir:
                gtString pathEnvVarValue;
                _processCreationData.getEnvironmentVariable(VSP_PATH_ENV_VARIABLE_NAME, pathEnvVarValue);

                if (pathEnvVarValue.isEmpty())
                {
                    // If the path isn't set, we are going to inherit the value, so get this process's value:
                    osGetCurrentProcessEnvVariableValue(VSP_PATH_ENV_VARIABLE_NAME, pathEnvVarValue);
                }

                pathEnvVarValue.prepend(osFilePath::osEnvironmentVariablePathsSeparator).prepend(debuggerInstallDirAsString);
                _processCreationData.setEnvironmentVariable(VSP_PATH_ENV_VARIABLE_NAME, pathEnvVarValue);

                // Set the process created data:
                globalVarsManager.setCurrentDebugProjectSettings(_processCreationData);

                // Set the debug port into the CodeXLVSPackageCode dll:
                vsdPackageConnector& theVSDPackageConnector = vsdPackageConnector::instance();
                theVSDPackageConnector.setDebugPort(pPort);
                IDebugPort2* piWrappedPort = theVSDPackageConnector.getWrappedDebugPort();

                if (piWrappedPort == NULL)
                {
                    piWrappedPort = pPort;
                }

                // Launch the debugged process but don't yet start its run:
                bool rcLaunch = gaLaunchDebuggedProcess(_processCreationData, true);

                GT_IF_WITH_ASSERT(rcLaunch)
                {
                    // Get the debugged process ID from the process debugger:
                    osProcessId processId = pdProcessDebugger::instance().debuggedProcessId();

                    // Create the process Id struct:
                    AD_PROCESS_ID adProcessId;
                    adProcessId.ProcessIdType = AD_PROCESS_ID_SYSTEM;
                    adProcessId.ProcessId.dwProcessId = (DWORD)processId;

                    // Get the representation of this process from the debug port:
                    theVSDPackageConnector.setProgramToBeEnumeratedByDebuggedProcess(this);
                    IDebugProcess2* pReturnedProcess = theVSDPackageConnector.debuggedProcess();

                    // If the process was not created by the CodeXLVSPackageDebugger dll, get it directly from the IDebugPort2:
                    if (NULL == pReturnedProcess)
                    {
                        HRESULT hr = piWrappedPort->GetProcess(adProcessId, &pReturnedProcess);
                        GT_ASSERT(SUCCEEDED(hr));
                    }

                    if (NULL != pReturnedProcess)
                    {
                        // Return this value and store it for us. Since the debug engine and the caller are
                        // both users of this interface, we need to retain it (so its ref count will be
                        // incremented by 2 - once for GetProcess and once for AddRef):
                        *ppDebugProcess = pReturnedProcess;
                        _piDebuggedProcess = pReturnedProcess;
                        _piDebuggedProcess->AddRef();
                    }
                }
                else // !rcLaunch
                {
                    retVal = E_FAIL;
                }
            }
            else // pPort == NULL
            {
                retVal = E_FAIL;
            }
        }
        else // ppDebugProcess == NULL
        {
            retVal = E_POINTER;
        }
    }
    return retVal;
}

HRESULT vspCDebugEngine::ResumeProcess(IDebugProcess2* pProcess)
{
    HRESULT retVal = S_OK;

    GT_IF_WITH_ASSERT(pProcess != NULL)
    {
        // Send the process node to the PDM. Note that we do not need to
        // manually publish the process via IDebugProgramPublisher2, since
        // we are in the same process as the SDM/PDM - so we can just notify the
        // debug port.

        // Get the debug port from the process:
        IDebugPort2* pDebugPort = NULL;
        HRESULT hr = pProcess->GetPort(&pDebugPort);
        GT_IF_WITH_ASSERT((hr == S_OK) && (pDebugPort != NULL))
        {
            // Make sure it is a default debug port:
            IDebugDefaultPort2* pDebugDefaultPort = NULL;
            hr = pDebugPort->QueryInterface(IID_IDebugDefaultPort2, (void**)&pDebugDefaultPort);
            GT_IF_WITH_ASSERT((hr == S_OK) && (pDebugDefaultPort != NULL))
            {
                // Get its notification interface:
                IDebugPortNotify2* pDebugPortNotify = NULL;
                hr = pDebugDefaultPort->GetPortNotify(&pDebugPortNotify);
                GT_IF_WITH_ASSERT((hr == S_OK) && (pDebugPortNotify != NULL))
                {
                    // Replace our old port notification interface, if we have one:
                    if (_piDebugPortNotify != NULL)
                    {
                        hr = _piDebugPortNotify->RemoveProgramNode(this);
                        GT_ASSERT(hr == S_OK);
                        _piDebugPortNotify->Release();
                    }

                    _piDebugPortNotify = pDebugPortNotify;

                    // Notify the Debug port of the program node creation:
                    retVal = _piDebugPortNotify->AddProgramNode(this);
                }

                pDebugDefaultPort->Release();
            }
            pDebugPort->Release();
        }
    }

    return retVal;
}

HRESULT vspCDebugEngine::CanTerminateProcess(IDebugProcess2* pProcess)
{
    GT_UNREFERENCED_PARAMETER(pProcess);

    HRESULT retVal = S_OK;

    return retVal;
}

HRESULT vspCDebugEngine::TerminateProcess(IDebugProcess2* pProcess)
{
    HRESULT retVal = S_OK;

    if ((pProcess != NULL) && (_piDebuggedProcess != NULL))
    {
        // Make sure this is our process:
        GUID paramProcessGuid = {0};
        GUID memberProcessGuid = {0};
        HRESULT hr = pProcess->GetProcessId(&paramProcessGuid);

        // Ignore (don't assert) E_NOTIMPL:
        if (hr == S_OK)
        {
            hr = _piDebuggedProcess->GetProcessId(&memberProcessGuid);

            if (hr == S_OK)
            {
                // Compare the GUIDs:
                GT_ASSERT(paramProcessGuid == memberProcessGuid);
            }
        }
    }

    // Terminate the debugged process:
    bool rcTerm = gaTerminateDebuggedProcess();

    if (!rcTerm)
    {
        // We could not terminate the process:
        GT_ASSERT(rcTerm);
        retVal = E_FAIL;
    }

    // Clear the process and port notification anyway:
    onProcessTermination();

    return retVal;
}


//////////////////////////////////////////////////////////////////////////////
// IDebugProgramNode2 methods
HRESULT vspCDebugEngine::GetProgramName(BSTR* pbstrProgramName)
{
    GT_UNREFERENCED_PARAMETER(pbstrProgramName);

    return E_NOTIMPL;
}

HRESULT vspCDebugEngine::GetHostName(DWORD dwHostNameType, BSTR* pbstrHostName)
{
    GT_UNREFERENCED_PARAMETER(dwHostNameType);
    GT_UNREFERENCED_PARAMETER(pbstrHostName);

    return E_NOTIMPL;
}

HRESULT vspCDebugEngine::GetHostMachineName_V7(BSTR* pbstrHostMachineName)
{
    GT_UNREFERENCED_PARAMETER(pbstrHostMachineName);

    return E_NOTIMPL;
}

HRESULT vspCDebugEngine::DetachDebugger_V7(void)
{
    return E_NOTIMPL;
}

HRESULT vspCDebugEngine::Attach_V7(IDebugProgram2* pMDMProgram,
                                   IDebugEventCallback2* pCallback,
                                   DWORD dwReason)
{
    GT_UNREFERENCED_PARAMETER(pMDMProgram);
    GT_UNREFERENCED_PARAMETER(pCallback);
    GT_UNREFERENCED_PARAMETER(dwReason);

    return E_NOTIMPL;
}

HRESULT vspCDebugEngine::GetEngineInfo(BSTR* pbstrEngine, GUID* pguidEngine)
{
    HRESULT retVal = S_OK;

    if (pbstrEngine != NULL)
    {
        // Return the engine name:
        *pbstrEngine = SysAllocString(VSP_STR_DebugEngineName);
    }

    if (pguidEngine != NULL)
    {
        // Return the engine GUID:
        *pguidEngine = __uuidof(vspDebugEngine);
    }

    return retVal;
}

HRESULT vspCDebugEngine::GetHostPid(AD_PROCESS_ID* pHostProcessId)
{
    HRESULT retVal = S_OK;

    if (pHostProcessId != NULL)
    {
        // Return the debugged process ID:
        osProcessId processId = pdProcessDebugger::instance().debuggedProcessId();
        pHostProcessId->ProcessIdType = AD_PROCESS_ID_SYSTEM;
        pHostProcessId->ProcessId.dwProcessId = processId;
    }
    else // pHostProcessId == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}

/*HRESULT vspCDebugEngine::CreatePendingBreakpoint(IDebugBreakpointRequest2 *pBPRequest,
IDebugPendingBreakpoint2 **ppPendingBP)
{
//TODO: CREATE BREAKPOINT
return E_NOTIMPL;
}*/

//////////////////////////////////////////////////////////////////////////////
// IDebugProgram2 methods
HRESULT vspCDebugEngine::EnumThreads(IEnumDebugThreads2** ppEnum)
{
    HRESULT retVal = S_OK;

    if (ppEnum != NULL)
    {
        // Create the enum object:
        vspCEnumDebugThreads* pEnumThreads = new vspCEnumDebugThreads(_debugProcessThreads);

        // Return it:
        *ppEnum = (IEnumDebugThreads2*)pEnumThreads;
    }
    else // ppEnum == NULL
    {
        retVal = E_POINTER;
    }

    return retVal;
}

HRESULT vspCDebugEngine::GetName(BSTR* pbstrName)
{
    GT_UNREFERENCED_PARAMETER(pbstrName);

    return E_NOTIMPL;
}

HRESULT vspCDebugEngine::GetProcess(IDebugProcess2** ppProcess)
{
    GT_UNREFERENCED_PARAMETER(ppProcess);

    return E_NOTIMPL;
}

HRESULT vspCDebugEngine::Terminate(void)
{
#if 0
    TerminateProcess(_piDebuggedProcess);
#else
    return E_NOTIMPL;
#endif
}

HRESULT vspCDebugEngine::Attach(IDebugEventCallback2* pCallback)
{
    GT_UNREFERENCED_PARAMETER(pCallback);

    return E_NOTIMPL;
}

HRESULT vspCDebugEngine::Detach(void)
{
    return E_NOTIMPL;
}

HRESULT vspCDebugEngine::GetDebugProperty(IDebugProperty2** ppProperty)
{
    GT_UNREFERENCED_PARAMETER(ppProperty);

    return E_NOTIMPL;
}

/*HRESULT vspCDebugEngine::CauseBreak(void)
{ return E_NOTIMPL; }*/

HRESULT vspCDebugEngine::EnumCodeContexts(IDebugDocumentPosition2* pDocPos,
                                          IEnumDebugCodeContexts2** ppEnum)
{
    HRESULT retVal = S_OK;

    if (NULL != ppEnum)
    {
        if (NULL != pDocPos)
        {
            BSTR bstrFileName = NULL;
            TEXT_POSITION startPos = {0};
            TEXT_POSITION endPos = {0};
            HRESULT hrName = pDocPos->GetFileName(&bstrFileName);
            HRESULT hrLine = pDocPos->GetRange(&startPos, &endPos);

            if (SUCCEEDED(hrName) && SUCCEEDED(hrLine))
            {
                gtVector<vspCDebugContext*> contexts;

                // Return an empty enumerator, since we don't support CPP source source
                vspCEnumDebugCodeContexts* pOutEnum = new vspCEnumDebugCodeContexts(contexts);
                *ppEnum = pOutEnum;
            }
            else
            {
                retVal = E_FAIL;
            }

            if (NULL != bstrFileName)
            {
                SysFreeString(bstrFileName);
            }
        }
        else
        {
            retVal = E_INVALIDARG;
        }
    }
    else
    {
        retVal = E_POINTER;
    }

    return retVal;
}

HRESULT vspCDebugEngine::GetMemoryBytes(IDebugMemoryBytes2** ppMemoryBytes)
{
    GT_UNREFERENCED_PARAMETER(ppMemoryBytes);

    return E_NOTIMPL;
}

HRESULT vspCDebugEngine::GetDisassemblyStream(DISASSEMBLY_STREAM_SCOPE dwScope,
                                              IDebugCodeContext2* pCodeContext,
                                              IDebugDisassemblyStream2** ppDisassemblyStream)
{
    GT_UNREFERENCED_PARAMETER(dwScope);
    GT_UNREFERENCED_PARAMETER(pCodeContext);
    GT_UNREFERENCED_PARAMETER(ppDisassemblyStream);

    return E_NOTIMPL;
}

HRESULT vspCDebugEngine::EnumModules(IEnumDebugModules2** ppEnum)
{
    HRESULT retVal = S_OK;

    if (ppEnum != NULL)
    {
        // Create the enum object:
        vspCEnumDebugModules* pEnumModules = new vspCEnumDebugModules(_debugProcessModules);

        // Return it:
        *ppEnum = (IEnumDebugModules2*)pEnumModules;
    }
    else // ppEnum == NULL
    {
        retVal = E_POINTER;
    }

    return retVal;
}

HRESULT vspCDebugEngine::GetENCUpdate(IDebugENCUpdate** ppUpdate)
{
    GT_UNREFERENCED_PARAMETER(ppUpdate);

    return E_NOTIMPL;
}

HRESULT vspCDebugEngine::EnumCodePaths(LPCOLESTR pszHint,
                                       IDebugCodeContext2* pStart,
                                       IDebugStackFrame2* pFrame,
                                       BOOL fSource,
                                       IEnumCodePaths2** ppEnum,
                                       IDebugCodeContext2** ppSafety)
{
    GT_UNREFERENCED_PARAMETER(pszHint);
    GT_UNREFERENCED_PARAMETER(pStart);
    GT_UNREFERENCED_PARAMETER(pFrame);
    GT_UNREFERENCED_PARAMETER(fSource);
    GT_UNREFERENCED_PARAMETER(ppEnum);
    GT_UNREFERENCED_PARAMETER(ppSafety);

    return E_NOTIMPL;
}

HRESULT vspCDebugEngine::WriteDump(DUMPTYPE DumpType, LPCOLESTR pszCrashDumpUrl)
{
    GT_UNREFERENCED_PARAMETER(DumpType);
    GT_UNREFERENCED_PARAMETER(pszCrashDumpUrl);

    return E_NOTIMPL;
}

HRESULT vspCDebugEngine::CanDetach(void)
{
    return E_NOTIMPL;
}

HRESULT vspCDebugEngine::GetProgramId(GUID* pguidProgramId)
{
    HRESULT retVal = S_OK;

    if (pguidProgramId != NULL)
    {
        // Return the program (current debug session)'s GUID as received
        // in IDebugEngine2::Attach:
        *pguidProgramId = _programGUID;
    }
    else
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}

HRESULT vspCDebugEngine::Execute(void)
{
    HRESULT retVal = S_OK;

    checkEditAndContinue();

    // Resume the debugged process:
    bool rcRes = gaResumeDebuggedProcess();

    if (!rcRes)
    {
        // We did not manage to resume:
        GT_ASSERT(rcRes);
        retVal = E_FAIL;
    }

    return retVal;
}

HRESULT vspCDebugEngine::Continue(IDebugThread2* pThread)
{
    GT_UNREFERENCED_PARAMETER(pThread);

    HRESULT retVal = S_OK;

    checkEditAndContinue();

    return retVal;
}

HRESULT vspCDebugEngine::Step(IDebugThread2* pThread, STEPKIND sk, STEPUNIT step)
{
    HRESULT retVal = S_OK;
    bool shouldResume = false;

    // We currently don't differentiate the step units, but verify we are getting an expected value:
    GT_ASSERT((step == STEP_STATEMENT) || (step == STEP_LINE) || (step == STEP_INSTRUCTION));

    if (_isCurrentlyKernelDebugging)
    {
        checkEditAndContinue();

        gaUpdateKernelSteppingWorkItemToCurrentCoordinate();
    }

    bool supportHostStep = gaCanGetHostDebugging();

    osThreadId threadId = OS_NO_THREAD_ID;

    if (nullptr != pThread)
    {
        DWORD dwThreadId = 0;
        HRESULT hr = pThread->GetThreadId(&dwThreadId);

        if (SUCCEEDED(hr))
        {
            threadId = (osThreadId)dwThreadId;
        }
    }

    // Interpret the step size:
    switch (sk)
    {
        case STEP_INTO:
        {
            if (_isCurrentlyKernelDebugging)
            {
                // Interpret "step in" as a kernel step:
                // TO_DO: support step in
                bool rcStp = gaSetKernelDebuggingCommand(AP_KERNEL_STEP_IN);
                GT_IF_WITH_ASSERT(rcStp)
                {
                    shouldResume = true;
                }
                else // !rcStp
                {
                    // Let VS know we failed:
                    retVal = E_FAIL;
                }
            }
            else
            {
                // Perform a "step in":
                bool isHostBP = gaIsHostBreakPoint();
                bool canStepInAPI = false;
                gdApplicationCommands* pGDAppCommands = gdApplicationCommands::gdInstance();
                GT_IF_WITH_ASSERT(nullptr != pGDAppCommands)
                {
                    canStepInAPI = pGDAppCommands->canStepIntoCurrentFunction();
                }

                if (supportHostStep && (isHostBP || (!canStepInAPI)))
                {
                    bool rcStp = gaHostDebuggerStepIn(threadId);
                    GT_ASSERT(rcStp);
                    shouldResume = false;

                    if (!rcStp)
                    {
                        retVal = E_FAIL;
                    }
                }
                else // !supportHostStep || !isHostBP
                {
                    bool rcStp = gaBreakInMonitoredFunctionCall();
                    GT_IF_WITH_ASSERT(rcStp)
                    {
                        shouldResume = true;
                    }
                    else // !rcStp
                    {
                        // Let VS know we failed:
                        retVal = E_FAIL;
                    }
                }
            }
        }
        break;

        case STEP_OVER:
        {
            if (_isCurrentlyKernelDebugging)
            {
                // Interpret "step over" as a kernel step:
                bool rcStp = gaSetKernelDebuggingCommand(AP_KERNEL_STEP_OVER);
                GT_IF_WITH_ASSERT(rcStp)
                {
                    shouldResume = true;
                }
                else // !rcStp
                {
                    // Let VS know we failed:
                    retVal = E_FAIL;
                }
            }
            else
            {
                // Interpret "step over" as a single step:
                if (supportHostStep)
                {
                    bool rcStp = gaHostDebuggerStepOver(threadId);
                    GT_ASSERT(rcStp);
                    shouldResume = false;

                    if (!rcStp)
                    {
                        retVal = E_FAIL;
                    }
                }
                else // !supportHostStep
                {
                    bool rcStp = gaBreakOnNextMonitoredFunctionCall();
                    GT_IF_WITH_ASSERT(rcStp)
                    {
                        shouldResume = true;
                    }
                    else // !rcStp
                    {
                        // Let VS know we failed:
                        retVal = E_FAIL;
                    }
                }
            }
        }
        break;

        case STEP_OUT:
        {
            if (_isCurrentlyKernelDebugging)
            {
                // Interpret "step out" as a kernel step:
                // TO_DO: support step out
                bool rcStp = gaSetKernelDebuggingCommand(AP_KERNEL_STEP_OUT);
                GT_IF_WITH_ASSERT(rcStp)
                {
                    shouldResume = true;
                }
                else // !rcStp
                {
                    // Let VS know we failed:
                    retVal = E_FAIL;
                }
            }
            else
            {
                // Interpret "step out" as frame step:
                if (supportHostStep)
                {
                    bool rcStp = gaHostDebuggerStepOut(threadId);
                    GT_ASSERT(rcStp);
                    shouldResume = false;

                    if (!rcStp)
                    {
                        retVal = E_FAIL;
                    }
                }
                else // !supportHostStep
                {
                    bool rcStp = gaBreakOnNextFrame();
                    GT_IF_WITH_ASSERT(rcStp)
                    {
                        shouldResume = true;
                    }
                    else // !rcStp
                    {
                        // Let VS know we failed:
                        retVal = E_FAIL;
                    }
                }
            }
        }
        break;

        case STEP_BACKWARDS:
        {
            // Stepping backwards is not currently supported
            retVal = E_NOTIMPL;
        }
        break;

        default:
        {
            // Unexpected value
            GT_ASSERT(false);
        }
        break;
    }

    // After setting up any step conditions, resume the debugged process run:
    if (shouldResume)
    {
        bool rcRes = gaResumeDebuggedProcess();

        if (!rcRes)
        {
            GT_ASSERT(rcRes);
            retVal = E_FAIL;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugEngine::onProcessTermination
// Description: Called when the process is terminated or the termination command
//              is given. It should be safe to call this funciton multiple times.
// Author:      Uri Shomroni
// Date:        5/4/2016
// ---------------------------------------------------------------------------
void vspCDebugEngine::onProcessTermination()
{
    // Release and clear our process interface:
    if (_piDebuggedProcess != NULL)
    {
        _piDebuggedProcess->Release();
        _piDebuggedProcess = NULL;
    }

    // Notify the debug port of the program node "destruction":
    if (_piDebugPortNotify != NULL)
    {
        HRESULT hr = _piDebugPortNotify->RemoveProgramNode(this);
        GT_ASSERT(hr == S_OK);
        _piDebugPortNotify->Release();
        _piDebugPortNotify = NULL;
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugEngine::onThreadCreated
// Description: Called when a thread is created, to generate the vspCDebugThread
//              object for it.
// Author:      Uri Shomroni
// Date:        20/9/2010
// ---------------------------------------------------------------------------
void vspCDebugEngine::onThreadCreated(osThreadId threadId)
{
    // Verify that we don't already have a representation for this thread:
    vspCDebugThread* pCreatedThread = getThread(threadId);
    GT_IF_WITH_ASSERT(pCreatedThread == NULL)
    {
        // Create the thread object:
        osThreadId mainThreadID = pdProcessDebugger::instance().mainThreadId();
        pCreatedThread = new vspCDebugThread(this, threadId, false, (threadId == mainThreadID));

        // Add it to the vector:
        _debugProcessThreads.push_back(pCreatedThread);
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugEngine::onThreadCreated
// Description: Called when a kernel debugging wavefront is created, to generate the vspCDebugThread
//              object for it.
// Author:      Uri Shomroni
// Date:        7/4/2013
// ---------------------------------------------------------------------------
void vspCDebugEngine::onKernelDebuggingWavefrontCreated(unsigned int wavefrontIndex)
{
    // Verify that we don't already have a representation for this wavefront:
    osThreadId spoofThreadId = DWORD_MAX - wavefrontIndex;
    vspCDebugThread* pCreatedThread = getThread(spoofThreadId);
    GT_IF_WITH_ASSERT(pCreatedThread == NULL)
    {
        osThreadId mainThreadID;

        // Create the thread object:
        mainThreadID = pdProcessDebugger::instance().mainThreadId();
        pCreatedThread = new vspCDebugThread(this, spoofThreadId, true, false);

        // Add it to the vector:
        _debugProcessThreads.push_back(pCreatedThread);
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugEngine::onThreadDestroyed
// Description: Called when a thread is destroyed, to release the vspCDebugThread
//              object for it.
// Author:      Uri Shomroni
// Date:        20/9/2010
// ---------------------------------------------------------------------------
void vspCDebugEngine::onThreadDestroyed(osThreadId threadId)
{
    vspCDebugThread* pDestroyedThread = NULL;

    // Iterate the threads:
    int numberOfThreads = (int)_debugProcessThreads.size();

    for (int i = 0; i < numberOfThreads; i++)
    {
        if (pDestroyedThread == NULL)
        {
            // If we still haven't found the thread, check if the next one is it:
            vspCDebugThread* pCurrentThread = _debugProcessThreads[i];
            GT_IF_WITH_ASSERT(pCurrentThread != NULL)
            {
                // Compare thread IDs:
                if (pCurrentThread->threadId() == threadId)
                {
                    pDestroyedThread = pCurrentThread;
                }
            }
        }
        else // pDestroyedThread != NULL
        {
            // We've already found the thread, move the remaining ones back in the vector.
            // Note that to get here we must have run through the loop at least once, so i > 0,
            // making i-1 a valid index:
            _debugProcessThreads[i - 1] = _debugProcessThreads[i];
        }
    }

    // If we found the thread:
    GT_IF_WITH_ASSERT(pDestroyedThread != NULL)
    {
        // Remove the last item from the vector (it's either a duplicate or the deleted thread):
        _debugProcessThreads.pop_back();

        // If someone is still holding our thread, they should know that it's dead:
        pDestroyedThread->setThreadState(THREADSTATE_DEAD);

        // Release the thread object:
        pDestroyedThread->Release();
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugEngine::onKernelDebuggingEnded
// Description: Called when kernel debugging ends, to release the vspCDebugThread
//              object for all active wavefronts.
// Author:      Uri Shomroni
// Date:        7/4/2010
// ---------------------------------------------------------------------------
void vspCDebugEngine::onKernelDebuggingEnded()
{
    gtVector<vspCDebugThread*> destroyedThreads;

    // Iterate the threads:
    int numberOfThreads = (int)_debugProcessThreads.size();

    for (int i = 0; i < numberOfThreads; i++)
    {
        // We've already found n threads, move the remaining ones back in the vector.
        // Note that to get here we must have run through the loop at least n times, so i > n - 1,
        // making i-n a valid index:
        int numberOfDestroyedThreads = (int)destroyedThreads.size();
        _debugProcessThreads[i - numberOfDestroyedThreads] = _debugProcessThreads[i];

        // If we still haven't found the thread, check if the next one is it:
        vspCDebugThread* pCurrentThread = _debugProcessThreads[i];
        GT_IF_WITH_ASSERT(pCurrentThread != NULL)
        {
            // Compare thread IDs:
            if (pCurrentThread->isKernelDebugging())
            {
                destroyedThreads.push_back(pCurrentThread);
            }
        }
    }

    // For each found thread:
    int numberOfDestroyedThreads = (int)destroyedThreads.size();

    for (int j = 0; j < numberOfDestroyedThreads; j++)
    {
        // Remove the last item from the vector (it's either a duplicate or the deleted thread):
        _debugProcessThreads.pop_back();

        // If someone is still holding our thread, they should know that it's dead:
        destroyedThreads[j]->setThreadState(THREADSTATE_DEAD);

        // Release the thread object:
        destroyedThreads[j]->Release();
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugEngine::getThread
// Description: Returns the IDebugThread2 implementation class for the thread
//              with the given ID, or NULL if it doesn't exist. If the caller
//              means to hold the pointer, they must call AddRef() on the
//              thread object.
// Author:      Uri Shomroni
// Date:        16/9/2010
// ---------------------------------------------------------------------------
vspCDebugThread* vspCDebugEngine::getThread(osThreadId threadId)
{
    vspCDebugThread* retVal = NULL;

    // Search for the thread:
    int numberOfThreads = (int)_debugProcessThreads.size();

    for (int i = 0; i < numberOfThreads; i++)
    {
        vspCDebugThread* pCurrentThread = _debugProcessThreads[i];
        GT_IF_WITH_ASSERT(pCurrentThread != NULL)
        {
            if (pCurrentThread->threadId() == threadId)
            {
                retVal = pCurrentThread;
                break;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugEngine::onModuleLoaded
// Description: Called when a module is loaded, to generate the vspCDebugModule
//              object for it.
// Author:      Uri Shomroni
// Date:        22/9/2010
// ---------------------------------------------------------------------------
void vspCDebugEngine::onModuleLoaded(const osFilePath& moduleFilePath, osInstructionPointer moduleLoadAddress, bool areDebugSymbolsLoaded)
{
    // Verify that we don't already have a representation for this module:
    vspCDebugModule* pLoadedModule = getModule(moduleFilePath);
    GT_IF_WITH_ASSERT(pLoadedModule == NULL)
    {
        // Create the module object:
        pLoadedModule = new vspCDebugModule(moduleFilePath, moduleLoadAddress, areDebugSymbolsLoaded);

        // Add it to the vector:
        _debugProcessModules.push_back(pLoadedModule);
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugEngine::onModuleUnloaded
// Description: Called when a module is unloaded, to release the vspCDebugModule
//              object for it.
// Author:      Uri Shomroni
// Date:        20/9/2010
// ---------------------------------------------------------------------------
void vspCDebugEngine::onModuleUnloaded(const osFilePath& moduleFilePath)
{
    vspCDebugModule* pUnloadedModule = NULL;

    // Iterate the modules:
    int numberOfModules = (int)_debugProcessModules.size();

    for (int i = 0; i < numberOfModules; i++)
    {
        if (pUnloadedModule == NULL)
        {
            // If we still haven't found the module, check if the next one is it:
            vspCDebugModule* pCurrentModule = _debugProcessModules[i];
            GT_IF_WITH_ASSERT(pCurrentModule != NULL)
            {
                // Compare Module paths:
                if (pCurrentModule->moduleFilePath() == moduleFilePath)
                {
                    pUnloadedModule = pCurrentModule;
                }
            }
        }
        else // pUnloadedModule != NULL
        {
            // We've already found the module, move the remaining ones back in the vector.
            // Note that to get here we must have run through the loop at least once, so i > 0,
            // making i-1 a valid index:
            _debugProcessModules[i - 1] = _debugProcessModules[i];
        }
    }

    // If we found the module:
    if (pUnloadedModule != NULL)
    {
        // Remove the last item from the vector (it's either a duplicate or the deleted module):
        _debugProcessModules.pop_back();

        // Release the module object:
        pUnloadedModule->Release();
    }
    else // pUnloadedModule == NULL
    {
        // Print a message to the log:
        gtString dbgMsg = VSP_STR_UnknownModuleUnloaded;
        dbgMsg.append(moduleFilePath.asString());
        OS_OUTPUT_DEBUG_LOG(dbgMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugEngine::getModule
// Description: Returns the IDebugModule2 implementation class for the module
//              with the given Path, or NULL if it doesn't exist. If the caller
//              means to hold the pointer, they must call AddRef() on the
//              module object.
// Author:      Uri Shomroni
// Date:        22/9/2010
// ---------------------------------------------------------------------------
vspCDebugModule* vspCDebugEngine::getModule(const osFilePath& moduleFilePath)
{
    vspCDebugModule* retVal = NULL;

    // Search for the module:
    int numberOfModules = (int)_debugProcessModules.size();

    for (int i = 0; i < numberOfModules; i++)
    {
        vspCDebugModule* pCurrentModule = _debugProcessModules[i];
        GT_IF_WITH_ASSERT(pCurrentModule != NULL)
        {
            if (pCurrentModule->moduleFilePath() == moduleFilePath)
            {
                retVal = pCurrentModule;
                break;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        vspCDebugEngine::checkEditAndContinue
// Description: Check if edit and continue notification is needed
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        13/11/2011
// ---------------------------------------------------------------------------
void vspCDebugEngine::checkEditAndContinue()
{
    GT_IF_WITH_ASSERT(_pOwner != NULL)
    {
        // if any of the opened .cl files were edited notify the user about edit and continue limitation

        if (_pOwner->IsAnyOpenedFileModified())
        {
            acMessageBox::instance().warning(AF_STR_WarningA, VSP_STR_EditAndContinueNotSupported);
        }

        // Clear the list for our next step
        _pOwner->ClearOpenFiles();
    }
}

void vspCDebugEngine::setOwner(IVscDebugEngineOwner* pOwner)
{
    _pOwner = pOwner;
}
