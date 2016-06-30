//------------------------------ gpObjectDataParser.cpp ------------------------------

// TODO Note: The file is created to match Object data parser to trace data parser.  Most functionality is not required, need more cleanup

#include <qtIgnoreCompilerWarnings.h>

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>

#include <CL/cl.h>

// Backend header files
#include "IParserListener.h"
#include "IParserProgressMonitor.h"
#include "CLAPIInfo.h"
#include "CLAtpFile.h"
#include "CLAPIFilterManager.h"
#include "CLAPIDefs.h"
#include "CLTimelineItems.h"
#include "DX12AtpFile.h"
#include "VulkanAtpFile.h"
#include "ProfileManager.h"

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>
#include <AMDTGpuProfiling/gpObjectDataParser.h>
#include <AMDTGpuProfiling/gpObjectDataContainer.h>
#include <AMDTGpuProfiling/SymbolInfo.h>

#ifdef GP_OBJECT_VIEW_ENABLE

#pragma message ("TODO: FA: Read this value from file")
#define GP_MAX_OBJECT_TO_PARSE 200000
#define GP_AOR_FILE_VERSION 2

#pragma message ("TODO: FA: remove GPUSessionTreeItemData from this class. Class should get a file path, and the occupancy data should be parsed differently")

/// True iff the user canceled the operation
bool m_sShouldCancelExport = false;
void OnCancelObj()
{
    m_sShouldCancelExport = true;
}

gpObjectDataParser::gpObjectDataParser() :
    m_pSessionItemData(nullptr),
    m_isOccupancyFileLoaded(false),
    m_isExecutingOccupancyFileLoad(false)
{
    /// Initialize the data container
    m_pSessionDataContainer = new gpObjectDataContainer;

    m_sShouldCancelExport = false;
}

gpObjectDataParser::~gpObjectDataParser()
{
}

bool gpObjectDataParser::Parse(const osFilePath& objectFilePath, GPUSessionTreeItemData* pSessionItemData, bool& wasParseCanceled)
{
    bool retVal = false;

    // Set the item data
    m_pSessionItemData = pSessionItemData;

    // Sanity check:
    GT_IF_WITH_ASSERT((m_pSessionItemData != nullptr) && (m_pSessionItemData->m_pParentData != nullptr))
    {
        // Set the session file path
        gtString extension;
        m_objectFilePath = objectFilePath;

        int fileVersion = GP_AOR_FILE_VERSION;
        m_objectFilePath.getFileExtension(extension);

        if (extension == GP_AOR_FileExtensionW)
        {
            fileVersion = GP_AOR_FILE_VERSION;
        }
        else
        {
            GT_ASSERT_EX(false, L"unknown file extension");
        }

        Config config;
        AtpFileParser parser(fileVersion);

        // add part parser for OpenCL API Object and Timestamp sections
        CLAtpFilePart clAtpPart(config, false);
        clAtpPart.AddProgressMonitor(this);
        parser.AddAtpFilePart(&clAtpPart);

        PerfMarkerAtpFilePart perfMarkerPart(config, false);
        perfMarkerPart.AddProgressMonitor(this);
        parser.AddAtpFilePart(&perfMarkerPart);

        DX12AtpFilePart dxFilePart(config, false);
        dxFilePart.AddProgressMonitor(this);
        parser.AddAtpFilePart(&dxFilePart);

        VKAtpFilePart vkFilePart(config, false);
        vkFilePart.AddProgressMonitor(this);
        parser.AddAtpFilePart(&vkFilePart);

        clAtpPart.AddListener(this);
        perfMarkerPart.AddListener(this);
        dxFilePart.AddListener(this);
        vkFilePart.AddListener(this);

        // Load the kernel occupancy file
        LoadOccupancyFile();

        // Load the session file
        retVal = parser.LoadFile(m_objectFilePath.asString().asASCIICharArray());
        GT_IF_WITH_ASSERT(retVal)
        {
            // Parse the file
            retVal = parser.Parse();

            bool parseWarning = false;
            string parseWarningMsg;
            parser.GetParseWarning(parseWarning, parseWarningMsg);

            if (!(retVal || parseWarning))
            {
                QString parseError = QString("Error parsing %1").arg(m_objectFilePath.asString().asASCIICharArray());

                if (parseWarning)
                {
                    parseError.append("\n").append(QString::fromStdString(parseWarningMsg));
                }

                Util::ShowWarningBox(parseError);
            }

            // Sanity check:
            GT_IF_WITH_ASSERT(m_pSessionDataContainer != nullptr)
            {
                m_pSessionDataContainer->FinalizeDataCollection();
            }

            // Set the user cancelation flag
            wasParseCanceled = m_sShouldCancelExport;
            m_sShouldCancelExport = false;
        }
    }

    return retVal;
}

void gpObjectDataParser::OnParse(CLAPIInfo* pAPIInfo, bool& stopParsing)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pAPIInfo != nullptr)
    {
        stopParsing = m_sShouldCancelExport;

        if (m_pSessionDataContainer->APICount() > GP_MAX_OBJECT_TO_PARSE)
        {
            stopParsing = true;
        }
    }
}

