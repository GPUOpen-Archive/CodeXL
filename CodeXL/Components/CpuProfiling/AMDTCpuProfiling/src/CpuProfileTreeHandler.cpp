//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileTreeHandler.cpp
///
//==================================================================================

// Infra:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTApplicationComponents/Include/acIcons.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTree.h>
#include <AMDTApplicationFramework/src/afUtils.h>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/ProfileApplicationTreeHandler.h>
#include <AMDTSharedProfiling/inc/SharedProfileSettingPage.h>
#include <AMDTSharedProfiling/inc/StringConstants.h>

// Backend:
#include <AMDTCpuProfilingRawData/inc/CpuProfileReader.h>

/// Local:
#include <inc/CpuProfileTreeHandler.h>
#include <inc/CpuProjectHandler.h>
#include <inc/StringConstants.h>


CpuProfileTreeHandler* CpuProfileTreeHandler::m_psMySingleInstance = nullptr;

//-------------------------------------------------------------------------------------------
CpuProfileTreeHandler::CpuProfileTreeHandler() : m_pApplicationTree(nullptr)
{
}

//-------------------------------------------------------------------------------------------
CpuProfileTreeHandler::~CpuProfileTreeHandler()
{

}

//-------------------------------------------------------------------------------------------
CpuProfileTreeHandler& CpuProfileTreeHandler::instance()
{
    // If my single instance was not created yet - create it:
    if (nullptr == m_psMySingleInstance)
    {
        m_psMySingleInstance = new CpuProfileTreeHandler;

    }

    return *m_psMySingleInstance;
}

