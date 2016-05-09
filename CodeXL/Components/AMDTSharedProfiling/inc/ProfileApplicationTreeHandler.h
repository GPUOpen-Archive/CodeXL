//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ProfileApplicationTreeHandler.h
///
//==================================================================================

#ifndef _PROFILEAPPLICATIONTREEHANDLER_H_
#define _PROFILEAPPLICATIONTREEHANDLER_H_

// Ignore warnings:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Qt:
#include <QtCore>
#include <QtWidgets>
#include <QWidget>
#include <QUrl>
#include <QMenu>
#include <QMap>


// Infra:
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTAPIClasses/Include/Events/apMDIViewActivatedEvent.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/views/afApplicationTree.h>
#include <AMDTApplicationFramework/Include/views/afBaseView.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTApplicationFramework/Include/afGlobalVariableChangedEvent.h>
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>

// Local:
#include <AMDTSharedProfiling/inc/SessionExplorerDefs.h>
#include <AMDTSharedProfiling/inc/SessionTreeNodeData.h>
#include <AMDTSharedProfiling/inc/SessionTypeTreeHandlerAbstract.h>

#include "LibExport.h"

#define SP_STR_SourceCodeNodeText L"Source Code"

//forward decl
class ProfilerSessionExplorerViewCreator;


class afBrowseAction;

class AMDTSHAREDPROFILING_API ProfileApplicationTreeHandler : public afApplicationTreeHandler, public apIEventsObserver, public afBaseView
{
    Q_OBJECT

public:
    static ProfileApplicationTreeHandler* instance();

    // Query whether this handler was initialized:
    bool WasTreeCreated() const { return (nullptr != m_pApplicationTree); };

    void InitializeProfileIcons();

    /// Destructor
    virtual ~ProfileApplicationTreeHandler();

    /// Session type tree handler:
    bool registerSessionTypeTreeHandler(const QString& profileType, SessionTypeTreeHandlerAbstract* pHandler);

    // afApplicationTreeHandler overrides:
    virtual bool BuildContextMenuForItems(const gtList<const afApplicationTreeItemData*> contextMenuItemsList, QMenu& menu);
    virtual afApplicationTreeItemData* FindMatchingTreeItem(const afApplicationTreeItemData& displayedItemId);
    virtual bool BuildItemHTMLPropeties(const afApplicationTreeItemData& displayedItemId, afHTMLContent& htmlContent);
    virtual void SetItemsVisibility();
    virtual bool IsDragDropSupported(QDropEvent* event, QString& dragDropFile, bool& shouldAccpet);
    virtual bool ExecuteDropEvent(QDropEvent* pEvent, const QString& dragDropFile);

    /// Go over the tree and look for a profile with the requested file path:
    /// \param filePath - the profile file path
    /// \return the item data related to the item requested
    virtual afApplicationTreeItemData* FindItemByProfileFilePath(const osFilePath& filePath);

    /// Go over the tree and look for a profile with the requested display name:
    /// \param displayName - the profile file path
    /// \return the item data related to the item requested
    afApplicationTreeItemData* FindItemByProfileDisplayName(const QString& displayName);

    /// Go over the children of the session (recursively), and look for a child with the requested item type:
    /// \param pSessionItemData - the session item data
    /// \param childItemType - the item type of the child
    afApplicationTreeItemData* FindSessionChildItemData(const afApplicationTreeItemData* pSessionItemData, afTreeItemType childItemType);

    /// Find the session related to the tree widget item in pTreeItemData (go up until you get to a session node)
    /// \ param pTreeItemData - the item data for the child of the session item
    afApplicationTreeItemData* FindParentSessionItemData(const afApplicationTreeItemData* pTreeItemData);

    /// Find the first parent that should be mark as selected. The first parent which is a session or frame will be returned
    /// \ param pTreeItemData - the item data for the child of the session item
    afApplicationTreeItemData* FindParentItemForSelection(const afApplicationTreeItemData* pTreeItemData);

