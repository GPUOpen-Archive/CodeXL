//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpTreeHandler.cpp
///
//==================================================================================

//------------------------------ tpTreeHandler.cpp ------------------------------

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
#include <AMDTSharedProfiling/inc/SharedProfileManager.h>
#include <AMDTSharedProfiling/inc/StringConstants.h>

// Local:
#include <inc/StringConstants.h>
#include <inc/tpTreeHandler.h>

tpTreeHandler* tpTreeHandler::m_psMySingleInstance = nullptr;

// ---------------------------------------------------------------------------
tpTreeHandler::tpTreeHandler() : m_pApplicationTree(nullptr)
{
}

// ---------------------------------------------------------------------------
tpTreeHandler::~tpTreeHandler()
{

}

//-------------------------------------------------------------------------------------------
tpTreeHandler& tpTreeHandler::Instance()
{
    // If my single instance was not created yet - create it:
    if (nullptr == m_psMySingleInstance)
    {
        m_psMySingleInstance = new tpTreeHandler;

    }

    return *m_psMySingleInstance;
}

// ---------------------------------------------------------------------------
bool tpTreeHandler::BuildSessionTree(const SessionTreeNodeData& sessionData, QTreeWidgetItem* pTreeItem)
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

                afApplicationTreeItemData* pItemData1 = new afApplicationTreeItemData;

                pItemData1->m_itemType = AF_TREE_ITEM_TP_OVERVIEW;
                pItemData1->setExtendedData(pTreeItemData);
                QTreeWidgetItem* pItem = m_pApplicationTree->addTreeItem(CP_STR_TreeNodeOverview, pItemData1, pTreeItem);
                GT_IF_WITH_ASSERT(pItem != nullptr)
                {
                    QPixmap* pModulesIcon = ProfileApplicationTreeHandler::instance()->TreeItemTypeToPixmap(pItemData1->m_itemType);
                    pItem->setIcon(0, QIcon(*pModulesIcon));
                }

                afApplicationTreeItemData* pItemData2 = new afApplicationTreeItemData;

                pItemData2->m_itemType = AF_TREE_ITEM_TP_TIMELINE;
                pItemData2->setExtendedData(pTreeItemData);
                pItem = m_pApplicationTree->addTreeItem(CP_STR_TreeNodeTimeline, pItemData2, pTreeItem);
                GT_IF_WITH_ASSERT(pItem != nullptr)
                {
                    QPixmap* pSummaryTypeIcon = ProfileApplicationTreeHandler::instance()->TreeItemTypeToPixmap(pItemData2->m_itemType);
                    pItem->setIcon(0, QIcon(*pSummaryTypeIcon));
                }
                retVal = true;
            }
        }
    }

    return retVal;
}


