//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/CLSummarizer.cpp $
/// \version $Revision: #16 $
/// \brief :  This file contains CLSummarizer
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/CLSummarizer.cpp#16 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================

// Qt
#include <qtIgnoreCompilerWarnings.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Local:
#include "CLSummarizer.h"


CLSummarizer::CLSummarizer(TraceSession* pSession) : m_pSession(pSession), m_summaryPagesLoaded(false), m_hasErrorWarningPage(false)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((pSession != nullptr) && (pSession->m_pParentData != nullptr))
    {
        m_atpFile = acGTStringToQString(pSession->m_pParentData->m_filePath.asString());
    }
}

CLSummarizer::~CLSummarizer()
{
}

bool CLSummarizer::CheckAndAddSummaryPage(const QString& strFileExt, const QString& strFilePath, const QString& strWhichSummaryFile, const QString& strWhichSummaryTitle)
{
    bool retVal = false;

    if (strFileExt.endsWith(strWhichSummaryFile))
    {
        retVal = true;
        QString summaryName = strFileExt;
        summaryName.chop(strWhichSummaryFile.length());

        if (!summaryName.isEmpty())
        {
            summaryName.append(" ");
        }

        summaryName.append(strWhichSummaryTitle);
        m_summaryPagesMap.insert(summaryName, strFilePath);
        m_pSession->AddAdditionalFile(strFilePath);
    }

    return retVal;
}

void CLSummarizer::CreateSummaryPages()
{
    if (!m_summaryPagesLoaded)
    {
        QFileInfo f(m_atpFile);
        QString expectedBaseName = f.baseName();
        QStringList filterList;
        filterList << "*.html";
        QFileInfoList fileInfos = f.dir().entryInfoList(filterList);

        foreach (QFileInfo fi, fileInfos)
        {
            QString baseName = fi.baseName();

            if (baseName == expectedBaseName)
            {
                QString fileExt = fi.completeSuffix();
                QString filePath = fi.filePath();
                QString summaryName;

                if (CheckAndAddSummaryPage(fileExt, filePath, Util::ms_APISUMFILE, Util::ms_APISUM))
                {
                    continue;
                }
                else  if (CheckAndAddSummaryPage(fileExt, filePath, Util::ms_CTXSUMFILE, Util::ms_CTXSUM))
                {
                    continue;
                }
                else  if (CheckAndAddSummaryPage(fileExt, filePath, Util::ms_TOP10KERNELFILE, Util::ms_TOP10KERNEL))
                {
                    continue;
                }
                else  if (CheckAndAddSummaryPage(fileExt, filePath, Util::ms_KERNELSUMFILE, Util::ms_KERNELSUM))
                {
                    continue;
                }
                else  if (CheckAndAddSummaryPage(fileExt, filePath, Util::ms_TOP10DATAFILE, Util::ms_TOP10DATA))
                {
                    continue;
                }
                else  if (CheckAndAddSummaryPage(fileExt, filePath, Util::ms_BESTPRACTICESFILE, Util::ms_BESTPRACTICES))
                {
                    m_hasErrorWarningPage = true;
                    continue;
                }
            }
        }

        m_summaryPagesLoaded = true;
    }
}

void CLSummarizer::UpdateRenamedSession(const osDirectory& oldSessionDirectory, const osDirectory& newSessionDirectory)
{
    // Go through the map of summary pages and update the session folder:
    QString oldSessionDir = acGTStringToQString(oldSessionDirectory.directoryPath().asString());
    QString newSessionDir = acGTStringToQString(newSessionDirectory.directoryPath().asString());

    QDir oldDir(oldSessionDir);
    QDir newDir(newSessionDir);

    QString oldBaseName = oldDir.dirName();
    QString newBaseName = newDir.dirName();

    oldSessionDir.replace('/', osFilePath::osPathSeparator);
    newSessionDir.replace('/', osFilePath::osPathSeparator);
    QMap<QString, QString>::iterator iter = m_summaryPagesMap.begin();
    QMap<QString, QString>::iterator iterEnd = m_summaryPagesMap.end();

    for (; iter != iterEnd; iter++)
    {
        // For each of the summary page names, change the base name to the renamed session name:
        QString oldFilePath = iter.value();
        QString newFilePath = iter.value();

        // Build the new file path:
        newFilePath.replace(oldBaseName, newBaseName);

        // The folder name has changes, but the file names haven't. Find the file path with the
        // new folder and old base name:
        gtString oldFolderBaseName;
        oldFolderBaseName.append(osFilePath::osPathSeparator);
        oldFolderBaseName.append(acQStringToGTString(oldBaseName));
        oldFolderBaseName.append(osFilePath::osPathSeparator);

        gtString newFolderBaseName;
        newFolderBaseName.append(osFilePath::osPathSeparator);
        newFolderBaseName.append(acQStringToGTString(newBaseName));
        newFolderBaseName.append(osFilePath::osPathSeparator);

        gtString fileToRename = acQStringToGTString(oldFilePath);
        fileToRename.replace('/', osFilePath::osPathSeparator);
        fileToRename.replace(oldFolderBaseName, newFolderBaseName);

        // Rename the summary file on disk:
        bool rc = QFile::rename(acGTStringToQString(fileToRename), newFilePath);
        GT_ASSERT(rc);

        m_summaryPagesMap[iter.key()] = newFilePath;
    }

}



