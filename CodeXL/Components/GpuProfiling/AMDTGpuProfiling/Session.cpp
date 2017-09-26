//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/Session.cpp $
/// \version $Revision: #62 $
/// \brief :  This file contains GPUSessionTreeItemData class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/Session.cpp#62 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtCore>
#include <QtWidgets>

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>

// Local:
#include <AMDTGpuProfiling/gpExecutionMode.h>
#include <AMDTGpuProfiling/gpStringConstants.h>
#include <AMDTGpuProfiling/gpViewsCreator.h>
#include <AMDTGpuProfiling/Session.h>
#include <AMDTGpuProfiling/ProfileManager.h>
#include "AtpUtils.h"

static int allocCounter = 0;


GPUSessionTreeItemData::GPUSessionTreeItemData() : SessionTreeNodeData()
{

}

GPUSessionTreeItemData::GPUSessionTreeItemData(const QString& strName,
                                               const QString& strWorkingDirectory,
                                               const QString& strSessionFilePath,
                                               const QString& strProjName,
                                               GPUProfileType profileType,
                                               bool isImported)
    : SessionTreeNodeData()
{
    m_workingDirectory = strWorkingDirectory;
    m_profileType = profileType;
    m_exeName.clear();
    m_propertyAdded = false;
    m_additionalFiles.clear();

    // Set the session name & display name:
    m_name = strName;
    m_displayName = strName;

    // When session is loaded from .csv file or .atp file,
    // unless we write project name into .csv file and .atp file, we set project name to "Unknown Project"
    m_projectName = strProjName;
    m_occupancyFileIsLoaded = false;
    m_occupancyFileLoadExecuted = false;
    m_properties.clear();
    m_propertyLinesCount = 0;
    m_isPropertySectionLoaded = false;
    m_versionMajor = m_versionMinor = -1;
    m_sessionAPIToTrace = APIToTrace_OPENCL;

    // call LoadProperties so that a renamed session will show the correct name
    LoadProperties(strSessionFilePath);
    m_isImported = isImported;
    m_displayName = strName;

}

GPUSessionTreeItemData::GPUSessionTreeItemData(const GPUSessionTreeItemData& other)
    : SessionTreeNodeData((const SessionTreeNodeData&)other)
{
    m_profileType = other.m_profileType;
    m_exeName = other.m_exeName;
    m_propertyAdded = other.m_propertyAdded;
    m_projectName = other.m_projectName;
    m_occupancyFileIsLoaded = other.m_occupancyFileIsLoaded;
    m_occupancyFileLoadExecuted = other.m_occupancyFileLoadExecuted;
    m_propertyLinesCount = other.m_propertyLinesCount;
    m_isPropertySectionLoaded = other.m_isPropertySectionLoaded;
    m_versionMajor = other.m_versionMajor;

    foreach (QString additionalFile, other.m_additionalFiles)
    {
        m_additionalFiles.append(additionalFile);
    }

    for (QList<QPair<QString, QString> >::const_iterator i = other.m_properties.begin(); i != other.m_properties.end(); ++i)
    {
        QString propName = (*i).first;
        QString propVal = (*i).second;
        m_properties.append(QPair<QString, QString>(propName, propVal));
    }
}

GPUSessionTreeItemData::~GPUSessionTreeItemData()
{
}


void GPUSessionTreeItemData::SearchForAdditionalFiles()
{
    // Search for additional files:
    QString directoryPath = QString::fromWCharArray(SessionDir().directoryPath().asString().asCharArray());
    QDir outDir(directoryPath);
    QFileInfoList files = outDir.entryInfoList(QDir::Files);

    foreach (QFileInfo fileInfo, files)
    {
        if (IsAdditionalFile(fileInfo))
        {
            if (fileInfo.exists())
            {
                AddAdditionalFile(fileInfo.filePath());
            }
        }
    }
}

void GPUSessionTreeItemData::AddAdditionalFile(const QString& file)
{
    QString filePath = Util::ToQtPath(file);

    if (!m_additionalFiles.contains(filePath))
    {
        m_additionalFiles.append(filePath);
    }
}

const QStringList& GPUSessionTreeItemData::GetAdditionalFiles()
{
    return m_additionalFiles;
}

void GPUSessionTreeItemData::SetAdditionalFiles(const QStringList& additionalFilesList)
{
    m_additionalFiles.clear();
    m_additionalFiles = additionalFilesList;
}

