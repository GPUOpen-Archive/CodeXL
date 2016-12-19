//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ProfileConfigs.cpp
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/src/ProfileConfigs.cpp#33 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569613 $
//=============================================================

// Qt:
#include <QtCore>
#include <QtWidgets>

// AMDTBaseTools
#include <AMDTBaseTools/Include/gtAssert.h>

//AMDTOSWrappers
#include <AMDTOSWrappers/Include/osMachine.h>
#include <AMDTOSWrappers/Include/osCpuid.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Infra:
#include <AMDTApplicationFramework/Include/afAidFunctions.h>

// Backend:
#include <AMDTCpuProfilingControl/inc/CpuProfileControl.h>
#include <AMDTCpuPerfEventUtils/inc/EventEngine.h>

// Local:
#include <inc/ProfileConfigs.h>
#include <inc/CpuProjectHandler.h>
#include "inc/StdAfx.h"

//static declarations
ProfileConfigs* ProfileConfigs::m_pMySingleInstance = nullptr;


ProfileConfigs& ProfileConfigs::instance()
{
    // If this class single instance was not already created:
    if (nullptr == m_pMySingleInstance)
    {
        // Create it:
        m_pMySingleInstance = new ProfileConfigs;
        GT_ASSERT(m_pMySingleInstance);
        m_pMySingleInstance->readAvailableProfiles();
    }

    return *m_pMySingleInstance;
}

ProfileConfigs::~ProfileConfigs()
{
    ConfigMap::iterator it = m_configs.begin();
    ConfigMap::iterator itEnd = m_configs.end();

    //Free the memory allocated for each profile configuration
    for (; it != itEnd; it++)
    {
        if (nullptr != it->second)
        {
            delete it->second ;
        }
    }
}

void ProfileConfigs::readAvailableProfiles()
{
    osFilePath basePath(osFilePath::OS_CODEXL_DATA_PATH);

    basePath.appendSubDirectory(L"Profiles");
    osDirectory baseDir(basePath);
    osCpuid cpuInfo;

    //If !APIC, no profiling available
    if (!cpuInfo.hasLocalApic())
    {
        return;
    }

    gtList<osFilePath> filePaths;
    gtList<osFilePath>::const_iterator it;
    gtList<osFilePath>::const_iterator endIt;

    //Read TBP
    if (baseDir.getContainedFilePaths(L"*.xml", osDirectory::SORT_BY_NAME_DESCENDING, filePaths))
    {
        it = filePaths.begin();
        endIt = filePaths.end();

        for (; it != endIt; it++)
        {
            osFilePath test(*it);
            AddConfiguration(test, &cpuInfo);
        }
    }

    loadCustomConfig();

    //If AMD, check in current family_model directory for Event configurations
    if (!cpuInfo.isCpuAmd())
    {
        return;
    }

    bool isPredefinedProfilesSupported = false;
    fnGetPredefinedProfilesAvailable(isPredefinedProfilesSupported);

    if (!isPredefinedProfilesSupported)
    {
        return;
    }

    gtString subName;

    //If the model mask is needed
    if (cpuInfo.getFamily() >= FAMILY_OR)
    {
        //since the model is like 0x10-1f, just need the mask (like 0x10), so shift right by 4 bits
        subName.appendFormattedString(L"0x%x_0x%x", cpuInfo.getFamily(), (cpuInfo.getModel() >> 4));
    }
    else
    {
        subName.appendFormattedString(L"0x%x", cpuInfo.getFamily());
    }

    basePath.appendSubDirectory(subName);
    baseDir.setDirectoryPath(basePath);

    if (baseDir.exists())
    {
        gtList<osFilePath> filePathsList;
        gtList<osFilePath>::const_iterator iterFilePath;
        gtList<osFilePath>::const_iterator endItFilePath;

        if (baseDir.getContainedFilePaths(L"*.xml", osDirectory::SORT_BY_NAME_DESCENDING, filePathsList))
        {
            iterFilePath = filePathsList.begin();
            endItFilePath = filePathsList.end();

            for (; iterFilePath != endItFilePath; iterFilePath++)
            {
                AddConfiguration(*iterFilePath, &cpuInfo);
            }
        }
    }
}