void gpObjectDataParser::OnParse(HSAAPIInfo* pAPIInfo, bool& stopParsing)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pAPIInfo != nullptr)
    {
        stopParsing = m_sShouldCancelExport;

        if (pAPIInfo->m_apiID != HSA_API_Type_Non_API_Dispatch)
        {
            // Add the item to the data container
            m_pSessionDataContainer->AddHSAItem(pAPIInfo);
        }

        if (m_pSessionDataContainer->APICount() > GP_MAX_OBJECT_TO_PARSE)
        {
            stopParsing = true;
        }
    }
}

void gpObjectDataParser::OnParse(SymbolFileEntry* pSymbolFileEntry, bool& stopParsing)
{
    stopParsing = m_sShouldCancelExport;

    // Sanity check
    GT_IF_WITH_ASSERT((pSymbolFileEntry != nullptr) && (m_pSessionDataContainer != nullptr))
    {
        osThreadId threadId = pSymbolFileEntry->m_tid;

        SymbolInfo* pEntry = nullptr;

        if ((pSymbolFileEntry->m_pStackEntry != nullptr) && (pSymbolFileEntry->m_pStackEntry->m_dwLineNum != (LineNum)(-1)) && (!pSymbolFileEntry->m_pStackEntry->m_strFile.empty()))
        {
            pEntry = new SymbolInfo(QString::fromStdString(pSymbolFileEntry->m_strAPIName),
                                    QString::fromStdString(pSymbolFileEntry->m_pStackEntry->m_strSymName),
                                    QString::fromStdString(pSymbolFileEntry->m_pStackEntry->m_strFile),
                                    pSymbolFileEntry->m_pStackEntry->m_dwLineNum);
        }
        else
        {
            pEntry = new SymbolInfo;
        }

        if (m_pSessionDataContainer->m_symbolTableMap.contains(threadId))
        {
            m_pSessionDataContainer->m_symbolTableMap[threadId].append(pEntry);
        }
        else
        {
            QList<SymbolInfo*> list;
            list.append(pEntry);
            m_pSessionDataContainer->m_symbolTableMap.insert(threadId, list);
        }
    }
}

void gpObjectDataParser::OnParse(PerfMarkerEntry* pPerfMarkerEntry, bool& stopParsing)
{
    GT_UNREFERENCED_PARAMETER(stopParsing);
    GT_UNREFERENCED_PARAMETER(pPerfMarkerEntry);
}

void gpObjectDataParser::OnParse(DX12APIInfo* pAPIInfo, bool& stopParsing)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((pAPIInfo != nullptr) && (m_pSessionDataContainer != nullptr))
    {
        stopParsing = m_sShouldCancelExport;

        if (!pAPIInfo->m_isGPU)
        {
            // Add the item to the data container
            m_pSessionDataContainer->AddDX12APIItem(pAPIInfo);
        }

        if (m_pSessionDataContainer->APICount() > GP_MAX_OBJECT_TO_PARSE)
        {
            stopParsing = true;
        }
    }
}

void gpObjectDataParser::OnParse(VKAPIInfo* pAPIInfo, bool& stopParsing)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((pAPIInfo != nullptr) && (m_pSessionDataContainer != nullptr))
    {
        stopParsing = m_sShouldCancelExport;

        if (!pAPIInfo->m_isGPU)
        {
            // Add the item to the data container
            m_pSessionDataContainer->AddVKAPIItem(pAPIInfo);
        }

        if (m_pSessionDataContainer->APICount() > GP_MAX_OBJECT_TO_PARSE)
        {
            stopParsing = true;
        }
    }
}


void gpObjectDataParser::SetAPINum(osThreadId threadId, unsigned int apiNum)
{
    // Set the API number
    m_pSessionDataContainer->SetAPINum(threadId, apiNum);
}

void gpObjectDataParser::OnParserProgress(const std::string& strProgressMessage, unsigned int uiCurItem, unsigned int uiTotalItems)
{
    gtString localProgressMsg;
    localProgressMsg.fromASCIIString(strProgressMessage.c_str());

    if (uiCurItem == 0)
    {
        afProgressBarWrapper::instance().ShowProgressDialog(localProgressMsg, uiTotalItems, 0, true, &OnCancelObj);
    }

    afProgressBarWrapper::instance().updateProgressBar((int)uiCurItem);
}


bool gpObjectDataParser::LoadOccupancyFile()
{
    bool retVal = true;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSessionItemData != nullptr)
    {
        // Only try for the first time. This function is called many times, try to load only on first time:
        if (!m_isOccupancyFileLoaded && !m_isExecutingOccupancyFileLoad)
        {
            m_isExecutingOccupancyFileLoad = true;

            // Load the occupancy file
            retVal = Util::LoadOccupancyFile(m_objectFilePath, m_pSessionDataContainer->m_occupancyInfoMap, m_pSessionItemData);

            m_isOccupancyFileLoaded = retVal;
        }
    }

    return retVal;
}

#endif
