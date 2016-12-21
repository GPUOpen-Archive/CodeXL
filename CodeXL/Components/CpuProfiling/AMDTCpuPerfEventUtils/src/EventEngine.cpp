//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file EventEngine.cpp
/// \brief Implementation of the EventEngine class.
///
//==================================================================================

#include <sstream>
#include <EventEngine.h>
#include <AMDTOSWrappers/Include/osCpuid.h>


static gtString toHexString(uint64_t num)
{
    std::wstringstream ss;
    ss << std::hex << num << std::dec;
    return gtString(ss.str().data());
}

bool EventEngine::Initialize(const osDirectory& eventsDirectory)
{
    bool rc = false;

    if (eventsDirectory.exists())
    {
        m_eventFileDirectory = eventsDirectory;
        rc = true;
    }
    else
    {
        osFilePath emptyFilePath;
        m_eventFileDirectory.setDirectoryPath(emptyFilePath);
    }

    return rc;
}

osFilePath EventEngine::GetEventFilePath(gtUInt32 cpuFamily, gtUInt32 cpuModel)
{
    gtString fileName;
    fileName.append(L"0x").append(toHexString(cpuFamily));

    // If the model mask is needed
    if (cpuFamily >= FAMILY_OR)
    {
        // since the model is like 0x10-1f, just need the mask (like 0x10), so shift right by 4 bits
        fileName.append(L"_0x").append(toHexString(cpuModel >> 4));
    }

    osFilePath filePathStr;
    filePathStr.setFileDirectory(m_eventFileDirectory);
    filePathStr.setFileName(fileName);
    filePathStr.setFileExtension(L"xml");

    if (!filePathStr.exists())
    {
        return osFilePath();
    }

    return filePathStr;
}

EventsFile* EventEngine::GetEventFile(gtUInt32 cpuFamily, gtUInt32 cpuModel)
{
    osFilePath fullPathString = GetEventFilePath(cpuFamily, cpuModel);

    if (fullPathString.isEmpty())
    {
        return nullptr;
    }

    EventsFile* pEventFile = new EventsFile();

    std::string fullPath;
    fullPathString.asString().asUtf8(fullPath);

    if (pEventFile != nullptr && pEventFile->Open(fullPath))
    {
        return pEventFile;
    }

    delete pEventFile;
    return nullptr;
}
