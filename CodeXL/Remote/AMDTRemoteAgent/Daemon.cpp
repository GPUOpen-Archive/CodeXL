//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file Daemon.cpp
///
//==================================================================================

// Daemon.cpp : Defines the entry point for the console application.
//
#include <iostream>
#include <sstream>
#include <algorithm>

#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtAssert.h>

#include <AMDTRemoteAgent/dmnServerThread.h>
#include <AMDTRemoteAgent/dmnServerThreadConfig.h>
#include <AMDTRemoteAgent/dmnThreadObserver.h>
#include <AMDTRemoteAgent/dmnUtils.h>
#include <AMDTRemoteAgent/dmnConfigManager.h>
#include <AMDTRemoteAgent/Public Include/dmnStringConstants.h>

#include <AMDTOSWrappers/Include/osTCPSocketServer.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osConsole.h>
#include <AMDTOSWrappers/Include/osOutOfMemoryHandling.h>

using namespace std;

// Should be read from a config file (or cmd line).
static const unsigned BACKLOG            = 16;


// Initializes the CodeXL Daemon logger.
static void InitDaemonLogger(osDebugLogSeverity initialiSeverity)
{
    osDebugLog& theDebugLog = osDebugLog::instance();

    theDebugLog.initialize(DMN_STR_CODEXL_REMOTE_AGENT, DMN_STR_CODEXL_REMOTE_AGENT_SPACED_W);
    theDebugLog.setLoggedSeverity(initialiSeverity);
}

// Extracts the CodeXL Daemon settings from the config file.
static bool ExtractDaemonSettings(dmnConfigManager* configManager)
{
    bool isOk = false;
    std::wstring errMessage;

    try
    {
        isOk = configManager->Init(errMessage);
    }
    catch (const std::exception& x)
    {
        wcout << errMessage << endl;
        dmnUtils::LogMessage(x.what(), OS_DEBUG_LOG_ERROR);
    }
    catch (...)
    {
        wcout << errMessage << endl;
        dmnUtils::LogMessage(L"Corrupted config file.", OS_DEBUG_LOG_ERROR);
    }

    return isOk;
}

// Terminates CodeXL Daemon with the error message.
static void TerminateDaemon(const std::wstring& errorMsg)
{
    wcout << errorMsg << endl << endl;
    wcout << DMN_STR_PRESS_ANY_KEY << endl;
    osWaitForKeyPress();
    exit(-1);
}

static void ExitWithoutWaitingForUser()
{
    wcout << DMN_STR_USAGE << endl;
    exit(-1);
}

dmnThreadObserver* pThreadObserver = NULL;

void dmnReleaseResources()
{
    // Clean.
    if (pThreadObserver != NULL)
    {
        pThreadObserver->clean();
        delete pThreadObserver;
        pThreadObserver = NULL;
    }
}

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

// This handler handles the various signals that
// a Windows console process may receive from the OS.
BOOL WINAPI ConsoleClosingHandler(DWORD dwCtrlType)
{
    switch (dwCtrlType)
    {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_LOGOFF_EVENT:
        case CTRL_SHUTDOWN_EVENT:
        {
            // Clean.
            dmnReleaseResources();
        }
    }

    // Return false for the OS terminate us and clean after us (if necessary).
    return FALSE;
}

#endif

int main(int argc, char** argv)
{
    // First, register the out-of-memory event handler.
    std::set_new_handler(osDumpCallStackAndExit);

    // A buffer to hold the user ip string (if necessary).
    gtString userIpAddr;

    // A buffer to hold the command line switch (if necessary).
    gtString cmdLineSwitch;

    // Check if the user requested to override the IP address used by the agent.
    if (argc != 1 && argc != 3)
    {
        // This is illegal.
        ExitWithoutWaitingForUser();
    }
    else if (argc == 3)
    {
        cmdLineSwitch.fromASCIIString(argv[1]);

        if (0 != cmdLineSwitch.compare(L"--ip"))
        {
            // This is illegal.
            ExitWithoutWaitingForUser();
        }

        // The user requested to override the IP address used by the agent.
        userIpAddr.fromASCIIString(argv[2]);

        // Input validation.
        static const int MAX_LENGTH = 64;

        if (userIpAddr.length() > MAX_LENGTH)
        {
            // Terminate the agent with an appropriate message.
            gtString errorMessage;
            errorMessage.appendFormattedString(DMN_STR_ERR_ADDRESS_NOT_VALID, userIpAddr.asCharArray());
            TerminateDaemon(errorMessage.asCharArray());
        }
    }

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    // If we are on Windows, set the console control event handler.
    ::SetConsoleCtrlHandler(ConsoleClosingHandler, TRUE);

    // set the current work directory so launched application will get the path of the needed dlls.
    osFilePath appExePath;
    osGetCurrentApplicationPath(appExePath);
    osDirectory appDir;
    appExePath.getFileDirectory(appDir);
    ::SetDllDirectory(appDir.asString().asCharArray());

#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS

    // Set the termination CB.
    std::set_terminate(dmnReleaseResources);

#endif

    // Set the thread naming prefix:
    osThread::setThreadNamingPrefix(DMN_STR_CODEXL_REMOTE_AGENT_SPACED_A);

    // Initialize the logger.
    InitDaemonLogger(OS_DEBUG_LOG_DEBUG);
    dmnUtils::LogMessage(L"CodeXL Remote Agent started running.", OS_DEBUG_LOG_INFO);


    // Initialize the configuration manager.
    dmnConfigManager* configManager = dmnConfigManager::Instance();

    if (configManager == NULL)
    {
        TerminateDaemon(DMN_STR_ERR_MISSING_SETTINGS);
    }

    // Extract the daemon settings.
    bool isOk = ExtractDaemonSettings(configManager);
    GT_ASSERT(isOk);

    if (!isOk)
    {
        TerminateDaemon(DMN_STR_ERR_CONFIGURATION);
    }

    try
    {
        // Create the server configuration object.
        dmnServerThreadConfig* pServerConfig =
            new dmnServerThreadConfig(BACKLOG, configManager->GetPortNumber(),
                                      configManager->GetReadTimeout(), configManager->GetWriteTimeout(), userIpAddr);

        // Create the server thread itself.
        dmnServerThread* pServerThread = new(std::nothrow) dmnServerThread(pServerConfig);

        // Create a thread observer to keep track of all spawned threads.
        pThreadObserver = new dmnThreadObserver();
        pServerThread->registerToThreadCreationEvent(pThreadObserver);
        pThreadObserver->onThreadCreation(pServerThread);

        // Run the server.
        isOk = pServerThread->execute();
        GT_ASSERT(isOk);

    }
    catch (const std::exception& x)
    {
        dmnUtils::HandleException(x);
    }
    catch (...) {}

    // Wait for user interaction on main thread.
    osWaitForKeyPress();

    // Clean.
    dmnReleaseResources();

    return 0;
}