bool GPUSessionTreeItemData::UpdatePropertiesSection(QString& errorMessage)
{
    bool retVal = true;

    try
    {
        errorMessage.clear();
        QString lineStart;

        if (m_profileType == PERFORMANCE)
        {
            lineStart = "# ";
        }

        // create a temp file to use to rewrite the output file with the new properties
        QTemporaryFile tempFile;

        if (!tempFile.open())
        {
            errorMessage = "Error in saving properties.  Unable to open temp file for saving"; // FIXME::Give system error
            retVal = false;
        }
        else
        {
            QString tempFileName = tempFile.fileName();
            // creating temp file
            {

                // Sanity check:
                GT_IF_WITH_ASSERT(m_pParentData != nullptr)
                {
                    QFile inFile(acGTStringToQString(m_pParentData->m_filePath.asString()));

                    if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text))
                    {
                        errorMessage = "Error in saving properties.  Unable to open session file"; // FIXME::Give system error
                        retVal = false;
                    }
                    else
                    {
                        QTextStream sr(&inFile);

                        QFile outFile(tempFileName);
                        outFile.open(QIODevice::WriteOnly | QIODevice::Text);
                        QTextStream sw(&outFile);

                        // write each property
                        for (QList<QPair<QString, QString> >::const_iterator i = m_properties.begin(); i != m_properties.end(); ++i)
                        {
                            sw << lineStart << (*i).first << "=" << (*i).second << QChar((int)'\n');
                        }

                        QString line;

                        // move beyond property section of original output file
                        while (!sr.atEnd())
                        {
                            line = sr.readLine();

                            if (m_profileType == PERFORMANCE && (line.trimmed().isEmpty() || line[0] != '#'))
                            {
                                break;
                            }
                            else if (m_profileType == API_TRACE && (line.trimmed().isEmpty() || line[0] == '='))
                            {
                                break;
                            }
                        }

                        if (!line.trimmed().isEmpty())
                        {
                            // write the line that caused the break out of the above while loop
                            sw << line << QChar((int)'\n');
                        }

                        // write the rest of the file
                        while (!sr.atEnd())
                        {
                            sw << sr.readLine() << QChar((int)'\n');
                        }

                        outFile.close();
                        inFile.close();
                    }
                }
            }


            // copy the temp file over the output file and then delete the temp file
            // Sanity check:
            GT_IF_WITH_ASSERT(m_pParentData != nullptr)
            {
                QString filePath = acGTStringToQString(m_pParentData->m_filePath.asString());
                QFile::remove(filePath);
                QFile::copy(tempFileName, filePath);
                QFile::remove(tempFileName);

            }

            if (m_propertyAdded)
            {
                ResetProperties();
            }
        }
    }
    catch (...)
    {
        errorMessage = "Error in saving session properties."; // FIXME::Give system error
        retVal = false;
    }

    if (!errorMessage.isEmpty())
    {
        Util::LogError(errorMessage);
    }

    return retVal;
}

bool GPUSessionTreeItemData::IsSessionFileValid()
{
    bool retVal = false;
    QString strPropVal;
    QString strRequiredSessionProperty = GetValidSessionProperty();

    if (!strRequiredSessionProperty.isEmpty())
    {
        retVal = GetProperty(strRequiredSessionProperty, strPropVal);
    }
    else if (m_profileType == FRAME_ANALYSIS)
    {
        retVal = true;
    }

    return retVal;
}

bool GPUSessionTreeItemData::LoadProperties(const QString& strSessionFilePath)
{
    bool retVal = false;

    if (m_isPropertySectionLoaded)
    {
        retVal = true;
    }
    else
    {
        m_propertyLinesCount = 0;
        QFile inFile(strSessionFilePath);

        if (inFile.open(QIODevice::ReadOnly))
        {
            QTextStream sr(&inFile);

            while (!sr.atEnd())
            {
                QString line = sr.readLine();

                if (line.trimmed().isEmpty() ||                        // if line is empty
                    (m_profileType == API_TRACE && line[0] != '=') ||  // or this is a trace file and we haven't hit the ===== divider yet
                    (m_profileType == PERFORMANCE && line[0] == '#'))  // or this is a perfcounter file and we find a property line (which starts with #)
                {
                    if (m_profileType == PERFORMANCE)
                    {
                        line = line.mid(1); //skip the # in the perf counter file
                    }

                    // properties
                    AddProperty(line);
                    m_propertyLinesCount++;
                }
                else
                {
                    m_isPropertySectionLoaded = true;
                    retVal = true;
                    break;
                }
            }
        }
    }

    return retVal;
}

