//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ProfileConfigs.h
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/inc/ProfileConfigs.h#14 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569613 $
//=============================================================
#ifndef _PROFILECONFIGS_H
#define _PROFILECONFIGS_H

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtMap.h>

//DcConfig
#include <AMDTCpuPerfEventUtils/inc/DcConfig.h>

//Predeclarations
class CPUSessionTreeItemData;
class osCpuid;

const gtString CUSTOM_PROFILE_NAME(L"Custom Profile");
const gtString IBS_PROFILE_NAME(L"Instruction-based Sampling");
const gtString TBP_PROFILE_NAME(L"Time-based Sampling");
const gtString CLU_PROFILE_NAME(L"Cache Line Utilization");
const float DEFAULT_TIMER_INTERVAL = 1.0; //1 mS

class ProfileConfigs
{
public:
    /// Get the singleton instance
    static ProfileConfigs& instance();
    virtual ~ProfileConfigs();

    void readAvailableProfiles();
    gtVector<gtString> getListOfProfiles();


    bool getProfileConfigByType(const gtString& profileSessionType, CPUSessionTreeItemData* pProfileSessionData);

    void setCustomConfig(const CPUSessionTreeItemData* pConfig);
    gtString getProfileDescription(gtString name);

    /// returns a profile type according to the event configuration list provided
    /// \param[in] eventConfigVec - the list contains the event masks and their sampling period
    gtString getProfileTypeByEventConfigs(const EventEncodeVec& eventConfigVec);

protected:
    /// Protected creator
    ProfileConfigs();

    /// Adds a profile to the configuration list
    bool AddConfiguration(const osFilePath& configPath, osCpuid* pCpuInfo);

    /// Returns true if the profile type is 'Instruction-based Sampling' according to the
    /// event configuration list, false otherwise
    bool isProfileTypeIBS(DcConfig* pConfig, const EventEncodeVec& eventConfigVec);

    /// Returns true if the profile type is 'Time-based Sampling' according to the
    /// event configuration list, false otherwise
    bool isProfileTypeTBP(DcConfig* pConfig, const EventEncodeVec& eventConfigVec);

    void loadCustomConfig();

    /// The singleton instance
    static ProfileConfigs* m_pMySingleInstance;

    typedef gtMap<gtString, DcConfig*> ConfigMap;
    ConfigMap m_configs;
};

#endif //_PROFILECONFIGS_H
