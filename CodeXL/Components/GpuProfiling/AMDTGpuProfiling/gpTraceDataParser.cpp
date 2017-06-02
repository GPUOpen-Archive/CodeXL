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
#include "CLAPIFilterManager.h"
#include "CLAPIDefs.h"
#include "CLTimelineItems.h"
#include "DX12Trace/DX12AtpFile.h"
#include "VulkanTrace/VulkanAtpFile.h"
#include "ProfileManager.h"

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>
#include <AMDTGpuProfiling/gpTraceDataParser.h>
#include <AMDTGpuProfiling/gpTraceDataContainer.h>
#include <AMDTGpuProfiling/SymbolInfo.h>
#include "AtpUtils.h"

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

        DX12AtpFilePart dxFilePart(config, false);
        dxFilePart.AddProgressMonitor(this);
        parser.AddAtpFilePart(&dxFilePart);

        VKAtpFilePart vkFilePart(config, false);
        vkFilePart.AddProgressMonitor(this);
        parser.AddAtpFilePart(&vkFilePart);

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

void gpTraceDataParser::OnParse(ICLAPIInfoDataHandler* pAPIInfo, bool& stopParsing)
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
            bool isEnquque = (pAPIInfo->GetCLApiType() & CL_ENQUEUE_BASE_API) == CL_ENQUEUE_BASE_API;

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


void gpTraceDataParser::OnParse(IHSAAPIInfoDataHandler* pAPIInfo, bool& stopParsing)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pAPIInfo != nullptr)
    {
        stopParsing = m_sShouldCancelParsing;

        // For dispatch API function, add a GPU item
        if (pAPIInfo->GetHSAApiTypeId() == HSA_API_Type_Non_API_Dispatch)
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

void gpTraceDataParser::OnParse(ISymbolFileEntryInfoDataHandler* pSymbolFileEntry, bool& stopParsing)
{
    stopParsing = m_sShouldCancelParsing;

    // Sanity check
    GT_IF_WITH_ASSERT((pSymbolFileEntry != nullptr) && (m_pSessionDataContainer != nullptr))
    {
        osThreadId threadId = pSymbolFileEntry->GetsymbolThreadId();

        SymbolInfo* pEntry = nullptr;

        IStackEntryInfoDataHandler* pStackEntryInfoHandler = pSymbolFileEntry->GetStackEntryInfoHandler();

        if (!pSymbolFileEntry->IsStackEntryNull() && (pStackEntryInfoHandler->GetLineNumber() != (LineNum)(-1)) && (!pStackEntryInfoHandler->GetFileNameString()))
        {
            pEntry = new SymbolInfo(QString::fromStdString(pSymbolFileEntry->GetSymbolApiName()),
                                    QString::fromStdString(pStackEntryInfoHandler->GetSymbolNameString()),
                                    QString::fromStdString(pStackEntryInfoHandler->GetFileNameString()),
                                    pStackEntryInfoHandler->GetLineNumber());
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

void gpTraceDataParser::OnParse(IPerfMarkerInfoDataHandler* pPerfMarkerEntry, bool& stopParsing)
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

            void* pPtr;

            if (!AtpUtils::Instance()->IsModuleLoaded())
            {
                AtpUtils::Instance()->LoadModule();
            }

            AtpDataHandlerFunc pAtpDataHandler_func = AtpUtils::Instance()->GetAtpDataHandlerFunc();
            IOccupancyFileInfoDataHandler* occupancyFileDataInfo = nullptr;
            if (nullptr != pAtpDataHandler_func)
            {
                pAtpDataHandler_func(&pPtr);
                IAtpDataHandler* pAtpDataHandler = reinterpret_cast<IAtpDataHandler*>(pPtr);
                std::string occupancyFile = m_traceFilePath.asString().asASCIICharArray();
                occupancyFileDataInfo = pAtpDataHandler->GetOccupancyFileInfoDataHandler(occupancyFile.c_str());
                m_isOccupancyFileLoaded = occupancyFileDataInfo->ParseOccupancyFile(occupancyFile.c_str());
            }

            if (m_isOccupancyFileLoaded)
            {
                osThreadId* osThreadIds;
                unsigned int threadCount;
                occupancyFileDataInfo->GetOccupancyThreads(&osThreadIds, threadCount);

                for (unsigned int i = 0; i < threadCount; i++)
                {
                    const IOccupancyInfoDataHandler* occupancyInfo;
                    unsigned int kernelCount;
                    occupancyFileDataInfo->GetKernelCountByThreadId(osThreadIds[i], kernelCount);
                    QList<const IOccupancyInfoDataHandler*> occupancyInfoList;

                    for (unsigned int j = 0; j < kernelCount; j++)
                    {
                        occupancyInfo = occupancyFileDataInfo->GetOccupancyInfoDataHandler(osThreadIds[i], j);
                        occupancyInfoList.push_back(occupancyInfo);
                    }

                    m_pSessionDataContainer->m_occupancyInfoMap.insert(osThreadIds[i], occupancyInfoList);
                }
            }

            retVal = m_isOccupancyFileLoaded;

        }
    }

    return retVal;

}