bool ProfileConfigs::AddConfiguration(const osFilePath& configPath, osCpuid* pCpuInfo)
{
    bool bRet = true;
    DcConfig*   pTemp = new DcConfig();


    if (!pTemp->ReadConfigFile(configPath.asString().asCharArray()))
    {
        bRet = false;
    }

    if (bRet)
    {
        DcConfigType configType = pTemp->GetConfigType();
        bRet = ((pCpuInfo->isIbsAvailable()) || ((DCConfigIBS != configType) && (DCConfigCLU != configType)));
    }

    if (bRet)
    {
        QString profileName;
        pTemp->GetConfigName(profileName);
        gtString gName = acQStringToGTString(profileName);
        ConfigMap::const_iterator itFind = m_configs.find(gName);

        if (itFind == m_configs.end())
        {
            m_configs[gName] = pTemp;
        }
        else
        {
            bRet = false;
        }
    }

    if (!bRet)
    {
        delete pTemp;
    }

    return bRet;
}

gtVector<gtString> ProfileConfigs::getListOfProfiles()
{
    gtVector<gtString> retList;
    ConfigMap::const_iterator it = m_configs.begin();
    ConfigMap::const_iterator itEnd = m_configs.end();

    //Free the memory allocated for each profile configuration
    for (; it != itEnd; it++)
    {
        // was emplace_back() but g++/Linux seems to not have emplace_back()
        retList.push_back(it->first);
    }

    return retList;
}

bool ProfileConfigs::getProfileConfigByType(const gtString& profileSessionType, CPUSessionTreeItemData* pProfileSessionData)
{
    bool bRet = false;

    if (nullptr != pProfileSessionData)
    {
        ConfigMap::const_iterator itFind = m_configs.find(profileSessionType);

        if (itFind != m_configs.end())
        {
            osGetAmountOfLocalMachineCPUs(pProfileSessionData->m_cores);
            pProfileSessionData->m_profileTypeStr = acGTStringToQString(profileSessionType);
            //TBP
            pProfileSessionData->m_msInterval = itFind->second->GetTimerInterval();

            //EBP
            itFind->second->GetEventInfo(pProfileSessionData->m_eventsVector);

            //IBS
            IbsConfig config;
            itFind->second->GetIBSInfo(&config) ;
            pProfileSessionData->m_fetchSample = config.fetchSampling;
            pProfileSessionData->m_opSample = config.opSampling;
            pProfileSessionData->m_opCycleCount = config.opCycleCount;
            pProfileSessionData->m_fetchInterval = config.fetchMaxCount;
            pProfileSessionData->m_opInterval = config.opMaxCount;

            //CLU
            CluConfig cluConfig;
            itFind->second->GetCLUInfo(&cluConfig) ;
            pProfileSessionData->m_cluSample = cluConfig.cluSampling;
            pProfileSessionData->m_cluCycleCount = cluConfig.cluCycleCount;
            pProfileSessionData->m_cluInterval = cluConfig.cluMaxCount;

            bRet = true;
        }
    }

    return bRet;
}

gtString ProfileConfigs::getProfileDescription(gtString name)
{
    gtString strRet;
    ConfigMap::const_iterator itFind = m_configs.find(name);

    if (itFind != m_configs.end())
    {
        QString description;
        itFind->second->GetDescription(description);
        strRet = acQStringToGTString(description);
    }

    return strRet;
}

ProfileConfigs::ProfileConfigs()
{
}

