//------------------------------ gpTreeHandler.cpp ------------------------------

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
#include <AMDTGpuProfiling/gpUIManager.h>
#include <AMDTGpuProfiling/Session.h>
#include <AMDTGpuProfiling/gpStringConstants.h>
#include <AMDTGpuProfiling/gpExecutionMode.h>
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

                    // Check if this is a DX profile session
                    gpSessionTreeNodeData* pDXSessionDataItemData = qobject_cast<gpSessionTreeNodeData*>(pItemData->extendedItemData());

                    if (pDXSessionDataItemData != nullptr)
                    {
                        BuildFrameAnalysisSessionTree(pTreeItem, pDXSessionDataItemData);
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
    AddTreeIconToMap(AC_ICON_FRAME_ANALYSIS_APPTREE_SINGLE);
    AddTreeIconToMap(AC_ICON_PROFILE_GPU_APPTREE_COUNTERS_MULTI);
    AddTreeIconToMap(AC_ICON_PROFILE_GPU_APPTREE_APPTRACE_MULTI);
    AddTreeIconToMap(AC_ICON_FRAME_ANALYSIS_APPTREE_MULTI);
    AddTreeIconToMap(AC_ICON_PROFILE_GPU_APPTREE_SUMMARY);
    AddTreeIconToMap(AC_ICON_PROFILE_GPU_APPTREE_CL_API);
    AddTreeIconToMap(AC_ICON_PROFILE_GPU_APPTREE_HSA_API);
    AddTreeIconToMap(AC_ICON_PROFILE_GPU_APPTREE_CONTEXT);
    AddTreeIconToMap(AC_ICON_PROFILE_GPU_APPTREE_KERNEL);
    AddTreeIconToMap(AC_ICON_PROFILE_GPU_APPTREE_TOP10_TRANSFER);
    AddTreeIconToMap(AC_ICON_PROFILE_GPU_APPTREE_TOP10_KERNEL);
    AddTreeIconToMap(AC_ICON_PROFILE_GPU_APPTREE_BESTPRACTICE);
    AddTreeIconToMap(AC_ICON_FRAME_ANALYSIS_APP_TREE_DASHBOARD);
    AddTreeIconToMap(AC_ICON_FRAME_ANALYSIS_APP_TREE_FRAME);
    AddTreeIconToMap(AC_ICON_FRAME_ANALYSIS_APP_TREE_OVERVIEW);
    AddTreeIconToMap(AC_ICON_FRAME_ANALYSIS_APP_TREE_TIMELINE);
    AddTreeIconToMap(AC_ICON_FRAME_ANALYSIS_APP_TREE_PERFORMANCE_PROFILE);
    AddTreeIconToMap(AC_ICON_FRAME_ANALYSIS_APP_TREE_IMAGE);
#ifdef GP_OBJECT_VIEW_ENABLE
    AddTreeIconToMap(AC_ICON_FRAME_ANALYSIS_APP_TREE_OBJECTINSPCTOR);
#endif
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
        else if (PM_profileTypeFrameAnalysis == sessionTypeAsStr)
        {
            profileType = FRAME_ANALYSIS;
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

                // Frame analysis Application trace
                case FRAME_ANALYSIS:
                    iconId = AC_ICON_FRAME_ANALYSIS_APPTREE_SINGLE;
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

                case FRAME_ANALYSIS:
                    iconId = AC_ICON_FRAME_ANALYSIS_APPTREE_MULTI;
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

        case AF_TREE_ITEM_GP_FRAME:
            iconId = AC_ICON_FRAME_ANALYSIS_APP_TREE_FRAME;
            break;

        case AF_TREE_ITEM_GP_FRAME_OVERVIEW:
            iconId = AC_ICON_FRAME_ANALYSIS_APP_TREE_OVERVIEW;
            break;

        case AF_TREE_ITEM_GP_FRAME_TIMELINE:
            iconId = AC_ICON_FRAME_ANALYSIS_APP_TREE_TIMELINE;
            break;

#ifdef GP_OBJECT_VIEW_ENABLE

        case AF_TREE_ITEM_GP_FRAME_OBJECTINSPCTOR:
            iconId = AC_ICON_FRAME_ANALYSIS_APP_TREE_OBJECTINSPCTOR;
            break;
#endif

        case AF_TREE_ITEM_GP_FRAME_PERFORMANCE_PROFILES:
            iconId = AC_ICON_FRAME_ANALYSIS_APP_TREE_PERFORMANCE_PROFILE;
            break;

        case AF_TREE_ITEM_GP_FRAME_IMAGE:
            iconId = AC_ICON_FRAME_ANALYSIS_APP_TREE_IMAGE;
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
                 (itemType == AF_TREE_ITEM_PROFILE_GPU_KERNELS) ||
                 (itemType == AF_TREE_ITEM_GP_FRAME) ||
                 (itemType == AF_TREE_ITEM_GP_FRAME_OVERVIEW) ||
                 (itemType == AF_TREE_ITEM_GP_FRAME_TIMELINE) ||
                 (itemType == AF_TREE_ITEM_GP_FRAME_PERFORMANCE_PROFILES) ||
                 (itemType == AF_TREE_ITEM_GP_FRAME_PERFORMANCE_PROFILE) ||
                 (itemType == AF_TREE_ITEM_GP_FRAME_IMAGE));

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

void gpTreeHandler::BuildFrameAnalysisSessionTree(QTreeWidgetItem* pTreeItem, gpSessionTreeNodeData* pSessionData)
{
    GT_UNREFERENCED_PARAMETER(pTreeItem);

    // Sanity check:
    afApplicationTree* pTree = afApplicationCommands::instance()->applicationTree();
    GT_IF_WITH_ASSERT((pSessionData != nullptr) && (pSessionData->m_pParentData != nullptr) && (pTree != nullptr))
    {
        afApplicationTreeItemData* pItemData = pSessionData->m_pParentData;
        GT_IF_WITH_ASSERT(pItemData != nullptr)
        {
            // Find all the frames captured for this session
            QList<int> framesIndices;
            gpUIManager::Instance()->GetListOfFrameFolders(pItemData->m_filePath, framesIndices);

            foreach (int frameIndex, framesIndices)
            {
                // Build the frame index tree item
                GetFrameTreeItem(pSessionData, frameIndex);
            }
        }
    }
}


QTreeWidgetItem* gpTreeHandler::GetFrameTreeItem(gpSessionTreeNodeData* pSessionData, int frameIndex)
{
    QTreeWidgetItem* pRetVal = nullptr;

    afApplicationTree* pTree = afApplicationCommands::instance()->applicationTree();
    // Sanity check:
    GT_IF_WITH_ASSERT((pSessionData != nullptr) && (pSessionData->m_pParentData != nullptr) && (pTree != nullptr))
    {
        // Build the frame node string
        QLocale locale(QLocale::English);
        QString frameIndexStr = locale.toString((qlonglong)frameIndex);
        QString frameNodeName = QString(GPU_STR_TreeNodeFrame).arg(frameIndexStr);

        // Get the current frame node
        afApplicationTreeItemData* pItemData = pSessionData->m_pParentData;

        // Sanity check:
        GT_IF_WITH_ASSERT(pItemData->m_pTreeWidgetItem != nullptr)
        {
            if (pItemData->m_pTreeWidgetItem->text(0) == frameNodeName)
            {
                pRetVal = pItemData->m_pTreeWidgetItem;
            }
            else
            {
                for (int i = 0; i < pItemData->m_pTreeWidgetItem->childCount(); i++)
                {
                    QTreeWidgetItem* pChild = pItemData->m_pTreeWidgetItem->child(i);

                    if (pChild != nullptr)
                    {
                        if (pChild->text(0) == frameNodeName)
                        {
                            pRetVal = pChild;
                            break;
                        }
                    }
                }
            }

            // Frame node was not yet created
            if (pRetVal == nullptr)
            {
                pRetVal = AddTreeItem(pSessionData, pItemData->m_pTreeWidgetItem, acQStringToGTString(frameNodeName), AF_TREE_ITEM_GP_FRAME, frameIndex);

                // Sanity check:
                GT_IF_WITH_ASSERT(pRetVal != nullptr)
                {
#ifdef INCLUDE_FRAME_ANALYSIS_PERFORMANCE_COUNTERS
                    // Add the overview node
                    AddTreeItem(pSessionData, pRetVal, GPU_STR_TreeNodeOverview, AF_TREE_ITEM_GP_FRAME_OVERVIEW, frameIndex, true);
#endif

                    // Add the timeline node
                    AddTreeItem(pSessionData, pRetVal, GPU_STR_TreeNodeTimeline, AF_TREE_ITEM_GP_FRAME_TIMELINE, frameIndex);

#ifdef INCLUDE_FRAME_ANALYSIS_PERFORMANCE_COUNTERS
                    // Add the performance profile node
                    AddTreeItem(pSessionData, pRetVal, GPU_STR_TreeNodePerformanceProfile, AF_TREE_ITEM_GP_FRAME_PERFORMANCE_PROFILES, frameIndex);

                    // Add the existing performance counters files to the tree node
                    AddExistingFileToPerformanceCountersNode(pSessionData, pRetVal, frameIndex);
#endif

                    // Add the image node
                    osFilePath filePath = BuildFrameChildFilePath(pSessionData, AF_TREE_ITEM_GP_FRAME_IMAGE, frameIndex);

                    if (filePath.exists())
                    {
                        AddTreeItem(pSessionData, pRetVal, GPU_STR_TreeNodeImage, AF_TREE_ITEM_GP_FRAME_IMAGE, frameIndex);
                    }

#ifdef GP_OBJECT_VIEW_ENABLE
                    // Add the Object Inspector node, set file info, for Object tree
                    AddTreeItem(pSessionData, pRetVal, GPU_STR_TreeNodeObjectInspector, AF_TREE_ITEM_GP_FRAME_OBJECTINSPCTOR, frameIndex);
#endif
                }
            }
        }
    }

    return pRetVal;
}

bool gpTreeHandler::UpdateFrameFilePath(gpSessionTreeNodeData* pSessionData, int frameIndex, const osFilePath& ltrFilePath)
{
    bool retVal = false;

    // Sanity check
    afApplicationTree* pTree = afApplicationCommands::instance()->applicationTree();
    GT_IF_WITH_ASSERT((pSessionData != nullptr) && (pTree != nullptr))
    {
        QTreeWidgetItem* pFrameItem = GetFrameTreeItem(pSessionData, frameIndex);

        if (pFrameItem != nullptr)
        {
            afApplicationTreeItemData* pFrameItemData = pTree->getTreeItemData(pFrameItem);

            if (pFrameItemData != nullptr)
            {
                pFrameItemData->m_filePath = ltrFilePath;
                gpSessionTreeNodeData* pGPData = qobject_cast<gpSessionTreeNodeData*>(pFrameItemData->extendedItemData());

                if (pGPData != nullptr)
                {
                    pGPData->m_frameFilePath = ltrFilePath;
                }

                // Get the timeline item data
                afApplicationTreeItemData* pTimelineItemData = GetFrameChildItemData(pFrameItem, AF_TREE_ITEM_GP_FRAME_TIMELINE);

                if (pTimelineItemData != nullptr)
                {
                    pTimelineItemData->m_filePath = ltrFilePath;
                    pGPData = qobject_cast<gpSessionTreeNodeData*>(pTimelineItemData->extendedItemData());

                    if (pGPData != nullptr)
                    {
                        pGPData->m_frameFilePath = ltrFilePath;
                    }
                }

                // Get the timeline item data
                afApplicationTreeItemData* pImageItemData = GetFrameChildItemData(pFrameItem, AF_TREE_ITEM_GP_FRAME_IMAGE);

                if (pImageItemData != nullptr)
                {
                    osFilePath imageFilePath = ltrFilePath;
                    imageFilePath.setFileExtension(GP_ThumbnailFileExtensionW);
                    pImageItemData->m_filePath = imageFilePath;
                    pGPData = qobject_cast<gpSessionTreeNodeData*>(pImageItemData->extendedItemData());

                    if (pGPData != nullptr)
                    {
                        pGPData->m_frameFilePath = imageFilePath;
                    }
                }

                retVal = true;
            }
        }
    }
    return retVal;
}

QTreeWidgetItem* gpTreeHandler::AddTreeItem(gpSessionTreeNodeData* pSessionData, QTreeWidgetItem* pParent,
                                            const gtString& nodeName, afTreeItemType itemType, int frameIndex, bool shouldCreateFile)
{
    QTreeWidgetItem* pRetVal = nullptr;

    // Sanity check:
    afApplicationTree* pTree = afApplicationCommands::instance()->applicationTree();
    GT_IF_WITH_ASSERT((pParent != nullptr) && (pSessionData != nullptr) && (pSessionData->m_pParentData != nullptr) && (pTree != nullptr))
    {
        // Add the frame node
        afApplicationTreeItemData* pFrameData = new afApplicationTreeItemData(true);
        pFrameData->m_itemType = itemType;
        pFrameData->m_filePathLineNumber = itemType;

        gpSessionTreeNodeData* pNewData = new gpSessionTreeNodeData(*pSessionData);
        pFrameData->setExtendedData(pNewData);

        // Set the frame index
        pNewData->m_frameIndex = frameIndex;

        // For frame item, build the file path, and for the frame children items, set the frame file path
        // The file path represents only separated MDI window. Frame children are opened as inner tab view in the frame view,
        // and it's path is set in the gpSessionTreeNodeData
#ifdef INCLUDE_FRAME_ANALYSIS_PERFORMANCE_COUNTERS
        pFrameData->m_filePath = BuildFrameChildFilePath(pSessionData, AF_TREE_ITEM_GP_FRAME_OVERVIEW, frameIndex);
#else
        pFrameData->m_filePath = BuildFrameChildFilePath(pSessionData, itemType, frameIndex);
#endif

        if ((itemType != AF_TREE_ITEM_GP_FRAME_OVERVIEW) && (itemType != AF_TREE_ITEM_GP_FRAME) && (itemType != AF_TREE_ITEM_GP_FRAME_PERFORMANCE_PROFILES))
        {
            pNewData->m_frameFilePath = BuildFrameChildFilePath(pSessionData, itemType, frameIndex);
        }

#ifdef GP_OBJECT_VIEW_ENABLE

        if (itemType == AF_TREE_ITEM_GP_FRAME_OBJECTINSPCTOR)
        {
            pNewData->m_frameFilePath = BuildFrameChildFilePath(pSessionData, itemType, frameIndex);
        }

#endif

        pRetVal = pTree->addTreeItem(nodeName, pFrameData, pParent);
        GT_IF_WITH_ASSERT(pRetVal != nullptr)
        {
            QPixmap* pAPIIcon = ProfileApplicationTreeHandler::instance()->TreeItemTypeToPixmap(itemType);
            GT_IF_WITH_ASSERT(pAPIIcon != nullptr)
            {
                pRetVal->setIcon(0, QIcon(*pAPIIcon));
            }

            if (shouldCreateFile)
            {
                // Create the file if it doesn't exist
                if (!pFrameData->m_filePath.exists())
                {
                    QFile outFile(acGTStringToQString(pFrameData->m_filePath.asString()));
                    outFile.open(QIODevice::WriteOnly | QIODevice::Text);
                    outFile.close();
                }
            }
        }

    }

    return pRetVal;
}

osFilePath gpTreeHandler::BuildFrameChildFilePath(gpSessionTreeNodeData* pSessionData, afTreeItemType childType, int frameIndex)
{
    osFilePath retVal;

    // Sanity check:
    GT_IF_WITH_ASSERT((pSessionData != nullptr) && (pSessionData->m_pParentData != nullptr))
    {
        gtString sessionDir = pSessionData->SessionDir().directoryPath().asString();
        sessionDir.append(osFilePath::osPathSeparator);

        // If the pSessionData is in session level, the frame folder should be added
        if (pSessionData->m_pParentData->m_itemType == AF_TREE_ITEM_PROFILE_SESSION)
        {
            sessionDir.append(acQStringToGTString(QString(GPU_STR_FrameSubFolderNameFormatA).arg(frameIndex)));
            sessionDir.append(osFilePath::osPathSeparator);
        }

        retVal.setFileDirectory(sessionDir);
        gtString fileName = acQStringToGTString(QString(GPU_STR_FrameTraceFileNameFormat).arg(pSessionData->m_displayName).arg(frameIndex));
        retVal.setFileName(fileName);

        switch (childType)
        {

            case AF_TREE_ITEM_GP_FRAME:
#ifdef INCLUDE_FRAME_ANALYSIS_PERFORMANCE_COUNTERS
                retVal.setFileExtension(GP_Overview_FileExtensionW);
#else
                retVal.setFileExtension(GP_LTR_FileExtensionW);
#endif
                break;

            case AF_TREE_ITEM_GP_FRAME_OVERVIEW:
                retVal.setFileExtension(GP_Overview_FileExtensionW);
                break;

            case AF_TREE_ITEM_GP_FRAME_TIMELINE:
                retVal.setFileExtension(GP_LTR_FileExtensionW);
                break;

            case AF_TREE_ITEM_GP_FRAME_PERFORMANCE_PROFILES:
            case AF_TREE_ITEM_GP_FRAME_PERFORMANCE_PROFILE:
                retVal.setFileExtension(GP_PerformanceCounters_FileExtensionW);
                break;

            case AF_TREE_ITEM_GP_FRAME_IMAGE:
                retVal.setFileExtension(GP_ThumbnailFileExtensionW);
                break;

#ifdef GP_OBJECT_VIEW_ENABLE

            case AF_TREE_ITEM_GP_FRAME_OBJECTINSPCTOR:
                retVal.setFileExtension(GP_AOR_FileExtensionW);
                break;
#endif

            default:
                retVal = pSessionData->m_pParentData->m_filePath;
                break;
        }
    }

    return retVal;
}


osFilePath gpTreeHandler::GetFrameChildFilePath(const osFilePath& sessionFilePath, int frameIndex, afTreeItemType childType)
{
    osFilePath retVal;

    // Get the session item data
    SessionTreeNodeData* pData = ProfileApplicationTreeHandler::instance()->FindSessionDataByProfileFilePath(sessionFilePath);
    gpSessionTreeNodeData* pSessionTreeNodeData = qobject_cast<gpSessionTreeNodeData*>(pData);
    GT_IF_WITH_ASSERT((pSessionTreeNodeData != nullptr) && (afApplicationCommands::instance()->applicationTree() != nullptr))
    {
        // Get the frame tree item
        QTreeWidgetItem* pFrameItem = GetFrameTreeItem(pSessionTreeNodeData, frameIndex);
        GT_IF_WITH_ASSERT(pFrameItem != nullptr)
        {
            // Go over the frame item tree children and find the requested one
            int frameChildCount = pFrameItem->childCount();

            for (int i = 0; i < frameChildCount; i++)
            {
                afApplicationTreeItemData* pChildData = afApplicationCommands::instance()->applicationTree()->GetChildItemData(pFrameItem, i);

                if (pChildData != nullptr)
                {
                    if (pChildData->m_itemType == childType)
                    {
                        gpSessionTreeNodeData* pChildGPData = qobject_cast<gpSessionTreeNodeData*>(pChildData->extendedItemData());

                        if (pChildGPData != nullptr)
                        {
                            retVal = pChildGPData->m_frameFilePath;
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

bool gpTreeHandler::GetOwningFrameFilePath(const osFilePath& frameChildFilePath, osFilePath& frameFilePath)
{
    bool retVal = false;

    if ((afApplicationCommands::instance() != nullptr) && (afApplicationCommands::instance()->applicationTree() != nullptr))
    {
        afApplicationTreeItemData* pChildItemData = afApplicationCommands::instance()->applicationTree()->FindItemByFilePath(frameChildFilePath);

        if (pChildItemData != nullptr)
        {
            QTreeWidgetItem* pFrameItem = afApplicationCommands::instance()->applicationTree()->getTreeItemParent(pChildItemData->m_pTreeWidgetItem);
            afApplicationTreeItemData* pFrameItemData = afApplicationCommands::instance()->applicationTree()->getTreeItemData(pFrameItem);

            if (pFrameItemData != nullptr)
            {
                if (pFrameItemData->m_itemType == AF_TREE_ITEM_GP_FRAME)
                {
                    frameFilePath = pFrameItemData->m_filePath;
                    retVal = true;
                }
                else
                {
                    pFrameItem = afApplicationCommands::instance()->applicationTree()->getTreeItemParent(pFrameItem);
                    pFrameItemData = afApplicationCommands::instance()->applicationTree()->getTreeItemData(pFrameItem);

                    if ((pFrameItemData != nullptr)  && (pFrameItemData->m_itemType == AF_TREE_ITEM_GP_FRAME))
                    {
                        frameFilePath = pFrameItemData->m_filePath;
                        retVal = true;
                    }
                }
            }
        }
    }

    return retVal;
}

void gpTreeHandler::AddExistingFileToPerformanceCountersNode(gpSessionTreeNodeData* pSessionData, QTreeWidgetItem* pFrameTreeItem, int frameIndex)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((pSessionData != nullptr) && (pFrameTreeItem != nullptr))
    {
        // Get the performance counters frame child
        QTreeWidgetItem* pPerformanceCountersItem = pFrameTreeItem->child(AF_TREE_ITEM_GP_FRAME_PERFORMANCE_PROFILES - AF_TREE_ITEM_GP_FRAME_FIRST_CHILD);

        // Sanity check:
        GT_IF_WITH_ASSERT(pPerformanceCountersItem != nullptr)
        {
            // Find the frame sub directory
            osFilePath frameDir = pSessionData->SessionDir().directoryPath();
            gtString frameSubFolder;
            frameSubFolder.appendFormattedString(GPU_STR_FrameSubFolderNameFormat, frameIndex);
            frameDir.appendSubDirectory(frameSubFolder);

            QString sessionDirStr = acGTStringToQString(frameDir.asString());
            QDir sessionFrameDir(acGTStringToQString(frameDir.asString()));
            QFileInfoList perfCountersDataFiles = sessionFrameDir.entryInfoList(QDir::Files, QDir::Time | QDir::Reversed);

            // Sort the directories by creation date, so the sessions appear chronologically
            qSort(perfCountersDataFiles.begin(), perfCountersDataFiles.end(), CompareDirOnDate);

            foreach (QFileInfo currentFile, perfCountersDataFiles)
            {
                QString extension = currentFile.suffix();

                if (extension == GP_PerformanceCounters_FileExtension)
                {
                    // Add the current performance file to tree
                    AddPerformanceCountersFileToTree(pSessionData, currentFile, pPerformanceCountersItem, frameIndex);
                }
            }
        }
    }
}

void gpTreeHandler::AddPerformanceCountersFileToTree(gpSessionTreeNodeData* pSessionData, const QFileInfo& performanceFilePath, QTreeWidgetItem* pPerformanceCountersItem, int frameIndex)
{
    afApplicationTree* pTree = afApplicationCommands::instance()->applicationTree();

    // We expect the following file format: "SESSIONNAME_Frame_FRAMEINDEX_Profile_HOUR_MINUTE_SEC"
    // And we want the tree item text to be: "Profile HOUR:MINUTE:SEC"
    QStringList fileNameParts = performanceFilePath.baseName().split(AF_STR_Underscore);

    // Sanity check:
    GT_IF_WITH_ASSERT((fileNameParts.size() >= 8) && (pTree != nullptr) && (pPerformanceCountersItem != nullptr) && (pSessionData != nullptr))
    {
        // Add a child to the performance counters node, with this file
        QString perfSessionName = QString(GP_Str_SessionFileNameFormat).arg(fileNameParts[5]).arg(fileNameParts[6]).arg(fileNameParts[7]);
        gtString perfCountersProfileName = acQStringToGTString(perfSessionName);

        afApplicationTreeItemData* pFileData = new afApplicationTreeItemData(true);
        pFileData->m_filePath = BuildFrameChildFilePath(pSessionData, AF_TREE_ITEM_GP_FRAME_OVERVIEW, frameIndex);
        pFileData->m_filePathLineNumber = AF_TREE_ITEM_GP_FRAME_PERFORMANCE_PROFILE;
        pFileData->m_itemType = AF_TREE_ITEM_GP_FRAME_PERFORMANCE_PROFILE;
        gpSessionTreeNodeData* pNewData = new gpSessionTreeNodeData(*pSessionData);
        pNewData->m_frameFilePath = acQStringToGTString(performanceFilePath.filePath());
        pNewData->m_frameIndex = frameIndex;
        pNewData->m_displayName = perfSessionName;
        pFileData->setExtendedData(pNewData);

        pTree->addTreeItem(perfCountersProfileName, pFileData, pPerformanceCountersItem);
    }
}

afApplicationTreeItemData* gpTreeHandler::GetFrameChildItemData(QTreeWidgetItem* pFrameItem, afTreeItemType childItemType)
{
    afApplicationTreeItemData* pRetVal = nullptr;

    // Sanity check
    afApplicationTree* pTree = afApplicationCommands::instance()->applicationTree();
    GT_IF_WITH_ASSERT((pFrameItem != nullptr) && (pTree != nullptr))
    {
        // Sanity check:
        GT_IF_WITH_ASSERT(pFrameItem != nullptr)
        {
            for (int i = 0; i < pFrameItem->childCount(); i++)
            {
                QTreeWidgetItem* pChildItem = pFrameItem->child(i);

                if (pChildItem != nullptr)
                {
                    afApplicationTreeItemData* pChildData = pTree->getTreeItemData(pChildItem);

                    if ((pChildData != nullptr) && (pChildData->m_itemType == childItemType))
                    {
                        pRetVal = pChildData;
                        break;
                    }
                }
            }
        }
    }
    return pRetVal;
}

void gpTreeHandler::AddTimelineToSession(const osFilePath& apiTraceFilePath, int frameIndex, bool shouldActivate)
{
    // Sanity check:
    afApplicationTree* pTree = afApplicationCommands::instance()->applicationTree();
    gpSessionTreeNodeData* pCurrentlyRunningSessionData = gpUIManager::Instance()->CurrentlyRunningSessionData();
    GT_IF_WITH_ASSERT((pCurrentlyRunningSessionData != nullptr) && (pCurrentlyRunningSessionData->m_pParentData != nullptr) && (pTree != nullptr))
    {
        afApplicationTreeItemData* pItemData = pCurrentlyRunningSessionData->m_pParentData;

        // Sanity check:
        GT_IF_WITH_ASSERT(pItemData->m_pTreeWidgetItem != nullptr)
        {
            // Find the frame child
            QTreeWidgetItem* pFrameItem = GetFrameTreeItem(pCurrentlyRunningSessionData, frameIndex);

            // Sanity check:
            GT_IF_WITH_ASSERT(pFrameItem != nullptr)
            {
                QTreeWidgetItem* pTimelineItem = pFrameItem->child(AF_TREE_ITEM_GP_FRAME_TIMELINE - AF_TREE_ITEM_GP_FRAME_FIRST_CHILD);

                // Sanity check:
                GT_IF_WITH_ASSERT(pTimelineItem != nullptr)
                {
                    afApplicationTreeItemData* pTimelineData = pTree->getTreeItemData(pTimelineItem);
                    afApplicationTreeItemData* pFrameData = pTree->getTreeItemData(pFrameItem);

                    // Sanity check:
                    GT_IF_WITH_ASSERT((pTimelineData != nullptr) && (pFrameData != nullptr))
                    {
                        pTimelineData->m_filePath = apiTraceFilePath;
                        pTimelineData->m_filePathLineNumber = AF_TREE_ITEM_GP_FRAME_TIMELINE;
                    }

                    if (shouldActivate)
                    {
                        pTree->expandItem(pTimelineItem);
                        pTree->selectItem(pTimelineData, true);

                    }
                }
            }
        }
    }
}

void gpTreeHandler::AddCapturedFrameToTree(int frameIndex, bool shouldExpand)
{
    // Sanity check:
    afApplicationTree* pTree = afApplicationCommands::instance()->applicationTree();
    gpSessionTreeNodeData* pCurrentlyRunningSessionData = gpUIManager::Instance()->CurrentlyRunningSessionData();
    GT_IF_WITH_ASSERT((pCurrentlyRunningSessionData != nullptr) && (pCurrentlyRunningSessionData->m_pParentData != nullptr) && (pTree != nullptr))
    {
        afApplicationTreeItemData* pItemData = pCurrentlyRunningSessionData->m_pParentData;

        // Sanity check:
        GT_IF_WITH_ASSERT(pItemData->m_pTreeWidgetItem != nullptr)
        {
            // Create the folder to the frame if it doesn't exist
            osFilePath frameDir = pCurrentlyRunningSessionData->SessionDir().directoryPath();
            gtString frameSubFolder;
            frameSubFolder.appendFormattedString(GPU_STR_FrameSubFolderNameFormat, frameIndex);
            frameDir.appendSubDirectory(frameSubFolder);
            osDirectory dir;
            frameDir.getFileDirectory(dir);

            if (!dir.exists())
            {
                dir.create();
            }

            // Find the frame child
            QTreeWidgetItem* pFrameItem = GetFrameTreeItem(pCurrentlyRunningSessionData, frameIndex);

            if (shouldExpand)
            {
                // Sanity check:
                GT_IF_WITH_ASSERT(pFrameItem != nullptr)
                {
                    pTree->expandItem(pFrameItem);
                }
            }
        }
    }
}

void gpTreeHandler::AddCountersDataFileToSession(const osFilePath& countersFilePath, int frameIndex, bool shouldActivate)
{
    // Sanity check:
    afApplicationTree* pTree = afApplicationCommands::instance()->applicationTree();
    gpSessionTreeNodeData* pCurrentlyRunningSessionData = gpUIManager::Instance()->CurrentlyRunningSessionData();
    GT_IF_WITH_ASSERT((pCurrentlyRunningSessionData != nullptr) && (pCurrentlyRunningSessionData->m_pParentData != nullptr) && (pTree != nullptr))
    {
        afApplicationTreeItemData* pItemData = pCurrentlyRunningSessionData->m_pParentData;

        // Sanity check:
        GT_IF_WITH_ASSERT(pItemData->m_pTreeWidgetItem != nullptr)
        {
            // Find the frame child
            QTreeWidgetItem* pFrameItem = GetFrameTreeItem(pCurrentlyRunningSessionData, frameIndex);

            // Sanity check:
            GT_IF_WITH_ASSERT(pFrameItem != nullptr)
            {
                QTreeWidgetItem* pPerformanceDataItem = pFrameItem->child(AF_TREE_ITEM_GP_FRAME_PERFORMANCE_PROFILES - AF_TREE_ITEM_GP_FRAME_FIRST_CHILD + 1);

                // Sanity check:
                GT_IF_WITH_ASSERT(pPerformanceDataItem != nullptr)
                {
                    afApplicationTreeItemData* pPerformanceData = pTree->getTreeItemData(pPerformanceDataItem);
                    afApplicationTreeItemData* pFrameData = pTree->getTreeItemData(pFrameItem);

                    // Sanity check:
                    GT_IF_WITH_ASSERT((pPerformanceData != nullptr) && (pFrameData != nullptr))
                    {
                        gpSessionTreeNodeData* pSessionData = qobject_cast<gpSessionTreeNodeData*>(pPerformanceData->extendedItemData());
                        GT_IF_WITH_ASSERT(pSessionData != nullptr)
                        {
                            pSessionData->m_frameFilePath = countersFilePath;
                            pPerformanceData->m_filePathLineNumber = AF_TREE_ITEM_GP_FRAME_PERFORMANCE_PROFILE;
                        }
                    }

                    if (shouldActivate)
                    {
                        pTree->expandItem(pPerformanceDataItem);
                        pTree->selectItem(pPerformanceData, true);
                    }

                    AddPerformanceCountersFileToTree(pCurrentlyRunningSessionData, QFileInfo(acGTStringToQString(countersFilePath.asString())), pPerformanceDataItem, frameIndex);
                }
            }
        }
    }
}

bool gpTreeHandler::ExportFile(const osDirectory& sessionDir, const QString& exportFilePath, SessionTreeNodeData* pSessionData)
{
    SharedProfileManager::instance().setExportIsRunning(true);
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    pApplicationCommands->updateToolbarCommands();

    PrepareTraceForSessionFrames(sessionDir, pSessionData);

    QString currentProjectPath = acGTStringToQString(afProjectManager::instance().currentProjectFilePath().fileDirectoryAsString());
    osFilePath filePath;
    acQStringToOSFilePath(currentProjectPath, filePath);

    osDirectorySerializer dirSerailzer;
    bool success = dirSerailzer.CompressDir(sessionDir, acQStringToGTString(exportFilePath));

    SharedProfileManager::instance().setExportIsRunning(false);
    pApplicationCommands->updateToolbarCommands();

    return success;
}

bool gpTreeHandler::PrepareTraceForSessionFrames(const osDirectory& sessionDir, SessionTreeNodeData* pSessionData)
{
    bool retVal = true;

    osFilePath sessionFilePath;
    sessionFilePath.setFileDirectory(sessionDir);

    // Get the list of frame thumbnails for the current session
    QList<int> frameIndicesList;
    gpUIManager::Instance()->GetListOfFrameFolders(sessionFilePath, frameIndicesList);
    gpExecutionMode* pModeManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();

    gtString sessionName;
    sessionFilePath.asString().getSubString(sessionFilePath.asString().findLastOf(osFilePath::osPathSeparator) + 1, sessionFilePath.asString().length(), sessionName);
    sessionFilePath.setFileName(sessionName);
    sessionFilePath.setFileExtension(GP_Dashbord_FileExtensionW);

    bool isFrameAnalysisExport = (0 != (afPluginConnectionManager::instance().getCurrentRunModeMask() & AF_FRAME_ANALYZE_CURRENTLY_EXPORTING));

    if (isFrameAnalysisExport)
    {
        afProgressBarWrapper::instance().ShowProgressDialog(GP_STR_FrameAnalysisExportProgressMsg, frameIndicesList.size()*GP_PROGRESS_COEF);
        afProgressBarWrapper::instance().SetProgressDialogCaption(GP_STR_FrameAnalysisExportProgressHeader);
    }

    foreach (int frameIndex, frameIndicesList)
    {
        if (isFrameAnalysisExport)
        {
            gtString msg;
            msg.appendFormattedString(GP_STR_FrameAnalysisExportCapturingFrameMsg, frameIndex);
            afProgressBarWrapper::instance().setProgressText(msg);
            afProgressBarWrapper::instance().incrementProgressBar(GP_PROGRESS_COEF / 2);
        }

        // Get the directory, overview and thumbnail paths for this frame
        QDir frameDir;
        QString overviewFilePath, thumbnailFilePath;
        FrameInfo currentFrameInfo;
        bool rc = gpUIManager::Instance()->GetPathsForFrame(sessionFilePath, frameIndex, frameDir, overviewFilePath, thumbnailFilePath);
        GT_IF_WITH_ASSERT(rc && pModeManager != nullptr)
        {
            retVal &= pModeManager->PrepareTraceFile(sessionFilePath, frameIndex, pSessionData, nullptr, false);
        }

        if (isFrameAnalysisExport)
        {
            afProgressBarWrapper::instance().incrementProgressBar(GP_PROGRESS_COEF / 2);
        }
    }

    if (isFrameAnalysisExport)
    {
        afProgressBarWrapper::instance().hideProgressBar();
    }

    return retVal;
}

bool gpTreeHandler::IsExportEnabled()const
{
    bool ret = false;

    if ((gpUIManager::Instance()->CurrentlyRunningSessionData() == nullptr) && afExecutionModeManager::instance().isActiveMode(PM_STR_FrameAnalysisMode))
    {
        ret = true;
    }

    return ret;
}

bool gpTreeHandler::RefreshSessionsFromServer()
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((ProfileManager::Instance() != nullptr) && (ProfileManager::Instance()->GetFrameAnalysisModeManager() != nullptr))
    {
        ProfileManager::Instance()->GetFrameAnalysisModeManager()->RefreshLoadedProjectSessionsFromServer();
    }
    return retVal;
}

