//=============================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \version $Revision: #6 $
/// \brief  This file is the header file for kaBackendManager class.
//
//=============================================================================
// $Id: //devtools/main/CodeXL/Components/ShaderAnalyzer/AMDTKernelAnalyzer/src/kaBackendManager.cpp#6 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=============================================================================

// Qt
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

#include <cstring>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <list>
#include <memory>
#include <string>
#include <typeinfo>
#include <vector>
#include <cmath>
#include <DeviceInfoUtils.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #pragma warning(push)
    #pragma warning(disable : 4127)
    #pragma warning(disable : 4251)
    #pragma warning(pop)
#endif

// C.
#include <stdio.h>

#include "boost/foreach.hpp"
#include "boost/shared_array.hpp"
#include "boost/smart_ptr.hpp"

// Infra
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osRawMemoryBuffer.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSAPIWrappers/Include/oaDriver.h>

// Framework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationFramework/Include/afMessageBox.h>

// Local
#include <AMDTKernelAnalyzer/src/kaApplicationTreeHandler.h>
#include <AMDTKernelAnalyzer/src/kaBackendManager.h>
#include <AMDTKernelAnalyzer/src/kaTreeModel.h>
#include <AMDTKernelAnalyzer/src/kaProjectDataManager.h>
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>
#include <AMDTKernelAnalyzer/src/kaDataAnalyzerFunctions.h>
#include <AMDTKernelAnalyzer/src/kaProjectDataManager.h>
#include <AMDTKernelAnalyzer/src/kaCliLauncher.h>
#include <AMDTKernelAnalyzer/Include/kcCliStringConstants.h>
#include <AMDTKernelAnalyzer/src/kaUtils.h>

// Backend.
#include <AMDTBackEnd/Include/beProgramBuilderOpenCL.h>
#include <AMDTBackEnd/Include/beInclude.h>
#include <AMDTBackEnd/Include/beBackend.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #include <processthreadsapi.h>
    #include <AMDTBaseTools/Include/gtString.h>
    #include <AMDTOSWrappers/Include/osFile.h>
    #include <AMDTOSWrappers/Include/osTime.h>
    #include <wtypesbase.h>
    #include <wtypes.h>
#endif

static const int POLLING_TIME = 25;  // milliseconds

// The following data should have unique string among them.  The strings
// can be anything so long they are not the same.  The strings are used
// as type markers.
static const char* const MY_BOOL = "b";
static const char* const MY_INT = "i";
static const char* const MY_STR = "s";

static bool s_forcedReset = false;

// Build output file types.
enum BuildOutputFileType
{
    // ISA file.
    boftISA,

    // IL or DX ASM file.
    boftIL,

    // Analysis file.
    boftAnalysis,

    // Statistics file.
    boftStatistics,

    // ELF Binary file
    boftBinary
};

// *** INTERNALLY-LINKED AUXILIARY FUNCTIONS - BEGIN ***

static gtString GenerateBuildOutputFileName(const gtString& fileName, const  BuildOutputFileType fileType, const gtString& outputDir)
{
    gtString ret(outputDir);
    ret.append(LR"(\)").append(fileName);

    if (fileType == boftIL)
    {
        ret << L"." << KA_STR_kernelViewILExtension;
    }
    else if (fileType == boftISA)
    {
        ret << L"." << KA_STR_kernelViewISAExtension;
    }
    else if (fileType == boftAnalysis)
    {
        ret << L"." << KA_STR_kernelViewISAExtension;
    }
    else if (fileType == boftStatistics)
    {
        ret << L"." << KA_STR_kernelViewExtension;
    }
    else if (fileType == boftBinary)
    {
        ret << L"." << KA_STR_kernelViewBinExtension;
    }

    return ret;
}

static gtString GenerateRenderBuildOutputFileName(BuildOutputFileType fileType, const gtString& outputDir)
{
    gtString ret(outputDir);
    ret.append(LR"(\)");

    if (fileType == boftIL)
    {
        ret << L"." << KA_STR_kernelViewILExtension;
    }
    else if (fileType == boftISA)
    {
        ret << L"." << KA_STR_kernelViewISAExtension;
    }
    else if (fileType == boftStatistics)
    {
        ret << L"." << KA_STR_kernelViewExtension;
    }
    else if (fileType == boftBinary)
    {
        ret << L"." << KA_STR_buildMainBinaryFileName;
    }

    return ret;
}

static bool AggregateAndRemoveStatisticsFiles(const gtString& statisticsFileName, const gtVector<gtString>& statisticsFiles)
{
    bool ret = false;
    bool isHeadersInAggregateFile = false;
    gtASCIIString aggregateText;

    for (const gtString& statsFile : statisticsFiles)
    {
        osFile deviceFile;
        gtASCIIString textLine;

        if (deviceFile.open(osFilePath(statsFile), osFile::OS_ASCII_TEXT_CHANNEL))
        {
            // First line is headers
            deviceFile.readLine(textLine);

            if (!isHeadersInAggregateFile)
            {
                aggregateText.append(textLine);
                aggregateText.append('\n');
                isHeadersInAggregateFile = true;
            }

            // Read the second line that contains the statistics
            deviceFile.readLine(textLine);
            aggregateText.append(textLine);
            aggregateText.append('\n');

            // Delete the single device statistics file
            deviceFile.close();
            deviceFile.deleteFile();
        }
    }

    // Write the aggregated statistics to a single file
    osFile aggregateFile;

    if (aggregateFile.open(osFilePath(statisticsFileName), osFile::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE))
    {
        ret = aggregateFile.write(aggregateText.asCharArray(), aggregateText.length());
    }

    return ret;
}

static gtString GenerateMetaFileName(const std::string& entryPointName, const std::string& deviceName, BuildOutputFileType fileType)
{
    gtString ret;
    ret << entryPointName.c_str() << " ";

    if (fileType == boftIL)
    {
        ret << deviceName.c_str() << "_" << KA_STR_buildMainILFileName << L"$0";
    }
    else if (fileType == boftISA)
    {
        ret << deviceName.c_str() << "_" << KA_STR_buildMainISAFileName << L"$1";
    }
    else if (fileType == boftAnalysis)
    {
        ret << KA_STR_buildMainAnalysisFileName << L"_NA$3";
    }
    else if (fileType == boftStatistics)
    {
        ret << KA_STR_buildMainStatisticsFileName << L"_$4";
    }
    else if (fileType == boftBinary)
    {
        ret << KA_STR_buildMainBinaryFileName << L"_$5";
    }

    return ret;
}


// *** INTERNALLY-LINKED AUXILIARY FUNCTIONS - END ***

std::queue<std::string> kaBackendManager::g_backendMessages;
QMutex                  kaBackendManager::g_backendMessageMutex;

enum FieldIndex
{
    NAME_INDEX,
    TOOLTIP_INDEX,
    TYPE_INDEX,

    TOTAL_INDEX
};

// This implements all the boost members that were in the kaBackendManager
// This is done so that boost is not exposed through kaBackendManager to VS which might cause
// conflict in compilations
class kaBackEndSmartPointers
{
public:
    kaBackEndSmartPointers();
    ~kaBackEndSmartPointers();

    /// Timer for reading logs
    boost::shared_ptr<QTimer>                       m_pStreamReadTimer;

    /// Build thread
    boost::shared_ptr<kaBackendManager::BuildThread>    m_pBuildThread;

    /// Build options.
    boost::shared_ptr<beProgramBuilderOpenCL::OpenCLOptions>    m_pCLOptions;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    /// DX build options.
    std::vector<boost::shared_ptr<beProgramBuilderDX::DXOptions>>  m_pDXOptions;
#endif

    /// A string holding source code for build
    boost::shared_ptr<std::string>                  m_pSourceCode;
};

kaBackEndSmartPointers::kaBackEndSmartPointers() : m_pStreamReadTimer(new QTimer), m_pSourceCode(new string)
{

}

kaBackEndSmartPointers::~kaBackEndSmartPointers() { ; };

// Static member initializations:
kaBackendManager* kaBackendManager::m_psMySingleInstance = nullptr;

//-----------------------------------------------------------------------------
kaBackendManager::kaBackendManager() : m_firstTimeRun(true), m_isInBuild(false), m_isInitialized(false)
{
    m_pBoost = new kaBackEndSmartPointers;
    m_pBackend = Backend::Instance();

    bool isCatalystInstalled = false;
    int driverError = OA_DRIVER_UNKNOWN;
    gtString driverVersion = oaGetDriverVersion(driverError);

    if (driverError != OA_DRIVER_NOT_FOUND)
    {
        isCatalystInstalled = true;
    }

    if (isCatalystInstalled && m_pBackend != nullptr && m_pBackend->Initialize(BuiltProgramKind_OpenCL, backendMessageCallback))
    {
        std::set<string> devices;

        beKA::beStatus beStatus = m_pBackend->theOpenCLBuilder()->GetDevices(devices);

        if (beKA::beStatus_SUCCESS == beStatus)
        {
            m_deviceNames = devices;
        }

        m_pBoost->m_pStreamReadTimer->setInterval(POLLING_TIME);
        bool rc = connect(m_pBoost->m_pStreamReadTimer.get(), SIGNAL(timeout()), this, SLOT(readLog()));
        GT_ASSERT(rc);

        m_pBoost->m_pBuildThread.reset(new BuildThread(*this));
        rc = connect(this, SIGNAL(MessageReady()), this, SLOT(readLog()));
        GT_ASSERT(rc);

        // compileResultReady should be the last slot to be called.
        // So connect it first.
        rc = connect(m_pBoost->m_pBuildThread.get(), SIGNAL(finished()), this, SLOT(compileResultReady()));
        GT_ASSERT(rc);

        // Connect signal/slot between build thread and polling timer
        rc = connect(m_pBoost->m_pBuildThread.get(), SIGNAL(started()), m_pBoost->m_pStreamReadTimer.get(), SLOT(start()));
        GT_ASSERT(rc);
        rc = connect(m_pBoost->m_pBuildThread.get(), SIGNAL(finished()), m_pBoost->m_pStreamReadTimer.get(), SLOT(stop()));
        GT_ASSERT(rc);

        m_pBoost->m_pBuildThread->m_pExternalBackend = m_pBackend;
        m_pBoost->m_pBuildThread->m_pExternalSourceCodePath = &m_sourceCodePaths;

        // Set to single run to read log once after some delay.  The delay is for letting
        // caller of this constructor to set up connection to some GUI components after
        // this instance is created.  Should not call readLog() directly in the constructor
        // because it has no effect without frontend QT connections.
        // Let the timer handle it.
        m_pBoost->m_pStreamReadTimer->setSingleShot(true);
        m_pBoost->m_pStreamReadTimer->start();

        m_lastFileBuilt = "";
        m_isInitialized = true;
    }
}

// ---------------------------------------------------------------------------
// Name:        kaBackendManager::instance
// Description: Returns the single instance of this class.
//              (If it does not exist - create it)
// Author:      Gilad Yarnitzky
// Date:        30/7/2013
// ---------------------------------------------------------------------------
kaBackendManager& kaBackendManager::instance()
{
    if (m_psMySingleInstance == nullptr)
    {
        m_psMySingleInstance = new kaBackendManager;
        GT_ASSERT(m_psMySingleInstance);
    }

    return *m_psMySingleInstance;
}

