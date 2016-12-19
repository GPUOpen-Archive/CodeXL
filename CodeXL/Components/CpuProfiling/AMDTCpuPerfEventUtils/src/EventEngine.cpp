//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file EventEngine.cpp
/// \brief Implementation of the EventEngine class.
///
//==================================================================================

#include <QtCore>
#include <EventEngine.h>
#include <AMDTOSWrappers/Include/osCpuid.h>


bool EventEngine::Initialize(QString eventsDirectory)
{
    m_eventFileDirectory.setPath(eventsDirectory);

    if (!m_eventFileDirectory.exists())
    {
        m_eventFileDirectory.setPath("");
        return false;
    }

    return true;
}

QString EventEngine::GetEventFilePath(gtUInt32 cpuFamily, gtUInt32 cpuModel)
{
    QString fileName;

    // If the model mask is needed
    if (cpuFamily >= FAMILY_OR)
    {
        //since the model is like 0x10-1f, just need the mask (like 0x10), so shift right by 4 bits
        fileName = "0x" + QString::number(cpuFamily, 16) + "_0x" + QString::number(cpuModel >> 4, 16);
    }
    else
    {
        fileName = "0x" + QString::number(cpuFamily, 16);
    }

    fileName += ".xml";

    if (!m_eventFileDirectory.exists(fileName))
    {
        return QString();
    }

    return m_eventFileDirectory.absoluteFilePath(fileName);
}

EventsFile* EventEngine::GetEventFile(gtUInt32 cpuFamily, gtUInt32 cpuModel)
{
    QString fullPathString = GetEventFilePath(cpuFamily, cpuModel);

    if (fullPathString.isEmpty())
    {
        return nullptr;
    }

    EventsFile* pEventFile = new EventsFile();

    if (pEventFile != nullptr && pEventFile->Open(fullPathString.toStdString()))
    {
        return pEventFile;
    }

    delete pEventFile;
    return nullptr;
}
