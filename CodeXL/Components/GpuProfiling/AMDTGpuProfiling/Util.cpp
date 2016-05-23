//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/Util.cpp $
/// \version $Revision: #63 $
/// \brief  This file contains Util class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/Util.cpp#63 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================

#include <qtIgnoreCompilerWarnings.h>

// Qt:
#include <QtCore>
#include <QtWidgets>

// Infra
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTAPIClasses/Include/apProjectSettings.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/src/afUtils.h>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/StringConstants.h>

// Remote
#include <AMDTRemoteClient/Include/CXLDaemonClient.h>

// Backend header files
#include "Defs.h"

// Local header files
#include <AMDTGpuProfiling/Util.h>
#include <AMDTGpuProfiling/Session.h>


const QString Util::ms_APISUMFILE("APISummary.html");
const QString Util::ms_CTXSUMFILE("ContextSummary.html");
const QString Util::ms_TOP10KERNELFILE("Top10KernelSummary.html");
const QString Util::ms_KERNELSUMFILE("KernelSummary.html");
const QString Util::ms_TOP10DATAFILE("Top10DataTransferSummary.html");
const QString Util::ms_BESTPRACTICESFILE("BestPractices.html");
const QString Util::ms_APISUM("API Summary");
const QString Util::ms_CTXSUM("Context Summary");
const QString Util::ms_TOP10KERNEL("Top10 Kernel Summary");
const QString Util::ms_KERNELSUM("Kernel Summary");
const QString Util::ms_TOP10DATA("Top10 Data Transfer Summary");
const QString Util::ms_BESTPRACTICES("Warning(s)/Error(s)");
const QString Util::ms_KERNEL_ASSEMBLY_FILE_PREFIX = KERNEL_ASSEMBLY_FILE_PREFIX;  // KERNEL_ASSEMBLY_FILE_PREFIX is defined in Defs.h in the backend
const QString Util::ms_TRUESTR = "True";
const QString Util::ms_FALSESTR = "False";
const QString Util::ms_APP_TRACE_OPTIONS_PAGE = GPU_NARROW_STR_TRACE_PROJECT_SETTINGS_DISPLAY;
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    const QString Util::ms_ENABLE_TIMEOUT_OPTION = QString("Try enabling the \"Write trace data in intervals during program execution\" option (or reducing the interval if the option is already enabled) on the \"%1\" project setting page.").arg(ms_APP_TRACE_OPTIONS_PAGE);
#endif
#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS)
    const QString Util::ms_ENABLE_TIMEOUT_OPTION = QString("Try reducing the \"Interval at which to write trace data during program execution\" on the \"%1\" project setting page.").arg(ms_APP_TRACE_OPTIONS_PAGE);
#endif

void Util::GetProfilerAppDataDir(osFilePath& appDataDir)
{
    afGetUserDataFolderPath(appDataDir);
}

bool Util::IsInternalBuild()
{
#if AMDT_BUILD_ACCESS == AMDT_INTERNAL_ACCESS
    return  true;
#else
    return false;
#endif
}

bool Util::GetInstallDirectory(osFilePath& installDir)
{
    return installDir.SetInstallRelatedPath(osFilePath::OS_CODEXL_BINARIES_PATH);
}

void Util::DeleteAllFilesInDirectory(const QString& strDirectory,
                                     const QString& strFilePrefix,
                                     const QString& strFileExtension)
{
    QDir directory(strDirectory);
    directory.setFilter(QDir::Files | QDir::Hidden);

    QFileInfoList FInfoList = directory.entryInfoList();

    for (int index = 0; index < FInfoList.size(); index++)
    {
        QFileInfo fileInfo = FInfoList.at(index);
        QString strFileName = fileInfo.fileName();

        if (strFileName.startsWith(strFilePrefix) && strFileName.endsWith(strFileExtension))
        {
            if (QFile::remove(fileInfo.filePath()) == false)
            {
                // TODO: Error
            }
        }
    }
}

