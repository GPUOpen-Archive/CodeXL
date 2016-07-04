//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ProfileApplicationTreeHandler.cpp
///
//==================================================================================

//
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtCore>
#include <QtWidgets>
#include <QStringList>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QAbstractButton>
#include <QPushButton>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QProcess>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apMonitoredObjectsTreeEvent.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osFileLauncher.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acIcons.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>


// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afBrowseAction.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/afPluginConnectionManager.h>
#include <AMDTApplicationFramework/Include/afQtCreatorsManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

// AMDTSharedProfiler:
#include <inc/ProfileApplicationTreeHandler.h>
#include <inc/SharedProfileManager.h>
#include <inc/SharedProfileSettingPage.h>
#include <inc/StringConstants.h>

//Static
ExplorerSessionId ProfileApplicationTreeHandler::m_sSessionCount = 0;
ProfileApplicationTreeHandler* ProfileApplicationTreeHandler::m_pMySingleInstance = nullptr;

ProfileApplicationTreeHandler::ProfileApplicationTreeHandler() : afApplicationTreeHandler(),
    afBaseView(&afProgressBarWrapper::instance()), m_pEmptySessionItemData(nullptr)
{
    m_pApplicationTree = nullptr;

    // Register as an events observer
    // Note: choosing a higher priority handler code so that the Session Explorer is called (to delete sessions)
    // before the two profilers are called (to add sessions for the newly opened project)
    apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_MANAGERS_EVENTS_HANDLING_PRIORITY);

    // Initialize the context menu for tree items:
    m_pOpenItemAction = new QAction(PM_STR_TREE_OPEN_ITEM, m_pApplicationTree);

    m_pMultipleSessionDeleteAction = new QAction(PM_STR_TREE_DELETE_MULTIPLE_SESSION, m_pApplicationTree);

    m_pSessionRenameAction = new QAction(PM_STR_TREE_RENAME_SESSION, m_pApplicationTree);
    m_pSessionDeleteAction = new QAction(PM_STR_TREE_DELETE_SESSION, m_pApplicationTree);

    m_pSessionRenameAction->setShortcut(QKeySequence("F2"));
    m_pSessionRenameAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    m_pSessionDeleteAction->setShortcut(QKeySequence(QKeySequence::Delete));
    m_pSessionDeleteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);

    m_pOpenFolderAction = new QAction(PM_STR_TREE_OPEN_CONTAINING_FOLDER, m_pApplicationTree);

    // global context menu items
    m_pDeleteAllSessionsAction = new QAction(PM_STR_TREE_DELETE_ALL_SESSIONS, m_pApplicationTree);

    m_pImportSessionAction = new afBrowseAction(PM_STR_lastBrowsedExportFolder);
    m_pImportSessionAction->setText(PM_STR_TREE_IMPORT_SESSION);
    m_pImportSessionAction->setParent(m_pApplicationTree);

    m_pExportSessionAction = new afBrowseAction(PM_STR_lastBrowsedExportFolder);
    m_pExportSessionAction->setText(PM_STR_TREE_EXPORT_SESSION);
    m_pExportSessionAction->setParent(m_pApplicationTree);

    m_pRefreshFromServerAction = new QAction(PM_STR_TREE_REFRESH_FROM_SERVER, m_pApplicationTree);
    m_pRefreshFromServerAction->setParent(m_pApplicationTree);

    bool rc = connect(m_pOpenItemAction, SIGNAL(triggered()), this, SLOT(OnItemOpen()));
    GT_ASSERT(rc);

    rc = connect(m_pSessionRenameAction, SIGNAL(triggered()), this, SLOT(OnSessionRename()));
    GT_ASSERT(rc);

    rc = connect(m_pSessionDeleteAction, SIGNAL(triggered()), this, SLOT(OnSessionDelete()));
    GT_ASSERT(rc);

    rc = connect(m_pMultipleSessionDeleteAction, SIGNAL(triggered()), this, SLOT(OnMultipleSessionDelete()));
    GT_ASSERT(rc);

    rc = connect(m_pOpenFolderAction, SIGNAL(triggered()), this, SLOT(OnOpenContainingFolder()));
    GT_ASSERT(rc);

    rc = connect(m_pImportSessionAction, SIGNAL(triggered()), this, SLOT(OnImportSession()));
    GT_ASSERT(rc);

    rc = connect(m_pExportSessionAction, SIGNAL(triggered()), this, SLOT(OnExportSession()));
    GT_ASSERT(rc);

    rc = connect(m_pRefreshFromServerAction, SIGNAL(triggered()), this, SLOT(OnRefreshFromServer()));
    GT_ASSERT(rc);

    rc = connect(m_pDeleteAllSessionsAction, SIGNAL(triggered()), this, SLOT(OnDeleteAllSessions()));
    GT_ASSERT(rc);

    m_bImportInProgress = false;
    m_startActionLastState = false;

    // Initialize the icons:
    InitializeProfileIcons();

    m_isTreeInitialized = false;

}

ProfileApplicationTreeHandler::~ProfileApplicationTreeHandler()
{
    for (QList<FileFilter*>::iterator i = m_profileFilterList.begin(); i != m_profileFilterList.end(); ++i)
    {
        delete *i;
    }

    m_profileFilterList.clear();

    for (QList<FileFilter*>::iterator i = m_frameAnalysisFilterList.begin(); i != m_frameAnalysisFilterList.end(); ++i)
    {
        delete *i;
    }

    m_frameAnalysisFilterList.clear();


    // Unregister as an events observer
    apEventsHandler::instance().unregisterEventsObserver(*this);

    delete m_pImportSessionAction;
    delete m_pExportSessionAction;
}

void ProfileApplicationTreeHandler::OnDeleteAllSessions()
{
    GT_IF_WITH_ASSERT(nullptr != m_pApplicationTree)
    {
        QList<QTreeWidgetItem*> treeSelectedItems = m_pApplicationTree->treeControl()->selectedItems();
        QTreeWidgetItem* pContextMenuItem = treeSelectedItems.at(0);
        GT_IF_WITH_ASSERT(nullptr != pContextMenuItem)
        {
            afApplicationTreeItemData* pItemData = m_pApplicationTree->getTreeItemData(pContextMenuItem);
            GT_IF_WITH_ASSERT(nullptr != pItemData)
            {
                // If this is the root then all sessions should be deleted:
                if (AF_TREE_ITEM_APP_ROOT == pItemData->m_itemType)
                {
                    QMessageBox::StandardButton userAnswer = acMessageBox::instance().question(afGlobalVariablesManager::ProductNameA(), PS_STR_TREE_DELETE_ALL_QUESTION, QMessageBox::Yes | QMessageBox::No);

                    if (userAnswer == QMessageBox::Yes)
                    {
                        DeleteAllSessions(nullptr, SESSION_EXPLORER_REMOVE_FILES);
                    }
                }
                else
                {
                    pItemData = m_pApplicationTree->getTreeItemData(pContextMenuItem);
                    GT_IF_WITH_ASSERT(nullptr != pItemData)
                    {
                        if (AF_TREE_ITEM_PROFILE_SESSION_TYPE == pItemData->m_itemType)
                        {
                            QString profileType = pItemData->m_pTreeWidgetItem->text(0);
                            QString message(QString(PS_STR_TREE_DELETE_ALL_FROM_TYPE_QUESTION).arg(profileType));
                            QMessageBox::StandardButton userAnswer = acMessageBox::instance().question(afGlobalVariablesManager::ProductNameA(), message, QMessageBox::Yes | QMessageBox::No);

                            if (userAnswer == QMessageBox::Yes)
                            {
                                DeleteAllSessions(pItemData->m_pTreeWidgetItem, SESSION_EXPLORER_REMOVE_FILES);
                            }
                        }
                    }
                }
            }
        }
    }
}