//-----------------------------------------------------------------------------
kaBackendManager::~kaBackendManager()
{
    // looks like Linux doesn't like it now that we moved KA as DLL into CodeXL.let's release it on windows only.
    m_stopExecutionThread = true;
    if (m_executionThread)
    {
        m_executionThread->join();
    }
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    if (m_pBoost != nullptr && m_pBoost->m_pBuildThread != nullptr)
    {
        bool isCLGL = (CL_BUILD == m_pBoost->m_pBuildThread->m_buildType || GL_BUILD == m_pBoost->m_pBuildThread->m_buildType);

        if (m_pBackend != nullptr && isCLGL && false == m_firstTimeRun)
        {
            m_pBackend->theOpenCLBuilder()->ReleaseProgram();
        }
    }

#endif

    // No need for the delete instance since the TSingleton mechanism destroys the Backend object when the application quits
    // This duplication of destructions caused BUG417721
    // beKA::DeleteInstance();
    m_pBackend = nullptr;

}

//-----------------------------------------------------------------------------
void kaBackendManager::reset()
{
    s_forcedReset = true;

    if (true == m_pBoost->m_pBuildThread->isRunning())
    {
        m_pBoost->m_pBuildThread->ForceEnd();
    }

    m_firstTimeRun = true;

    s_forcedReset = false;
}

//-----------------------------------------------------------------------------
const set<string>& kaBackendManager::getDeviceNames() const
{
    return m_deviceNames;
}

//-----------------------------------------------------------------------------
bool kaBackendManager::isInBuild() const
{
    return m_isInBuild;
}

//-----------------------------------------------------------------------------
bool kaBackendManager::isExportable(const QStringList& selectedDeviceNames) const
{
    bool ret = false;

    bool isCLGL = (CL_BUILD == m_pBoost->m_pBuildThread->m_buildType || GL_BUILD == m_pBoost->m_pBuildThread->m_buildType);

    bool isCompileSuccess = (m_pBoost->m_pBuildThread->m_compileStatus == beKA::beStatus_SUCCESS);

    if (isCompileSuccess && isCLGL && false == m_firstTimeRun)
    {
        QStringList::const_iterator it;

        for (it = selectedDeviceNames.begin(); it != selectedDeviceNames.end(); ++it)
        {
            std::string sDevice(it->toLatin1().data());

            if (true == m_pBackend->theOpenCLBuilder()->CompileOK(sDevice))
            {
                ret = true;
                break;
            }
        }
    }

    return ret;
}

//-----------------------------------------------------------------------------
const std::string& kaBackendManager::getOpenCLVersionInfo()
{
    return  m_pBackend->theOpenCLBuilder()->GetOpenCLVersionInfo();
}

//-----------------------------------------------------------------------------
beKA::beStatus kaBackendManager::getASICsTreeList(beKA::DeviceTableKind kind,
                                                  bool includeCPU,
                                                  QStringList* pStrListOut)
{
    beKA::beStatus ret = beKA::beStatus_SUCCESS;

    if (pStrListOut != nullptr)
    {
        pStrListOut->clear();
    }

    if (m_isInitialized)
    {
        std::vector<GDT_GfxCardInfo> deviceTable;

        switch (kind)
        {
            case beKA::DeviceTableKind_OpenCL:
            {
                beProgramBuilderOpenCL* pOpenCLBuilder = m_pBackend->theOpenCLBuilder();

                if (nullptr != pOpenCLBuilder)
                {
                    ret = pOpenCLBuilder->GetDeviceTable(deviceTable);

                    if (ret == beKA::beStatus_SUCCESS)
                    {
                        ret = makeASICsStringList(deviceTable, includeCPU, &m_deviceNameMaketNameMapCL, nullptr, pStrListOut);
                    }
                }
            }
            break;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

            case beKA::DeviceTableKind_DX:
                ret = makeASICsStringList(deviceTable, includeCPU,
                                          &m_deviceNameMaketNameMapDX,
                                          &m_deviceNameIDMapDX,
                                          pStrListOut);
                break;
#endif

            case beKA::DeviceTableKind_OpenGL:
                ret = makeASICsStringList(deviceTable, includeCPU,
                                          &m_deviceNameMaketNameMapGL,
                                          nullptr,
                                          pStrListOut);
                break;

            default:
                assert(false);
                break;
        }
    }
    else
    {
        ret = beKA::beStatus_BACKEND_NOT_INITIALIZED;
    }

    return ret;
}

//-----------------------------------------------------------------------------
beKA::beStatus kaBackendManager::makeASICsStringList(const std::vector<GDT_GfxCardInfo>& deviceTable,
                                                     bool includeCPU,
                                                     std::multimap<QString, QString>* pNameNameMapOut,
                                                     std::multimap<QString, UINT>* pNameIdMapOut,
                                                     QStringList* pStrListOut)
{
    QStringList myList;
    beKA::beStatus ret = beKA::beStatus_SUCCESS;

    // Base layer
    myList.push_back(CheckableTreeItem::encodeHeaderData(QString("Devices")));

    GDT_HW_GENERATION gen = GDT_HW_GENERATION_NONE;
    GDT_HW_GENERATION currentGen = GDT_HW_GENERATION_NONE;
    bool isDeviceSelected = false;
    QString str;
    string calName;
    QString deviceName;
    bool isGCN = true;
    QStringList tempDeviceList;

    for (vector<GDT_GfxCardInfo>::const_iterator it = deviceTable.begin(); it != deviceTable.end(); ++it)
    {
        if (gen != it->m_generation)
        {
            isGCN = true;
            gen = it->m_generation;

            std::string sHwGenDisplayName;
            AMDTDeviceInfoUtils::Instance()->GetHardwareGenerationDisplayName(gen, sHwGenDisplayName);
            QStringList nonGCNVersions = QString(KA_STR_NonGCNVersions).split(" ");

            for (const auto& itr : nonGCNVersions)
            {
                if (QString(sHwGenDisplayName.c_str()).contains(itr, Qt::CaseInsensitive))
                {
                    isGCN = false;
                    break;
                }
            }

            switch (gen)
            {
                // It is very important that the depth passed to encodeData() matches the hierarchy.
                // All fields are set to checked by default.
                case GDT_HW_GENERATION_SOUTHERNISLAND:
                    str = QString(sHwGenDisplayName.c_str()) + QString(KA_STR_familyNameSICards);
                    myList.push_back(CheckableTreeItem::encodeData(str, isGCN, 1));
                    break;

                case GDT_HW_GENERATION_SEAISLAND:
                    str = QString(sHwGenDisplayName.c_str()) + QString(KA_STR_familyNameCICards);
                    myList.push_back(CheckableTreeItem::encodeData(str, isGCN, 1));
                    break;

                case GDT_HW_GENERATION_VOLCANICISLAND:
                    str = QString(sHwGenDisplayName.c_str()) + QString(KA_STR_familyNameVICards);
                    myList.push_back(CheckableTreeItem::encodeData(str, isGCN, 1));
                    break;

                default:
                    // You must have a new hardware generation.
                    // Add code...
                    assert(false);
                    break;
            }
        }

        if (calName != string(it->m_szCALName))
        {
            calName = string(it->m_szCALName);
            str = QString(calName.c_str());
            deviceName = str;

            if (currentGen != gen)
            {
                isDeviceSelected = true;
                currentGen = gen;
            }
            else
            {
                isDeviceSelected = false;
            }

            myList.push_back(CheckableTreeItem::encodeData(str, isDeviceSelected, 2));
        }

        ostringstream ss;
        ss << std::uppercase << std::hex << it->m_deviceID << "  " << it->m_revID;
        str = QString(ss.str().c_str());
        str.append("  ");
        str.append(it->m_szMarketingName);

        // only add the current device if it is a unique deviceID/MarketingName pair
        if (!tempDeviceList.contains(str))
        {
            tempDeviceList.append(str);

            if (nullptr != pNameIdMapOut)
            {
                std::pair<QString, UINT> prStrID(deviceName, it->m_deviceID);
                pNameIdMapOut->insert(prStrID);
            }

            myList.push_back(CheckableTreeItem::encodeData(str, isDeviceSelected, 3));

            if (nullptr != pNameNameMapOut)
            {
                std::pair<QString, QString> deviceNameMarketName(deviceName, str);
                pNameNameMapOut->insert(deviceNameMarketName);
            }
        }
    }

    if (true == includeCPU)
    {
        // Get the CPU device
        std::set<string> devices;
        ret = m_pBackend->theOpenCLBuilder()->GetDevices(devices);

        if (ret == beKA::beStatus_SUCCESS)
        {
            bool foundCPU = false;

            for (set<string>::const_iterator devIter = devices.begin(); devIter != devices.end(); ++devIter)
            {
                cl_device_type deviceType;

                if (beKA::beStatus_SUCCESS == m_pBackend->theOpenCLBuilder()->GetDeviceType(*devIter, deviceType) &&
                    CL_DEVICE_TYPE_CPU == deviceType)
                {
                    if (false == foundCPU)
                    {
                        foundCPU = true;
                        str = QString("CPU devices:");
                        myList.push_back(CheckableTreeItem::encodeData(str, true, 1));
                    }

                    str = QString(devIter->c_str());
                    myList.push_back(CheckableTreeItem::encodeData(str, true, 2));
                }
            }
        }
    }

    // Level 2 above is the name of ASCIs that will be send to the compiler.
    syncASICsTreeListWithPersistent(myList, pStrListOut);

    return ret;
}

//-----------------------------------------------------------------------------
void kaBackendManager::syncASICsTreeListWithPersistent(QStringList& source,
                                                       QStringList* pStrList)
{
    if (0 < pStrList->size())
    {
        QStringList::iterator it;
        QString str;
        QStringList::iterator found;

        for (it = source.begin(); it != source.end(); ++it)
        {
            // find string on pStrList
            found = std::find(pStrList->begin(), pStrList->end(), *it);

            if (pStrList->end() == found)
            {
                // If not found, change checked state and find again
                str = *it;
                CheckableTreeItem::toggleStr(str);

                found = std::find(pStrList->begin(), pStrList->end(), str);

                if (pStrList->end() != found)
                {
                    // If found, replace it with the new string.
                    *it = str;

                    // Erase the node on pStrList so we do not have to search on it again
                    pStrList->erase(found);
                }
            }
            else
            {
                // Erase the node on pStrList so we do not have to search on it again
                pStrList->erase(found);
            }
        }
    }

    // Replace pStrList with the new synchronized one.  Any new hardware
    // will be automatically added.
    *pStrList = source;
}

//-----------------------------------------------------------------------------
QString kaBackendManager::getDeviceMarketingNames(const QString& deviceName,
                                                  const std::multimap<QString, QString>& nameNameMap) const
{
    multimap<QString, QString>::const_iterator lowerBound;
    multimap<QString, QString>::const_iterator upperBound;
    multimap<QString, QString>::const_iterator mapIt;
    QString ret;
    QChar newLineChar('\n');

    lowerBound = nameNameMap.lower_bound(deviceName);
    upperBound = nameNameMap.upper_bound(deviceName);

    if (lowerBound != upperBound)
    {
        for (mapIt = lowerBound; mapIt != upperBound; ++mapIt)
        {
            ret.append(mapIt->second);
            ret.append(newLineChar);
        }

        // remove the last new line char
        ret.chop(1);
    }

    return ret;
}