//-------------------------------------------------------------------------------------------
bool CpuProfileTreeHandler::BuildSessionTree(const SessionTreeNodeData& sessionTreeNodeData, QTreeWidgetItem* pTreeItem)
{
    bool retVal = false;
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(pApplicationCommands != nullptr)
    {
        m_pApplicationTree = pApplicationCommands->applicationTree();
        GT_IF_WITH_ASSERT((pTreeItem != nullptr) && (m_pApplicationTree != nullptr))
        {
            afApplicationTreeItemData* pItemData = m_pApplicationTree->getTreeItemData(pTreeItem);
            GT_IF_WITH_ASSERT(pItemData != nullptr)
            {
                CPUSessionTreeItemData* pCPUTreeItemData = qobject_cast<CPUSessionTreeItemData*>(pItemData->extendedItemData());
                GT_IF_WITH_ASSERT(pCPUTreeItemData != nullptr)
                {

                    afApplicationTreeItemData* pItemData1 = new afApplicationTreeItemData;

                    pItemData1->m_itemType = AF_TREE_ITEM_PROFILE_CPU_OVERVIEW;
                    pItemData1->m_filePath = pItemData->m_filePath;
                    pItemData1->m_filePathLineNumber = (int)AF_TREE_ITEM_PROFILE_CPU_OVERVIEW;
                    pItemData1->setExtendedData(pCPUTreeItemData);
                    QTreeWidgetItem* pItem = m_pApplicationTree->addTreeItem(CPUSessionTreeItemData::sessionDisplayTypeToString(AF_TREE_ITEM_PROFILE_CPU_OVERVIEW), pItemData1, pTreeItem);
                    GT_IF_WITH_ASSERT(pItem != nullptr)
                    {
                        QPixmap* pSummaryTypeIcon = ProfileApplicationTreeHandler::instance()->TreeItemTypeToPixmap(pItemData1->m_itemType);
                        pItem->setIcon(0, QIcon(*pSummaryTypeIcon));
                    }

                    afApplicationTreeItemData* pItemData2 = new afApplicationTreeItemData;

                    pItemData2->m_itemType = AF_TREE_ITEM_PROFILE_CPU_MODULES;
                    pItemData2->m_filePathLineNumber = (int)AF_TREE_ITEM_PROFILE_CPU_MODULES;
                    pItemData2->m_filePath = pItemData->m_filePath;
                    pItemData2->setExtendedData(pCPUTreeItemData);
                    pItem = m_pApplicationTree->addTreeItem(CPUSessionTreeItemData::sessionDisplayTypeToString(AF_TREE_ITEM_PROFILE_CPU_MODULES), pItemData2, pTreeItem);
                    GT_IF_WITH_ASSERT(pItem != nullptr)
                    {
                        QPixmap* pModulesIcon = ProfileApplicationTreeHandler::instance()->TreeItemTypeToPixmap(pItemData2->m_itemType);
                        pItem->setIcon(0, QIcon(*pModulesIcon));
                    }

                    // Add the call graph item only if the CSS is collected:
                    const CPUSessionTreeItemData& cpuSessionData = (const CPUSessionTreeItemData&)sessionTreeNodeData;

                    if (cpuSessionData.ShouldCollectCSS(true))
                    {
                        afApplicationTreeItemData* pItemData3 = new afApplicationTreeItemData;

                        pItemData3->m_itemType = AF_TREE_ITEM_PROFILE_CPU_CALL_GRAPH;
                        pItemData3->m_filePathLineNumber = (int)AF_TREE_ITEM_PROFILE_CPU_CALL_GRAPH;
                        pItemData3->m_filePath = pItemData->m_filePath;
                        pItemData3->setExtendedData(pCPUTreeItemData);
                        pItem = m_pApplicationTree->addTreeItem(CPUSessionTreeItemData::sessionDisplayTypeToString(AF_TREE_ITEM_PROFILE_CPU_CALL_GRAPH), pItemData3, pTreeItem);
                        GT_IF_WITH_ASSERT(pItem != nullptr)
                        {
                            QPixmap* pCallGraphIcon = ProfileApplicationTreeHandler::instance()->TreeItemTypeToPixmap(pItemData3->m_itemType);
                            pItem->setIcon(0, QIcon(*pCallGraphIcon));
                        }
                    }

                    afApplicationTreeItemData* pItemData4 = new afApplicationTreeItemData;

                    pItemData4->m_itemType = AF_TREE_ITEM_PROFILE_CPU_FUNCTIONS;
                    pItemData4->m_filePathLineNumber = (int)AF_TREE_ITEM_PROFILE_CPU_FUNCTIONS;
                    pItemData4->m_filePath = pItemData->m_filePath;
                    pItemData4->setExtendedData(pCPUTreeItemData);
                    pItem = m_pApplicationTree->addTreeItem(CPUSessionTreeItemData::sessionDisplayTypeToString(AF_TREE_ITEM_PROFILE_CPU_FUNCTIONS), pItemData4, pTreeItem);
                    GT_IF_WITH_ASSERT(pItem != nullptr)
                    {
                        QPixmap* pFunctionsIcon = ProfileApplicationTreeHandler::instance()->TreeItemTypeToPixmap(pItemData4->m_itemType);
                        pItem->setIcon(0, QIcon(*pFunctionsIcon));
                    }
                }
            }
        }
    }

    return retVal;
}

//-------------------------------------------------------------------------------------------
afApplicationTreeItemData* CpuProfileTreeHandler::findChildItemData(const afApplicationTreeItemData* pParentItemData, afTreeItemType childType)
{
    afApplicationTreeItemData* pRetVal = nullptr;
    GT_IF_WITH_ASSERT((pParentItemData != nullptr) && (pParentItemData->m_pTreeWidgetItem != nullptr) && (m_pApplicationTree != nullptr))
    {
        // Go through the session children and look for it's module child:
        int childCount = pParentItemData->m_pTreeWidgetItem->childCount();

        for (int i = 0; i < childCount; i++)
        {
            QTreeWidgetItem* pItem = pParentItemData->m_pTreeWidgetItem->child(i);
            GT_IF_WITH_ASSERT(pItem != nullptr)
            {
                afApplicationTreeItemData* pItemData = m_pApplicationTree->getTreeItemData(pItem);
                GT_IF_WITH_ASSERT(pItemData != nullptr)
                {
                    if (pItemData->m_itemType == childType)
                    {
                        pRetVal = pItemData;
                        break;
                    }
                }
            }
        }
    }

    return pRetVal;
}