bool Util::IsKernelName(const QString& strText)
{
    QString strTrimmedText = strText.trimmed();

    // TODO: this can always return true now -- we don't show data transfer in csv view
    // check if it is a memory operation
    if (strTrimmedText.startsWith("CreateBuffer") ||
        strTrimmedText.startsWith("CreateImage") ||
        strTrimmedText == "ReadBuffer" ||
        strTrimmedText == "ReadBufferAsynch" ||
        strTrimmedText == "WriteBuffer" ||
        strTrimmedText == "WriteBufferAsynch" ||
        strTrimmedText == "ReadImage2D" ||
        strTrimmedText == "ReadImage2DAsynch" ||
        strTrimmedText == "ReadImage3D" ||
        strTrimmedText == "ReadImage3DAsynch" ||
        strTrimmedText == "WriteImage2D" ||
        strTrimmedText == "WriteImage2DAsynch" ||
        strTrimmedText == "WriteImage3D" ||
        strTrimmedText == "WriteImage3DAsynch" ||
        strTrimmedText == "MapBuffer" ||
        strTrimmedText == "MapBufferAsynch" ||
        strTrimmedText == "MapImage2D" ||
        strTrimmedText == "MapImage2DAsynch" ||
        strTrimmedText == "MapImage3D" ||
        strTrimmedText == "MapImage3DAsynch" ||
        strTrimmedText == "UnmapMemBuffer" ||
        strTrimmedText == "UnmapMemImage2D" ||
        strTrimmedText == "UnmapMemImage3D" ||
        strTrimmedText == "CopyBuffer" ||
        strTrimmedText == "CopyImage2D" ||
        strTrimmedText == "CopyImage3D" ||
        strTrimmedText == "CopyImage2DToBuffer" ||
        strTrimmedText == "CopyImage3DToBuffer" ||
        strTrimmedText == "CopyBufferToImage2D" ||
        strTrimmedText == "CopyBufferToImage3D" ||
        strTrimmedText == "D3D11_MAP_READ"      ||
        strTrimmedText == "D3D11_MAP_WRITE"     ||
        strTrimmedText == "D3D11_MAP_READ_WRITE")
    {
        return false;
    }

    return true;
}


QString Util::GetProjectDirectoryFromProfilerOutput(const QString& strDirectory)
{
    if (strDirectory.isEmpty() || strDirectory.isNull() ||
        strDirectory.size() < 2)
    {
        return QString();
    }

    QString strTemp = strDirectory;

    strTemp.chop(1);
    int index = strTemp.lastIndexOf("//");

    if (index <= 0)
    {
        return QString();
    }

    strTemp.chop(index + 1);

    return strTemp;
}

QString Util::StripDeviceNameFromKernelName(const QString& strKernelName)
{
    QString strLocalKernelName = strKernelName;
    int position = strLocalKernelName.lastIndexOf('_');

    if (-1 != position)
    {
        // found the device name
        strLocalKernelName.chop(position);
    }

    return strLocalKernelName;
}

QString Util::GetFirstSubString(const QString& str)
{
    // two cases:
    // 1. the first substring is inside quotes
    // 2. the first substring is not in quotes
    QString strTrimmed = str.trimmed();

    if ((strTrimmed.size() > 0) && ('\"' != strTrimmed[0]))
    {
        int space = strTrimmed.indexOf(' ');

        if (space < 0)
        {
            return str;
        }

        // CASE 2
        return strTrimmed.left(space);
    }

    int secondQuote = strTrimmed.indexOf('\"', 1);

    if (secondQuote < 1)
    {
        // error, let's just return the original string
        return str;
    }

    // CASE 1
    return strTrimmed.left(secondQuote + 1);
}

bool Util::IsZeroValue(const QString& strValue)
{
    float value = 0;
    bool status = false;

    value = strValue.toFloat(&status);

    if (status == false)
    {
        return false;
    }

    float eps = 0.00000001f;

    if (fabs(value) < eps)
    {
        return true;
    }

    return false;
}

QString Util::RemoveTrailingZero(const QString& strValue)
{
    if (strValue.endsWith(".00"))
    {
        return strValue.left(strValue.size() - 3);
    }

    return strValue;
}

QString Util::ProcessCmdLineArgsStr(const QString& strArgs)
{
    bool quoteBegin = false;
    int beginIdx = -1;
    int lastEndIdx = 0;

    QString strTemp;

    for (int i = 0; i < strArgs.size(); i++)
    {
        if (strArgs.at(i) == '"')
        {
            if (quoteBegin)
            {
                // end quote
                if (lastEndIdx == 0)
                {
                    strTemp.append(strArgs.mid(lastEndIdx, beginIdx - lastEndIdx));
                }
                else
                {
                    strTemp.append(strArgs.mid(lastEndIdx + 1, beginIdx - 1 - lastEndIdx));
                }

                QString str = "\\\"" + strArgs.mid(beginIdx + 1, i - beginIdx - 1) + "\\\"";
                strTemp.append(str);
                lastEndIdx = i;
                quoteBegin = false;
            }
            else
            {
                quoteBegin = true;
                beginIdx = i;
            }
        }
    }

    if (lastEndIdx != 0)
    {
        QString str = strArgs.mid(lastEndIdx + 1);
        strTemp.append(str);
    }

    // no string arg found
    if (beginIdx == -1)
    {
        strTemp.append(strArgs);
    }

    return strTemp;
}

