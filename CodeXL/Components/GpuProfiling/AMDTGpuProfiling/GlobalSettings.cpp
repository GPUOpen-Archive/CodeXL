//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/GlobalSettings.cpp $
/// \version $Revision: #8 $
/// \brief :  This file contains GlobalSettings
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/GlobalSettings.cpp#8 $
// Last checkin:   $DateTime: 2015/08/31 01:56:31 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 538740 $
//=====================================================================

// Qt:
#include <QtCore>
#include <QtWidgets>

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Local:
#include "GlobalSettings.h"
#include <AMDTGpuProfiling/Util.h>


GlobalSettings::GlobalSettings()
{
    osFilePath settingsFile;
    Util::GetProfilerAppDataDir(settingsFile);
    settingsFile.setFileName(L"settings.sptmp");
    gtString settingsFileAsStr = settingsFile.asString();
    m_configFile = acGTStringToQString(settingsFileAsStr);
    Load();
}

void GlobalSettings::Save()
{
    QFile outFile(m_configFile);

    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        Util::ShowWarningBox("Unable to save settings");
        return;
    }

    QTextStream sw(&outFile);
    m_generalOpt.Save(sw);
}

void GlobalSettings::Load()
{
    QFile iFile(m_configFile);

    if (iFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream sr(&iFile);

        while (!sr.atEnd())
        {
            QString str = sr.readLine();

            if (str.trimmed().isEmpty())
            {
                continue;
            }

            QStringList setting = str.split("=", QString::SkipEmptyParts);

            if (setting.length() != 2)
            {
                continue;
            }

            if (setting[0].startsWith("GeneralOptions."))
            {
                m_generalOpt.Load(setting);
            }
        }
    }
}

//---------------------------------------------------------------
// GeneralOptions
//---------------------------------------------------------------

/// Initializes a new instance of the GeneralOptions class
GeneralOptions::GeneralOptions():
    constDeleteOptions("GeneralOption.DeleteOptions"),
    constShowDetailDeletion("GeneralOption.ShowDetailDeletion"),
    constShowProfileSetting("GeneralOption.ProfileSetting")
{
    m_delOption = NEVER;
    m_showProfileSetting = false;
    m_showDetailDeletion = true;
}

void GeneralOptions::Save(QTextStream& sw)
{
    sw << QString("%1=%2\n").arg(constDeleteOptions).arg(m_delOption);
    sw << QString("%1=%2\n").arg(constShowDetailDeletion).arg(Util::BoolToQString(m_showDetailDeletion));
    sw << QString("%1=%2\n").arg(constShowProfileSetting).arg(Util::BoolToQString(m_showProfileSetting));
}

void GeneralOptions::Load(QStringList setting)
{
    if (setting[0] == constDeleteOptions)
    {
        if (setting[1] == "Always")
        {
            m_delOption = ALWAYS;
        }
        else if (setting[1] == "Never")
        {
            m_delOption = NEVER;
        }
        else
        {
            m_delOption = ASK;
        }
    }
    else if (setting[0] == constShowDetailDeletion)
    {
        m_showDetailDeletion = (setting[1] == Util::ms_TRUESTR) ? true : false;
    }
    else if (setting[0] == constShowProfileSetting)
    {
        m_showProfileSetting = (setting[1] == Util::ms_TRUESTR) ? true : false;
    }
}

void GeneralOptions::Load(QMap<QString, bool> setting)
{
    QMap<QString, bool>::const_iterator index = setting.constBegin();

    while (index != setting.constEnd())
    {
        if ((index.key() == "Always") && (index.value()))
        {
            m_delOption = ALWAYS;
        }
        else if ((index.key() == "Never") && (index.value()))
        {
            m_delOption = NEVER;
        }
        else if ((index.key() == "ASK") && (index.value()))
        {
            m_delOption = ASK;
        }
        else if (index.key() == "ShowDetailDeletion")
        {
            m_showDetailDeletion = index.value();
        }

        index++;
    }
}