//-------------------------------------------------------------------------------------------
bool CpuProfileTreeHandler::ExtendSessionHTMLPropeties(afTreeItemType sessionTreeItemType, const SessionTreeNodeData* pSessionData, afHTMLContent& htmlContent)
{
    bool retVal = false;

    // Check if this is a session of a session child:
    bool isItemSessionChild = ((sessionTreeItemType == AF_TREE_ITEM_PROFILE_SESSION) || DoesTreeItemTypeBelongsToHandler(sessionTreeItemType));

    const CPUSessionTreeItemData* pCPUSessionData = qobject_cast<const CPUSessionTreeItemData*>(pSessionData);

    //Ignore sessions that aren't mine
    if (pCPUSessionData != nullptr && isItemSessionChild && pSessionData->m_pParentData != nullptr)
    {
        retVal = true;

        // If there is an available profile data file, open it
        gtString content;
        CpuProfileReader profileReader;

        if (profileReader.open(pSessionData->m_pParentData->m_filePath.asString().asCharArray()))
        {
            CpuProfileInfo* pProfileInfo = profileReader.getProfileInfo();

            if (nullptr != pProfileInfo)
            {
                unsigned int cpuFamily = pProfileInfo->m_cpuFamily;
                unsigned int cpuModel = pProfileInfo->m_cpuModel;
                unsigned int cpuCount = pProfileInfo->m_numCpus;
                content = L"CPU Details: ";
                content.appendFormattedString(CP_overviewPageProfileCPUDetailsStr, cpuFamily, cpuModel, cpuCount);
                htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, content);
            }

            profileReader.close();
        }

        // Profile Duration
        gtString duration = acDurationAsString(pSessionData->m_profileDuration);
        content = CP_overviewPageProfileDuration;
        content.appendFormattedString(L": %ls", duration.asCharArray());

        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, content);

        //Profile Start options
        if (pSessionData->m_startDelay > 0)
        {
            content.makeEmpty();
            content.appendFormattedString(L"Start delay: %d seconds", pSessionData->m_startDelay);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, content);
        }

        content.makeEmpty();
        content.appendFormattedString(L"%ls: 0x%llx", CP_overviewPageProfileCPUAffinity, pCPUSessionData->m_startAffinity);
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, content);

        if (pCPUSessionData->m_isProfilePaused)
        {
            content = L"The profile was paused at time of starting";
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, content);
        }

        if (pCPUSessionData->ShouldCollectCSS(true))
        {
            content.makeEmpty();

            gtString cssSupportFpoFormattedStr;
            const wchar_t* pCssScopeStr = nullptr;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

            switch (pCPUSessionData->m_cssScope)
            {
                case CP_CSS_SCOPE_USER:
                    pCssScopeStr = _T(CP_STR_cpuProfileProjectSettingsCallStackUserSpace);
                    break;

                case CP_CSS_SCOPE_KERNEL:
                    pCssScopeStr = _T(CP_STR_cpuProfileProjectSettingsCallStackKernelSpace);
                    break;

                case CP_CSS_SCOPE_ALL:
                    pCssScopeStr = _T(CP_STR_cpuProfileProjectSettingsCallStackUserKernelSpaces);
                    break;

                case CP_CSS_SCOPE_UNKNOWN:
                    break;
            }

            const wchar_t* pCssSupportFpoStr = pCPUSessionData->IsFpoChecked() ? L"On" : L"Off";
            cssSupportFpoFormattedStr.appendFormattedString(CP_overviewCallStackInformationFpoSubstr, pCssSupportFpoStr);
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

            gtString cssScopeFormattedStr;

            if (nullptr != pCssScopeStr)
            {
                cssScopeFormattedStr.appendFormattedString(CP_overviewCallStackInformationScopeSubstr, pCssScopeStr);
            }

            content.appendFormattedString(CP_overviewCallStackInformation, cssScopeFormattedStr.asCharArray(),
                                          pCPUSessionData->GetCssUnwindLevel(),
                                          pCPUSessionData->m_cssInterval,
                                          cssSupportFpoFormattedStr.asCharArray());

            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, content);
        }
    }

    return retVal;
}

