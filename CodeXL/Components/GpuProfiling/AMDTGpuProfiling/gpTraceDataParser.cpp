//------------------------------ GPUProfileSessionDataParser.cpp ------------------------------

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
#include <AMDTGpuProfiling/gpTraceDataParser.h>
#include <AMDTGpuProfiling/gpTraceDataContainer.h>
#include <AMDTGpuProfiling/SymbolInfo.h>

#pragma message ("TODO: FA: Read this value from file")
#define GP_ATP_FILE_VERSION 0
#define GP_ATR_FILE_VERSION 1
#define GP_MAX_API_TO_PARSE 200000


#pragma message ("TODO: FA: remove GPUSessionTreeItemData from this class. Class should get a file path, and the occupancy data should be parsed differently")

/// True iff the user canceled the operation
static  bool m_sShouldCancelParsing = false;
void OnCancel()
{
    m_sShouldCancelParsing = true;
}

gpTraceDataParser::gpTraceDataParser() :
    m_pSessionItemData(nullptr),
    m_isOccupancyFileLoaded(false),
    m_isExecutingOccupancyFileLoad(false)
{
    /// Initialize the data container
    m_pSessionDataContainer = new gpTraceDataContainer;

    m_sShouldCancelParsing = false;
}

gpTraceDataParser::~gpTraceDataParser()
{
}

bool gpTraceDataParser::Parse(const osFilePath& traceFilePath, GPUSessionTreeItemData* pSessionItemData, bool& wasParseCanceled)
{
    bool retVal = false;

    // Set the item data
    m_pSessionItemData = pSessionItemData;

    // Sanity check:
    GT_IF_WITH_ASSERT((m_pSessionItemData != nullptr) && (m_pSessionItemData->m_pParentData != nullptr))
    {
        // Set the session file path
        gtString extension;
        m_traceFilePath = traceFilePath;

        int fileVersion = GP_ATR_FILE_VERSION;
        m_traceFilePath.getFileExtension(extension);

        if (extension == GP_ATP_FileExtensionW)
        {
            fileVersion = GP_ATP_FILE_VERSION;
        }
        else if (extension == GP_LTR_FileExtensionW)
        {
            fileVersion = GP_ATR_FILE_VERSION;
        }
        else
        {
            GT_ASSERT_EX(false, L"unknown file extension");
        }

        Config config;
        AtpFileParser parser(fileVersion);

        // add part parser for OpenCL API Trace and Timestamp sections
        CLAtpFilePart clAtpPart(config, false);
        clAtpPart.AddProgressMonitor(this);
        parser.AddAtpFilePart(&clAtpPart);

        // add part parser for HSA API Trace and Timestamp sections
        HSAAtpFilePart hsaAtpPart(config);
        hsaAtpPart.AddProgressMonitor(this);
        parser.AddAtpFilePart(&hsaAtpPart);

        // add part parser for cl Stack Trace section
        StackTraceAtpFilePart clStackPart("ocl", config, false);
        clStackPart.AddProgressMonitor(this);
        parser.AddAtpFilePart(&clStackPart);

        // add part parser for hsa Stack Trace section
        StackTraceAtpFilePart hsaStackPart("hsa", config, false);
        hsaStackPart.AddProgressMonitor(this);
        parser.AddAtpFilePart(&hsaStackPart);

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
        hsaAtpPart.AddListener(this);
        clStackPart.AddListener(this);
        hsaStackPart.AddListener(this);
        perfMarkerPart.AddListener(this);
        dxFilePart.AddListener(this);
        vkFilePart.AddListener(this);

        // Load the kernel occupancy file
        LoadOccupancyFile();

        // Load the session file
        retVal = parser.LoadFile(m_traceFilePath.asString().asASCIICharArray());
        GT_IF_WITH_ASSERT(retVal)
        {
            // Parse the file
            retVal = parser.Parse();

            bool parseWarning = false;
            std::string parseWarningMsg;
            parser.GetParseWarning(parseWarning, parseWarningMsg);

            if (!(retVal) || parseWarning)
            {
                QString parseError = QString("Error parsing %1").arg(m_traceFilePath.asString().asASCIICharArray());

                if (parseWarning)
                {
                    parseError.append("\n").append(QString::fromStdString(parseWarningMsg));
                }

                Util::ShowWarningBox(parseError);
                m_sShouldCancelParsing = true;
            }

            // Sanity check:
            GT_IF_WITH_ASSERT(m_pSessionDataContainer != nullptr)
            {
                m_pSessionDataContainer->FinalizeDataCollection();
            }

            // Set the user cancelation flag
            wasParseCanceled = m_sShouldCancelParsing;
            m_sShouldCancelParsing = false;
        }
    }

    return retVal;

}