    /// Go over the tree and look for a profile with the requested file path:
    /// \param filePath - the profile file path
    /// \return the item data related to the item requested
    SessionTreeNodeData* FindSessionDataByProfileFilePath(const osFilePath& filePath);

    /// Adds tree node for the session -- NOTE this is not currently thread-safe
    /// \param sessionInfo - contain the data for the added session
    /// \return the added session node id
    ExplorerSessionId AddSession(SessionTreeNodeData* pNewItemSessionData, bool activateItem);

    /// Expand the tree according to the new session type and mode
    /// \param mode the new mode
    /// \param the session type
    void ExpandCurrentSessionType(const gtString& mode, const gtString& sessionType);

    /// Initializes the application tree pointer, and connect the relevant slots to the tree object
    void InitializeApplicationTree();

    /// Adds a file filter for the Open dialog shown when the user imports a file.
    /// This method should be called for each supported filter
    /// \param strDescription description of the filter to be shown in the File Open Dialog
    /// \param strFileMask the mask of the filter
    void AddImportFileFilter(const QString& strDescription, const QString& strFileMask, const gtString& modeName);

    /// Gets the specified session node session data
    /// \param sessionId the id of the session whose node is needed
    /// \return the specified session node data, or NULL if not found
    SessionTreeNodeData* GetSessionTreeNodeData(ExplorerSessionId sessionId);

    /// Gets the specified session node tree item data
    /// \param sessionId the id of the session whose node is needed
    /// \return the specified session node, or NULL if not found
    afApplicationTreeItemData* GetSessionNodeItemData(ExplorerSessionId sessionId);

    /// Return the pixmap pointer for the requested item type icon
    /// \ param itemType - the item type
    QPixmap* TreeItemTypeToPixmap(afTreeItemType itemType, const QString& sessionTypeAsStr = "");

    /// Gets the name and directory for the next session for the given project name and directory
    /// \param strProjectName the name of the project (i.e. the parent node name in the explorer view)
    /// \param projectPath the full path of directory the project lives in.  Note: if projectPath is empty, then only the strSessionName is filled in
    /// \param projectPath the full path of directory the project lives in.  Note: if projectPath is empty, then only the strSessionName is filled in
    /// \param[out] strSessionDisplayName the session display name of the next session. This is based on the strSesssionFileName, but will look nicer
    /// \param[out] sessionDir the directory of the next session (if projectPath is non-empty)
    /// \param[in] shouldCreateDir should the session directort be created
    /// \return true if successful, false otherwise
    bool GetNextSessionNameAndDir(const gtString& strProjectName, const osDirectory& projectPath, gtString& strSessionDisplayName, osDirectory& sessionDir, bool shouldCreateDir = true);

    /// Add or returns the tree widget item for the requested profile type:
    /// \param profileTypeStr the string describing the profile type
    /// \return QTreeWidgetItem* the item for the requested profile type
    QTreeWidgetItem* GetProfileTypeNode(const QString& profileTypeStr, bool shouldCreate = true);

    /// Uses the empty session node to create a new session
    /// \return afApplicationTreeItemData* the pointer for the renamed session item data or null if there is no empty session
    afApplicationTreeItemData* RenameEmptySession(const QString& newSessionName);

    /// Checks if the tree has an empty session:
    /// \return true if there is an empty session, false if not
    bool DoesEmptySessionExist() const;

    /// Checks if the tree has an empty session node (even if not initialized:
    /// \return true if there is an empty session, false if not
    bool DoesEmptySessionNodeExist() const { return (nullptr != m_pEmptySessionItemData) && (nullptr != m_pEmptySessionItemData->m_pTreeWidgetItem); };

    /// Get the specific session type tree handler
    /// \param sessionType the name of the current session
    SessionTypeTreeHandlerAbstract* GetSessionTypeTreeHandler(const QString& sessionType);

