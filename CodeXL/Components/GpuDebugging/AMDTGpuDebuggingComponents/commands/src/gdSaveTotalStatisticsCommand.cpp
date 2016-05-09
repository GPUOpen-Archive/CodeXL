//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdSaveTotalStatisticsCommand.cpp
///
//==================================================================================

//------------------------------ gdSaveTotalStatisticsCommand.cpp ------------------------------

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
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/commands/gdSaveTotalStatisticsCommand.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdStatisticsView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdTotalStatisticsView.h>


gdSaveTotalStatisticsCommand::gdSaveTotalStatisticsCommand(const osFilePath& fileName)
    : _filePath(fileName), _totalStatisticsOutputString(AF_STR_EmptyA)
{
}

// ---------------------------------------------------------------------------
// Name:        gdSaveTotalStatisticsCommand::~gdSaveTotalStatisticsCommand
// Description: Destructor.
// Author:      Sigal Algranaty
// Date:        26/8/2008
// ---------------------------------------------------------------------------
gdSaveTotalStatisticsCommand::~gdSaveTotalStatisticsCommand()
{
    // Do Nothing...
}

// ---------------------------------------------------------------------------
// Name:        gdSaveTotalStatisticsCommand::canExecuteSpecificCommand
// Description: Answers the question - can we save the function Calls Statistics
// Author:      Sigal Algranaty
// Date:        26/8/2008
// Implementation Notes:
// Currently - the answer is always yes.
// ---------------------------------------------------------------------------
bool gdSaveTotalStatisticsCommand::canExecuteSpecificCommand()
{
    return true;
}

// ---------------------------------------------------------------------------
// Name:        gdSaveTotalStatisticsCommand::executeSpecificCommand
// Description: Save the function calls statistics of the G-Debugger application.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        26/8/2008
// ---------------------------------------------------------------------------
bool gdSaveTotalStatisticsCommand::executeSpecificCommand()
{
    bool rc = false;

    rc = getTotalStatisticsData();
    GT_IF_WITH_ASSERT(rc)
    {
        rc = writeTotalStatisticsDataToFile();
    }

    return rc;
}