bool GPUSessionTreeItemData::IsAdditionalFile(const QFileInfo& fileInfo)
{
    bool retVal = false;
    QString fileExt = fileInfo.suffix();
    QString baseName = fileInfo.completeBaseName();

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pParentData != nullptr)
    {
        if (fileExt == "occupancy")
        {
            QFileInfo sessionFile(acGTStringToQString(m_pParentData->m_filePath.asString()));

            if (baseName == sessionFile.completeBaseName())
            {
                retVal = true;
                osFilePath occupancyFilePath;
                occupancyFilePath.setFileDirectory(SessionDir());
                occupancyFilePath.setFileName(acQStringToGTString(baseName));
                occupancyFilePath.setFileExtension(L"occupancy");
                m_occupancyFile = acGTStringToQString(occupancyFilePath.asString());
            }
        }
        else if (fileExt == "html")
        {
            retVal = fileInfo.baseName().endsWith("_Occupancy");
        }
        else
        {
            retVal = fileExt == "js" || fileExt == "css";
        }

    }

    return retVal;
}

void GPUSessionTreeItemData::AddProperty(const QString& input)
{
    QStringList arrStr = input.split('=');
    QString propName;
    QString propVal;

    if (arrStr.length() == 2)
    {
        propName = arrStr[0].trimmed();
        propVal = arrStr[1];

        m_properties.append(QPair<QString, QString>(propName, propVal));

        if (propName == "Application")
        {
            m_exeFullPath = propVal.trimmed();
            propVal = propVal.replace('\\', '/');
            int idx = propVal.lastIndexOf('/');

            if (idx >= 0)
            {
                m_exeName = propVal.mid(idx + 1);
            }
            else
            {
                m_exeName = propVal;
            }
        }
        else if (propName == "ApplicationArgs")
        {
            m_commandArguments = propVal.trimmed();
        }
        else if (propName == "API")
        {
            m_sessionAPIToTrace = APIToTrace_Unknown;

            if (propVal == "OpenCL")
            {
                m_sessionAPIToTrace = APIToTrace_OPENCL;
            }
            else if (propVal == "HSA")
            {
                m_sessionAPIToTrace = APIToTrace_HSA;
            }
        }
        else if (propName == "WorkingDirectory")
        {
            m_workingDirectory = propVal.trimmed();
        }
        else if (propName == "TraceFileVersion")
        {
            int major, minor;

            if (Util::ParseOccupancyFileVersionString(propVal, major, minor))
            {
                m_versionMajor = major;
                m_versionMinor = minor;
            }
        }
        else if (propName == GP_Str_ATPPropertyDisplayName)
        {
            propVal = propVal.trimmed();

            if (!propVal.isEmpty())
            {
                m_displayName = propVal;
            }
        }
    }
    else if (arrStr.size() == 3)
    {
        propName = arrStr[0].trimmed();

        // Environment variables structure: EnvVar=name1=value1:
        if (propName == "EnvVar")
        {
            QString envName = arrStr[1];
            QString envValue = arrStr[2];

            if (!m_envVariables.isEmpty())
            {
                m_envVariables.append(L"\n");
            }

            QString envVal = envName + "=" + envValue;
            gtString envVarStr = acQStringToGTString(envVal);
            m_envVariables.append(envVarStr);
        }
    }
}

void GPUSessionTreeItemData::ResetProperties()
{
    m_properties.clear();
    m_isPropertySectionLoaded = false;
    m_propertyLinesCount = 0;
    m_propertyAdded = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pParentData != nullptr)
    {
        LoadProperties(acGTStringToQString(m_pParentData->m_filePath.asString()));
    }
}


void GPUSessionTreeItemData::SetDisplayName(const QString& displayName)
{
    SessionTreeNodeData::SetDisplayName(displayName);

    //Update output file
    QString errorMessage;

    if (!UpdateDisplayNameInOutputFile(displayName, errorMessage))
    {
        Util::ShowWarningBox(errorMessage);
    }
}

