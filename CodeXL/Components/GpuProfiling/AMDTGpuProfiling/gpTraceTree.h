//------------------------------ gpTraceTree.h ------------------------------

#ifndef _GPTRACETREE_H_
#define _GPTRACETREE_H_

#include <qtIgnoreCompilerWarnings.h>

// Qt:
#include <QTreeView>
#include <QAbstractItemModel>

// Local:
#include <AMDTGpuProfiling/CLAPIDefs.h>
#include <AMDTGpuProfiling/ProfileSessionDataItem.h>

class gpTraceDataContainer;
class DXAPITreeModel;
class SymbolInfo;
class gpTraceView;
// ----------------------------------------------------------------------------------
// Class Name:          gpTraceTree
// General Description: A trace table for a DX profile session
// ----------------------------------------------------------------------------------
class gpTraceTree : public QTreeView
{
    Q_OBJECT

public:

    /// Constructor
    /// \param threadID the table thread ID
    /// \param pDataContainer the profiled session data container
    /// \param pSessionView the parent session view
    gpTraceTree(osThreadId threadID, gpTraceDataContainer* pDataContainer, gpTraceView* pSessionView);
    ~gpTraceTree();

    /// Get an item for the requested QModelIndex:
    /// \param item the item's model index
    /// \return the ProfileSessionDataItem item representing the item
    ProfileSessionDataItem* GetItem(const QModelIndex& item);

    /// Translates a table physical index to ProfileSessionDataItem::ProfileSessionDataColumnIndex
    /// \param tableColumnIndex the physical index
    /// \return the logical index based on the table content
    ProfileSessionDataItem::ProfileSessionDataColumnIndex TableIndexToColumnIndex(int tableColumnIndex);

    /// Selects 1 or more rows in the table with the given sampleId
    void SelectRowBySampleId(ProfileSessionDataItem* pItem);

protected slots:

    /// Is handling the context menu creation and display
    /// \param pt the point at which context menu is requested
    void OnContextMenu(const QPoint& pt);

    /// Is handling the copy action
    void OnEditCopy();

    /// Is handling the select all action
    void OnEditSelectAll();

    /// handler for when the Goto source menu item is selected
    void OnGotoSource();

    /// handler for when the zoom item in timeline is selected
    void OnZoomItemInTimeline();

    /// handler for when the collapse all action is selected
    void OnCollapseAll();

    /// handler for when the expand all action is selected
    void OnExpandAll();

    /// Handler for export to CSV context menu:
    void OnExportToCSV();

protected:
    /// Overridden handler to handle resizing of the column headers
    /// \param event the event params
    void resizeEvent(QResizeEvent* event);

    /// Build the context menu for the table
    void BuildContextMenu();

private:
    /// Get the fill weight for the specified section (column)
    /// \param sectionIndex the index of the section whose fill weight is needed
    /// \return the the fill weight for the specified section (column)
    float GetSectionFillWeight(int sectionIndex);

    /// The data model for this table
    DXAPITreeModel* m_pDataModel;

    /// A pointer to the parent session view
    gpTraceView* m_pSessionView;

    /// Context menu and actions
    QMenu* m_pContextMenu;
    QAction* m_pGotoSourceAction;
    QAction* m_pZoomInTimelineAction;
    QAction* m_pExpandAllAction;
    QAction* m_pCollapseAllAction;
    QAction* m_pCopyAction;
    QAction* m_pSelectAllAction;
    QAction* m_pExportToCSVAction;

    /// Used for caching the symbol info between the context menu handler and the "Go to source" click handler
    SymbolInfo* m_pCahcedSymbolInfo;

};


#endif // _GPTRACETREE_H_