void gpTraceDataParser::OnParse(CLAPIInfo* pAPIInfo, bool& stopParsing)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pAPIInfo != nullptr)
    {
        stopParsing = m_sShouldCancelParsing;

        // Add the item to the data container
        ProfileSessionDataItem* pAPIItem = m_pSessionDataContainer->AddCLItem(pAPIInfo);

        // Sanity check:
        GT_IF_WITH_ASSERT(pAPIItem != nullptr)
        {
            // For enqueue memory objects, added a GPU item
            bool isEnquque = (pAPIInfo->m_Type & CL_ENQUEUE_BASE_API) == CL_ENQUEUE_BASE_API;

            if (isEnquque)
            {
                // Enqueue API functions have a matching GPU item
                m_pSessionDataContainer->AddCLGPUItem(pAPIItem);
            }
        }

        if (m_pSessionDataContainer->APICount() > GP_MAX_API_TO_PARSE)
        {
            stopParsing = true;
        }
    }
}


void gpTraceDataParser::OnParse(HSAAPIInfo* pAPIInfo, bool& stopParsing)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pAPIInfo != nullptr)
    {
        stopParsing = m_sShouldCancelParsing;

        // For dispatch API function, add a GPU item
        if (pAPIInfo->m_apiID == HSA_API_Type_Non_API_Dispatch)
        {
            // Enqueue API functions have a matching GPU item
            m_pSessionDataContainer->AddHSAGPUItem(pAPIInfo);
        }
        else
        {
            // Add the item to the data container
            m_pSessionDataContainer->AddHSAItem(pAPIInfo);
        }

        if (m_pSessionDataContainer->APICount() > GP_MAX_API_TO_PARSE)
        {
            stopParsing = true;
        }
    }
}

void gpTraceDataParser::OnParse(SymbolFileEntry* pSymbolFileEntry, bool& stopParsing)
{
    stopParsing = m_sShouldCancelParsing;

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

void gpTraceDataParser::OnParse(PerfMarkerEntry* pPerfMarkerEntry, bool& stopParsing)
{
    GT_UNREFERENCED_PARAMETER(stopParsing);

    // Sanity check:
    GT_IF_WITH_ASSERT((pPerfMarkerEntry != nullptr) && (m_pSessionDataContainer != nullptr))
    {
        // Add the performance marker to the container
        m_pSessionDataContainer->AddPerformanceMarker(pPerfMarkerEntry);
    }
}

void gpTraceDataParser::OnParse(DX12APIInfo* pAPIInfo, bool& stopParsing)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((pAPIInfo != nullptr) && (m_pSessionDataContainer != nullptr))
    {
        stopParsing = m_sShouldCancelParsing;

        if (!pAPIInfo->m_isGPU)
        {
            // Add the item to the data container
            m_pSessionDataContainer->AddDX12APIItem(pAPIInfo);
        }

        else
        {
            // Add the item to the data container
            m_pSessionDataContainer->AddDX12GPUTraceItem((DX12GPUTraceInfo*)pAPIInfo);
        }
    }
}

void gpTraceDataParser::OnParse(VKAPIInfo* pAPIInfo, bool& stopParsing)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((pAPIInfo != nullptr) && (m_pSessionDataContainer != nullptr))
    {
        stopParsing = m_sShouldCancelParsing;

        if (!pAPIInfo->m_isGPU)
        {
            // Add the item to the data container
            m_pSessionDataContainer->AddVKAPIItem(pAPIInfo);
        }

        else
        {
            // Add the item to the data container
            m_pSessionDataContainer->AddVKGPUTraceItem((VKGPUTraceInfo*)pAPIInfo);
        }
    }
}


void gpTraceDataParser::SetAPINum(osThreadId threadId, unsigned int apiNum)
{
    // Set the API number
    m_pSessionDataContainer->SetAPINum(threadId, apiNum);
}

void gpTraceDataParser::OnParserProgress(const std::string& strProgressMessage, unsigned int uiCurItem, unsigned int uiTotalItems)
{
    gtString localProgressMsg;
    localProgressMsg.fromASCIIString(strProgressMessage.c_str());

    if (uiCurItem == 0)
    {
        afProgressBarWrapper::instance().ShowProgressDialog(localProgressMsg, uiTotalItems, 0, true, &OnCancel);
    }

    afProgressBarWrapper::instance().updateProgressBar((int)uiCurItem);
}


bool gpTraceDataParser::LoadOccupancyFile()
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
            retVal = Util::LoadOccupancyFile(m_traceFilePath, m_pSessionDataContainer->m_occupancyInfoMap, m_pSessionItemData);

            m_isOccupancyFileLoaded = retVal;

        }
    }

    return retVal;

}



