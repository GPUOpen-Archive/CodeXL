//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpOverview.cpp
///
//==================================================================================

//------------------------------ tpOverview.cpp ------------------------------

// Qt
#include <qtIgnoreCompilerWarnings.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTApplicationComponents/Include/acQHTMLWindow.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>

// AMDTCommonHeaders:
#include <AMDTCommonHeaders/AMDTDefinitions.h>


// Local:
#include <inc/StringConstants.h>
#include <inc/tpOverview.h>

#define TOP_THREADS_NUM 5

tpOverview::tpOverview(QWidget* pParent, tpSessionData* pSessionData, tpSessionTreeNodeData* pSessionTreeData) :
    acSplitter(Qt::Vertical, pParent),
    m_pThreadsExecutionTimeTable(nullptr),
    m_pSessionData(pSessionData),
    m_pHTMLView(nullptr)
{
    // Allocate and fill the HTML window:
    m_pHTMLView = new acQHTMLWindow(nullptr);
    FillHTMLWindow(pSessionTreeData);

    // Create the threads table:
    m_pThreadsExecutionTimeTable = new acListCtrl(nullptr);

    // Initialize the headers:
    QStringList headers;
    headers << CP_STR_ThreadsOverviewTableProcess;
    headers << CP_STR_ThreadsOverviewTablePID;
    headers << CP_STR_ThreadsOverviewTableTID;
    headers << CP_STR_ThreadsOverviewTableExecutionTime;
    m_pThreadsExecutionTimeTable->initHeaders(headers, false);

    addWidget(m_pThreadsExecutionTimeTable);
    addWidget(m_pHTMLView);


    // Get the vector containing the requested top threads:
    QVector<AMDTThreadId> topThreadsVector;
    m_pSessionData->GetTopExecTimeThreads(topThreadsVector, TOP_THREADS_NUM);

    // for each thread in map add a row into table
    QVector<AMDTThreadId>::const_iterator it = topThreadsVector.constBegin();

    for (; it != topThreadsVector.constEnd(); it++)
    {
        AMDTThreadId threadId = *it;
        AMDTProcessId processId = 0;
        QString processName;
        AMDTUInt64 threadExecTime = 0;

        if (m_pSessionData->GetThreadData(threadId, processId, processName, threadExecTime))
        {
            QStringList currentThreadList;
            // insert process name
            currentThreadList << processName;

            // insert process id to table
            currentThreadList << QString("%1").arg(processId);

            // insert thread id to table
            currentThreadList << QString("%1").arg(threadId);

            // insert thread total execution time to table
            currentThreadList << QString("%1").arg(threadExecTime);

            m_pThreadsExecutionTimeTable->addRow(currentThreadList, nullptr);
        }
    }
}

tpOverview::~tpOverview()
{

}


