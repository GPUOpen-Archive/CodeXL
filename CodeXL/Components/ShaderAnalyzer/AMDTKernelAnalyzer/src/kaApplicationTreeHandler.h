//------------------------------ kaApplicationTreeHandler.h ------------------------------

#ifndef __KAAPPLICATIONTREEHANDLER_H
#define __KAAPPLICATIONTREEHANDLER_H

// std
#include <set>

#include <QtWidgets>
#include <QMouseEvent>

class afApplicationTree;
class afApplicationTreeItemData;
class kaTreeDataExtension;
class kaSourceFile;
class afBrowseAction;
class kaProgram;
class kaComputeProgram;
class kaNonPipelinedProgram;
class kaRenderingProgram;
class kaExportBinariesDialog;

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/views/afApplicationTree.h>

// Local:
#include <AMDTKernelAnalyzer/Include/kaAMDTKernelAnalyzerDLLBuild.h>
#include <AMDTKernelAnalyzer/src/kaDataTypes.h>


// ----------------------------------------------------------------------------------
// Class Name:          kaApplicationTreeHandler: public afApplicationTreeHandler
// General Description: handles KA tree specific related issues
//
// Author:              Gilad Yarnitzky
// Creation Date:       22/8/2013
// ----------------------------------------------------------------------------------
class KA_API kaApplicationTreeHandler : public afApplicationTreeHandler, public apIEventsObserver
{
private:
    Q_OBJECT

    enum kaTreeIconIndex
    {
        KA_PIXMAP_KERNELS,
        KA_PIXMAP_VK_FOLDER,
        KA_PIXMAP_CL_FOLDER,
        KA_PIXMAP_GL_FOLDER,
        KA_PIXMAP_DX_FOLDER,
        KA_PIXMAP_SOURCE,
        KA_PIXMAP_OVERVIEW,
        KA_PIXMAP_KERNEL,
        KA_PIXMAP_IL_FILE,
        KA_PIXMAP_ISA_FILE,
        KA_PIXMAP_IL_ISA_FILE,
        KA_PIXMAP_STATISTICS,
        KA_PIXMAP_ANALYSIS,
        KA_PIXMAP_BUILD,
        KA_PIXMAP_ADD_FILE,
        KA_PIXMAP_NEW_FILE,
        KA_PIXMAP_FOLDER,
        KA_PIXMAP_OBJECT_TYPES_AMOUNT
    };

public:

    kaApplicationTreeHandler();
    virtual ~kaApplicationTreeHandler();

    static kaApplicationTreeHandler* instance();

    // Query whether this handler was initialized:
    bool WasTreeCreated() const { return (nullptr != m_pApplicationTree); };

    virtual bool BuildContextMenuForItems(const gtList<const afApplicationTreeItemData*> contextMenuItemsList, QMenu& menu);
    virtual bool BuildItemHTMLPropeties(const afApplicationTreeItemData& displayedItemId, afHTMLContent& htmlContent);
    virtual afApplicationTreeItemData* FindMatchingTreeItem(const afApplicationTreeItemData& displayedItemId);
    virtual void SetItemsVisibility();
    virtual void PostContextMenuAction();

    virtual bool IsDragDropSupported(QDropEvent* pEvent, QString& dragDropFile, bool& shouldAccpet);
    virtual bool IsItemDroppable(QTreeWidgetItem* pItem);
    virtual bool CanItemBeDragged(QTreeWidgetItem* pItem);
    virtual bool ExecuteDropEvent(QDropEvent* pEvent, const QString& dragDropFile);

    void ExecuteDropForDraggedTreeItem(const QMimeData* pMimeData, QDropEvent* pEvent);

    /// Executing drop on Program node dragged from OS items
    /// \param [in] pMimeData - to extract URLs
    /// \param [in] pEvent - to get destination node
    void DropOutertemsOnRelevantProgram(const QMimeData* pMimeData, QDropEvent* pEvent);

    /// manipulating tree:
    void setFilesRootNode(QTreeWidgetItem* iFilesRootNode) { m_pProgramsRootNode = iFilesRootNode; }