// Qt has a built-in function to get File name using FileInfo
// Will see if we can use that in spite of this.
// Will see if framework can help here.
QString Util::GetFileNameFromPath(const QString& strPath)
{
    QString strFile;
    int index = strPath.lastIndexOf("//");

    if (index <= 0)
    {
        return QString::null;
    }

    strFile = strPath.right(strPath.size() - (index + 1));
    return strFile;
}

QString Util::RemoveDupSlash(const QString& strPath)
{
    bool slashStart = false;
    QString strTemp;

    for (int i = 0; i < strPath.size(); i++)
    {
        if (strPath.at(i) == '/')
        {
            if (slashStart)
            {
                // remove this slash
                continue;
            }
            else
            {
                slashStart = true;
                strTemp.append(strPath.at(i));
            }
        }
        else if (slashStart)
        {
            slashStart = false;
            strTemp.append(strPath.at(i));
        }
        else
        {
            strTemp.append(strPath.at(i));
        }
    }

    return strTemp;
}

QString Util::GetURLFromFileName(const QString& strFileName)
{
    QString strTempFileName = strFileName;
    strTempFileName = strTempFileName.replace('\\', '/');
    return Util::RemoveDupSlash(strTempFileName);
}

bool Util::RemoveFileOrDirectory(const QString& strPath, QString& strDetailReport)
{
    bool tmpDeleteSucccess = true;

    QString strError;

    bool validFileOrDirectory = false;
    QFileInfo fileInfo(strPath);
    QString filePath = fileInfo.filePath();
    QString strDisplayFilePath = filePath.replace("//", "/");

    if (fileInfo.exists() && fileInfo.isFile())
    {
        QFile file(filePath);

        if (file.remove())
        {
            validFileOrDirectory = true;
            tmpDeleteSucccess = true;
        }
        else
        {
            tmpDeleteSucccess = false;
            strError = file.errorString().trimmed();
        }
    }
    else if (fileInfo.isDir())
    {
        QDir dir(filePath);

        if (dir.rmdir(filePath))
        {
            validFileOrDirectory = true;
            tmpDeleteSucccess = true;
        }
        else
        {
            tmpDeleteSucccess = false;
            strError.clear();
        }
    }

    if (validFileOrDirectory)
    {
        strDetailReport = GP_Str_MessageDeleted;
    }
    else
    {
        strDetailReport = GP_Str_ErrorUnableToDelete;
    }

    strDetailReport.append(": ").append(strDisplayFilePath);

    if (!strError.isEmpty())
    {
        strDetailReport.append(": ").append(strError);
    }

    if (!strDetailReport.endsWith("\n"))
    {
        strDetailReport.append("\n");
    }

    return tmpDeleteSucccess;
}

void Util::CleanSessionDir(const osDirectory& sessionDir)
{
    gtList<osFilePath> fileEntries;
    sessionDir.getContainedFilePaths(L"*.*", osDirectory::SORT_BY_NAME_ASCENDING, fileEntries);

    for (gtList<osFilePath>::const_iterator i = fileEntries.begin(); i != fileEntries.end(); ++i)
    {
        gtString fileExt;
        (*i).getFileExtension(fileExt);

        if (fileExt == L"html" ||
            fileExt == L"js" ||
            fileExt == L"css" ||
            fileExt == L"atp" ||
            fileExt == L"csv" ||
            fileExt == L"occupancy" ||
            fileExt == L"st" ||
            fileExt == L"perfmarker" ||
            fileExt == L"il" ||
            fileExt == L"hsail" ||
            fileExt == L"cl" ||
            fileExt == L"isa" ||
            fileExt == L"asm" ||
            fileExt == L"csl" ||
            fileExt == L"rls")
        {
            osFile((*i)).deleteFile();
        }
    }
}


QString Util::ToQtPath(const QString& strOldPath)
{
    QFileInfo fileInfo(strOldPath);
    return fileInfo.filePath();
}

QString Util::BoolToQString(bool val)
{
    if (val)
    {
        return ms_TRUESTR;
    }
    else
    {
        return ms_FALSESTR;
    }
}

bool Util::QStringToBool(const QString& strBoolVal)
{
    return (0 == strBoolVal.compare(ms_TRUESTR, Qt::CaseInsensitive));
}

void Util::ShowWarningBox(const QString& strMessage)
{
    if (!strMessage.trimmed().isEmpty())
    {
        LogWarning(strMessage);
        acMessageBox::instance().warning(afGlobalVariablesManager::ProductNameA(), strMessage, QMessageBox::Ok);
    }
}

void Util::ShowErrorBox(const QString& strMessage)
{
    if (!strMessage.trimmed().isEmpty())
    {
        LogError(strMessage);
        acMessageBox::instance().critical(afGlobalVariablesManager::ProductNameA(), strMessage, QMessageBox::Ok);
    }
}