void ProfileConfigs::setCustomConfig(const CPUSessionTreeItemData* pConfig)
{
    ConfigMap::iterator it = m_configs.find(CUSTOM_PROFILE_NAME);
    //Timer part
    it->second->SetTimerInterval(pConfig->m_msInterval);
    //Ibs part
    IbsConfig tempIbs;
    tempIbs.fetchMaxCount = pConfig->m_fetchInterval;
    tempIbs.fetchSampling = pConfig->m_fetchSample;
    tempIbs.opCycleCount = pConfig->m_opCycleCount;
    tempIbs.opMaxCount = pConfig->m_opInterval;
    tempIbs.opSampling = pConfig->m_opSample;
    it->second->SetIBSInfo(&tempIbs);
    //Clu part
    CluConfig tempClu;
    tempClu.cluCycleCount = pConfig->m_cluCycleCount;
    tempClu.cluMaxCount = pConfig->m_cluInterval;
    tempClu.cluSampling = pConfig->m_cluSample;
    it->second->SetCLUInfo(&tempClu);
    //Event part
    it->second->SetEventInfo(pConfig->m_eventsVector);

    //Get the path
    osFilePath configDirPath;
    afGetUserDataFolderPath(configDirPath);
    configDirPath.appendSubDirectory(L"Profiles");
    configDirPath.setFileName(CUSTOM_PROFILE_NAME);
    configDirPath.setFileExtension(L"xml");
    //Ensure directory exists
    osDirectory existCheck;
    configDirPath.getFileDirectory(existCheck);

    if (!existCheck.exists())
    {
        existCheck.create();
    }

    osFilePath basePath(osFilePath::OS_CODEXL_DATA_PATH);

    basePath.appendSubDirectory(L"Profiles");

    it->second->WriteConfigFile(configDirPath.asString().asCharArray(),
                                basePath.asString().asCharArray());
}

void ProfileConfigs::loadCustomConfig()
{
    osFilePath configDirPath;
    afGetUserDataFolderPath(configDirPath);
    configDirPath.appendSubDirectory(L"Profiles");
    configDirPath.setFileName(CUSTOM_PROFILE_NAME);
    configDirPath.setFileExtension(L"xml");

    DcConfig* pTemp = new DcConfig();


    if (!pTemp->ReadConfigFile((wchar_t*)configDirPath.asString().asCharArray()))
    {
        //Ensure directory exists
        osDirectory existCheck;
        configDirPath.getFileDirectory(existCheck);

        if (!existCheck.exists())
        {
            existCheck.create();
        }

        //Create new, with only a timer event
        pTemp->SetTimerInterval(DEFAULT_TIMER_INTERVAL);
        pTemp->SetConfigType(DCConfigMultiple);
        pTemp->SetConfigName(QString::fromWCharArray(CUSTOM_PROFILE_NAME.asCharArray()));
    }

    m_configs[CUSTOM_PROFILE_NAME] = pTemp;
}

bool ProfileConfigs::isProfileTypeIBS(DcConfig* pConfig, const EventEncodeVec& eventConfigVec)
{
    bool allIBS = true;
    bool foundIBSFetch = false;
    gtUInt64 IBSFetchCount = 0;
    bool foundIBSOp = false;
    gtUInt64 IBSOpCount = 0;
    gtUInt16 mask = GT_INT16_MAX;

    for (unsigned int i = 0; i < eventConfigVec.size(); i++)
    {
        // Extract out the 16 bit event-id out of event mask
        gtUInt16 event = eventConfigVec[i].eventMask & mask;

        if ((!IsIbsFetchEvent(event)) && (!IsIbsOpEvent(event)))
        {
            allIBS = false;
            break;
        }

        if (event == GetIbsFetchEvent())
        {
            foundIBSFetch = true;
            IBSFetchCount = eventConfigVec[i].eventCount;
        }

        if (event == GetIbsOpEvent())
        {
            foundIBSOp = true;
            IBSOpCount = eventConfigVec[i].eventCount;
        }
    }

    IbsConfig ibsInfo;
    pConfig->GetIBSInfo(&ibsInfo);

    // verify:
    // 1. All events in the event vector, are either IBS_FETCH or IBS_OP
    // 2. Presence of IBS_FETCH and IBS_OP and their sampling periods
    // 3. TODO: Whether the IBS_OP is by cycle or dispatch (have to modify CAProfileInfo, probably will do it later)
    return (allIBS && (foundIBSFetch == ibsInfo.fetchSampling)
            && (IBSFetchCount == ibsInfo.fetchMaxCount)
            && (foundIBSOp == ibsInfo.opSampling)
            && (IBSOpCount == ibsInfo.opMaxCount));
}

