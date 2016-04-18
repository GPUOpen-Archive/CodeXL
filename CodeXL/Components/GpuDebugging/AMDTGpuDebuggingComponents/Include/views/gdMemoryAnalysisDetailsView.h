//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdMemoryAnalysisDetailsView.h
///
//==================================================================================

//------------------------------ gdMemoryAnalysisDetailsView.h ------------------------------

#ifndef __GDMEMORYANALYSISDETAILSVIEW
#define __GDMEMORYANALYSISDETAILSVIEW

// Infra:
#include <AMDTApplicationComponents/Include/acListCtrl.h>
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>


// Local:
#include <AMDTGpuDebuggingComponents/Include/gdDebugApplicationTreeData.h>
#include <AMDTGpuDebuggingComponents/Include/gdDebugApplicationTreeHandler.h>

// Defines the items sort direction:
enum gdMemoryAnalysisDetailsSortDirection
{
    GD_SORT_BY_NAME,
    GD_SORT_BY_TYPE,
    GD_SORT_BY_SIZE,
    GD_SORT_BY_DIMENSIONS,
    GD_SORT_BY_MIPMAP,
    GD_SORT_BY_FORMAT,
    GD_SORT_BY_ATTACHMENT_TARGET,
    GD_SORT_BY_ATTACHMENT_POINT,
    GD_SORT_BY_PIXEL_TYPE,
    GD_SORT_BY_VBO_ATTACHMENT,
    GD_SORT_BY_SYNC_HANDLE,
    GD_SORT_BY_SHADER_TYPE,
    GD_SORT_BY_DATA_TYPE,
    GD_SORT_BY_OBJECTS_COUNT,
    GD_SORT_BY_RENDER_CONTEXT,
    GD_SORT_BY_AMOUNT_OF_VERTICES,
    GD_SORT_BY_MEMORY_FLAGS,
    GD_SORT_BY_PACKET_SIZE,
    GD_SORT_BY_MAX_PACKETS,
    GD_SORT_BY_REF_COUNT,
    GD_SORT_BY_AMOUNT_OF_EVENTS,
    GD_SORT_BY_OUT_OF_ORDER_EXE_MODE,
    GD_SORT_BY_PROFILING_MODE,
    GD_SORT_BY_ON_DEVICE,
    GD_SORT_BY_FILTER_MODE,
    GD_SORT_BY_ADDRESSING_MODE
};
struct gdMemorySortInfo
{
    gdMemoryAnalysisDetailsSortDirection _sortType;
    Qt::SortOrder _sortOrder;
};

// ----------------------------------------------------------------------------------
// Class Name:           GD_API gdMemoryAnalysisDetailsView: public acWXListCtrl , public apIEventsObserver
// General Description: This class represents a memory objects list viewer. This viewer is added to the Memory viewer as the
//                      list component.
// Author:               Sigal Algranaty
// Creation Date:        20/7/2008
// ----------------------------------------------------------------------------------
class GD_API gdMemoryAnalysisDetailsView: public acListCtrl , public afBaseView, public apIEventsObserver
{
    Q_OBJECT

public:

    gdMemoryAnalysisDetailsView(QWidget* pParent, afProgressBarWrapper* pProgressBar, gdDebugApplicationTreeHandler* pMemoryTree);
    virtual ~gdMemoryAnalysisDetailsView();

    class GD_API gdMemoryWidgetItem : public QTableWidgetItem
    {

    public:

        gdMemoryWidgetItem(const QString& text, gdMemoryAnalysisDetailsView* pParent);
        ~gdMemoryWidgetItem();

        virtual bool operator<(const QTableWidgetItem& other) const;

    protected:
        gdMemoryAnalysisDetailsView* _pParent;
    };

    // Clears the statistics:
    void getColumnsWidths(gtVector<float>& vColumnWidths);
    void clearAndDisplayMessage();

    // Overrides apIEventsObserver:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);
    virtual const wchar_t* eventObserverName() const { return L"MemoryAnalysisDetailsView"; };

    // List items update and display functions:
    bool updateListItems(QTreeWidgetItem* pselectedTreeItemId, bool rebuildData);
    bool displayObjectsList(afApplicationTreeItemData* pObjectItemData, bool& isAlreadyDisplayed);
    bool findDisplayedListIDByItemID(const afApplicationTreeItemData* pObjectItemID, afApplicationTreeItemData& itemListId);
    bool addItemToList(QTreeWidgetItem* pitemToAdd, bool shouldSelectItem, gtUInt32& totalSize);

    // Used in specific cases to select a list item without changing the tree selection
    void setSelectionFromPreviousItem(const afApplicationTreeItemData& previouslySelectedItem);

    // Used to figure out if the list has / needs a total line or not
    static bool doesItemTypeHaveTotalLine(const afTreeItemType& listItemsType);

    // Check if an item is currently displayed in the detailed view:
    bool itemDataToParentType(const afApplicationTreeItemData* pItemData, afTreeItemType& parentType) const;
    afApplicationTreeItemData& currentDisplayedListID() {return m_currentDisplayedItem;};

    // Sort 2 items:
    bool isItemSmallerThen(afApplicationTreeItemData* pItemData1, afApplicationTreeItemData* pItemData2);

public slots:

    // Events are declared public, since we call them from the statistics viewer:
    virtual void onAboutToShowContextMenu();
    void onSaveMemoryData();
    void onChartItemClicked(int clickedItemIndex);

protected slots:

    virtual QTableWidgetItem* allocateNewWidgetItem(const QString& text);
    void onColumnHeaderClick(int columnIndex);

protected:

    bool setListCtrlColumns();

    void initSortTypeMapping();

    void extendContextMenu();

    // OpenGL:
    bool addGLTextureItem(QTreeWidgetItem* pTextureItemTreeItem, bool bSelectItem, gtUInt32& textureObjectSize, bool isInBeginEndBlock);
    bool addRenderBufferItem(QTreeWidgetItem* pbufferItemTreeItem, bool bSelectItem, gtUInt32& renderBufferSize);
    bool addStaticBufferItem(QTreeWidgetItem* pbufferItemTreeItem, bool bSelectItem, gtUInt32& staticBufferSize);
    bool addProgramItem(QTreeWidgetItem* pprogramItemTreeItem, bool bSelectItem, gtUInt32& programSize);
    bool addProgramPipelineItem(QTreeWidgetItem* pprogramItemTreeItem, bool shouldSelectItem);
    bool addShaderItem(QTreeWidgetItem* pShaderItemTreeItem, bool bSelectItem, gtUInt32& shaderSize);
    bool addDisplayListItem(QTreeWidgetItem* pdpyListItemTreeItem, bool bSelectItem, gtUInt32& dpyListSize);
    bool addFBOItem(QTreeWidgetItem* pfboItemTreeItem, bool bSelectItem, gtUInt32& fboSize);
    bool addFBOAttchmentItem(QTreeWidgetItem* pfboAttachmentItemTreeItem, bool shouldSelectItem, gtUInt32& fboSize);
    bool addPBufferItem(QTreeWidgetItem* pbufferItemTreeItem, bool bSelectItem, gtUInt32& pbufferSize);
    bool addPBufferStaticBufferItem(QTreeWidgetItem* pbufferItemTreeItem, bool shouldSelectItem, gtUInt32& pbufferSize);
    bool addContextItem(QTreeWidgetItem* prenderContextTreeItemId, bool bSelectItem, gtUInt32& renderContextSize);
    bool addSyncObjectItem(QTreeWidgetItem* psyncObjectItemTreeItem, bool bSelectItem, gtUInt32& pbufferSize);
    bool addRenderContextItem(QTreeWidgetItem* prenderContextTreeItemId, bool bSelectItem, gtUInt32& renderContextSize);
    bool addGLContextChildObjectToList(QTreeWidgetItem* pobjectItemTreeItem, bool bSelectItem, gtUInt32& renderContextObjectSize);
    bool addVBOItem(QTreeWidgetItem* pvboItemTreeItem, bool bSelectItem, gtUInt32& vboSize);

    // OpenCL:
    bool addCLContextChildObjectToList(QTreeWidgetItem* pobjectItemTreeItem, bool bSelectItem, gtUInt32& clContextObjectSize);
    bool addCLImageItem(QTreeWidgetItem* ptextureItemTreeItem, bool bSelectItem, gtUInt32& textureObjectSize);
    bool addCLBufferItem(QTreeWidgetItem* pbufferItemTreeItem, bool bSelectItem, gtUInt32& bufferObjectSize);
    bool addCLSubBufferItem(QTreeWidgetItem* psubBufferItemTreeItem, bool bSelectItem, gtUInt32& bufferObjectSize);
    bool addCLPipeItem(QTreeWidgetItem* pPipeItemTreeItem, bool bSelectItem, gtUInt32& pipeObjectSize);
    bool addCLCommandQueueItem(QTreeWidgetItem* pcommandQueueItemTreeItem, bool bSelectItem, gtUInt32& commandQueueObjectSize);
    bool addCLProgramItem(QTreeWidgetItem* pprogramItemTreeItem, bool bSelectItem, gtUInt32& programObjectSize);
    bool addCLKernelItem(QTreeWidgetItem* pkernelItemTreeItem, bool bSelectItem, gtUInt32& kernelObjectSize);
    bool addCLSamplerItem(QTreeWidgetItem* psamplerItemTreeItem, bool bSelectItem, gtUInt32& programObjectSize);
    bool addCLEventItem(QTreeWidgetItem* pEventItemTreeItem, bool bSelectItem, gtUInt32& programObjectSize);

    void calculateTreeItemSize(QTreeWidgetItem* ptreeItemId, gtUInt32& itemSize);
    bool addTotalItem(gtUInt32 totalSize, bool isInBeginEndBlock, unsigned int objectsCount = 0);

    static void sizeStringFromNumber(gtString& sizeString, gtUInt32 size);
    bool addGlSamplerItem(QTreeWidgetItem* pitemToAdd, bool shouldSelectItem);
private:

    // Pointer to the memory tree viewer:
    gdDebugApplicationTreeHandler* _pMemoryTree;

    // Context id:
    int _chosenContextId;

    // Contain true iff we're in the process of resize:
    bool _shouldIgnoreSizeEvent;

    gdMemorySortInfo _sortInfo;
    gtMap<afTreeItemType, gtVector<gdMemoryAnalysisDetailsSortDirection>*> _memoryObjectTypeClickedColumnToSortType;

    // The last item updated - is used for avoiding multiple list update:
    afApplicationTreeItemData m_currentDisplayedItem;
    afTreeItemType m_currentDisplayedItemType;

    // Context menu:
    QAction* _pExportAction;

private:
    friend class gdSaveFunctionCallsStatisticsCommand;

};


#endif  // __gdTotalStatisticsView