void Util::LogWarning(const QString& strMessage)
{
    OS_OUTPUT_DEBUG_LOG(acQStringToGTString(strMessage).asCharArray(), OS_DEBUG_LOG_INFO);
}

void Util::LogError(const QString& strMessage)
{
    OS_OUTPUT_DEBUG_LOG(acQStringToGTString(strMessage).asCharArray(), OS_DEBUG_LOG_ERROR);
}

// Note : May not required, as file path should not contain '\' now.
// Will removed it.
QString Util::ToBackSlash(const QString& strPath)
{
    QString strTempPath = strPath;
    strTempPath.replace('/', '\\');
    return strTempPath.replace("\\\\", "\\");
}

QString Util::ToString(QStandardItemModel* model, const QModelIndex& index)
{
    return model->data(index).toString();
}

QString Util::GetProfileTypeName(GPUProfileType profileType)
{
    // TODO: use static consts instead of hardcoding these strings (and/or consolidate these with similar string literals in ProfileManager.cpp)
    switch (profileType)
    {
        case PERFORMANCE:
            return PM_profileTypePerformanceCounters;

        case API_TRACE:
            return PM_profileTypeApplicationTrace;

        case FRAME_ANALYSIS:
            return PM_profileTypeFrameAnalysis;

        default:
            return "";
    }
}

QString Util::AppendFileExtension(const QString& fileName, const QString& fileExtension)
{
    QString fileNameWithExtension = fileName;

    if (fileName.isEmpty() || fileExtension.isEmpty())
    {
        return fileName;
    }

    if (!fileName.endsWith(fileExtension))
    {
        fileNameWithExtension.append(fileExtension);
    }

    return fileNameWithExtension;
}

void Util::SetTreeWidgetItemChecked(QTreeWidget* treeWidget, const QString& strTreeWidgetItem, bool checked, bool checkAll)
{
    QTreeWidgetItemIterator iterator(treeWidget);
    Qt::CheckState checkState = checked ? Qt::Checked : Qt::Unchecked;

    while (*iterator)
    {
        if ((*iterator)->text(0) == strTreeWidgetItem)
        {
            if ((*iterator)->checkState(0) != checkState)
            {
                (*iterator)->setCheckState(0, checkState);
            }

            if (!checkAll)
            {
                break;
            }
        }

        ++iterator;
    }
}

void Util::SetCheckState(QTreeWidget* treeWidget, bool checked)
{
    QTreeWidgetItemIterator iterator(treeWidget);

    while (*iterator)
    {
        if (checked)
        {
            (*iterator)->setCheckState(0, Qt::Checked);
        }
        else
        {
            (*iterator)->setCheckState(0, Qt::Unchecked);
        }

        ++iterator;
    }
}

QTreeWidgetItem* Util::FindTreeItem(QTreeWidget* treeWidget, const QString& nodeName)
{
    QTreeWidgetItem* itemFound =  nullptr;
    QTreeWidgetItemIterator iterator(treeWidget);

    while (*iterator)
    {
        if ((*iterator)->text(0) == nodeName)
        {
            itemFound = *iterator;
            break;
        }

        iterator++;
    }

    return itemFound;
}

int Util::ItemsSelectedInTreeWidget(QTreeWidget* treeWidget)
{
    int numberOfCountersChecked = 0;
    QTreeWidgetItemIterator iterator(treeWidget);

    while (*iterator)
    {
        if ((*iterator)->childCount() == 0)
        {
            if ((*iterator)->checkState(0) == Qt::Checked)
            {
                numberOfCountersChecked++;
            }
        }

        iterator++;
    }

    return numberOfCountersChecked;
}

void Util::UpdateTreeWidgetItemCheckState(QTreeWidgetItem* treeWidgetItem, bool updateParent, bool updateCount, int& counterCount)
{
    if (treeWidgetItem != nullptr)
    {
        Qt::CheckState checkState = treeWidgetItem->checkState(0);

        // if it's a leaf node, then alter the count var passed in
        if (updateCount && (treeWidgetItem->childCount() == 0))
        {
            counterCount += checkState == Qt::Checked ? 1 : -1;
        }

        // first set all children to have the same state as this node
        for (int i = 0; i < treeWidgetItem->childCount(); i++)
        {
            bool doUpdateCount = treeWidgetItem->child(i)->checkState(0) != checkState;

            if (doUpdateCount)
            {
                treeWidgetItem->child(i)->setCheckState(0, checkState);
            }

            UpdateTreeWidgetItemCheckState(treeWidgetItem->child(i), false, doUpdateCount, counterCount);
        }

        if (updateParent)
        {
            // now walk the parent chain to check the parent's check state
            UpdateParentTreeWidgetItemCheckState(treeWidgetItem->parent(), true);
        }
    }
}