//-------------------------------------------------------------------------------------------
void CpuProfileTreeHandler::InitializeProfileIcons()
{
    // Create the icons:
    AddTreeIconToMap(AC_ICON_PROFILE_CPU_APPTREE_TBS_SINGLE);
    AddTreeIconToMap(AC_ICON_PROFILE_CPU_APPTREE_ASSESS_SINGLE);
    AddTreeIconToMap(AC_ICON_PROFILE_CPU_APPTREE_IBS_SINGLE);
    AddTreeIconToMap(AC_ICON_PROFILE_CPU_APPTREE_BRANCHING_SINGLE);
    AddTreeIconToMap(AC_ICON_PROFILE_CPU_APPTREE_DATAACCESS_SINGLE);
    AddTreeIconToMap(AC_ICON_PROFILE_CPU_APPTREE_INSTACCESS_SINGLE);
    AddTreeIconToMap(AC_ICON_PROFILE_CPU_APPTREE_L2CACHE_SINGLE);
    AddTreeIconToMap(AC_ICON_PROFILE_CPU_APPTREE_CUSTOM_SINGLE);
    AddTreeIconToMap(AC_ICON_PROFILE_CPU_APPTREE_CLU_SINGLE);
    AddTreeIconToMap(AC_ICON_PROFILE_CPU_APPTREE_TBS_MULTI);
    AddTreeIconToMap(AC_ICON_PROFILE_CPU_APPTREE_ASSESS_MULTI);
    AddTreeIconToMap(AC_ICON_PROFILE_CPU_APPTREE_IBS_MULTI);
    AddTreeIconToMap(AC_ICON_PROFILE_CPU_APPTREE_BRANCHING_MULTI);
    AddTreeIconToMap(AC_ICON_PROFILE_CPU_APPTREE_DATAACCESS_MULTI);
    AddTreeIconToMap(AC_ICON_PROFILE_CPU_APPTREE_INSTACCESS_MULTI);
    AddTreeIconToMap(AC_ICON_PROFILE_CPU_APPTREE_L2CACHE_MULTI);
    AddTreeIconToMap(AC_ICON_PROFILE_CPU_APPTREE_CUSTOM_MULTI);
    AddTreeIconToMap(AC_ICON_PROFILE_CPU_APPTREE_CLU_MULTI);
    AddTreeIconToMap(AC_ICON_PROFILE_CPU_APPTREE_OVERVIEW);
    AddTreeIconToMap(AC_ICON_PROFILE_CPU_APPTREE_MODULES);
    AddTreeIconToMap(AC_ICON_PROFILE_CPU_APPTREE_CALLGRAPH);
    AddTreeIconToMap(AC_ICON_PROFILE_CPU_APPTREE_FUNCTIONS);
    AddTreeIconToMap(AC_ICON_PROFILE_CPU_APPTREE_SOURCES);
    AddTreeIconToMap(AC_ICON_PROFILE_CPU_APPTREE_SOURCE);
}