    /// Static utility - extracts the project name from the session directory path
    /// \param sessionDirectory the session directory
    /// \return the project name
    static QString GetProjectNameFromSessionDir(osDirectory& sessionDirectory);

public slots:
    /// Make necessary changes in UI to indicate that import is Ended
    void onFileImportedComplete();

    /// Handle creation of application tree
    void OnApplicationTreeCreated();

protected:

    /// apIEventsObserver events callback function
    /// \param eve the event
    /// \param[out] vetoEvent flag indicating whether or not the event should be vetoed
    void onEvent(const apEvent& eve, bool& vetoEvent);

    /// apIEventsObserver Gets the events observer name -- used for logging
    /// \return the events observer name
    const wchar_t* eventObserverName() const;

    // Handle mdi activation event
    void OnMdiActivateEvent(const apMDIViewActivatedEvent& activateEvent);

    /// Handler of current tree node changed (selected)
    /// \param item pointer to the session that is now selected
    void CurrentTreeNodeChanged(QTreeWidgetItem* item);

    /// Add tree node for the session
    /// \param sessionId the id of the session being added
    /// \param sessionInfo - the information for the new session
    /// \return True if node was added, false otherwise
    bool AddSessionTreeNode(ExplorerSessionId sessionId, SessionTreeNodeData* pNewItemSessionData);

    /// Get (or create) the tree widget item for the requested item profile type + project
    /// \param pNewItemSessionData the item data for the item that is added to tree
    /// \return QTreeWidgetItem* if the item was found, null for error
    QTreeWidgetItem* CreateItemForSession(SessionTreeNodeData* pNewItemSessionData);

    /// Initialize the application tree instance
    void InitTree();

    /// Checks if Importing is Ok with current mode
    /// Disabled if not in Profile mode or profiler is running
    /// \param msg reason when importing is not ok
    /// \return True if Importing is Ok with current mode
    bool isImportingOkWithCurrentMode(QString& msg);

    /// Add a node to the tree for the empty session creation:
    /// \param shouldSelect should the created node be selected
    void AddEmptySessionCreationNode(bool shouldSelect);

    /// Create an empty session:
    void OnCreateEmptySession();

    /// Rename the session:
    /// \param pItemData the item data representing the session that should be renamed:
    void RenameSession(afApplicationTreeItemData* pItemData, const QString& newNameAfterRevision);

    // A pointer to the framework tree:
    afApplicationTree* m_pApplicationTree;

signals:

    /// emitted when edit is requested
    void EditItemRequested();

    /// Signal emitted when the session needs to be displayed (the user double-clicks it, or presses Enter on it in the Explorer tree)
    /// \param sessionId the ID of the session to be displayed
    /// \itemType the item clicked for this session open
    void SessionShown(ExplorerSessionId sessionId);

    /// Signal emitted when a session is selected
    /// \param sessionId the ID of the session that is selected
    void SessionSelected(ExplorerSessionId sessionId);

    /// Signal emitted when the session needs to be deleted (the user has asked to delete the session)
    /// \param sessionId the ID of the session to be deleted
    /// \param deleteType flag indicating whether the session is merely being removed from the tree view, or if the user has requested that the session be removed from disk as well
    /// \param[out] sessionDeleted should be set ONLY by the owner of the session to indicate if the session could be deleted or not
    void SessionDeleted(ExplorerSessionId sessionId, SessionExplorerDeleteType deleteType, bool& sessionDeleted);

    /// Signal emitted when the session needs to be renamed (the user has asked to rename the session)
    /// \param pRenamedSessionData the item data for the session been renamed
    /// \param oldSessionFilePath the old session file path
    /// \param oldSessionDir the session folder before the rename
    void SessionRenamed(SessionTreeNodeData* pRenamedSessionData, const osFilePath& oldSessionFilePath, const osDirectory& oldSessionDir);

    /// Signal emitted before the session is renamed
    /// \param pAboutToRenameSessionData the item data for the object which is about to be renamed
    /// \param[out] isRenameEnabled is rename enabled?
    /// \param[out] renameDisableMessage the message for the user describing that currently the rename operation is disabled
    void BeforeSessionRename(SessionTreeNodeData* pAboutToRenameSessionData, bool& isRenameEnabled, QString& renameDisableMessage);

