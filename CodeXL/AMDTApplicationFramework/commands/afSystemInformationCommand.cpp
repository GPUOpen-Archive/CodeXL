//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afSystemInformationCommand.cpp
///
//==================================================================================

// Qt:
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>
#include <AMDTOSWrappers/Include/osMachine.h>
#include <AMDTOSAPIWrappers/Include/oaDisplay.h>
#include <AMDTOSAPIWrappers/Include/oaDriver.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>
#include <AMDTOSAPIWrappers/Include/oaHiddenWindow.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTOSAPIWrappers/Include/oaDeviceContext.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLRenderContext.h>
#include <AMDTOSWrappers/Include/osCpuid.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osPipeSocketServer.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTAPIClasses/Include/apCLDevice.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/commands/afSystemInformationCommand.h>

gtList< gtList <gtString> > afSystemInformationCommand::m_sGraphicCardInfoList;
gtList< gtList <gtString> > afSystemInformationCommand::m_sPixelInfoList;
gtList< gtList <gtString> > afSystemInformationCommand::m_sOpenGLExtInfoList;

// standard C
#include <string.h>
#include <stdio.h>

afSystemInformationCommandThread::afSystemInformationCommandThread() : osThread(L"afSystemInformationCommandThread"), m_isGatheringData(true)
{
    bool rc = oaOpenGLRenderContext::InitHiddenWindow();
    GT_ASSERT(rc);
}