//-------------------------------------------------------------------------------------------
QPixmap* CpuProfileTreeHandler::TreeItemTypeToPixmap(afTreeItemType itemType, const QString& sessionTypeAsStr)
{
    QPixmap* pRetPixmap = nullptr;

    acIconId iconId = AC_ICON_EMPTY;
    int sessionTypeIdx = -1;

    if ((AF_TREE_ITEM_PROFILE_SESSION == itemType) || (AF_TREE_ITEM_PROFILE_SESSION_TYPE == itemType))
    {
        if (PM_profileTypeTimeBased == sessionTypeAsStr)
        {
            sessionTypeIdx = CPU_TREE_TIME_BASED_SESSION;
        }
        else if (PM_profileTypeAssesPerformance == sessionTypeAsStr)
        {
            sessionTypeIdx = CPU_TREE_ASSESS_PERFORMANCE_SESSION;
        }
        else if (PM_profileTypeInstructionBasedSampling == sessionTypeAsStr)
        {
            sessionTypeIdx = CPU_TREE_INSTRUCTION_BASED_SAMPLING_SESSION;
        }
        else if (PM_profileTypeInvestigateBranching == sessionTypeAsStr)
        {
            sessionTypeIdx = CPU_TREE_INVESTIGATE_BRANCHING_SESSION;
        }
        else if (PM_profileTypeInvestigateDataAccess == sessionTypeAsStr)
        {
            sessionTypeIdx = CPU_TREE_INVESTIGATE_DATA_ACCESS_SESSION;
        }
        else if (PM_profileTypeInvestigateInstructionAccess == sessionTypeAsStr)
        {
            sessionTypeIdx = CPU_TREE_INSTRUCTION_ACCESS_SESSION;
        }
        else if (PM_profileTypeInvestigateInstructionL2CacheAccess == sessionTypeAsStr)
        {
            sessionTypeIdx = CPU_TREE_L2_CACHE_ACCESS_SESSION;
        }
        else if (PM_profileTypeCustomProfile == sessionTypeAsStr)
        {
            sessionTypeIdx = CPU_TREE_CUSTOM_PROFILE_SESSION;
        }
        else if (PM_profileTypeCLU == sessionTypeAsStr)
        {
            sessionTypeIdx = CPU_TREE_CACHE_LINE_SESSION;
        }
    }

    switch (itemType)
    {
        case AF_TREE_ITEM_PROFILE_SESSION:
        {
            switch (sessionTypeIdx)
            {
                case CPU_TREE_TIME_BASED_SESSION:
                    iconId = AC_ICON_PROFILE_CPU_APPTREE_TBS_SINGLE;
                    break;

                case CPU_TREE_ASSESS_PERFORMANCE_SESSION:
                    iconId = AC_ICON_PROFILE_CPU_APPTREE_ASSESS_SINGLE;
                    break;

                case CPU_TREE_INSTRUCTION_BASED_SAMPLING_SESSION:
                    iconId = AC_ICON_PROFILE_CPU_APPTREE_IBS_SINGLE;
                    break;

                case CPU_TREE_INVESTIGATE_BRANCHING_SESSION:
                    iconId = AC_ICON_PROFILE_CPU_APPTREE_BRANCHING_SINGLE;
                    break;

                case CPU_TREE_INVESTIGATE_DATA_ACCESS_SESSION:
                    iconId = AC_ICON_PROFILE_CPU_APPTREE_DATAACCESS_SINGLE;
                    break;

                case CPU_TREE_INSTRUCTION_ACCESS_SESSION:
                    iconId = AC_ICON_PROFILE_CPU_APPTREE_INSTACCESS_SINGLE;
                    break;

                case CPU_TREE_L2_CACHE_ACCESS_SESSION:
                    iconId = AC_ICON_PROFILE_CPU_APPTREE_L2CACHE_SINGLE;
                    break;

                case CPU_TREE_CUSTOM_PROFILE_SESSION:
                    iconId = AC_ICON_PROFILE_CPU_APPTREE_CUSTOM_SINGLE;
                    break;

                case CPU_TREE_CACHE_LINE_SESSION:
                    iconId = AC_ICON_PROFILE_CPU_APPTREE_CLU_SINGLE;
                    break;

                default:
                    break;
            }
        }
        break;

        case AF_TREE_ITEM_PROFILE_SESSION_TYPE:
        {
            switch (sessionTypeIdx)
            {
                case CPU_TREE_TIME_BASED_SESSION:
                    iconId = AC_ICON_PROFILE_CPU_APPTREE_TBS_MULTI;
                    break;

                case CPU_TREE_ASSESS_PERFORMANCE_SESSION:
                    iconId = AC_ICON_PROFILE_CPU_APPTREE_ASSESS_MULTI;
                    break;

                case CPU_TREE_INSTRUCTION_BASED_SAMPLING_SESSION:
                    iconId = AC_ICON_PROFILE_CPU_APPTREE_IBS_MULTI;
                    break;

                case CPU_TREE_INVESTIGATE_BRANCHING_SESSION:
                    iconId = AC_ICON_PROFILE_CPU_APPTREE_BRANCHING_MULTI;
                    break;

                case CPU_TREE_INVESTIGATE_DATA_ACCESS_SESSION:
                    iconId = AC_ICON_PROFILE_CPU_APPTREE_DATAACCESS_MULTI;
                    break;

                case CPU_TREE_INSTRUCTION_ACCESS_SESSION:
                    iconId = AC_ICON_PROFILE_CPU_APPTREE_INSTACCESS_MULTI;
                    break;

                case CPU_TREE_L2_CACHE_ACCESS_SESSION:
                    iconId = AC_ICON_PROFILE_CPU_APPTREE_L2CACHE_MULTI;
                    break;

                case CPU_TREE_CUSTOM_PROFILE_SESSION:
                    iconId = AC_ICON_PROFILE_CPU_APPTREE_CUSTOM_MULTI;
                    break;

                case CPU_TREE_CACHE_LINE_SESSION:
                    iconId = AC_ICON_PROFILE_CPU_APPTREE_CLU_MULTI;
                    break;

                default:
                    break;
            }
        }
        break;

        case AF_TREE_ITEM_PROFILE_CPU_OVERVIEW:
            iconId = AC_ICON_PROFILE_CPU_APPTREE_OVERVIEW;
            break;

        case AF_TREE_ITEM_PROFILE_CPU_MODULES:
            iconId = AC_ICON_PROFILE_CPU_APPTREE_MODULES;
            break;

        case AF_TREE_ITEM_PROFILE_CPU_CALL_GRAPH:
            iconId = AC_ICON_PROFILE_CPU_APPTREE_CALLGRAPH;
            break;

        case AF_TREE_ITEM_PROFILE_CPU_FUNCTIONS:
            iconId = AC_ICON_PROFILE_CPU_APPTREE_FUNCTIONS;
            break;

        case AF_TREE_ITEM_PROFILE_CPU_SOURCE_CODES:
            iconId = AC_ICON_PROFILE_CPU_APPTREE_SOURCES;
            break;

        case AF_TREE_ITEM_PROFILE_CPU_SOURCE_CODE:
            iconId = AC_ICON_PROFILE_CPU_APPTREE_SOURCE;
            break;

        default:
        {
            break;
        }
    }

    pRetPixmap = m_iconsMap[iconId];

    return pRetPixmap;
}