    /// Add a node will creating the afApplicationTreeItemData and kaTreeDataExtension for it:
    QTreeWidgetItem* AddNode(const gtString& iNodeName, afTreeItemType iNodeType, const osFilePath& iFilePath, int iLineNumber, QPixmap* iPixmap, QTreeWidgetItem* pParent);

    /// Add a file to the tree:
    /// \param iFilePath the file path
    /// \param focusNode should the node be focused after it is added
    /// \param fromProgram checks if the file is being added from the program
    void AddFile(const osFilePath& iFilePath, bool focusNode, kaSourceFile* pFileData, bool fromProgram = false);

    kaProgram* GetActiveProgram() const;
    const afApplicationTreeItemData*  GetActiveProgramApplicaionTreeData() const;

    void  GetActiveProgramms(std::vector<kaProgram*>& activeProgramms) const;
    void  GetActiveProgramApplicaionTreeData(std::set<const  afApplicationTreeItemData*>& activeProgrammItemDatas) const;

    /// Add program to the tree
    /// \param focusNode should the node be focused after it is added
    /// \param pProgram pointer to the created kaProgram object
    void AddProgram(bool focusNode, kaProgram* pProgram);

    /// update the tree based on the data from the project manager:
    void fillTreeFromProjectManager();

    /// Build tree nodes for the programs
    void BuildProgramsOutputTree();

    /// Clean tree nodes for a specific program
    void CleanProgramOutputTree(kaProgram* pProgram);

    /// Build tree nodes for a specific program
    void BuildProgramOutputTree(kaProgram* pProgram, const AnalyzerBuildArchitecture buildArchitecture);

    /// Get a program tree item, or null if the program doesn't exist
    QTreeWidgetItem* GetProgramTreeItem(kaProgram* pProgram) const;

    /// Get the html information of the overview node:
    bool getOverviewHtmlInfo(const osFilePath& filePath, afHTMLContent& htmlContent);

    /// Get the active CL files path from tree:
    /// \param filePath - output - the active cl file path
    /// \return number of active cl files
    unsigned int activeBuildFiles(gtVector<osFilePath>& filesPath);

    unsigned int AddActiveBuildFiles(const afApplicationTreeItemData* pItemData, gtVector<osFilePath> &filePaths);

    /// Checks if the specified file path already exist in the tree:
    /// \param clFilePath the requested file path
    void selectFileNode(const osFilePath& clFilePath);

    /// Return the currently active item data in kernel analyzer tree:
    const afApplicationTreeItemData* activeItemData() const { return m_pActiveItemData; }
    void setActiveItemData(const afApplicationTreeItemData* pData) { m_pActiveItemData = pData; }

    /// Select an active item cl file node if nothing is select
    void SelectFirstSourceFileNode();

    /// Checks if a default kernel file should be created:
    bool ShouldCreateDefaultKernel() const { return m_shouldCreateDefaultKernel; };

    /// Set the flag stating that a default kernel should be created:
    /// \param shouldCreate
    void SetShouldCreateDefaultKernel(bool shouldCreate) { m_shouldCreateDefaultKernel = shouldCreate; }

    /// close editor if we have an editor
    void CloseEditor();

    /// Clear all CL files from tree view
    void ClearTree();

    /// Get build command String for the node related to pKAData
    /// \param pKAData item data related to the build item
    /// \param itemType the type of the item that should be built
    QString GetBuildCommandString(kaTreeDataExtension* pKAData, afTreeItemType itemType);

    /// Get a build command string for a single file
    QString GetBuildCommandString(const osFilePath& filePath);

    /// Get build command string
    QString GetBuildCommandString();

    // Get active item file path
    // \param filePath to be filled by the active file path.The active item must be of AF_TREE_ITEM_KA_FILE type(otherwise it can't be active)
    // \param Returns true if the filePath was found
    bool GetActiveItemFilePath(osFilePath& activeFilePath);

    /// Returns file type string by passed enum
    /// \param [in] fileType
    /// \param [out] typeName
    void GetFileTypeString(kaFileTypes fileType, gtString& typeName)const;