    /// Signal emitted when a file is imported (the user has asked to import the specified file)
    /// \param strFileName the name of the file being imported
    /// \param[out] imported should be set to TRUE by a handler if and only if the handler adds a session for the imported file.  Otherwise the handler should not change the value
    void FileImported(const QString& strFileName, bool& imported);

    /// Signal emmited when a session is added to the tree:
    /// This signal is required since we use the data in CodeXL Explorer to identify sessions and load them into VS:
    void SessionAddedToTree();

protected slots:

    /// Called when session rename editor is closed
    /// \param newValue
    void OnEditorClosed(const QString& newValue);

    /// Handles the rename request:
    void OnSessionRename();

    /// Handles the delete request:
    void OnSessionDelete();

    /// Handles the multiple sessions delete request:
    void OnMultipleSessionDelete();

    /// Handles the open containing folder request:
    void OnOpenContainingFolder();

    /// Handles the import a session request:
    void OnImportSession();

    /// Handles the export a session request:
    void OnExportSession();

    /// Handles the refresh sessions from server request:
    void OnRefreshFromServer();

    /// Handles the delete all sessions request:
    void OnDeleteAllSessions();

    /// Handles the open item action:
    void OnItemOpen();

private:
    /// Initializes a new instance of the ProfileApplicationTreeHandler class.
    /// Should only be used by the instance() method:
    ProfileApplicationTreeHandler();

    /// Add context menu for the node
    /// \param pNode Node for which context menu to be added
    /// \return True on successfully addition of at least one context menu item
    bool AddContextMenu(QTreeWidgetItem* pNode);

    /// Validate session name
    /// \param sessionName name of the session
    /// \param[out] errMsg error message if the session name is not valid
    /// \return True if session name is valid or false if not valid
    bool IsSessionNameValid(const QString& sessionName, QString& errMsg);

    /// Nested struct used to hold file filter info
    struct FileFilter
    {
        /// Initializes a new instance of the FileFilter class
        /// \param strDescription the description of the file filter
        /// \param strFileMask the mask of the file filter
        FileFilter(const QString& strDescription, const QString& strFileMask);

        QString m_strDescription; ///< Description of file filter
        QString m_strMask;        ///< Mask of file filter
    };

    /// Import a file
    /// \param importFilePath import file path
    /// \return True if success in import
    bool ImportFile(const QString& importFilePath);

    /// Deletes the specified node and emits the SessionDeleted signal
    /// \param sessionNode the node to be deleted
    /// \param deleteType flag indicating whether the session is merely being removed from the tree view, or if the user has requested that the session be removed from disk as well
    void DeleteSessionNode(afApplicationTreeItemData* pNodeData, SessionExplorerDeleteType deleteType, bool saveProjectAfterDeletion);

    /// Helper function used when all sessions should be deleted
    /// \param newFilesOnly flag indicating whether only newly-added sessions should be deleted
    /// \param deleteType flag indicating whether the session whould be removed from disk in addition to being removed from the Session Explorer Tree View
    void DeleteAllSessions(QTreeWidgetItem* pProfileTypeItem, SessionExplorerDeleteType deleteType);

    /// Handler for when a global variable changes
    /// \param event the global variable changed event
    void GlobalVariableChangedHandler(const afGlobalVariableChangedEvent& event);

    /// Checks the registered filters to see if the specified file is included in the filter
    /// \param fileExt the extension of the file to check
    /// \return true if the extension is contained in the filter list, false otherwise
    bool CheckFilterList(const QString& fileExt);

    /// Make necessary changes in UI to indicate that import is started
    void ImportStarted();

    /// Recursive utility used for searching for an item matching displayedItemId
    /// \ param displayedItemId - the item data to compare to
    afApplicationTreeItemData* FindMatchingTreeItemRecursive(QTreeWidgetItem* pItem, const afApplicationTreeItemData& displayedItemId);