bool GPUSessionTreeItemData::UpdateDisplayNameInOutputFile(const QString& displayName, QString& errorMessage)
{
    bool foundExisting = false;

    for (QList<QPair<QString, QString> >::iterator it = m_properties.begin(); it != m_properties.end(); ++it)
    {
        if ((*it).first == GP_Str_ATPPropertyDisplayName)
        {
            QString strCurDisplayName = (*it).second;

            if (strCurDisplayName != displayName)
            {
                foundExisting = true;
                (*it).second = displayName;
            }

            break;
        }
    }

    if (!foundExisting)
    {
        m_propertyAdded = true;
        m_properties.append(QPair<QString, QString>(GP_Str_ATPPropertyDisplayName, displayName));
    }

    return UpdatePropertiesSection(errorMessage);
}

void GPUSessionTreeItemData::FlushData()
{
    m_occupancyFileIsLoaded = false;
}

GPUProfileType GPUSessionTreeItemData::GetProfileType() const
{
    return m_profileType;
}

QString GPUSessionTreeItemData::GetOccupancyFile() const
{
    return m_occupancyFile;
}

int GPUSessionTreeItemData::GetPropertyCount()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pParentData != nullptr)
    {
        LoadProperties(acGTStringToQString(m_pParentData->m_filePath.asString()));
    }
    return m_propertyLinesCount;
}

bool GPUSessionTreeItemData::GetProperty(const QString& strPropName, QString& strPropValue)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pParentData != nullptr)
    {
        LoadProperties(acGTStringToQString(m_pParentData->m_filePath.asString()));
    }

    bool retVal = false;
    strPropValue.clear();

    for (QList<QPair<QString, QString> >::const_iterator it = m_properties.begin(); it != m_properties.end(); ++it)
    {
        if ((*it).first == strPropName)
        {
            strPropValue = (*it).second;
            retVal = true;
            break;
        }
    }

    return retVal;
}

int GPUSessionTreeItemData::GetVersionMajor() const
{
    return m_versionMajor;
}

int GPUSessionTreeItemData::GetVersionMinor() const
{
    return m_versionMinor;
}

const OccupancyTable& GPUSessionTreeItemData::LoadAndGetOccupancyTable()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pParentData != nullptr)
    {
        if (!m_occupancyFileIsLoaded)
        {
            if (!AtpUtils::Instance()->IsModuleLoaded())
            {
                AtpUtils::Instance()->LoadModule();
            }

            void* pPtr;
            AtpDataHandlerFunc pAtpDataHandler_func = AtpUtils::Instance()->GetAtpDataHandlerFunc();
            IOccupancyFileInfoDataHandler* pOccupancyFileDataInfo = nullptr;

            if (nullptr != pAtpDataHandler_func)
            {
                pAtpDataHandler_func(&pPtr);
                IAtpDataHandler* pApDataHandler = reinterpret_cast<IAtpDataHandler*>(pPtr);
                std::string occupancyFile = m_occupancyFile.toStdString();

                pOccupancyFileDataInfo = pApDataHandler->GetOccupancyFileInfoDataHandler(occupancyFile.c_str());

                if (nullptr != pOccupancyFileDataInfo)
                {
                    if (!pOccupancyFileDataInfo->IsDataReady())
                    {
                        m_occupancyFileIsLoaded = pOccupancyFileDataInfo->ParseOccupancyFile(occupancyFile.c_str());
                    }
                }
            }

            if (m_occupancyFileIsLoaded && nullptr != pOccupancyFileDataInfo)
            {
                osThreadId* pOsThreadIds;
                unsigned int threadCount;
                pOccupancyFileDataInfo->GetOccupancyThreads(&pOsThreadIds, threadCount);

                for (unsigned int i = 0; i < threadCount; i++)
                {
                    const IOccupancyInfoDataHandler* occupancyInfo;
                    unsigned int kernelCount;
                    pOccupancyFileDataInfo->GetKernelCountByThreadId(pOsThreadIds[i], kernelCount);
                    QList<const IOccupancyInfoDataHandler*> occupancyInfoList;

                    for (unsigned int j = 0; j < kernelCount; j++)
                    {
                        occupancyInfo = pOccupancyFileDataInfo->GetOccupancyInfoDataHandler(pOsThreadIds[i], j);
                        occupancyInfoList.push_back(occupancyInfo);
                    }

                    m_occupancyTable.insert(pOsThreadIds[i], occupancyInfoList);
                }
            }
        }
    }
    return m_occupancyTable;
}