int afSystemInformationCommandThread::entryPoint()
{
    int retVal = 0;

    // Create a system information command:
    afSystemInformationCommand sysInfoCmd;

    // Use try-catch as defense against crashes when collecting system details
    try
    {
        // Collect the system information data and append to a string:
        retVal = (sysInfoCmd.getSystemInformationDataAsString(m_systemInformationStr) ? 1 : 0);

        m_isGatheringData = false;

        // collect all the opengl data in the same thread
        sysInfoCmd.collectGraphicCardInformation();
        sysInfoCmd.collectPixelFormatInformation();
        sysInfoCmd.collectOpenGLExtensionsInformation();

    }
    catch (...)
    {
        OS_OUTPUT_DEBUG_LOG(AF_STR_FAILED_TO_COLLECT_SYSTEM_INFORMATION, OS_DEBUG_LOG_ERROR);
        retVal = false;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afSystemInformationCommand::afSystemInformationCommand
// Description: Constructor
// Author:      Avi Shapira
// Date:        13/11/2003
// ---------------------------------------------------------------------------
afSystemInformationCommand::afSystemInformationCommand()
    : _hOpenCLModule(0), _pclGetPlatformIDs(nullptr), _pclGetPlatformInfo(nullptr), _pclGetDeviceIDs(nullptr), _pclGetDeviceInfo(nullptr)
{
}


// ---------------------------------------------------------------------------
// Name:        afSystemInformationCommand::~afSystemInformationCommand
// Description: Destructor
// Author:      Yaki Tebeka
// Date:        1/8/2004
// ---------------------------------------------------------------------------
afSystemInformationCommand::~afSystemInformationCommand()
{
}


// ---------------------------------------------------------------------------
// Name:        afSystemInformationCommand::canExecuteSpecificCommand
// Description: Will not execute
// Author:      Avi Shapira
// Date:        13/11/2003
// ---------------------------------------------------------------------------
bool afSystemInformationCommand::canExecuteSpecificCommand()
{
    return false;
}


// ---------------------------------------------------------------------------
// Name:        afSystemInformationCommand::executeSpecificCommand
// Description: Will not execute
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        13/11/2003
// ---------------------------------------------------------------------------
bool afSystemInformationCommand::executeSpecificCommand()
{
    return false;
}

// ---------------------------------------------------------------------------
// Name:        afSystemInformationCommand::collectSystemInformation
// Description: Collect the system information for the gdSystemInformationDialog.
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        15/11/2003
// ---------------------------------------------------------------------------
bool afSystemInformationCommand::collectSystemInformation(gtList< gtList <gtString> >& infoData)
{
    gtString osVersionName;
    gtString osVersionIntAsString;
    int majorVersion = 0;
    int minorVersion = 0;
    int buildNumber = 0;

    // Get the OS Version:
    osGetOperatingSystemVersionString(osVersionName);
    osGetOperatingSystemVersionNumber(majorVersion, minorVersion, buildNumber);

    osVersionIntAsString.appendFormattedString(L"%d.%d.%d", majorVersion, minorVersion, buildNumber);

    // Get CPU information:
    gtString numberOfProcessors;
    gtString processorType;

    bool rcCPU = osGetLocalMachineCPUInformationStrings(numberOfProcessors, processorType);
    GT_ASSERT(rcCPU)

    // Get memory information:
    gtString totalRam;
    gtString availRam;
    gtString totalPage;
    gtString availPage;
    gtString totalVirtual;
    gtString availVirtual;

    bool rcMem = osGetLocalMachineMemoryInformationStrings(totalRam, availRam, totalPage, availPage, totalVirtual, availVirtual);
    GT_ASSERT(rcMem);

    // Get the local machine name:
    gtString localMachineName = AF_STR_Unknown;
    osGetLocalMachineName(localMachineName);

    // Get the username and domain
    gtString userName;
    gtString userDomain;
    bool rcUser = osGetLocalMachineUserAndDomain(userName, userDomain);
    GT_ASSERT(rcUser);

    // Get the catalyst driver version:
    int driverError = OA_DRIVER_UNKNOWN;
    gtString driverVersion = oaGetDriverVersion(driverError);

    //////////////////////////////////////////////////////////////////////////
    // Add the item into the list
    //////////////////////////////////////////////////////////////////////////

    gtList <gtString> line;
    /////////////////////////
    //Title of the listCtrl//
    /////////////////////////
    line.push_back(AF_STR_Item);
    line.push_back(AF_STR_Value);
    infoData.push_back(line);

    //Computer Name
    line.clear(); //Clear the list
    line.push_back(AF_STR_SystemInformationCommandComputerName);
    line.push_back(localMachineName);
    infoData.push_back(line);

    //Domain Name
    // In windows we do not display the domain since it has the same value as computer name:
#if (AMDT_BUILD_TARGET == !AMDT_WINDOWS_OS)
    line.clear(); //Clear the list
    line.push_back(AF_STR_SystemInformationCommandDomainName);
    line.push_back(userDomain);
    infoData.push_back(line);
#endif

    //User Name
    line.clear(); //Clear the list
    line.push_back(AF_STR_User);
    line.push_back(userName);
    infoData.push_back(line);

    //OS Version as string
    line.clear(); //Clear the list
    line.push_back(AF_STR_SystemInformationCommandOSVersionName);
    line.push_back(osVersionName);
    infoData.push_back(line);

    //OS Version as number
    line.clear(); //Clear the list
    line.push_back(AF_STR_SystemInformationCommandOSVersionNumber);
    line.push_back(osVersionIntAsString);
    infoData.push_back(line);

    //Number Of Processors
    line.clear(); //Clear the list
    line.push_back(AF_STR_SystemInformationCommandNumberOfCPUCores);
    line.push_back(numberOfProcessors);
    infoData.push_back(line);

    // Processor Type
    line.clear(); //Clear the list
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    line.push_back(AF_STR_SystemInformationCommandSystemType);
#else
    line.push_back(AF_STR_SystemInformationCommandCPUType);
#endif
    line.push_back(processorType);
    infoData.push_back(line);

    // Cpu family and model
    osCpuid cpuInfo;
    gtString cpuFamily;
    cpuFamily.appendFormattedString(L"Family 0x%x, Model 0x%x, Stepping 0x%x",
                                    cpuInfo.getFamily(), cpuInfo.getModel(), cpuInfo.getStepping());
    line.clear(); //Clear the list
    line.push_back(AF_STR_SystemInformationCommandCpuFamily);
    line.push_back(cpuFamily);
    infoData.push_back(line);

    if (driverError != OA_DRIVER_NOT_FOUND)
    {
        // Available Page files
        line.clear(); //Clear the list
        line.push_back(AF_STR_SystemInformationCommandDriverVersion);
        line.push_back(driverVersion);
        infoData.push_back(line);
    }

    // Total physical memory
    line.clear(); //Clear the list
    line.push_back(AF_STR_SystemInformationCommandTotalPhysicalMemory);
    line.push_back(totalRam);
    infoData.push_back(line);

    // Available physical memory
    line.clear(); //Clear the list
    line.push_back(AF_STR_SystemInformationCommandAvailablePhysicalMemory);
    line.push_back(availRam);
    infoData.push_back(line);

    // Total virtual memory
    line.clear(); //Clear the list
    line.push_back(AF_STR_SystemInformationCommandTotalVirtualMemory);
    line.push_back(totalVirtual);
    infoData.push_back(line);

    // Available virtual memory
    line.clear(); //Clear the list
    line.push_back(AF_STR_SystemInformationCommandAvailableVirtualMemory);
    line.push_back(availVirtual);
    infoData.push_back(line);

    // Total page file
    line.clear(); //Clear the list
    line.push_back(AF_STR_SystemInformationCommandTotalPageFiles);
    line.push_back(totalPage);
    infoData.push_back(line);

    // Available Page files
    line.clear(); //Clear the list
    line.push_back(AF_STR_SystemInformationCommandAvailablePageFiles);
    line.push_back(availPage);
    infoData.push_back(line);

    // Get the computer path:
    gtString computerPath;
    gtString computerPathDelims;
    bool rcPath = osGetLocalMachineSystemPathAndDelims(computerPath, computerPathDelims);
    GT_ASSERT(rcPath);
    gtStringTokenizer tokenizer(computerPath, computerPathDelims);

    gtString currentDir;
    bool rcToken = tokenizer.getNextToken(currentDir);

    if (rcToken)
    {
        // Push the first path directory:
        line.clear();
        line.push_back(AF_STR_SystemInformationCommandComputerPath);
        line.push_back(currentDir);
        infoData.push_back(line);

        // Push the rest of the path directories:
        while (tokenizer.getNextToken(currentDir))
        {
            line.clear();
            line.push_back(AF_STR_Empty);

            line.push_back(currentDir);
            infoData.push_back(line);
        }
    }

    return true;
}

// ---------------------------------------------------------------------------
// Name:        afSystemInformationCommand::collectDisplayInformation
// Description: Collect the display information for the gdSystemInformationDialog.
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        16/11/2003
// ---------------------------------------------------------------------------
bool afSystemInformationCommand::collectDisplayInformation(gtList< gtList <gtString> >& infoData)
{
    int numberOfMonitorsInt = 0;
    gtString numberOfMonitors;
    gtString screenResolution;

    // Get the Monitor name
    gtString deviceName;
    gtString monitorName;
    bool rc = oaGetDisplayMonitorInfo(deviceName, monitorName);
    GT_ASSERT(rc);

    // Get the screen resolution
    int screenWidth = QApplication::desktop()->screenGeometry().width();
    int screenHight = QApplication::desktop()->screenGeometry().height();

    screenResolution.appendFormattedString(AF_STR_IntFormat, screenWidth);
    screenResolution.append(L" by ");
    screenResolution.appendFormattedString(AF_STR_IntFormat, screenHight);
    screenResolution.append(L" pixels");

    // Get the Number of displays
    numberOfMonitorsInt = oaGetLocalMachineAmountOfMonitors();
    numberOfMonitors.appendFormattedString(AF_STR_IntFormat, numberOfMonitorsInt);


    //////////////////////////////////////////////////////////////////////////
    // Add the item into the list
    //////////////////////////////////////////////////////////////////////////

    gtList <gtString> line;
    ///////////////////////////
    // Title of the listCtrl //
    ///////////////////////////
    line.push_back(AF_STR_Item);
    line.push_back(AF_STR_Value);
    infoData.push_back(line);

    // Number of monitors
    line.clear(); //Clear the list
    line.push_back(AF_STR_SystemInformationCommandNumberOfMonitors);
    line.push_back(numberOfMonitors);
    infoData.push_back(line);

    // Monitor name
    line.clear(); //Clear the list
    line.push_back(AF_STR_SystemInformationCommandPrimaryMonitorName);
    line.push_back(monitorName);
    infoData.push_back(line);

    // Screen resolution
    line.clear(); //Clear the list
    line.push_back(AF_STR_SystemInformationCommandScreenResolution);
    line.push_back(screenResolution);
    infoData.push_back(line);

    return true;
}


// ---------------------------------------------------------------------------
// Name:        afSystemInformationCommand::collectGraphicCardInformation
// Description: Collect the graphic card information
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        1/8/2004
// ---------------------------------------------------------------------------
bool afSystemInformationCommand::collectGraphicCardInformation()
{
    bool retVal = false;

    // Get the graphic card information:
    gtString vendorString, rendererString, versionString, shadingLangVersionString, rendererType;
    retVal = getGraphicCardDetails(vendorString, rendererString, versionString, shadingLangVersionString, rendererType);

    // Set the columns titles:
    gtString column_1 = AF_STR_Item;
    gtString column_2 = AF_STR_Value;
    gtList <gtString> titleLine;
    titleLine.push_back(column_1);
    titleLine.push_back(column_2);
    m_sGraphicCardInfoList.push_back(titleLine);

    // Set the first line strings:
    gtString item1 = AF_STR_SystemInformationCommandRendererVendor;
    gtList <gtString> firstLine;
    firstLine.push_back(item1);
    firstLine .push_back(vendorString);
    m_sGraphicCardInfoList.push_back(firstLine);

    // Set the second line strings:
    gtString item2 = AF_STR_SystemInformationCommandRendererName;
    gtList <gtString> secondLine;
    secondLine.push_back(item2);
    secondLine.push_back(rendererString);
    m_sGraphicCardInfoList.push_back(secondLine);

    // Set the third line strings:
    gtString item3 = AF_STR_SystemInformationCommandRendererVersion;
    gtList <gtString> thirdLine;
    thirdLine.push_back(item3);
    thirdLine.push_back(versionString);
    m_sGraphicCardInfoList.push_back(thirdLine);

    // Set the third line strings:
    gtString item4 = AF_STR_SystemInformationCommandShadingLanguageVersion;
    gtList <gtString> fourthLine;
    fourthLine.push_back(item4);
    fourthLine.push_back(shadingLangVersionString);
    m_sGraphicCardInfoList.push_back(fourthLine);

    // Set the fourth line strings:
    gtString item5 = AF_STR_SystemInformationCommandRendererType;
    gtList <gtString> fifthLine;
    fifthLine.push_back(item5);
    fifthLine.push_back(rendererType);

    m_sGraphicCardInfoList.push_back(fifthLine);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        afSystemInformationCommand::getGraphicCardDetails
// Description: Return graphic details
// Arguments: gtString& vendorString
//            gtString& rendererString
//            gtString& versionString
//            gtString& shadingLangVersionString
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        6/1/2010
// ---------------------------------------------------------------------------
bool afSystemInformationCommand::getGraphicCardDetails(gtString& vendorString, gtString& rendererString, gtString& versionString, gtString& shadingLangVersionString, gtString& rendererType)
{
    bool retVal = false;
    vendorString = AF_STR_NotAvailable;
    rendererString = AF_STR_NotAvailable;
    versionString = AF_STR_NotAvailable;
    shadingLangVersionString = AF_STR_NotAvailable;
    rendererType = AF_STR_NotAvailable;

    // Use try-catch as defense against crashes when collecting graphic card details - this is reported as causing crashes
    // on several user stations. See https://community.amd.com/thread/187230
    try
    {

        // Get the "default" render context:
        oaOpenGLRenderContext* pDefaultRenderContext = oaOpenGLRenderContext::getDefaultRenderContext();
        GT_IF_WITH_ASSERT(pDefaultRenderContext != nullptr)
        {
            // Make the "default" render context the current context of this thread:
            // Notice: The "default" render context should remain the current context of the application
            //         thread from now on!
            bool rc = pDefaultRenderContext->makeCurrent();
            GT_IF_WITH_ASSERT(rc)
            {
                // Query the current OpenGL implementation:


                // Get the information from the OpenGL default context:
                bool rcVend = pDefaultRenderContext->getOpenGLString(GL_VENDOR, vendorString);
                bool rcRend = pDefaultRenderContext->getOpenGLString(GL_RENDERER, rendererString);
                bool rcVers = pDefaultRenderContext->getOpenGLString(GL_VERSION, versionString);
                retVal = rcVend && rcRend && rcVers;

                bool rcSLan = pDefaultRenderContext->getOpenGLString(GL_SHADING_LANGUAGE_VERSION, shadingLangVersionString);

                // "Soft" assert:
                if (!rcSLan)
                {
                    OS_OUTPUT_DEBUG_LOG(AF_STR_SystemInformationCommandShadingLanguageUpdateError, OS_DEBUG_LOG_DEBUG);
                }

                rc = pDefaultRenderContext->doneCurrent();
                GT_ASSERT(rc);
            }
        }


        // Calculate the maximal hardware acceleration support:
        oaPixelFormat::HardwareSupport maximalHardwareAcceleration = calculateMaximalHadrwareSupport();

        if (maximalHardwareAcceleration == oaPixelFormat::FULL_HARDWARE_ACCELERATION)
        {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
            {
                rendererType = AF_STR_SystemInformationCommandInstallableClient;
            }
#else
            {
                rendererType = AF_STR_SystemInformationCommandHardwareRenderer;
            }
#endif
        }
        else if (maximalHardwareAcceleration == oaPixelFormat::PARTIAL_HARDWARE_ACCELERATION)
        {
            rendererType = AF_STR_SystemInformationCommandMiniClient;
        }
        else
        {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
            {
                rendererType = AF_STR_SystemInformationCommandGenericOpenGLSoftwareRenderer;
            }
#else
            {
                rendererType = AF_STR_SystemInformationCommandSoftwareRenderer;
            }
#endif
        }
    }
    catch (exception& e)
    {
        std::string eStr(e.what());
        gtString logMsgErrorDescription;
        logMsgErrorDescription.fromASCIIString(eStr.c_str(), eStr.length());
        gtString logMsg;
        logMsg.appendFormattedString(AF_STR_FAILED_TO_COLLECT_GPU_DETAILS_AND_EXCEPTION_DESC, logMsgErrorDescription.asCharArray());
        OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
        retVal = false;
    }
    catch (...)
    {
        OS_OUTPUT_DEBUG_LOG(AF_STR_FAILED_TO_COLLECT_GPU_DETAILS, OS_DEBUG_LOG_ERROR);
        retVal = false;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afSystemInformationCommand::collectPixelFormatInformation
// Description: Collect the pixel format information for the gdSystemInformationDialog.
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        16/11/2003
// ---------------------------------------------------------------------------
bool afSystemInformationCommand::collectPixelFormatInformation()
{
#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
    // Pixel formats are allocated and releasable objects in CGL, and cannot be maintained,
    // recreated, or enumerated. Thus, we do not include them here. See also comments on this
    // subject in gdSystemInformationDialog.cpp / .h
#else

    gtList <gtString> line;
    /////////////////////////
    //Title of the listCtrl//
    /////////////////////////
    line.push_back(AF_STR_Id);
    line.push_back(AF_STR_SystemInformationCommandDBuff);
    line.push_back(AF_STR_SystemInformationCommandZBuff);
    line.push_back(AF_STR_SystemInformationCommandStBuff);
    line.push_back(AF_STR_SystemInformationCommandColorBits);
    line.push_back(AF_STR_SystemInformationCommandRBits);
    line.push_back(AF_STR_SystemInformationCommandGBits);
    line.push_back(AF_STR_SystemInformationCommandBBits);
    line.push_back(AF_STR_SystemInformationCommandABits);
    line.push_back(AF_STR_SystemInformationCommandAccBuff);
    line.push_back(AF_STR_OpenGL);
    line.push_back(AF_STR_SystemInformationCommandGDI);
    line.push_back(AF_STR_SystemInformationCommandStereo);
    line.push_back(AF_STR_SystemInformationCommandHardware);
    line.push_back(AF_STR_SystemInformationCommandPixelType);
    m_sPixelInfoList.push_back(line);
    line.clear();

    // Creating a window:
    // Get the "default" render context:
    oaOpenGLRenderContext* pDefaultRenderContext = oaOpenGLRenderContext::getDefaultRenderContext();
    GT_IF_WITH_ASSERT(pDefaultRenderContext != nullptr)
    {
        // Get its containing device context:
        const oaDeviceContext& defaultRenderContextDC = pDefaultRenderContext->getContainingDeviceContext();

        int amountOfPixelFormats = defaultRenderContextDC.amountOfAvailablePixelFormats();

        for (int i = 1; i <= amountOfPixelFormats; i++)
        {
            gtAutoPtr<oaPixelFormat> aptrCurrentPixelFormat;
            int pixelFormatID = defaultRenderContextDC.getPixelFormatIDFromIndex(i);
            bool rc = defaultRenderContextDC.getPixelFormat(pixelFormatID, aptrCurrentPixelFormat);

            if (rc)
            {
                gtString serialNumber, amountOfZBufferBitsString, amountOfStencilBufferBitsString,
                         amountOfColorBitsString, amountOfRedBitsString, amountOfGreenBitsString,
                         amountOfBlueBitsString, amountOfAlphaBitsString, amountOfAccumulationBufferBitsString;

                // Adding the serial number
                serialNumber.appendFormattedString(AF_STR_IntFormat, i);
                line.push_back(serialNumber);

                //Adding the isDoubleBuffered
                bool isDoubleBuffered = aptrCurrentPixelFormat->isDoubleBuffered();

                if (isDoubleBuffered)
                {
                    line.push_back(AF_STR_Yes);
                }
                else
                {
                    line.push_back(AF_STR_No);
                }

                //Adding the amountOfZBufferBits
                int amountOfZBufferBits = aptrCurrentPixelFormat->amountOfZBufferBits();
                amountOfZBufferBitsString.appendFormattedString(AF_STR_IntFormat, amountOfZBufferBits);
                line.push_back(amountOfZBufferBitsString);

                //Adding the amountOfStencilBufferBits
                int amountOfStencilBufferBits = aptrCurrentPixelFormat->amountOfStencilBufferBits();
                amountOfStencilBufferBitsString.appendFormattedString(AF_STR_IntFormat, amountOfStencilBufferBits);
                line.push_back(amountOfStencilBufferBitsString);

                //Adding the amountOfColorBits
                int amountOfColorBits = aptrCurrentPixelFormat->amountOfColorBits();
                amountOfColorBitsString.appendFormattedString(AF_STR_IntFormat, amountOfColorBits);
                line.push_back(amountOfColorBitsString);

                //Adding the amountOfRedBits
                int amountOfRedBits = aptrCurrentPixelFormat->amountOfRedBits();
                amountOfRedBitsString.appendFormattedString(AF_STR_IntFormat, amountOfRedBits);
                line.push_back(amountOfRedBitsString);

                //Adding the amountOfGreenBits
                int amountOfGreenBits = aptrCurrentPixelFormat->amountOfGreenBits();
                amountOfGreenBitsString.appendFormattedString(AF_STR_IntFormat, amountOfGreenBits);
                line.push_back(amountOfGreenBitsString);

                //Adding the amountOfBlueBits
                int amountOfBlueBits = aptrCurrentPixelFormat->amountOfBlueBits();
                amountOfBlueBitsString.appendFormattedString(AF_STR_IntFormat, amountOfBlueBits);
                line.push_back(amountOfBlueBitsString);

                //Adding the amountOfAlphaBits
                int amountOfAlphaBits = aptrCurrentPixelFormat->amountOfAlphaBits();
                amountOfAlphaBitsString.appendFormattedString(AF_STR_IntFormat, amountOfAlphaBits);
                line.push_back(amountOfAlphaBitsString);

                //Adding the amountOfAccumulationBufferBits
                int amountOfAccumulationBufferBits = aptrCurrentPixelFormat->amountOfAccumulationBufferBits();
                amountOfAccumulationBufferBitsString.appendFormattedString(AF_STR_IntFormat, amountOfAccumulationBufferBits);
                line.push_back(amountOfAccumulationBufferBitsString);

                //Adding the supportsOpenGL
                bool supportsOpenGL = aptrCurrentPixelFormat->supportsOpenGL();

                if (supportsOpenGL)
                {
                    line.push_back(AF_STR_Yes);
                }
                else
                {
                    line.push_back(AF_STR_No);
                }

                //Adding the supportsNativeRendering
                bool supportsNativeRendering = aptrCurrentPixelFormat->supportsNativeRendering();

                if (supportsNativeRendering)
                {
                    line.push_back(AF_STR_Yes);
                }
                else
                {
                    line.push_back(AF_STR_No);
                }

                //Adding the isStereoscopic
                bool isStereoscopic = aptrCurrentPixelFormat->isStereoscopic();

                if (isStereoscopic)
                {
                    line.push_back(AF_STR_Yes);
                }
                else
                {
                    line.push_back(AF_STR_No);
                }

                //Adding the hardwareSupport
                oaPixelFormat::HardwareSupport hardwareSupport = aptrCurrentPixelFormat->hardwareSupport();

                if (hardwareSupport == oaPixelFormat::FULL_HARDWARE_ACCELERATION)
                {
                    line.push_back(AF_STR_Full);
                }
                else if (hardwareSupport == oaPixelFormat::PARTIAL_HARDWARE_ACCELERATION)
                {
                    line.push_back(AF_STR_Partial);
                }
                else if (hardwareSupport == oaPixelFormat::NO_HARDWARE_ACCELERATION)
                {
                    line.push_back(AF_STR_None);
                }
                else
                {
                    line.push_back(AF_STR_Unknown);
                }

                //Adding the pixelType
                oaPixelFormat::PixelType pixelType = aptrCurrentPixelFormat->pixelType();

                if (pixelType == oaPixelFormat::COLOR_INDEX)
                {
                    line.push_back(AF_STR_SystemInformationCommandColorIndex);
                }
                else if (pixelType == oaPixelFormat::RGBA)
                {
                    line.push_back(AF_STR_RGBA);
                }
                else
                {
                    line.push_back(AF_STR_Unknown);
                }

                m_sPixelInfoList.push_back(line);
                line.clear();
            }
        }
    }
#endif

    return true;
}


// ---------------------------------------------------------------------------
// Name:        afSystemInformationCommand::collectOpenGLExtensionsInformation
// Description: Collect the OpenGL Extensions information for the gdSystemInformationDialog.
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        16/11/2003
// ---------------------------------------------------------------------------
bool afSystemInformationCommand::collectOpenGLExtensionsInformation()
{
    bool retVal = true;

    // Get a list of the OpenGL and WGL extensions:
    gtList <gtString> extensionsStringList;
    bool rc = getExtensionsList(extensionsStringList);

    if (rc)
    {
        // Sort the extension lexicographically:
        extensionsStringList.sort();

        // Set the column title:
        gtString column_1 = AF_STR_NumberShort;
        gtString column_2 = AF_STR_SystemInformationCommandExtensionName;
        gtList <gtString> titleLine;
        titleLine.push_back(column_1);
        titleLine.push_back(column_2);
        m_sOpenGLExtInfoList.push_back(titleLine);

        // Push the OpenGL extensions:
        int currentExtnsionIndex = 1;
        gtList <gtString>::iterator iter = extensionsStringList.begin();
        gtList <gtString>::iterator endIter = extensionsStringList.end();

        while (iter != endIter)
        {
            column_1.makeEmpty();
            column_1.appendFormattedString(L"%d    ", currentExtnsionIndex);

            column_2 = *iter;

            gtList <gtString> currentLine;
            currentLine.push_back(column_1);
            currentLine.push_back(column_2);
            m_sOpenGLExtInfoList.push_back(currentLine);

            iter++;
            currentExtnsionIndex++;
        }

        retVal = true;
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        afSystemInformationCommand::collectOpenCLPlatformsInformation
// Description: Collects the OpenCL platforms information.
// Arguments:   infoData - Will get the OpenCL platforms information.
//              fullAttributesList - get the full / reduced list of attributes
// Return Val:  bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        18/3/2010
// ---------------------------------------------------------------------------
bool afSystemInformationCommand::collectOpenCLPlatformsInformation(gtList< gtList <gtString> >& infoData, bool fullAttributesList)
{
    bool retVal = false;

    // Build the platform titles and parameter names:
    buildOpenCLPlatformsTitlesAndParameterNames(infoData);

    // Get the platform ids:
    bool rcGetPlatformIds = collectOpenCLPlatformIds();
    retVal = rcGetPlatformIds;

    // Iterate the platforms:
    gtMap<oaCLPlatformID, int>::const_iterator iter = _platformIdToName.begin();

    while (iter != _platformIdToName.end())
    {
        // Collect the platform information:
        int platformId = (*iter).second;
        oaCLPlatformID platformHandle = (*iter).first;
        bool rcPlatformInfo = collectOpenCLSinglePlatformInformation(platformId, platformHandle, infoData);
        GT_ASSERT(rcPlatformInfo);

        retVal = retVal && rcPlatformInfo;
        iter++;
    }

    if (fullAttributesList)
    {
        // Collect the platforms extensions:
        bool rcExtensios = collectOpenCLPlatformsExtensions(infoData);
        retVal = retVal && rcExtensios;
    }

    // Clean up:
    releaseOpenCLFunctionPointers();

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        afSystemInformationCommand::collectOpenCLSinglePlatformInformation
// Description: Collects the information of a single OpenCL platform.
// Arguments:
//  platformID - The platform's index.
//  platformHandle - OpenCL handle for this platform.
//  infoData - Will get the OpenCL platform information.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        18/3/2010
// ---------------------------------------------------------------------------
bool afSystemInformationCommand::collectOpenCLSinglePlatformInformation(int platformID, oaCLPlatformID platformHandle, gtList< gtList <gtString> >& infoData)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pclGetPlatformInfo != nullptr)
    {
        gtList< gtList <gtString> >::iterator iter = infoData.begin();
        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& titleLinesList = (*iter);

            // Push the platform id:
            gtString currPlatformID = AF_STR_Platform;
            currPlatformID.append(AF_STR_Space);
            currPlatformID.appendFormattedString(AF_STR_IntFormat, platformID + 1);
            titleLinesList.push_back(currPlatformID);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            // Get the platform's vendor:
            gtString platformVendor;
            bool rcVendor = platformParamAsString(platformHandle, CL_PLATFORM_VENDOR, platformVendor);
            GT_ASSERT(rcVendor);

            gtList<gtString>& vendorsList = (*iter);
            vendorsList.push_back(platformVendor);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            // Get the platform's name:
            gtString platformName;
            bool rcName = platformParamAsString(platformHandle, CL_PLATFORM_NAME, platformName);
            GT_ASSERT(rcName);

            gtList<gtString>& namessLinesList = (*iter);
            namessLinesList.push_back(platformName);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            // Get the platform's profile:
            gtString platformProfile;
            bool rcProfile = platformParamAsString(platformHandle, CL_PLATFORM_PROFILE, platformProfile);
            GT_ASSERT(rcProfile);

            gtList<gtString>& profilesLinesList = (*iter);
            profilesLinesList.push_back(platformProfile);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            // Get the platform's version:
            gtString platformVersion;
            bool rcVersion = platformParamAsString(platformHandle, CL_PLATFORM_VERSION, platformVersion);
            GT_ASSERT(rcVersion);

            gtList<gtString>& versionsLinesList = (*iter);
            versionsLinesList.push_back(platformVersion);
            iter++;
        }


        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        afSystemInformationCommand::collectOpenCLPlatformsExtensions
// Description: Collects the OpenCL platforms extensions list.
// Arguments:   infoData - Will get the platforms extensions list.
// Return Val:  bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        18/3/2010
// ---------------------------------------------------------------------------
bool afSystemInformationCommand::collectOpenCLPlatformsExtensions(gtList< gtList <gtString> >& infoData)
{
    bool retVal = false;

    // Get the platforms extensions list:
    gtVector< gtVector <gtString> > platformsExtensions;
    bool rcGetExt = getOpenCLPlatformsExtensions(platformsExtensions);
    GT_IF_WITH_ASSERT(rcGetExt)
    {
        retVal = true;

        // Calculate the longest extensions list:
        int longestExtesnionsListSize = 0;
        int platformsAmount = (int)_platformIdToName.size();

        for (int k = 0; k < platformsAmount; k++)
        {
            int currExtensionListSize = platformsExtensions[k].size();

            if (longestExtesnionsListSize < currExtensionListSize)
            {
                longestExtesnionsListSize = currExtensionListSize;
            }
        }

        // Iterate the extensions lists by extensions order:
        for (int i = 0; i < longestExtesnionsListSize; i++)
        {
            // Will hold the current horizontal line of the OpenCL extensions:
            gtList <gtString> extensionsHorizontalLine;

            // Contains first column content:
            gtString firstColumnContent;

            if (i == 0)
            {
                firstColumnContent = AF_STR_SystemInformationCommandExtensions;
            }

            extensionsHorizontalLine.push_back(firstColumnContent);

            // Iterate the platforms:
            for (int j = 0; j < platformsAmount; j++)
            {
                // Will contain the current extension name:
                gtString currExtensionName;

                const gtVector <gtString>& currPlatformExtensions = platformsExtensions[j];

                if (i < (int)currPlatformExtensions.size())
                {
                    currExtensionName = currPlatformExtensions[i];
                }

                extensionsHorizontalLine.push_back(currExtensionName);
            }

            infoData.push_back(extensionsHorizontalLine);
        }
    }


    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        afSystemInformationCommand::collectOpenCLDevicesExtensions
// Description: Collects the OpenCL devices extensions list.
// Arguments:   infoData - Will get the platforms extensions list.
//              devicesList - the list of devices
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        12/4/2010
// ---------------------------------------------------------------------------
bool afSystemInformationCommand::collectOpenCLDevicesExtensions(gtList< gtList <gtString> >& infoData, const gtPtrVector<apCLDevice*>& devicesList)
{
    bool retVal = true;

    // Check what is the longest device extensions list:
    int longestExtesnionsListSize = 0;
    int devicesAmount = (int)devicesList.size();

    for (int k = 0; k < devicesAmount; k++)
    {
        if (devicesList[k] != nullptr)
        {
            // Get the current device extensions list:
            int currExtensionListSize = devicesList[k]->extensions().size();

            if (longestExtesnionsListSize < currExtensionListSize)
            {
                longestExtesnionsListSize = currExtensionListSize;
            }
        }
    }

    // Iterate the extensions lists by extensions order:
    for (int i = 0; i < longestExtesnionsListSize; i++)
    {
        // Will hold the current horizontal line of the OpenCL extensions:
        gtList <gtString> extensionsHorizontalLine;

        // Contains first column content:
        gtString firstColumnContent;

        if (i == 0)
        {
            firstColumnContent = AF_STR_SystemInformationCommandExtensions;
        }

        extensionsHorizontalLine.push_back(firstColumnContent);

        // Iterate the devices:
        for (int j = 0; j < devicesAmount; j++)
        {
            // Will contain the current extension name:
            gtString currExtensionName;

            if (devicesList[j] != nullptr)
            {
                // Get the current device extensions:
                const gtVector <gtString>& currDeviceExtensions = devicesList[j]->extensions();

                if (i < (int)currDeviceExtensions.size())
                {
                    currExtensionName = currDeviceExtensions[i];
                }

                extensionsHorizontalLine.push_back(currExtensionName);
            }
        }

        infoData.push_back(extensionsHorizontalLine);
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        afSystemInformationCommand::getOpenCLPlatformsExtensions
// Description: Retrieves the OpenCL platforms extensions list.
// Arguments:   platformsExtensions - Will get the platforms extensions list.
// Return Val:  bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        18/3/2010
// ---------------------------------------------------------------------------
bool afSystemInformationCommand::getOpenCLPlatformsExtensions(gtVector< gtVector <gtString> >& platformsExtensions)
{
    bool retVal = true;

    gtMap <oaCLPlatformID, int>::const_iterator iter = _platformIdToName.begin();

    while (iter != _platformIdToName.end())
    {
        // Collect the platform information:
        int platformId = (*iter).second;
        GT_ASSERT(platformId > -1);

        oaCLPlatformID platformHandle = (*iter).first;

        // Will get the current platforms extensions list:
        gtVector <gtString> currPlatformExtensions;

        // Get the platform's extensions:
        gtString currPlatformExtensionsString;
        bool rcExtensions = platformParamAsString(platformHandle, CL_PLATFORM_EXTENSIONS, currPlatformExtensionsString);
        GT_IF_WITH_ASSERT(rcExtensions)
        {
            // Parse the extensions string, which are space separated:
            gtString currentExtension;
            gtStringTokenizer strTokenizer(currPlatformExtensionsString, AF_STR_Space);

            while (strTokenizer.getNextToken(currentExtension))
            {
                currPlatformExtensions.push_back(currentExtension);
            }
        }

        platformsExtensions.push_back(currPlatformExtensions);
        iter++;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
bool afSystemInformationCommand::CollectAllOpenCLDevicesInformation(gtList< gtList <gtString> >& infoData)
{
    bool retVal = false;

    gtList< gtList <gtString> > listOf32BitRows;
    gtList< gtList <gtString> > listOf64BitRows;

    // Collect Open CL device information for both 32 & 64 bit
    // collect first the 64 bit since it will work on all machines due to the way it is built
    // then collect the 32 bit that might not work on some machines. doing it the other way will prevent collecting the 64 bit because of timeout in collecting the 32 bit
    bool res64bit = CollectOpenCLDevicesInformation(listOf64BitRows, af64BitApp);
    bool res32bit = CollectOpenCLDevicesInformation(listOf32BitRows, af32BitApp);

    // Merge results to one table
    GT_IF_WITH_ASSERT(res32bit || res64bit)
    {
        // Build merged headings list, original lists should have identical headings, no need to go over both of them
        gtList <gtString> headings;
        // Already verified the infoData32bit has at list one item

        int numOf32BitColumns = 0;
        int numOf64BitColumns = 0;

        // Handle column titles from the 32-bit table
        if (listOf32BitRows.size() > 0)
        {
            gtList <gtString>& originalColumnHeadings = listOf32BitRows.front();
            numOf32BitColumns = originalColumnHeadings.size();

            GT_ASSERT(originalColumnHeadings.size() > 0 && originalColumnHeadings.front() == AF_STR_SystemInformationCommandParameter);
            // Push heading of first column
            headings.push_back(AF_STR_SystemInformationCommandParameter);
            originalColumnHeadings.pop_front();

            // Modify the rest of the column headings to indicate device architecture 32/64-bit
            // For example replace Device1(GPU) with Device1(32-bit, GPU) and Device1(64-bit, GPU)
            while (originalColumnHeadings.size() > 0)
            {
                originalColumnHeadings.front().replace(AF_STR_LeftParenthesis, AF_STR_32HeadingStr, false);
                headings.push_back(originalColumnHeadings.front());
                originalColumnHeadings.pop_front();
            }

            // Remove first row, already handled
            listOf32BitRows.pop_front();
        }

        // Handle column titles from the 64-bit table
        if (listOf64BitRows.size() > 0)
        {
            gtList <gtString>& originalColumnHeadings = listOf64BitRows.front();
            numOf64BitColumns = originalColumnHeadings.size();

            GT_ASSERT(originalColumnHeadings.size() > 0 && originalColumnHeadings.front() == AF_STR_SystemInformationCommandParameter);

            // If we hadn't already pushed this column title in the 32-bit block above
            if (listOf32BitRows.size() == 0)
            {
                // Push heading of first column
                headings.push_back(AF_STR_SystemInformationCommandParameter);
            }

            originalColumnHeadings.pop_front();

            // Modify the rest of the column headings to indicate device architecture 32/64-bit
            // For example replace Device1(GPU) with Device1(32-bit, GPU) and Device1(64-bit, GPU)
            while (originalColumnHeadings.size() > 0)
            {
                originalColumnHeadings.front().replace(AF_STR_LeftParenthesis, AF_STR_64HeadingStr, false);
                headings.push_back(originalColumnHeadings.front());
                originalColumnHeadings.pop_front();
            }

            // Remove first row, already handled
            listOf64BitRows.pop_front();
        }


        // Push the row of column headings into the merged table
        infoData.push_back(headings);

        while (listOf32BitRows.size() > 0 || listOf64BitRows.size() > 0)
        {
            gtList <gtString> mergedRow;

            // insert the first column which contains the row header, i.e the name of the parameter
            if (listOf32BitRows.size() > 0)
            {
                // Push only the first cell from the first row
                mergedRow.push_back(listOf32BitRows.front().front());
                listOf32BitRows.front().pop_front();

                // We don't need the first cell of the 64-bit line anymore because we used the row
                // header from the 32-bit table. So we drop the 64-bit row header
                if (listOf64BitRows.size() > 0)
                {
                    listOf64BitRows.front().pop_front();
                }
            }
            else
            {
                // Push only the first cell from the first row
                mergedRow.push_back(listOf64BitRows.front().front());
                listOf64BitRows.front().pop_front();
            }


            if (listOf32BitRows.size() > 0)
            {
                // Push the rest of the 32-bit row into the merged row
                while (listOf32BitRows.front().size() > 0)
                {
                    mergedRow.push_back(listOf32BitRows.front().front());
                    listOf32BitRows.front().pop_front();
                }

                listOf32BitRows.pop_front();
            }
            else
            {
                // Push empty elements into the merged row as placeholders for the 32-bit
                // items, because we have exhausted the 32-bit table
                for (int i = 0; i < numOf32BitColumns - 1; ++i)
                {
                    mergedRow.push_back(gtString());
                }
            }

            if (listOf64BitRows.size() > 0)
            {
                // Push the rest of the 64-bit row into the merged row
                while (listOf64BitRows.front().size() > 0)
                {
                    mergedRow.push_back(listOf64BitRows.front().front());
                    listOf64BitRows.front().pop_front();
                }

                listOf64BitRows.pop_front();
            }
            else
            {
                // Push empty elements into the merged row as placeholders for the 32-bit
                // items, because we have exhausted the 64-bit table
                for (int i = 0; i < numOf64BitColumns - 1; ++i)
                {
                    mergedRow.push_back(gtString());
                }
            }

            infoData.push_back(mergedRow);
        }

        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"bool retVal = ( (infoData.size=%d  > 0) && (listOf32BitRows.size=%d == 0) && (listOf64BitRows.size=%d == 0))", infoData.size(), listOf32BitRows.size(), listOf64BitRows.size());
        retVal = (infoData.size() > 0 && listOf32BitRows.size() == 0 && listOf64BitRows.size() == 0);

    }

    return retVal;
}

// ---------------------------------------------------------------------------
bool afSystemInformationCommand::CollectOpenCLDevicesInformation(gtList< gtList <gtString> >& infoData, afApplicationType appType)
{
    bool retVal = false;

    // Create socket server:
    gtString pipeName;
    GenerateUniquePipeName(pipeName);

    osPipeSocketServer* pServer = new osPipeSocketServer(pipeName);
    pServer->setReadOperationTimeOut(OS_CHANNEL_DEFAULT_TIME_OUT);
    // Open the socket server side (codeXL side):
    bool rcAPISocket = pServer->open();

    GT_IF_WITH_ASSERT(rcAPISocket)
    {
        // Generate all params needed to launch executable that will collect the data
        gtString fileName;
        osFilePath filePath(osFilePath::OS_CODEXL_BINARIES_PATH);
        osFilePath dirPath(osFilePath::OS_CODEXL_BINARIES_PATH);
        gtString cmdLineArgs = pipeName;
        osProcessId procId;
        osProcessHandle procHandle = 0;
        osThreadHandle procThreadHandle = 0;

        if (af32BitApp == appType)
        {
            fileName = AF_STR_SystemInformationCommandHelper32Name;
        }
        else if (af64BitApp == appType)
        {
            fileName = AF_STR_SystemInformationCommandHelper64Name;
        }
        else
        {
            // Shouldn't get here
            GT_ASSERT(false);
        }

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        fileName.append(GDT_DEBUG_SUFFIX_W GDT_BUILD_SUFFIX_W);
        filePath.setFileExtension(AF_STR_exeFileExtension);
#endif

        filePath.setFileName(fileName);

        GT_IF_WITH_ASSERT(filePath.exists())
        {
            bool rcLaunchProcess = osLaunchSuspendedProcess(filePath, cmdLineArgs, dirPath, procId, procHandle, procThreadHandle, false, false, false);
            GT_IF_WITH_ASSERT(rcLaunchProcess)
            {
                bool rcResumeProcess = osResumeSuspendedProcess(procId, procHandle, procThreadHandle, true);
                bool rcWaitConnection = pServer->waitForClientConnection();

                if (!rcWaitConnection)
                {
                    retVal = false;
                }

                GT_IF_WITH_ASSERT(rcResumeProcess && rcWaitConnection)
                {
                    gtString curStr;
                    int mainListSize;
                    int curListSize;

                    *pServer >> curStr;
                    curStr.toIntNumber(mainListSize);

                    for (int i = 0; i < mainListSize; i++)
                    {
                        *pServer >> curStr;
                        gtList<gtString> curList;
                        curStr.toIntNumber(curListSize);

                        for (int j = 0; j < curListSize; j++)
                        {
                            *pServer >> curStr;
                            curList.push_back(curStr);
                        }

                        infoData.push_back(curList);
                    }

                    retVal = true;
                }

                // verify process is over
                bool isAlive;

                if (osIsProcessAlive(procId, isAlive) && isAlive)
                {
                    osTerminateProcess(procId);
                }
            }
        }
    }
    pServer->close();
    delete pServer;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afSystemInformationCommand::collectOpenCLDevicesInformation
// Description: Collects the OpenCL devices information.
// Arguments:   infoData - Will get the OpenCL devices information.
//              fullAttributesList - fill the full attributes list / fill only reduced list
// Return Val:  bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        11/3/2010
// ---------------------------------------------------------------------------
bool afSystemInformationCommand::CollectOpenCLDevicesInformation(gtList< gtList <gtString> >& infoData, bool fullAttributesList)
{
    bool retVal = false;

    // Get the local machine OpenCL devices list:
    gtPtrVector<apCLDevice*> devicesList;
    bool rcGetDevices = CollectOpenCLDevicesInformation(devicesList, fullAttributesList);

    if (rcGetDevices)
    {
        retVal = true;

        // Build the device titles and para
        buildOpenCLDevicesTitlesAndParameterNames(infoData);

        // Get the platform ids:
        bool rcGetPlatformIds = collectOpenCLPlatformIds();
        GT_ASSERT(rcGetPlatformIds);

        // Iterate the devices and fill the parameters:
        int devicesAmount = devicesList.size();

        for (int i = 0; i < devicesAmount; i++)
        {
            // Get the current device:
            const apCLDevice* pCurrentDevice = devicesList[i];
            GT_IF_WITH_ASSERT(pCurrentDevice != nullptr)
            {
                bool rc = collectOpenCLSingleDeviceInformation(infoData, pCurrentDevice, i, fullAttributesList);
                GT_ASSERT(rc);
            }
        }

        retVal = true;
    }

    if (fullAttributesList)
    {
        // Collect the devices extensions:
        bool rcExtensios = collectOpenCLDevicesExtensions(infoData, devicesList);
        retVal = retVal && rcExtensios;
    }

    // Clean up:
    devicesList.deleteElementsAndClear();

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afSystemInformationCommand::collectOpenCLDevicesInformation
// Description: Collects the OpenCL devices information.
// Arguments:   devicesList - Will get the OpenCL devices information.
//              fullAttributesList - get all the attributes / only a reduced list
// Return Val:  bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        11/3/2010
// ---------------------------------------------------------------------------
bool afSystemInformationCommand::CollectOpenCLDevicesInformation(gtPtrVector<apCLDevice*>& devicesList, bool fullAttributesList)
{
    bool retVal = false;

    bool rcOpenCLFunctions = initOpenCLFunctionPointers();

    if (rcOpenCLFunctions)
    {
        // Get OpenCL platforms count:
        cl_uint amountOfPlatforms = 0;
        cl_uint clRetVal = _pclGetPlatformIDs(0, nullptr, &amountOfPlatforms);
        GT_IF_WITH_ASSERT(clRetVal == CL_SUCCESS)
        {
            retVal = true;

            if (amountOfPlatforms > 0)
            {
                // if there's a platform or more, make space for ID's:
                cl_platform_id* pPlatformIds = new cl_platform_id[amountOfPlatforms];

                // Get the platform ids:
                clRetVal = _pclGetPlatformIDs(amountOfPlatforms, pPlatformIds, nullptr);
                GT_IF_WITH_ASSERT(clRetVal == CL_SUCCESS)
                {
                    for (cl_uint i = 0; i < amountOfPlatforms; ++i)
                    {
                        // Get the current platform id:
                        cl_platform_id currentPlatformId = pPlatformIds[i];

                        // Get devices for this platform:
                        cl_uint amountOfDevices;
                        clRetVal = _pclGetDeviceIDs(currentPlatformId, CL_DEVICE_TYPE_ALL, 0, nullptr, &amountOfDevices);
                        GT_IF_WITH_ASSERT(clRetVal == CL_SUCCESS)
                        {
                            // Allocate a memory for the current platform device ids:
                            cl_device_id* pDevicesIds = new cl_device_id[amountOfDevices];
                            clRetVal = _pclGetDeviceIDs(currentPlatformId, CL_DEVICE_TYPE_ALL, amountOfDevices, pDevicesIds, nullptr);
                            GT_IF_WITH_ASSERT(clRetVal == CL_SUCCESS)
                            {
                                // Iterate the platforms devices:
                                for (int j = 0; j < (int) amountOfDevices; j++)
                                {
                                    // Create a data object for the current device:
                                    gtInt32 deviceAPIID = (gtInt32)devicesList.size();
                                    apCLDevice* pNewDevice = new apCLDevice(deviceAPIID, (oaCLDeviceID)(pDevicesIds[j]));

                                    // Log the device's details:
                                    pNewDevice->initialize(_pclGetDeviceInfo, fullAttributesList);
                                    devicesList.push_back(pNewDevice);
                                }
                            }

                            // Delete the device ids vector:
                            delete[] pDevicesIds;
                        }
                    }
                }
                // Delete the platform ids vector:
                delete[] pPlatformIds;
            }
        }

        // Clean up:
        releaseOpenCLFunctionPointers();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afSystemInformationCommand::getNumberOfOpenCLDevices
// Description: Returns the number of OpenCL devices
// Arguments: int& numberOfDevices
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        6/4/2010
// ---------------------------------------------------------------------------
bool afSystemInformationCommand::getNumberOfOpenCLDevices(int& numberOfDevices)
{
    bool retVal = false;

    int totalNumberOfDevices = 0;


    bool rcOpenCLFunctions = initOpenCLFunctionPointers();

    if (rcOpenCLFunctions)
    {
        // Get OpenCL platforms count:
        cl_uint amountOfPlatforms = 0;
        cl_uint clRetVal = _pclGetPlatformIDs(0, nullptr, &amountOfPlatforms);
        GT_IF_WITH_ASSERT(clRetVal == CL_SUCCESS)
        {
            retVal = (amountOfPlatforms == 0);

            if (amountOfPlatforms > 0)
            {
                // if there's a platform or more, make space for ID's:
                cl_platform_id* pPlatformIds = new cl_platform_id[amountOfPlatforms];

                // Get the platform ids:
                clRetVal = _pclGetPlatformIDs(amountOfPlatforms, pPlatformIds, nullptr);
                GT_IF_WITH_ASSERT(clRetVal == CL_SUCCESS)
                {
                    retVal = true;

                    for (cl_uint i = 0; i < amountOfPlatforms; ++i)
                    {
                        bool rcPlat = false;

                        // Get the current platform id:
                        cl_platform_id currentPlatformId = pPlatformIds[i];

                        // Get devices for this platform:
                        cl_uint amountOfDevices;
                        clRetVal = _pclGetDeviceIDs(currentPlatformId, CL_DEVICE_TYPE_ALL, 0, nullptr, &amountOfDevices);
                        GT_IF_WITH_ASSERT(clRetVal == CL_SUCCESS)
                        {
                            totalNumberOfDevices += (int)amountOfDevices;
                            rcPlat = true;
                        }

                        retVal = retVal && rcPlat;
                    }
                }
                // Delete the platform ids vector:
                delete[] pPlatformIds;
            }
        }

        // Clean up:
        releaseOpenCLFunctionPointers();
    }

    if (retVal)
    {
        numberOfDevices = totalNumberOfDevices;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afSystemInformationCommand::getExtensionsList
// Description: Returns a list of OpenGL and WGL extensions.
// Arguments:   extensionsStringList - Output parameter. Will get the list of
//                                     extension strings.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        22/8/2004
// ---------------------------------------------------------------------------
bool afSystemInformationCommand::getExtensionsList(gtList <gtString>& extensionsStringList)
{
    bool retVal = false;

    // Get the "default" render context:
    oaOpenGLRenderContext* pDefaultRenderContext = oaOpenGLRenderContext::getDefaultRenderContext();
    GT_IF_WITH_ASSERT(pDefaultRenderContext != nullptr)
    {
        // Make the "default" render context the current context of this thread:
        // Notice: The "default" render context should remain the current context of the application
        //         thread from now on!
        bool rc = pDefaultRenderContext->makeCurrent();
        GT_IF_WITH_ASSERT(rc)
        {
            // Query OpenGL extensions list:
            // ----------------------------
            gtString extensionsString;
            bool rcExt = pDefaultRenderContext->extensionsString(extensionsString);
            GT_ASSERT(rcExt);

            // The extension strings are space separated:
            gtStringTokenizer strTokenizer(extensionsString, AF_STR_Space);
            gtString currentExtension;

            // Iterate over the extension strings, and push them into the output list:
            while (strTokenizer.getNextToken(currentExtension))
            {
                extensionsStringList.push_back(currentExtension);
            }

            // Query WGL / GLX extensions list:
            // --------------------------
            gtString platformSpecificExtensions;
            bool rcPlatform = pDefaultRenderContext->platformSpecificExtensionsString(platformSpecificExtensions);

            // We do not assert as in Windows the function for getting this information is an extension function itsself
            // and in Mac there are no extensions at all.
            if (rcPlatform)
            {
                // Add them to the list
                gtStringTokenizer platformSpecificStrTokenizer(platformSpecificExtensions, AF_STR_Space);
                gtString currentPlatformSpecificExtension;

                // Iterate over the extension strings, and push them into the output list:
                while (platformSpecificStrTokenizer.getNextToken(currentPlatformSpecificExtension))
                {
                    extensionsStringList.push_back(currentPlatformSpecificExtension);
                }
            }

            rc = pDefaultRenderContext->doneCurrent();
            GT_ASSERT(rc);

            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        afSystemInformationCommand::calculateMaximalHadrwareSupport
// Description: Calculates and returns the maximal hardware acceleration support
//              that the  current machine offers.
//
// Author:      Yaki Tebeka
// Date:        13/7/2006
// Implementation notes:
//   We relay on the fact that the "default" render context always uses the
//   maximal hardware acceleration support.
// ---------------------------------------------------------------------------
oaPixelFormat::HardwareSupport afSystemInformationCommand::calculateMaximalHadrwareSupport() const
{
    oaPixelFormat::HardwareSupport retVal = oaPixelFormat::NO_HARDWARE_ACCELERATION;

    // Get the "default" render context:
    oaOpenGLRenderContext* pDefaultRenderContext = oaOpenGLRenderContext::getDefaultRenderContext();
    GT_IF_WITH_ASSERT(pDefaultRenderContext != nullptr)
    {
        // Get its containing device context:
        const oaDeviceContext& defaultRenderContextDC = pDefaultRenderContext->getContainingDeviceContext();
        const oaHiddenWindow* defaultRenderContextWindow = defaultRenderContextDC.getRelatedWindow();

        if (defaultRenderContextWindow != nullptr)
        {
            // Get its active pixel format (see "Implementation notes" above):
            oaPixelFormatId activePixelFormatId = OA_NO_PIXEL_FORMAT_ID;
            bool rc = defaultRenderContextWindow->getActivePixelFormat(activePixelFormatId);
            GT_IF_WITH_ASSERT(rc && (activePixelFormatId != OA_NO_PIXEL_FORMAT_ID))
            {
                // Get its wrapper class:
                gtAutoPtr<oaPixelFormat> activePixelFormat;
                rc = defaultRenderContextDC.getPixelFormat(activePixelFormatId, activePixelFormat);
                GT_IF_WITH_ASSERT(rc)
                {
                    // Get its hardware acceleration support:
                    retVal = activePixelFormat->hardwareSupport();
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        afSystemInformationCommand::initOpenCLFunctionPointers
// Description: Loads the OpenCL module and retrieves the needed OpenCL function pointers.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        11/3/2010
// ---------------------------------------------------------------------------
bool afSystemInformationCommand::initOpenCLFunctionPointers()
{
    bool retVal = false;

    // Get the system OpenCL module path:
    gtVector<osFilePath> systemOCLModulePaths;
    bool rcOCLModule = osGetSystemOpenCLModulePath(systemOCLModulePaths);

    if (rcOCLModule)
    {
        // Load the OpenCL module:
        bool rcOCLModuleLoad = false;
        _hOpenCLModule = OS_NO_MODULE_HANDLE;
        int numberOfPaths = (int)systemOCLModulePaths.size();

        for (int i = 0; (i < numberOfPaths) && (!rcOCLModuleLoad); i++)
        {
            rcOCLModuleLoad = osLoadModule(systemOCLModulePaths[i], _hOpenCLModule, nullptr, false);
        }

        if (rcOCLModuleLoad)
        {
            osProcedureAddress pFunctionHandler = nullptr;
            bool rcGetPlatformIDs = osGetProcedureAddress(_hOpenCLModule, "clGetPlatformIDs", pFunctionHandler);
            GT_IF_WITH_ASSERT(rcGetPlatformIDs)
            {
                _pclGetPlatformIDs = (PFNCLGETPLATFORMIDSPROC)pFunctionHandler;
            }

            bool rcGetPlatformInfo = osGetProcedureAddress(_hOpenCLModule, "clGetPlatformInfo", pFunctionHandler);
            GT_IF_WITH_ASSERT(rcGetPlatformInfo)
            {
                _pclGetPlatformInfo = (PFNCLGETPLATFORMINFOSPROC)pFunctionHandler;
            }

            bool rcGetDeviceIds = osGetProcedureAddress(_hOpenCLModule, "clGetDeviceIDs", pFunctionHandler);
            GT_IF_WITH_ASSERT(rcGetDeviceIds)
            {
                _pclGetDeviceIDs = (PFNCLGETDEVICEIDSPROC)pFunctionHandler;
            }

            bool rcGetDeviceInfo = osGetProcedureAddress(_hOpenCLModule, "clGetDeviceInfo", pFunctionHandler);
            GT_IF_WITH_ASSERT(rcGetDeviceInfo)
            {
                _pclGetDeviceInfo = (PFNCLGETDEVICEINFOPROC)pFunctionHandler;
            }

            retVal = ((_pclGetPlatformIDs != nullptr) && (_pclGetDeviceIDs != nullptr) && (_pclGetDeviceInfo != nullptr));
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        afSystemInformationCommand::releaseOpenCLFunctionPointers
// Description: Releases the OpenCL module and terminated it's function pointers.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        11/3/2010
// ---------------------------------------------------------------------------
bool afSystemInformationCommand::releaseOpenCLFunctionPointers()
{
    bool retVal = true;

    if ((_pclGetPlatformIDs != nullptr) || (_pclGetPlatformInfo != nullptr)
        || (_pclGetDeviceIDs != nullptr) || (_pclGetDeviceInfo != nullptr))
    {
        retVal = false;

        // Terminate the OpenCL function's pointers:
        _pclGetPlatformIDs = nullptr;
        _pclGetPlatformInfo = nullptr;
        _pclGetDeviceIDs = nullptr;
        _pclGetDeviceInfo = nullptr;

        // Release the OpenCL Module:
        bool rcReleaseModule = osReleaseModule(_hOpenCLModule);
        GT_IF_WITH_ASSERT(rcReleaseModule)
        {
            retVal = true;
        }

    }

    _hOpenCLModule = 0;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        afSystemInformationCommand::collectOpenCLSingleDeviceInformation
// Description: Collect a single device information
// Arguments:   gtList< gtList <gtString> >& infoData
//            const apCLDevice* pDevice
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        17/3/2010
// ---------------------------------------------------------------------------
bool afSystemInformationCommand::collectOpenCLSingleDeviceInformation(gtList< gtList <gtString> >& infoData, const apCLDevice* pDevice, int deviceIndex, bool fullAttributesList)
{
    bool retVal = true;
    gtList< gtList <gtString> >::iterator iter = infoData.begin();

    GT_IF_WITH_ASSERT(iter != infoData.end())
    {
        gtList<gtString>& currentList = (*iter);
        gtString currDeviceId = AF_STR_Device;
        currDeviceId.append(AF_STR_Space);
        currDeviceId.appendFormattedString(AF_STR_IntFormat, deviceIndex + 1);

        // Add the device type to the name:
        gtString currDeviceTypeAsStr;
        apCLDeviceTypeAsString(pDevice->deviceType(), currDeviceTypeAsStr);
        currDeviceId.appendFormattedString(L" (%ls)", currDeviceTypeAsStr.asCharArray());

        currentList.push_back(currDeviceId);
        iter++;
    }

    GT_IF_WITH_ASSERT(iter != infoData.end())
    {
        gtList<gtString>& currentList = (*iter);
        gtString attrStr;
        int platformName = 0;

        // Try to find this platform in the map:
        gtMap<oaCLPlatformID, int>::const_iterator iterFind = _platformIdToName.find(pDevice->platformID());
        GT_IF_WITH_ASSERT(iterFind != _platformIdToName.end())
        {
            platformName = (*iterFind).second;
        }
        attrStr.appendFormattedString(AF_STR_IntFormat, platformName + 1);
        currentList.push_back(attrStr);
        iter++;
    }

    GT_IF_WITH_ASSERT(iter != infoData.end())
    {
        gtList<gtString>& currentList = (*iter);
        gtString currDeviceTypeAsStr;
        apCLDeviceTypeAsString(pDevice->deviceType(), currDeviceTypeAsStr);
        currentList.push_back(currDeviceTypeAsStr);
        iter++;
    }

    GT_IF_WITH_ASSERT(iter != infoData.end())
    {
        gtList<gtString>& currentList = (*iter);
        const gtString& currDeviceName = pDevice->deviceNameForDisplay();
        currentList.push_back(currDeviceName);
        iter++;
    }

    GT_IF_WITH_ASSERT(iter != infoData.end())
    {
        gtList<gtString>& currentList = (*iter);
        const gtString& currDeviceVendor = pDevice->deviceVendor();
        currentList.push_back(currDeviceVendor);
        iter++;
    }

    if (fullAttributesList)
    {
        // Collect the device extended information:
        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            cl_command_queue_properties queueProperties = pDevice->deviceQueueProperties();
            gtString queuePropertiesAsStr;
            apCLDeviceQueuePropertiesAsString(queueProperties, queuePropertiesAsStr);
            currentList.push_back(queuePropertiesAsStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtUInt64 maxGlobVarSize = pDevice->deviceMaxGlobalVariableSize();
            gtString maxGlobVarSizeAsString;
            maxGlobVarSizeAsString.fromMemorySize(maxGlobVarSize);
            currentList.push_back(maxGlobVarSizeAsString);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtUInt64 globVarPrefTotSz = pDevice->deviceGlobalVariablePreferredTotalSize();
            gtString globVarPrefTotSzAsString;
            globVarPrefTotSzAsString.fromMemorySize((gtUInt64)globVarPrefTotSz);
            currentList.push_back(globVarPrefTotSzAsString);
            iter++;
        }
        // Collect the device extended information:
        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            cl_command_queue_properties queueOnDevProperties = pDevice->queueOnDeviceProperties();
            gtString queueOnDevPropertiesAsStr;
            apCLDeviceQueuePropertiesAsString(queueOnDevProperties, queueOnDevPropertiesAsStr);
            currentList.push_back(queueOnDevPropertiesAsStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtUInt32 qodPrefSize = pDevice->queueOnDevicePreferredSize();
            gtString qodPrefSizeAsString;
            qodPrefSizeAsString.fromMemorySize((gtUInt64)qodPrefSize);
            currentList.push_back(qodPrefSizeAsString);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtUInt32 qodMaxSize = pDevice->queueOnDeviceMaxSize();
            gtString qodMaxSizeAsString;
            qodMaxSizeAsString.fromMemorySize((gtUInt64)qodMaxSize);
            currentList.push_back(qodMaxSizeAsString);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString maxQODsAsString;
            maxQODsAsString.appendFormattedString(AF_STR_UIntFormat, pDevice->deviceMaxOnDeviceQueues());
            currentList.push_back(maxQODsAsString);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString maxQODEventsAsString;
            maxQODEventsAsString.appendFormattedString(AF_STR_UIntFormat, pDevice->deviceMaxOnDeviceEvents());
            currentList.push_back(maxQODEventsAsString);
            iter++;
        }


        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            cl_device_svm_capabilities svmCapabilities = pDevice->deviceSVMCapabilities();
            gtString svmCapabilitiesAsStr;
            apCLDeviceSVMCapabilitiesAsString(svmCapabilities, svmCapabilitiesAsStr);
            currentList.push_back(svmCapabilitiesAsStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString maxPipeArgsAsString;
            maxPipeArgsAsString.appendFormattedString(AF_STR_UIntFormat, pDevice->deviceMaxPipeArgs());
            currentList.push_back(maxPipeArgsAsString);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString maxPipeActiveReservAsString;
            maxPipeActiveReservAsString.appendFormattedString(AF_STR_UIntFormat, pDevice->devicePipeMaxActiveReservations());
            currentList.push_back(maxPipeActiveReservAsString);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtUInt32 maxPipePacketSz = pDevice->devicePipeMaxPacketSize();
            gtString maxPipePacketSzAsString;
            maxPipePacketSzAsString.fromMemorySize((gtUInt64)maxPipePacketSz);
            currentList.push_back(maxPipePacketSzAsString);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString prefPlatAtomicAlignAsString;
            prefPlatAtomicAlignAsString.appendFormattedString(AF_STR_UIntFormat, pDevice->devicePreferredPlatformAtomicAlignment());
            currentList.push_back(prefPlatAtomicAlignAsString);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString prefGlobAtomicAlignAsString;
            prefGlobAtomicAlignAsString.appendFormattedString(AF_STR_UIntFormat, pDevice->devicePreferredGlobalAtomicAlignment());
            currentList.push_back(prefGlobAtomicAlignAsString);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString prefLocAtomicAlignAsString;
            prefLocAtomicAlignAsString.appendFormattedString(AF_STR_UIntFormat, pDevice->devicePreferredLocalAtomicAlignment());
            currentList.push_back(prefLocAtomicAlignAsString);
            iter++;
        }

        // Gilad: Hidden information until we solve bug 7159
        //GT_IF_WITH_ASSERT (iter != infoData.end())
        //{
        //  gtList<gtString>& currentList = (*iter);
        //  gtString bitsAsStr;
        //  bitsAsStr.appendFormattedString(AF_STR_IntFormat, pDevice->addressBits());
        //  currentList.push_back(bitsAsStr);
        //  iter++;
        //  retVal = true;
        //}

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString availbleStr = (pDevice->isAvailable()) ?  AF_STR_Yes : AF_STR_No;;
            currentList.push_back(availbleStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString availbleStr = (pDevice->isCompilerAvailable()) ?  AF_STR_Yes : AF_STR_No;;
            currentList.push_back(availbleStr);
            iter++;
        }

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString deviceFPConfigAsStr;
            apCLFPConfigAsString(pDevice->deviceSingleFPConfig(), deviceFPConfigAsStr);
            currentList.push_back(deviceFPConfigAsStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString deviceFPConfigAsStr;
            apCLFPConfigAsString(pDevice->deviceDoubleFPConfig(), deviceFPConfigAsStr);
            currentList.push_back(deviceFPConfigAsStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString deviceFPConfigAsStr;
            apCLFPConfigAsString(pDevice->deviceHalfFPConfig(), deviceFPConfigAsStr);
            currentList.push_back(deviceFPConfigAsStr);
            iter++;
        }
#endif

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString propertyStr = (pDevice->isEndianLittle()) ?  AF_STR_Yes : AF_STR_No;;
            currentList.push_back(propertyStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString propertyStr = (pDevice->isDeviceErrorCorrectionSupport()) ?  AF_STR_Yes : AF_STR_No;;
            currentList.push_back(propertyStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrAsStr;
            apCLExecutionCapabilitiesAsString(pDevice->deviceExecutionCapabilities(), attrAsStr);
            currentList.push_back(attrAsStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.fromMemorySize(pDevice->globalMemCacheSize());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrAsStr;
            apCLMemoryCacheTypeAsString(pDevice->memoryCacheType(), attrAsStr);
            currentList.push_back(attrAsStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.fromMemorySize((gtUInt64)pDevice->globalMemCacheLineSize());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.fromMemorySize(pDevice->globalMemSize());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString propertyStr = AF_STR_NotAvailable;

            if (pDevice->clMinorVersion() >= 1)
            {
                propertyStr = (pDevice->isHostUnifiedMemory()) ?  AF_STR_Yes : AF_STR_No;;
            }

            currentList.push_back(propertyStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString propertyStr = (pDevice->isImageSupport()) ?  AF_STR_Yes : AF_STR_No;;
            currentList.push_back(propertyStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtUInt32 image2Ddims[2];
            pDevice->maxImage2DDimension(image2Ddims[0], image2Ddims[1]);

            gtString attrStr;
            attrStr.appendFormattedString(L"(%uw, %uh)", image2Ddims[0], image2Ddims[1]);
            currentList.push_back(attrStr);

            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtUInt32 image3Ddims[3];
            pDevice->maxImage3DDimension(image3Ddims[0], image3Ddims[1], image3Ddims[2]);

            gtString attrStr;
            attrStr.appendFormattedString(L"(%uw, %uh, %ud)", image3Ddims[0], image3Ddims[1], image3Ddims[2]);
            currentList.push_back(attrStr);

            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.fromMemorySize(pDevice->localMemSize());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrAsStr;
            apCLLocalMemTypeAsString(pDevice->localMemType(), attrAsStr);
            currentList.push_back(attrAsStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->maxClockFrequency());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->maxComputeUnits());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->maxConstantArgs());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.fromMemorySize(pDevice->maxConstantBufferSize());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.fromMemorySize(pDevice->maxMemAllocSize());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.fromMemorySize((gtUInt64)pDevice->maxParamSize());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->maxReadImageArgs());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->maxSamplers());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.appendFormattedString(AF_STR_UInt64Format, (gtUInt64)pDevice->maxWorkgroupSize());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->maxWorkItemDimensions());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.append(AF_STR_LeftParenthesis);
            gtUInt32 amountOfDims = pDevice->maxWorkItemDimensions();
            gtUInt32* pMaxWorkItems = pDevice->maxWorkItemSizes();

            for (gtUInt32 i = 0; i < amountOfDims; i++)
            {
                gtUInt32 maxWorkItemSize = pMaxWorkItems[i];
                attrStr.appendFormattedString(AF_STR_UIntFormat, maxWorkItemSize);

                if (i < (amountOfDims - 1))
                {
                    attrStr.appendFormattedString(AF_STR_Comma);
                }
            }

            attrStr.append(AF_STR_RightParenthesis);
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            gtUInt32 maxImages = pDevice->maxWriteImageArgs();

            if (0 < maxImages)
            {
                attrStr.appendFormattedString(AF_STR_UIntFormat, maxImages);
            }
            else
            {
                attrStr = AF_STR_NotAvailable;
            }

            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            gtUInt32 maxImages = pDevice->maxReadWriteImageArgs();

            if (0 < maxImages)
            {
                attrStr.appendFormattedString(AF_STR_UIntFormat, maxImages);
            }
            else
            {
                attrStr = AF_STR_NotAvailable;
            }

            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->memBaseAddrAlign());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.fromMemorySize((gtUInt64)pDevice->minDataTypeAlignSize());
            currentList.push_back(attrStr);
            iter++;
        }

        // Device OpenCL C version:
        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString propertyStr = AF_STR_NotAvailable;

            if (pDevice->clMinorVersion() >= 1)
            {
                propertyStr = pDevice->deviceOpenCLCVersion();
            }

            currentList.push_back(propertyStr);
            iter++;
        }

        // Native vector width size for built-in scalar types:
        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr = AF_STR_NotAvailable;

            if (pDevice->clMinorVersion() >= 1)
            {
                attrStr.makeEmpty();
                attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->nativeVecWidthChar());
            }

            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr = AF_STR_NotAvailable;

            if (pDevice->clMinorVersion() >= 1)
            {
                attrStr.makeEmpty();
                attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->nativeVecWidthShort());
            }

            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr = AF_STR_NotAvailable;

            if (pDevice->clMinorVersion() >= 1)
            {
                attrStr.makeEmpty();
                attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->nativeVecWidthInt());
            }

            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr = AF_STR_NotAvailable;

            if (pDevice->clMinorVersion() >= 1)
            {
                attrStr.makeEmpty();
                attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->nativeVecWidthLong());
            }

            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr = AF_STR_NotAvailable;

            if (pDevice->clMinorVersion() >= 1)
            {
                attrStr.makeEmpty();
                attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->nativeVecWidthFloat());
            }

            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr = AF_STR_NotAvailable;

            if (pDevice->clMinorVersion() >= 1)
            {
                attrStr.makeEmpty();
                attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->nativeVecWidthDouble());
            }

            currentList.push_back(attrStr);
            iter++;
        }

        // Preferred native vector width size for built-in scalar types:
        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr = AF_STR_NotAvailable;

            if (pDevice->clMinorVersion() >= 1)
            {
                attrStr.makeEmpty();
                attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->nativeVecWidthHalf());
            }

            currentList.push_back(attrStr);
            iter++;
        }

        // Preferred native vector width size for built-in scalar types:
        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->preferredVecWidthChar());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->preferredVecWidthShort());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->preferredVecWidthInt());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->preferredVecWidthLong());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->preferredVecWidthFloat());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->preferredVecWidthDouble());
            currentList.push_back(attrStr);
            iter++;
        }

        // Preferred native vector width size for built-in scalar types:
        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr = AF_STR_NotAvailable;

            if (pDevice->clMinorVersion() >= 1)
            {
                attrStr.makeEmpty();
                attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->preferredVecWidthHalf());
            }

            currentList.push_back(attrStr);
            iter++;
        }

        // Device profile:
        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            currentList.push_back(pDevice->profileStr());
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.appendFormattedString(AF_STR_UInt64Format, (gtUInt64)pDevice->profilingTimerResolution());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            currentList.push_back(pDevice->deviceVersion());
            iter++;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        afSystemInformationCommand::buildOpenCLPlatformsTitlesAndParameterNames
// Description: Builds the OpenCL platforms titles, and list of attributes
// Arguments: infoData - Will get the titles and attributes.
// Author:      Yaki Tebeka
// Date:        18/3/2010
// ---------------------------------------------------------------------------
void afSystemInformationCommand::buildOpenCLPlatformsTitlesAndParameterNames(gtList< gtList <gtString> >& infoData)
{
    // Will contain the devices parameters:
    gtList <gtString> titleLine;
    titleLine.push_back(AF_STR_SystemInformationCommandParameter);
    infoData.push_back(titleLine);

    gtList <gtString> devicesVendors;
    devicesVendors.push_back(AF_STR_SystemInformationCommandVendor);
    infoData.push_back(devicesVendors);

    gtList <gtString> devicesNames;
    devicesNames.push_back(AF_STR_Name);
    infoData.push_back(devicesNames);

    gtList <gtString> devicesProfiles;
    devicesProfiles.push_back(AF_STR_SystemInformationCommandProfile);
    infoData.push_back(devicesProfiles);

    gtList <gtString> devicesVersion;
    devicesVersion.push_back(AF_STR_SystemInformationCommandVersion);
    infoData.push_back(devicesVersion);
}


// ---------------------------------------------------------------------------
// Name:        afSystemInformationCommand::buildOpenCLDevicesTitlesAndParameterNames
// Description: Builds the OpenCL device titles, and list of attributes
// Arguments: infoData - Will get the titles and attributes.
// Author:      Sigal Algranaty
// Date:        17/3/2010
// ---------------------------------------------------------------------------
void afSystemInformationCommand::buildOpenCLDevicesTitlesAndParameterNames(gtList< gtList <gtString> >& infoData)
{
    // Clear the list:
    infoData.clear();

    // Will contain the devices parameters:
    gtList <gtString> titleLine;
    titleLine.push_back(AF_STR_SystemInformationCommandParameter);
    infoData.push_back(titleLine);

    gtList <gtString> platformIDList;
    platformIDList.push_back(AF_STR_SystemInformationCommandPlatformID);
    infoData.push_back(platformIDList);

    gtList <gtString> devicesTypes;
    devicesTypes.push_back(AF_STR_SystemInformationCommandDeviceType);
    infoData.push_back(devicesTypes);

    gtList <gtString> devicesNames;
    devicesNames.push_back(AF_STR_SystemInformationCommandDeviceName);
    infoData.push_back(devicesNames);

    gtList <gtString> devicesVendors;
    devicesVendors.push_back(AF_STR_SystemInformationCommandVendor);
    infoData.push_back(devicesVendors);

    gtList <gtString> devicesCommandQueueProperties;
    devicesCommandQueueProperties.push_back(AF_STR_SystemInformationCommandCommandQueueProperties);
    infoData.push_back(devicesCommandQueueProperties);

    gtList <gtString> devicesMaxGlobalVariableSize;
    devicesMaxGlobalVariableSize.push_back(AF_STR_SystemInformationCommandMaxGlobalVariableSize);
    infoData.push_back(devicesMaxGlobalVariableSize);

    gtList <gtString> devicesGlobalVariablePreferredTotalSize;
    devicesGlobalVariablePreferredTotalSize.push_back(AF_STR_SystemInformationCommandGlobalVariablePreferredTotalSize);
    infoData.push_back(devicesGlobalVariablePreferredTotalSize);

    gtList <gtString> devicesQueueOnDeviceProperties;
    devicesQueueOnDeviceProperties.push_back(AF_STR_SystemInformationCommandQueueOnDeviceProperties);
    infoData.push_back(devicesQueueOnDeviceProperties);

    gtList <gtString> devicesQueueOnDevicePreferredSize;
    devicesQueueOnDevicePreferredSize.push_back(AF_STR_SystemInformationCommandQueueOnDevicePreferredSize);
    infoData.push_back(devicesQueueOnDevicePreferredSize);

    gtList <gtString> devicesQueueOnDeviceMaxSize;
    devicesQueueOnDeviceMaxSize.push_back(AF_STR_SystemInformationCommandQueueOnDeviceMaxSize);
    infoData.push_back(devicesQueueOnDeviceMaxSize);

    gtList <gtString> devicesMaxOnDeviceQueues;
    devicesMaxOnDeviceQueues.push_back(AF_STR_SystemInformationCommandMaxOnDeviceQueues);
    infoData.push_back(devicesMaxOnDeviceQueues);

    gtList <gtString> devicesMaxOnDeviceEvents;
    devicesMaxOnDeviceEvents.push_back(AF_STR_SystemInformationCommandMaxOnDeviceEvents);
    infoData.push_back(devicesMaxOnDeviceEvents);

    gtList <gtString> devicesSVMCapabilities;
    devicesSVMCapabilities.push_back(AF_STR_SystemInformationCommandSVMCapabilities);
    infoData.push_back(devicesSVMCapabilities);

    gtList <gtString> devicesMaxPipeArgs;
    devicesMaxPipeArgs.push_back(AF_STR_SystemInformationCommandMaxPipeArgs);
    infoData.push_back(devicesMaxPipeArgs);

    gtList <gtString> devicesPipeMaxActiveReservations;
    devicesPipeMaxActiveReservations.push_back(AF_STR_SystemInformationCommandPipeMaxActiveReservations);
    infoData.push_back(devicesPipeMaxActiveReservations);

    gtList <gtString> devicesPipeMaxPacketSize;
    devicesPipeMaxPacketSize.push_back(AF_STR_SystemInformationCommandPipeMaxPacketSize);
    infoData.push_back(devicesPipeMaxPacketSize);

    gtList <gtString> devicesPreferredPlatformAtomicAlignment;
    devicesPreferredPlatformAtomicAlignment.push_back(AF_STR_SystemInformationCommandPreferredPlatformAtomicAlignment);
    infoData.push_back(devicesPreferredPlatformAtomicAlignment);

    gtList <gtString> devicesPreferredGlobalAtomicAlignment;
    devicesPreferredGlobalAtomicAlignment.push_back(AF_STR_SystemInformationCommandPreferredGlobalAtomicAlignment);
    infoData.push_back(devicesPreferredGlobalAtomicAlignment);

    gtList <gtString> devicesPreferredLocalAtomicAlignment;
    devicesPreferredLocalAtomicAlignment.push_back(AF_STR_SystemInformationCommandPreferredLocalAtomicAlignment);
    infoData.push_back(devicesPreferredLocalAtomicAlignment);


    // Gilad: Hide information until bug 7159 is solved
    //  gtList <gtString> devicesAddress;
    //  devicesAddress.push_back(AF_STR_SystemInformationCommandAddressBits);
    //  infoData.push_back(devicesAddress);

    gtList <gtString> devicesIsAvailableBits;
    devicesIsAvailableBits.push_back(AF_STR_SystemInformationCommandIsAvailable);
    infoData.push_back(devicesIsAvailableBits);

    gtList <gtString> devicesIsCompilerAvailable;
    devicesIsCompilerAvailable.push_back(AF_STR_SystemInformationCommandIsCompilerAvailable);
    infoData.push_back(devicesIsCompilerAvailable);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    gtList <gtString> devicesSingleFPConfig;
    devicesSingleFPConfig.push_back(AF_STR_SystemInformationCommandSingleFPConfig);
    infoData.push_back(devicesSingleFPConfig);

    gtList <gtString> devicesDoubleFPConfig;
    devicesDoubleFPConfig.push_back(AF_STR_SystemInformationCommandDoubleFPConfig);
    infoData.push_back(devicesDoubleFPConfig);

    gtList <gtString> devicesHalfFPConfig;
    devicesHalfFPConfig.push_back(AF_STR_SystemInformationCommandHalfFPConfig);
    infoData.push_back(devicesHalfFPConfig);
#endif

    gtList <gtString> devicesIsLittleEndian;
    devicesIsLittleEndian.push_back(AF_STR_SystemInformationCommandIsLittleEndian);
    infoData.push_back(devicesIsLittleEndian);

    gtList <gtString> devicesIsErrorCorrectionSupport;
    devicesIsErrorCorrectionSupport.push_back(AF_STR_SystemInformationCommandErrorCorrectionSupport);
    infoData.push_back(devicesIsErrorCorrectionSupport);

    gtList <gtString> deviceExecutionCapabilitiesList;
    deviceExecutionCapabilitiesList.push_back(AF_STR_SystemInformationCommandExecutionCapabilities);
    infoData.push_back(deviceExecutionCapabilitiesList);

    gtList <gtString> deviceGlobalMemCacheSizeList;
    deviceGlobalMemCacheSizeList.push_back(AF_STR_SystemInformationCommandGlobalMemoryCacheSize);
    infoData.push_back(deviceGlobalMemCacheSizeList);

    gtList <gtString> deviceMemCacheTypeList;
    deviceMemCacheTypeList.push_back(AF_STR_SystemInformationCommandMemoryCacheType);
    infoData.push_back(deviceMemCacheTypeList);

    gtList <gtString> deviceGlobalMemCacheLineSizeList;
    deviceGlobalMemCacheLineSizeList.push_back(AF_STR_SystemInformationCommandGlobalMemoryCacheLineSize);
    infoData.push_back(deviceGlobalMemCacheLineSizeList);

    gtList <gtString> deviceGlobalMemSizeList;
    deviceGlobalMemSizeList.push_back(AF_STR_SystemInformationCommandGlobalMemorySize);
    infoData.push_back(deviceGlobalMemSizeList);

    gtList <gtString> deviceisHostUnifiedMemoryList;
    deviceisHostUnifiedMemoryList.push_back(AF_STR_SystemInformationCommandHostUnifiedMemory);
    infoData.push_back(deviceisHostUnifiedMemoryList);

    gtList <gtString> deviceIsImageSupportList;
    deviceIsImageSupportList.push_back(AF_STR_SystemInformationCommandAreImageSupported);
    infoData.push_back(deviceIsImageSupportList);

    gtList <gtString> maxImage2DDimensionList;
    maxImage2DDimensionList.push_back(AF_STR_SystemInformationCommandMaxImage2DDimensions);
    infoData.push_back(maxImage2DDimensionList);

    gtList <gtString> maxImage3DDimensionList;
    maxImage3DDimensionList.push_back(AF_STR_SystemInformationCommandMaxImage3DDimensions);
    infoData.push_back(maxImage3DDimensionList);

    gtList <gtString> deviceLocalMemSizeList;
    deviceLocalMemSizeList.push_back(AF_STR_SystemInformationCommandLocalMemorySize);
    infoData.push_back(deviceLocalMemSizeList);

    gtList <gtString> deviceLocalMemTypeList;
    deviceLocalMemTypeList.push_back(AF_STR_SystemInformationCommandLocalMemoryType);
    infoData.push_back(deviceLocalMemTypeList);

    gtList <gtString> maxClockFrequencyList;
    maxClockFrequencyList.push_back(AF_STR_SystemInformationCommandMaxClockFrequency);
    infoData.push_back(maxClockFrequencyList);

    gtList <gtString> maxComputeUnitsList;
    maxComputeUnitsList.push_back(AF_STR_SystemInformationCommandMaxComputeUnits);
    infoData.push_back(maxComputeUnitsList);

    gtList <gtString> maxConstantArgsList;
    maxConstantArgsList.push_back(AF_STR_SystemInformationCommandMaxConstantArguments);
    infoData.push_back(maxConstantArgsList);

    gtList <gtString> maxConstantBufferSizeList;
    maxConstantBufferSizeList.push_back(AF_STR_SystemInformationCommandMaxConstantBufferSize);
    infoData.push_back(maxConstantBufferSizeList);

    gtList <gtString> maxMemAllocSizeList;
    maxMemAllocSizeList.push_back(AF_STR_SystemInformationCommandMaxMemoryAllocationSize);
    infoData.push_back(maxMemAllocSizeList);

    gtList <gtString> maxParamSizeList;
    maxParamSizeList.push_back(AF_STR_SystemInformationCommandMaxParameterSize);
    infoData.push_back(maxParamSizeList);

    gtList <gtString> maxReadImageArgsList;
    maxReadImageArgsList.push_back(AF_STR_SystemInformationCommandReadImageArguments);
    infoData.push_back(maxReadImageArgsList);

    gtList <gtString> maxSamplersList;
    maxSamplersList.push_back(AF_STR_SystemInformationCommandMaxSamplers);
    infoData.push_back(maxSamplersList);

    gtList <gtString> maxWorkgroupSizeList;
    maxWorkgroupSizeList.push_back(AF_STR_SystemInformationCommandMaxWorkgroupSize);
    infoData.push_back(maxWorkgroupSizeList);

    gtList <gtString> maxWorkItemDimensionsList;
    maxWorkItemDimensionsList.push_back(AF_STR_SystemInformationCommandMaxWorkItemDimensions);
    infoData.push_back(maxWorkItemDimensionsList);

    gtList <gtString> maxWorkItemSizesList;
    maxWorkItemSizesList.push_back(AF_STR_SystemInformationCommandMaxWorkItemSizes);
    infoData.push_back(maxWorkItemSizesList);

    gtList <gtString> maxWriteImageArgsList;
    maxWriteImageArgsList.push_back(AF_STR_SystemInformationCommandMaxWriteImageArguments);
    infoData.push_back(maxWriteImageArgsList);

    gtList <gtString> maxReadWriteImageArgsList;
    maxReadWriteImageArgsList.push_back(AF_STR_SystemInformationCommandMaxReadWriteImageArguments);
    infoData.push_back(maxReadWriteImageArgsList);

    gtList <gtString> memBaseAddrAlignList;
    memBaseAddrAlignList.push_back(AF_STR_SystemInformationCommandMemoryBaseAddressAlignment);
    infoData.push_back(memBaseAddrAlignList);

    gtList <gtString> minDataTypeAlignSizeList;
    minDataTypeAlignSizeList.push_back(AF_STR_SystemInformationCommandMinimalDataTypeAlignmentSize);
    infoData.push_back(minDataTypeAlignSizeList);

    gtList <gtString> openCLCVersionList;
    openCLCVersionList.push_back(AF_STR_SystemInformationCommandOpenCLCVersion);
    infoData.push_back(openCLCVersionList);

    gtList <gtString> nativeVecWidthCharList;
    nativeVecWidthCharList.push_back(AF_STR_SystemInformationCommandNativeCharVectorWidth);
    infoData.push_back(nativeVecWidthCharList);

    gtList <gtString> nativeVecWidthShortList;
    nativeVecWidthShortList.push_back(AF_STR_SystemInformationCommandNativeShortVectorWidth);
    infoData.push_back(nativeVecWidthShortList);

    gtList <gtString> nativeVecWidthIntList;
    nativeVecWidthIntList.push_back(AF_STR_SystemInformationCommandNativeIntVectorWidth);
    infoData.push_back(nativeVecWidthIntList);

    gtList <gtString> nativeVecWidthLongList;
    nativeVecWidthLongList.push_back(AF_STR_SystemInformationCommandNativeLongVectorWidth);
    infoData.push_back(nativeVecWidthLongList);

    gtList <gtString> nativeVecWidthFloatList;
    nativeVecWidthFloatList.push_back(AF_STR_SystemInformationCommandNativeFloatVectorWidth);
    infoData.push_back(nativeVecWidthFloatList);

    gtList <gtString> nativeVecWidthDoubleList;
    nativeVecWidthDoubleList.push_back(AF_STR_SystemInformationCommandNativeDoubleVectorWidth);
    infoData.push_back(nativeVecWidthDoubleList);

    gtList <gtString> nativeVecWidthHalfList;
    nativeVecWidthHalfList.push_back(AF_STR_SystemInformationCommandNativeHalfVectorWidth);
    infoData.push_back(nativeVecWidthHalfList);

    gtList <gtString> preferredVecWidthCharList;
    preferredVecWidthCharList.push_back(AF_STR_SystemInformationCommandPreferredCharVectorWidth);
    infoData.push_back(preferredVecWidthCharList);

    gtList <gtString> preferredVecWidthShortList;
    preferredVecWidthShortList.push_back(AF_STR_SystemInformationCommandPreferredShortVectorWidth);
    infoData.push_back(preferredVecWidthShortList);

    gtList <gtString> preferredVecWidthIntList;
    preferredVecWidthIntList.push_back(AF_STR_SystemInformationCommandPreferredIntVectorWidth);
    infoData.push_back(preferredVecWidthIntList);

    gtList <gtString> preferredVecWidthLongList;
    preferredVecWidthLongList.push_back(AF_STR_SystemInformationCommandPreferredLongVectorWidth);
    infoData.push_back(preferredVecWidthLongList);

    gtList <gtString> preferredVecWidthFloatList;
    preferredVecWidthFloatList.push_back(AF_STR_SystemInformationCommandPreferredFloatVectorWidth);
    infoData.push_back(preferredVecWidthFloatList);

    gtList <gtString> preferredVecWidthDoubleList;
    preferredVecWidthDoubleList.push_back(AF_STR_SystemInformationCommandPreferredDoubleVectorWidth);
    infoData.push_back(preferredVecWidthDoubleList);

    gtList <gtString> preferredVecWidthHalfList;
    preferredVecWidthHalfList.push_back(AF_STR_SystemInformationCommandPreferredHalfVectorWidth);
    infoData.push_back(preferredVecWidthHalfList);

    gtList <gtString> profileStrList;
    profileStrList.push_back(AF_STR_SystemInformationCommandProfile);
    infoData.push_back(profileStrList);

    gtList <gtString> profilingTimerResolutionList;
    profilingTimerResolutionList.push_back(AF_STR_SystemInformationCommandProfilingTimerResolution);
    infoData.push_back(profilingTimerResolutionList);


    gtList <gtString> deviceVendorIDList;
    deviceVendorIDList.push_back(AF_STR_SystemInformationCommandVendorID);
    infoData.push_back(deviceVendorIDList);

}


// ---------------------------------------------------------------------------
// Name:        afSystemInformationCommand::platformParamAsString
// Description: Queries the value of an OpenCL's platform parameter.
// Arguments:
//   platformHandle - The OpenCL platform handle
//   paramName - The queried parameter name.
//   paramValueAsStr - Will get the parameter's value as a string.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        17/3/2010
// ---------------------------------------------------------------------------
bool afSystemInformationCommand::platformParamAsString(oaCLPlatformID platformHandle, cl_platform_info paramName, gtString& paramValueAsStr)
{
    bool retVal = false;

    // Get the parameter value's length:
    gtSizeType stringLen = 0;
    cl_platform_id platformId = (cl_platform_id)platformHandle;
    cl_int clRetVal = _pclGetPlatformInfo(platformId, paramName, 0, nullptr, &stringLen);
    GT_IF_WITH_ASSERT((clRetVal == CL_SUCCESS) && (stringLen > 0))
    {
        // Allocate space for the parameter's value:
        char* pParamValue = new char[stringLen + 1];

        // Get the parameter's value:
        clRetVal = _pclGetPlatformInfo((cl_platform_id)platformHandle, paramName, stringLen + 1, pParamValue, nullptr);
        GT_IF_WITH_ASSERT(clRetVal == CL_SUCCESS)
        {
            paramValueAsStr.fromASCIIString(pParamValue);
            retVal = true;
        }

        // Clean up:
        delete[] pParamValue;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        afSystemInformationCommand::collectOpenCLPlatformIds
// Description: Get the OpenCL platform ids, and insert it one by one to the
//              platform to name map
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        11/4/2010
// ---------------------------------------------------------------------------
bool afSystemInformationCommand::collectOpenCLPlatformIds()
{
    bool retVal = false;
    // Initialize the OpenCL platform function pointers:
    bool rcOpenCLFunctions = initOpenCLFunctionPointers();

    if (rcOpenCLFunctions)
    {
        // Get OpenCL platforms count:
        cl_uint amountOfPlatforms = 0;
        cl_uint clRetVal = _pclGetPlatformIDs(0, nullptr, &amountOfPlatforms);
        GT_IF_WITH_ASSERT(clRetVal == CL_SUCCESS)
        {
            retVal = true;

            // If there are OpenCL platforms registered in the OpenCL ICD:
            if (0 < amountOfPlatforms)
            {
                // Allocate space for the platform IDs:
                cl_platform_id* pPlatformIds = new cl_platform_id[amountOfPlatforms];

                // Get the platform ids:
                clRetVal = _pclGetPlatformIDs(amountOfPlatforms, pPlatformIds, nullptr);
                GT_IF_WITH_ASSERT(clRetVal == CL_SUCCESS)
                {
                    // Iterate the platforms:
                    for (cl_uint i = 0; i < amountOfPlatforms; ++i)
                    {
                        // Get the current platform id:
                        cl_platform_id currentPlatformId = pPlatformIds[i];

                        // Add this platform to the mapping:
                        oaCLPlatformID platformId = oaCLPlatformID(currentPlatformId);
                        _platformIdToName[platformId] = i;
                    }
                }

                // Clean up:
                delete[] pPlatformIds;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afSystemInformationCommand::getSystemInformationDataAsString
// Description: Collect the system information data and append to a string
// Arguments:   gtString& systemInformationStr
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        5/5/2011
// ---------------------------------------------------------------------------
bool afSystemInformationCommand::getSystemInformationDataAsString(gtString& systemInformationStr)
{
    bool retVal = false;

    // empty the string:
    systemInformationStr.makeEmpty();

    // Build the system module versions as string:
    bool rcGetModuleVersions = getSystemModulesVersionAsString(systemInformationStr);

    // Get graphic card details:
    gtString vendorString, rendererString, versionString, shadingLangVersionString, rendererType;
    bool rcGPU = getGraphicCardDetails(vendorString, rendererString, versionString, shadingLangVersionString, rendererType);

    // TO_DO: crash information - do we want to replace this GT_IF_WITH_ASSERT with simple if ?
    GT_IF_WITH_ASSERT(rcGPU)
    {
        systemInformationStr.append(AF_STR_NewLine AF_STR_NewLine AF_STR_ContextsInformationDialogGLRendererTitle L":" AF_STR_NewLine);
        systemInformationStr.appendFormattedString(L" - %ls: %ls", AF_STR_ContextsInformationDialogRendererVendor, vendorString.asCharArray());
        systemInformationStr.append(AF_STR_NewLine);
        systemInformationStr.appendFormattedString(L" - %ls: %ls", AF_STR_ContextsInformationDialogRendererName, rendererString.asCharArray());
        systemInformationStr.append(AF_STR_NewLine);
        systemInformationStr.appendFormattedString(L" - %ls: %ls", AF_STR_ContextsInformationDialogRendererVersion, versionString.asCharArray());
        systemInformationStr.append(AF_STR_NewLine);
        systemInformationStr.appendFormattedString(L" - %ls: %ls", AF_STR_ContextsInformationDialogRendererType, rendererType.asCharArray());
        systemInformationStr.append(AF_STR_NewLine);
    }

    // Get Drivers version:
    gtString calVersion;
    bool rcCalVersion = oaGetCalVersion(calVersion);
    int driverError = OA_DRIVER_UNKNOWN;
    gtString driverVersion = oaGetDriverVersion(driverError);
    GT_IF_WITH_ASSERT(rcCalVersion && (driverError != OA_DRIVER_NOT_FOUND))
    {
        systemInformationStr.append(AF_STR_NewLine AF_STR_ContextsInformationDialogDriversTitle L":" AF_STR_NewLine);
        systemInformationStr.appendFormattedString(L" - %ls: %ls", AF_STR_ContextsInformationDialogDriversCALVersion, calVersion.asCharArray());
        systemInformationStr.append(AF_STR_NewLine);
        systemInformationStr.appendFormattedString(L" - %ls: %ls", AF_STR_ContextsInformationDialogDriversCatalystVersion, driverVersion.asCharArray());
        systemInformationStr.append(AF_STR_NewLine);
    }

    // Get the OpenCL devices information:
    gtList<gtList<gtString> > devicesInfo;
    bool rcCLDevices = CollectOpenCLDevicesInformation(devicesInfo, false);

    if (rcCLDevices)
    {
        // Append the devices strings to the system information string:
        appendFlattenedListOfStrings(devicesInfo, systemInformationStr);
    }

    // Get the OpenCL devices information:
    gtList<gtList<gtString> > platformsInfo;
    bool rcCLPlatforms = collectOpenCLPlatformsInformation(platformsInfo, false);

    if (rcCLPlatforms)
    {
        // Append the platforms strings to the system information string:
        appendFlattenedListOfStrings(platformsInfo, systemInformationStr);
    }

    // Do not fail for OpenCL functionality - there are machines with no OpenCL installed:
    retVal = rcGPU && rcGetModuleVersions;
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afSystemInformationCommand::getSystemModulesVersionAsString
// Description: Append the system modules versions to the system information string
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        8/12/2010
// ---------------------------------------------------------------------------
bool afSystemInformationCommand::getSystemModulesVersionAsString(gtString& systemModulesVersionStr)
{
    bool retVal = false;

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Add windows system DLL names:
    _systemModuleNames.push_back(OS_ATI_OGL_DRIVER_DLL_NAME);
    _systemModuleNames.push_back(OS_ATI_O6_DRIVER_DLL_NAME);
    _systemModuleNames.push_back(OS_ATI_CFX32_DRIVER_DLL_NAME);
    _systemModuleNames.push_back(OS_ATI_CFX64_DRIVER_DLL_NAME);
    _systemModuleNames.push_back(OS_ATI_OCL_DRIVER_DLL_NAME);
    _systemModuleNames.push_back(OS_ATI_CALDD_DRIVER_DLL_NAME);
    _systemModuleNames.push_back(OS_ATI_OCL64_DRIVER_DLL_NAME);

    _systemModuleNames.push_back(OS_INTEL_32_DRIVER_DLL_NAME);
    _systemModuleNames.push_back(OS_INTEL_64_DRIVER_DLL_NAME);

    _systemModuleNames.push_back(OS_NVIDIA_OGL_DRIVER_DLL_NAME);
    _systemModuleNames.push_back(OS_NVIDIA_OGL32_DRIVER_DLL_NAME);
    _systemModuleNames.push_back(OS_NVIDIA_VISTA32_OGL_DRIVER_DLL_NAME);
    _systemModuleNames.push_back(OS_NVIDIA_VISTA64_OGL_DRIVER_DLL_NAME);

    _systemModuleNames.push_back(OS_NVIDIA_CUDA_DRIVER_DLL_NAME);
    _systemModuleNames.push_back(OS_OPENCL_MODULE_NAME);

    // Go through the module names for this OS and find the file version:
    if (_systemModuleNames.size() > 0)
    {
        systemModulesVersionStr.append(AF_STR_SendErrorReportString7);
    }

    for (int i = 0 ; i < (int)_systemModuleNames.size(); i++)
    {
        gtString moduleVersion;
        bool rcGetVersion = osGetSystemModuleVersionAsString(_systemModuleNames[i], moduleVersion);

        if (rcGetVersion)
        {
            systemModulesVersionStr.appendFormattedString(L"\n - %ls.dll: %ls", _systemModuleNames[i].asCharArray(), moduleVersion.asCharArray());
            retVal = true;
        }
    }

#else // AMDT_BUILD_TARGET != AMDT_WINDOWS_OS
    (void)(systemModulesVersionStr); // Resolve the compiler warning for the Linux variant
    // Linux shared objects do not have versions. Mac libraries only change per operating system version.
    // So, on both these platforms we don't need to check for a module version on the system modules.
    retVal = true;
#endif

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        afSystemInformationCommand::appendFlattenedListOfStrings
// Description: This function get a table of strings for multiple devices or platform
//              descriptions, and it flattens the table to a list with each of the
//              devices / platforms in separate
// Arguments:   gtList<gtList<gtString&>> infoList
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        8/12/2010
// ---------------------------------------------------------------------------
void afSystemInformationCommand::appendFlattenedListOfStrings(gtList<gtList<gtString> >& infoList, gtString& stringToAppnedTo)
{
    // Iterate the system information list:
    gtVector<gtString> finalStrings;

    gtList<gtList<gtString> >::const_iterator iterInfoList = infoList.begin();
    gtList<gtList<gtString> >::const_iterator iterInfoListEnd = infoList.end();

    if (iterInfoList != iterInfoListEnd)
    {
        // Get the first device properties list - the devices names:
        gtList<gtString> names = (*iterInfoList);

        // Get the amount of list of strings:
        int amountOfLists = names.size() - 1;

        // Create a new string for each device, and add the device name into it:
        gtList<gtString>::const_iterator iterAttribs = names.begin();
        gtList<gtString>::const_iterator iterAttribsEnd = names.end();

        // Start with the second string (the first is only the "Name" title:
        for (iterAttribs++; iterAttribs != iterAttribsEnd; iterAttribs ++)
        {
            gtString currentString;
            currentString.appendFormattedString(L"%ls:", (*iterAttribs).asCharArray());
            finalStrings.push_back(currentString);
        }

        // Iterate each of the devices attributes, and add to the device description strings:
        for (iterInfoList ++; iterInfoList != iterInfoListEnd; iterInfoList ++)
        {
            // Get the current devices attribute:
            gtList<gtString> currentDevicesAttribute = (*iterInfoList);

            // Create a new string for each device, and add the device name into it:
            gtList<gtString>::const_iterator iterDeviceAttr = currentDevicesAttribute.begin();
            gtList<gtString>::const_iterator iterDeviceAttrEnd = currentDevicesAttribute.end();

            gtString attributeName = (*iterDeviceAttr);

            // Start with the second string (the first is only the "Device Name" title:
            int listIndex = 0;

            for (iterDeviceAttr++; iterDeviceAttr != iterDeviceAttrEnd; iterDeviceAttr++)
            {
                GT_IF_WITH_ASSERT(listIndex < amountOfLists)
                {
                    gtString& currentDeviceString = finalStrings[listIndex];
                    gtString attrValue = (*iterDeviceAttr);
                    currentDeviceString.appendFormattedString(L"\n - %ls: %ls", attributeName.asCharArray(), attrValue.asCharArray());

                    // Increment the device index:
                    listIndex++;
                }
            }
        }

        // Add each of the devices strings to the system information string:
        for (int i = 0; i < amountOfLists; i++)
        {
            // Get the current device string:
            gtString currentDeviceString = finalStrings[i];

            stringToAppnedTo.append(AF_STR_NewLine);
            stringToAppnedTo.append(currentDeviceString);
            stringToAppnedTo.append(AF_STR_NewLine);
        }
    }
}

// ---------------------------------------------------------------------------
void afSystemInformationCommand::GenerateUniquePipeName(gtString& pipeName)
{
    pipeName = AF_STR_SystemInformationCommandPipePrefix;

    // Get the current time and date as strings:
    osTime curretTime;
    curretTime.setFromCurrentTime();

    gtString dateAsString;
    curretTime.dateAsString(dateAsString, osTime::UNDERSCORE_SAPERATOR, osTime::LOCAL);

    gtString timeAsString;
    curretTime.timeAsString(timeAsString, osTime::UNDERSCORE_SAPERATOR, osTime::LOCAL);

    // Append the date and time to the file name:
    pipeName += L"-";
    pipeName += dateAsString;
    pipeName += L"-";
    pipeName += timeAsString;
}