void tpOverview::FillHTMLWindow(tpSessionTreeNodeData* pSessionTreeData)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pHTMLView != nullptr && pSessionTreeData != nullptr)
    {
        gtString htmlStr;
        gtString htmlStr2;
        afHTMLContent content;

        // exec header
        content.addHTMLItem(afHTMLContent::AP_HTML_TITLE, CP_STR_InfoViewExecutionHeader);

        // target path
        gtString htmlLineStr;
        htmlLineStr.appendFormattedString(CP_STR_InfoViewViewHTMLLineStructure, CP_STR_InfoViewTargetPath, acQStringToGTString(pSessionTreeData->m_exeFullPath).asCharArray());
        content.addHTMLItem(afHTMLContent::AP_HTML_LINE, htmlLineStr);

        // working dir
        htmlLineStr = L"";
        htmlLineStr.appendFormattedString(CP_STR_InfoViewViewHTMLLineStructure, CP_STR_InfoViewWorkingDirectory, acQStringToGTString(pSessionTreeData->m_workingDirectory).asCharArray());
        content.addHTMLItem(afHTMLContent::AP_HTML_LINE, htmlLineStr);

        // data folder
        htmlLineStr = L"";
        htmlLineStr.appendFormattedString(CP_STR_InfoViewViewHTMLLineStructure, CP_STR_InfoViewDataFolder, pSessionTreeData->SessionDir().directoryPath().asString().asCharArray());
        content.addHTMLItem(afHTMLContent::AP_HTML_LINE, htmlLineStr);

        // command line arguments
        htmlLineStr = L"";
        htmlLineStr.appendFormattedString(CP_STR_InfoViewViewHTMLLineStructure, CP_STR_InfoViewCommandLineaArguments, acQStringToGTString(pSessionTreeData->m_commandArguments).asCharArray());
        content.addHTMLItem(afHTMLContent::AP_HTML_LINE, htmlLineStr);

        // Environment Variables
        htmlLineStr = L"";
        htmlLineStr.appendFormattedString(CP_STR_InfoViewViewHTMLLineStructure, CP_STR_InfoViewEnvironmentVariables, pSessionTreeData->m_envVariables.asCharArray());
        content.addHTMLItem(afHTMLContent::AP_HTML_LINE, htmlLineStr);

        // Call stack sampling
        htmlLineStr = L"";
        htmlLineStr.appendFormattedString(CP_STR_InfoViewViewHTMLLineStructure, CP_STR_InfoViewCallStackSampling, L"");
        content.addHTMLItem(afHTMLContent::AP_HTML_LINE, htmlLineStr);

        // Stack unwind depth
        htmlLineStr = L"";
        htmlLineStr.appendFormattedString(CP_STR_InfoViewViewHTMLLineStructure, CP_STR_InfoViewStackUnwindDepth, L"");
        content.addHTMLItem(afHTMLContent::AP_HTML_LINE, htmlLineStr);

        // Profile details header
        content.addHTMLItem(afHTMLContent::AP_HTML_TITLE, CP_STR_InfoViewProfileDetailsHeader);

        // Profile session type
        htmlLineStr = L"";
        htmlLineStr.appendFormattedString(CP_STR_InfoViewViewHTMLLineStructure, CP_STR_InfoViewProfileSessionType, CP_STR_ThreadProfileName);
        content.addHTMLItem(afHTMLContent::AP_HTML_LINE, htmlLineStr);

        // Profile started
        htmlLineStr = L"";
        htmlLineStr.appendFormattedString(CP_STR_InfoViewViewHTMLLineStructure, CP_STR_InfoViewProfileStarted, acQStringToGTString(pSessionTreeData->m_startTime).asCharArray());

        // Total processes in Profile
        htmlStr2 = L"";
        gtString gtStrTotal;
        gtStrTotal.appendFormattedString(L"%d", m_pSessionData->TotalProcessesCount());
        htmlStr2.appendFormattedString(CP_STR_InfoViewViewHTMLLineStructure, CP_STR_InfoViewTotalProcesses, gtStrTotal.asCharArray());
        content.addHTMLItem(afHTMLContent::AP_HTML_LINE, htmlLineStr, htmlStr2);

        // Profile ended
        htmlLineStr = L"";
        htmlLineStr.appendFormattedString(CP_STR_InfoViewViewHTMLLineStructure, CP_STR_InfoViewProfileEnded, acQStringToGTString(pSessionTreeData->m_endTime).asCharArray());

        // Total threads in profile
        htmlStr2 = L"";
        gtStrTotal = L"";
        gtStrTotal.appendFormattedString(L"%d", m_pSessionData->TotalThreadsCount());
        htmlStr2.appendFormattedString(CP_STR_InfoViewViewHTMLLineStructure, CP_STR_InfoViewTotalThreads, gtStrTotal.asCharArray());
        content.addHTMLItem(afHTMLContent::AP_HTML_LINE, htmlLineStr, htmlStr2);

        // Get the HTML as string:
        content.toString(htmlStr);

        // Convert the HTML items into a string:
        m_pHTMLView->setHtml(acGTStringToQString(htmlStr));
    }
}