    /// Returns currently selected tree node data
    afApplicationTreeItemData* GetSelectedItemData();

    /// Returns currently selected tree node datas
    void GetSelectedItemDatas(std::vector<afApplicationTreeItemData*>& result) const;

    /// Adds or set an existing or new file node under program
    /// \param addedFilePath
    /// \param pProgramTreeItemData the program tree item data
    /// \param programChildItemType the program child type on which the file should be set or added
    void AddFileNodeToProgramBranch(const osFilePath& addedFilePath, const afApplicationTreeItemData* pProgramTreeItemData, afTreeItemType programChildItemType);

    /// Returns selected item's parent program item data
    /// \param [in] pProgram
    /// \return afApplicationTreeItemData
    afApplicationTreeItemData* GetProgramItemData(kaProgram* pProgram) const;


    /// Find the parent item data to the tree item attached to pTreeItemData
    /// \param pTreeItemData - the item data for the child of the cl file item
    /// \param parentItemType the requested type of the searched for parent
    const afApplicationTreeItemData* FindParentItemDataOfType(const afApplicationTreeItemData* pTreeItemData, afTreeItemType parentItemType) const;

    /// Returns true if path exists and selected item type fits the conditions
    /// \param pSelectedItemData [in]
    /// \param pParentProgramItemData [in]
    /// \param addedFilePath [in]
    /// \param associateToItemType [in] - if adding from main menu this may be AF_TREE_ITEM_TYPE_NONE
    bool ShouldAddFileToProgramBranch(const afApplicationTreeItemData* pSelectedItemData, const afApplicationTreeItemData* pParentProgramItemData, osFilePath addedFilePath, afTreeItemType& associateToItemType) const;


protected:

    /// apIEventsObserver events callback function
    /// \param eve the event
    /// \param[out] vetoEvent flag indicating whether or not the event should be vetoed
    void onEvent(const apEvent& eve, bool& vetoEvent);

    /// Overrides apIEventsObserver:
    const wchar_t* eventObserverName() const { return L"SessionExplorerEventsObserver"; }

    /// Handler of current tree node changed (selected)
    /// \param item pointer to the session that is now selected
    void onCurrentTreeNodeChanged(QTreeWidgetItem* pItem);

    /// sort kernel devices by family name:
    /// \ param pkernelTreeItem - kernel tree item which child item devices needs sorting
    void reorderKernelNodeByFamily(QTreeWidgetItem* pkernelTreeItem);

    /// sort render gl/vulkan program by stage:
    /// \ param pProgramTreeItem - program tree item which child item stages needs sorting
    void ReorderRenderingProgramByPipeLine(QTreeWidgetItem* pProgramTreeItem);

    /// service function that finds the family name from the text
    /// \ param pkernelTreeItem  - kernel tree item which we parses for the family name
    /// \param[out] family name of the node
    QString familyName(QTreeWidgetItem* pTreeItem);

    /// Build a context menu for a single tree item:
    /// \param pItemData the item data for which the menu should be displayed
    /// \param[out\ the Qt context menu
    bool BuildContextMenuForSingleItem(const afApplicationTreeItemData* pItemData, QMenu& menu);

    /// Build a context menu for a tree file item
    void BuildMenuForFileItem(const afApplicationTreeItemData* pItemData, QMenu& menu);

    /// Build a context menu for a tree shader file item
    void BuildMenuForShaderFileItem(const afApplicationTreeItemData* pItemData, QMenu& menu);

    /// Build a context menu for a tree shader file item
    void BuildMenuForProgramItem(const afApplicationTreeItemData* pItemData, QMenu& menu);

    /// Checks if a kernel name is valid:
    /// \param kernelName the kernel name
    /// \param errorMessage if the kernel name is invalid, this string will contain an error message describing the reason
    /// \param moreThenOnce invalid name if kernel name appears more then once
    bool IsNewFileNameValid(const QString& kernelName, QString& errorMessage, bool moreThenOnce = true);

