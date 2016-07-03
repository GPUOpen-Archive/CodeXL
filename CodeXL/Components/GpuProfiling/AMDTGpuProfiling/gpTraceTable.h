//------------------------------ gpTraceTable.h ------------------------------

#ifndef _GPTRACETABLE_H_
#define _GPTRACETABLE_H_

#include <qtIgnoreCompilerWarnings.h>

// Qt:
#include <QTableView>

// Local:
#include <AMDTGpuProfiling/CLAPIDefs.h>
#include <AMDTGpuProfiling/ProfileSessionDataItem.h>

class gpTraceDataContainer;
class gpTableModelBase;
class gpTraceView;
class SymbolInfo;
class afBrowseAction;

// ----------------------------------------------------------------------------------
// Class Name:          DXTraceTable
// General Description: A trace table for a DX profile session
// ----------------------------------------------------------------------------------
class gpTraceTable : public QTableView
{
    Q_OBJECT

public:

    /// Constructor
    /// \param threadID the table thread ID
    /// \param pDataContainer the session data container
    /// \param pSessionView the parent session view
    gpTraceTable(osThreadId threadID, gpTraceDataContainer* pDataContainer, gpTraceView* pSessionView);

    /// Constructor
    /// \param queueName the GPU items queue name
    /// \param pDataContainer the session data container
    /// \param pSessionView the parent session view
    gpTraceTable(const QString& queueName, gpTraceDataContainer* pDataContainer, gpTraceView* pSessionView);

    /// Destructor
    virtual ~gpTraceTable();

    /// Get an item for the requested QModelIndex:
    /// \param item the item's model index
    /// \return the ProfileSessionDataItem item representing the item
    ProfileSessionDataItem* GetItem(const QModelIndex& item);

    /// Translates a table physical index to ProfileSessionDataItem::ProfileSessionDataColumnIndex
    /// \param tableColumnIndex the physical index
    /// \return the logical index based on the table content
    ProfileSessionDataItem::ProfileSessionDataColumnIndex TableIndexToColumnIndex(int tableColumnIndex);

    /// set the color of the selection background (does not affect focus)
    void SetSelectionBackgroundColor(const QColor& color);

    /// selects a row by value of specified column
    void SelectRowByColValue(int col, int val);

    /// selects a row by index
    void SelectRow(int rowIndex);

public slots:

    /// Is handling the copy action
    void OnEditCopy();

    /// Is handling the select all action
    void OnEditSelectAll();

protected slots:

    /// Is handling the context menu creation and display
    /// \param pt the point at which context menu is requested
    void OnContextMenu(const QPoint& pt);

    /// handler for when the Goto source menu item is selected
    void OnGotoSource();

    /// handler for when the zoom item in timeline is selected
    void OnZoomItemInTimeline();

    /// Handler for export to CSV context menu:
    void OnExportToCSV();

    /// Is called when the table header is clicked for the first time
    void OnTableHeaderPressed(int section);


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
    gpTableModelBase* m_pDataModel;

    /// A pointer to the parent session view
    gpTraceView* m_pSessionView;

    /// Context menu and actions
    QMenu* m_pContextMenu;
    QAction* m_pZoomInTimelineAction;
    QAction* m_pCopyAction;
    QAction* m_pSelectAllAction;
    afBrowseAction* m_pExportToCSVAction;

    /// Used for caching the symbol info between the context menu handler and the "Go to source" click handler
    SymbolInfo* m_pCahcedSymbolInfo;

    /// This flag is true by default. The table columns are resized by default as long as the user don't touch the 
    /// table header handles. When the user first touches the header, the flag becomes false.
    /// This is a workaround. We would like to resize the table until the Qt view finishes resizing itself, but we don't know to track this point
    bool m_shouldAutoResizeTableColumns;
};

#endif // _GPTRACETABLE_H_