void Util::UpdateParentTreeWidgetItemCheckState(QTreeWidgetItem* treeWidgetItem, bool checkChildren)
{
    if (treeWidgetItem != nullptr)
    {
        if (!checkChildren)
        {
            treeWidgetItem->setCheckState(0, Qt::PartiallyChecked);
        }
        else
        {
            Qt::CheckState checkState = Qt::Unchecked;

            for (int i = 0; i < treeWidgetItem->childCount(); i++)
            {
                if (i == 0)
                {
                    // Check only visible children:
                    checkState = Qt::Unchecked;

                    if (!treeWidgetItem->child(i)->isHidden())
                    {
                        checkState = treeWidgetItem->child(i)->checkState(0);
                    }
                }
                else
                {
                    if ((checkState != treeWidgetItem->child(i)->checkState(0) && !treeWidgetItem->child(i)->isHidden()))
                    {
                        checkState = Qt::PartiallyChecked;
                        checkChildren = false;
                        break;
                    }
                }
            }

            treeWidgetItem->setCheckState(0, checkState);
        }

        UpdateParentTreeWidgetItemCheckState(treeWidgetItem->parent(), checkChildren);
    }
}

bool Util::GetKernelFiles(const GPUSessionTreeItemData* pSessionData, const QString& strKernelName, gtList<osFilePath>& kernelFiles)
{
    bool retVal = false;

    if (pSessionData != nullptr)
    {
        QString strFileMask = Util::ms_KERNEL_ASSEMBLY_FILE_PREFIX;
        strFileMask.append(strKernelName).append(".*");

        if (pSessionData->SessionDir().getContainedFilePaths(acQStringToGTString(strFileMask), osDirectory::SORT_BY_NAME_ASCENDING, kernelFiles))
        {
            retVal = kernelFiles.size() > 0;
        }
    }

    return retVal;
}

bool Util::IsCodeAvailable(const GPUSessionTreeItemData* pSessionData, const QString& strKernelName)
{
    bool retVal = false;

    gtList<osFilePath> kernelFilesList;

    if (GetKernelFiles(pSessionData, strKernelName, kernelFilesList))
    {
        retVal = !kernelFilesList.empty();
    }

    return retVal;
}
osFilePath Util::GetInstalledPathForSampleFile(const osFilePath& filePath)
{
    osFilePath retVal = filePath;

#if AMDT_BUILD_CONFIGURATION == AMDT_RELEASE_BUILD
    // The following code that checks for the teapot sample sources is stolen from gdCallsStackListCtrl.cpp
    // This code should probably be moved into the framework, so that it can be used by both the debugger
    // and profiler without each component having its own copy of this code.
    gtString sourceCodeFileName;
    retVal.getFileName(sourceCodeFileName);

    // If the file is the GRTeaPot example
    // change the path to be relative to the installation directory
    static const gtString amdTeaPotLibSrcName1 = L"amdtteapotoclsmokesystem";
    static const gtString amdTeaPotLibSrcName2 = L"amdtteapotoglcanvas";
    static const gtString amdTeaPotLibSrcName3 = L"amdtteapotrenderstate";
    static const gtString amdTeaPotLibSrcName4 = L"amdtfluidgrid";
    static const gtString amdTeaPotLibSrcName5 = L"amdtimage";
    static const gtString amdTeaPotLibSrcName6 = L"amdtopenclhelper";
    static const gtString amdTeaPotLibSrcName7 = L"amdtopenglhelper";
    static const gtString amdTeaPotLibSrcName8 = L"amdtopenglmath";

    static const gtString amdTeaPotSrcName1 = L"teapot";
    static const gtString amdTeaPotSrcName2 = L"glwindow";
    static const gtString amdTeaPotSrcName3 = L"amdtteapot";

    gtString sourceCodeFileNameLower = sourceCodeFileName;
    sourceCodeFileNameLower.toLowerCase();

    if ((sourceCodeFileNameLower == amdTeaPotLibSrcName1) ||
        (sourceCodeFileNameLower == amdTeaPotLibSrcName2) ||
        (sourceCodeFileNameLower == amdTeaPotLibSrcName3) ||
        (sourceCodeFileNameLower == amdTeaPotLibSrcName4) ||
        (sourceCodeFileNameLower == amdTeaPotLibSrcName5) ||
        (sourceCodeFileNameLower == amdTeaPotLibSrcName6) ||
        (sourceCodeFileNameLower == amdTeaPotLibSrcName7) ||
        (sourceCodeFileNameLower == amdTeaPotLibSrcName8))
    {
        // In debug we will take the source file from its original location
        gtString CodeXLTeaPotDirString;
        osFilePath CodeXLTeapotSrcDir;
        bool rc = CodeXLTeapotSrcDir.SetInstallRelatedPath(osFilePath::OS_CODEXL_TEAPOT_SOURCES_LIB_PATH);
        GT_ASSERT(rc);

        CodeXLTeaPotDirString = CodeXLTeapotSrcDir.asString();

        retVal.setFileDirectory(CodeXLTeaPotDirString);
    }
    else if ((sourceCodeFileNameLower == amdTeaPotSrcName1) ||
             (sourceCodeFileNameLower == amdTeaPotSrcName2) ||
             (sourceCodeFileNameLower == amdTeaPotSrcName3))
    {
        // In debug we will take the source file from its original location
        gtString CodeXLTeaPotDirString;
        osFilePath CodeXLTeapotSrcDir;

        bool rc = CodeXLTeapotSrcDir.SetInstallRelatedPath(osFilePath::OS_CODEXL_TEAPOT_SOURCES_PATH);
        GT_ASSERT(rc);

        CodeXLTeaPotDirString = CodeXLTeapotSrcDir.asString();

        retVal.setFileDirectory(CodeXLTeaPotDirString);
    }

#endif

    return retVal;
}