void ProfileApplicationTreeHandler::DeleteAllSessions(QTreeWidgetItem* pProfileTypeItem, SessionExplorerDeleteType deleteType)
{
    GT_IF_WITH_ASSERT(nullptr != m_pApplicationTree)
    {
        // Reset the tree history list:
        m_pApplicationTree->resetLastSelectedItem();

        QTreeWidgetItem* pRootItem = m_pApplicationTree->headerItem();

        if (nullptr != pRootItem)
        {
            int profileTypesCount = pRootItem->childCount();

            for (int i = profileTypesCount - 1; i >= 0; i--)
            {
                QTreeWidgetItem* pChild = pRootItem->child(i);

                if (nullptr != pChild)
                {
                    // If the profile type item is specified, delete only it's children:
                    bool shouldDeleteParentChildren = true;

                    if (nullptr != pProfileTypeItem)
                    {
                        shouldDeleteParentChildren = (pChild == pProfileTypeItem);
                    }

                    if (shouldDeleteParentChildren)
                    {
                        // Get the amount of sessions for this profile type:
                        int sessionCount = pChild->childCount();

                        // Remove children in reverse order:
                        for (int j = sessionCount - 1; j >= 0; j--)
                        {
                            QTreeWidgetItem* pSessionItem = pChild->child(j);
                            GT_IF_WITH_ASSERT(nullptr != pSessionItem)
                            {
                                afApplicationTreeItemData* pItemData = m_pApplicationTree->getTreeItemData(pSessionItem);
                                GT_IF_WITH_ASSERT(nullptr != pItemData)
                                {
                                    // Do not delete empty session:
                                    bool isEmptySession = (AF_TREE_ITEM_PROFILE_EMPTY_SESSION == pItemData->m_itemType);

                                    if (!isEmptySession)
                                    {
                                        // Check if this is a session node:
                                        SessionTreeNodeData* pSessionData = qobject_cast<SessionTreeNodeData*>(pItemData->extendedItemData());

                                        if ((nullptr != pSessionData) || (AF_TREE_ITEM_PROFILE_SESSION == pItemData->m_itemType))
                                        {
                                            DeleteSessionNode(pItemData, deleteType, false);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Save the project after the sessions deletion:
            afApplicationCommands::instance()->OnFileSaveProject();

            // Clear the history list:
            m_pApplicationTree->resetLastSelectedItem();

            // If files should be deleted, remove the project profiles session folder:
            if (deleteType == SESSION_EXPLORER_REMOVE_FILES)
            {
                osFilePath projectFilePath;
                afGetUserDataFolderPath(projectFilePath);

                // Add the "ProjectName_ProfileOutput" to the folder:
                gtString projectProfilesLocation = afProjectManager::instance().currentProjectSettings().projectName();
                projectProfilesLocation += AF_STR_ProfileDirExtension;
                projectFilePath.appendSubDirectory(projectProfilesLocation);
                osDirectory projectDir;
                bool rc = projectFilePath.getFileDirectory(projectDir);
                GT_IF_WITH_ASSERT(rc)
                {
                    // Check if the folder is empty:
                    gtList<osFilePath> dirEntries;
                    projectDir.getContainedFilePaths(L"*", osDirectory::SORT_BY_NAME_ASCENDING, dirEntries);
                    bool isEmptyDir = dirEntries.length() == 0;

                    if (isEmptyDir)
                    {
                        projectDir.getSubDirectoriesPaths(osDirectory::SORT_BY_NAME_ASCENDING, dirEntries);
                        isEmptyDir = dirEntries.length() == 0;

                        if (isEmptyDir && projectDir.exists())
                        {
                            rc = projectDir.deleteRecursively();
                            GT_ASSERT(rc);
                        }
                    }
                }
            }
        }
    }
}


void ProfileApplicationTreeHandler::OnImportSession()
{
    QString filters;
    QString allFilters;

    bool isFrameAnalysisSession = afExecutionModeManager::instance().isActiveMode(PM_STR_FrameAnalysisMode);

    if (isFrameAnalysisSession)
    {
        for (QList<FileFilter*>::const_iterator i = m_frameAnalysisFilterList.begin(); i != m_frameAnalysisFilterList.end(); ++i)
        {
            if (!filters.isEmpty())
            {
                filters.append(";;");
            }

            filters.append(QString("%1 (%2)").arg((*i)->m_strDescription).arg((*i)->m_strMask));
        }

        if (!allFilters.isEmpty())
        {
            if (!filters.isEmpty())
            {
                filters.prepend(";;");
            }

            filters.prepend(QString(tr("Frame Analysis Session Files (%1)")).arg(allFilters));
        }
    }
    else
    {
        for (QList<FileFilter*>::const_iterator i = m_profileFilterList.begin(); i != m_profileFilterList.end(); ++i)
        {
            if (!filters.isEmpty())
            {
                filters.append(";;");
            }

            filters.append(QString("%1 (%2)").arg((*i)->m_strDescription).arg((*i)->m_strMask));

            if (!allFilters.isEmpty())
            {
                allFilters.append(" ");
            }

            allFilters += (*i)->m_strMask;
        }

        if (!allFilters.isEmpty())
        {
            if (!filters.isEmpty())
            {
                filters.prepend(";;");
            }

            filters.prepend(QString(tr("Profile Session Files (%1)")).arg(allFilters));
        }

        if (!filters.isEmpty())
        {
            filters.append(";;");
            filters.append(AF_STR_allFileDetails);
        }
    }



    QString strDir;
    strDir = QString::fromWCharArray(afProjectManager::instance().currentProjectFilePath().fileDirectoryAsString().asCharArray());

    // Set the CodeXL projects location as default browse path for the imported file:
    if (m_pImportSessionAction->LastBrowsedFolder().isEmpty())
    {
        QString currentProjectPath = acGTStringToQString(afProjectManager::instance().currentProjectFilePath().fileDirectoryAsString());
        m_pImportSessionAction->SetLastBrowsedFolder(currentProjectPath);
    }

    QString dialogTitle(isFrameAnalysisSession ? PM_STR_ImportFrameAnalysisDialogTitle : PM_STR_ImportDialogTitle);
    QString defaultPath("");
    QString importFilePath = afApplicationCommands::instance()->ShowFileSelectionDialog(dialogTitle, defaultPath, filters, m_pImportSessionAction);

    //on open file dialog not pressed cancell button
    if (false == importFilePath.isNull())
    {
        osFilePath filePath;
        acQStringToOSFilePath(importFilePath, filePath);

        // Look for an item with the same file path:
        afApplicationTreeItemData* pExistingItemData = FindItemByProfileFilePath(filePath);

        if (nullptr != pExistingItemData)
        {
            acMessageBox::instance().information(AF_STR_InformationA, PM_STR_PROFILE_TREE_SESSION_EXIST);
            m_pApplicationTree->expandItem(pExistingItemData->m_pTreeWidgetItem);
            m_pApplicationTree->selectItem(pExistingItemData, true);
        }
        else
        {
            // Import the file:
            ImportFile(importFilePath);
        }
    }

}

bool ProfileApplicationTreeHandler::ImportFile(const QString& importFilePath)
{
    // Check if these is a loaded project:
    bool isProjectLoaded = !afProjectManager::instance().currentProjectSettings().projectName().isEmpty();

    if (!isProjectLoaded)
    {
        afApplicationCommands::instance()->CreateDefaultProject(PM_STR_PROFILE_MODE);
    }

    ImportStarted();
    bool success = false;
    emit FileImported(importFilePath, success);
    return success;
}

void ProfileApplicationTreeHandler::OnExportSession()
{
    GT_IF_WITH_ASSERT(nullptr != m_pApplicationTree && nullptr != m_pApplicationTree->treeControl())
    {
        QList<QTreeWidgetItem*> treeSelectedItems = m_pApplicationTree->treeControl()->selectedItems();

        if (treeSelectedItems.count() == 1)
        {
            QTreeWidgetItem* pContextMenuItem = treeSelectedItems.at(0);
            GT_IF_WITH_ASSERT(nullptr != pContextMenuItem)
            {
                afApplicationTreeItemData* pTreeItemData = m_pApplicationTree->getTreeItemData(pContextMenuItem);
                GT_IF_WITH_ASSERT(nullptr != pTreeItemData)
                {
                    afApplicationTreeItemData* pNodeData = FindParentSessionItemData(pTreeItemData);

                    GT_IF_WITH_ASSERT(nullptr != pNodeData)
                    {
                        QString report;
                        osDirectory sessionDir;
                        pNodeData->m_filePath.getFileDirectory(sessionDir);
                        SessionTreeNodeData* pSessionData = qobject_cast<SessionTreeNodeData*>(pNodeData->extendedItemData());

                        if (nullptr != pSessionData && nullptr != m_pExportSessionAction)
                        {
                            QString sessionName = pSessionData->m_displayName;

                            QString strDir;
                            strDir = QString::fromWCharArray(afProjectManager::instance().currentProjectFilePath().fileDirectoryAsString().asCharArray());

                            // Set the CodeXL projects location as default browse path for the exported file:
                            if (m_pExportSessionAction->LastBrowsedFolder().isEmpty())
                            {
                                QString currentProjectPath = acGTStringToQString(afProjectManager::instance().currentProjectFilePath().fileDirectoryAsString());
                                m_pExportSessionAction->SetLastBrowsedFolder(currentProjectPath);
                            }

                            QString dialogTitle(PM_STR_ExportDialogTitle);
                            QString defaultPath("");
                            QString fullFileName = sessionName;
                            fullFileName.append(AF_STR_HyphenA);
                            fullFileName.append(afGlobalVariablesManager::ProductNameA());
                            fullFileName.append("." + acGTStringToQString(AF_STR_frameAnalysisArchivedFileExtension));

                            bool ret = afApplicationCommands::instance()->ShowQTSaveFileDialog(defaultPath, fullFileName, m_pExportSessionAction->LastBrowsedFolder(), m_pApplicationTree, "", dialogTitle);

                            if (ret == true)
                            {
                                QString pathToSave = defaultPath;
                                int lastIndex = pathToSave.lastIndexOf("/");
                                pathToSave.chop(pathToSave.length() - lastIndex);
                                m_pExportSessionAction->SetLastBrowsedFolder(pathToSave);

                                gtList<SessionTypeTreeHandlerAbstract*>::iterator it = m_sessionTypeToTreeHandlerList.begin();

                                for (; (it != m_sessionTypeToTreeHandlerList.end()); it++)
                                {
                                    ret = (*it)->ExportFile(sessionDir, defaultPath, pSessionData);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void ProfileApplicationTreeHandler::OnRefreshFromServer()
{
    // This is an ugly workaround. Currently, the only object that can build and handle context menus is ProfileApplicationTreeHandler.
    // It is supposed to handle context menus for all profilers, and for the frame analysis mode.
    // In the future we will need to refine this implementation, until then we will need to find the tree handler that handles frame analysis, and call it
    gtList<SessionTypeTreeHandlerAbstract*>::iterator it = m_sessionTypeToTreeHandlerList.begin();

    for (; (it != m_sessionTypeToTreeHandlerList.end()); it++)
    {
        // Break on the first handler that handles it
        if ((*it)->RefreshSessionsFromServer())
        {
            break;
        }
    }
}

void ProfileApplicationTreeHandler::OnSessionDelete()
{
    // Get the clicked item from application tree:
    GT_IF_WITH_ASSERT(nullptr != m_pApplicationTree && nullptr != m_pApplicationTree->treeControl())
    {
        QList<QTreeWidgetItem*> treeSelectedItems = m_pApplicationTree->treeControl()->selectedItems();

        if (treeSelectedItems.count() > 0)
        {
            QTreeWidgetItem* pContextMenuItem = treeSelectedItems.at(0);
            GT_IF_WITH_ASSERT(nullptr != pContextMenuItem)
            {
                afApplicationTreeItemData* pTreeItemData = m_pApplicationTree->getTreeItemData(pContextMenuItem);
                GT_IF_WITH_ASSERT(nullptr != pTreeItemData)
                {
                    afApplicationTreeItemData* pSessionItemData = FindParentSessionItemData(pTreeItemData);

                    if ((nullptr != pSessionItemData) && (nullptr != pSessionItemData->m_pTreeWidgetItem))
                    {
                        // Disable the deletion of the empty session:
                        if (AF_TREE_ITEM_PROFILE_EMPTY_SESSION != pSessionItemData->m_itemType)
                        {
                            QMessageBox::StandardButton result;
                            QString message(QString(PS_STR_TREE_DELETE_QUESTION).arg(pSessionItemData->m_pTreeWidgetItem->text(0)));
                            result = acMessageBox::instance().question(afGlobalVariablesManager::ProductNameA(), message, QMessageBox::Yes | QMessageBox::No);

                            if (QMessageBox::Yes == result)
                            {
                                // Delete the session, and save the project:
                                DeleteSessionNode(pSessionItemData, SESSION_EXPLORER_REMOVE_FILES, true);
                            }
                        }
                    }
                }
            }
        }
    }
}


void ProfileApplicationTreeHandler::OnMultipleSessionDelete()
{
    // Get the clicked item from application tree:
    GT_IF_WITH_ASSERT(nullptr != m_pApplicationTree && nullptr != m_pApplicationTree->treeControl())
    {
        QMessageBox::StandardButton result;
        result = acMessageBox::instance().question(afGlobalVariablesManager::ProductNameA(), PS_STR_TREE_DELETE_MULTIPLE_QUESTION, QMessageBox::Yes | QMessageBox::No);

        if (QMessageBox::Yes == result)
        {
            QList<QTreeWidgetItem*> treeSelectedItems = m_pApplicationTree->treeControl()->selectedItems();

            foreach (QTreeWidgetItem* pContextMenuItem, treeSelectedItems)
            {
                if (nullptr != pContextMenuItem)
                {
                    afApplicationTreeItemData* pTreeItemData = m_pApplicationTree->getTreeItemData(pContextMenuItem);
                    GT_IF_WITH_ASSERT(nullptr != pTreeItemData)
                    {
                        afApplicationTreeItemData* pSessionItemData = FindParentSessionItemData(pTreeItemData);
                        GT_IF_WITH_ASSERT((nullptr != pSessionItemData) && (nullptr != pSessionItemData->m_pTreeWidgetItem))
                        {
                            // Delete the current session. Do not save the project (save it after the multiple deletion):
                            DeleteSessionNode(pSessionItemData, SESSION_EXPLORER_REMOVE_FILES, false);
                        }
                    }
                }
            }

            // Save the project:
            afApplicationCommands::instance()->OnFileSaveProject();
        }
    }
}

void ProfileApplicationTreeHandler::OnItemOpen()
{
    // Get the clicked item from application tree:
    GT_IF_WITH_ASSERT(nullptr != m_pApplicationTree && nullptr != m_pApplicationTree->treeControl())
    {
        QList<QTreeWidgetItem*> treeSelectedItems = m_pApplicationTree->treeControl()->selectedItems();
        QTreeWidgetItem* pContextMenuItem = treeSelectedItems.at(0);
        GT_IF_WITH_ASSERT(nullptr != pContextMenuItem)
        {
            afApplicationTreeItemData* pItemData = m_pApplicationTree->getTreeItemData(pContextMenuItem);
            GT_IF_WITH_ASSERT(nullptr != pItemData)
            {
                // Expand and activate the session item:
                m_pApplicationTree->expandItem(pItemData->m_pTreeWidgetItem);
                m_pApplicationTree->selectItem(pItemData, true);
            }
        }
    }
}


void ProfileApplicationTreeHandler::OnOpenContainingFolder()
{
    // Get the clicked item from application tree:
    GT_IF_WITH_ASSERT(nullptr != m_pApplicationTree && nullptr != m_pApplicationTree->treeControl())
    {
        QList<QTreeWidgetItem*> treeSelectedItems = m_pApplicationTree->treeControl()->selectedItems();
        QTreeWidgetItem* pContextMenuItem = treeSelectedItems.at(0);
        GT_IF_WITH_ASSERT(nullptr != pContextMenuItem)
        {
            afApplicationTreeItemData* pItemData = m_pApplicationTree->getTreeItemData(pContextMenuItem);
            GT_IF_WITH_ASSERT(nullptr != pItemData)
            {
                SessionTreeNodeData* pSessionData = qobject_cast<SessionTreeNodeData*>(pItemData->extendedItemData());
                GT_IF_WITH_ASSERT(pSessionData != nullptr)
                {
                    osDirectory sessionDir = pSessionData->SessionDir();
                    // Open the  file:
                    osFileLauncher fileLauncher(sessionDir.directoryPath().asString().asCharArray());
                    fileLauncher.launchFile();
                }
            }
        }
    }
}

void ProfileApplicationTreeHandler::CurrentTreeNodeChanged(QTreeWidgetItem* pItem)
{
    if (nullptr != pItem && nullptr != m_pApplicationTree)
    {
        afApplicationTreeItemData* pItemData = m_pApplicationTree->getTreeItemData(pItem);

        if (nullptr != pItemData)
        {
            SessionTreeNodeData* pSessionNodeData = qobject_cast<SessionTreeNodeData*>(pItemData->extendedItemData());

            if (nullptr != pSessionNodeData)
            {
                emit SessionSelected(pSessionNodeData->m_sessionId);
            }

            afApplicationTreeItemData* pRelatedSessionItemData = nullptr;

            if (AF_TREE_ITEM_PROFILE_SESSION == pItemData->m_itemType)
            {
                pRelatedSessionItemData = pItemData;
            }
            else
            {
                pRelatedSessionItemData = FindParentItemForSelection(pItemData);
            }

            if (nullptr != pRelatedSessionItemData)
            {
                // Set the related profile type item as bold:
                SetItemFontBold(pRelatedSessionItemData);
            }
        }
    }
}

ExplorerSessionId ProfileApplicationTreeHandler::AddSession(SessionTreeNodeData* pNewItemSessionData, bool activateItem)
{
    ExplorerSessionId retVal = SESSION_ID_ERROR;

    if (!WasTreeCreated())
    {
        InitializeApplicationTree();
    }

    if (WasTreeCreated())
    {
        retVal = ++m_sSessionCount;
        GT_IF_WITH_ASSERT(retVal != SESSION_ID_ERROR)
        {
            bool rc = AddSessionTreeNode(retVal, pNewItemSessionData);
            GT_IF_WITH_ASSERT(rc)
            {
                // add the session to the naming helper using the session dir
                std::wstring tmpParentNodeName(pNewItemSessionData->m_projectName.toStdWString());
                gtString strSessionDir = pNewItemSessionData->SessionDir().directoryPath().asString();
                strSessionDir = strSessionDir.truncate(strSessionDir.reverseFind(osFilePath::osPathSeparator) + 1, -1);
                pNewItemSessionData->m_sessionId = retVal;

                if (activateItem)
                {
                    afApplicationTreeItemData* pSessionItemData = GetSessionNodeItemData(retVal);
                    GT_IF_WITH_ASSERT(nullptr != pSessionItemData && nullptr != m_pApplicationTree)
                    {
                        // Expand and activate the session item:
                        m_pApplicationTree->expandItem(pSessionItemData->m_pTreeWidgetItem);
                        m_pApplicationTree->selectItem(pSessionItemData, true);
                    }
                }
            }

            // Add empty session node to tree for power profile sessions:
            if (pNewItemSessionData->m_profileTypeStr == PM_STR_OnlineProfileName)
            {
                AddEmptySessionCreationNode(false);
            }
        }

        if (m_bImportInProgress)
        {
            onFileImportedComplete();
        }

        // Emit a signal indicating that a session was added.
        // This signal is emitted since we use the data in CodeXL Explorer to identify sessions and load them into VS:
        emit SessionAddedToTree();
    }

    return retVal;
}

void ProfileApplicationTreeHandler::AddImportFileFilter(const QString& strDescription, const QString& strFileMask, const gtString& modeName)
{
    FileFilter* fileFilter = new FileFilter(strDescription, strFileMask);

    if (modeName.compare(PM_STR_FrameAnalysisMode) == 0)
    {
        m_frameAnalysisFilterList.push_back(fileFilter);
    }
    else
    {
        m_profileFilterList.push_back(fileFilter);
    }
}

bool ProfileApplicationTreeHandler::IsDragDropSupported(QDropEvent* event, QString& dragDropFile, bool& shouldAccpet)
{
    bool retVal = false;

    shouldAccpet = false;

    // We only allow drag and drop of sessions when no process is running:
    bool isProcessRunning = (afPluginConnectionManager::instance().getCurrentRunModeMask() & AF_DEBUGGED_PROCESS_EXISTS);

    if (!isProcessRunning)
    {
        // Check if this file type is supported by the profile extension:
        const QMimeData* mimeData = event->mimeData();

        if (mimeData->hasUrls())
        {
            if (mimeData->urls().size() == 1)
            {
                QList<QUrl> urlList = mimeData->urls();

                for (int i = 0; i < urlList.size() && i < 32; ++i)
                {
                    QString importFilePath = urlList.at(i).toLocalFile();
                    QFileInfo fileInfo(importFilePath);

                    if (fileInfo.exists() && CheckFilterList(fileInfo.suffix()))
                    {
                        shouldAccpet = true;
                        dragDropFile = importFilePath;
                        retVal = true;
                    }
                }
            }
        }

        if (shouldAccpet)
        {
            QString msg;

            // Allow import (drag and drop) only when plugins are registered, and when there is a loaded project:
            bool canImport = !m_profileFilterList.isEmpty();

            retVal = isImportingOkWithCurrentMode(msg) && canImport;

            if (!retVal)
            {
                shouldAccpet = false;
                acMessageBox::instance().information(afGlobalVariablesManager::ProductNameA(), msg, QMessageBox::Ok);
            }
        }
    }

    return retVal;
}

bool ProfileApplicationTreeHandler::isImportingOkWithCurrentMode(QString& msg)
{
    bool retVal = false;

    bool isProjectLoaded = !afProjectManager::instance().currentProjectSettings().projectName().isEmpty();

    // Disable import while import is in progress:
    if (m_bImportInProgress)
    {
        msg = PM_STR_ImportWarningImporting;
    }

    // Disable import while not in profile mode (check mode only when a project is loaded):
    else if (!(afExecutionModeManager::instance().isActiveMode(PM_STR_PROFILE_MODE) || afExecutionModeManager::instance().isActiveMode(PM_STR_FrameAnalysisMode)) && isProjectLoaded)
    {
        msg = PM_STR_ImportWarning;
    }

    // Disable import while process is running:
    else if (SharedProfileManager::instance().isProfiling())
    {
        msg = PM_STR_ImportWarning;
    }

    // Disable import while we are processing a profile session:
    else if (!SharedProfileManager::instance().IsProfileComplete())
    {
        msg = PM_STR_ImportWarningProcessing;
    }
    else
    {
        retVal = true;
    }

    return retVal;
}

bool ProfileApplicationTreeHandler::CheckFilterList(const QString& fileExt)
{
    bool retVal = false;

    bool isInProfileMode = afExecutionModeManager::instance().isActiveMode(PM_STR_PROFILE_MODE);

    if (m_profileFilterList.count() > 0 && isInProfileMode)
    {
        foreach (FileFilter* pFilter, m_profileFilterList)
        {
            if (nullptr != pFilter)
            {
                QFileInfo fileInfo(pFilter->m_strMask);

                if (fileInfo.suffix() == fileExt)
                {
                    return true;
                }
            }
        }
    }


    bool isInFrameAnalysisMode = afExecutionModeManager::instance().isActiveMode(PM_STR_FrameAnalysisMode);

    if (m_frameAnalysisFilterList.count() > 0 && isInFrameAnalysisMode)
    {
        foreach (FileFilter* filter, m_frameAnalysisFilterList)
        {
            QFileInfo fileInfo(filter->m_strMask);

            if (fileInfo.suffix() == fileExt)
            {
                return true;
            }
        }
    }

    return retVal;
}

void ProfileApplicationTreeHandler::DeleteSessionNode(afApplicationTreeItemData* pNodeData, SessionExplorerDeleteType deleteType, bool saveProjectAfterDeletion)
{
    GT_IF_WITH_ASSERT((nullptr != pNodeData) && (nullptr != m_pApplicationTree))
    {
        SessionTreeNodeData* pSessionData = qobject_cast<SessionTreeNodeData*>(pNodeData->extendedItemData());

        if (nullptr != pSessionData)
        {
            QString sessionName = pSessionData->m_displayName;

            bool canDelete = true;

            if (!sessionName.trimmed().isEmpty())
            {
                // Emit a signal informing the profiler plug-ins that the session is about to be deleted.
                // This signal handling is:
                // 1. Performing any action required for the session deletion
                // 2. Asks the user for permission to close if needed
                // 3. Assigns 'canDelete' true / false iff the session can be deleted
                emit SessionDeleted(pSessionData->m_sessionId, SESSION_EXPLORER_REMOVE_FROM_TREE_ONLY, canDelete);
            }

            // If files should be deleted, delete it:
            if ((deleteType == SESSION_EXPLORER_REMOVE_FILES) && canDelete)
            {
                QString report;
                osDirectory sessionDir;
                pNodeData->m_filePath.getFileDirectory(sessionDir);
                bool rc = pSessionData->DeleteSessionFilesFromDisk(report);

                if (!rc)
                {
                    if (report.isEmpty())
                    {
                        QString errorStr = PS_STR_TREE_DELETE_FAILED_REPORT;
                        errorStr.append(" Session Dir is ");
                        errorStr.append(sessionDir.directoryPath().fileDirectoryAsString().asASCIICharArray());
                        OS_OUTPUT_DEBUG_LOG(errorStr.toStdWString().c_str(), OS_DEBUG_LOG_ERROR);

                    }
                    else
                    {
                        QString errorStr = PS_STR_TREE_DELETE_FAILED_REPORT;
                        acMessageBox::instance().critical(AF_STR_ErrorA, errorStr);
                    }
                }
            }

            if (saveProjectAfterDeletion)
            {
                // Save the session list to the project:
                afApplicationCommands::instance()->OnFileSaveProject();
            }

            // Delete the tree item only at the end of the function. Removing the tree item also deleted the session item data,
            // so the session data can no longer be used.
            if (canDelete)
            {
                // Remove the item from tree:
                m_pApplicationTree->removeTreeItem(pNodeData->m_pTreeWidgetItem, true);
            }

        }
    }
}

void ProfileApplicationTreeHandler::onEvent(const apEvent& eve, bool& vetoEvent)
{
    Q_UNUSED(vetoEvent);

    if (!WasTreeCreated())
    {
        InitializeApplicationTree();
    }

    bool isInProfileMode = afExecutionModeManager::instance().isActiveMode(PM_STR_PROFILE_MODE);
    bool isInFrameAnalysisMode = afExecutionModeManager::instance().isActiveMode(PM_STR_FrameAnalysisMode);

    // Get the event type:
    apEvent::EventType eventType = eve.eventType();

    // handle the Global var changed event
    switch (eventType)
    {
        case apEvent::APP_GLOBAL_VARIABLE_CHANGED:
        {
            GlobalVariableChangedHandler((const afGlobalVariableChangedEvent&)eve);
        }
        break;

        case apEvent::GD_MONITORED_OBJECT_SELECTED_EVENT:
        {
            if (isInProfileMode || isInFrameAnalysisMode)
            {
                // Bug fix: We should make sure that we are in Profile mode, otherwise
                // we should not enter this block (makes the system hang in some scenarios).
                // Get the activation event:
                const apMonitoredObjectsTreeSelectedEvent& activationEvent = (const apMonitoredObjectsTreeSelectedEvent&)eve;

                // Get the pItem data;
                afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)activationEvent.selectedItemData();

                if (nullptr != pItemData)
                {
                    // Display the pItem:
                    CurrentTreeNodeChanged(pItemData->m_pTreeWidgetItem);
                }
            }
        }
        break;

        case apEvent::AP_EXECUTION_MODE_CHANGED_EVENT:
        {
            // Notice:
            // Some of the other plug-in will probably want to use "Delete" key for its actions.
            // Therefore, we only enable the delete action while in profile mode, so that we "free" the shortcut to other plug-ins:
            // Check if we're switching to profile mode:
            const apExecutionModeChangedEvent& execChangedEvent = (const apExecutionModeChangedEvent&)eve;
            bool isProfile = (execChangedEvent.modeType() == PM_STR_PROFILE_MODE);

            if (nullptr != m_pSessionDeleteAction)
            {
                m_pSessionDeleteAction->setEnabled(isProfile);
            }

            if (nullptr != m_pSessionRenameAction)
            {
                m_pSessionRenameAction->setEnabled(isProfile);
            }

            gtString sessionType = execChangedEvent.sessionTypeName();

            if (sessionType.isEmpty() && (execChangedEvent.sessionTypeIndex() >= 0))
            {
                sessionType = SharedProfileManager::instance().sessionTypeName(execChangedEvent.sessionTypeIndex());
            }

            // Enable empty session node:
            EnableEmptySessionNode(sessionType);

            if (WasTreeCreated())
            {
                // Expand the sessions according to the session type
                ExpandCurrentSessionType(execChangedEvent.modeType(), sessionType);
            }
        }
        break;

        case apEvent::GD_MONITORED_OBJECT_ACTIVATED_EVENT:
        {
            // Get the activation event:
            const apMonitoredObjectsTreeActivatedEvent& activationEvent = (const apMonitoredObjectsTreeActivatedEvent&)eve;

            // Get the item data;
            afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)activationEvent.selectedItemData();

            if (nullptr != pItemData)
            {
                // Check if a profile session is currently running:
                if (AF_TREE_ITEM_PROFILE_EMPTY_SESSION == pItemData->m_itemType)
                {
                    // Create an empty session:
                    OnCreateEmptySession();
                }

            }
        }
        break;

        case apEvent::AP_MDI_ACTIVATED_EVENT:
        {
            if (WasTreeCreated())
            {
                const apMDIViewActivatedEvent& activationEvent = (const apMDIViewActivatedEvent&)eve;
                OnMdiActivateEvent(activationEvent);
            }
        }
        break;

        default:
            // Do nothing
            break;
    }
}


void ProfileApplicationTreeHandler::ExpandCurrentSessionType(const gtString& mode, const gtString& sessionType)
{
    // Collapse all profile type items
    GT_IF_WITH_ASSERT((m_pApplicationTree != nullptr) && (m_pApplicationTree->treeControl() != nullptr))
    {
        QTreeWidgetItem* pRoot = m_pApplicationTree->treeControl()->headerItem();

        if (pRoot != nullptr)
        {
            for (int i = 0; i < pRoot->childCount(); i++)
            {
                QTreeWidgetItem* pTopItem = pRoot->child(i);
                GT_IF_WITH_ASSERT(pTopItem != nullptr)
                {
                    pTopItem->setExpanded(false);
                }
            }
        }
    }

    // Find the session type item. Notice, in frame analysis mode there is no session type, so
    // we look for the frame analysis node
    QString typeToSearch = (mode == PM_STR_FrameAnalysisMode) ? PM_profileTypeFrameAnalysis : acGTStringToQString(sessionType);
    QTreeWidgetItem* pTypeItem = GetProfileTypeNode(typeToSearch, false);

    if ((pTypeItem != nullptr) && (pTypeItem->childCount() > 0))
    {
        // Expand the session type item
        pTypeItem->setExpanded(true);

        pTypeItem->child(0)->setExpanded(true);

        // Expand the first session in tree
        if (mode == PM_STR_FrameAnalysisMode)
        {
            if (pTypeItem->child(0)->childCount() > 0)
            {
                pTypeItem->child(0)->child(0)->setExpanded(true);
            }
        }
    }
}


const wchar_t* ProfileApplicationTreeHandler::eventObserverName() const
{
    return L"SessionExplorerEventsObserver";
}

void ProfileApplicationTreeHandler::OnMdiActivateEvent(const apMDIViewActivatedEvent& activateEvent)
{
    // Sanity check
    GT_IF_WITH_ASSERT((nullptr != m_pApplicationTree) && (nullptr != m_pApplicationTree->treeControl()))
    {
        // Do nothing when the mode is not profile or frame analysis
        bool isInProfileMode = afExecutionModeManager::instance().isActiveMode(PM_STR_PROFILE_MODE);
        bool isInFrameAnalysisMode = afExecutionModeManager::instance().isActiveMode(PM_STR_FrameAnalysisMode);

        if (isInProfileMode || isInFrameAnalysisMode)
        {
            // Find the item that is related to the file selected
            afApplicationTreeItemData* pItemToSelect = m_pApplicationTree->FindItemByFilePath(activateEvent.filePath());

            if (nullptr != pItemToSelect)
            {
                // Get the currently selected item. If it is a child of the selected item, then don't do anything
                bool shouldSelect = true;
                const afApplicationTreeItemData* pCurrentlySelectedItemData = m_pApplicationTree->getCurrentlySelectedItemData();

                if ((nullptr != pCurrentlySelectedItemData) && (nullptr != pCurrentlySelectedItemData->m_pTreeWidgetItem))
                {
                    bool isSelectedAncestorOf = m_pApplicationTree->treeControl()->isAncestor(pCurrentlySelectedItemData->m_pTreeWidgetItem, pItemToSelect->m_pTreeWidgetItem);

                    if (isSelectedAncestorOf)
                    {
                        shouldSelect = false;
                    }
                }

                if (shouldSelect)
                {
                    // Select the item
                    m_pApplicationTree->selectItem(pItemToSelect, false);
                }
            }
        }
    }
}

void ProfileApplicationTreeHandler::GlobalVariableChangedHandler(const afGlobalVariableChangedEvent& event)
{
    // Get id of the global variable that was changed
    afGlobalVariableChangedEvent::GlobalVariableId variableId = event.changedVariableId();

    // If the project file path was changed
    if (variableId == afGlobalVariableChangedEvent::CURRENT_PROJECT)
    {
        // Expand the tree:
        if (WasTreeCreated() && nullptr != m_pApplicationTree)
        {
            // Clear the sessions tree:
            DeleteAllSessions(nullptr, SESSION_EXPLORER_REMOVE_FROM_TREE_ONLY);

            QTreeWidgetItem* pHeaderItem = m_pApplicationTree->headerItem();
            m_pApplicationTree->expandItem(pHeaderItem);
        }
    }
}

ProfileApplicationTreeHandler::FileFilter::FileFilter(const QString& strDescription, const QString& strFileMask) :
    m_strDescription(strDescription),
    m_strMask(strFileMask)
{
}

ProfileApplicationTreeHandler* ProfileApplicationTreeHandler::instance()
{
    if (nullptr == m_pMySingleInstance)
    {
        afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
        GT_IF_WITH_ASSERT(nullptr != pApplicationCommands)
        {
            m_pMySingleInstance = new ProfileApplicationTreeHandler();

        }
    }

    return m_pMySingleInstance;
}

void ProfileApplicationTreeHandler::InitializeApplicationTree()
{
    // Call this function only once:
    if (!m_isTreeInitialized)
    {
        afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
        GT_IF_WITH_ASSERT(nullptr != pApplicationCommands)
        {
            // Try to get the application tree handle:
            m_pApplicationTree = pApplicationCommands->applicationTree();

            if (WasTreeCreated())
            {
                // Should add a signal to main window that the tree was created, and register all tree handler when handling this signal
                OnApplicationTreeCreated();
            }
            else
            {
                // The tree was not create yet, connect to the signal that it is created, and register then:
                bool rc = connect(&afQtCreatorsManager::instance(), SIGNAL(ApplicationTreeCreated()), this, SLOT(OnApplicationTreeCreated()));
                GT_ASSERT(rc);
            }
        }

        // Mark the tree as initialized (to avoid multiple calls of this function:
        m_isTreeInitialized = true;
    }
}

bool ProfileApplicationTreeHandler::BuildContextMenuForItems(const gtList<const afApplicationTreeItemData*> contextMenuItemsList, QMenu& menu)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(nullptr != m_pApplicationTree)
    {
        if (contextMenuItemsList.size() == 1)
        {
            // Build the menu for a tree single item selection:
            const afApplicationTreeItemData* pItemData = contextMenuItemsList.front();
            retVal = BuildContextMenuForSingleItem(pItemData, menu);
        }
        else if (contextMenuItemsList.size() > 1)
        {
            // Add multiple deletion of sessions action
            bool isInProfileMode = afExecutionModeManager::instance().isActiveMode(PM_STR_PROFILE_MODE);
            bool isInFrameAnalysisMode = afExecutionModeManager::instance().isActiveMode(PM_STR_FrameAnalysisMode);

            if (isInProfileMode || isInFrameAnalysisMode)
            {
                retVal = true;
                gtList<const afApplicationTreeItemData*>::const_iterator iter = contextMenuItemsList.begin();
                gtList<const afApplicationTreeItemData*>::const_iterator iterEnd = contextMenuItemsList.end();

                for (; iter != iterEnd; iter++)
                {
                    const afApplicationTreeItemData* pItemData = *iter;

                    if (nullptr != pItemData)
                    {
                        if (AF_TREE_ITEM_PROFILE_SESSION != pItemData->m_itemType)
                        {
                            retVal = false;
                            break;
                        }
                    }
                }

                if (retVal)
                {
                    // Add multiple selection action:
                    menu.addAction(m_pMultipleSessionDeleteAction);
                }
            }
        }
    }

    return retVal;
}


afApplicationTreeItemData* ProfileApplicationTreeHandler::GetSessionNodeItemData(ExplorerSessionId sessionId)
{
    afApplicationTreeItemData* pRetVal = nullptr;
    GT_IF_WITH_ASSERT(nullptr != m_pApplicationTree)
    {
        // Get the project count:
        int projectRootCount = m_pApplicationTree->topLevelItemCount();

        // There supposed to be only one top level item representing the project:
        GT_IF_WITH_ASSERT(projectRootCount == 1)
        {
            QTreeWidgetItem* pProjectRoot = m_pApplicationTree->headerItem();
            GT_IF_WITH_ASSERT(nullptr != pProjectRoot)
            {
                int sessionTypesCount = pProjectRoot->childCount();

                for (int i = 0; i < sessionTypesCount; ++i)
                {
                    QTreeWidgetItem* pSessionTypeItem = pProjectRoot->child(i);
                    GT_IF_WITH_ASSERT(nullptr != pSessionTypeItem)
                    {
                        int sessionsCount = pSessionTypeItem->childCount();

                        for (int k = 0; k < sessionsCount; ++k)
                        {
                            QTreeWidgetItem* pSessionItem = pSessionTypeItem->child(k);
                            GT_IF_WITH_ASSERT(nullptr != pSessionItem)
                            {
                                afApplicationTreeItemData* pItemData = m_pApplicationTree->getTreeItemData(pSessionItem);

                                if (nullptr != pItemData)
                                {
                                    SessionTreeNodeData* pSessionNodeData = qobject_cast<SessionTreeNodeData*>(pItemData->extendedItemData());

                                    if (nullptr != pSessionNodeData)
                                    {
                                        if (pSessionNodeData->m_sessionId == sessionId)
                                        {
                                            pRetVal = pItemData;
                                            break;
                                        }
                                    }
                                }
                            }
                        }

                        if (nullptr != pRetVal)
                        {
                            break;
                        }
                    }

                    if (nullptr != pRetVal)
                    {
                        break;
                    }
                }
            }
        }
    }

    return pRetVal;
}

bool ProfileApplicationTreeHandler::AddSessionTreeNode(ExplorerSessionId sessionId, SessionTreeNodeData* pNewItemSessionData)
{
    bool retVal = false;
    GT_IF_WITH_ASSERT((nullptr != m_pApplicationTree) && (nullptr != pNewItemSessionData) && (nullptr != m_pApplicationTree->headerItem()))
    {
        // First check if an item with this name exist:
        afApplicationTreeItemData* pExistingItem = (pNewItemSessionData->m_pParentData != nullptr) ?  m_pApplicationTree->FindItemByFilePath(pNewItemSessionData->m_pParentData->m_filePath) :
                                                   FindItemByProfileDisplayName(pNewItemSessionData->m_displayName);


        if (pExistingItem != NULL)
        {
            if (pNewItemSessionData->m_isImported)
            {
                acMessageBox::instance().information(AF_STR_InformationA, PM_STR_PROFILE_TREE_SESSION_EXIST);
                m_pApplicationTree->expandItem(pExistingItem->m_pTreeWidgetItem);
                m_pApplicationTree->selectItem(pExistingItem, true);
                retVal = true;
            }

            else
            {
                retVal = false;
                GT_ASSERT_EX(false, L"This item already exist");
            }
        }
        else
        {
            // Item doesn't exist:
            if (nullptr == pNewItemSessionData->m_pParentData)
            {
                // Create a new item data
                pNewItemSessionData->m_pParentData = new afApplicationTreeItemData;
            }

            pNewItemSessionData->m_pParentData->m_itemType = AF_TREE_ITEM_PROFILE_SESSION;

            // Get the project name:
            QTreeWidgetItem* pSessionTreeWidgetItem = CreateItemForSession(pNewItemSessionData);
            GT_IF_WITH_ASSERT(nullptr != pSessionTreeWidgetItem)
            {
                // Set the new session id:
                pNewItemSessionData->m_sessionId = sessionId;

                // Find the matching session type tree handler, and build the tree item:
                SessionTypeTreeHandlerAbstract* pSessionTreeHandler = GetSessionTypeTreeHandler(pNewItemSessionData->m_profileTypeStr);

                // ENH433885: This is a workaround for Intel processors. No need to restrict the import of
                // CPU profiles generated on AMD processors.
                if (pNewItemSessionData->m_isImported && nullptr == pSessionTreeHandler)
                {
                    gtList<SessionTypeTreeHandlerAbstract*>::iterator it = m_sessionTypeToTreeHandlerList.begin();

                    for (; it != m_sessionTypeToTreeHandlerList.end() && (nullptr == pSessionTreeHandler); it++)
                    {
                        if ((*it)->DoesTreeNodeDataBelongsToHandler(pNewItemSessionData))
                        {
                            pSessionTreeHandler = (*it);
                        }
                    }
                }

                GT_IF_WITH_ASSERT(nullptr != pSessionTreeHandler)
                {
                    // Build the session tree structure:
                    pSessionTreeHandler->BuildSessionTree(*pNewItemSessionData, pSessionTreeWidgetItem);

                    retVal = true;
                }
            }
        }
    }

    return retVal;

}

void ProfileApplicationTreeHandler::OnEditorClosed(const QString& newValue)
{
    // Handle editing only when the mode is profile:
    bool shouldHandle = afExecutionModeManager::instance().isActiveMode(PM_STR_PROFILE_MODE);
    shouldHandle = shouldHandle || afExecutionModeManager::instance().isActiveMode(PM_STR_FrameAnalysisMode);

    if (shouldHandle)
    {
        GT_IF_WITH_ASSERT(nullptr != m_pApplicationTree && nullptr != m_pApplicationTree->treeControl())
        {
            QList<QTreeWidgetItem*> treeSelectedItems = m_pApplicationTree->treeControl()->selectedItems();
            QTreeWidgetItem* pItem = nullptr;

            if (!treeSelectedItems.isEmpty())
            {
                pItem = treeSelectedItems.at(0);
            }

            if (nullptr != pItem)
            {
                bool isRenameAttempt = newValue != m_nameBeforeRename;
                QString errMsg;
                QString newNameAfterRevision = newValue;

                if (isRenameAttempt)
                {
                    if (!IsSessionNameValid(newValue, errMsg))
                    {
                        acMessageBox::instance().critical(afGlobalVariablesManager::ProductNameA(), errMsg);
                        newNameAfterRevision = m_nameBeforeRename;

                        // Set the previous name:
                        pItem->setText(0, m_nameBeforeRename);

                        if (isRenameAttempt)
                        {
                            // If the rename was rejected, force retry:
                            OnSessionRename();
                        }
                    }
                    else
                    {
                        afApplicationTreeItemData* pItemData = m_pApplicationTree->getTreeItemData(pItem);
                        GT_IF_WITH_ASSERT(nullptr != pItemData)
                        {
                            RenameSession(pItemData, newNameAfterRevision);
                        }
                    }
                }
            }
        }
    }
}

void ProfileApplicationTreeHandler::OnSessionRename()
{
    GT_IF_WITH_ASSERT(nullptr != m_pApplicationTree && nullptr != m_pApplicationTree->treeControl())
    {
        QList<QTreeWidgetItem*> treeSelectedItems = m_pApplicationTree->treeControl()->selectedItems();

        // Make sure that the list is not empty:
        GT_IF_WITH_ASSERT(!treeSelectedItems.isEmpty())
        {
            QTreeWidgetItem* pContextMenuItem = treeSelectedItems.at(0);
            GT_IF_WITH_ASSERT(nullptr != pContextMenuItem)
            {
                // Save the name before rename:
                afApplicationTreeItemData* pCurrentItemData = m_pApplicationTree->getTreeItemData(pContextMenuItem);

                if (nullptr != pCurrentItemData)
                {
                    SessionTreeNodeData* pCurrentNodeData = qobject_cast<SessionTreeNodeData*>(pCurrentItemData->extendedItemData());

                    if (nullptr != pCurrentNodeData)
                    {
                        QString currentText = pContextMenuItem->text(0);
                        m_nameBeforeRename = pCurrentNodeData->m_displayName;
                    }
                }

                m_pApplicationTree->editCurrentItem();
            }
        }
    }
}

bool ProfileApplicationTreeHandler::IsSessionNameValid(const QString& sessionName, QString& errMsg)
{
    bool valid = false;
    errMsg.clear();

    if ((sessionName.length() > 0) && (sessionName.trimmed().isEmpty()))
    {
        errMsg = PM_STR_RENAME_ONLY_WHITESPACES;
    }
    else if (sessionName.length() != sessionName.trimmed().length())
    {
        errMsg = PM_STR_RENAME_LEADING_OR_TRAILING;
    }
    else if (sessionName.trimmed().isEmpty())
    {
        errMsg = PM_STR_RENAME_EMPTY;
    }
    else if (sessionName.contains(QRegExp("[*?/:<>%|\\\\]")))
    {
        errMsg = PM_STR_RENAME_SPECIAL;
    }
    else
    {
        valid = true;
    }
    GT_IF_WITH_ASSERT(nullptr != m_pApplicationTree && nullptr != m_pApplicationTree->treeControl())
    {
        // Search for an item with the same name:
        QTreeWidgetItem* pRootItem = m_pApplicationTree->treeControl()->topLevelItem(0);

        GT_IF_WITH_ASSERT(nullptr != pRootItem)
        {
            int sessionTypesCount = pRootItem->childCount();

            for (int i = 0; i < sessionTypesCount; i++)
            {
                QTreeWidgetItem* pSessionTypeItem = pRootItem->child(i);

                if (nullptr != pSessionTypeItem)
                {
                    int sessionsCount = pSessionTypeItem->childCount();

                    for (int j = 0; j < sessionsCount; j++)
                    {
                        QTreeWidgetItem* pSessionItem = pSessionTypeItem->child(j);
                        afApplicationTreeItemData* pItemData = m_pApplicationTree->getTreeItemData(pSessionItem);

                        if ((nullptr != pItemData) && (nullptr != pItemData->m_pTreeWidgetItem))
                        {
                            // Do not compare the currently selected item, this is the renamed session:
                            if (!pItemData->m_pTreeWidgetItem->isSelected())
                            {
                                SessionTreeNodeData* pSessionData = qobject_cast<SessionTreeNodeData*>(pItemData->extendedItemData());

                                if (nullptr != pSessionData)
                                {
                                    if (pSessionData->m_displayName == sessionName)
                                    {
                                        valid = false;
                                        errMsg = QString(PM_STR_RENAME_SESSION_EXISTS).arg(sessionName);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return valid;
}

afApplicationTreeItemData* ProfileApplicationTreeHandler::FindMatchingTreeItem(const afApplicationTreeItemData& displayedItemId)
{
    afApplicationTreeItemData* pRetVal = nullptr;
    GT_IF_WITH_ASSERT(nullptr != m_pApplicationTree)
    {
        pRetVal = FindMatchingTreeItemRecursive(m_pApplicationTree->headerItem(), displayedItemId);
    }
    return pRetVal;
}


afApplicationTreeItemData* ProfileApplicationTreeHandler::FindMatchingTreeItemRecursive(QTreeWidgetItem* pItem, const afApplicationTreeItemData& displayedItemId)
{
    afApplicationTreeItemData* pRetVal = nullptr;
    GT_IF_WITH_ASSERT(nullptr != m_pApplicationTree)
    {
        if (nullptr != pItem)
        {
            int childCount = pItem->childCount();

            for (int i = 0; i < childCount; i++)
            {
                afApplicationTreeItemData* pChildData = m_pApplicationTree->getTreeItemData(pItem->child(i));

                if (nullptr != pChildData)
                {
                    if (pChildData->isSameObject(&displayedItemId))
                    {
                        pRetVal = pChildData;
                        break;
                    }
                    else
                    {
                        pRetVal = FindMatchingTreeItemRecursive(pChildData->m_pTreeWidgetItem, displayedItemId);

                        if (nullptr != pRetVal)
                        {
                            break;
                        }
                    }
                }
            }
        }
    }
    return pRetVal;
}

void ProfileApplicationTreeHandler::SetItemsVisibility()
{
    // Check if the profile mode is currently active, and show / hide all the profile session items in according with:
    bool isInProfileMode = afExecutionModeManager::instance().isActiveMode(PM_STR_PROFILE_MODE);
    bool isInFrameAnalysisMode = afExecutionModeManager::instance().isActiveMode(PM_STR_FrameAnalysisMode);

    GT_IF_WITH_ASSERT(nullptr != m_pApplicationTree)
    {
        QTreeWidgetItem* pRootItem = m_pApplicationTree->headerItem();

        if (nullptr != pRootItem)
        {
            int childCount = pRootItem->childCount();

            for (int i = 0; i < childCount; i++)
            {
                afApplicationTreeItemData* pChildData = m_pApplicationTree->getTreeItemData(pRootItem->child(i));

                if (nullptr != pChildData)
                {
                    if (AF_TREE_ITEM_PROFILE_SESSION_TYPE == pChildData->m_itemType)
                    {
                        if (nullptr != pChildData->m_pTreeWidgetItem)
                        {
                            bool isFrameAnalysisNode = pChildData->m_pTreeWidgetItem->text(0).contains(PM_profileTypeFrameAnalysisPrefix);
                            bool shouldShow = (isInFrameAnalysisMode && isFrameAnalysisNode) || (isInProfileMode && !isFrameAnalysisNode);

                            // Show / hide the child
                            pChildData->m_pTreeWidgetItem->setHidden(!shouldShow);
                        }
                    }
                }
            }
        }
    }
}

void ProfileApplicationTreeHandler::ImportStarted()
{
    m_bImportInProgress = true;
    SharedProfileManager::instance().setImportIsRunning(m_bImportInProgress);

    m_startActionLastState = afExecutionModeManager::instance().isStartActionEnabled();
    afExecutionModeManager::instance().updateStartAction(false);
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    pApplicationCommands->updateToolbarCommands();
}

void ProfileApplicationTreeHandler::onFileImportedComplete()
{
    m_bImportInProgress = false;
    SharedProfileManager::instance().setImportIsRunning(m_bImportInProgress);
    afExecutionModeManager::instance().updateStartAction(m_startActionLastState);
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    pApplicationCommands->updateToolbarCommands();

}

bool ProfileApplicationTreeHandler::registerSessionTypeTreeHandler(const QString& profileType, SessionTypeTreeHandlerAbstract* pHandler)
{
    bool retVal = false;

    if (nullptr == m_sessionTypeToTreeHandlerMap[profileType])
    {
        m_sessionTypeToTreeHandlerMap[profileType] = pHandler;
    }

    // check if it is in the list of handler and add it if it is not:
    gtList<SessionTypeTreeHandlerAbstract*>::iterator it = m_sessionTypeToTreeHandlerList.begin();
    bool handlerFound = false;

    for (; it != m_sessionTypeToTreeHandlerList.end(); it++)
    {
        if (pHandler == (*it))
        {
            handlerFound = true;
            break;
        }
    }

    if (!handlerFound)
    {
        m_sessionTypeToTreeHandlerList.push_back(pHandler);
        pHandler->InitializeProfileIcons();
    }

    return retVal;
}

SessionTreeNodeData* ProfileApplicationTreeHandler::GetSessionTreeNodeData(ExplorerSessionId sessionId)
{
    SessionTreeNodeData* pRetVal = nullptr;
    afApplicationTreeItemData* pData = GetSessionNodeItemData(sessionId);

    if (nullptr != pData)
    {
        pRetVal = qobject_cast<SessionTreeNodeData*>(pData->extendedItemData());
    }

    return pRetVal;

}

QTreeWidgetItem* ProfileApplicationTreeHandler::CreateItemForSession(SessionTreeNodeData* pNewItemSessionData)
{
    // Profile type items are supposed to be children of top-level item of the tree:
    QTreeWidgetItem* pRetVal = nullptr;

    GT_IF_WITH_ASSERT((nullptr != m_pApplicationTree) && (nullptr != pNewItemSessionData))
    {
        // Make sure the requested profile type exist:
        GT_IF_WITH_ASSERT(!pNewItemSessionData->m_profileTypeStr.isEmpty())
        {
            QTreeWidget* pTree = m_pApplicationTree->treeControl();
            GT_IF_WITH_ASSERT(nullptr != pTree)
            {
                // Add the requested session type node:
                QTreeWidgetItem* pProfileTypeItem = GetProfileTypeNode(pNewItemSessionData->m_profileTypeStr);

                GT_IF_WITH_ASSERT(nullptr != pProfileTypeItem)
                {
                    int sessionsCount = pProfileTypeItem->childCount();

                    for (int i = 0; i < sessionsCount; i++)
                    {
                        QTreeWidgetItem* pItem = pProfileTypeItem->child(i);
                        QString text = pItem->text(0);

                        if (text == pNewItemSessionData->m_displayName)
                        {
                            pRetVal = pItem;
                            break;
                        }
                    }

                    // Profile type was not added yet:
                    if (nullptr == pRetVal)
                    {
                        if (nullptr == pNewItemSessionData->m_pParentData)
                        {
                            pNewItemSessionData->m_pParentData = new afApplicationTreeItemData;
                        }

                        pNewItemSessionData->m_pParentData->m_itemType = AF_TREE_ITEM_PROFILE_SESSION;
                        pNewItemSessionData->m_pParentData->setExtendedData(pNewItemSessionData);
                        gtString profileTypeAsGTString = acQStringToGTString(pNewItemSessionData->m_profileTypeStr);
                        QPixmap* pPixmap = TreeItemTypeToPixmap(AF_TREE_ITEM_PROFILE_SESSION, pNewItemSessionData->m_profileTypeStr);

                        // Check if there is an empty session (if there is, it should always be the last):
                        QTreeWidgetItem* pPreceding = nullptr;

                        if (DoesEmptySessionNodeExist())
                        {
                            if (m_pEmptySessionItemData->m_pTreeWidgetItem->parent() == pProfileTypeItem)
                            {
                                pPreceding = m_pEmptySessionItemData->m_pTreeWidgetItem;
                            }
                        }

                        // Add the session to the tree (in frame analysis text will be replaced later, when we add the child frames)
                        pRetVal = m_pApplicationTree->insertTreeItem(acQStringToGTString(pNewItemSessionData->m_displayName), pNewItemSessionData->m_pParentData, pProfileTypeItem, pPreceding);
                        GT_IF_WITH_ASSERT((nullptr != pRetVal) && (nullptr != pPixmap))
                        {
                            pRetVal->setIcon(0, QIcon(*pPixmap));

                            // Make the item enabled for name change editing
                            Qt::ItemFlags itemFlags = pRetVal->flags();
                            itemFlags |= Qt::ItemIsEditable;
                            pRetVal->setFlags(itemFlags);
                        }
                    }
                }
            }
        }
    }

    return pRetVal;
}

afApplicationTreeItemData* ProfileApplicationTreeHandler::FindParentSessionItemData(const afApplicationTreeItemData* pTreeItemData)
{
    afApplicationTreeItemData* pRetVal = nullptr;

    const afApplicationTreeItemData* pTempItem = pTreeItemData;
    GT_IF_WITH_ASSERT((nullptr != pTreeItemData) && (nullptr != m_pApplicationTree) && (nullptr != m_pApplicationTree->headerItem()))
    {
        const QTreeWidgetItem* pHeaderItem = m_pApplicationTree->headerItem();

        while ((nullptr == pRetVal) && (nullptr != pTempItem) && (nullptr != pTempItem->m_pTreeWidgetItem) && (pHeaderItem != pTempItem->m_pTreeWidgetItem))
        {
            if ((AF_TREE_ITEM_PROFILE_SESSION == pTempItem->m_itemType) || (AF_TREE_ITEM_PROFILE_EMPTY_SESSION == pTempItem->m_itemType))
            {
                pRetVal = (afApplicationTreeItemData*)pTempItem;
                break;
            }

            // Continue to search in parent:
            QTreeWidgetItem* pParentItem = pTempItem->m_pTreeWidgetItem->parent();

            if (nullptr != pParentItem)
            {
                pTempItem = m_pApplicationTree->getTreeItemData(pParentItem);
            }
            else
            {
                pTempItem = nullptr;
            }
        }
    }

    return pRetVal;
}

afApplicationTreeItemData* ProfileApplicationTreeHandler::FindParentItemForSelection(const afApplicationTreeItemData* pTreeItemData)
{
    afApplicationTreeItemData* pRetVal = nullptr;

    const afApplicationTreeItemData* pTempItem = pTreeItemData;
    GT_IF_WITH_ASSERT((nullptr != pTreeItemData) && (nullptr != m_pApplicationTree))
    {
        const QTreeWidgetItem* pHeaderItem = m_pApplicationTree->headerItem();
        GT_ASSERT(nullptr != pHeaderItem);

        while ((nullptr == pRetVal) && (nullptr != pTempItem) && (nullptr != pTempItem->m_pTreeWidgetItem) && (pHeaderItem != pTempItem->m_pTreeWidgetItem))
        {
            if ((AF_TREE_ITEM_PROFILE_SESSION == pTempItem->m_itemType) || (AF_TREE_ITEM_PROFILE_EMPTY_SESSION == pTempItem->m_itemType) || (AF_TREE_ITEM_GP_FRAME == pTempItem->m_itemType))
            {
                pRetVal = (afApplicationTreeItemData*)pTempItem;
                break;
            }

            // Continue to search in parent:
            QTreeWidgetItem* pParentItem = pTempItem->m_pTreeWidgetItem->parent();

            if (nullptr != pParentItem)
            {
                pTempItem = m_pApplicationTree->getTreeItemData(pParentItem);
            }
            else
            {
                pTempItem = nullptr;
            }
        }
    }

    return pRetVal;
}

void ProfileApplicationTreeHandler::InitializeProfileIcons()
{
    // Create the icons:
    QPixmap* pPixmap1 = new QPixmap;
    acSetIconInPixmap(*pPixmap1, AC_ICON_PROFILE_APPTREE_SESSION_SINGLE);
    m_iconsMap[AC_ICON_PROFILE_APPTREE_SESSION_SINGLE] = pPixmap1;

    QPixmap* pPixmap2 = new QPixmap;
    acSetIconInPixmap(*pPixmap2, AC_ICON_PROFILE_APPTREE_SESSION_MULTI);
    m_iconsMap[AC_ICON_PROFILE_APPTREE_SESSION_MULTI] = pPixmap2;

    QPixmap* pPixmap3 = new QPixmap;
    acSetIconInPixmap(*pPixmap3, AC_ICON_PROFILE_APPTREE_KERNEL_MULTI);
    m_iconsMap[AC_ICON_PROFILE_APPTREE_KERNEL_MULTI] = pPixmap3;

    QPixmap* pPixmap4 = new QPixmap;
    acSetIconInPixmap(*pPixmap4, AC_ICON_PROFILE_APPTREE_KERNEL_SINGLE);
    m_iconsMap[AC_ICON_PROFILE_APPTREE_KERNEL_SINGLE] = pPixmap4;
}

QPixmap* ProfileApplicationTreeHandler::TreeItemTypeToPixmap(afTreeItemType itemType, const QString& sessionTypeAsStr)
{
    QPixmap* pRetVal = nullptr;

    gtList<SessionTypeTreeHandlerAbstract*>::iterator it = m_sessionTypeToTreeHandlerList.begin();

    for (; (it != m_sessionTypeToTreeHandlerList.end()) && (nullptr == pRetVal); it++)
    {
        pRetVal = (*it)->TreeItemTypeToPixmap(itemType, sessionTypeAsStr);
    }

    if (nullptr == pRetVal)
    {
        acIconId iconId = AC_ICON_EMPTY;

        switch (itemType)
        {
            case AF_TREE_ITEM_PROFILE_SESSION:
                iconId = AC_ICON_PROFILE_APPTREE_SESSION_SINGLE;
                break;

            case AF_TREE_ITEM_PROFILE_SESSION_TYPE:
                iconId = AC_ICON_PROFILE_APPTREE_SESSION_MULTI;
                break;

            case AF_TREE_ITEM_PROFILE_GPU_KERNELS:
                iconId = AC_ICON_PROFILE_APPTREE_KERNEL_MULTI;
                break;

            case AF_TREE_ITEM_PROFILE_GPU_KERNEL:
                iconId = AC_ICON_PROFILE_APPTREE_KERNEL_SINGLE;
                break;

            default:
            {
                break;
            }
        }

        if (m_iconsMap.contains(iconId))
        {
            pRetVal = m_iconsMap[iconId];
        }
    }

    return pRetVal;
}

bool ProfileApplicationTreeHandler::canItemBeOpened(const afApplicationTreeItemData* pItemData)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(nullptr != pItemData)
    {
        bool canBeOpened = false;

        gtList<SessionTypeTreeHandlerAbstract*>::iterator it = m_sessionTypeToTreeHandlerList.begin();

        for (; it != m_sessionTypeToTreeHandlerList.end() && !canBeOpened; it++)
        {
            canBeOpened |= (*it)->DoesTreeItemTypeBelongsToHandler(pItemData->m_itemType);
        }

        // The following item types are items that can be opened:
        if ((AF_TREE_ITEM_PROFILE_SESSION == pItemData->m_itemType) || canBeOpened)
        {
            retVal = true;
        }
    }

    return retVal;
}

afApplicationTreeItemData* ProfileApplicationTreeHandler::FindItemByProfileFilePath(const osFilePath& filePath)
{
    afApplicationTreeItemData* pRetVal = nullptr;

    GT_IF_WITH_ASSERT(nullptr != m_pApplicationTree)
    {
        pRetVal = m_pApplicationTree->FindItemByFilePath(filePath);
    }

    return pRetVal;
}

SessionTreeNodeData* ProfileApplicationTreeHandler::FindSessionDataByProfileFilePath(const osFilePath& filePath)
{
    SessionTreeNodeData* pRetVal = nullptr;

    afApplicationTreeItemData* pItemData = FindItemByProfileFilePath(filePath);

    if (nullptr != pItemData)
    {
        pRetVal = qobject_cast<SessionTreeNodeData*>(pItemData->extendedItemData());
    }

    return pRetVal;
}

afApplicationTreeItemData* ProfileApplicationTreeHandler::FindItemByProfileDisplayName(const QString& displayName)
{
    afApplicationTreeItemData* pRetVal = nullptr;

    GT_IF_WITH_ASSERT((nullptr != m_pApplicationTree) && (nullptr != m_pApplicationTree->headerItem()))
    {
        int amountOfSessionTypes = m_pApplicationTree->headerItem()->childCount();

        for (int i = 0; i < amountOfSessionTypes; i++)
        {
            QTreeWidgetItem* pSessionTypeItem = m_pApplicationTree->headerItem()->child(i);
            GT_IF_WITH_ASSERT(nullptr != pSessionTypeItem)
            {
                int amountOfSessions = pSessionTypeItem->childCount();

                for (int j = 0; j < amountOfSessions; j++)
                {
                    QTreeWidgetItem* pSessionItem = pSessionTypeItem->child(j);
                    GT_IF_WITH_ASSERT(nullptr != pSessionItem)
                    {
                        afApplicationTreeItemData* pItemData = m_pApplicationTree->getTreeItemData(pSessionItem);
                        GT_IF_WITH_ASSERT(nullptr != pItemData)
                        {
                            SessionTreeNodeData* pSessionData = qobject_cast<SessionTreeNodeData*>(pItemData->extendedItemData());

                            if (nullptr != pSessionData)
                            {
                                if (pSessionData->m_displayName == displayName)
                                {
                                    pRetVal = pItemData;
                                    break;
                                }
                            }
                        }
                    }
                }

                if (nullptr != pRetVal)
                {
                    break;
                }
            }
        }
    }
    return pRetVal;
}

afApplicationTreeItemData* ProfileApplicationTreeHandler::FindSessionChildItemData(const afApplicationTreeItemData* pSessionItemData, afTreeItemType childItemType)
{
    afApplicationTreeItemData* pRetVal = nullptr;

    // Sanity check:
    GT_IF_WITH_ASSERT((nullptr != pSessionItemData) && (nullptr != pSessionItemData->m_pTreeWidgetItem))
    {
        int childCount = pSessionItemData->m_pTreeWidgetItem->childCount();

        for (int i = 0; (i < childCount) && (nullptr == pRetVal); i++)
        {
            afApplicationTreeItemData* pChildData = m_pApplicationTree->getTreeItemData(pSessionItemData->m_pTreeWidgetItem->child(i));

            if (nullptr != pChildData)
            {
                if (pChildData->m_itemType == childItemType)
                {
                    pRetVal = pChildData;
                    break;
                }

                if (nullptr != pChildData->m_pTreeWidgetItem)
                {
                    int grandChildCount = pChildData->m_pTreeWidgetItem->childCount();

                    for (int j = 0; j < grandChildCount; j++)
                    {
                        afApplicationTreeItemData* pGrandChildData = m_pApplicationTree->getTreeItemData(pChildData->m_pTreeWidgetItem->child(j));

                        if (nullptr != pGrandChildData)
                        {
                            if (pGrandChildData->m_itemType == childItemType)
                            {
                                pRetVal = pGrandChildData;
                                break;
                            }

                        }
                    }
                }
            }
        }
    }

    return pRetVal;
}


void ProfileApplicationTreeHandler::SetItemFontBold(const afApplicationTreeItemData* pItemForBoldFont)
{
    GT_IF_WITH_ASSERT((nullptr != pItemForBoldFont) && (nullptr != m_pApplicationTree))
    {
        // First set all items as not bold
        QTreeWidgetItemIterator treeItemIter(m_pApplicationTree->treeControl());

        while (*treeItemIter)
        {
            QTreeWidgetItem* pItem = (*treeItemIter);

            if (nullptr != pItem)
            {
                QFont font = pItem->font(0);
                font.setBold(false);
                pItem->setFont(0, font);
            }

            ++treeItemIter;
        }

        // Now find the parent session type node and mark as bold
        QString sessionTypeStr;
        SessionTreeNodeData* pSessionData = qobject_cast<SessionTreeNodeData*>(pItemForBoldFont->extendedItemData());

        if (nullptr != pSessionData)
        {
            sessionTypeStr = pSessionData->m_profileTypeStr;
        }

        // Get the project count:
        int projectRootCount = m_pApplicationTree->topLevelItemCount();

        // There supposed to be only one top level item representing the project:
        if ((projectRootCount == 1) && (!sessionTypeStr.isEmpty()))
        {
            QTreeWidgetItem* pProjectRoot = m_pApplicationTree->headerItem();
            GT_IF_WITH_ASSERT(nullptr != pProjectRoot)
            {
                int sessionTypesCount = pProjectRoot->childCount();

                for (int i = 0; i < sessionTypesCount; ++i)
                {
                    QTreeWidgetItem* pSessionTypeItem = pProjectRoot->child(i);
                    GT_IF_WITH_ASSERT(nullptr != pSessionTypeItem)
                    {
                        // Set this profile type as bold / normal:
                        bool shouldSetBold = (pSessionTypeItem->text(0).endsWith(sessionTypeStr));
                        QFont font = pSessionTypeItem->font(0);
                        font.setBold(shouldSetBold);
                        pSessionTypeItem->setFont(0, font);

                        // Now mark the requested item as selected
                        QTreeWidgetItem* pItemToSelect = pItemForBoldFont->m_pTreeWidgetItem;
                        GT_IF_WITH_ASSERT(nullptr != pItemToSelect)
                        {
                            font = pItemToSelect->font(0);
                            font.setBold(true);
                            pItemToSelect->setFont(0, font);
                        }

                        if (AF_TREE_ITEM_PROFILE_SESSION != pItemForBoldFont->m_itemType)
                        {
                            // Mark also the parent
                            afApplicationTreeItemData* pSessionItem = FindParentSessionItemData(pItemForBoldFont);

                            if ((nullptr != pSessionItem) && (nullptr != pSessionItem->m_pTreeWidgetItem))
                            {
                                font = pSessionItem->m_pTreeWidgetItem->font(0);
                                font.setBold(true);
                                pSessionItem->m_pTreeWidgetItem->setFont(0, font);

                                // Expand both the child and the parent
                                pSessionItem->m_pTreeWidgetItem->setExpanded(true);
                                pItemForBoldFont->m_pTreeWidgetItem->setExpanded(true);
                            }
                        }
                    }
                }
            }
        }
    }
}


gtString ProfileApplicationTreeHandler::GetProfileTypeWithPrefix(const QString& sessionTypeAsStr)
{
    gtString retVal = acQStringToGTString(sessionTypeAsStr);

    gtList<SessionTypeTreeHandlerAbstract*>::iterator it = m_sessionTypeToTreeHandlerList.begin();

    bool foundHandler = false;

    for (; it != m_sessionTypeToTreeHandlerList.end() && !foundHandler; it++)
    {
        foundHandler = (*it)->GetProfileTypeWithPrefix(sessionTypeAsStr, retVal);
    }

    return retVal;
}



bool ProfileApplicationTreeHandler::GetNextSessionNameAndDir(const gtString& strProjectName, const osDirectory& projectPath, gtString& strSessionDisplayName, osDirectory& sessionDir, bool shouldCreateDirectory)
{
    bool retVal = false;
    int newSessionId = 1;

    // Check if this is a new / imported session:
    bool isNewSession = (strSessionDisplayName == PM_STR_NewSessionName);
    bool isSessionImported = !strSessionDisplayName.isEmpty();
    gtString strSessionFolderName;

    if (!isSessionImported)
    {
        // Build the session name from the time stamp:
        osTime timing;
        timing.setFromCurrentTime();
        timing.dateAsString(strSessionDisplayName, osTime::NAME_SCHEME_SHORT_FILE, osTime::LOCAL);

        // If this is a remote session, append the "@<ip>" suffix, where <ip>
        // is the remote host's ip address.
        if (afProjectManager::instance().currentProjectSettings().isRemoteTarget())
        {
            gtString suffix = L"@";
            suffix.append(afProjectManager::instance().currentProjectSettings().remoteTargetName());
            strSessionDisplayName.append(suffix);
        }
    }
    else
    {
        if (!isNewSession)
        {
            strSessionFolderName.append(L"_Imported");
        }
    }

    // Set the folder name to be the session display name:
    strSessionFolderName = strSessionDisplayName;

    // Find the parent directory for the profile session path:
    // the directory is the "PROJECTPATH/ProjectName_ProfilerOutput"
    bool doesProjectSessionsDirExist = false;
    osFilePath projectSessionsDir = projectPath.directoryPath();
    osDirectory projectSessionOSDir;
    GT_IF_WITH_ASSERT(!projectSessionsDir.asString().isEmpty())
    {
        gtString projectFolderName = strProjectName;
        projectFolderName += AF_STR_ProfileDirExtension;
        projectSessionsDir.appendSubDirectory(projectFolderName);
        bool rc = projectSessionsDir.getFileDirectory(projectSessionOSDir);
        GT_IF_WITH_ASSERT(rc)
        {
            doesProjectSessionsDirExist = projectSessionOSDir.exists();

            if (!doesProjectSessionsDirExist)
            {
                rc = projectSessionOSDir.create();
                GT_ASSERT(rc);
                doesProjectSessionsDirExist = true;
            }
        }

        GT_IF_WITH_ASSERT(doesProjectSessionsDirExist)
        {
            // Set the default name as the folder name:
            osFilePath outputDir = projectSessionOSDir.directoryPath();
            outputDir.appendSubDirectory(strSessionFolderName);
            outputDir.getFileDirectory(sessionDir);

            if (sessionDir.exists() && !isNewSession)
            {
                gtString strNameBase(strSessionFolderName);

                do
                {
                    gtString strBaseTemp = strNameBase;

                    // Append a '_' if one does not exist there:
                    if (strBaseTemp[strBaseTemp.length() - 1] != L'_')
                    {
                        strBaseTemp.append(L'_');
                    }

                    strBaseTemp.appendFormattedString(L"%d", newSessionId++);

                    outputDir = projectSessionOSDir.directoryPath();
                    outputDir.appendSubDirectory(strBaseTemp);

                    if (!outputDir.getFileDirectory(sessionDir))
                    {
                        continue;
                    }

                    if (!sessionDir.exists())
                    {
                        strSessionFolderName = strBaseTemp;
                        strSessionDisplayName = strBaseTemp;
                    }
                }
                while (sessionDir.exists());
            }

            if (shouldCreateDirectory)
            {
                // create the directory if it doesn't exist
                retVal = sessionDir.create();
            }
        }
    }

    return retVal;
}

bool ProfileApplicationTreeHandler::ExecuteDropEvent(QWidget* receiver, QDropEvent* pEvent, const QString& dragDropFile)
{
    Q_UNUSED(pEvent);
    Q_UNUSED(receiver);

    bool retVal = true;

    // Look for an item with the same file path:
    osFilePath draggedSessionPath(acQStringToGTString(dragDropFile));
    afApplicationTreeItemData* pExistingItemData = FindItemByProfileFilePath(draggedSessionPath);

    if (nullptr != pExistingItemData && nullptr != m_pApplicationTree)
    {
        acMessageBox::instance().information(AF_STR_InformationA, PM_STR_PROFILE_TREE_SESSION_EXIST);
        m_pApplicationTree->expandItem(pExistingItemData->m_pTreeWidgetItem);
        m_pApplicationTree->selectItem(pExistingItemData, true);
    }
    else
    {
        // Import the file:
        retVal = ImportFile(dragDropFile);
    }

    return retVal;
}

bool ProfileApplicationTreeHandler::BuildContextMenuForSingleItem(const afApplicationTreeItemData* pItemData, QMenu& menu)
{
    bool retVal = false;

    if (nullptr != pItemData)
    {
        //
        bool shouldMenuBeOpened = false;
        bool doesSessionExist = false;
        bool isSessionRunning = false;
        afRunModes currentRunModes = afPluginConnectionManager::instance().getCurrentRunModeMask();
        bool isSessionInExport = (0 != (currentRunModes & AF_FRAME_ANALYZE_CURRENTLY_EXPORTING));
        bool isSessionInImport = (0 != (currentRunModes & AF_FRAME_ANALYZE_CURRENTLY_IMPORTING));

        // Check if this item can be opened:
        bool canOpen = canItemBeOpened(pItemData) && !isSessionInExport && !isSessionInImport;

        QString sessionDisplayName;

        afTreeDataExtension* pExtendedItemData = pItemData->extendedItemData();

        if (nullptr != pExtendedItemData)
        {
            SessionTreeNodeData* pTreeNodeData = qobject_cast<SessionTreeNodeData*>(pExtendedItemData);

            // The selected menu item is a profile item, so the context menu should be opened
            shouldMenuBeOpened = (nullptr != pTreeNodeData);

            if (nullptr != pTreeNodeData)
            {
                sessionDisplayName = pTreeNodeData->m_displayName;
                doesSessionExist = pTreeNodeData->SessionDir().exists();
                isSessionRunning = pTreeNodeData->m_isSessionRunning;
            }
        }
        else
        {
            // If this is not a profile item, open the context menu for:
            // Session type node in profile / frame analysis mode
            // Application root node in profile / frame analysis mode
            shouldMenuBeOpened = (AF_TREE_ITEM_PROFILE_SESSION_TYPE == pItemData->m_itemType) || (AF_TREE_ITEM_APP_ROOT == pItemData->m_itemType);
            shouldMenuBeOpened = shouldMenuBeOpened && CurrentModeIsSupported();
        }

        if (shouldMenuBeOpened)
        {
            bool isSession = (AF_TREE_ITEM_PROFILE_SESSION == pItemData->m_itemType);
            bool isSessionType = (isSession || (AF_TREE_ITEM_PROFILE_SESSION_TYPE == pItemData->m_itemType));

            bool isProcessRunning = (afPluginConnectionManager::instance().getCurrentRunModeMask() & AF_DEBUGGED_PROCESS_EXISTS);

            m_pSessionDeleteAction->setEnabled(isSession && !isSessionRunning && !isSessionInExport && !isSessionInImport);
            m_pSessionDeleteAction->setVisible(isSession);

            // Enable rename session only for the session node:
            m_pSessionRenameAction->setEnabled(isSession && !isSessionRunning && !isSessionInExport && !isSessionInImport);
            m_pSessionRenameAction->setVisible(isSession);

            m_pOpenFolderAction->setEnabled(doesSessionExist);
            m_pOpenFolderAction->setVisible(doesSessionExist);


            // Check if delete all should be enabled:
            bool shouldDeleteAll = ((AF_TREE_ITEM_APP_ROOT == pItemData->m_itemType) || (AF_TREE_ITEM_PROFILE_SESSION_TYPE == pItemData->m_itemType));
            shouldDeleteAll = shouldDeleteAll && !isProcessRunning;

            // check if has at least one sub item visible
            bool hasVisibleChildren = false;

            if ((nullptr != m_pApplicationTree) && (nullptr != m_pApplicationTree->headerItem()))
            {
                for (int i = 0; i < m_pApplicationTree->headerItem()->childCount(); i++)
                {
                    if (nullptr != m_pApplicationTree->headerItem()->child(i) &&
                        !m_pApplicationTree->headerItem()->child(i)->isHidden())
                    {
                        hasVisibleChildren = true;
                        break;
                    }
                }
            }

            shouldDeleteAll &= hasVisibleChildren;

            m_pDeleteAllSessionsAction->setEnabled(shouldDeleteAll && !isSessionInExport && !isSessionInImport);
            m_pDeleteAllSessionsAction->setVisible(shouldDeleteAll);

            if (isSessionType && shouldDeleteAll)
            {
                QString deleteStr = QString(PM_STR_TREE_DELETE_ALL_TYPE).arg(sessionDisplayName);
                m_pDeleteAllSessionsAction->setText(deleteStr);
            }
            else
            {
                m_pDeleteAllSessionsAction->setText(PM_STR_TREE_DELETE_ALL_SESSIONS);
            }

            // Check if import should be enabled:
            QString msg;

            // Import is enabled when there are filters for import files (is filled when the plugins are registered), and there is a loaded project:
            bool canImport = !m_profileFilterList.isEmpty() && !afProjectManager::instance().currentProjectSettings().projectName().isEmpty();

            bool isImportEnabledForNode = isSessionType || (AF_TREE_ITEM_APP_ROOT == pItemData->m_itemType);
            bool isImportEnabled = canImport && isImportingOkWithCurrentMode(msg) && isImportEnabledForNode;
            m_pImportSessionAction->setEnabled(isImportEnabled && !isSessionInExport && !isSessionInImport);
            m_pImportSessionAction->setVisible(isImportEnabled);

            if (canOpen)
            {
                QString openItemText = PM_STR_TREE_OPEN_ITEM_PREFIX;

                // Add the item string:
                openItemText += pItemData->m_pTreeWidgetItem->text(0);
                m_pOpenItemAction->setText(openItemText);

                QFont font = m_pOpenItemAction->font();
                font.setBold(true);
                m_pOpenItemAction->setFont(font);

                menu.addAction(m_pOpenItemAction);
                menu.addSeparator();
            }

            // Check if export should be enabled:
            bool isExportEnabledForNode = false;
            gtList<SessionTypeTreeHandlerAbstract*>::iterator it = m_sessionTypeToTreeHandlerList.begin();

            for (; (it != m_sessionTypeToTreeHandlerList.end() && isExportEnabledForNode == false); it++)
            {
                isExportEnabledForNode |= (*it)->IsExportEnabled();
            }

            isExportEnabledForNode &= canOpen;

            m_pExportSessionAction->setEnabled(isExportEnabledForNode && !isSessionInExport && !isSessionInImport);
            m_pExportSessionAction->setVisible(isExportEnabledForNode);

            bool isRefreshEnabled = isImportEnabled;
            isRefreshEnabled = isRefreshEnabled && (afProjectManager::instance().currentProjectSettings().isRemoteTarget());
            m_pRefreshFromServerAction->setEnabled(isRefreshEnabled);
            m_pRefreshFromServerAction->setVisible(isRefreshEnabled);

            // Set the menu:
            menu.addAction(m_pSessionRenameAction);
            menu.addAction(m_pSessionDeleteAction);
            menu.addAction(m_pOpenFolderAction);

            if (m_pImportSessionAction->isVisible() || m_pDeleteAllSessionsAction->isVisible())
            {
                menu.addSeparator();
                menu.addAction(m_pImportSessionAction);
                menu.addAction(m_pDeleteAllSessionsAction);
            }

            if (m_pExportSessionAction->isVisible())
            {
                menu.addAction(m_pExportSessionAction);
            }

            if (m_pRefreshFromServerAction->isVisible())
            {
                menu.addSeparator();
                menu.addAction(m_pRefreshFromServerAction);
            }

            retVal = true;

        }
    }

    return retVal;
}

bool ProfileApplicationTreeHandler::BuildItemHTMLPropeties(const afApplicationTreeItemData& displayedItemId, afHTMLContent& htmlContent)
{
    bool retVal = false;

    // Get the session tree node data:
    SessionTreeNodeData* pSessionData = qobject_cast<SessionTreeNodeData*>(displayedItemId.extendedItemData());

    if (nullptr != pSessionData)
    {
        // Build the profile header HTML section:
        pSessionData->BuildSessionHTML(htmlContent);

        // Check if any of the handlers supports this item:
        gtList<SessionTypeTreeHandlerAbstract*>::iterator it = m_sessionTypeToTreeHandlerList.begin();

        for (; it != m_sessionTypeToTreeHandlerList.end(); it++)
        {
            retVal = (*it)->ExtendSessionHTMLPropeties(displayedItemId.m_itemType, pSessionData, htmlContent);

            if (retVal)
            {
                break;
            }
        }
    }

    return retVal;
}

void ProfileApplicationTreeHandler::OnApplicationTreeCreated()
{
    m_pApplicationTree = afApplicationCommands::instance()->applicationTree();

    GT_IF_WITH_ASSERT(nullptr != m_pApplicationTree && nullptr != m_pApplicationTree->treeControl())
    {
        bool rc = connect(m_pApplicationTree->treeControl(), SIGNAL(onItemCloseEditor(const QString&)), this, SLOT(OnEditorClosed(const QString&)));
        GT_ASSERT(rc);

        m_pApplicationTree->registerApplicationTreeHandler(this);

        // The actions with shortcuts needs to be added to the tree as actions (otherwise the shortcut is not found):
        m_pApplicationTree->treeControl()->addAction(m_pSessionRenameAction);
        m_pApplicationTree->treeControl()->addAction(m_pSessionDeleteAction);
        m_pApplicationTree->treeControl()->addAction(m_pDeleteAllSessionsAction);
    }
}

QTreeWidgetItem* ProfileApplicationTreeHandler::GetProfileTypeNode(const QString& profileTypeStr, bool shouldCreate)
{
    // Look for the profile type tree item:
    QTreeWidgetItem* pRetVal = nullptr;

    GT_IF_WITH_ASSERT((m_pApplicationTree != nullptr) && (m_pApplicationTree->treeControl() != nullptr))
    {
        QTreeWidgetItem* pTopItem = m_pApplicationTree->treeControl()->topLevelItem(0);
        GT_IF_WITH_ASSERT(pTopItem != nullptr)
        {
            int sessionTypesCount = pTopItem->childCount();

            for (int i = 0; i < sessionTypesCount; i++)
            {
                QTreeWidgetItem* pItem = pTopItem->child(i);
                QString text = pItem->text(0);

                // Compare the end, since we add prefix:
                if (text.endsWith(profileTypeStr))
                {
                    pRetVal = pItem;
                    break;
                }
            }
        }

        // Profile type was not added yet:
        if ((pRetVal == nullptr) && shouldCreate)
        {
            afApplicationTreeItemData* pProfileTypeData = new afApplicationTreeItemData;

            pProfileTypeData->m_itemType = AF_TREE_ITEM_PROFILE_SESSION_TYPE;

            // Get the profile type string with GPU or CPU prefix:
            gtString profileTypeAsGTString = GetProfileTypeWithPrefix(profileTypeStr);

            QPixmap* pPixmap = TreeItemTypeToPixmap(AF_TREE_ITEM_PROFILE_SESSION_TYPE, profileTypeStr);
            pRetVal = m_pApplicationTree->addTreeItem(profileTypeAsGTString, pProfileTypeData, m_pApplicationTree->headerItem());
            GT_IF_WITH_ASSERT((pRetVal != nullptr) && (pPixmap != nullptr))
            {
                pRetVal->setIcon(0, QIcon(*pPixmap));
            }
        }
    }

    return pRetVal;
}

void ProfileApplicationTreeHandler::AddEmptySessionCreationNode(bool shouldSelect)
{
    // Check if the item exists:
    if (nullptr == m_pEmptySessionItemData)
    {
        // Get the power profile type node:
        QTreeWidgetItem* pPowerProfileTypeNode = ProfileApplicationTreeHandler::instance()->GetProfileTypeNode(PM_STR_OnlineProfileName);
        GT_IF_WITH_ASSERT(nullptr != pPowerProfileTypeNode)
        {
            // Add an item:
            m_pEmptySessionItemData = new afApplicationTreeItemData;
            m_pEmptySessionItemData->m_itemType = AF_TREE_ITEM_PROFILE_EMPTY_SESSION;
            m_pEmptySessionItemData->m_isItemRemovable = false;

            // Add the item to the tree:
            QTreeWidgetItem* pNewItem = afApplicationCommands::instance()->applicationTree()->addTreeItem(PM_STR_NewSessionNodeName, m_pEmptySessionItemData, pPowerProfileTypeNode);

            // Set the item icon:
            GT_IF_WITH_ASSERT(nullptr != pNewItem)
            {
                QPixmap* pPixmap = new QPixmap;
                bool rc = acSetIconInPixmap(*pPixmap, AC_ICON_PROFILE_PWR_APPTREE_NEW);
                GT_IF_WITH_ASSERT(rc)
                {
                    pNewItem->setIcon(0, QIcon(*pPixmap));
                }
            }
        }
    }

    // Make sure that the empty session is visible:
    GT_IF_WITH_ASSERT(DoesEmptySessionNodeExist())
    {
        QTreeWidgetItem* pEmptySessionParent = m_pEmptySessionItemData->m_pTreeWidgetItem->parent();
        GT_IF_WITH_ASSERT(nullptr != pEmptySessionParent)
        {
            bool isInProfileMode = afExecutionModeManager::instance().isActiveMode(PM_STR_PROFILE_MODE);
            pEmptySessionParent->setHidden(!isInProfileMode);
        }

        if (shouldSelect)
        {
            afApplicationCommands::instance()->applicationTree()->selectItem(m_pEmptySessionItemData, false);
        }
    }
}

void ProfileApplicationTreeHandler::OnCreateEmptySession()
{
    // Check if the empty session node can be activated:
    gtString message;
    gtString currentSessionType = SharedProfileManager::instance().sessionTypeName(afExecutionModeManager::instance().activeSessionType());

    bool isEmptySessionEnabled = IsEmptySessionEnabled(message, currentSessionType, true);

    if (isEmptySessionEnabled)
    {
        bool doesExist = DoesEmptySessionExist();

        if (!doesExist)
        {
            // If there is no project loaded, create a default one:
            if (afProjectManager::instance().currentProjectSettings().projectName().isEmpty())
            {
                afApplicationCommands::instance()->CreateDefaultProject(PM_STR_PROFILE_MODE, PM_STR_OnlineProfileNameW);

                // Set the scope as system wide:
                SharedProfileSettingPage::Instance()->CurrentSharedProfileSettings().m_profileScope = PM_PROFILE_SCOPE_SYS_WIDE;
            }

            // If the empty session was not created, create it:
            GT_IF_WITH_ASSERT(DoesEmptySessionNodeExist())
            {
                // Find the matching session type tree handler, and build the tree item:
                SessionTypeTreeHandlerAbstract* pSessionTreeHandler = GetSessionTypeTreeHandler(PM_STR_OnlineProfileName);

                // Once the handler found, create the empty session:
                GT_IF_WITH_ASSERT(nullptr != pSessionTreeHandler)
                {
                    SessionTreeNodeData* pEmptySessionData = pSessionTreeHandler->CreateEmptySessionData(m_pEmptySessionItemData);

                    if (nullptr != pEmptySessionData)
                    {
                        pSessionTreeHandler->BuildSessionTree(*pEmptySessionData, m_pEmptySessionItemData->m_pTreeWidgetItem);

                        // Set the session id to the empty session:
                        pEmptySessionData->m_sessionId = ++m_sSessionCount;

                        // select the item and expend it only if it was created if not this will cause infinite loop and crash.
                        GT_IF_WITH_ASSERT(DoesEmptySessionNodeExist() && nullptr != m_pApplicationTree)
                        {
                            // Select the session node and expand it:
                            afApplicationCommands::instance()->applicationTree()->selectItem(m_pEmptySessionItemData, true);
                            m_pApplicationTree->expandItem(m_pEmptySessionItemData->m_pTreeWidgetItem);
                        }
                    }
                }
            }
        }
    }
}

void ProfileApplicationTreeHandler::RenameSession(afApplicationTreeItemData* pItemData, const QString& newNameAfterRevision)
{
    SessionTreeNodeData* pSessionData = qobject_cast<SessionTreeNodeData*>(pItemData->extendedItemData());

    if ((nullptr != pSessionData) && (nullptr != pItemData->m_pTreeWidgetItem))
    {
        QString sessionDisplayName = pSessionData->GetNameWithImportSuffix(newNameAfterRevision);

        // Store the file path before rename:
        osFilePath oldSessionPath = pItemData->m_filePath;
        gtString sessionFileNameBeforeRename;
        oldSessionPath.getFileName(sessionFileNameBeforeRename);

        // Emit a "Before session rename" signal. Some of the users need preparations for the rename (release file handlers):
        bool isRenameEnabled = true;
        QString renameErrorMessage;
        emit BeforeSessionRename(pSessionData, isRenameEnabled, renameErrorMessage);

        if (isRenameEnabled)
        {
            // Rename the folder (in GPU profile that is how the session name is determined):
            osDirectory oldSessionDir = pSessionData->SessionDir();
            osDirectory oldSessionDirBeforeRename = oldSessionDir;
            osDirectory newSessionDir = oldSessionDir;
            newSessionDir.upOneLevel();
            osFilePath sessionDirPath = newSessionDir.directoryPath();

            gtString gtSessionDisplayName = acQStringToGTString(sessionDisplayName);
            sessionDirPath.appendSubDirectory(gtSessionDisplayName);

            // Make sure that the folder is initialized:
            bool rc = false;
            GT_IF_WITH_ASSERT(oldSessionDir.exists() && !oldSessionDir.directoryPath().asString().isEmpty())
            {
                rc = oldSessionDir.rename(sessionDirPath.asString());
            }

            if (rc)
            {
                // update the newDispName with the new dir name
                pSessionData->m_displayName = sessionDisplayName;
                pItemData->m_filePath.setFileDirectory(sessionDirPath.asString());
                pItemData->m_filePath.setFileName(gtSessionDisplayName);

                // Rename the session file:
                osFilePath sessionPathBeforeRename = pItemData->m_filePath;
                sessionPathBeforeRename.setFileName(sessionFileNameBeforeRename);

                rc = QFile::rename(acGTStringToQString(sessionPathBeforeRename.asString()), acGTStringToQString(pItemData->m_filePath.asString()));
                GT_ASSERT(rc);

                // Let the extensions handle the rename:
                emit SessionRenamed(pSessionData, oldSessionPath, oldSessionDirBeforeRename);

                afMainAppWindow* pMainWindow = afMainAppWindow::instance();

                if (nullptr != pMainWindow)
                {
                    // If we're in SA:
                    afQMdiSubWindow* pSubWindow = pMainWindow->findMDISubWindow(oldSessionPath);

                    // If there is an opened MDI for this file, rename it's path:
                    if (nullptr != pSubWindow)
                    {
                        pSubWindow->setFilePath(pItemData->m_filePath);
                        pMainWindow->renameMDIWindow(m_nameBeforeRename, sessionDisplayName, acGTStringToQString(pItemData->m_filePath.asString()));
                    }
                }

                // Set the item name:
                pItemData->m_pTreeWidgetItem->setText(0, newNameAfterRevision);
            }
            else
            {
                acMessageBox::instance().critical(afGlobalVariablesManager::ProductNameA(), PM_STR_RENAME_FILE_CANNOT_BE_RENAMED);

                // Set the previous name:
                pItemData->m_pTreeWidgetItem->setText(0, m_nameBeforeRename);
            }

            afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
            GT_IF_WITH_ASSERT(nullptr != pApplicationCommands)
            {
                // Save the project with the new session name:
                pApplicationCommands->OnFileSaveProject();
            }
        }
        else
        {
            if (!renameErrorMessage.isEmpty())
            {
                // Rename is disabled currently:
                acMessageBox::instance().information(afGlobalVariablesManager::ProductNameA(), renameErrorMessage);
            }

            // Set the previous name:
            pItemData->m_pTreeWidgetItem->setText(0, m_nameBeforeRename);
        }
    }
}

afApplicationTreeItemData* ProfileApplicationTreeHandler::RenameEmptySession(const QString& newSessionName)
{
    afApplicationTreeItemData* pRetVal = nullptr;

    // Check if an empty session exists in tree:
    bool doesEmptySessionExists = false;

    if (DoesEmptySessionNodeExist())
    {
        doesEmptySessionExists = (m_pEmptySessionItemData->m_pTreeWidgetItem->childCount() > 0);

        if (doesEmptySessionExists)
        {
            // Get the old name before rename:
            m_nameBeforeRename = m_pEmptySessionItemData->m_pTreeWidgetItem->text(0);

            // Rename the empty session:
            RenameSession(m_pEmptySessionItemData, newSessionName);

            // Set the tree widget item icon:
            QPixmap* pPixmap = TreeItemTypeToPixmap(AF_TREE_ITEM_PROFILE_SESSION, PM_STR_OnlineProfileName);
            GT_IF_WITH_ASSERT(nullptr != pPixmap)
            {
                m_pEmptySessionItemData->m_pTreeWidgetItem->setIcon(0, QIcon(*pPixmap));
            }

            // Make the item enabled for name change editing:
            Qt::ItemFlags itemFlags = m_pEmptySessionItemData->m_pTreeWidgetItem->flags();
            itemFlags |= Qt::ItemIsEditable;
            m_pEmptySessionItemData->m_pTreeWidgetItem->setFlags(itemFlags);

            pRetVal = m_pEmptySessionItemData;

            // Mark the previously empty session as a session:
            pRetVal->m_itemType = AF_TREE_ITEM_PROFILE_SESSION;
            m_pEmptySessionItemData = nullptr;
        }

        // Add another empty node creation:
        AddEmptySessionCreationNode(false);
    }

    return pRetVal;
}

bool ProfileApplicationTreeHandler::DoesEmptySessionExist() const
{
    bool retVal = false;

    // Check if an empty session exists in tree:
    if (DoesEmptySessionNodeExist())
    {
        retVal = (0 < m_pEmptySessionItemData->m_pTreeWidgetItem->childCount());
    }

    return retVal;
}

bool ProfileApplicationTreeHandler::IsEmptySessionEnabled(gtString& message, const gtString& sessionType, bool shouldPopupUserMessage)
{
    bool retVal = false;

    bool isProcessRunning = (afPluginConnectionManager::instance().getCurrentRunModeMask() & AF_DEBUGGED_PROCESS_EXISTS);
    bool isPower = (sessionType == PM_STR_OnlineProfileNameW);

    if (isProcessRunning)
    {
        message = PM_STR_NewSessionCreationErrorProcessRunning;
    }

    // When the process is running, disable the new session activation (do not popup the user):
    if (!isPower && !isProcessRunning)
    {
        if (shouldPopupUserMessage)
        {
            // The current profile type is not power profile. Ask the user if he wants to switch to power profile:
            QMessageBox::StandardButton userAnswer = acMessageBox::instance().question(afGlobalVariablesManager::ProductNameA(), PM_STR_NewSessionCreationOtherProfileTypeQuestion, QMessageBox::Yes | QMessageBox::No);

            if (userAnswer == QMessageBox::Yes)
            {
                // Create a session type changed event, and make sure it is executed immediately (call handleDebugEvent, not registerPendingDebugEvent):
                apExecutionModeChangedEvent executionModeEvent(PM_STR_PROFILE_MODE, PM_STR_OnlineProfileNameW);
                apEventsHandler::instance().handleDebugEvent(executionModeEvent);

                // Enable creation of empty session:
                retVal = true;
            }
        }
        else
        {
            message = PM_STR_NewSessionCreationErrorOtherProfileType;
        }
    }

    if (!isProcessRunning && isPower)
    {
        retVal = true;
    }

    return retVal;
}

void ProfileApplicationTreeHandler::EnableEmptySessionNode(const gtString& sessionType)
{
    gtString message;
    bool isEnabled = IsEmptySessionEnabled(message, sessionType, false);

    if (isEnabled)
    {
        // Add an empty session creation node:
        AddEmptySessionCreationNode(true);

        GT_IF_WITH_ASSERT(DoesEmptySessionNodeExist() && nullptr != m_pApplicationTree)
        {
            m_pEmptySessionItemData->m_pTreeWidgetItem->setForeground(0, Qt::black);
            m_pApplicationTree->selectItem(m_pEmptySessionItemData, false);
            m_pApplicationTree->treeControl()->scrollToItem(m_pEmptySessionItemData->m_pTreeWidgetItem, QAbstractItemView::EnsureVisible);
        }
    }
    else
    {
        if (DoesEmptySessionNodeExist())
        {
            m_pEmptySessionItemData->m_pTreeWidgetItem->setForeground(0, Qt::gray);
        }
    }
}

bool ProfileApplicationTreeHandler::CurrentModeIsSupported()
{
    bool retVal = false;

    retVal = afExecutionModeManager::instance().isActiveMode(PM_STR_PROFILE_MODE) || afExecutionModeManager::instance().isActiveMode(PM_STR_FrameAnalysisMode);
    return retVal;

}

SessionTypeTreeHandlerAbstract* ProfileApplicationTreeHandler::GetSessionTypeTreeHandler(const QString& sessionType)
{
    SessionTypeTreeHandlerAbstract* retVal = nullptr;

    gtMap<QString, SessionTypeTreeHandlerAbstract*>::iterator it = m_sessionTypeToTreeHandlerMap.begin();
    gtMap<QString, SessionTypeTreeHandlerAbstract*>::iterator itEnd = m_sessionTypeToTreeHandlerMap.end();

    for (; it != itEnd; it++)
    {
        const QString currentStr = it->first;

        if (currentStr.endsWith(sessionType))
        {
            retVal = it->second;
            break;
        }
    }

    return retVal;
}

QString ProfileApplicationTreeHandler::GetProjectNameFromSessionDir(osDirectory& sessionDirectory)
{
    QString retStr;

    gtString fullPathStr = sessionDirectory.directoryPath().asString();
    osDirectory upDirectory(sessionDirectory);
    gtString upOneDirPathStr = upDirectory.upOneLevel().directoryPath().asString();

    gtString projectNameAsStr;

    if (0 != upOneDirPathStr.compare(fullPathStr))
    {
        fullPathStr.getSubString(upOneDirPathStr.length() + 1, fullPathStr.length(), projectNameAsStr);

        GT_ASSERT(projectNameAsStr.length() != 0);
        retStr = acGTStringToQString(projectNameAsStr);
    }

    return retStr;
}