//-------------------------------------------------------------------------------------------
afApplicationTreeItemData* CpuProfileTreeHandler::AddSourceCodeToSessionNode(SessionTreeNodeData* pSourceCodeItemSessionData)
{
    afApplicationTreeItemData* pRetVal = nullptr;

    if (nullptr == m_pApplicationTree)
    {
        afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
        GT_IF_WITH_ASSERT(pApplicationCommands != nullptr)
        {
            m_pApplicationTree = pApplicationCommands->applicationTree();
        }
    }

    // Sanity check:
    GT_IF_WITH_ASSERT((pSourceCodeItemSessionData != nullptr) && (m_pApplicationTree != nullptr) && (m_pApplicationTree->headerItem() != nullptr))
    {
        // Find the session item data by display name:
        afApplicationTreeItemData* pSessionData = ProfileApplicationTreeHandler::instance()->FindItemByProfileDisplayName(pSourceCodeItemSessionData->m_displayName);
        GT_IF_WITH_ASSERT(pSessionData != nullptr)
        {
            // Find the source code child of the session:
            afApplicationTreeItemData* pSessionSourceCodeData = createSourceCodeNodeForSession(pSessionData);
            GT_IF_WITH_ASSERT(pSessionSourceCodeData != nullptr)
            {
                // First check if an item with this name exist:
                pRetVal = FindSessionSourceCodeItemData(pSessionData, pSourceCodeItemSessionData->m_exeFullPath);

                if (pRetVal == nullptr)
                {
                    // Source code item doesn't exist:
                    pRetVal = new afApplicationTreeItemData;


                    pRetVal->m_itemType = AF_TREE_ITEM_PROFILE_CPU_SOURCE_CODE;
                    pRetVal->setExtendedData(pSourceCodeItemSessionData);

                    QPixmap* pPixmap = ProfileApplicationTreeHandler::instance()->TreeItemTypeToPixmap(AF_TREE_ITEM_PROFILE_CPU_SOURCE_CODE);
                    QFileInfo filePath(pSourceCodeItemSessionData->m_exeFullPath);
                    QTreeWidgetItem* pSourceCodeItem = m_pApplicationTree->addTreeItem(acQStringToGTString(filePath.fileName()), pRetVal, pSessionSourceCodeData->m_pTreeWidgetItem);
                    GT_IF_WITH_ASSERT((pSourceCodeItem != nullptr) && (pPixmap != nullptr))
                    {
                        pSourceCodeItem->setIcon(0, QIcon(*pPixmap));
                    }
                }
            }
        }
    }

    return pRetVal;
}