gtString Util::SummaryTypeToGTString(afTreeItemType summaryType)
{
    gtString retVal;
    QString summaryTypeStr;

    switch (summaryType)
    {
        case AF_TREE_ITEM_PROFILE_GPU_API_SUMMARY:
            summaryTypeStr = Util::ms_APISUM;
            break;

        case AF_TREE_ITEM_PROFILE_GPU_CONTEXT_SUMMARY:
            summaryTypeStr = Util::ms_CTXSUM;
            break;

        case AF_TREE_ITEM_PROFILE_GPU_TOP10_KERNEL_SUMMARY:
            summaryTypeStr = Util::ms_TOP10KERNEL;
            break;

        case AF_TREE_ITEM_PROFILE_KERNEL_SUMMARY:
            summaryTypeStr = Util::ms_KERNELSUM;
            break;

        case AF_TREE_ITEM_PROFILE_GPU_TOP10_TRANSFER_SUMMARY:
            summaryTypeStr = Util::ms_TOP10DATA;
            break;

        case AF_TREE_ITEM_PROFILE_SESSION:
        case AF_TREE_ITEM_PROFILE_GPU_BEST_PRACTICE_SUMMARY:
            summaryTypeStr = Util::ms_BESTPRACTICES;
            break;

        default:
            GT_ASSERT_EX(false, L"Unknown summary type");
            break;
    }

    retVal = acQStringToGTString(summaryTypeStr);

    return retVal;
}

afTreeItemType Util::GetEnumTypeFromSumPageName(const QString& page)
{
    afTreeItemType retVal = AF_TREE_ITEM_PROFILE_GPU_API_SUMMARY;

    if (page.endsWith(Util::ms_APISUM))
    {
        retVal = AF_TREE_ITEM_PROFILE_GPU_API_SUMMARY;
    }
    else if (page.endsWith(Util::ms_CTXSUM))
    {
        retVal = AF_TREE_ITEM_PROFILE_GPU_CONTEXT_SUMMARY;
    }
    else if (page.endsWith(Util::ms_TOP10KERNEL))
    {
        retVal = AF_TREE_ITEM_PROFILE_GPU_TOP10_KERNEL_SUMMARY;
    }
    else if (page.endsWith(Util::ms_KERNELSUM))
    {
        retVal = AF_TREE_ITEM_PROFILE_KERNEL_SUMMARY;
    }
    else if (page.endsWith(Util::ms_TOP10DATA))
    {
        retVal = AF_TREE_ITEM_PROFILE_GPU_TOP10_TRANSFER_SUMMARY;
    }
    else if (page.endsWith(Util::ms_BESTPRACTICES))
    {
        retVal = AF_TREE_ITEM_PROFILE_GPU_BEST_PRACTICE_SUMMARY;
    }

    return retVal;
}


QString Util::GetShortFileNameFromSumPageName(const QString& page)
{
    QString retVal = page;

    if (page.endsWith(Util::ms_APISUM))
    {
        retVal = GPU_CSV_FileNameAPISummary;
    }
    else if (page.endsWith(Util::ms_CTXSUM))
    {
        retVal = GPU_CSV_FileNameContextSummary;
    }
    else if (page.endsWith(Util::ms_TOP10KERNEL))
    {
        retVal = GPU_CSV_FileNameTop10KernelSummary;
    }
    else if (page.endsWith(Util::ms_KERNELSUM))
    {
        retVal = GPU_CSV_FileNameKernelSummary;
    }
    else if (page.endsWith(Util::ms_TOP10DATA))
    {
        retVal = GPU_CSV_FileNameTop10DataSummary;
    }
    else if (page.endsWith(Util::ms_BESTPRACTICES))
    {
        retVal = GPU_CSV_FileNameErrorsWarnings;
    }

    return retVal;
}


