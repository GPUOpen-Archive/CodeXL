//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdSaveProfilingDataCommand.cpp
///
//==================================================================================

//------------------------------ gdSaveProfilingDataCommand.cpp ------------------------------

// Qt
#include <QtWidgets>

// Infra
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTAPIClasses/Include/apProjectSettings.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationComponents/Include/acWXListCtrl.h>
#include <AMDTApplicationComponents/Include/acMultiSamplesGraph.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdPerformanceCountersTimer.h>
#include <AMDTGpuDebuggingComponents/Include/gdAppWindowsManger.h>
#include <AMDTGpuDebuggingComponents/Include/commands/gdSaveProfilingDataCommand.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdPerformanceGraphView.h>

gdSaveProfilingDataCommand::gdSaveProfilingDataCommand(const osFilePath& fileName)
    : _filePath(fileName), _profilingDataOutputString(AF_STR_Empty)
{
}

// ---------------------------------------------------------------------------
// Name:        gdSaveProfilingDataCommand::~gdSaveProfilingDataCommand
// Description: Destructor.
// Author:      Avi Shapira
// Date:        10/1/2007
// ---------------------------------------------------------------------------
gdSaveProfilingDataCommand::~gdSaveProfilingDataCommand()
{
}

// ---------------------------------------------------------------------------
// Name:        gdSaveProfilingDataCommand::canExecuteSpecificCommand
// Description: Answers the question - can we save the state variables of the CodeXL application.
// Author:      Avi Shapira
// Date:        10/11/2003
// ---------------------------------------------------------------------------
bool gdSaveProfilingDataCommand::canExecuteSpecificCommand()
{
    bool retVal = canExecuteSaveProfilingDataCommand();
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdSaveProfilingDataCommand::canExecuteSaveProfilingDataCommand
// Description:  Answers the question - can we save the state variables of the CodeXL application.
// Author:      Yaki Tebeka
// Date:        7/1/2010
// ---------------------------------------------------------------------------
bool gdSaveProfilingDataCommand::canExecuteSaveProfilingDataCommand()
{
    bool retVal = true;

    // Check if there is already a debugged process running:
    bool debuggedProcessExists = gaDebuggedProcessExists();

    if (debuggedProcessExists)
    {
        // This command cannot be run when the debugged process is running:
        bool processSuspended = gaIsDebuggedProcessSuspended();

        if (!processSuspended)
        {
            retVal = false;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdSaveProfilingDataCommand::executeSpecificCommand
// Description: Save the state varibles of the G-Debugger application.
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        10/11/2003
// ---------------------------------------------------------------------------
bool gdSaveProfilingDataCommand::executeSpecificCommand()
{
    bool rc = false;

    rc = getProfilingData();
    rc = rc && writeProfilingDataToFile();

    return rc;
}

// ---------------------------------------------------------------------------
// Name:        gdSaveProfilingDataCommand::getProfilingData
// Description: Get the OpenGL Profiling data into _profilingDataOutputString
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        2/8/2004
// ---------------------------------------------------------------------------
bool gdSaveProfilingDataCommand::getProfilingData()
{
    bool retVal = true;
    _profilingDataOutputString.makeEmpty();

    // Get the performance timer interval in mSec:
    gdPerformanceCountersTimer& thePeformanceCountersTimer = gdPerformanceCountersTimer::instance();
    float timerInterval = thePeformanceCountersTimer.timerInterval();

    // Get the Performance Graph View pointer:
    gdAppWindowsManger& theAppWindowsMgr = gdAppWindowsManger::instance();
    gdPerformanceGraphView* pPerformanceGraphView = (gdPerformanceGraphView*)(theAppWindowsMgr.getAppWindow(ID_PERFORMANCE_GRAPH_VIEW));
    GT_IF_WITH_ASSERT(pPerformanceGraphView != NULL)
    {
        // Get the Multi Sample Graph:
        const acMultiSamplesGraph* pMultiSampleGraph = pPerformanceGraphView->multiSampleGraph();

        if (pMultiSampleGraph)
        {
            // Get the amount of graphs:
            int amountOfGraphs = pMultiSampleGraph->amountOfSampleGraphs();

            // If we have graphs:
            if (amountOfGraphs > 0)
            {
                //////////////////////////////////////////////////////////////////////////
                // Add the Graph counters title
                //////////////////////////////////////////////////////////////////////////

                // Add the Time (Title):
                _profilingDataOutputString.append(L"Time (sec),");

                // Run on the Graphs - Add the Graph Counters names:
                for (int graphIndex = 0; graphIndex < amountOfGraphs; graphIndex++)
                {
                    // Get the List Ctrl:
                    const acWXListCtrl* pRightSideListCtrl = pPerformanceGraphView->rightSideListCtrl();

                    // Get the current displayed counter info (user data):
                    const gdPerformanceCounterData* pCurDisplayedCounterInfo = (gdPerformanceCounterData*)(pRightSideListCtrl->GetItemData(graphIndex));

                    if (pCurDisplayedCounterInfo)
                    {
                        // Get the counter ID:
                        apCounterID counterID = pCurDisplayedCounterInfo->_counterID;
                        int counterGlobalIndex = -1;
                        retVal = retVal && gaGetPerformanceCounterIndex(counterID, counterGlobalIndex);
                        GT_IF_WITH_ASSERT(retVal)
                        {
                            // Get the performance counter information:
                            const apCounterInfo* pCounterInfo = gaGetPerformanceCounterInfo(counterGlobalIndex);

                            if (pCounterInfo)
                            {
                                // Get the counter graph name:
                                gtString graphName = pCounterInfo->_name;

                                // For per context counters, append context index:
                                if (pCounterInfo->_counterScopeType == apCounterScope::AP_CONTEXT_COUNTER)
                                {
                                    int pos = graphName.find(L"%d");

                                    if (pos >= 0)
                                    {
                                        gtString graphGenericName = graphName;
                                        graphName = AF_STR_Empty;
                                        // Counter name is generic, and context index append is needed
                                        graphName.appendFormattedString(graphGenericName.asCharArray(), counterID._counterScope._contextID._contextId);
                                    }
                                }
                                else if (pCounterInfo->_counterScopeType == apCounterScope::AP_QUEUE_COUNTER)
                                {
                                    int pos1 = graphName.find(L"%d");
                                    int pos2 = graphName.find(L"%d", pos1 + 1);

                                    if ((pos1 >= 0) && (pos2 >= 0))
                                    {
                                        // Counter name is generic, and context index append is needed:
                                        gtString graphGenericName = graphName;
                                        graphName = AF_STR_Empty;
                                        graphName.appendFormattedString(graphGenericName.asCharArray(), counterID._counterScope._contextID._contextId, counterID._counterScope._queueId);
                                    }
                                }

                                // Add the graph name (Title):
                                _profilingDataOutputString.append(graphName);
                            }
                        }
                    }

                    // We are not at the last graph
                    if (graphIndex != (amountOfGraphs - 1))
                    {
                        // Add a comma
                        _profilingDataOutputString.append(L",");
                    }
                }

                // Add a new line (after the title):
                _profilingDataOutputString.append(AF_STR_NewLine);

                //////////////////////////////////////////////////////////////////////////
                // Add the Graph counters data
                //////////////////////////////////////////////////////////////////////////
                // Get the graph id of the first graph:
                int firstGraphId = pMultiSampleGraph->sampleGraphId(0);
                // Get the sample amount of the first graph:
                int firstGraphSampleAmount = pMultiSampleGraph->samplesAmount(firstGraphId);

                float timerIntervalSecond = timerInterval / 1000;

                // Run on the samples (lines in the output file):
                for (int sampleIndex = 0; sampleIndex < firstGraphSampleAmount; sampleIndex++)
                {
                    // Add the sample time:
                    float sampleTimeInt = (sampleIndex * timerIntervalSecond);

                    // Add the sample time:
                    _profilingDataOutputString.appendFormattedString(L"%.2f,", sampleTimeInt);

                    // Run on the Graphs (Columns in the output file):
                    for (int graphIndex = 0; graphIndex < amountOfGraphs; graphIndex++)
                    {
                        // Get the graph Id (from the index):
                        int graphId = pMultiSampleGraph->sampleGraphId(graphIndex);

                        double sampleValue = 0;
                        // Get the Sample of the graph:
                        bool rc = pMultiSampleGraph->getSample(graphId, sampleIndex, sampleValue);

                        // If we got the sample:
                        if (rc)
                        {
                            _profilingDataOutputString.appendFormattedString(L"%.0f", sampleValue);
                        }
                        else
                        {
                            _profilingDataOutputString.append(AF_STR_NotAvailable);
                        }

                        // We are not at the last graph
                        if (graphIndex != (amountOfGraphs - 1))
                        {
                            // Add a comma
                            _profilingDataOutputString.append(L",");
                        }
                    }

                    // Add a new line:
                    _profilingDataOutputString.append(AF_STR_NewLine);
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdSaveProfilingDataCommand::writeProfilingDataToFile
// Description: Write the Profiling data into a file
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        2/8/2004
// ---------------------------------------------------------------------------
bool gdSaveProfilingDataCommand::writeProfilingDataToFile()
{
    bool retVal = true;

    // Open the file for writing:
    osFile stateVarFile;
    retVal = stateVarFile.open(_filePath, osChannel::OS_UNICODE_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);

    if (retVal)
    {
        // Get the date and time:
        osTime fileSavedDateAndTime;
        fileSavedDateAndTime.setFromCurrentTime();
        gtString fileSavedDate;
        gtString fileSavedTime;
        fileSavedDateAndTime.dateAsString(fileSavedDate, osTime::UNDERSCORE_SAPERATOR, osTime::LOCAL);
        fileSavedDateAndTime.timeAsString(fileSavedTime, osTime::WINDOWS_STYLE, osTime::LOCAL);

        // Get the current project name:
        gtString projectFileName = afProjectManager::instance().currentProjectSettings().projectName();

        // Construct the file header:
        gtString fileHeader;
        fileHeader.append(L"/////////////////////////////////////////////////////////////////////////////////");
        fileHeader.append(AF_STR_NewLine);
        fileHeader.append(L"// This File contains OpenGL Profiling Data information\n");
        fileHeader.append(L"// Project name: ") += projectFileName += AF_STR_NewLine;
        fileHeader.append(L"// Generation date: ") += fileSavedDate += AF_STR_NewLine;
        fileHeader.append(L"// Generation time: ") += fileSavedTime += AF_STR_NewLine;
        fileHeader.append(L"//\n");
        fileHeader.append(L"// Generated by CodeXL - an OpenCL and OpenGL Debugger\n");
        fileHeader.append(L"// http://gpuopen.com/\n");
        fileHeader.append(L"/////////////////////////////////////////////////////////////////////////////////\n\n");

        // Write the string to the file:
        stateVarFile << fileHeader;
        stateVarFile << _profilingDataOutputString;
        stateVarFile.close();
    }

    return retVal;
}