void GPUSessionTreeItemData::SetFilesFolder(const gtString& newFolder)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pParentData != nullptr)
    {
        // Iterate the additional files and relocate them:
        QStringList newAdditionalFiles;

        foreach (QString additionalFile, m_additionalFiles)
        {
            gtString fileName, fileExt;
            osFilePath sourcePath(acQStringToGTString(additionalFile));
            sourcePath.setFileDirectory(newFolder);

            // Append the new file to the list of relocated files:
            newAdditionalFiles << acGTStringToQString(sourcePath.asString());
        }

        osFilePath outputFilePath(m_pParentData->m_filePath);
        outputFilePath.setFileDirectory(newFolder);

        // Set the relocated profile output path:
        m_pParentData->m_filePath = outputFilePath;
    }
}


void GPUSessionTreeItemData::GetSessionCSVFile(osFilePath& sessionCSVFile) const
{
    gtString extension;

    if (m_pParentData != nullptr)
    {
        m_pParentData->m_filePath.getFileExtension(extension);

        if (extension == AF_STR_GpuProfileSessionFileExtension)
        {
            QString filePathStr;
            bool rc = gpViewsCreator::GetSessionFileFromTempPCFile(m_pParentData->m_filePath, filePathStr);
            GT_IF_WITH_ASSERT(rc)
            {
                sessionCSVFile = acQStringToGTString(filePathStr);
            }
        }
        else
        {
            sessionCSVFile = m_pParentData->m_filePath;
        }
    }
}

//--------------------------------------------------------
// TraceSession
//--------------------------------------------------------
TraceSession::TraceSession(const QString& strName,
                           const QString& strWorkingDirectory,
                           const QString& strSessionFilePath,
                           const QString& strProjName,
                           bool isImported)
    : GPUSessionTreeItemData(strName,
                             strWorkingDirectory,
                             strSessionFilePath,
                             strProjName,
                             API_TRACE,
                             isImported),
      m_exlcudedAPIsChecked(false)
{
}

TraceSession::~TraceSession()
{
}

bool TraceSession::IsAdditionalFile(const QFileInfo& fileInfo)
{
    bool retVal = false;

    QString fileName = fileInfo.fileName();
    QString fileExt = fileInfo.suffix();
    QString baseName = fileInfo.baseName();

    bool isMarkersFile = (fileExt == "st") || (fileExt == GP_Str_MarkersFileExtension);
    bool isRulesFile = (fileName == GPU_RulesFullFileName);
    bool isHTMLSummary = fileName.endsWith(Util::ms_APISUMFILE) ||
                         fileName.endsWith(Util::ms_BESTPRACTICESFILE) ||
                         fileName.endsWith(Util::ms_CTXSUMFILE) ||
                         fileName.endsWith(Util::ms_KERNELSUMFILE) ||
                         fileName.endsWith(Util::ms_TOP10DATAFILE) ||
                         fileName.endsWith(Util::ms_TOP10KERNELFILE);

    // Summary pages should be related to the current project:
    if (isHTMLSummary)
    {
        isHTMLSummary = fileName.startsWith(m_name);
    }

    retVal = isMarkersFile || isRulesFile || isHTMLSummary || GPUSessionTreeItemData::IsAdditionalFile(fileInfo);

    return retVal;
}

void TraceSession::ResetProperties()
{
    GPUSessionTreeItemData::ResetProperties();

    m_excludedAPIs.clear();
    m_exlcudedAPIsChecked = false;
}

QString TraceSession::GetValidSessionProperty() const
{
    return QString("TraceFileVersion");
}

bool TraceSession::GetExcludedAPIs(QStringList& excludedAPIsList)
{
    bool retVal = false;

    if (!m_exlcudedAPIsChecked)
    {
        QString excludedAPIs;

        // read OCL excluded APIs
        if (GetProperty("CLExcludedAPIs", excludedAPIs) && !excludedAPIs.isEmpty())
        {
            m_excludedAPIs = excludedAPIs.split(',');
        }

        // read HSA excluded APIs, and merge them into the list of OCL excluded APIs
        if (GetProperty("HSAExcludedAPIs", excludedAPIs) && !excludedAPIs.isEmpty())
        {
            m_excludedAPIs.append(excludedAPIs.split(','));
        }

        m_exlcudedAPIsChecked = true;
    }

    retVal = !m_excludedAPIs.isEmpty();

    if (retVal)
    {
        excludedAPIsList = m_excludedAPIs;
    }
    else
    {
        excludedAPIsList.clear();
    }

    return retVal;
}

