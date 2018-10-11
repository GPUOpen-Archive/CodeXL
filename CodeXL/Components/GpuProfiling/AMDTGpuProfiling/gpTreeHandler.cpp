//=============================================================
// Copyright (c) 2015-2018 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file
/// \brief GPU Profiler Tree Handler
//=============================================================

#include <qtIgnoreCompilerWarnings.h>

// Infra:
#include <AMDTOSWrappers/Include/osDirectorySerializer.h>

// infra
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTApplicationComponents/Include/acIcons.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>

// AMDTApplicationFramework
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTree.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/Include/afPluginConnectionManager.h>
#include <AMDTSharedProfiling/inc/StringConstants.h>

// Local:
#include <AMDTGpuProfiling/gpTreeHandler.h>
#include <AMDTGpuProfiling/Session.h>
#include <AMDTGpuProfiling/gpStringConstants.h>
#include <AMDTGpuProfiling/ProfileManager.h>

#define GP_PROGRESS_COEF 10

/// compare directory based on session index
/// \param sessionOneDir first directory
/// \param sessionTwoDir second directory
/// \return True if first one is having less valued index
bool CompareDirOnDate(QFileInfo sessionOneDir, QFileInfo sessionTwoDir)
{
    return sessionOneDir.created() < sessionTwoDir.created();
}

gpTreeHandler* gpTreeHandler::m_psMySingleInstance = NULL;

gpTreeHandler::gpTreeHandler()
{
}

gpTreeHandler::~gpTreeHandler()
{

}


gpTreeHandler& gpTreeHandler::Instance()
{
    // If my single instance was not created yet - create it:
    if (nullptr == m_psMySingleInstance)
    {
        m_psMySingleInstance = new gpTreeHandler;

    }

    return *m_psMySingleInstance;
}


bool gpTreeHandler::BuildSessionTree(const SessionTreeNodeData& sessionTreeNodeData, QTreeWidgetItem* pTreeItem)
{
    (void)(sessionTreeNodeData); // unused
    bool retVal = false;
    GT_IF_WITH_ASSERT(pTreeItem != nullptr)
    {
        afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
        GT_IF_WITH_ASSERT(pApplicationCommands != nullptr)
        {
            afApplicationTree* pTree = pApplicationCommands->applicationTree();
            GT_IF_WITH_ASSERT(pTree != nullptr)
            {
                afApplicationTreeItemData* pItemData = pTree->getTreeItemData(pTreeItem);
                GT_IF_WITH_ASSERT(pItemData != nullptr)
                {
                    // Check if this is an application trace session
                    TraceSession* pGPUTreeItemData = qobject_cast<TraceSession*>(pItemData->extendedItemData());

                    if (pGPUTreeItemData != nullptr)
                    {
                        BuildApplicationTraceSessionTree(pTreeItem, pGPUTreeItemData);
                    }
                }
            }
        }
    }

    return retVal;
}

bool gpTreeHandler::ExtendSessionHTMLPropeties(afTreeItemType sessionTreeItemType, const SessionTreeNodeData* pSessionData, afHTMLContent& htmlContent)
{
    GT_UNREFERENCED_PARAMETER(htmlContent);
    bool retVal = false;

    // Check if this is a session of a session child:
    bool isItemSessionChild = ((sessionTreeItemType == AF_TREE_ITEM_PROFILE_SESSION) || DoesTreeItemTypeBelongsToHandler(sessionTreeItemType));

    // populate the Properties view with information from the selected session in the GPUSessionTreeItemData Explorer
    const GPUSessionTreeItemData* pGPUSessionData = qobject_cast<const GPUSessionTreeItemData*>(pSessionData);

    if (pGPUSessionData != nullptr && isItemSessionChild)
    {
        retVal = true;
    }

    return retVal;
}