bool ProfileConfigs::isProfileTypeTBP(DcConfig* pConfig, const EventEncodeVec& eventConfigVec)
{
    if (eventConfigVec.size() != 1)
    {
        return false;
    }

    // Extract out the 16 bit event-id out of event mask
    gtUInt16 mask = GT_INT16_MAX;
    gtUInt16 event = eventConfigVec[0].eventMask & mask;

    // verify:
    // event is timer event
    // time interval (event vector has the interval in the unit of 0.1 ms)
    return ((event == GetTimerEvent())
            && (eventConfigVec[0].eventCount == (pConfig->GetTimerInterval() * 10)));
}

gtString ProfileConfigs::getProfileTypeByEventConfigs(const EventEncodeVec& eventConfigVec)
{
    gtString profileType;
    bool foundOne = false;

    ConfigMap::const_iterator iter = m_configs.begin();
    ConfigMap::const_iterator iterEnd = m_configs.end();

    // go through the configuration and compare until you find the right configuration:
    for (; iter != iterEnd; iter++)
    {
        if (iter->first.compareNoCase(CUSTOM_PROFILE_NAME) == 0)
        {
            continue;
        }

        DcConfig* pConfig = iter->second;

        GT_IF_WITH_ASSERT(pConfig != nullptr)
        {
            bool isEqual = false;

            if (iter->first.compareNoCase(TBP_PROFILE_NAME) == 0)
            {
                isEqual = isProfileTypeTBP(pConfig, eventConfigVec);
            }
            else if (iter->first.compareNoCase(IBS_PROFILE_NAME) == 0)
            {
                isEqual = isProfileTypeIBS(pConfig, eventConfigVec);
            }
            else
            {
                // Get the current number of events
                unsigned int numberOfEvents = pConfig->GetNumberOfEvents();

                if ((numberOfEvents > 0) && (numberOfEvents == eventConfigVec.size()))
                {
                    // Allocate a pointer for the events config
                    DcEventConfig* pEventsConfig = new DcEventConfig[numberOfEvents];


                    // Get the events info
                    pConfig->GetEventInfo(pEventsConfig, numberOfEvents);
                    GT_IF_WITH_ASSERT(pEventsConfig != nullptr)
                    {
                        // Verify: all the eventmasks and their sampling period
                        DcEventConfig* pCurrentConfig = pEventsConfig;

                        for (unsigned int i = 0 ; i < numberOfEvents; i++)
                        {
                            isEqual = false;
                            EventMaskType eventMask = EncodeEvent(pCurrentConfig->pmc);

                            for (unsigned int j = 0; j < numberOfEvents; j++)
                            {
                                isEqual = (eventMask == eventConfigVec[j].eventMask)
                                          && (pCurrentConfig->eventCount == eventConfigVec[j].eventCount);

                                if (isEqual)
                                {
                                    break;
                                }
                            }

                            if (!isEqual)
                            {
                                break;
                            }

                            pCurrentConfig++;
                        }
                    }
                }
            }

            if (isEqual)
            {
                profileType = iter->first;
                foundOne = true;
                break;
            }
        }
    }

    if (!foundOne)
    {
        profileType = CUSTOM_PROFILE_NAME;
    }

    return profileType;
}