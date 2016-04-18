//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppTreeHandler.cpp
///
//==================================================================================

//------------------------------ ppTreeHandler.cpp ------------------------------

// Infra:
#include <AMDTApplicationComponents/Include/acIcons.h>
#include <AMDTAPIClasses/Include/apProjectSettings.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/src/afUtils.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/ProfileApplicationTreeHandler.h>
#include <AMDTSharedProfiling/inc/StringConstants.h>

// Local:
#include <AMDTPowerProfiling/src/ppTreeHandler.h>
#include <AMDTPowerProfiling/src/ppAppController.h>
#include <AMDTPowerProfiling/src/ppSessionView.h>
#include <AMDTPowerProfiling/Include/ppStringConstants.h>

ppTreeHandler* ppTreeHandler::m_psMySingleInstance = nullptr;

// ---------------------------------------------------------------------------
ppTreeHandler::ppTreeHandler() : m_pApplicationTree(nullptr)
{

}

// ---------------------------------------------------------------------------
ppTreeHandler::~ppTreeHandler()
{

}

//-------------------------------------------------------------------------------------------
ppTreeHandler& ppTreeHandler::instance()
{
    // If my single instance was not created yet - create it:
    if (nullptr == m_psMySingleInstance)
    {
        m_psMySingleInstance = new ppTreeHandler;

    }

    return *m_psMySingleInstance;
}

// ---------------------------------------------------------------------------
bool ppTreeHandler::BuildSessionTree(const SessionTreeNodeData& sessionData, QTreeWidgetItem* pTreeItem)
{
    GT_UNREFERENCED_PARAMETER(sessionData);

    bool retVal = false;

    if (nullptr == m_pApplicationTree)
    {
        m_pApplicationTree = GetApplicationTree();
    }

    GT_IF_WITH_ASSERT((pTreeItem != nullptr) && (m_pApplicationTree != nullptr))
    {
        afApplicationTreeItemData* pItemData = m_pApplicationTree->getTreeItemData(pTreeItem);
        GT_IF_WITH_ASSERT(pItemData != nullptr)
        {
            afTreeDataExtension* pTreeItemData = qobject_cast<afTreeDataExtension*>(pItemData->extendedItemData());
            GT_IF_WITH_ASSERT(pTreeItemData != nullptr)
            {
                if (ProfileApplicationTreeHandler::instance()->FindSessionChildItemData(pItemData, AF_TREE_ITEM_PP_TIMELINE) == nullptr)
                {
                    afApplicationTreeItemData* pItemData1 = new afApplicationTreeItemData;
                    pItemData1->m_filePath = pItemData->m_filePath;

                    pItemData1->m_itemType = AF_TREE_ITEM_PP_TIMELINE;
                    pItemData1->setExtendedData(pTreeItemData);
                    pItemData1->m_filePathLineNumber = pItemData->m_filePathLineNumber;
                    QTreeWidgetItem* pItem = m_pApplicationTree->addTreeItem(PP_STR_TreeNodeTimeline, pItemData1, pTreeItem);
                    GT_IF_WITH_ASSERT(pItem != nullptr)
                    {
                        QPixmap* pModulesIcon = ProfileApplicationTreeHandler::instance()->TreeItemTypeToPixmap(pItemData1->m_itemType);
                        pItem->setIcon(0, QIcon(*pModulesIcon));
                    }

                    afApplicationTreeItemData* pItemData2 = new afApplicationTreeItemData;

                    pItemData2->m_itemType = AF_TREE_ITEM_PP_SUMMARY;
                    pItemData2->m_filePath = pItemData1->m_filePath;
                    pItemData2->m_filePathLineNumber = pItemData->m_filePathLineNumber;
                    pItemData2->setExtendedData(pTreeItemData);
                    pItem = m_pApplicationTree->addTreeItem(PP_STR_TreeNodeSummary, pItemData2, pTreeItem);
                    GT_IF_WITH_ASSERT(pItem != nullptr)
                    {
                        QPixmap* pSummaryTypeIcon = ProfileApplicationTreeHandler::instance()->TreeItemTypeToPixmap(pItemData2->m_itemType);
                        pItem->setIcon(0, QIcon(*pSummaryTypeIcon));
                    }

                    pItemData->setExtendedData(pTreeItemData);
                }

                retVal = true;
            }
        }
    }

    return retVal;
}