//-------------------------------------------------------------------------------------------
afApplicationTreeItemData* CpuProfileTreeHandler::createSourceCodeNodeForSession(afApplicationTreeItemData* pSessionItemData)
{
    afApplicationTreeItemData* pRetVal = nullptr;

    // Check if the node exists:
    pRetVal = ProfileApplicationTreeHandler::instance()->FindSessionChildItemData(pSessionItemData, AF_TREE_ITEM_PROFILE_CPU_SOURCE_CODES);

    if (pRetVal == nullptr)
    {
        // Sanity check:
        GT_IF_WITH_ASSERT(m_pApplicationTree != nullptr)
        {
            pRetVal = new afApplicationTreeItemData;


            pRetVal->m_itemType = AF_TREE_ITEM_PROFILE_CPU_SOURCE_CODES;
            pRetVal->setExtendedData(pSessionItemData->extendedItemData());
            QTreeWidgetItem* pItem = m_pApplicationTree->addTreeItem(SP_STR_SourceCodeNodeText, pRetVal, pSessionItemData->m_pTreeWidgetItem);
            GT_IF_WITH_ASSERT(pItem != nullptr)
            {
                QPixmap* pPixmap = ProfileApplicationTreeHandler::instance()->TreeItemTypeToPixmap(AF_TREE_ITEM_PROFILE_CPU_SOURCE_CODES);
                pItem->setIcon(0, QIcon(*pPixmap));
            }
        }
    }

    return pRetVal;
}