void TraceSession::FlushData()
{
    GPUSessionTreeItemData::FlushData();
}

//--------------------------------------------------------
// PerformanceCounterSession
//--------------------------------------------------------
PerformanceCounterSession::PerformanceCounterSession(const QString& strName,
                                                     const QString& strWorkingDirectory,
                                                     const QString& strSessionFilePath,
                                                     const QString& strProjName,
                                                     bool isImported)
    : GPUSessionTreeItemData(strName,
                             strWorkingDirectory,
                             strSessionFilePath,
                             strProjName,
                             PERFORMANCE,
                             isImported)
{
}

PerformanceCounterSession::~PerformanceCounterSession()
{
}

bool PerformanceCounterSession::IsAdditionalFile(const QFileInfo& fileInfo)
{
    bool retVal = false;

    QString basename = fileInfo.fileName();
    QString fileExt = fileInfo.suffix();

    if (basename.startsWith(Util::ms_KERNEL_ASSEMBLY_FILE_PREFIX))
    {
        retVal = fileExt.compare("cl", Qt::CaseInsensitive) == 0 ||
                 fileExt.compare("hsail", Qt::CaseInsensitive) == 0 ||
                 fileExt.compare("il", Qt::CaseInsensitive) == 0 ||
                 fileExt.compare("isa", Qt::CaseInsensitive) == 0 ||
                 fileExt.compare("asm", Qt::CaseInsensitive) == 0;
    }
    else
    {
        retVal = (basename == GPU_PerformanceCountersFullFileName) || GPUSessionTreeItemData::IsAdditionalFile(fileInfo);
    }

    return retVal;
}

QString PerformanceCounterSession::GetValidSessionProperty() const
{
    return QString(GPU_STR_FileHeader_ProfileFileVersion);
}

void PerformanceCounterSession::UpdateRenamePCTmpFile()
{
    // Write the renamed session file name to the temp file:
    osFile tempFile;
    bool rc = tempFile.open(m_sessionTemporaryFile, osChannel::OS_UNICODE_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);
    GT_IF_WITH_ASSERT(rc && (m_pParentData != nullptr))
    {
        rc = tempFile.writeString(m_pParentData->m_filePath.asString());
        GT_IF_WITH_ASSERT(rc)
        {
            tempFile.close();
        }
    }
}


osDirectory PerformanceCounterSession::SessionDir() const
{
    osDirectory retVal;

    osFilePath sessionCSVFile;
    GetSessionCSVFile(sessionCSVFile);

    if (!sessionCSVFile.getFileDirectory(retVal))
    {
        retVal = osDirectory();
    }

    return retVal;
}

//--------------------------------------------------------
// gpSessionTreeNodeData
//--------------------------------------------------------
gpSessionTreeNodeData::gpSessionTreeNodeData(const QString& strName,
                                             const QString& strWorkingDirectory,
                                             const QString& strSessionFilePath,
                                             const QString& strProjName,
                                             bool isImported)
    : GPUSessionTreeItemData(strName,
                             strWorkingDirectory,
                             strSessionFilePath,
                             strProjName,
                             FRAME_ANALYSIS,
                             isImported)
{
    m_frameIndex.first = -1;
    m_frameIndex.second = -1;
    m_strSessionFilePath = strSessionFilePath;

    allocCounter++;
}

gpSessionTreeNodeData::~gpSessionTreeNodeData()
{
    allocCounter--;
}

void gpSessionTreeNodeData::StartSessionHTML(afHTMLContent& htmlContent) const
{
    htmlContent.setTitle(GP_Str_FrameAnalysisSessionHTMLHeading);
}

bool gpSessionTreeNodeData::DeleteSessionFilesFromDisk(QString& report)
{
    bool retVal = false;

    // Make sure that session files are deleted on the server
    GT_IF_WITH_ASSERT((ProfileManager::Instance() != nullptr) && (ProfileManager::Instance()->GetFrameAnalysisModeManager() != nullptr))
    {
        // Make sure that frame analysis sessions are deleted from the server
        ProfileManager::Instance()->GetFrameAnalysisModeManager()->HandleSessionDeletion(this);
    }

    // Call the base class implementation
    SessionTreeNodeData::DeleteSessionFilesFromDisk(report);
    return retVal;

}

