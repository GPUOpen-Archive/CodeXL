//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file contains the entry point for the CLTraceAgent.
//==============================================================================

#include <string.h>
#include <iostream>
#include "CLTraceAgent.h"
#include "../CLCommon/CLFunctionDefs.h"
#include "CLAPITraceEntry.h"
#include "CLEventHandler.h"
#include "CLEventManager.h"
#include "CLAPIInfoManager.h"
#include "PMCSamplerManager.h"
#include "../Common/Logger.h"
#include "../Common/FileUtils.h"
#include "../Common/GlobalSettings.h"
#include "../Common/Version.h"
#include "../Common/OSUtils.h"
#include "../Common/StackTracer.h"

using namespace std;
using namespace GPULogger;

static cl_icd_dispatch_table original_dispatch;
static cl_icd_dispatch_table modified_dispatch;

void DumpTrace()
{
    static bool alreadyDumped = false;

    if (!alreadyDumped)
    {
        alreadyDumped = true;

        if (!CLAPIInfoManager::Instance()->IsTimeOutMode())
        {
            CLAPIInfoManager::Instance()->SaveToOutputFile();
        }

        CLEventManager::Instance()->Release();
        CLAPIInfoManager::Instance()->Release();
    }
}

extern "C" DLL_PUBLIC void amdtCodeXLStopProfiling()
{
    CLAPIInfoManager::Instance()->StopTracing();
}

extern "C" DLL_PUBLIC void amdtCodeXLResumeProfiling()
{
    CLAPIInfoManager::Instance()->ResumeTracing();
}

#ifdef _WIN32
    #include <windows.h>
#endif

#ifdef _WIN32
/// Vectored Exception Handler used to handle crashes in a profiled app during automated tests
/// This exception handler is only installed when the profiler is started in test mode (--__testmode__)
/// This handler just terminates the process (the assumption being that test apps will never raise an OS exception)
/// \param exception_info the exception information structure containing the exception record
/// \return a value indicating what should be done with the exception
LONG CALLBACK TestModeExceptionHandler(PEXCEPTION_POINTERS except_info)
{
    if (except_info->ExceptionRecord->ExceptionCode >= STATUS_ACCESS_VIOLATION)
    {
        TerminateProcess(GetCurrentProcess(), TEST_EXCEPTION_EXIT_CODE);
        return EXCEPTION_CONTINUE_EXECUTION;
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

// On Windows, we can't dump data in OnUnload() because of ocl runtime bug
extern "C" DLL_PUBLIC void OnExitProcess()
{
    DumpTrace();
}

BOOL APIENTRY DllMain(HMODULE,
                      DWORD   ul_reason_for_call,
                      LPVOID)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_PROCESS_DETACH:
        {
            // Dump all data out
            DumpTrace();
            OSUtils::Instance()->ShutdownUserTimer(); //TODO: need to call this on Linux as well
        }
        break;

        case DLL_THREAD_DETACH:
            break;
    }

    return TRUE;
}

#else

void __attribute__((destructor)) libUnload(void)
{
    /*
    Don't call unload() before ocl runtime fixed this bug, anything called inside this function will have unexpected behaviors
    if( CLAPIInfoManager::Instance()->IsTimeOutMode() )
    {
    CLAPIInfoManager::Instance()->StopTimer();
    CLAPIInfoManager::Instance()->FlushTraceData( true );
    CLAPIInfoManager::Instance()->TrySwapBuffer();
    CLAPIInfoManager::Instance()->FlushTraceData( true );
    }
    else
    {
    CLAPIInfoManager::Instance()->SaveAPITraceDataToFile();
    CLAPIInfoManager::Instance()->SaveTimestampToFile();
    }

    CLAPIInfoManager::Instance()->StopTimer();

    CLEventManager::Instance()->Release();
    CLAPIInfoManager::Instance()->Release();
    */
}

#endif

cl_icd_dispatch_table* GetRealDispatchTable()
{
    return  &original_dispatch;
}

#ifdef CL_UNITTEST_MOCK
void SetRealDispatchTblToMock(void)
{
    SetRealDispatchTblToMock(original_dispatch);
}
#endif

// NOTE: In order for stack tracing to work correctly in a release build, clAgent_OnLoad must be the last function exported from CLTraceAgent.dll
// See Windows implementation of CLAPIBase::CreateStackEntry(), which expects clAgent_OnLoad to be in the stack trace
cl_int CL_CALLBACK
clAgent_OnLoad(cl_agent* agent)
{
#ifdef _DEBUG
    FileUtils::CheckForDebuggerAttach();
#endif

    std::cout << "CodeXL GPU Profiler " << GPUPROFILER_BACKEND_VERSION_STRING << " is Enabled\n";

    cl_int err = agent->GetICDDispatchTable(
                     agent, &original_dispatch, sizeof(original_dispatch));

    if (err != CL_SUCCESS)
    {
        return err;
    }

    memcpy(&modified_dispatch, &original_dispatch, sizeof(modified_dispatch));

    InitNextCLFunctions(original_dispatch);

    Parameters params;
    FileUtils::GetParametersFromFile(params);
#ifdef _WIN32

    if (params.m_bTestMode)
    {
        AddVectoredExceptionHandler(1, &TestModeExceptionHandler);
    }

#endif //_WIN32

    if (params.m_bStartDisabled)
    {
        CLAPIInfoManager::Instance()->StopTracing();
    }

    OSUtils::Instance()->SetupUserTimer(params);

    StackTracer::Instance()->InitSymPath();
    CLAPIInfoManager::Instance()->SetOutputFile(params.m_strOutputFile);
    GlobalSettings::GetInstance()->m_params = params;
    SetGlobalTraceFlags(params.m_bQueryRetStat, params.m_bCollapseClGetEventInfo);

    if (!params.m_strAPIFilterFile.empty())
    {
        CLAPIInfoManager::Instance()->LoadAPIFilterFile(params.m_strAPIFilterFile);
    }

    if (err != CL_SUCCESS)
    {
        return err;
    }

    std::string strLogFile = FileUtils::GetDefaultOutputPath() + "cltraceagent.log";
    LogFileInitialize(strLogFile.c_str());

    CreateAPITraceDispatchTable(modified_dispatch);

    err = agent->SetICDDispatchTable(
              agent, &modified_dispatch, sizeof(modified_dispatch));

    // Set Event callback
    cl_agent_callbacks callbacks;
    memset(&callbacks, '\0', sizeof(callbacks));
    CreateCLEventCallbackDispatchTable(callbacks);
    agent->SetCallbacks(agent, &callbacks, sizeof(callbacks));

    cl_agent_capabilities caps;
    memset(&caps, '\0', sizeof(caps));
    caps.canGenerateEventEvents = 1;
    agent->SetCapabilities(agent, &caps, CL_AGENT_ADD_CAPABILITIES);

    if (params.m_bTimeOutBasedOutput)
    {
        CLAPIInfoManager::Instance()->SetInterval(params.m_uiTimeOutInterval);
        CLEventManager::Instance()->SetTimeOutMode(true);

        if (!CLAPIInfoManager::Instance()->StartTimer(TimerThread))
        {
            std::cout << "Failed to initialize CLTraceAgent." << std::endl;
        }
    }

    if (params.m_bUserPMC)
    {
        PMCSamplerManager::Instance()->LoadPMCSamplers(params.m_strUserPMCLibPath.c_str());
    }

    return CL_SUCCESS;
}