bool Util::ParseOccupancyFileVersionString(const QString& versionStr, int& majorVersion, int& minorVersion)
{
    bool ok = false;
    int dotIndex = versionStr.indexOf('.');
    majorVersion = versionStr.mid(0, dotIndex).toInt(&ok);

    if (ok)
    {
        QString restOfVersionStr = versionStr.mid(dotIndex + 1);
        dotIndex = restOfVersionStr.indexOf('.');

        if (dotIndex != -1)
        {
            minorVersion = restOfVersionStr.mid(0, dotIndex).toInt(&ok);
        }
        else
        {
            minorVersion = restOfVersionStr.toInt(&ok);
        }
    }

    if (!ok)
    {
        majorVersion = minorVersion = 0;
    }

    return ok;
}

bool Util::LoadOccupancyFile(const osFilePath& sessionFilePath, OccupancyTable& occupancyTable, GPUSessionTreeItemData* pSessionData)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(pSessionData != nullptr)
    {
        QFileInfo fileInfo(acGTStringToQString(sessionFilePath.asString()));

        fileInfo.setFile(pSessionData->OccupancyFile());
        retVal = fileInfo.exists();

        if (retVal)
        {
            pSessionData->AddAdditionalFile(pSessionData->OccupancyFile());

            try
            {
                QFile inFile(pSessionData->OccupancyFile());
                retVal = inFile.open(QIODevice::ReadOnly | QIODevice::Text);

                if (retVal)
                {
                    // Update the progress bar
                    afProgressBarWrapper::instance().ShowProgressDialog(GPU_STR_TraceViewLoadingOccupancyProgress, 100);

                    float percentDone = 0;
                    qint64 fileSize = inFile.size();
                    bool readingProperties = true;
                    QString delimiter = ",";

                    while (readingProperties && !inFile.atEnd())
                    {
                        QString line = inFile.readLine();

                        if (line.trimmed().isEmpty() || line[0] == '#')
                        {
                            // Property section
                            QString propStr = line.mid(1);
                            int major, minor;
                            QStringList arrStr = propStr.split('=');

                            if (arrStr.length() == 2 && arrStr[0].trimmed() == "ProfilerVersion")
                            {
                                ParseOccupancyFileVersionString(arrStr[1], major, minor);
                            }
                            else if (arrStr.length() == 2 && arrStr[0].trimmed() == "ListSeparator")
                            {
                                delimiter = arrStr[1].trimmed();
                            }
                        }
                        else
                        {
                            readingProperties = false;
                        }
                    }

                    // when we exit the above loop, line contains the column headers, which aren't actually used by the client
                    while (!inFile.atEnd())
                    {
                        if ((inFile.pos() / (float)(fileSize)) > (percentDone + 0.01f))
                        {
                            percentDone += 0.01f;
                            afProgressBarWrapper::instance().setProgressText(GPU_STR_TraceViewLoadingOccupancyProgress);
                            afProgressBarWrapper::instance().updateProgressBar((int)(percentDone * 100));
                        }

                        // Get the next line and parse it
                        QString line = inFile.readLine();
                        retVal = retVal && Util::ParseOccupancySingleLine(line, delimiter, occupancyTable);

                        if (!retVal)
                        {
                            break;
                        }
                    }
                }
            }
            catch (...)
            {
                retVal = false;
            }

            if (!retVal)
            {
                ClearOccupancyTable(occupancyTable);
            }
        }

        if (!retVal)
        {
            Util::LogError("Unable to load occupancy data");
        }
    }

    return retVal;
}

