//------------------------------ gpTraceModels.h ------------------------------

#ifndef _GPTRACEMODELS_H_
#define _GPTRACEMODELS_H_

#include <qtIgnoreCompilerWarnings.h>

// Qt:
#include <QAbstractItemModel>
#include <QtWidgets>

// Local:
#include <AMDTGpuProfiling/CLAPIDefs.h>
#include <AMDTGpuProfiling/ProfileSessionDataItem.h>

class gpTraceDataContainer;
class SymbolInfo;

// ----------------------------------------------------------------------------------
// Class Name:          DXTableModelBase
// General Description: This class is a base class for all DX items tables / trees models.
//                      Use this model for an API table
// ----------------------------------------------------------------------------------
class gpTableModelBase : public QAbstractItemModel
{
    Q_OBJECT

public:

    /// Constructor
    /// \param the thread id (for CPU trace)
    /// \param pDataContainer the session data container
    gpTableModelBase(gpTraceDataContainer* pDataContainer);

    /// Constructor
    /// \param the thread id (for CPU trace)
    /// \param pDataContainer the session data container
    gpTableModelBase(osThreadId threadID, gpTraceDataContainer* pDataContainer);

    /// Destructor
    virtual ~gpTableModelBase();

    /// Override ancestor method. Gets the number of rows in the table
    /// \param parent the parent index
    /// \return the number of rows
    int rowCount(const QModelIndex& parent) const;

    /// Override ancestor method. Gets the number of columns in the table
    /// \param parent the parent index
    /// \return the number of columns
    int columnCount(const QModelIndex& parent) const;

    /// Override ancestor method. Gets the data for the specified index and role
    /// \param index the model index
    /// \param role the role
    /// \return the data
    QVariant data(const QModelIndex& index, int role) const;

    /// Override ancestor method. Gets the header data for the specified section, orientation and role
    /// \param section the header section
    /// \param orientation the orientation
    /// \param role the role
    /// \return the header data
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    /// Override ancestor method. Gets the flags for the specified index
    /// \param index the index whose flags are needed
    /// \return the flags for the specified index
    Qt::ItemFlags flags(const QModelIndex& index) const;

    /// Override ancestor method. Gets the index for the specified row, column, parent
    /// \param row the row whose index is needed
    /// \param column the column whose index is needed
    /// \param parent the parent whose index is needed
    /// \return the index for the specified row, column, parent
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;

    /// Override ancestor method. Gets the parent index for the specified index
    /// \param index the index whose parent is needed
    /// \return the parent index for the specified index
    virtual QModelIndex parent(const QModelIndex& index) const;

    /// Sets the model visual properties (colors, sizes etc'):
    /// \param defaultForegroundColor the default color used for items in the table
    /// \param linkColor the color to use for painting hyper-links
    /// \param font the font used in the table
    /// \param filteredColumns list of columns that should not be displayed
    void SetVisualProperties(const QColor& defaultForegroundColor, const QColor& linkColor, const QFont& font);

    /// Get an item for the requested QModelIndex:
    /// \param item the item's model index
    /// \return the ProfileSessionDataItem item representing the item
    virtual ProfileSessionDataItem* GetItem(const QModelIndex& item) const;

    /// Retrieve the symbol info for the API specified by threadId and callIndex
    /// The function simply access the data container and get the info
    /// \param callIndex the call index of the API
    /// \return the symbol info for the API specified by threadId and callIndex or NULL if not found
    SymbolInfo* GetSymbolInfo(int callIndex);

    /// Does the session contain symbol information?
    /// \return true for session with symbol information
    bool SessionHasSymbolInformation();

    /// Export the table data to a csv file
    /// \param outputFilePath the csv file path
    virtual bool ExportToCSV(const QString& outputFilePath);

    /// Translates a table physical index to ProfileSessionDataItem::ProfileSessionDataColumnIndex
    /// \param tableColumnIndex the physical index
    /// \return the logical index based on the table content
    ProfileSessionDataItem::ProfileSessionDataColumnIndex TableIndexToColumnIndex(int tableColumnIndex);

protected:

    /// The session data container object
    gpTraceDataContainer* m_pSessionDataContainer;

    /// Vector of indices to display
    QVector<ProfileSessionDataItem::ProfileSessionDataColumnIndex> m_columnIndicesVec;

    /// The color for links in the table:
    QColor m_linkColor;

    /// The default foreground (font) color for this model:
    QColor m_defaultForegroundColor;

    /// The font used for this model:
    QFont m_font;

    /// The underlined font used for this model:
    QFont m_underlineFont;

    /// Contain the table thread ID (for CPU traces)
    osThreadId m_threadID;

};
class DXAPIGPUTableModel : public gpTableModelBase
{
    Q_OBJECT

public:

    /// \param the queue name (for GPU trace)
    /// \param pDataContainer the session data container
    DXAPIGPUTableModel(const QString& queueName, gpTraceDataContainer* pDataContainer);

    /// Destructor
    virtual ~DXAPIGPUTableModel();

    /// Override ancestor method. Gets the number of rows in the table
    /// \param parent the parent index
    /// \return the number of rows
    virtual int rowCount(const QModelIndex& parent) const;

    /// Override ancestor method. Gets the index for the specified row, column, parent
    /// \param row the row whose index is needed
    /// \param column the column whose index is needed
    /// \param parent the parent whose index is needed
    /// \return the index for the specified row, column, parent
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;

    /// Get an item for the requested QModelIndex:
    /// \param item the item's model index
    /// \return the ProfileSessionDataItem item representing the item
    virtual ProfileSessionDataItem* GetItem(const QModelIndex& item) const;
    virtual bool ExportToCSV(const QString& outputFilePath);

protected:

    /// Contain the queue name (for GPU trace)
    QString m_queueName;

};

class DXAPITreeModel : public gpTableModelBase
{
    Q_OBJECT

public:

    DXAPITreeModel(osThreadId threadID, gpTraceDataContainer* pDataContainer);
    ~DXAPITreeModel();

    /// Override ancestor method. Gets the number of rows in the table
    /// \param parent the parent index
    /// \return the number of rows
    int rowCount(const QModelIndex& parent) const;

    /// Override ancestor method. Gets the index for the specified row, column, parent
    /// \param row the row whose index is needed
    /// \param column the column whose index is needed
    /// \param parent the parent whose index is needed
    /// \return the index for the specified row, column, parent
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;

    /// Override ancestor method. Gets the parent index for the specified index
    /// \param index the index whose parent is needed
    /// \return the parent index for the specified index
    QModelIndex parent(const QModelIndex& index) const;

    /// Get an item for the requested QModelIndex:
    /// \param item the item's model index
    /// \return the ProfileSessionDataItem item representing the item
    virtual ProfileSessionDataItem* GetItem(const QModelIndex& item) const;


    /// Export the table data to a csv file
    /// \param outputFilePath the csv file path
    virtual bool ExportToCSV(const QString& outputFilePath);

protected:

    /// The root of the tree
    ProfileSessionDataItem* m_pRootItem;
};

#endif // GPTRACEMODELS__H