    /// Checks if a kernel name is valid:
    /// \param programName the suggested program name
    /// \param errorMessage if the kernel name is invalid, this string will contain an error message describing the reason
    bool IsProgramNameValid(const QString& programName, QString& errorMessage);

    /// build the files tree for a specific directory
    void BuildProgramOutputTree(osDirectory& buildDir, kaProgram* pProgram, bool is32Dir);

    /// Finds the tree item representing the input program
    /// \param pProgram the program data
    /// \return tree node with the program child
    QTreeWidgetItem* FindProgramTreeItem(kaProgram* pProgram) const;

    /// Finds the tree item representing the program file child
    /// \param pProgramParentItem the program tree item
    /// \param filePath the file path
    /// \return tree node with the program child
    QTreeWidgetItem* FindProgramFileTreeItem(QTreeWidgetItem* pProgramParentItem, const osFilePath& filePath);

    /// add a file to a non piped line program. the name parts of such file is different device_filename_ext_kernel.ext
    /// \param program pointer to the owning program
    /// \param pBuildItem tree item that hold the parent output build dir (x86 or x64)
    /// \param nameParts the file name split into its parts
    /// \params fileExt file extension
    /// \param is32Dir is the files looked at are in the 32 bit dir
    void AddNonPipedLineFile(kaProgram* pProgram, QTreeWidgetItem* pBuildItem, QStringList& nameParts, QString fileExt, bool is32Dir);

    /// add a file to a piped line program. the name parts of such file is different device_section.ext
    /// \param program pointer to the owning program
    /// \param pBuildItem tree item that hold the parent output build dir (x86 or x64)
    /// \param nameParts the file name split into its parts
    /// \params fileExt file extension
    /// \param is32Dir is the files looked at are in the 32 bit dir
    void AddPipedLineFile(kaProgram* pProgram, QTreeWidgetItem* pBuildDir, QStringList& nameParts, QString fileExt, bool is32Dir);

    /// reorder the build output sub tree (output32 or output64). in pipe programs it will be reorder the section. in non piped it will reorder the kernels in each file
    /// \param program pointer to the owning program
    /// \param pOutputNode the node to reorder (32 node or 64 node)
    void ReorderProgramOutputSubTree(kaProgram* pProgram, QTreeWidgetItem* pOutputNode);

    /// \param program pointer to the owning program
    /// \param pBuildItem tree item that hold the parent output build dir (x86 or x64)
    /// \param nameParts the file name split into its parts
    /// \params fileExt file extension
    /// \param is32Dir is the files looked at are in the 32 bit dir
    void CreateFileTreeNode(kaProgram* pProgram, QTreeWidgetItem* pParentNode, osFilePath& filePath, QStringList& nameParts, QString& fileExt, bool is32Dir);

    /// Create the devices file that is needed to keep the data for the VS coherency in the info which devices are opened
    /// \param program pointer to the owning program
    /// \param nameParts the file name split into its parts
    /// \param is32Dir is the files looked at are in the 32 bit dir
    /// \param filePath the filepath to be written in the devices file
    /// \return the filename of the devices file without ext
    gtString CreateDevicesFile(kaProgram* pProgram, QStringList& nameParts, bool is32Dir, osFilePath& filePath);

    /// Get item type for string name
    afTreeItemType GetItemType(QString& name);

private:
    void UpdateIncludeCheckBoxes(kaExportBinariesDialog* pDlg, const bool disable32BitCheckBox, const bool disable64BitCheckbox) const;
    /// Add file to program and update filetype depends on program type
    bool AddFileToProgram(kaProgram* pProgram, afTreeItemType programChildItemType, const int fileID, kaSourceFile* pFile) const;
    ///update items program node to bold
    void UpdateItemProgramNodeToBold(QTreeWidgetItem* pCurrentItem) const;


public slots:

    /// Tree current selection changed - check if active program should be set
    void OnTreeCurrentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*);

    /// handle the build command:
    void OnBuild();

    /// Handle the new file context menu command. If programChildItemType is a rendering stage, the file should be attached to the parent rendering program
    /// \param programChildItemType the item type describing the rendering program stage, on which the file should be attached
    void OnNewFile(afTreeItemType programChildItemType = AF_TREE_ITEM_ITEM_NONE);

    /// handle the add file context menu command:
    void OnAddFile(afTreeItemType associateToItemType = AF_TREE_ITEM_ITEM_NONE);

    /// checks if adding multiple files to program branch allowed
    /// \param [in] pParentProgramItemData
    /// \retval true in case of DXFolder and CLFolder - othrerwise false
    bool IsAddingMultipleFilesToProgramBranchAllowed(const afApplicationTreeItemData* pParentProgramItemData)const;

private slots:
    /// handlers for the menu items:

    /// handle the new program context menu command:
    void OnNewProgram();


    /// handle the cancel build context menu command:
    void OnCancelBuild();

    void OnCLFileOpen();
    void OnGotoSourceCode();
    void OnOpenContainingFolder();

    /// Is handling the open output folder action
    void OnOpenOutputFolder();

    /// Handling the rename context menu action:
    void OnRename();

    /// Handling Add and Create shader actions
    void OnAddCreateShader();

    /// A generic slot that handles all the remove actions. There is only one slot for all the remove actions,
    /// since we want that all these action will respond to 'Delete' key
    void OnRemove();

    void OnBuildComplete(const gtString& clFilePath);

    void OnExportBinaries();
    void OnApplicationTreeCreated();

    /// Called when session rename editor is started
    /// \param pLineEditor
    void OnEditorStarted(QLineEdit* pLneEditor);

    /// Is handling the editor close signal:
    void OnEditorClosed(const QString& editedText);

    /// Is handling the tree selection changed signal
    void OnTreeItemSelectionChanged();

    /// Handling change on project settings: files source path modification
    void OnSourcePathChanged(gtString oldSourcePaths, gtString newSourcePaths);

    /// Export selected device ISA to CSV
    void OnExportToCSVAction();

    /// Handling tree elements drag move
    void OnDragMoveEvent(QDragMoveEvent *pEvent);

signals:
    /// Send a signal that a KA document selection was changed in the tree:
    void KADocumentSelectionChanged();