// ---------------------------------------------------------------------------
// Name:        gdSaveTotalStatisticsCommand::getTotalStatisticsData
// Description: Get the OpenGL Total Statistics data
//              into _totalStatisticsOutputString
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        26/8/2008
// ---------------------------------------------------------------------------
bool gdSaveTotalStatisticsCommand::getTotalStatisticsData()
{
    bool retVal = true;

    // Get the Function Calls Statistics View:
    gdApplicationCommands* pApplicationCommands = gdApplicationCommands::gdInstance();
    GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
    {
        gdStatisticsView* pStatisticsView = pApplicationCommands->statisticsPanel()->statisticsView();
        GT_IF_WITH_ASSERT(pStatisticsView != NULL)
        {
            gdStatisticsViewBase* pTotalStatisticsView = pStatisticsView->statisticsPage(GD_STATISTICS_VIEW_TOTAL_INDEX);
            GT_IF_WITH_ASSERT(pTotalStatisticsView != NULL)
            {
                // Create table header
                _totalStatisticsOutputString.append(GD_STR_TotalStatisticsViewerColumn1Title);
                _totalStatisticsOutputString.append(",");
                _totalStatisticsOutputString.append(GD_STR_TotalStatisticsViewerColumn2Title);
                _totalStatisticsOutputString.append(",");
                _totalStatisticsOutputString.append(GD_STR_TotalStatisticsViewerColumn3Title);
                _totalStatisticsOutputString.append(",");
                _totalStatisticsOutputString.append(GD_STR_TotalStatisticsViewerColumn4Title);
                _totalStatisticsOutputString.append(",");
                _totalStatisticsOutputString.append(GD_STR_TotalStatisticsViewerColumn5Title);

                // Add a new line (after the table header):
                _totalStatisticsOutputString.append("\n");

                // Now we create the table body, looping through the total statistics
                // list and adding all the list items

                // Loop through all the selected items in the list:
                for (int rowIndex = 0; rowIndex < pTotalStatisticsView->rowCount(); rowIndex++)
                {
                    // Get the function name
                    QString functionTypeName;
                    pTotalStatisticsView->getItemText(rowIndex, 0, functionTypeName);

                    QString tmp;
                    pTotalStatisticsView->getItemText(rowIndex, 1, tmp);
                    gtASCIIString functionTotalPercentage = tmp.toLatin1().data();

                    // get the # of calls data
                    QString functionTotalCallNumber;
                    pTotalStatisticsView->getItemText(rowIndex, 2, functionTotalCallNumber);
                    functionTotalCallNumber.remove(GT_THOUSANDS_SEPARATOR);

                    QString functionFramePercentage;
                    pTotalStatisticsView->getItemText(rowIndex, 3, functionFramePercentage);

                    QString functionFrameCallNumber;
                    pTotalStatisticsView->getItemText(rowIndex, 4, functionFrameCallNumber);
                    functionFrameCallNumber.remove(GT_THOUSANDS_SEPARATOR);

                    _totalStatisticsOutputString.append(functionTypeName.toLatin1().data());
                    _totalStatisticsOutputString.append(",");
                    _totalStatisticsOutputString.append(functionTotalPercentage);

                    if (!functionTotalPercentage.isEmpty() && functionTotalPercentage != AF_STR_NotAvailableA)
                    {
                        _totalStatisticsOutputString.append("%");
                    }

                    _totalStatisticsOutputString.append(",");
                    _totalStatisticsOutputString.append(functionTotalCallNumber.toLatin1().data());
                    _totalStatisticsOutputString.append(",");
                    _totalStatisticsOutputString.append(functionFramePercentage.toLatin1().data());

                    if (!functionFramePercentage.isEmpty() && functionTotalPercentage != AF_STR_NotAvailableA)
                    {
                        _totalStatisticsOutputString.append("%");
                    }

                    _totalStatisticsOutputString.append(",");
                    _totalStatisticsOutputString.append(functionFrameCallNumber.toLatin1().data());

                    // Add a line break between the table body rows
                    _totalStatisticsOutputString.append("\n");
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdSaveTotalStatisticsCommand::writeTotalStatisticsDataToFile
// Description: Write the Function Calls Statistics data into a file
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        26/8/2008
// ---------------------------------------------------------------------------
bool gdSaveTotalStatisticsCommand::writeTotalStatisticsDataToFile()
{
    bool retVal = true;
    gtString fileHeader;
    osTime fileSavedDateAndTime;
    gtString fileSavedDate;
    gtString fileSavedTime;

    osFilePath filePath(_filePath);
    osFile totalStatisticsFile;

    retVal = totalStatisticsFile.open(filePath, osChannel::OS_UNICODE_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);

    if (retVal)
    {
        fileSavedDateAndTime.setFromCurrentTime();

        fileSavedDateAndTime.dateAsString(fileSavedDate, osTime::UNDERSCORE_SAPERATOR, osTime::LOCAL);
        fileSavedDateAndTime.timeAsString(fileSavedTime, osTime::WINDOWS_STYLE, osTime::LOCAL);

        // Get the current project name:
        gtString projectFileName = afProjectManager::instance().currentProjectSettings().projectName();

        fileHeader.append(GD_STR_StatisticsExportFileHeaderSeperator);
        fileHeader.append(GD_STR_StatisticsExportFileHeaderTitlePrefix);
        fileHeader.append(GD_STR_saveTotalStatistics);
        fileHeader.append(GD_STR_StatisticsExportFileHeaderTitlePostfix);
        fileHeader.append(GD_STR_StatisticsExportFileHeaderProjectName) += projectFileName += AF_STR_NewLine;
        fileHeader.append(GD_STR_StatisticsExportFileHeaderGenerationDate) += fileSavedDate += AF_STR_NewLine;
        fileHeader.append(GD_STR_StatisticsExportFileHeaderGenerationTime) += fileSavedTime += L"\n//\n";
        fileHeader.append(GD_STR_StatisticsExportFileHeaderGeneratedBy);
        fileHeader.append(GD_STR_StatisticsExportFileHeaderWebSite);
        fileHeader.append(GD_STR_StatisticsExportFileHeaderSeperator) += AF_STR_NewLine;

        totalStatisticsFile << fileHeader;
        totalStatisticsFile << _totalStatisticsOutputString;
        totalStatisticsFile.close();
    }

    return retVal;
}
