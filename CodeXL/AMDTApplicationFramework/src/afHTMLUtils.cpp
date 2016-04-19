//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afHTMLUtils.cpp
///
//==================================================================================

// Infra:
#include <AMDTAPIClasses/Include/apDebugProjectSettings.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessRunStartedEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessTerminatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessCreatedEvent.h>

/// Local:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afHTMLContent.h>
#include <AMDTApplicationFramework/Include/afHTMLUtils.h>



afHTMLUtils::afHTMLUtils()
{

}

// ---------------------------------------------------------------------------
// Name:        afHTMLUtils::buildProcessRunStartedEventPropertiesString
// Description: Builds a process run started event properties string
// Arguments: const apDebugProjectSettings& processStartedData  - the process creation data
//            const apDebuggedProcessRunStartedEvent& processRunStartedEvent
//            gtString& propertiesHTMLMessage
// Return Val: void
// Author:      Sigal Algranaty
// Date:        20/12/2009
// ---------------------------------------------------------------------------
void afHTMLUtils::buildProcessRunStartedEventPropertiesString(const apDebugProjectSettings& processStartedData, const apDebuggedProcessRunStartedEvent& processRunStartedEvent, gtString& propertiesHTMLMessage)
{
    // Build the HTML content:
    afHTMLContent htmlContent(AF_STR_ProcessEventsViewPropertiesTitleProcessStarted);

    // Report the process started time:
    const osTime& startedTime = processRunStartedEvent.processRunStartedTime();
    gtString processStartedTime;
    gtString processStartedDate;
    startedTime.dateAsString(processStartedDate, osTime::WINDOWS_STYLE, osTime::LOCAL);
    startedTime.timeAsString(processStartedTime, osTime::WINDOWS_STYLE, osTime::LOCAL);
    processStartedDate.appendFormattedString(L" %ls", processStartedTime.asCharArray());

    // Get the executable path string:
    gtString executablePathStr = processStartedData.executablePath().asString();

    // Get the working directory:
    gtString workDirStr = processStartedData.workDirectory().asString();

    // Get the process arguments:
    gtString argumentsStr = processStartedData.commandLineArguments();

    if (argumentsStr.isEmpty())
    {
        argumentsStr = AF_STR_None;
    }

    // Build the process name:
    gtString processName, fileExtension;
    processStartedData.executablePath().getFileName(processName);
    processStartedData.executablePath().getFileExtension(fileExtension);

    if (!fileExtension.isEmpty())
    {
        processName.appendFormattedString(L".%ls", fileExtension.asCharArray());
    }

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ProcessName, processName);

    // Add the created thread properties to the table;
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ProcessEventsViewStartTime, processStartedDate);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ProcessEventsViewExePath, executablePathStr);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ProcessEventsViewWorkDir, workDirStr);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_Arguments, argumentsStr);

    // Convert the HTML content to a string:
    htmlContent.toString(propertiesHTMLMessage);

}

// ---------------------------------------------------------------------------
// Name:        afHTMLUtils::buildProcessTerminationEventPropertiesString
// Description: Builds a process termination event properties string
// Arguments: const apDebuggedProcessTerminatedEvent& processTerminationEvent
//            gtString& propertiesHTMLMessage
// Return Val: void
// Author:      Sigal Algranaty
// Date:        20/12/2009
// ---------------------------------------------------------------------------
void afHTMLUtils::buildProcessTerminationEventPropertiesString(const apDebugProjectSettings& processStartedData, const apDebuggedProcessTerminatedEvent& processTerminationEvent, gtString& propertiesHTMLMessage)
{
    // Build the HTML content:
    afHTMLContent htmlContent(AF_STR_ProcessEventsViewProcessExit);

    // Build the process exit code string:
    long exitCode = processTerminationEvent.processExitCode();
    gtString exitCodeString;
    exitCodeString.appendFormattedString(L"%ld", exitCode);

    // Build the process name:
    gtString processName, fileExtension;
    processStartedData.executablePath().getFileName(processName);
    processStartedData.executablePath().getFileExtension(fileExtension);

    if (!fileExtension.isEmpty())
    {
        processName.appendFormattedString(L".%ls", fileExtension.asCharArray());
    }

    // Report the process exit date and time:
    osTime terminationTime = processTerminationEvent.processTerminationTime();
    gtString processExitDate;
    gtString processExitTime;
    terminationTime.dateAsString(processExitDate, osTime::WINDOWS_STYLE, osTime::LOCAL);
    terminationTime.timeAsString(processExitTime, osTime::WINDOWS_STYLE, osTime::LOCAL);
    processExitDate.appendFormattedString(L" %ls", processExitTime.asCharArray());

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, processName);

    // Add the created thread properties to the table;
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ProcessEventsViewExitCode, exitCodeString);

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ProcessEventsViewExitTime, processExitDate);


    // Convert the HTML content to a string:
    htmlContent.toString(propertiesHTMLMessage);
}


// ---------------------------------------------------------------------------
// Name:        afHTMLUtils::buildProcessCreatedEventPropertiesString
// Description:
// Arguments: const apDebugProjectSettings& processCreationData
//            gtString& propertiesHTMLMessage
// Return Val: void
// Author:      Sigal Algranaty
// Date:        15/12/2009
// ---------------------------------------------------------------------------
void afHTMLUtils::buildProcessCreatedEventPropertiesString(const apDebugProjectSettings& processCreationData, const apDebuggedProcessCreatedEvent& processCreatedEvent, gtString& propertiesHTMLMessage)
{
    // Report the process creation date and time:
    const osTime& creationTime = processCreatedEvent.processCreationTime();
    gtString processCreationDate;
    gtString processCreationTime;
    creationTime.dateAsString(processCreationDate, osTime::WINDOWS_STYLE, osTime::LOCAL);
    creationTime.timeAsString(processCreationTime, osTime::WINDOWS_STYLE, osTime::LOCAL);
    processCreationDate.appendFormattedString(L" %ls", processCreationTime.asCharArray());

    // Get the executable path report:
    gtString executablePathReport = processCreationData.executablePath().asString();

    // Get the work dir:
    gtString workDirReport = processCreationData.workDirectory().asString();

    // Get the argument report:
    gtString argumentsReport = processCreationData.commandLineArguments();

    if (argumentsReport.isEmpty())
    {
        argumentsReport = AF_STR_None;
    }

    // Build the process name:
    gtString processName, fileExtension;
    processCreationData.executablePath().getFileName(processName);
    processCreationData.executablePath().getFileExtension(fileExtension);

    if (!fileExtension.isEmpty())
    {
        processName.appendFormattedString(L".%ls", fileExtension.asCharArray());
    }

    // Build the HTML content:
    afHTMLContent htmlContent(AF_STR_ProcessEventsViewPropertiesTitleProcessCreated);

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ProcessName, processName);

    // Add the created process properties to the table;

    // "Started waiting time" should not be shown, as it is not interesting:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ProcessEventsViewCreationTime, processCreationDate);

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ProcessEventsViewExePath, executablePathReport);

    // Working directory and program arguments are not used in remote targets:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ProcessEventsViewWorkDir, workDirReport);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_Arguments, argumentsReport);


    // Convert the HTML content to a string:
    htmlContent.toString(propertiesHTMLMessage);

}
