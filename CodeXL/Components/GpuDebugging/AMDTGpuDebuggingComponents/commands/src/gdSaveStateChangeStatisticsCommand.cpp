//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdSaveStateChangeStatisticsCommand.cpp
///
//==================================================================================

//------------------------------ gdSaveStateChangeStatisticsCommand.cpp ------------------------------

// Qt
#include <qtIgnoreCompilerWarnings.h>
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

#include <AMDTApplicationComponents/Include/acChartWindow.h>
#include <AMDTGpuDebuggingComponents/Include/gdStatisticsPanel.h>

// Infra
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afProjectManager.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/commands/gdSaveStateChangeStatisticsCommand.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdStateChangeStatisticsView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdStatisticsView.h>


gdSaveStateChangeStatisticsCommand::gdSaveStateChangeStatisticsCommand(const osFilePath& fileName)
    : _filePath(fileName), _stateChangeStatisticsOutputString(AF_STR_EmptyA)
{
}

// ---------------------------------------------------------------------------
// Name:        gdSaveStateChangeStatisticsCommand::~gdSaveStateChangeStatisticsCommand
// Description: Destructor.
// Author:      Sigal Algranaty
// Date:        26/8/2008
// ---------------------------------------------------------------------------
gdSaveStateChangeStatisticsCommand::~gdSaveStateChangeStatisticsCommand()
{
    // Do Nothing...
}

// ---------------------------------------------------------------------------
// Name:        gdSaveStateChangeStatisticsCommand::canExecuteSpecificCommand
// Description: Answers the question - can we save the function Calls Statistics
// Author:      Sigal Algranaty
// Date:        26/8/2008
// Implementation Notes:
// Currently - the answer is always yes.
// ---------------------------------------------------------------------------
bool gdSaveStateChangeStatisticsCommand::canExecuteSpecificCommand()
{
    return true;
}

// ---------------------------------------------------------------------------
// Name:        gdSaveStateChangeStatisticsCommand::executeSpecificCommand
// Description: Save the function calls statistics of the G-Debugger application.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        26/8/2008
// ---------------------------------------------------------------------------
bool gdSaveStateChangeStatisticsCommand::executeSpecificCommand()
{
    bool rc = false;

    rc = getStateChangeStatisticsData();
    GT_IF_WITH_ASSERT(rc)
    {
        rc = writeStateChangeStatisticsDataToFile();
    }

    return rc;
}

