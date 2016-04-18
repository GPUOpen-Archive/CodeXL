//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/CLAPIFilterManager.cpp $
/// \version $Revision: #11 $
/// \brief :  This file contains CLAPIFilterManager
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/CLAPIFilterManager.cpp#11 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================

// a specific issue in Qt that is not added to the general qtIgnoreCompileWarning
#if AMDT_CPP_COMPILER == AMDT_VISUAL_CPP_COMPILER
    #pragma warning( disable : 4800 )
#endif

#include <qtIgnoreCompilerWarnings.h>
#include <QFile>
#include <QIODevice>
#include <QTextStream>


// Infra:
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Local:
#include <AMDTGpuProfiling/Util.h>
#include <AMDTGpuProfiling/CLAPIFilterManager.h>
#include <AMDTGpuProfiling/ProjectSettings.h>
#include <AMDTGpuProfiling/HSAAPIDefs.h>
#include <AMDTGpuProfiling/CLAPIDefs.h>


CLAPIFilterManager::CLAPIFilterManager(const QString& filename)
{
    m_aPIFilterFile = Util::ToQtPath(filename);
}

void CLAPIFilterManager::Save(const QStringList& filterAPIslist, APIToTrace apiTrace)
{
    // first will remove old file:
    if (QFile::exists(m_aPIFilterFile))
    {
        QFile::remove(m_aPIFilterFile);
    }

    if (filterAPIslist.count() == 0)
    {
        return;
    }

    // Add a log message:
    gtString fileName = acQStringToGTString(m_aPIFilterFile);
    gtString logMessage = (L"Saving API filter file to: ");
    logMessage.append(fileName);
    OS_OUTPUT_DEBUG_LOG(logMessage.asCharArray(), OS_DEBUG_LOG_INFO);

    QFile file(m_aPIFilterFile);

    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream wr(&file);

        foreach (QString apiStr, filterAPIslist)
        {
            bool shouldAddAPI = false;

            if (apiTrace == APIToTrace_HSA)
            {
                shouldAddAPI = HSAAPIDefs::Instance()->IsHSAAPI(apiStr);
            }
            else if (apiTrace == APIToTrace_OPENCL)
            {
                shouldAddAPI = CLAPIDefs::Instance()->IsCLAPI(apiStr);
            }

            if (shouldAddAPI)
            {
                if (!apiStr.isEmpty())
                {
                    wr << apiStr << "\n";
                }
            }
        }
    }

    m_apiFilterList = filterAPIslist;
    file.close();
}

bool CLAPIFilterManager::GetEnabled() const
{
    return m_apiFilterList.count() > 0;
}


QString CLAPIFilterManager::GetAPIFilterFile() const
{
    return m_aPIFilterFile;
}



