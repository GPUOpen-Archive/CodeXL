//==============================================================================
/// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file PLayerMain.cpp
/// \brief The main entrypoint for the CapturePlayer application. This small
/// application is responsible for loading a Capture file, and executing
/// playback of all captured calls. The player is able to respond to commands
/// issued by HTTP requests.
///
/// THERE SHOULD NOT BE ANY API-SPECIFIC CODE WITHIN THE PLAYER'S MAIN.
/// Instead, load a Capture file from disk, and execute it. Doing so will load
/// our Server plugins, which we can use to communicate with the player process.
///
//==============================================================================

#include <stdio.h>

#ifdef WIN32
#include "DX12Player.h"
#else
#include <signal.h>
#include "WinDefs.h"
#endif

#include "VulkanPlayer.h"
#include <tinyxml.h>
#include "../Common/StreamLog.h"
#include "../Common/TraceMetadata.h"
#include "../Common/FrameInfo.h"
#include "../Common/Logger.h"

#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osModuleArchitecture.h>
#include <AMDTOSWrappers/Include/osProcess.h>

const UINT windowWidth = 800; ///< Render window width
const UINT windowHeight = 600; ///< Render window height

#ifdef _DEBUG
    #ifdef WIN32
        #define CP_ASSERT(s) if (s == false) { __debugbreak(); }
    #else
        #define CP_ASSERT(s) if (!(s)) { raise(SIGTRAP); }
    #endif
#else
    #define CP_ASSERT(s)
#endif

/// Main entry point
/// \param hInstance A handle to the current instance of the application.
/// \param hPrevInstance A handle to the previous instance of the application.This parameter is always NULL.
/// \param lpCmdLine The command line for the application, excluding the program name.
/// \param nCmdShow Controls how the window is to be shown.
/// \return Returns the exit value contained in message's wParam parameter or 0.
#ifdef WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

#else
int main()
{
#endif

    osModuleArchitecture moduleArchitecture;
    osRuntimePlatform currentPlatform;
    gtString executablePathString;
    gtString commandLine;
    gtString workingDirectory;

    // Get the process information
    bool bRes = osGetProcessLaunchInfo(osGetCurrentProcessId(), moduleArchitecture, currentPlatform, executablePathString, commandLine, workingDirectory);
    CP_ASSERT(bRes == true);
    if (bRes != true)
    {
        Log(logERROR, "Could not get the process information from: osGetProcessLaunchInfo\n");
        exit(EXIT_FAILURE);
    }

    // Create a meta data object to read the XML into
    TraceMetadata mtf;
    FrameInfo frameInfo;
    mtf.mFrameInfo = &frameInfo;

    // Read the XML data
    bRes = ReadMetadataFile(commandLine.asASCIICharArray(), &mtf);
    CP_ASSERT(bRes == true);
    if (bRes != true)
    {
        Log(logERROR, "ReadMetadataFile: FAILED reading %s\n", commandLine.asASCIICharArray());
        exit(EXIT_FAILURE);
    }

    // Declare a player pointer
    BasePlayer* pPlayer = NULL;

#ifdef WIN32
    size_t found = mtf.mAPIString.find("Vulkan");
    if (found != std::string::npos)
    {
        pPlayer = new VulkanPlayer();
    }
    else
    {
        found = mtf.mAPIString.find("DX12");
        if (found != std::string::npos)
        {
            pPlayer = new DX12Player();
        }
        else
        {
            // Default to using DX12 to support older traces that were prior to Vulkan support in GPS/CodeXL 
            pPlayer = new DX12Player();
        }
    }

    // Initialize the render window 
    bRes = pPlayer->InitializeWindow(hInstance, windowWidth, windowHeight);
#else
    // Linux only has vulkan (for now)
    pPlayer = new VulkanPlayer();

    // Initialize the render window 
    bRes = pPlayer->InitializeWindow(nullptr, windowWidth, windowHeight);
#endif

    CP_ASSERT(bRes == true);
    if (bRes != true)
    {
        Log(logERROR, "InitializeWindow FAILED.\n");
        exit(EXIT_FAILURE);
    }

    // Initialize render loop graphics
    bRes = pPlayer->InitializeGraphics();
    CP_ASSERT(bRes == true);
    if (bRes != true)
    {
        Log(logERROR, "InitializeGraphics FAILED.\n");
        exit(EXIT_FAILURE);
    }

    // Spin on the render loop to process commands
    pPlayer->RenderLoop();

    // Cleanup
    pPlayer->Destroy();

    return 0;
}