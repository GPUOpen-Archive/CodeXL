//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file StdAfx.cpp
/// \brief Source file that includes just the standard includes
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/src/StdAfx.cpp#18 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569613 $
//=============================================================

#if defined (_WIN32)
    #pragma warning(disable : 4311 4312) //ignore qt warnings
#endif

#include <QtCore>
#include <QtWidgets>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QFileDialog>

#include "inc/StdAfx.h"

#if defined (_WIN32)
    #pragma warning(default : 4311 4312)
#endif


bool ReadSessionCacheFileMap(const QString& sessionDir, CacheFileMap& cache)
{
    QString mapPath;
    mapPath = sessionDir + CACHE_FILE_MAP;

    QFile mapFile(mapPath);
    QTextStream stream(&mapFile);
    QString line;

    if (!mapFile.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        return false;
    }

    stream.setCodec("UTF-16");

    while (!stream.atEnd())
    {
        QString originalPath;
        QString cachedPath;

        // read the next line, which contains originalPath:= src.java
        line = stream.readLine();

        originalPath    = line.section(QObject::tr(":="), 0, 0);
        cachedPath  = line.section(QObject::tr(":="), 1, 1);

        cache.insert(originalPath, cachedPath);
    }

    mapFile.close();
    return true;
}

bool WriteSessionCacheFileMap(const QString& sessionDir, CacheFileMap& cache)
{

    QString mapPath;
    mapPath = sessionDir + CACHE_FILE_MAP;

    QFile mapFile(mapPath);
    QTextStream stream(&mapFile);

    if (!mapFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        return false;
    }

    stream.setCodec("UTF-16");
    CacheFileMap::Iterator it;

    for (it = cache.begin(); it != cache.end(); ++it)
    {
        stream << it.key() << ":=" << it.value() << endl;
    }

    mapFile.close();
    return true;
}

//returns true if cached file found
bool GetCachedFile(QString sessionDir, QString filePath, QString& cachedPath)
{
    CacheFileMap cache;

    //don't want to remove the \0 on the original filepath if it exists
    QString jncTest = filePath;
    jncTest.remove(QChar('\0'));

    cachedPath = jncTest;

    if (jncTest.endsWith(".jnc") || jncTest.endsWith(".ocl"))
    {
        // Make sure paths use slashes consistently
        filePath.replace('/', PATH_SLASH);
        sessionDir.replace('/', PATH_SLASH);

        //oclt jit file
        if (!filePath.startsWith(sessionDir))
        {
            cachedPath =  sessionDir + PATH_SLASH + filePath;
        }
    }
    else
    {
        //default is same path
        if (!ReadSessionCacheFileMap(sessionDir, cache))
        {
            return false;
        }

        if (!cache.contains(jncTest))
        {
            return false;
        }

        cachedPath = cache[jncTest];
    }

    return true;
}

//returns true if the caching was successful
//filePath is the original path, altSource is the user-specified source for the filePath
bool CacheFile(const QString& sessionDir, QString filePath, const QString& altSource, bool symsToo)
{
    CacheFileMap cache;

    //check for current cache
    if (!ReadSessionCacheFileMap(sessionDir, cache))
    {
        return false;
    }

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    PVOID oldValue = nullptr;
    BOOL doRedirect = false;
    IsWow64Process(GetCurrentProcess(), &doRedirect);

    if (doRedirect)
    {
        doRedirect = (BOOL) Wow64DisableWow64FsRedirection(&oldValue);
    }

#endif

    //if needed, create cache sub-dir
    QString cachePath = sessionDir + "/cache/";
    QDir dir(cachePath);

    if (!dir.exists())
    {
        dir.mkpath(cachePath);
    }

    //determine cache name
    filePath.remove(QChar('\0'));
    QFileInfo original(filePath);
    int additional = 1;

    QString existTest = cachePath + original.fileName();

    while (QFile::exists(existTest))
    {
        existTest = cachePath + original.baseName() + " "
                    + QString::number(additional++) + "." + original.completeSuffix();
    }

    //copy to cache
    QString base = altSource.isEmpty() ? filePath : altSource;

    if (!QFile::copy(base, existTest))
    {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

        if (doRedirect)
        {
            Wow64RevertWow64FsRedirection(oldValue);
        }

#endif
        return false;
    }

    if (symsToo)
    {
        QFileInfo baseInfo(base);
        QString symBase = baseInfo.absolutePath() + "/" + baseInfo.baseName() + ".pdb";
        baseInfo.setFile(existTest);
        QString symCopy = baseInfo.absolutePath() + "/" + baseInfo.baseName() + ".pdb";
        QFile::copy(symBase, symCopy);
    }

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    if (doRedirect)
    {
        Wow64RevertWow64FsRedirection(oldValue);
    }

#endif

    //add to cache map
    cache.insert(filePath, existTest);

    return WriteSessionCacheFileMap(sessionDir, cache);
} //CacheFile

//returns true if was able to add the relocation to the cache map
bool CacheRelocatedSource(const QString& sessionDir, QString filePath, const QString& relocationPath, bool cache, bool symsToo)
{
    bool retVal;
    filePath.remove(QChar('\0'));

    if (cache)
    {
        retVal = CacheFile(sessionDir, filePath, relocationPath, symsToo);
    }
    else
    {
        CacheFileMap cacheMap;

        if (!ReadSessionCacheFileMap(sessionDir, cacheMap))
        {
            return false;
        }

        cacheMap.insert(filePath, relocationPath);
        retVal = WriteSessionCacheFileMap(sessionDir, cacheMap);
    }

    return retVal;
}

QString FindModuleFile(QWidget* pParent, const QString& originalPath)
{
    QString tryFile = originalPath;
    QFileInfo fileInfo(originalPath);
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    PVOID oldValue = nullptr;
    BOOL doRedirect = false;
    IsWow64Process(GetCurrentProcess(), &doRedirect);

    if (doRedirect)
    {
        doRedirect = (BOOL) Wow64DisableWow64FsRedirection(&oldValue);
    }

#endif

    while (!QFile::exists(tryFile))
    {
        // We did not find the file, ask user where it is.
        // Save the path of the user selected and use it next time.
        tryFile = QFileDialog::getOpenFileName(pParent,
                                               "Locate module file " + fileInfo.fileName(),
                                               originalPath, "Module File (*.*)");

        if (tryFile.isEmpty())
        {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

            if (doRedirect)
            {
                Wow64RevertWow64FsRedirection(oldValue);
            }

#endif
            return QString::null;
        }
    }

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    if (doRedirect)
    {
        Wow64RevertWow64FsRedirection(oldValue);
    }

#endif
    return tryFile;
}

bool IsPreJITModule(QString moduleName)
{
    return (moduleName.toLower().contains("assembly\\nativeimages") &&
            moduleName.contains(".ni."));
}


SampleKeyType::SampleKeyType()
{
    cpu = 0;
    event = 0;
}

SampleKeyType::SampleKeyType(int a, EventMaskType b)
{
    cpu = a;
    event = b;
}
