//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief GlobalSetting class used by all profiler agents
//==============================================================================


#ifndef _GLOBAL_SETTINGS_H_
#define _GLOBAL_SETTINGS_H_

#include "Defs.h"

/// This is a singleton class that maintain all global settings
class GlobalSettings
{
public:
    /// static public accessor
    static GlobalSettings* GetInstance()
    {
        return &m_sInstance;
    }

private:

    /// private Constructor
    GlobalSettings()
    {
        m_bVerbose = false;
    }

    /// disable copy constructor
    GlobalSettings(const GlobalSettings& obj);
    /// disable assignment op
    GlobalSettings& operator = (const GlobalSettings& obj);

public:
    bool m_bVerbose;                       ///< Verbose setting
    Parameters m_params;                   ///< parameters

private:
    static GlobalSettings m_sInstance;     ///< private static instance
};

#endif //_GLOBAL_SETTINGS_H_