// ---------------------------------------------------------------------------
// Name:        gdSaveStateChangeStatisticsCommand::getStateChangeStatisticsData
// Description: Get the OpenGL Function Calls Statistics data
//              into _stateChangeStatisticsOutputString
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        26/8/2008
// ---------------------------------------------------------------------------
bool gdSaveStateChangeStatisticsCommand::getStateChangeStatisticsData()
{
    bool retVal = false;

    /*// Get the Function Calls Statistics View:
    gdApplicationCommands* pApplicationCommands = gdApplicationCommands::gdInstance();
    GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
    {
    gdStatisticsView* pStatisticsView = pApplicationCommands->statisticsPanel()->statisticsView();
    GT_IF_WITH_ASSERT(pStatisticsView != NULL)
    {
    gdStatisticsViewBase* pStateChangeStatisticsView = pStatisticsView->statisticsPage(GD_STATISTICS_VIEW_STATE_CHANGE_INDEX);
    GT_IF_WITH_ASSERT(pStateChangeStatisticsView != NULL)
    {
    // Create table header
    _stateChangeStatisticsOutputString.append(GD_STR_StateChangeStatisticsViewerColumn1Title);
    _stateChangeStatisticsOutputString.append(",");
    _stateChangeStatisticsOutputString.append(GD_STR_StateChangeStatisticsViewerColumn2Title);
    _stateChangeStatisticsOutputString.append(",");
    _stateChangeStatisticsOutputString.append(GD_STR_StateChangeStatisticsViewerColumn3Title);
    _stateChangeStatisticsOutputString.append(",");
    _stateChangeStatisticsOutputString.append(GD_STR_StateChangeStatisticsViewerColumn4Title);
    _stateChangeStatisticsOutputString.append(",");
    _stateChangeStatisticsOutputString.append(GD_STR_StateChangeStatisticsViewerColumn5Title);

    // Add a new line (after the table header):
    _stateChangeStatisticsOutputString.append("\n");

    // Loop through all the selected items in the list:
    for( int rowIndex = 0; rowIndex < pStateChangeStatisticsView->rowCount(); rowIndex++)
    {
    // Get the function name
    QString functionName;
    functionName = pStateChangeStatisticsView->getItemText(rowIndex, 0, functionName);
    QString functionTotalAmountOfCalls;
    functionName = pStateChangeStatisticsView->getItemText(rowIndex, 1, functionTotalAmountOfCalls);
    functionTotalAmountOfCalls.remove(GT_THOUSANDS_SEPARATOR);

    QString functionEffectiveCallsAmount;
    functionName = pStateChangeStatisticsView->getItemText(rowIndex, 2, functionTotalAmountOfCalls);
    functionTotalAmountOfCalls.remove(GT_THOUSANDS_SEPARATOR);

    QString functionRedundantCallsAmount;
    functionName = pStateChangeStatisticsView->getItemText(rowIndex, 3, functionTotalAmountOfCalls);
    functionTotalAmountOfCalls.remove(GT_THOUSANDS_SEPARATOR);

    QString functionRedundantCallsPercentage;
    functionName = pStateChangeStatisticsView->getItemText(rowIndex, 4, functionTotalAmountOfCalls);
    functionTotalAmountOfCalls.remove(GT_THOUSANDS_SEPARATOR);

    _stateChangeStatisticsOutputString.append(functionName.toLatin1().data());
    _stateChangeStatisticsOutputString.append(",");
    _stateChangeStatisticsOutputString.append(functionTotalAmountOfCalls.toLatin1().data());
    _stateChangeStatisticsOutputString.append(",");
    _stateChangeStatisticsOutputString.append(functionEffectiveCallsAmount.toLatin1().data());
    _stateChangeStatisticsOutputString.append(",");
    _stateChangeStatisticsOutputString.append(functionRedundantCallsAmount.toLatin1().data());
    _stateChangeStatisticsOutputString.append(",");
    _stateChangeStatisticsOutputString.append(functionRedundantCallsPercentage.toLatin1().data());
    _stateChangeStatisticsOutputString.append("%");

    // Add a line break between the table body rows
    _stateChangeStatisticsOutputString.append("\n");
    }
    }
    }
    }*/

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdSaveStateChangeStatisticsCommand::writeStateChangeStatisticsDataToFile
// Description: Write the Function Calls Statistics data into a file
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        26/8/2008
// ---------------------------------------------------------------------------
bool gdSaveStateChangeStatisticsCommand::writeStateChangeStatisticsDataToFile()
{
    bool retVal = true;
    gtString fileHeader;
    osTime fileSavedDateAndTime;
    gtString fileSavedDate;
    gtString fileSavedTime;

    osFilePath filePath(_filePath);
    osFile stateChangeStatisticsFile;

    retVal = stateChangeStatisticsFile.open(filePath, osChannel::OS_UNICODE_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);

    if (retVal)
    {
        fileSavedDateAndTime.setFromCurrentTime();

        fileSavedDateAndTime.dateAsString(fileSavedDate, osTime::UNDERSCORE_SAPERATOR, osTime::LOCAL);
        fileSavedDateAndTime.timeAsString(fileSavedTime, osTime::WINDOWS_STYLE, osTime::LOCAL);

        // Get the current project name:
        gtString projectFileName = afProjectManager::instance().currentProjectSettings().projectName();

        fileHeader.append(GD_STR_StatisticsExportFileHeaderSeperator);
        fileHeader.append(GD_STR_StatisticsExportFileHeaderTitlePrefix);
        fileHeader.append(GD_STR_saveStateChageStatistics);
        fileHeader.append(GD_STR_StatisticsExportFileHeaderTitlePostfix);
        fileHeader.append(GD_STR_StatisticsExportFileHeaderProjectName) += projectFileName += AF_STR_NewLine;;
        fileHeader.append(GD_STR_StatisticsExportFileHeaderGenerationDate) += fileSavedDate += AF_STR_NewLine;
        fileHeader.append(GD_STR_StatisticsExportFileHeaderGenerationTime) += fileSavedTime += L"\n//\n";
        fileHeader.append(GD_STR_StatisticsExportFileHeaderGeneratedBy);
        fileHeader.append(GD_STR_StatisticsExportFileHeaderWebSite);
        fileHeader.append(GD_STR_StatisticsExportFileHeaderSeperator) += AF_STR_NewLine;

        stateChangeStatisticsFile << fileHeader;
        stateChangeStatisticsFile << _stateChangeStatisticsOutputString;
        stateChangeStatisticsFile.close();
    }

    return retVal;
}