void gpTreeHandler::InitializeProfileIcons()
{
    // Create the icons:
    AddTreeIconToMap(AC_ICON_PROFILE_GPU_APPTREE_COUNTERS_SINGLE);
    AddTreeIconToMap(AC_ICON_PROFILE_GPU_APPTREE_APPTRACE_SINGLE);
    AddTreeIconToMap(AC_ICON_PROFILE_GPU_APPTREE_COUNTERS_MULTI);
    AddTreeIconToMap(AC_ICON_PROFILE_GPU_APPTREE_APPTRACE_MULTI);
    AddTreeIconToMap(AC_ICON_PROFILE_GPU_APPTREE_SUMMARY);
    AddTreeIconToMap(AC_ICON_PROFILE_GPU_APPTREE_CL_API);
    AddTreeIconToMap(AC_ICON_PROFILE_GPU_APPTREE_HSA_API);
    AddTreeIconToMap(AC_ICON_PROFILE_GPU_APPTREE_CONTEXT);
    AddTreeIconToMap(AC_ICON_PROFILE_GPU_APPTREE_KERNEL);
    AddTreeIconToMap(AC_ICON_PROFILE_GPU_APPTREE_TOP10_TRANSFER);
    AddTreeIconToMap(AC_ICON_PROFILE_GPU_APPTREE_TOP10_KERNEL);
    AddTreeIconToMap(AC_ICON_PROFILE_GPU_APPTREE_BESTPRACTICE);
}

QPixmap* gpTreeHandler::TreeItemTypeToPixmap(afTreeItemType itemType, const QString& sessionTypeAsStr)
{
    QPixmap* pRetPixmap = nullptr;

    acIconId iconId = AC_ICON_EMPTY;
    GPUProfileType profileType = NA_PROFILE_TYPE;

    if ((AF_TREE_ITEM_PROFILE_SESSION == itemType) || (AF_TREE_ITEM_PROFILE_SESSION_TYPE == itemType))
    {
        if (GPU_str_profileTypePerformanceCounters == sessionTypeAsStr)
        {
            profileType = PERFORMANCE;
        }
        else if (GPU_str_profileTypeApplicationTrace == sessionTypeAsStr)
        {
            profileType = API_TRACE;
        }
    }

    bool isHSA = (GP_Str_HSA == sessionTypeAsStr);

    switch (itemType)
    {
        case AF_TREE_ITEM_PROFILE_SESSION:
        {
            switch (profileType)
            {
                // Perf counters
                case PERFORMANCE:
                    iconId = AC_ICON_PROFILE_GPU_APPTREE_COUNTERS_SINGLE;
                    break;

                // Application trace
                case API_TRACE:
                    iconId = AC_ICON_PROFILE_GPU_APPTREE_APPTRACE_SINGLE;
                    break;

                default:
                    break;
            }
        }
        break;

        case AF_TREE_ITEM_PROFILE_SESSION_TYPE:
        {
            switch (profileType)
            {
                case PERFORMANCE:
                    iconId = AC_ICON_PROFILE_GPU_APPTREE_COUNTERS_MULTI;
                    break;

                case API_TRACE:
                    iconId = AC_ICON_PROFILE_GPU_APPTREE_APPTRACE_MULTI;
                    break;

                default:
                    break;
            }
        }
        break;

        case AF_TREE_ITEM_PROFILE_GPU_SUMMARY:
            iconId = AC_ICON_PROFILE_GPU_APPTREE_SUMMARY;
            break;

        case AF_TREE_ITEM_PROFILE_GPU_API_SUMMARY:
            iconId = isHSA ? AC_ICON_PROFILE_GPU_APPTREE_HSA_API : AC_ICON_PROFILE_GPU_APPTREE_CL_API;
            break;

        case AF_TREE_ITEM_PROFILE_GPU_CONTEXT_SUMMARY:
            iconId = AC_ICON_PROFILE_GPU_APPTREE_CONTEXT;
            break;

        case AF_TREE_ITEM_PROFILE_KERNEL_SUMMARY:
            iconId = AC_ICON_PROFILE_GPU_APPTREE_KERNEL;
            break;

        case AF_TREE_ITEM_PROFILE_GPU_TOP10_TRANSFER_SUMMARY:
            iconId = AC_ICON_PROFILE_GPU_APPTREE_TOP10_TRANSFER;
            break;

        case AF_TREE_ITEM_PROFILE_GPU_TOP10_KERNEL_SUMMARY:
            iconId = AC_ICON_PROFILE_GPU_APPTREE_TOP10_KERNEL;
            break;

        case AF_TREE_ITEM_PROFILE_GPU_BEST_PRACTICE_SUMMARY:
            iconId = AC_ICON_PROFILE_GPU_APPTREE_BESTPRACTICE;
            break;

        default:
            break;
    }

    pRetPixmap = m_iconsMap[iconId];

    return pRetPixmap;
}