private:

    void OpenLastBuildStatisticsNode(kaProgram* pActiveProgram) const;
    /// Recursive function for finding tree item:
    afApplicationTreeItemData* FindFileMatchingTreeItemData(const osFilePath& filePath);

    /// Recursive function for finding tree item:
    afApplicationTreeItemData* FindMatchingTreeItem(QTreeWidgetItem* pCurrentTreeItem, const afApplicationTreeItemData& dataLookedFor);

    /// Add the "Add CL File" tree item to the tree:
    /// \return true iff the operation ended successfully
    bool AddFileAddTreeItem(QTreeWidgetItem* pParent = nullptr);

    /// Add the "Create CL File" tree item to the tree:
    /// \return true iff the operation ended successfully
    bool AddFileCreateTreeItem(QTreeWidgetItem* pParent = nullptr);

    /// Add the "Create a new Program..." tree item to the tree:
    /// \return true iff the operation ended successfully
    bool AddProgramCreateTreeItem();


    /// Replace the path in the ka extension data of a tree node
    void SetNewFilePath(QTreeWidgetItem* treeItem, const osFilePath& filePath);

    /// Delete all associated data from project to a ka data -> tree node and data files
    void DeleteTreeDataExtension(kaTreeDataExtension* pKAData);

    /// Rename the file represented in pItemData to the new selected file name:
    /// \param pItemData the item data representing the item in tree (expecting item of type AF_TREE_ITEM_KA_FILE, or a file reference item in programs)
    /// \param oldFilePath the old file path
    /// \param newFileName the new requested file name (expecting fileName.extension):
    void RenameSourceFile(afApplicationTreeItemData* pItemData, osFilePath oldFilePath, const QString& newFileName);

    /// Rename the program represented in pItemData to the new selected program name:
    /// \param pItemData the item data representing the item in tree (expecting item of type AF_TREE_ITEM_KA_PROGRAM)
    /// \param oldProgramName the old program name
    /// \param newProgramName the new requested program name
    void RenameProgram(afApplicationTreeItemData* pItemData, const gtString& oldProgramName, const gtString& newProgramName);

    /// Go through all the "devices.cxltxt" and "overview.cxlovr" files, and write the new cl file path into them:
    void SetNewPathInTxtFiles(osFilePath& newDirPath, osFilePath& newFilePath);

    /// Is iterating the tree widget item of pItemData, and its children, and renames the file path to newFilePath:
    /// \param pItemData the item data representing the item in tree
    /// \param newFilePath full path of the analyzed file
    void RenameFileRecursive(afApplicationTreeItemData* pItemData, const osFilePath& newFilePath);

    /// The function should go over all opened views, find the one that are related to the renamed file, and handle the rename:
    /// * Change the view caption to the new name
    /// * Reload the file
    /// \param oldFilePath the file path for the old source file
    /// \param oldBuildDirectory the directory for the file before the rename process
    /// \param pRenamedItemData the item data containing all the new file paths (expecting item of type AF_TREE_ITEM_KA_FILE)
    void HandleRenamedOpenedViews(const osFilePath& oldFilePath, const osFilePath& oldBuildDirectory, afApplicationTreeItemData* pRenamedItemData);

    /// Check if the rename is possible. The function checks for related opened windows, and for existing output binary
    /// artifacts. If there are opened windows / output artifacts, the function asks the user for a permission to close the windows / delete the artifacts
    /// \param pRenamedItemData the item data related to the renamed item
    /// \return true iff the rename operation should proceed
    bool IsRenamePossible(const afApplicationTreeItemData* pRenamedItemData);

    /// Get the icon for a filePath
    kaTreeIconIndex GetIconForFile(const osFilePath& filePath, const kaSourceFile* pFileData = nullptr) const;

    /// Build a rendering program tree item node
    void BuildRenderingProgramTreeItemNode(kaRenderingProgram* pRenderingProgram, QTreeWidgetItem* pProgramTreeNodeItem);

    /// Build a copmute program tree item node
    void BuildComputeProgramTreeItemNode(kaComputeProgram* pComputeProgram, QTreeWidgetItem* pProgramTreeNodeItem);

    /// Build a non pipelined program tree item
    void BuildNPProgramTreeItemNode(kaNonPipelinedProgram* pNPProgram, QTreeWidgetItem* pProgramTreeNodeItem);


    /// Remove a shader from program
    void OnRemoveShaderFromProgram();

    /// Remove a program from project
    void OnRemoveProgramFromProject();

    /// Find the child item of a requested parent. If there are multiple items with the same type, the function returns the one
    /// with file path equals to filePath
    afApplicationTreeItemData* FindChildOfType(QTreeWidgetItem* pParent, afTreeItemType childType, const osFilePath& filePath);

    /// Set/add a child to a program node. The item is added or set according to the program type
    /// \param pFile the file to add to the program
    /// \param pProgramTreeItemData the item data for the program item
    /// \param addedTreeItemType the program child item type on which the file should be added
    afApplicationTreeItemData* AddProgramTreeItemChild(QTreeWidgetItem* pProgramTreeNodeItem, afTreeItemType itemType, const osFilePath& childFilePath);

    /// Find the position of the new file item to add. The return value is the first non-file item in the tree
    QTreeWidgetItem* FindProgramFileBeforeItem(QTreeWidgetItem* pProgramItem);

    gtString ProgramItemTypeAsText(afTreeItemType itemType, const gtString& itemText);

    /// Sets the new file on the appropriate tree item
    void SetFileInTree(kaSourceFile* pFile, const afApplicationTreeItemData* pProgramTreeItemData, afTreeItemType addedTreeItemType);


    /// This function is called after a file rename. It goes over all the file references in all the current project programs, and fixes it
    /// \param oldFilePath the file path before rename
    /// \param newFilePath the file path after rename
    void RenameAllFileOccurencesInTree(const osFilePath& oldFilePath, const osFilePath& newFilePath);

    /// Get the program child file type according to the item data and the source file pointer
    /// \param the program child tree item data
    /// \param the source file attached to this program child
    /// \return afTreeItemType describing the item type, or AF_TREE_ITEM_ITEM_NONE if not found
    afTreeItemType ExtractProgramChildType(const afApplicationTreeItemData* pProgramTreeItemData, kaSourceFile* pFile);

    /// Remove a file (with fileId) from the list of programs. Remove the file both from the kaProgram item, and from the tree
    /// \param listOfRelatedPrograms list of related programs (all should include fileId as a referenced file)
    /// \param fileId the file id to be removed
    void RemoveFileFromPrograms(const QList<kaProgram*>& listOfRelatedPrograms, int fileId);

    /// Removes stage name and round brackets and highlights file name
    /// \param pLineEditor
    void StartStageShaderNameEditing(QLineEdit* pLineEditor);

    /// Get the file
    void StageNodeNameFromFileName(QString& newFileName, const QString& fileNameBeforeEdit) const;

    /// Rename a file item (the function is called after the editor is closed, and the name was received from user)
    /// \param newFileName the new edited file name
    /// \param pRenamedItemData the item data for the renamed item
    void RenameFile(afApplicationTreeItemData* pRenamedItemData, const QString& newFileName);

    /// Rename a program item (the function is called after the editor is closed, and the name was received from user)
    /// \param newProgramName the new edited file name
    /// \param pRenamedItemData the item data for the renamed item
    void RenameProgram(afApplicationTreeItemData* pRenamedItemData, const QString& newProgramName);

    /// Returns the selected item type.
    /// If multiple items are selected:
    /// 1. For multiple items with the same type the type will be returned
    /// 2. For multiple items with 'brother' item types, one of them will returned
    /// 3. For multiple items with different types, AF_TREE_ITEM_ITEM_NONE will be returned
    afTreeItemType GetSelectedItemType();
    /// Only kaSingletonsDelete should delete me:
    friend class kaSingletonsDelete;
    /// My single instance:
    static kaApplicationTreeHandler* m_psMySingleInstance;

    /// application tree
    afApplicationTree* m_pApplicationTree;
    QTreeWidgetItem* m_pProgramsRootNode;
    QTreeWidgetItem* m_pAddFileTreeItem;
    QTreeWidgetItem* m_pNewFileTreeItem;
    QTreeWidgetItem* m_pNewProgramTreeItem;
    const afApplicationTreeItemData* m_pActiveItemData;

    /// Context menu actions:
    QAction* m_pAddAction;
    QAction* m_pNewAction;
    QAction* m_pOpenAction;
    QAction* m_pBuildAction;
    QAction* m_pCancelBuildAction;
    QAction* m_pExportAction;
    QAction* m_pRemoveAction;
    QAction* m_pAddShaderAction;
    QAction* m_pCreateShaderAction;
    QAction* m_pGotoSourceAction;
    QAction* m_pOpenContainingFolderAction;
    QAction* m_pOpenOutputFolderAction;
    QAction* m_pRenameFileAction;

    /// Program actions:
    QAction* m_pNewProgramAction;

    /// Export device ISA to CSV
    afBrowseAction* m_pExportToCSVAction;

    // Tree icon images
    QPixmap* m_pIconsArray;

    /// Contain the name of a kernel before edit:
    QString m_fileNameBeforeEdit;

    /// Contain a pointer to the edited / renamed item
    QTreeWidgetItem* m_pRenamedItem;

    /// Contain true iff default kernel should
    bool m_shouldCreateDefaultKernel;

    /// True iff we're in the process of renaming. When we rename the file, we should not rename the kernel name:
    bool m_isRenaming;

    /// Created tree node data with the "Create..." action
    afApplicationTreeItemData* m_pCreatedSourceItemData;

    /// a flag that mark if after the context menu is closed we should take the rename action
    bool m_shouldTakeRenameAction;

    /// a stage shader tree item that was highlighted as a drop destination hint
    QTreeWidgetItem* m_pLastHintItem = nullptr;
};

#endif // __KAAPPLICATIONTREEHANDLER_H