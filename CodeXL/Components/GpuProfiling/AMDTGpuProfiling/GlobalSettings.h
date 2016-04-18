//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/GlobalSettings.h $
/// \version $Revision: #5 $
/// \brief :  This file contains APITraceOptions, CLAPIRule,
/// \brief :  GeneralOptions classes
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/GlobalSettings.h#5 $
// Last checkin:   $DateTime: 2015/11/19 03:18:47 $
// Last edited by: $Author: gyarnitz $
// Change list:    $Change: 549127 $
//=====================================================================
#ifndef _GLOBAL_SETTINGS_H_
#define _GLOBAL_SETTINGS_H_

#include <qtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <QStringList>
#include <QTextStream>

#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>

#include <TSingleton.h>

/// delete session file option
enum DeleteSessionFileOption
{
    /// Not a DeleteSessionFileOption
    NA_DELETE_SESSION_FILE_OPTION = 0,

    /// Always delete File
    ALWAYS,

    /// Never delete File
    NEVER,

    /// Ask user every time
    ASK
};

/// General options for GPU Profiler
class GeneralOptions
{
public:

    /// Initializes a new instance of the GeneralOptions class
    GeneralOptions();

    /// Save General options
    /// \param sw stream writer
    void Save(QTextStream& sw);

    /// Load General settings
    /// \param setting setting name and setting value
    void Load(QStringList setting);

    /// Load General settings
    /// \param setting setting name and setting value
    void Load(QMap<QString, bool> setting);

    DeleteSessionFileOption m_delOption;   ///< Flag indicating current delete option
    bool m_showDetailDeletion;             ///< Indicates whether or not to show detail of delete session file.
    bool m_showProfileSetting;             ///< Indicates whether to show the profile setting everytime before profiling
    const QString constDeleteOptions;      ///< Constant string for Delete Option
    const QString constShowDetailDeletion; ///< Constant string for showing detail of deletion
    const QString constShowProfileSetting; ///< Constant string for showing profile setting

private:
    /// disabled default copy ctor
    GeneralOptions(const GeneralOptions&);

    /// disabled default assignment operator
    GeneralOptions& operator = (const GeneralOptions&);
};

/// Global options for GPU Profiler
class AMDT_GPU_PROF_API GlobalSettings : public TSingleton<GlobalSettings>
{
    /// TSingleton needs to be able to use our constructor.
    friend class TSingleton<GlobalSettings>;

public:

    /// Save to file
    void Save();

    /// Load settings from disk
    void Load();

    GeneralOptions m_generalOpt; ///< General options
    QString        m_configFile; ///< Config file name

private:
    /// Hide ctor
    GlobalSettings();

    /// disabled default copy ctor
    GlobalSettings(const GlobalSettings&);

    /// disabled default assignment operator
    GlobalSettings& operator = (const GlobalSettings&);
};

#endif // _GLOBAL_SETTINGS_H_