bool ppTreeHandler::ExtendSessionHTMLPropeties(afTreeItemType sessionTreeItemType, const SessionTreeNodeData* pSessionData, afHTMLContent& htmlContent)
{
    GT_UNREFERENCED_PARAMETER(htmlContent);

    bool retVal = false;

    // Check if this is a session of a session child:
    bool isItemSessionChild = ((sessionTreeItemType == AF_TREE_ITEM_PROFILE_SESSION) || DoesTreeItemTypeBelongsToHandler(sessionTreeItemType));

    const ppSessionTreeNodeData* pPowerSessionData = qobject_cast<const ppSessionTreeNodeData*>(pSessionData);

    //Ignore sessions that aren't mine
    if (pPowerSessionData != nullptr && isItemSessionChild)
    {
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
void ppTreeHandler::InitializeProfileIcons()
{
    // Create the icons
    AddTreeIconToMap(AC_ICON_PROFILE_PWR_APPTREE_SESSION_SINGLE);
    AddTreeIconToMap(AC_ICON_PROFILE_PWR_APPTREE_SESSION_MULTI);
    AddTreeIconToMap(AC_ICON_PROFILE_PWR_APPTREE_SUMMARY);
    AddTreeIconToMap(AC_ICON_PROFILE_PWR_APPTREE_TIMELINE);
    AddTreeIconToMap(AC_ICON_PROFILE_PWR_APPTREE_NEW);
}

// ---------------------------------------------------------------------------
QPixmap* ppTreeHandler::TreeItemTypeToPixmap(afTreeItemType itemType, const QString& sessionTypeAsStr)
{
    QPixmap* pRetPixmap = nullptr;

    acIconId iconId = AC_ICON_EMPTY;
    int sessionTypeIdx = -1;

    if ((AF_TREE_ITEM_PROFILE_SESSION == itemType) || (AF_TREE_ITEM_PROFILE_SESSION_TYPE == itemType))
    {
        if (acGTStringToQString(PP_STR_OnlineProfileName) == sessionTypeAsStr)
        {
            sessionTypeIdx = PP_TREE_ONLINE_SESSION;
        }
    }

    switch (itemType)
    {
        case AF_TREE_ITEM_PROFILE_SESSION:
        {
            switch (sessionTypeIdx)
            {
                case PP_TREE_ONLINE_SESSION:
                    iconId = AC_ICON_PROFILE_PWR_APPTREE_SESSION_SINGLE;
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
                case PP_TREE_ONLINE_SESSION:
                    iconId = AC_ICON_PROFILE_PWR_APPTREE_SESSION_MULTI;
                    break;

                default:
                    break;
            }
        }
        break;

        case AF_TREE_ITEM_PP_SUMMARY:
            iconId = AC_ICON_PROFILE_PWR_APPTREE_SUMMARY;
            break;

        case AF_TREE_ITEM_PP_TIMELINE:
            iconId = AC_ICON_PROFILE_PWR_APPTREE_TIMELINE;
            break;

        default:
            break;
    }

    pRetPixmap = m_iconsMap[iconId];

    return pRetPixmap;
}

// ---------------------------------------------------------------------------
bool ppTreeHandler::GetProfileTypeWithPrefix(const QString& sessionTypeAsStr, gtString& sessionTypeWithPrefix)
{
    GT_UNREFERENCED_PARAMETER(sessionTypeAsStr);
    GT_UNREFERENCED_PARAMETER(sessionTypeWithPrefix);

    bool retVal = true;

    // no prefix for Power profiling at this stage

    return retVal;
}

// ---------------------------------------------------------------------------
bool ppTreeHandler::DoesTreeItemTypeBelongsToHandler(const afTreeItemType itemType) const
{
    bool retVal = false;

    if ((AF_TREE_ITEM_PP_SUMMARY == itemType) || (AF_TREE_ITEM_PP_TIMELINE == itemType))
    {
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
bool ppTreeHandler::DoesTreeNodeDataBelongsToHandler(const SessionTreeNodeData* pSessionData) const
{
    bool retVal = false;

    QString sessionName = pSessionData->m_profileTypeStr;

    retVal = (sessionName == acGTStringToQString(PP_STR_OnlineProfileName));

    return retVal;

}

// ---------------------------------------------------------------------------
afApplicationTree* ppTreeHandler::GetApplicationTree()
{
    afApplicationTree* retVal = nullptr;

    if (nullptr == m_pApplicationTree)
    {
        afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
        GT_IF_WITH_ASSERT(pApplicationCommands != nullptr)
        {
            retVal = pApplicationCommands->applicationTree();
        }
    }
    else
    {
        retVal = m_pApplicationTree;
    }

    return retVal;
}

SessionTreeNodeData* ppTreeHandler::CreateEmptySessionData(afApplicationTreeItemData* pParentData)
{
    SessionTreeNodeData* pRetVal = nullptr;

    // try and reinit the midtier in case it was handled by another instance of codexl or BE
    if (!ppAppController::instance().IsMidTierInitialized() && ppAppController::instance().MidTierInitError() != PPR_NOT_SUPPORTED)
    {
        ppAppController::instance().InitMiddleTier();
    }

    if (ppAppController::instance().IsMidTierInitialized())
    {
        // Create the session data:
        const apProjectSettings& projectSettings = afProjectManager::instance().currentProjectSettings();

        gtString projectName = projectSettings.projectName();
        gtString projectDir = afProjectManager::instance().currentProjectFilePath().fileDirectoryAsString();

        gtString strSessionDisplayName = PM_STR_NewSessionName;
        osDirectory sessionOsDir;
        osFilePath projectDirPath(projectDir);

        // get the next session name and dir from the session naming helper (and clean the dir if there is any old cruft)
        ProfileApplicationTreeHandler::instance()->GetNextSessionNameAndDir(projectName, projectDirPath, strSessionDisplayName, sessionOsDir);

        pRetVal = new ppSessionTreeNodeData();
        pRetVal->m_name = acGTStringToQString(strSessionDisplayName);
        pRetVal->m_displayName = pRetVal->m_name;
        pRetVal->m_profileTypeStr = acGTStringToQString(PP_STR_OnlineProfileName);

        // Sanity check:
        GT_IF_WITH_ASSERT(pParentData != nullptr)
        {
            osFilePath outputFilePath(sessionOsDir.directoryPath());
            outputFilePath.setFileName(strSessionDisplayName);
            outputFilePath.setFileExtension(PP_STR_dbFileExt);
            pParentData->setExtendedData(pRetVal);
            pParentData->m_filePath = outputFilePath;
            pParentData->m_filePathLineNumber = ppSessionController::PP_SESSION_STATE_NEW;

            // Create the session file:
            osFile newSessionFile;
            bool rc = newSessionFile.open(outputFilePath, osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);
            GT_IF_WITH_ASSERT(rc)
            {
                newSessionFile.close();
            }
        }

        pRetVal->m_isImported = false;
        pRetVal->m_projectName = acGTStringToQString(projectName);

        pRetVal->m_commandArguments.clear();

        gtString workDirAsStr = projectSettings.workDirectory().asString();
        pRetVal->m_workingDirectory = acGTStringToQString(workDirAsStr);
        gtString exeName;
        projectSettings.executablePath().getFileNameAndExtension(exeName);
        pRetVal->m_exeName = acGTStringToQString(exeName);
        gtString executablePathAsStr = projectSettings.executablePath().asString();
        pRetVal->m_exeFullPath = acGTStringToQString(executablePathAsStr);

        pRetVal->m_profileScope = PM_PROFILE_SCOPE_SYS_WIDE;
        pRetVal->m_shouldProfileEntireDuration = true;
    }

    return pRetVal;
}