bool gpTreeHandler::DoesSummaryPagesExist(const SessionTreeNodeData* pGPUTreeItemData)
{
    // Default should be true:
    bool retVal = true;

    // Sanity check:
    GT_IF_WITH_ASSERT(pGPUTreeItemData != nullptr)
    {
        // Find all the html files named "*Summary.html"
        gtList<osFilePath> filePathsList;
        bool rc = pGPUTreeItemData->SessionDir().getContainedFilePaths(GPU_SummaryFileName, osDirectory::SORT_BY_NAME_ASCENDING, filePathsList);
        GT_IF_WITH_ASSERT(rc)
        {
            retVal = !filePathsList.empty();
        }
    }

    return retVal;
}

bool gpTreeHandler::DoesSingleSummaryPageExist(const SessionTreeNodeData* pGPUTreeItemData, const gtString& fileName)
{
    // Default should be true:
    bool retVal = true;

    // Sanity check:
    GT_IF_WITH_ASSERT(pGPUTreeItemData != nullptr)
    {
        // Find all the html files named "*Summary.html"
        gtList<osFilePath> filePathsList;
        bool rc = pGPUTreeItemData->SessionDir().getContainedFilePaths(fileName, osDirectory::SORT_BY_NAME_ASCENDING, filePathsList);
        GT_IF_WITH_ASSERT(rc)
        {
            retVal = !filePathsList.empty();
        }
    }

    return retVal;
}

bool gpTreeHandler::GetProfileTypeWithPrefix(const QString& sessionTypeAsStr, gtString& sessionTypeWithPrefix)
{
    bool retVal = false;

    if (GPU_str_profileTypePerformanceCounters == sessionTypeAsStr)
    {
        sessionTypeWithPrefix.prepend(GP_profileTypePerformanceCountersPrefix);
        retVal = true;
    }

    return retVal;
}

bool gpTreeHandler::DoesTreeItemTypeBelongsToHandler(const afTreeItemType itemType) const
{
    bool retVal = false;

    retVal = (
                 (itemType == AF_TREE_ITEM_PROFILE_GPU_API_SUMMARY) ||
                 (itemType == AF_TREE_ITEM_PROFILE_GPU_CONTEXT_SUMMARY) ||
                 (itemType == AF_TREE_ITEM_PROFILE_KERNEL_SUMMARY) ||
                 (itemType == AF_TREE_ITEM_PROFILE_GPU_TOP10_TRANSFER_SUMMARY) ||
                 (itemType == AF_TREE_ITEM_PROFILE_GPU_TOP10_KERNEL_SUMMARY) ||
                 (itemType == AF_TREE_ITEM_PROFILE_GPU_BEST_PRACTICE_SUMMARY) ||
                 (itemType == AF_TREE_ITEM_PROFILE_GPU_KERNELS));

    return retVal;
}

bool gpTreeHandler::DoesTreeNodeDataBelongsToHandler(const SessionTreeNodeData* pSessionData) const
{
    bool retVal = false;

    if (QString(GP_profileTypePerformanceCountersWithPrefix).endsWith(pSessionData->m_profileTypeStr) ||
        QString(GP_profileTypeApplicationTraceWithPrefix).endsWith(pSessionData->m_profileTypeStr))
    {
        retVal = true;
    }

    return retVal;
}

