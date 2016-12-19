//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuTranslationMonitor.cpp
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/src/CpuTranslationMonitor.cpp#29 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569613 $
//=============================================================

#include <QtCore>
#include <QtWidgets>

// Infra:
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apProfileProgressEvent.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osThread.h>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/SharedProfileManager.h>

// Local:
#include <inc/CpuTranslationMonitor.h>
#include <inc/StringConstants.h>
#include <inc/CpuProjectHandler.h>
#include <inc/Auxil.h>

static void ProgressBarCallback(apProfileProgressEvent& eve)
{
    apEventsHandler::instance().registerPendingDebugEvent(eve);
}

CpuTranslationMonitor::CpuTranslationMonitor(ReaderHandle* pHandle, const gtString& sessionFile, HRESULT* pRet):
    QThread(nullptr), m_pHandle(pHandle), m_sessionFile(sessionFile), m_pRet(pRet)
{
    setObjectName("CpuTranslationThread");
}

CpuTranslationMonitor::~CpuTranslationMonitor()
{
}

void CpuTranslationMonitor::run()
{
    // Mark the profile session processing as in progress (profile actions should be disabled while translating):
    SharedProfileManager::instance().SetProfileSessionProcessingComplete(false);

    apProfileProgressEvent eve(CPU_STR_PROJECT_EXTENSION, L"Preparing raw data translation...", 0);
    apEventsHandler::instance().registerPendingDebugEvent(eve);

    // Write a message to the log:
    gtString message;
    message.appendFormattedString(L"CPU Translate writing to file: %ls", m_sessionFile.asCharArray());
    OS_OUTPUT_DEBUG_LOG(message.asCharArray(), OS_DEBUG_LOG_EXTENSIVE);

    gtString searchPath;
    gtString serverList;
    gtString cachePath;

    AuxGetSymbolSearchPath(searchPath, serverList, cachePath);

    const wchar_t* pSearchPath = (!searchPath.isEmpty()) ? searchPath.asCharArray() : nullptr;
    const wchar_t* pServerList = (!serverList.isEmpty()) ? serverList.asCharArray() : nullptr;
    const wchar_t* pCachePath = (nullptr != pServerList && !cachePath.isEmpty()) ? cachePath.asCharArray() : nullptr;

    *m_pRet = fnCpuProfileDataTranslate(m_pHandle, m_sessionFile.asCharArray(), ProgressBarCallback, pSearchPath, pServerList, pCachePath);

    if (S_OK != *m_pRet)
    {
        gtString msg;
        gtString additional;

        if ((int)E_NODATA == *m_pRet)
        {
            additional = L"\nThere were no data records, was the profiling paused the entire duration?\n";
        }
        else if ((int)E_INVALIDDATA == *m_pRet)
        {
            additional = L"\nThe raw data file may be corrupted.\n";
        }

        msg.appendFormattedString(L"Could not write the profile data file (%ls). %ls",
                                  m_sessionFile.asCharArray(), additional.asCharArray());
        OS_OUTPUT_DEBUG_LOG(msg.asCharArray(), OS_DEBUG_LOG_ERROR);
        eve.setProgress(msg);
        eve.setAborted(true);
        apEventsHandler::instance().registerPendingDebugEvent(eve);
        fnCloseProfile(&m_pHandle);
        CpuProjectHandler::instance().emitFileImportedComplete();
    }
    else
    {
        *m_pRet = fnCloseProfile(&m_pHandle);
    }

    //Send finish percentage
    eve.setProgress(L"");
    eve.setValue(100);
    eve.setAborted(true);
    apEventsHandler::instance().registerPendingDebugEvent(eve);

    // Mark the profile session processing as completed:
    SharedProfileManager::instance().SetProfileSessionProcessingComplete(true);

    exec();
}