bool tpTreeHandler::ExtendSessionHTMLPropeties(afTreeItemType sessionTreeItemType, const SessionTreeNodeData* pSessionData, afHTMLContent& htmlContent)
{
    GT_UNREFERENCED_PARAMETER(htmlContent);

    bool retVal = false;

    // Check if this is a session of a session child:
    bool isItemSessionChild = ((sessionTreeItemType == AF_TREE_ITEM_PROFILE_SESSION) || DoesTreeItemTypeBelongsToHandler(sessionTreeItemType));

    const tpSessionTreeNodeData* pThreadSessionData = qobject_cast<const tpSessionTreeNodeData*>(pSessionData);

    //Ignore sessions that aren't mine
    if (pThreadSessionData != nullptr && isItemSessionChild)
    {
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
void tpTreeHandler::InitializeProfileIcons()
{
    // Create the icons
    AddTreeIconToMap(AC_ICON_PROFILE_THR_APPTREE_SESSION_SINGLE);
    AddTreeIconToMap(AC_ICON_PROFILE_THR_APPTREE_SESSION_MULTI);
    AddTreeIconToMap(AC_ICON_PROFILE_THR_APPTREE_OVERVIEW);
    AddTreeIconToMap(AC_ICON_PROFILE_THR_APPTREE_TIMELINE);
}

// ---------------------------------------------------------------------------
QPixmap* tpTreeHandler::TreeItemTypeToPixmap(afTreeItemType itemType, const QString& sessionTypeAsStr)
{
    GT_UNREFERENCED_PARAMETER(sessionTypeAsStr);

    QPixmap* pRetPixmap = nullptr;

    acIconId iconId = AC_ICON_EMPTY;

    switch (itemType)
    {
        case AF_TREE_ITEM_TP_OVERVIEW:
            iconId = AC_ICON_PROFILE_THR_APPTREE_OVERVIEW;
            break;

        case AF_TREE_ITEM_TP_TIMELINE:
            iconId = AC_ICON_PROFILE_THR_APPTREE_TIMELINE;
            break;

        default:
            break;

    }

    pRetPixmap = m_iconsMap[iconId];


    return pRetPixmap;
}

// ---------------------------------------------------------------------------
bool tpTreeHandler::GetProfileTypeWithPrefix(const QString& sessionTypeAsStr, gtString& sessionTypeWithPrefix)
{
    bool retVal = true;

    if (PM_profileTypeThreadProfile == sessionTypeAsStr)
    {
        sessionTypeWithPrefix.prepend(L"CPU: ");
        retVal = true;
    }


    return retVal;
}

// ---------------------------------------------------------------------------
bool tpTreeHandler::DoesTreeItemTypeBelongsToHandler(const afTreeItemType itemType) const
{
    bool retVal = false;

    if ((AF_TREE_ITEM_PP_SUMMARY == itemType) || (AF_TREE_ITEM_PP_TIMELINE == itemType))
    {
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
bool tpTreeHandler::DoesTreeNodeDataBelongsToHandler(const SessionTreeNodeData* pSessionData) const
{
    GT_UNREFERENCED_PARAMETER(pSessionData);

    bool retVal = false;

#pragma message ("TODO: TP :")

    return retVal;

}

// ---------------------------------------------------------------------------
afApplicationTree* tpTreeHandler::GetApplicationTree()
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

SessionTreeNodeData* tpTreeHandler::CreateEmptySessionData(afApplicationTreeItemData* pParentData)
{
    SessionTreeNodeData* pRetVal = nullptr;

    // Create the session data:
    const apProjectSettings& projectSettings = afProjectManager::instance().currentProjectSettings();

    gtString projectName = projectSettings.projectName();
    gtString projectDir = afProjectManager::instance().currentProjectFilePath().fileDirectoryAsString();

    gtString strSessionDisplayName = PM_STR_NewSessionName;
    osDirectory sessionOsDir;
    osFilePath projectDirPath(projectDir);

    // get the next session name and dir from the session naming helper (and clean the dir if there is any old cruft)
    ProfileApplicationTreeHandler::instance()->GetNextSessionNameAndDir(projectName, projectDirPath, strSessionDisplayName, sessionOsDir);

    pRetVal = new tpSessionTreeNodeData();
    pRetVal->m_name = acGTStringToQString(strSessionDisplayName);
    pRetVal->m_displayName = pRetVal->m_name;
    pRetVal->m_profileTypeStr = acGTStringToQString(CP_STR_ThreadProfileName);

    // Sanity check:
    GT_IF_WITH_ASSERT(pParentData != nullptr)
    {
        osFilePath outputFilePath(sessionOsDir.directoryPath());
        outputFilePath.setFileName(strSessionDisplayName);
        outputFilePath.setFileExtension(AF_STR_profileFileExtension7);
        pParentData->setExtendedData(pRetVal);
        pRetVal->m_pParentData->m_filePath = outputFilePath;

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

    return pRetVal;
}

tpSessionTreeNodeData::tpSessionTreeNodeData() : m_pid(0)
{

}