void gpTreeHandler::BuildApplicationTraceSessionTree(QTreeWidgetItem* pTreeItem, TraceSession* pGPUTreeItemData)
{
    // Sanity check:
    afApplicationTree* pTree = afApplicationCommands::instance()->applicationTree();
    GT_IF_WITH_ASSERT((pGPUTreeItemData != nullptr) && (pGPUTreeItemData->m_pParentData != nullptr) && (pTree != nullptr))
    {
        // Check if the session folder contain summary pages (if not, no need to add the summary nodes to the tree)
        bool doesSummaryExist = DoesSummaryPagesExist(pGPUTreeItemData);

        if (doesSummaryExist)
        {
            // Add the API summary tree item
            AddSummaryFileTreeItem(pGPUTreeItemData, pTreeItem, GPU_APISummaryFileName, AF_TREE_ITEM_PROFILE_GPU_API_SUMMARY);

            // Add the context summary tree item
            AddSummaryFileTreeItem(pGPUTreeItemData, pTreeItem, GPU_ContextSummaryFileName, AF_TREE_ITEM_PROFILE_GPU_CONTEXT_SUMMARY);

            // Add the kernel summary tree item
            AddSummaryFileTreeItem(pGPUTreeItemData, pTreeItem, GPU_KernelSummaryFileName, AF_TREE_ITEM_PROFILE_KERNEL_SUMMARY);

            // Add the top 10 transfer summary tree item
            AddSummaryFileTreeItem(pGPUTreeItemData, pTreeItem, GPU_Top10DataTransferSummaryFileName, AF_TREE_ITEM_PROFILE_GPU_TOP10_TRANSFER_SUMMARY);

            // Add the top 10 kernel summary tree item
            AddSummaryFileTreeItem(pGPUTreeItemData, pTreeItem, GPU_Top10KernelSummaryFileName, AF_TREE_ITEM_PROFILE_GPU_TOP10_KERNEL_SUMMARY);

            // Add the best practices tree item
            AddSummaryFileTreeItem(pGPUTreeItemData, pTreeItem, GPU_BestPracticesFileName, AF_TREE_ITEM_PROFILE_GPU_BEST_PRACTICE_SUMMARY);
        }
    }
}

void gpTreeHandler::AddSummaryFileTreeItem(TraceSession* pGPUTreeItemData, QTreeWidgetItem* pParent, const gtString& summaryFileName, afTreeItemType summaryItemType)
{
    QTreeWidgetItem* pItem = nullptr;
    afApplicationTree* pTree = afApplicationCommands::instance()->applicationTree();
    ProfileApplicationTreeHandler* pTheAppTreeHandler = ProfileApplicationTreeHandler::instance();

    // Sanity check:
    GT_IF_WITH_ASSERT((pTree != nullptr) && (pGPUTreeItemData != nullptr) && (pParent != nullptr) && (pTheAppTreeHandler != nullptr))
    {
        bool doesHTMLFileExist = DoesSingleSummaryPageExist(pGPUTreeItemData, summaryFileName);

        if (doesHTMLFileExist)
        {
            afApplicationTreeItemData* pSummaryData = new afApplicationTreeItemData(true);
            pSummaryData->m_filePath = pGPUTreeItemData->m_pParentData->m_filePath;
            pSummaryData->m_itemType = summaryItemType;
            pSummaryData->m_filePathLineNumber = summaryItemType;
            pSummaryData->setExtendedData(pGPUTreeItemData);
            pItem = pTree->addTreeItem(Util::SummaryTypeToGTString(summaryItemType), pSummaryData, pParent);
            GT_IF_WITH_ASSERT(pItem != nullptr)
            {
                bool isHSASession = DoesSingleSummaryPageExist(pGPUTreeItemData, GPU_HSAAPISummaryFileName);
                bool isCLSession = DoesSingleSummaryPageExist(pGPUTreeItemData, GPU_CLAPISummaryFileName);
                GT_ASSERT(isHSASession || isCLSession);
                QPixmap* pAPIIcon = pTheAppTreeHandler->TreeItemTypeToPixmap(pSummaryData->m_itemType, isHSASession ? GP_Str_HSA : GP_Str_CL);
                GT_IF_WITH_ASSERT(pAPIIcon != nullptr)
                {
                    pItem->setIcon(0, QIcon(*pAPIIcon));
                }
            }
        }
    }
}