bool Util::ParseOccupancySingleLine(const QString& line, const QString& delimiter, OccupancyTable& occupancyTable)
{
    bool retVal = true;

    if (!line.trimmed().isEmpty())
    {
        // Split the line
        QStringList tokens = line.split(delimiter, QString::SkipEmptyParts);

        bool allOk = true;
        QList<uint> uintValues;
        uint threadId = 0;
        double occupancy = 0.0;
        QString kernel;
        QString deviceName;
        uintValues.reserve(OccupancyInfo::OCCUPANCY_DATA_fieldsCount);

        for (int i = 0; i < OccupancyInfo::OCCUPANCY_DATA_fieldsCount; i++)
        {
            uintValues << 0;
        }

        // Expecting OccupancyInfo::OCCUPANCY_DATA_fieldsCount number of parameters in a line
        retVal = (tokens.length() == OccupancyInfo::OCCUPANCY_DATA_fieldsCount);
        GT_IF_WITH_ASSERT(retVal)
        {

            // We expect that the structure of the occupancy line is as follows
            // 0 threadId (uint), 1 kernel (string), 2 device name (string), 3 - 22 uint parameters
            // (computeUnits, maxWavesPerComputeUnit, maxWGPerCU, maxVGPRs, maxSGPRs, maxLDS, usedVGPRs, usedSGPRs, usedLDS, wavefrontSize, workGroupSize, wavesPerWorkGroup,
            // maxWorkGroupSize, maxWavesPerWorkGroup, globalWorkSize, maxGlobalWorkSize, wavesLimitedByVGPR, wavesLimitedBySGPR, wavesLimitedByLDS, wavesLimitedByWorkGroup,
            // 23 occupancy (double)

            bool ok = false;
            occupancy = tokens[OccupancyInfo::OCCUPANCY_DATA_occupancy].toDouble(&ok);
            kernel = tokens[OccupancyInfo::OCCUPANCY_DATA_kernelName];
            deviceName = tokens[OccupancyInfo::OCCUPANCY_DATA_deviceName];
            threadId = tokens[OccupancyInfo::OCCUPANCY_DATA_threadId].toUInt(&ok);
            allOk = allOk && ok;

            // Extract uint values 3-21
            for (int i = OccupancyInfo::OCCUPANCY_DATA_computeUnits; i <= OccupancyInfo::OCCUPANCY_DATA_wavesLimitedByWorkgroup; i++)
            {
                uintValues[i] = tokens[i].toUInt(&ok);
                allOk = allOk && ok;
            }
        }

        if (allOk)
        {
            OccupancyInfo* occupancyInfo = new OccupancyInfo(uintValues, occupancy, kernel, deviceName);

            if (occupancyTable.contains(threadId))
            {
                occupancyTable[threadId].append(occupancyInfo);
            }
            else
            {
                QList<OccupancyInfo*> list;
                list.append(occupancyInfo);
                occupancyTable.insert(threadId, list);
            }
        }
        else
        {
            retVal = false;
        }
    }

    return retVal;
}

void Util::ClearOccupancyTable(OccupancyTable& occTable)
{
    for (OccupancyTable::iterator it = occTable.begin(); it != occTable.end(); ++it)
    {
        QList<OccupancyInfo*> occInfoList = occTable[it.key()];

        for (QList<OccupancyInfo*>::iterator listIt = occInfoList.begin(); listIt != occInfoList.end(); ++listIt)
        {
            SAFE_DELETE(*listIt);
        }

        occInfoList.clear();
    }

    occTable.clear();
}



bool Util::CheckOccupancyDeviceName(const QString& strDeviceName1, const QString& strDeviceName2)
{
    bool retVal = false;

    if (strDeviceName1 == strDeviceName2)
    {
        retVal = true;
    }
    else
    {
        QString strAlternateDeviceName1 = strDeviceName1;
        QString strAlternateDeviceName2 = strDeviceName2;
        strAlternateDeviceName1.replace(" ", "_");
        strAlternateDeviceName2.replace(" ", "_");
        retVal = strAlternateDeviceName1 == strAlternateDeviceName2;
    }

    return retVal;
}

bool Util::IsHSAEnabled()
{
    //on windows by default it's false
    bool isHSAInstalled = false;

    const auto& projectSettings = afProjectManager::instance().currentProjectSettings();
    if (projectSettings.isRemoteTarget())
    {
        // Retrieve the daemon's address.
        const auto dmnPort = projectSettings.remoteTargetDaemonConnectionPort();
        const auto dmnIp = projectSettings.remoteTargetName();

        osPortAddress daemonAddr(dmnIp, dmnPort);

        // Initialize the daemon if required.
        static const unsigned CONNECTION_VALIDATION_TIMEOUT_MS = 1500;
        bool retVal = CXLDaemonClient::IsInitialized(daemonAddr) || CXLDaemonClient::Init(daemonAddr, CONNECTION_VALIDATION_TIMEOUT_MS);
        GT_ASSERT_EX(retVal, GPU_STR_REMOTE_AGENT_INIT_FAILURE_WITH_CTX);

        CXLDaemonClient* pDmnClient = CXLDaemonClient::GetInstance();
        GT_IF_WITH_ASSERT(pDmnClient != NULL)
        {
            if (retVal)
            {
                // Connect to the daemon.
                osPortAddress addrBuffer;
                retVal = pDmnClient->ConnectToDaemon(addrBuffer);
                GT_IF_WITH_ASSERT(retVal)
                {
                    isHSAInstalled = pDmnClient->IsHSAEnabled();
                }
            }
        }
    }
//on Linux we do real check if machine supports HSA
#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS)
    else
    {
        isHSAInstalled = (afGlobalVariablesManager::instance().InstalledAMDComponentsBitmask() & AF_AMD_HSA_COMPONENT);
    }
#endif
    return isHSAInstalled;
}