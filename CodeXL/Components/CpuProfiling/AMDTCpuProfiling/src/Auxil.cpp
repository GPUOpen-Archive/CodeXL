//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file Auxil.cpp
/// \brief  Implements the initial framework interface for the CodeAnalyst component
///
//==================================================================================

#ifdef PROTO_ONLY
    #include <string.h>
#else
    #include <memory.h>
    #include <assert.h>
#endif

// Qt:
#include <QtCore>

// AMDTOSWrappers:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>

//Local
#include "inc/Auxil.h"
#include <inc/CpuProfilingOptions.h>

void AuxGetSymbolSearchPath(gtString& searchPath, gtString& serverList, gtString& cachePath)
{
    gtUInt32 dUseSymServer = 0;

    PROFILE_OPTIONS* pao = CpuProfilingOptions::instance().options();

    if (pao->addDebug)
    {
        searchPath = acQStringToGTString(pao->debugSearchPaths);
    }

    QString symServerList;

    // check symbol server options
    if (pao->enableSymServer)
    {
        dUseSymServer = pao->useSymSrvMask;
        symServerList = pao->symSrvList;
        cachePath = acQStringToGTString(pao->symbolDownloadDir);
    }

    serverList.makeEmpty();

    if (!symServerList.isEmpty() && 0 != dUseSymServer)
    {
        QStringList servers = symServerList.split(';', QString::SkipEmptyParts);

        for (int i = 0; i < servers.size(); ++i)
        {
            if ((dUseSymServer & (1 << i)) != 0)
            {
                serverList += acQStringToGTString(servers.at(i));
                serverList += L';';
            }
        }
    }
}