//-------------------------------------------------------------------------------------------
afApplicationTreeItemData* CpuProfileTreeHandler::FindSessionSourceCodeItemData(const afApplicationTreeItemData* pSessionItemData, const QString& moduleFilePath)
{
    afApplicationTreeItemData* pRetVal = nullptr;

    // Sanity check:
    GT_IF_WITH_ASSERT((pSessionItemData != nullptr) && (pSessionItemData->m_pTreeWidgetItem != nullptr))
    {
        afApplicationTreeItemData* pSourceCodeItemData = ProfileApplicationTreeHandler::instance()->FindSessionChildItemData(pSessionItemData, AF_TREE_ITEM_PROFILE_CPU_SOURCE_CODES);

        if ((pSourceCodeItemData != nullptr) && (pSourceCodeItemData->m_pTreeWidgetItem != nullptr))
        {
            int childCount = pSourceCodeItemData->m_pTreeWidgetItem->childCount();

            for (int i = 0; (i < childCount) && (pRetVal == nullptr); i++)
            {
                afApplicationTreeItemData* pChildData = m_pApplicationTree->getTreeItemData(pSourceCodeItemData->m_pTreeWidgetItem->child(i));

                if (pChildData != nullptr)
                {
                    SessionTreeNodeData* pSessionData = qobject_cast<SessionTreeNodeData*>(pChildData->extendedItemData());
                    GT_IF_WITH_ASSERT(pSessionData != nullptr)
                    {
                        if (pSessionData->m_exeFullPath == moduleFilePath)
                        {
                            pRetVal = pChildData;
                            break;
                        }
                    }
                }
            }
        }
    }

    return pRetVal;
}

//-------------------------------------------------------------------------------------------
bool CpuProfileTreeHandler::GetProfileTypeWithPrefix(const QString& sessionTypeAsStr, gtString& sessionTypeWithPrefix)
{
    bool retVal = false;

    if ((PM_profileTypeTimeBased == sessionTypeAsStr) ||
        (PM_profileTypeAssesPerformance == sessionTypeAsStr) ||
        (PM_profileTypeCLU == sessionTypeAsStr) ||
        (PM_profileTypeInstructionBasedSampling == sessionTypeAsStr) ||
        (PM_profileTypeInvestigateBranching == sessionTypeAsStr) ||
        (PM_profileTypeInvestigateDataAccess == sessionTypeAsStr) ||
        (PM_profileTypeInvestigateInstructionAccess == sessionTypeAsStr) ||
        (PM_profileTypeInvestigateInstructionL2CacheAccess == sessionTypeAsStr) ||
        (PM_profileTypeCustomProfile == sessionTypeAsStr))
    {
        sessionTypeWithPrefix.prepend(L"CPU: ");
        retVal = true;
    }

    return retVal;
}

//-------------------------------------------------------------------------------------------
bool CpuProfileTreeHandler::DoesTreeItemTypeBelongsToHandler(const afTreeItemType itemType) const
{
    bool retVal = false;

    retVal = ((itemType == AF_TREE_ITEM_PROFILE_CPU_OVERVIEW) || (itemType == AF_TREE_ITEM_PROFILE_CPU_MODULES)
              || (itemType == AF_TREE_ITEM_PROFILE_CPU_CALL_GRAPH) || (itemType == AF_TREE_ITEM_PROFILE_CPU_FUNCTIONS) || (itemType == AF_TREE_ITEM_PROFILE_CPU_SOURCE_CODES)
              || (itemType == AF_TREE_ITEM_PROFILE_CPU_SOURCE_CODE));

    return retVal;
}

//-------------------------------------------------------------------------------------------
bool CpuProfileTreeHandler::DoesTreeNodeDataBelongsToHandler(const SessionTreeNodeData* pSessionData) const
{
    bool retVal = false;

    QString sessionName = pSessionData->m_profileTypeStr;

    retVal = ((sessionName == PM_profileTypeTimeBased) ||
              (sessionName == PM_profileTypeCLU) ||
              (sessionName == PM_profileTypeAssesPerformance) ||
              (sessionName == PM_profileTypeInstructionBasedSampling) ||
              (sessionName == PM_profileTypeInstructionBasedSampling) ||
              (sessionName == PM_profileTypeInvestigateBranching) ||
              (sessionName == PM_profileTypeInvestigateDataAccess) ||
              (sessionName == PM_profileTypeInvestigateInstructionAccess) ||
              (sessionName == PM_profileTypeInvestigateInstructionL2CacheAccess) ||
              (sessionName == PM_profileTypeCustomProfile));

    return retVal;
}
