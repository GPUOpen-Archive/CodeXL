//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afOpenCLDeviceInformationCollector.cpp
///
//==================================================================================

// infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// local
#include <AMDTApplicationFramework/Include/commands/afSystemInformationCommand.h>
#include <AMDTApplicationFramework/Include/afOpenCLDeviceInformationCollector.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// ---------------------------------------------------------------------------
afOpenCLDeviceInformationCollector::afOpenCLDeviceInformationCollector()
    : osThread(L"openCLDeviceInfoCollector"), m_isActive(false), m_dataCollectedOk(false)
{
}

// ---------------------------------------------------------------------------
afOpenCLDeviceInformationCollector::~afOpenCLDeviceInformationCollector()
{
}

// ---------------------------------------------------------------------------
int afOpenCLDeviceInformationCollector::entryPoint()
{
    int retVal = 0;
    afSystemInformationCommand sysInfoCmd;

    m_dataCollectedOk = sysInfoCmd.CollectAllOpenCLDevicesInformation(m_openCLDevicesInfoData);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"m_dataCollectedOk = %d, m_openCLDevicesInfoData.size = %d", m_dataCollectedOk, m_openCLDevicesInfoData.size());

    m_isActive = false;

    return retVal;
}

// ---------------------------------------------------------------------------
bool afOpenCLDeviceInformationCollector::StartCollectingInfo()
{
    bool retVal = false;
    m_dataCollectedOk = false;
    m_isActive = false;
    m_openCLDevicesInfoData.clear();

    if (osThread::execute())
    {
        m_dataCollectedOk = true;
        m_isActive = true;
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
bool& afOpenCLDeviceInformationCollector::IsActive()
{
    return m_isActive;
}

// ---------------------------------------------------------------------------
void afOpenCLDeviceInformationCollector::StopCollectingInfo()
{
    if (m_isActive)
    {
        m_dataCollectedOk = false;
        // Terminate thread
        osThread::terminate();
        // Clear data as the thread didn't finish it's work
        m_openCLDevicesInfoData.clear();
        m_isActive = false;
    }
}

// ---------------------------------------------------------------------------
bool afOpenCLDeviceInformationCollector::GetOpenCLDeviceInformation(gtList< gtList <gtString> >& openCLDevicesInfoData)
{
    bool retVal = false;
    openCLDevicesInfoData.clear();

    if (!m_isActive && m_dataCollectedOk)
    {
        openCLDevicesInfoData = m_openCLDevicesInfoData;
        retVal = true;
    }

    return retVal;
}