    /// Checks if the input item can be opened (activated):
    /// \param pItemData - the item data for the checked item
    bool canItemBeOpened(const afApplicationTreeItemData* pItemData);

    /// Marks an item as bold. The parent profile type node related to this session, will also have bold font
    /// \param pItemForBoldFont - the item data attached to the item that should be bold
    void SetItemFontBold(const afApplicationTreeItemData* pItemForBoldFont);

    /// Get the profile type string with a prefix (GPU or CPU):
    /// \param sessionTypeAsStr - the profile type string
    gtString GetProfileTypeWithPrefix(const QString& sessionTypeAsStr);

    /// Builds a tree context menu for tree single selected item:
    /// \param pItemData the item data for the single item selected in tree
    /// \param menu the menu to build
    bool BuildContextMenuForSingleItem(const afApplicationTreeItemData* pItemData, QMenu& menu);

    /// Checks if the "New Session" tree node should be enabled:
    /// \param message [output] a message to the user describing a reason why the empty session node cannot be used
    /// \param sessionType the string describing the current session type
    /// \param shouldPopupUserMessage true iff the user should be prompt to change the profile type to power:
    /// \return true if the session node can be activated, false if not
    bool IsEmptySessionEnabled(gtString& message, const gtString& sessionType, bool shouldPopupUserMessage);

    /// Enable or disables the empty session node:
    /// \param sessionType the string describing the current session type
    void EnableEmptySessionNode(const gtString& sessionType);

    /// Check if the current CodeXL execution mode is supported by this manager
    bool CurrentModeIsSupported();

private:

    /// Only kaSingletonsDelete should delete me:
    friend class SharedProfileSingletonsDelete;

    ///< Static singleton instance
    static ProfileApplicationTreeHandler* m_pMySingleInstance;

    ///< Static member used to count sessions -- this is used to return a unique ExplorerSessionId for each session added
    static ExplorerSessionId m_sSessionCount;

    /// Previous string name for rename
    QString m_nameBeforeRename;

    ///< Old value of the current node
    QString m_oldValueOfCurrentNode;

    /// Session item open action:
    QAction* m_pOpenItemAction;

    /// Session item context menu delete:
    QAction* m_pSessionDeleteAction;

    /// Session items context menu delete:
    QAction* m_pMultipleSessionDeleteAction;

    /// Session item context menu rename
    QAction* m_pSessionRenameAction;

    /// Session item context menu Open Containing folder
    QAction* m_pOpenFolderAction;

    /// Session item context menu Open Containing folder
    QAction* m_pDeleteAllSessionsAction;

    /// Session item context menu Open Containing folder
    /// Session item context menu import
    afBrowseAction* m_pImportSessionAction;

    /// Session item context menu export
    afBrowseAction* m_pExportSessionAction;

    /// Session item context menu refresh from server
    QAction* m_pRefreshFromServerAction;

    /// Indicates if import is in progress
    bool m_bImportInProgress;

    ///Last state of Start button (used for restoring when import is completed)
    bool m_startActionLastState;

    QList<FileFilter*>         m_profileFilterList;           ///< List of supported file filters that can be imported
    QList<FileFilter*>         m_frameAnalysisFilterList;           ///< List of supported file filters that can be imported

    /// Handler list for each profile type
    gtMap<QString, SessionTypeTreeHandlerAbstract*> m_sessionTypeToTreeHandlerMap;

    /// Vector with the profile handler. hold one instance of each one.
    gtList<SessionTypeTreeHandlerAbstract*> m_sessionTypeToTreeHandlerList;
    QMap<acIconId, QPixmap*> m_iconsMap;

    /// An item used for empty session creation:
    afApplicationTreeItemData* m_pEmptySessionItemData;

    /// True iff the application tree was already initialized. Is used to avoid multiple initialization:
    bool m_isTreeInitialized;

};

#endif // _PROFILEAPPLICATIONTREEHANDLER_H_