//-----------------------------------------------------------------------------
void kaBackendManager::setUpForBuild(const QString& sourceCode, const std::string& fileName, int numDevices)
{
    // Set to multi run.
    m_pBoost->m_pStreamReadTimer->setSingleShot(false);

    gtASCIIString message;
    message.appendFormattedString(KA_STR_BUILD_STARTED, fileName.c_str(), numDevices);

    std::string buildMessage(message.asCharArray());
    backendMessageCallback(buildMessage);
    emit MessageReady();

    if (m_pBoost->m_pBuildThread->m_buildType != DX_FXC_BUILD)
    {
        *(m_pBoost->m_pSourceCode) = string(sourceCode.toLatin1().data());
        m_firstTimeRun = false;
    }
}

static void getFileContent(const std::string& fileFullPath, std::string& fileContent)
{
    std::ifstream file(fileFullPath);
    fileContent = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

//-----------------------------------------------------------------------------
void kaBackendManager::buildCLProgram(
    const QString& sourceCode,
    const set<string>& selectedDeviceNames,
    const QString& buildOptions,
    const QString& buildSourcePathAndName,
    bool analyze)
{
    kaSourceFile* pDataFile = KA_PROJECT_DATA_MGR_INSTANCE.dataFileByPath(acQStringToGTString(buildSourcePathAndName));

    if (m_pBoost != nullptr && pDataFile != nullptr)
    {
        m_pBoost->m_pBuildThread->m_buildType = CL_BUILD;
        m_pBoost->m_pBuildThread->m_bitness = m_buildArchitecture;
        m_pBoost->m_pBuildThread->m_kernelName = pDataFile->EntryPointFunction().asASCIICharArray();
        m_isInBuild = true;

        if (nullptr == m_pBoost->m_pCLOptions.get())
        {
            m_pBoost->m_pCLOptions.reset(new beProgramBuilderOpenCL::OpenCLOptions);
            m_pBoost->m_pCLOptions->m_SourceLanguage = beKA::SourceLanguage_OpenCL;
        }

        m_pBoost->m_pBuildThread->m_pExternalOptions = m_pBoost->m_pCLOptions.get();

        if (false == m_pBoost->m_pBuildThread->isRunning())
        {
            QString updatedBuildOptions = buildOptions;

            // Filter out the CPU devices, which are currently unsupported.
            beProgramBuilderOpenCL* pClBuilder = m_pBackend->theOpenCLBuilder();

            if (pClBuilder != nullptr)
            {
                m_pBoost->m_pCLOptions->m_SelectedDevices.clear();
                cl_device_type deviceType;
                beKA::beStatus rc = beStatus_Invalid;

                for (const std::string& selectedDevice : selectedDeviceNames)
                {
                    rc = pClBuilder->GetDeviceType(selectedDevice, deviceType);

                    if ((rc == beStatus_SUCCESS) && (deviceType != CL_DEVICE_TYPE_CPU))
                    {
                        m_pBoost->m_pCLOptions->m_SelectedDevices.insert(selectedDevice);
                    }
                }
            }

            UpdateOBuildOption(updatedBuildOptions);
            m_pBoost->m_pCLOptions->m_OpenCLCompileOptions.clear();
            m_pBoost->m_pCLOptions->m_OpenCLCompileOptions.push_back(string(updatedBuildOptions.toLatin1().data()));
            m_pBoost->m_pCLOptions->m_Analyze = analyze;
            m_sourceCodeFullPathName = acQStringToGTString(buildSourcePathAndName);
            std::string sSourceCodePath;
            gtWideStringToUtf8String(m_sourceCodeFullPathName.asCharArray(), sSourceCodePath);
            unsigned int sourceNameStart = sSourceCodePath.find_last_of("/\\");
            std::string fileName = sSourceCodePath.substr(sourceNameStart + 1, sSourceCodePath.length());
            sSourceCodePath = sSourceCodePath.substr(0, sourceNameStart);
            int numDevices = selectedDeviceNames.size();
            m_sourceCodePaths.clear();
            m_sourceCodePaths.push_back(sSourceCodePath);

            m_lastFileBuilt = fileName.c_str();

            // Set the source code for the builder thread.
            getFileContent(m_sourceCodeFullPathName.asASCIICharArray(), m_pBoost->m_pBuildThread->m_pExternalSourceCode);

            setUpForBuild(sourceCode, fileName, numDevices);

            emit buildStart(acQStringToGTString(sourceCode));

            m_pBoost->m_pBuildThread->start();
        }
    }
}

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

/* UNUSED FUNCTION
static void split(const std::string &s, char delim, std::vector<std::string> &elems)
{
elems.clear();
std::stringstream ss(s);
std::string item;
while (std::getline(ss, item, delim))
{
elems.push_back(item);
}
}

static bool ExtractDxCompilerDefines(const QString& userDefines,
std::vector<std::pair<std::string, std::string>>& extractedDefines)
{
bool ret = false;
extractedDefines.clear();
const char DELIMITER = ';';
const char EQUALS_SYMBOL = '=';
const std::string& definesStr = userDefines.toStdString();

std::vector<std::string> defines;
split(definesStr, DELIMITER, defines);

for (size_t i = 0; i < defines.size(); ++i)
{
const std::string& currDefinition = defines[i];

// HANDLE TRIMMING!

// Split the definition to key and value.
size_t equalsSymbolIndex = currDefinition.find(EQUALS_SYMBOL);
if (equalsSymbolIndex != std::string::npos)
{
// We have both key and value.
std::string key = currDefinition.substr(0, equalsSymbolIndex);
std::string value = currDefinition.substr(equalsSymbolIndex + 1);

// Add the key value definition to our container.
std::pair<std::string, std::string> pair(key, value);
extractedDefines.push_back(pair);
}
else
{
// We only have a single definition (not a key-value pair).
// Add the key value definition to our container.
std::pair<std::string, std::string> pair(currDefinition, " ");
extractedDefines.push_back(pair);
}
}

ret = !extractedDefines.empty();
return ret;
}

static bool ExtractDxCompilerIncludes(const QString& userIncludes,
std::vector<std::string>& extractedIncludes)
{
bool ret = false;
extractedIncludes.clear();
const char DELIMITER = ';';
const std::string& definesStr = userIncludes.toStdString();

if (!userIncludes.contains(DELIMITER))
{
// Edge case: only a single include directory, without a delimiter.
const QString& includeDir = userIncludes.trimmed();
if (!includeDir.isEmpty())
{
extractedIncludes.push_back(includeDir.toStdString());
}
}
else
{
std::vector<std::string> paths;
split(definesStr, DELIMITER, paths);

// Copy the paths.
if (!paths.empty())
{
std::copy(paths.begin(), paths.end(), std::back_inserter(extractedIncludes));
}
}
ret = !extractedIncludes.empty();
return ret;
}


// Auxiliary function to read an input file.
// To be relocated to a utilities class.
static bool ReadProgramFile(const std::string& inputFile, std::string& programSource)
{
bool ret = false;
ifstream input;
input.open(inputFile.c_str(), ios::ate | ios::binary);

if (input)
{
ifstream::pos_type fileSize = 0;
fileSize = input.tellg();
if (fileSize != static_cast<ifstream::pos_type>(0))
{
input.seekg(0, ios::beg);
programSource.resize(size_t(fileSize));
input.read(&programSource[0], fileSize);
ret = true;
}
input.close();
}
return ret;
}
*/
static void DxPrintIgnoredOptions()
{
    // If required, print the message about the ignored build option.
    const QStringList& buildOptionsToIgnore = KA_PROJECT_DATA_MGR_INSTANCE.ShaderAvoidableBuildOptions();

    if (!buildOptionsToIgnore.isEmpty())
    {
        QString outputStr = KA_STR_DX_OutputMsg_IgnoringStrs;
        bool isFrist = true;

        foreach (QString str, buildOptionsToIgnore)
        {
            if (!isFrist)
            {
                outputStr.append(", ");
            }

            outputStr.append(str);

            isFrist = false;
        }

        outputStr.append(".\n");
        afApplicationCommands::instance()->AddStringToInformationView(outputStr);
    }
}



void kaBackendManager::AddDxBinSearchPath(const gtString& path)
{
    Backend::AddDxSearchDir(path.asASCIICharArray());
}

#endif

//-----------------------------------------------------------------------------
void kaBackendManager::UpdateOBuildOption(QString& optionsStr)
{
    QStringList optionsParam = optionsStr.split(" ");
    int indexOfO = optionsParam.indexOf("-o");

    if (-1 != indexOfO)
    {
        afGlobalVariablesManager& theGlobalVariablesManager = afGlobalVariablesManager::instance();
        osFilePath currentLogFilesPath = theGlobalVariablesManager.logFilesDirectoryPath();
        // make sure there is a single trailing path separator for the temp dir
        gtString currentLogFilesPathGtStr = currentLogFilesPath.asString();
        currentLogFilesPathGtStr.removeTrailing(osFilePath::osPathSeparator);
        currentLogFilesPathGtStr.append(osFilePath::osPathSeparator);
        QString currentLogFilesPathStr = acGTStringToQString(currentLogFilesPathGtStr);

        osDirectory currentLogDir(currentLogFilesPath);

        bool isTempDirWrittable = currentLogDir.isWriteAccessible();

        // need to use tmp dir
        int validDirError = 0;
        // if tempdir is used it means the options need to be rebuild
        osDirectory optionDirectory;

        // check if next param is directory or it exists
        if (indexOfO != optionsParam.count() - 1)
        {
            gtString pathStr = acQStringToGTString(optionsParam[indexOfO + 1]);

            // Check if the dir name has parenthesis
            if (pathStr.findFirstOf(L"\"") != -1)
            {
                // find the string with the ending parenthesis and join the strings to the current one
                QString currentString = optionsParam[indexOfO + 1];

                while ((indexOfO + 2 < optionsParam.count()) && (optionsParam[indexOfO + 2].lastIndexOf("\"") == optionsParam[indexOfO + 2].length() - 1))
                {
                    currentString = currentString + " " + optionsParam[indexOfO + 2];
                    optionsParam.removeAt(indexOfO + 2);
                }

                optionsParam[indexOfO + 1] = currentString;
                pathStr = acQStringToGTString(currentString);
                pathStr.replace(L"\"", L"");
                // remove the path in parenthesis (should be removed when an EPR is resolved)
                optionsParam.removeAt(indexOfO + 1);
                optionsParam.removeAt(indexOfO);
                validDirError = KA_BUILD_O_PARENTHESIS_ERROR;
            }

            if (0 == validDirError)
            {
                optionDirectory.setDirectoryFullPathFromString(pathStr);
                // check if there is a directory provided after the -o

                if (optionDirectory.exists())
                {
                    // make sure there is a single trailing path separator for the supplied work dir
                    pathStr.removeTrailing(osFilePath::osPathSeparator);
                    pathStr.append(osFilePath::osPathSeparator);

                    // Currently spaces are not allowed in path and it is removed but this code is
                    // working ready for the case when the KA_BUILD_O_PARENTHESIS_ERROR is removed when the EPR is resolved
                    if (pathStr.find(L" ") != -1)
                    {
                        gtString newPath;
                        newPath.appendFormattedString(L"\"%ls\"", pathStr.asCharArray());
                        optionsParam[indexOfO + 1] = acGTStringToQString(newPath);
                    }
                    else
                    {
                        optionsParam[indexOfO + 1] = acGTStringToQString(pathStr);
                    }

                    // check that we have permission to write to that dir, if not replace it with the temp dir
                    if (!optionDirectory.isWriteAccessible())
                    {
                        validDirError = KA_BUILD_O_WRITABLE_DIR_ERROR;
                        // remove the -o dir from the options
                        optionsParam.removeAt(indexOfO + 1);
                        optionsParam.removeAt(indexOfO);
                    }
                }
                else if (isTempDirWrittable)
                {
                    optionsParam.insert(indexOfO + 1, currentLogFilesPathStr);
                }
                else
                {
                    validDirError = KA_BUILD_O_WRITABLE_DIR_ERROR;
                    // remove the -o from the options
                    optionsParam.removeAt(indexOfO);
                }
            }
        }
        else
        {
            // the -o option is the last special case:
            // assume the next param was not a dir then test the temp dir and use it
            if (isTempDirWrittable)
            {
                // insert the temp dir
                optionsParam.push_back(currentLogFilesPathStr);
            }
            else
            {
                validDirError = KA_BUILD_O_WRITABLE_DIR_ERROR;
                // remove the -o from the options
                optionsParam.removeAt(indexOfO);
            }
        }

        // if there is no valid dir notify the user that the option was removed:
        switch (validDirError)
        {
            case KA_BUILD_O_PARENTHESIS_ERROR:  addStringToInformationView(KA_STR_buildOptionRemovedNoParenthesis); break;

            case KA_BUILD_O_WRITABLE_DIR_ERROR: addStringToInformationView(KA_STR_buildOptionRemovedNoWritableDir); break;

            default: break;
        }
    }

    // Patch for problem in OpenCL. until it is fixed we block this option in CodeXL (BUG451367)
    int indexOfCreateLib = optionsParam.indexOf("-create-library");

    if (-1 != indexOfCreateLib)
    {
        std::set<std::string>::iterator iter = m_pBoost->m_pCLOptions->m_SelectedDevices.begin();

        for (; iter != m_pBoost->m_pCLOptions->m_SelectedDevices.end(); iter++)
        {
            std::string sDeviceName(*iter);
            optionsParam.removeAt(indexOfCreateLib);

            if (sDeviceName.find("/") != std::string::npos)
            {
                // we remove the -create-library option and let the user know
                optionsParam.removeAt(indexOfCreateLib);
                std::string sMsg(KA_STR_RemovedOptionCreateLibrary);
                backendMessageCallback(sMsg);
                emit MessageReady();
            }
        }
    }

    // Prepend each OpenCL compiler option with the "--OpenCLoption " prefix
    for (int i = 0; i < optionsParam.size(); ++i)
    {
        QString& param = optionsParam[i];

        // If this is an option (begins with hyphen) and it isnt -I or -D
        if (param.size() > 1 && param[0] == '-' && param[1] != 'D' && param[1] != 'I')
        {
            // Prepend with the "--OpenCLoption " prefix
            QString modifiedParam(KA_STR_CLIPrefixForCompilerOption);

            // For -x option, need to wrap this param and the next in quotes because they are one, e.g. -x clc++ should become "-x clc++"
            if ((param[1] == 'x' || param == "-save-temp" || param == "-fuse-native") && i + 1 < optionsParam.size())
            {
                modifiedParam.append('\"');
                optionsParam[i + 1].append('\"');
            }

            modifiedParam.append(param);

            // Replace this param in the list
            optionsParam.replace(i, modifiedParam);
        }
    }

    // since there are almost always a change to the supplied dir we always rebuild the string of options:
    optionsStr = optionsParam.join(' ');
}

beKA::beStatus kaBackendManager::isBackendInitialized() const
{
    beKA::beStatus ret = beKA::beStatus_SUCCESS;

    if (!m_pBackend->theOpenCLBuilder()->IsInitialized())
    {
        ret = beKA::beStatus_OpenCL_MODULE_NOT_LOADED;
    }

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    else if (!m_pBackend->theOpenDXBuilder()->IsInitialized())
    {
        ret = beKA::beStatus_D3DCompile_MODULE_NOT_LOADED;
    }

#endif
    return ret;
}

//-----------------------------------------------------------------------------
void kaBackendManager::getBinary(const gtString& deviceName, const gtString& binaryFileOnDisk, bool source, bool IL,
                                 bool debugInfo, bool llvmIR,
                                 bool isa, std::vector<char>& outputBuffer)
{
    beKA::BinaryOptions options;
    string suppression;

    if (false == source)
    {
        suppression = ".source";
        options.m_SuppressSection.push_back(suppression);
    }

    if (false == IL)
    {
        suppression = ".amdil";
        options.m_SuppressSection.push_back(suppression);
        suppression = ".debugil";
        options.m_SuppressSection.push_back(suppression);
    }

    if (false == debugInfo)
    {
        suppression = ".debugil";
        options.m_SuppressSection.push_back(suppression);
        suppression = ".debug_info";
        options.m_SuppressSection.push_back(suppression);
        suppression = ".debug_abbrev";
        options.m_SuppressSection.push_back(suppression);
        suppression = ".debug_line";
        options.m_SuppressSection.push_back(suppression);
        suppression = ".debug_pubnames";
        options.m_SuppressSection.push_back(suppression);
        suppression = ".debug_pubtypes";
        options.m_SuppressSection.push_back(suppression);
        suppression = ".debug_loc";
        options.m_SuppressSection.push_back(suppression);
        suppression = ".debug_str";
        options.m_SuppressSection.push_back(suppression);
    }

    if (false == llvmIR)
    {
        suppression = ".llvmir";
        options.m_SuppressSection.push_back(suppression);
    }

    if (false == isa)
    {
        suppression = ".text";
        options.m_SuppressSection.push_back(suppression);
    }

    osFilePath path;
    path.setFullPathFromString(binaryFileOnDisk);

    if (path.exists())
    {
        m_pBackend->theOpenCLBuilder()->GetBinaryFromFile(path.asString().asUTF8CharArray(), options, outputBuffer);
    }
    else
    {
        // The kernel was not built yet
        gtString msg;
        msg.appendFormattedString(KA_STR_NoBinariesForDevice, deviceName.asCharArray());
        backendMessageCallback(msg.asASCIICharArray());
        emit MessageReady();
    }
}

//-----------------------------------------------------------------------------
void  kaBackendManager::backendMessageCallback(const std::string& message)
{
    g_backendMessageMutex.lock();
    g_backendMessages.push(message);
    g_backendMessageMutex.unlock();
}

//-----------------------------------------------------------------------------
bool kaBackendManager::readLog()
{
    bool ret = false;

    g_backendMessageMutex.lock();

    while (false == g_backendMessages.empty())
    {
        ret = true;
        std::string str = g_backendMessages.front();
        g_backendMessages.pop();
        addStringToInformationView(str);
    }

    g_backendMessageMutex.unlock();

    return ret;
}

//-----------------------------------------------------------------------------
void kaBackendManager::compileResultReady()
{
    if (m_buildType == BuiltProgramKind_OpenCL ||
        m_buildType == BuiltProgramKind_DX ||
        m_buildType == BuiltProgramKind_OpenGL ||
        m_buildType == BuiltProgramKind_Vulkan)
    {
        if (beKA::beStatus_CL_BUILD_PROGRAM_ICE == m_pBoost->m_pBuildThread->m_compileStatus)
        {
            QMessageBox::critical(nullptr, "Error", "Fatal compiler error encountered. KernelAnalyzer will now close.");
            exit(0);
        }

        // Read the log to clear any pending messages.
        emit MessageReady();

        m_isInBuild = false;

        // signal that all the build is done (empty source name):
        emit buildComplete(GetLastBuildProgramName());

        m_buildCompleted = true;
    }
}

//-----------------------------------------------------------------------------
kaBackendManager::BuildThread::BuildThread(kaBackendManager& owner) : m_compileStatus(beKA::beStatus_Invalid), m_shouldBeCanceled(false), m_owner(owner)
{

}

void kaBackendManager::BuildThread::ForceEnd()
{
    if (nullptr != m_pExternalBackend && nullptr != m_pExternalBackend->theOpenCLBuilder())
    {
        m_pExternalBackend->theOpenCLBuilder()->ForceEnd();
    }
}

bool kaBackendManager::BuildThread::BuildOpenclSourceFile(kaSourceFile* pCurrentFile,
                                                          const gtString& isaFileName,
                                                          const gtString& binaryFile,
                                                          const gtString& ilFileName,
                                                          const gtString& analysisFileName,
                                                          const std::string& sourceCodeFullPathName,
                                                          int& numOfSuccessfulBuilds)
{
    bool result = true;

    if (pCurrentFile != nullptr)
    {

        beProgramBuilderOpenCL::OpenCLOptions* pOclOptions =
            static_cast<beProgramBuilderOpenCL::OpenCLOptions*>(m_pExternalOptions);

        GT_IF_WITH_ASSERT(pOclOptions != nullptr)
        {
            // Compile the code.
            std::string cliOutput;

            // The index of the current build.
            int currBuildNumber = 1;

            // Reset the counter.
            numOfSuccessfulBuilds = 0;
            const auto& devices = pOclOptions->m_SelectedDevices;
            int numOfBuildsOverall = devices.size();

            gtString sourcePathAsGtStr;
            sourcePathAsGtStr << sourceCodeFullPathName.c_str();

            if (!m_shouldBeCanceled)
            {
                // Print build prologue for the current source file.
                m_owner.PrintBuildPrologue(sourcePathAsGtStr, numOfBuildsOverall, true);
                // Add build options if they exist
                std::string buildOptions;

                for (size_t i = 0; i < pOclOptions->m_OpenCLCompileOptions.size(); ++i)
                {
                    if (i > 0)
                    {
                        buildOptions += " ";
                    }

                    buildOptions += pOclOptions->m_OpenCLCompileOptions[i];
                }

                int numberOfBuildAttempts = 0;

                for (const std::string& device : devices)
                {
                    if (!m_shouldBeCanceled)
                    {
                        // Notify the user that the build started for the current device.
                        gtString fileName;
                        pCurrentFile->filePath().getFileName(fileName);
                        std::stringstream buildMsg;
                        buildMsg << std::endl << currBuildNumber++ << "> " << fileName.asASCIICharArray() << " for " << device << ": ";
                        backendMessageCallback(buildMsg.str());
                        m_owner.triggerMessageReady();

                        // Launch the session for that specific device.
                        cliOutput.clear();
                        LaunchOpenCLSessionForDevice(m_bitness, isaFileName.asASCIICharArray(), ilFileName.asASCIICharArray(),
                                                     analysisFileName.asASCIICharArray(), binaryFile.asASCIICharArray(), device, sourceCodeFullPathName, buildOptions, m_shouldBeCanceled, cliOutput);

                        const size_t MIN_OUTPUT_LEN = 3;

                        if (!(cliOutput.size() < MIN_OUTPUT_LEN && m_shouldBeCanceled))
                        {
                            ++numberOfBuildAttempts;
                        }

                        if (cliOutput.find(KA_CLI_STR_STATUS_SUCCESS) != string::npos)
                        {
                            // We have another successful build.
                            numOfSuccessfulBuilds++;

                            // Store the name of the binary with the CL file's associated data
                            pCurrentFile->buildFiles().append(acGTStringToQString(binaryFile));
                        }


                        // Inform the user.
                        backendMessageCallback(cliOutput);
                        m_owner.triggerMessageReady();
                    }
                }//for all devices

                AggregateOpenCLStatistics(pCurrentFile, devices, analysisFileName);

                PrintBuildEpilogue(numOfBuildsOverall, numOfSuccessfulBuilds, numberOfBuildAttempts);
                result = (numOfBuildsOverall - numOfSuccessfulBuilds) == 0;
            }
        }
    }

    return result;
}



void kaBackendManager::BuildThread::AggregateOpenCLStatistics(kaSourceFile* pCurrentFile, const std::set<std::string>& devices, const gtString& dummyStatisticsFileName) const
{
    gtList<osFilePath> filePathsList;
    osDirectory outputDir = osFilePath(dummyStatisticsFileName).clearFileExtension().clearFileName();//base folder of statistics files

    GT_IF_WITH_ASSERT(outputDir.getContainedFilePaths(L"*." KA_STR_kernelViewExtension, osDirectory::SORT_BY_NAME_ASCENDING, filePathsList))//get all new statistics file
    {
        //the map key is the kernel name
        gtMap<gtString, gtVector<gtString>> currentFileStatistics;
        gtString currentFileName;
        pCurrentFile->filePath().getFileName(currentFileName);

        //find statistics files for this current file and for all devices
        for (osFilePath& filePath : filePathsList)
        {
            gtString statsFileName;
            filePath.getFileName(statsFileName);
            //use files that are only for devices that were built for this current program:
            auto itt = std::find_if(devices.begin(), devices.end(), [&](const std::string & deviceName) -> bool
            {
                return statsFileName.startsWith(gtString().fromASCIIString(deviceName.c_str()));
            });

            if (statsFileName.endsWith(currentFileName) && itt != devices.end())
            {
                //cut off the kernel name , by removing device and file name
                size_t startIx = itt->size() + 1,
                       endIx = statsFileName.length() - currentFileName.length() - 2;
                GT_ASSERT(startIx < endIx);
                gtString kernelName;
                statsFileName.getSubString(startIx, endIx, kernelName);
                //add stats file with kernel name as key for this current source file
                currentFileStatistics[kernelName].push_back(filePath.asString());
            }
        }

        gtString statsFileName;
        osFilePath statsFilePath(dummyStatisticsFileName);
        statsFilePath.getFileName(statsFileName);
        const gtString statsPrfx = KA_STR_buildMainStatisticsFileName L"_";

        // Collect the statistics results into a single file, for all kernels for this current file
        for (const auto& itr : currentFileStatistics)
        {
            //current stats filename is KA_STR_buildMainStatisticsFileName + kernel name + current file name
            gtString currentSatsFName = statsPrfx;
            currentSatsFName.append(itr.first).append(L"_").append(statsFileName);
            statsFilePath.setFileName(currentSatsFName);
            AggregateAndRemoveStatisticsFiles(statsFilePath.asString(), itr.second);
        }

        // Handle the statistics.
        const QString statisticsMetaFileName = acGTStringToQString(GenerateMetaFileName(m_kernelName, "", boftStatistics));
        pCurrentFile->buildFiles().append(statisticsMetaFileName);
    }
}

void kaBackendManager::BuildThread::BuildGlslShader(kaSourceFile* pCurrentFile,
                                                    const gtString& isaFileName, const gtString& statisticsFileName, const std::string& sourceCodeFullPathName, int& numOfSuccessfulBuilds)
{
    // Statistics are not yet supported for OpenGL in the front-end.
    GT_UNREFERENCED_PARAMETER(statisticsFileName);

    if (pCurrentFile != nullptr)
    {
        std::string cliOutput;
        gtVector<gtString> statisticsFiles;

        // The index of the current build.
        int currBuildNumber = 1;
        bool isAnyDeviceBuildSuccessful = false;

        // Reset the counter.
        numOfSuccessfulBuilds = 0;

        for (const std::string& device : m_deviceNames)
        {
            if (!m_shouldBeCanceled)
            {
                // Prepare the dummy file names that contain the meta-data.
                // This is being done just to align with the current tree handler mechanism,
                // which is sub-optimal and will be revised in the next development time frame.
                // This is temporary code.
                const QString isaMetaFileName = acGTStringToQString(GenerateMetaFileName(m_kernelName, device, boftISA));

                // Generate a file name to hold statistics for this device. The content of this file is copied to the aggregate statistics file
                // after this devices loop is done, and the individual device statistics files are deleted.
                gtString deviceSpecificStatisticsFile;
                GeneralFileNameToDeviceSpecificFileName(statisticsFileName, device, deviceSpecificStatisticsFile);
                statisticsFiles.push_back(deviceSpecificStatisticsFile);

                // Notify the user that the build started for the current device.
                std::stringstream buildMsg;
                buildMsg << std::endl << currBuildNumber++ << "> " << device << ": ";
                backendMessageCallback(buildMsg.str());
                m_owner.triggerMessageReady();

                // Launch the session for that specific device.
                cliOutput.clear();
                LaunchOpenGLSessionForDevice(m_bitness, m_glShaderType,
                                             isaFileName.asASCIICharArray(), "", deviceSpecificStatisticsFile.asASCIICharArray(), device, sourceCodeFullPathName, m_shouldBeCanceled, cliOutput);

                // Inform the user.
                backendMessageCallback(cliOutput);
                m_owner.triggerMessageReady();

                if (cliOutput.find(KA_CLI_STR_STATUS_SUCCESS) != string::npos)
                {
                    // We have another successful build.
                    numOfSuccessfulBuilds++;

                    // Add the required meta files (to align with the existing tree handler mechanism).
                    pCurrentFile->buildFiles().append(isaMetaFileName);
                    isAnyDeviceBuildSuccessful = true;
                }
            }
        }

        if (isAnyDeviceBuildSuccessful)
        {
            // Collect the statistics results into a single file
            AggregateAndRemoveStatisticsFiles(statisticsFileName, statisticsFiles);

            // Handle the statistics.
            const QString statisticsMetaFileName = acGTStringToQString(GenerateMetaFileName(m_kernelName, "", boftStatistics));
            pCurrentFile->buildFiles().append(statisticsMetaFileName);
        }
    }
}

bool kaBackendManager::BuildThread::BuildRenderProgram(const gtString& buildOutputDir)
{

    // Prepare the arguments for the CLI launcher.
    const gtString isaFileName = GenerateRenderBuildOutputFileName(boftISA, buildOutputDir);
    const gtString ilFileName = GenerateRenderBuildOutputFileName(boftIL, buildOutputDir);
    const gtString statisticsFileName = GenerateRenderBuildOutputFileName(boftStatistics, buildOutputDir);
    const gtString binaryFileName = GenerateRenderBuildOutputFileName(boftBinary, buildOutputDir);

    // Prepare the source code file's full path.
    std::string sourceCodeFullPathName;


    // Reset the status to non-successful.
    m_compileStatus = beStatus_Invalid;

    // Counter for the number of successful builds overall.
    // Will be used to summarize the build results for the user.
    int numOfSuccessfulBuilds = 0;

    std::string cliOutput;

    gtMap<gtString, gtVector<gtString>> statisticsFilesGroups;  //stats per stage for all devices
    InitKaStageStatisticsGroups(statisticsFilesGroups);

    // The index of the current build.
    int currBuildNumber = 1;
    bool isAnyDeviceBuildSuccessful = false;

    // Generate a base file name to hold statistics for this devices.
    osFilePath statsFilePath;
    statsFilePath.setFullPathFromString(statisticsFileName);

    // Fix the base file path.
    gtString fixedFileName;
    fixedFileName << KA_STR_buildMainStatisticsFileName;
    statsFilePath.setFileName(fixedFileName);
    gtString deviceStatisticsBaseFileName = statsFilePath.asString();

    int numOfBuildsOverall = m_deviceNames.size();

    int numOfBuildAttempts = 0;

    for (const std::string& device : m_deviceNames)
    {
        if (!m_shouldBeCanceled)
        {
            // Prepare the dummy file names that contain the meta-data.
            // This is being done just to align with the current tree handler mechanism,
            // which is sub-optimal and will be revised in the next development time frame.
            // This is temporary code.
            const QString isaMetaFileName = acGTStringToQString(GenerateMetaFileName(m_kernelName, device, boftISA));

            UpdateStatisticsFileGroups(statisticsFilesGroups, statisticsFileName, device);

            // Notify the user that the build started for the current device.
            std::stringstream buildMsg;
            buildMsg << std::endl << currBuildNumber++ << "> " << device << ": ";
            backendMessageCallback(buildMsg.str());
            m_owner.triggerMessageReady();

            // Launch the session for that specific device.
            cliOutput.clear();
            LaunchRenderSessionForDevice(m_buildType, m_bitness, m_shadersPaths,
                                         isaFileName.asASCIICharArray(), ilFileName.asASCIICharArray(),
                                         deviceStatisticsBaseFileName.asASCIICharArray(), binaryFileName.asASCIICharArray(),
                                         device, m_shouldBeCanceled, cliOutput);

            // Inform the user.
            backendMessageCallback(cliOutput);
            m_owner.triggerMessageReady();

            if (cliOutput.find(KA_CLI_STR_STATUS_SUCCESS) != string::npos)
            {
                // We have another successful build.
                numOfSuccessfulBuilds++;

                // Add the required meta files (to align with the existing tree handler mechanism).
                isAnyDeviceBuildSuccessful = true;
            }

            const size_t MIN_OUTPUT_LEN = 3;

            if (!(cliOutput.size() < MIN_OUTPUT_LEN && m_shouldBeCanceled))
            {
                ++numOfBuildAttempts;
            }
        }
    }

    //  for (const gtVector<gtString>& group : statisticsFilesGroups)

    if (isAnyDeviceBuildSuccessful)
    {
        for (auto& itr : statisticsFilesGroups)
        {
            gtString statsFileName = buildOutputDir;
            statsFileName.append(LR"(\)").append(KA_STR_buildMainStatisticsFileName).append(L"_").append(itr.first).append(L".").append(KA_STR_kernelViewExtension);
            // Collect the statistics results into a single file
            AggregateAndRemoveStatisticsFiles(statsFileName, itr.second);
        }

        // Handle the statistics.
        const QString statisticsMetaFileName = acGTStringToQString(GenerateMetaFileName(m_kernelName, "", boftStatistics));
    }

    // Print the build epilogue.
    PrintBuildEpilogue(numOfBuildsOverall, numOfSuccessfulBuilds, numOfBuildAttempts);
    bool result = m_shouldBeCanceled == false && (numOfBuildsOverall - numOfSuccessfulBuilds) == 0;
    return result;
}
void kaBackendManager::BuildThread::PrintBuildEpilogue(int numOfBuildsOverall, int numOfSuccessfulBuilds, const int devicesBuiltCount)
{
    gtASCIIString message;

    if (m_shouldBeCanceled)
    {
        const int numOfSkippedDevices = numOfBuildsOverall - devicesBuiltCount;

        if (numOfSkippedDevices == numOfBuildsOverall)
        {
            message = KA_STR_BUILD_CANCELLED_BY_USER_NO_SKIPPED;
        }
        else
        {
            message.append(KA_STR_BUILD_CANCELLED_BY_USER_PREFIX);
            message.append(gtASCIIString(to_string(numOfSkippedDevices).c_str()));
            message.append(KA_STR_BUILD_CANCELLED_BY_USER_SUFFIX);
        }
    }
    else
    {
        // Notify the user.
        message.appendFormattedString(KA_STR_BUILD_COMPLETED_PREFIX, numOfBuildsOverall);
        const int failedCount = numOfBuildsOverall - numOfSuccessfulBuilds;
        message.appendFormattedString("%d succeeded, %d failed", numOfSuccessfulBuilds, failedCount);
        message.appendFormattedString("%s", KA_STR_BUILD_COMPLETED_SUFFIX);
    }

    // Print the output message.
    kaBackendManager::backendMessageCallback(message.asCharArray());
    m_owner.triggerMessageReady();
}

gtString kaBackendManager::BuildThread::GetBuildOutputDir() const
{
    // Get the target directory.
    osDirectory output32Dir, output64Dir;
    const bool isProgram32Bit = KA_PROJECT_DATA_MGR_INSTANCE.GetBuildArchitecture() == kaBuildArch32_bit;
    m_pCurrentProgram->GetAndCreateOutputDirectories(output32Dir, output64Dir, isProgram32Bit, !isProgram32Bit);
    osDirectory targetDirectoryForOutput = (m_bitness == kaBuildArch32_bit) ? output32Dir : output64Dir;

    // Create the output directory if it does not exist.
    bool isOutputDirCreate = targetDirectoryForOutput.create();
    GT_ASSERT(isOutputDirCreate);

    // The output directory.
    osFilePath outputPath = targetDirectoryForOutput.directoryPath();


    gtString buildOutputDir = targetDirectoryForOutput.directoryPath().asString(true);
    return buildOutputDir;
}

void kaBackendManager::BuildThread::UpdateStatisticsFileGroups(gtMap<gtString, gtVector<gtString>>& statisticsFilesGroups, const gtString& statisticsFileName, const std::string& device) const
{
    for (auto& itr : statisticsFilesGroups)
    {
        osFilePath statsFilePath;
        statsFilePath.setFullPathFromString(statisticsFileName);
        gtString fixedFileName;
        fixedFileName.append(gtString().fromASCIIString(device.c_str())).append(L"_").append(itr.first).append(L"_").append(KA_STR_buildMainStatisticsFileName);
        statsFilePath.setFileName(fixedFileName);
        statsFilePath.setFileExtension(KA_STR_kernelViewExtension);

        itr.second.push_back(statsFilePath.asString());
    }
}

void kaBackendManager::BuildThread::InitKaStageStatisticsGroups(gtMap<gtString, gtVector<gtString>>& statisticsFilesGroups) const
{
    if (m_shadersPaths.m_vertexShader.isEmpty() == false)
    {
        statisticsFilesGroups[gtString().fromASCIIString(KA_CLI_STR_VERTEX_ABBREVIATION)] = gtVector<gtString>();
    }

    if (m_shadersPaths.m_tessControlShader.isEmpty() == false)
    {
        statisticsFilesGroups[gtString().fromASCIIString(KA_CLI_STR_TESS_CTRL_ABBREVIATION)] = gtVector<gtString>();
    }

    if (m_shadersPaths.m_tessEvaluationShader.isEmpty() == false)
    {
        statisticsFilesGroups[gtString().fromASCIIString(KA_CLI_STR_TESS_EVAL_ABBREVIATION)] = gtVector<gtString>();
    }

    if (m_shadersPaths.m_geometryShader.isEmpty() == false)
    {
        statisticsFilesGroups[gtString().fromASCIIString(KA_CLI_STR_GEOMETRY_ABBREVIATION)] = gtVector<gtString>();
    }

    if (m_shadersPaths.m_fragmentShader.isEmpty() == false)
    {
        statisticsFilesGroups[gtString().fromASCIIString(KA_CLI_STR_FRAGMENT_ABBREVIATION)] = gtVector<gtString>();
    }

    if (m_shadersPaths.m_computeShader.isEmpty() == false)
    {
        statisticsFilesGroups[gtString().fromASCIIString(KA_CLI_STR_COMPUTE_ABBREVIATION)] = gtVector<gtString>();
    }
}

#if AMDT_BUILD_TARGET==AMDT_WINDOWS_OS

bool kaBackendManager::BuildThread::BuildDxShader(kaSourceFile* pCurrentFile,
                                                  const gtString& isaFileName, const gtString& dxAsmFileName, const gtString& binFileName, const gtString& statisticsFileName,
                                                  const std::string& sourceCodeFullPathName,
                                                  int& numOfSuccessfulBuilds, const gtString& entryPoint, const gtString& profile, bool isIntrinsicsEnabled)
{
    bool result = false;

    if (pCurrentFile != nullptr && !m_shouldBeCanceled)
    {
        std::string cliOutput;
        gtVector<gtString> statisticsFiles;

        // The index of the current build.
        int currBuildNumber = 1;
        int numOfBuildsOverall = m_deviceNames.size();
        bool isAnyDeviceBuildSuccessful = false;

        // Reset the counter.
        numOfSuccessfulBuilds = 0;

        gtString sourcePathAsGtStr;
        sourcePathAsGtStr << sourceCodeFullPathName.c_str();

        if (!m_shouldBeCanceled)
        {
            // Print build prologue for the current source file.
            m_owner.PrintBuildPrologue(sourcePathAsGtStr, numOfBuildsOverall, true);

            int numOfBuildAttempts = 0;

            for (const std::string& device : m_deviceNames)
            {
                if (!m_shouldBeCanceled)
                {
                    // Generate a file name to hold statistics for this device. The content of this file is copied to the aggregate statistics file
                    // after this devices loop is done, and the individual device statistics files are deleted.
                    osFilePath currentStatsFilePath = GenerateDXStatsFName(pCurrentFile, statisticsFileName, device);


                    // Notify the user that the build started for the current device.
                    std::stringstream buildMsg;
                    buildMsg << std::endl << currBuildNumber++ << "> " << device << ": ";
                    backendMessageCallback(buildMsg.str());
                    m_owner.triggerMessageReady();

                    // Launch the session for that specific device.
                    cliOutput.clear();

                    // Patch up the DX ASM output file to contain the device name.
                    osFilePath dxAsmFilePath(dxAsmFileName);
                    gtString originalDxAsmFileName;
                    dxAsmFilePath.getFileName(originalDxAsmFileName);
                    gtString fixedDxAsmFileName;
                    fixedDxAsmFileName.fromASCIIString(device.c_str());
                    fixedDxAsmFileName << L"_" << originalDxAsmFileName;
                    dxAsmFilePath.setFileName(fixedDxAsmFileName);

                    // Launch the CLI.
                    LaunchDXSessionForDevice(m_bitness, profile.asASCIICharArray(),
                                             entryPoint.asASCIICharArray(),
                                             m_dxBuildOptions.m_buildOptions.toStdString(),
                                             m_dxBuildOptions, isaFileName.asASCIICharArray(), binFileName.asASCIICharArray(), "",
                                             currentStatsFilePath.asString().asASCIICharArray(), dxAsmFilePath.asString().asASCIICharArray(),
                                             device, sourceCodeFullPathName, isIntrinsicsEnabled, m_shouldBeCanceled, cliOutput);

                    // Rename the DX ASM file to align with the front-end's expected file name.
                    osFilePath fixedDxAsmFilePath(dxAsmFilePath);
                    dxAsmFilePath.getFileName(originalDxAsmFileName);
                    fixedDxAsmFileName.makeEmpty();
                    fixedDxAsmFileName << entryPoint;
                    fixedDxAsmFileName << L"_" << originalDxAsmFileName;
                    fixedDxAsmFilePath.setFileName(fixedDxAsmFileName);
                    gtString deviceName;
                    deviceName.fromASCIIString(device.c_str());
                    deviceName << L"_";
                    fixedDxAsmFileName.replace(deviceName, L"");
                    fixedDxAsmFileName.prepend(deviceName);
                    dxAsmFilePath.setFileName(fixedDxAsmFileName);
                    fixedDxAsmFilePath.Rename(dxAsmFilePath.asString());

                    // Handle the statistics file.
                    gtString statsFileName;
                    currentStatsFilePath.getFileName(statsFileName);
                    statsFileName.prepend(L"_").prepend(entryPoint);
                    currentStatsFilePath.setFileName(statsFileName);
                    statisticsFiles.push_back(currentStatsFilePath.asString());

                    // Inform the user.
                    backendMessageCallback(cliOutput);
                    m_owner.triggerMessageReady();

                    if (cliOutput.find(KA_CLI_STR_STATUS_SUCCESS) != string::npos)
                    {
                        // We have another successful build.
                        numOfSuccessfulBuilds++;

                        isAnyDeviceBuildSuccessful = true;
                    }

                    const size_t MIN_OUTPUT_LEN = 3;

                    if (!(cliOutput.size() < MIN_OUTPUT_LEN && m_shouldBeCanceled))
                    {
                        ++numOfBuildAttempts;
                    }
                }
            }

            if (isAnyDeviceBuildSuccessful)
            {
                // Collect the statistics results into a single file
                AggregateAndRemoveStatisticsFiles(statisticsFileName, statisticsFiles);

                // Handle the statistics.
                const QString statisticsMetaFileName = acGTStringToQString(GenerateMetaFileName(m_kernelName, "", boftStatistics));
            }


            // Print the build epilogue.
            PrintBuildEpilogue(numOfBuildsOverall, numOfSuccessfulBuilds, currBuildNumber);
            result = (numOfBuildsOverall - numOfSuccessfulBuilds) == 0;
        }
    }

    return result;
}

osFilePath kaBackendManager::BuildThread::GenerateDXStatsFName(kaSourceFile* pCurrentFile,
                                                               const gtString& statisticsFileName,
                                                               const std::string& device) const
{
    gtString statsFileName;
    pCurrentFile->filePath().getFileName(statsFileName);
    // Take the base statistics file path.
    osFilePath statsFilePath;
    statsFilePath.setFullPathFromString(statisticsFileName);
    statsFileName.append(L"_" KA_STR_buildMainStatisticsFileName L"_").append(gtString().fromASCIIString(device.c_str()));
    statsFilePath.setFileName(statsFileName);
    return statsFilePath;
}

#endif

//-----------------------------------------------------------------------------
void kaBackendManager::BuildThread::run()
{
    // This is a thread function.  This function should not manipulate any
    // QT object that has signal/slot connections with other QT objects.

    // reset m_shoudBeCanceld
    m_shouldBeCanceled = false;
    // reset build succeded flag
    m_buildSucceded = false;

    // Get the full path where to write the output results.
    GT_ASSERT(m_pCurrentProgram != nullptr);

    gtString buildOutputDir = GetBuildOutputDir();

    if (VK_BUILD == m_buildType || GL_BUILD == m_buildType)
    {
        m_buildSucceded = BuildRenderProgram(buildOutputDir);
    }
    else if (CL_BUILD == m_buildType)
    {
        m_buildSucceded = LaunchOpenCLBuild(buildOutputDir);

    }

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    else if (DX_BUILD == m_buildType)
    {
        m_buildSucceded = LaunchDxBuild(buildOutputDir);

    }

#endif
}


#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
bool kaBackendManager::BuildThread::LaunchDxBuild(const gtString& buildOutputDir)
{
    bool result = true;
    int  numOfSuccessfulBuilds(0);
    const auto& ids = m_pCurrentProgram->GetFileIDsVector();
    gtList<kaSourceFile*>  sourceFiles = KA_PROJECT_DATA_MGR_INSTANCE.GetSourceFilesByIds(ids);

    for (kaSourceFile* pCurrentFile : sourceFiles)
    {
        GT_IF_WITH_ASSERT(pCurrentFile != nullptr)
        {
            gtString profile, entryPoint;
            kaDxFolder* pDxCurrentProgram = static_cast<kaDxFolder*>(m_pCurrentProgram.get());
            const int currentFileId = pCurrentFile->id();
            GT_ASSERT(pDxCurrentProgram->GetFileProfile(currentFileId, profile));
            GT_ASSERT(pDxCurrentProgram->GetFileSelectedEntryPoint(currentFileId, entryPoint));

            bool isShaderIntrinsicsEnabled = false;
            isShaderIntrinsicsEnabled = kaProjectDataManager::instance().IsD3D11ShaderIntrinsicsExtensionEnabled();

            // Prepare the source code file's full path.
            std::string sourceCodeFullPathName;
            pCurrentFile->filePath().asString().asUtf8(sourceCodeFullPathName);

            gtString sourceFileName;
            pCurrentFile->filePath().getFileName(sourceFileName);
            // Prepare the arguments for the CLI launcher.
            const gtString isaFileName = GenerateBuildOutputFileName(sourceFileName, boftISA, buildOutputDir);
            const gtString binFileName = GenerateBuildOutputFileName(sourceFileName, boftBinary, buildOutputDir);
            const gtString dxAsmFileName = GenerateBuildOutputFileName(sourceFileName, boftIL, buildOutputDir);
            const gtString statisticsFileName = GenerateBuildOutputFileName(gtString(sourceFileName).prepend(L"_").prepend(entryPoint).prepend(KA_STR_buildMainStatisticsFileName L"_"),
                                                                            boftStatistics, buildOutputDir);

            result &= BuildDxShader(pCurrentFile, isaFileName, dxAsmFileName, binFileName,
                                    statisticsFileName, sourceCodeFullPathName, numOfSuccessfulBuilds, entryPoint, profile, isShaderIntrinsicsEnabled);
        }

    }

    return result;
}
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS


bool kaBackendManager::BuildThread::LaunchOpenCLBuild(const gtString& buildOutputDir)
{
    bool result = true;
    const auto& ids = m_pCurrentProgram->GetFileIDsVector();
    gtList<kaSourceFile*>  sourceFiles = KA_PROJECT_DATA_MGR_INSTANCE.GetSourceFilesByIds(ids);

    // Launch the OpenCL build.
    for (kaSourceFile* pCurrentFile : sourceFiles)
    {
        // Prepare the source code file's full path.
        std::string sourceCodeFullPathName;
        pCurrentFile->filePath().asString().asUtf8(sourceCodeFullPathName);
        gtString sourceFileName;
        pCurrentFile->filePath().getFileName(sourceFileName);

        // Prepare the arguments for the CLI launcher.
        const gtString isaFileName = GenerateBuildOutputFileName(sourceFileName, boftISA, buildOutputDir);
        const gtString ilFileName = GenerateBuildOutputFileName(sourceFileName, boftIL, buildOutputDir);
        const gtString binFileName = GenerateBuildOutputFileName(sourceFileName, boftBinary, buildOutputDir);
        const gtString statisticsFileName = GenerateBuildOutputFileName(sourceFileName, boftStatistics, buildOutputDir);
        int numOfSuccessfulBuilds(0);

        result &= BuildOpenclSourceFile(pCurrentFile, isaFileName, binFileName, ilFileName, statisticsFileName, sourceCodeFullPathName, numOfSuccessfulBuilds);
    }

    return result;
}

//-----------------------------------------------------------------------------
void kaBackendManager::printBuildSummary(int totalGoodBuild, int totalBuild)
{
    QString successfulBuilds = QString("%1").arg(totalGoodBuild);
    QString totalBuilds = QString("%1").arg(totalBuild);
    QString message = QString("\n========== Build: %1 of %2 succeeded ==========\n\n").arg(successfulBuilds, totalBuilds);
    backendMessageCallback(std::string(message.toLatin1().data()));

    // Read log once more to make sure all contents are dumped.
    emit MessageReady();
}


void kaBackendManager::buildGLProgram(const QString& sourceCode, const std::set<std::string>& selectedDeviceNames,
                                      const QString& buildSourcePathAndName, const GLAdditionalBuildOptions& additionalOptions)
{
    if (m_pBoost != nullptr && m_pBoost->m_pBuildThread != nullptr)
    {
        m_pBoost->m_pBuildThread->m_buildType = GL_BUILD;
        m_pBoost->m_pBuildThread->m_bitness = m_buildArchitecture;
        m_pBoost->m_pBuildThread->m_glShaderType = additionalOptions.m_shaderType.asASCIICharArray();
        m_pBoost->m_pBuildThread->m_deviceNames = selectedDeviceNames;

        // Since we don't have named shaders in OpenGL, use the type as the name.
        m_pBoost->m_pBuildThread->m_kernelName = KA_STR_buildDXEntryPoint;
        m_isInBuild = true;

        if (false == m_pBoost->m_pBuildThread->isRunning())
        {
            m_sourceCodeFullPathName = acQStringToGTString(buildSourcePathAndName);
            std::string sSourceCodePath;
            gtWideStringToUtf8String(m_sourceCodeFullPathName.asCharArray(), sSourceCodePath);
            unsigned int sourceNameStart = sSourceCodePath.find_last_of("/\\");
            std::string fileName = sSourceCodePath.substr(sourceNameStart + 1, sSourceCodePath.length());
            sSourceCodePath = sSourceCodePath.substr(0, sourceNameStart);
            int numDevices = selectedDeviceNames.size();
            m_sourceCodePaths.clear();
            m_sourceCodePaths.push_back(sSourceCodePath);

            m_lastFileBuilt = fileName.c_str();

            // Set the source code for the builder thread.
            getFileContent(m_sourceCodeFullPathName.asASCIICharArray(), m_pBoost->m_pBuildThread->m_pExternalSourceCode);
            setUpForBuild(sourceCode, fileName, numDevices);
            emit buildStart(acQStringToGTString(sourceCode));
            m_pBoost->m_pBuildThread->start();
        }
    }
}

//-------------------------------------------------------------------------------------------------------------
void kaBackendManager::PrepareProgramBuild(kaProgram* pProgram,
                                           const std::set<std::string>& selectedDeviceNames,
                                           AnalyzerBuildArchitecture bitness /*= kaBuildArch32_bit*/)
{
    GT_IF_WITH_ASSERT(pProgram != nullptr)
    {
        m_buildArchitecture = bitness;
        //copy devices names
        m_selectedDeviceNamesForPendingBuild = selectedDeviceNames;

        afApplicationCommands::instance()->ClearInformationView();

        switch (pProgram->GetBuildType())
        {
            case kaProgramVK_Rendering:
            case kaProgramVK_Compute:
            {
                m_buildType = BuiltProgramKind_Vulkan;
                PrepareProgramBuildInner(pProgram, VK_BUILD);
            }
            break;

            case kaProgramGL_Rendering:
            case kaProgramGL_Compute:
            {
                m_buildType = BuiltProgramKind_Vulkan;
                PrepareProgramBuildInner(pProgram, GL_BUILD);
            }
            break;

            case kaProgramCL:
            {
                m_buildType = BuiltProgramKind_OpenCL;
                PrepareProgramBuildInner(pProgram, CL_BUILD);
            }
            break;

            case kaProgramDX:
            {
                m_buildType = BuiltProgramKind_DX;
                PrepareProgramBuildInner(pProgram, DX_BUILD);
            }
            break;

            default:
                break;
        }
    }
}

//-------------------------------------------------------------------------------------------------------------
void kaBackendManager::PrepareProgramBuildInner(kaProgram* pProgram, const BuildType buildType)
{
    // Print build prologue.
    const size_t numOfDevicesToBuild = m_selectedDeviceNamesForPendingBuild.size();
    const gtString programDisplayName = pProgram->GetProgramDisplayName();
    PrintBuildPrologue(programDisplayName, numOfDevicesToBuild, false);

    // for multiple programs build we add programs into synchronized queue and execute them sequentially in parallel thread
    m_executionWaitingList.push(make_pair(pProgram->Clone(), buildType));
    if (m_executionThread == nullptr)
    {
        m_executionThread.reset(new std::thread(
            [&]()
        {
            while (m_stopExecutionThread == false)
            {
                // wait till new program to build added or execution stop signal received
                while (m_stopExecutionThread == false && (m_buildCompleted == false || m_executionWaitingList.isEmpty()))
                {
                    osSleep(50);
                }
                if (m_stopExecutionThread == false && 
                    m_pBoost != nullptr && 
                    m_pBoost->m_pBuildThread != nullptr &&
                    m_buildCompleted &&
                    m_executionWaitingList.isEmpty() == false)
                {
                    auto programBuildType = m_executionWaitingList.pop();
                    m_buildCompleted = false;
                    ExecuteBuildThread(programBuildType.second, programBuildType.first);
                    delete programBuildType.first;
                }
            }
        }
        ));
    }//if m_executionThread == nullptr
}

void kaBackendManager::ExecuteBuildThread(const BuildType buildType, kaProgram* pProgram)
{
    m_pBoost->m_pBuildThread->m_buildType = buildType;
    m_pBoost->m_pBuildThread->m_bitness = m_buildArchitecture;
    m_pBoost->m_pBuildThread->m_deviceNames = m_selectedDeviceNamesForPendingBuild;   
    m_pBoost->m_pBuildThread->m_shadersPaths.Clear();
    m_isInBuild = true;

    if (false == m_pBoost->m_pBuildThread->isRunning())
    {
        m_pBoost->m_pBuildThread->m_pCurrentProgram.reset(pProgram->Clone());

        if (pProgram != nullptr)
        {
            const kaProgramTypes  programBuildType = pProgram->GetBuildType();

            if (kaProgramVK_Compute == programBuildType || programBuildType == kaProgramGL_Compute)
            {
                kaComputeProgram* pVKCompute = dynamic_cast<kaComputeProgram*>(pProgram);

                if (pVKCompute != nullptr)
                {
                    int computeShaderID = pVKCompute->GetFileID();
                    osFilePath filePath;
                    KA_PROJECT_DATA_MGR_INSTANCE.GetFilePathByID(computeShaderID, filePath);
                    m_pBoost->m_pBuildThread->m_shadersPaths.m_computeShader = filePath.asString();
                }
            }
            else if (kaProgramVK_Rendering == programBuildType || kaProgramGL_Rendering == programBuildType)
            {
                kaRenderingProgram* pRenderProgram = dynamic_cast<kaRenderingProgram*>(pProgram);

                if (pRenderProgram != nullptr)
                {
                    pRenderProgram->GetPipelinePaths(m_pBoost->m_pBuildThread->m_shadersPaths);
                }

            }
            else if (kaProgramCL == programBuildType)
            {
                if (nullptr == m_pBoost->m_pCLOptions.get())
                {
                    m_pBoost->m_pCLOptions.reset(new beProgramBuilderOpenCL::OpenCLOptions());
                    m_pBoost->m_pCLOptions->m_SourceLanguage = beKA::SourceLanguage_OpenCL;
                }

                m_pBoost->m_pCLOptions->m_SelectedDevices = m_selectedDeviceNamesForPendingBuild;
                m_pBoost->m_pCLOptions->m_OpenCLCompileOptions.clear();

                QString qstrBuildOptions = KA_PROJECT_DATA_MGR_INSTANCE.BuildOptions();
                UpdateOBuildOption(qstrBuildOptions);
                m_pBoost->m_pCLOptions->m_OpenCLCompileOptions.push_back(string(qstrBuildOptions.toLatin1().constData()));
                m_pBoost->m_pBuildThread->m_pExternalOptions = m_pBoost->m_pCLOptions.get();

            }

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
            else if (kaProgramDX == programBuildType)
            {

                PreparedDxAdditionOptions(m_pBoost->m_pBuildThread->m_dxBuildOptions);
                // Print the message about the ignored options (if any).
                DxPrintIgnoredOptions();
            }

#endif
        }

        m_pBoost->m_pBuildThread->start();
    }
}

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
bool  kaBackendManager::PreparedDxAdditionOptions(DXAdditionalBuildOptions& additionalDxBuildOptions) const
{
    bool isCommandOk = true;
    bool isDefaultD3dCompiler = false;
    additionalDxBuildOptions.m_builderType = KA_PROJECT_DATA_MGR_INSTANCE.ShaderCompileType();
    additionalDxBuildOptions.m_additionalMacros = KA_PROJECT_DATA_MGR_INSTANCE.ShaderMacros();
    additionalDxBuildOptions.m_additionalIncludes = KA_PROJECT_DATA_MGR_INSTANCE.ShaderIncludes();
    additionalDxBuildOptions.m_buildOptionsMask = 0;
    additionalDxBuildOptions.m_buildOptions = KA_PROJECT_DATA_MGR_INSTANCE.ShaderBuildOptions();

    if (additionalDxBuildOptions.m_builderType == KA_STR_HLSL_optionsDialogD3DCompileType)
    {
        additionalDxBuildOptions.m_buildOptionsMask = KA_PROJECT_DATA_MGR_INSTANCE.ShaderD3dBuildOptionsMask();

        additionalDxBuildOptions.m_builderPath = KA_PROJECT_DATA_MGR_INSTANCE.ShaderD3dBuilderPath();

        // Check if the default compiler option was chosen.
        if (additionalDxBuildOptions.m_builderPath == KA_STR_HLSL_optionsDialogDefaultCompiler)
        {
            isDefaultD3dCompiler = true;

            // We no longer need to pass the path for the default compiler, because the CLI already knows it.
            additionalDxBuildOptions.m_builderPath.clear();
        }

        // if path empty - there is no d3d compiler dll selected or default is selected and the bundled dll are missing
        if (!isDefaultD3dCompiler && additionalDxBuildOptions.m_builderPath.isEmpty())
        {
            acMessageBox::instance().critical(AF_STR_ErrorA, KA_STR_DirectXNoD3dBuilderSelectedError);
            isCommandOk = false;

        }
        // if no file was selected - the current selected item in the combo is "browse.."
        else if (additionalDxBuildOptions.m_builderPath == KA_STR_HLSL_optionsDialogBrowse)
        {
            // no builder file was selected
            acMessageBox::instance().critical(AF_STR_ErrorA, KA_STR_DirectXNoD3dBuilderSelectedError);
            isCommandOk = false;
        }

    }
    else
    {
        additionalDxBuildOptions.m_builderPath = KA_PROJECT_DATA_MGR_INSTANCE.ShaderFxcBuilderPath();

        // check for valid builder path
        if (additionalDxBuildOptions.m_builderPath == KA_STR_HLSL_optionsDialogBrowse)
        {
            // no builder file was selected
            acMessageBox::instance().critical(AF_STR_ErrorA, KA_STR_DirectXNoFxcBuilderSelectedError);
            isCommandOk = false;
        }
    }

    return isCommandOk;
}
#endif //AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

void kaBackendManager::PrintBuildPrologue(const gtString& programName, size_t numOfDevicesToBuild, bool shouldPrependCR)
{
    gtASCIIString message;
    message.appendFormattedString(KA_STR_BUILD_STARTED, programName.asASCIICharArray(), numOfDevicesToBuild);

    if (shouldPrependCR)
    {
        message.prepend("\n");
    }

    kaPipelinedProgram* pPipelineProgram = dynamic_cast<kaPipelinedProgram*>(KA_PROJECT_DATA_MGR_INSTANCE.GetProgram(programName));

    if (pPipelineProgram != nullptr)
    {
        for (int stage = kaPipelinedProgram::KA_PIPELINE_STAGE_NONE + 1; stage < kaPipelinedProgram::KA_PIPELINE_STAGE_LAST; ++stage)
        {
            osFilePath stageFilePath;
            kaPipelinedProgram::PipelinedStage pipeLineStage = static_cast<kaPipelinedProgram::PipelinedStage>(stage);
            pPipelineProgram->GetFilePath(pipeLineStage, stageFilePath);

            if (stageFilePath.isEmpty() == false)
            {
                gtString stageAsString = kaUtils::PipeLineStageToLongFormat(pipeLineStage);
                message.appendFormattedString(AF_STR_BUILDING_STAGE).appendFormattedString(stageAsString.asASCIICharArray()).appendFormattedString(AF_STR_FILENAME_SEPARATOR).appendFormattedString(stageFilePath.asString().asASCIICharArray()).appendFormattedString("\n");

            }
        }

        message.appendFormattedString(KA_STR_BUILD_STARTED, programName.asASCIICharArray(), numOfDevicesToBuild);
    }

    std::string buildMessage(message.asCharArray());
    backendMessageCallback(buildMessage);
    emit MessageReady();
}

// ---------------------------------------------------------------------------
void kaBackendManager::addStringToInformationView(const std::string& msg)
{
    // Remove terminating newline because the output window adds a newline automatically after each line that is added from this queue
    std::string msgWithNoTerminatingNewLine(msg);
    int lastCharPos = msgWithNoTerminatingNewLine.size() - 1;

    if (lastCharPos >= 0)
    {
        // If the string ends with a newline character
        if (msgWithNoTerminatingNewLine[lastCharPos] == '\n')
        {
            // Remove the newline character
            msgWithNoTerminatingNewLine.erase(lastCharPos, 1);
        }
    }

    if (afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
    {
        // If we are inside VS, don't call AddStringToInformationView() directly,
        // since we might be in a different thread in this place. Instead, emit
        // a signal which would be handled by the main thread. This way we prevent
        // a scenario where a thread other than the main thread attempts to change
        // the GUI (which makes the VS build output pane invalidated).
        emit printMessageForUser(QString::fromLatin1(msgWithNoTerminatingNewLine.c_str()));
    }
    else
    {
        // Otherwise (we're in SA), directly call the function.
        afApplicationCommands::instance()->AddStringToInformationView(QString::fromLatin1(msgWithNoTerminatingNewLine.c_str()));
    }
}

// *** TEMPORARY CHANGE: SUBMITTED INTENTIONALLY ***
// These functions are temporarily unused.
// Restricting this sections to Windows to avoid Linux build failure.
// *** TEMPORARY CHANGE: SUBMITTED INTENTIONALLY ***
#if AMDT_BUILD_TARGET==AMDT_WINDOWS_OS

bool kaBackendManager::isAnalysisRequired() const
{
    return m_analyzePendingBuild;
}

#endif



// Convert "XXXXX.YYY" to "XXXXX DEVICENAME.YYY"
// It is important to separate the kernel name from the deice with a space character, because this is the format that the build files name parser expects
void kaBackendManager::GeneralFileNameToDeviceSpecificFileName(const gtString& generalFileName, const std::string& device, gtString& deviceSpecificFile)
{
    // Clear the output buffer.
    deviceSpecificFile.makeEmpty();

    // Take the base statistics file path.
    osFilePath statsFilePath;
    statsFilePath.setFullPathFromString(generalFileName);

    // Fix the base file path.
    gtString fixedFileName;
    fixedFileName << KA_STR_buildMainStatisticsFileName << "_" << device.c_str();
    statsFilePath.setFileName(fixedFileName);
    statsFilePath.setFileExtension(KA_STR_kernelViewExtension);

    // Fill the output buffer.
    deviceSpecificFile = statsFilePath.asString();
}

void kaBackendManager::CancelBuild()
{
    m_pBoost->m_pBuildThread->m_shouldBeCanceled = true;
}

bool kaBackendManager::IsBuildCancelled() const
{
    return m_pBoost->m_pBuildThread->m_shouldBeCanceled;
}

bool kaBackendManager::IsBuildSucceded() const
{
    return m_pBoost->m_pBuildThread->m_buildSucceded;
}

gtVector<int> kaBackendManager::GetLastBuildProgramFileIds() const
{
    gtVector<int> result;

    if (m_pBoost != nullptr && m_pBoost->m_pBuildThread != nullptr && m_pBoost->m_pBuildThread->m_pCurrentProgram != nullptr)
    {
        result = m_pBoost->m_pBuildThread->m_pCurrentProgram->GetFileIDsVector();
    }

    return result;
}

gtString kaBackendManager::GetLastBuildProgramName() const
{
    gtString result;

    if (m_pBoost != nullptr && m_pBoost->m_pBuildThread != nullptr && m_pBoost->m_pBuildThread->m_pCurrentProgram != nullptr)
    {
        result = m_pBoost->m_pBuildThread->m_pCurrentProgram->GetProgramName();
    }

    return result;

}
#if AMDT_BUILD_TARGET==AMDT_WINDOWS_OS
bool kaBackendManager::GetCurrentFileAdditonalDXBuildOptions(DXAdditionalBuildOptions& currentFileAdditionalBuildOptions) const
{
    bool isCommandOk = true;
    bool isDefaultD3dCompiler = false;

    currentFileAdditionalBuildOptions.m_builderType = KA_PROJECT_DATA_MGR_INSTANCE.ShaderCompileType();
    currentFileAdditionalBuildOptions.m_additionalMacros = KA_PROJECT_DATA_MGR_INSTANCE.ShaderMacros();
    currentFileAdditionalBuildOptions.m_additionalIncludes = KA_PROJECT_DATA_MGR_INSTANCE.ShaderIncludes();
    currentFileAdditionalBuildOptions.m_buildOptionsMask = 0;

    if (currentFileAdditionalBuildOptions.m_builderType == KA_STR_HLSL_optionsDialogD3DCompileType)
    {
        currentFileAdditionalBuildOptions.m_buildOptionsMask = KA_PROJECT_DATA_MGR_INSTANCE.ShaderD3dBuildOptionsMask();

        currentFileAdditionalBuildOptions.m_builderPath = KA_PROJECT_DATA_MGR_INSTANCE.ShaderD3dBuilderPath();

        // Check if the default compiler option was chosen.
        if (currentFileAdditionalBuildOptions.m_builderPath == KA_STR_HLSL_optionsDialogDefaultCompiler)
        {
            isDefaultD3dCompiler = true;

            // We no longer need to pass the path for the default compiler, because the CLI already knows it.
            currentFileAdditionalBuildOptions.m_builderPath.clear();
        }

        // if path empty - there is no d3d compiler dll selected or default is selected and the bundled dll are missing
        if (!isDefaultD3dCompiler && currentFileAdditionalBuildOptions.m_builderPath.isEmpty())
        {
            acMessageBox::instance().critical(AF_STR_ErrorA, KA_STR_DirectXNoD3dBuilderSelectedError);
            isCommandOk = false;

        }
        // if no file was selected - the current selected item in the combo is "browse.."
        else if (currentFileAdditionalBuildOptions.m_builderPath == KA_STR_HLSL_optionsDialogBrowse)
        {
            // no builder file was selected
            acMessageBox::instance().critical(AF_STR_ErrorA, KA_STR_DirectXNoD3dBuilderSelectedError);
            isCommandOk = false;
        }

    }
    else
    {
        currentFileAdditionalBuildOptions.m_builderPath = KA_PROJECT_DATA_MGR_INSTANCE.ShaderFxcBuilderPath();

        // check for valid builder path
        if (currentFileAdditionalBuildOptions.m_builderPath == KA_STR_HLSL_optionsDialogBrowse)
        {
            // no builder file was selected
            acMessageBox::instance().critical(AF_STR_ErrorA, KA_STR_DirectXNoFxcBuilderSelectedError);
            isCommandOk = false;
        }
    }

    return isCommandOk;
}

#endif
