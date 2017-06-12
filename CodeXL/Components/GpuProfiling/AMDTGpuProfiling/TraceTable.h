//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/TraceTable.h $
/// \version $Revision: #34 $
/// \brief  This file contains TraceTable class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/TraceTable.h#34 $
// Last checkin:   $DateTime: 2016/04/13 04:25:48 $
// Last edited by: $Author: rbober $
// Change list:    $Change: 568808 $
//=====================================================================
#ifndef _TRACE_TABLE_H_
#define _TRACE_TABLE_H_

#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable: 4127)
    #pragma warning(disable: 4189)
    #pragma warning(disable: 4251)
#endif
// Qt
#include <QtWidgets>
#include <QTreeView>
#include <QStandardItem>
#include <QStack>
#ifdef _WIN32
    #pragma warning(pop)
#endif

// boost
#include <boost/icl/split_interval_map.hpp>

//BackEnd
#include <ATPParserInterface.h>

// forward declarations
class acTimelineItem;

// enum for trace table item types
enum TraceTableItemType
{
    API,            ///< API Item
    PERFMARKER      ///< Perfmarker Item
};

/// Class to hold the item data for the API Trace table model
class TraceTableItem
{
public:
    /// Initializes a new instance of the TraceTableItem class
    /// \param strAPIPrefix an API-specific prefix used (along with the sequence id) to uniquely identify each trace item (i.e. "OpenCL" or "HSA")
    /// \param strApiName the name of the method for this item
    /// \param pApiInfo structure containing info about this api (params, return val, timestamps, etc...)
    /// \param pTimelineItem the timeline item that corresponds to this trace item
    /// \param pDeviceBlock the device block timeline item that corresponds to this trace item (can be NULL)
    /// \param pOccupancyInfo the occupancy info that corresponds to this trace item (can be NULL)
    TraceTableItem(const QString& strAPIPrefix, const QString& strApiName, IAPIInfoDataHandler* pApiInfo, acTimelineItem* pTimelineItem, acTimelineItem* pDeviceBlock, IOccupancyInfoDataHandler* pOccupancyInfo);

    /// Initializes a new instance of the TraceTableItem class
    /// \param strAPIPrefix an API-specific prefix used (along with the sequence id) to uniquely identify each trace item (i.e. "OpenCL" or "HSA")
    /// \param strMarkerName the name of the marker for this item
    /// \param pMarkerEntry structure containing info about this marker
    /// \param pTimelineItem the timeline item that corresponds to this trace item
    TraceTableItem(const QString& strAPIPrefix, const QString& strMarkerName, IPerfMarkerInfoDataHandler* pMarkerEntry, acTimelineItem* pTimelineItem);

    /// Destructor
    virtual ~TraceTableItem();

    /// Appends a child to the end of the child list of this item
    /// \param child the child item to append
    void AppendChild(TraceTableItem* child);

    /// Inserts a child at the specified index in the child list of this item
    /// \param index the index at which to insert the child
    /// \param child the child item to append
    void InsertChild(int index, TraceTableItem* child);

    /// Gets the parent of this item
    /// \return the parent of this item
    TraceTableItem* GetParent() const { return m_parent; }

    /// Gets the unique ID for this item
    /// \return the unique ID for this item
    QVariant GetUniqueId() { return m_uniqueId; }

    /// Gets the number of children this node has
    /// \return the number of children this node has
    int GetChildCount() const { return m_children.count(); }

    /// Reserves a count places of children:
    void ReserveChildrenCount(int count) { m_children.reserve(count); }

    /// Gets the specified child
    /// \param childIndex the index of the child requested
    /// \return the specified child
    TraceTableItem* GetChild(int childIndex) const { return m_children[childIndex]; }

    /// Gets the data for the specified column
    /// \param columnIndex the index of the column whose data is needed
    /// \return the data for the specified column
    QVariant GetColumnData(int columnIndex) const;

    /// Gets the data for the specified column
    /// \param columnIndex the index of the column whose data should be set
    /// \param data the data for the specified column
    void SetColumnData(int columnIndex, QVariant data);

    /// Gets the timeline item associated with this item
    /// \return the timeline item associated with this item
    acTimelineItem* GetTimelineItem() const { return m_pTimelineItem; }

    /// \Set the timeline item associated with this item
    void SetTimelineItem(acTimelineItem* pItem) { m_pTimelineItem = pItem; }

    /// Gets the device block timeline item associated with this item (can be null)
    /// \return the device block timeline item associated with this item
    acTimelineItem* GetDeviceBlock() const { return m_pDeviceBlock; }

    /// Gets the occupancy info associated with this item (can be null)
    /// \return the occupancy info associated with this item
    IOccupancyInfoDataHandler* GetOccupancyInfo() const { return m_pOccupancyInfo; }

    /// Gets the index of this item within its parent
    /// \return the index of this item within its parent
    int GetRow() const;

    /// Update the start & end indices with the current added child index:
    /// \param childStartIndex start updating from childStartIndex
    /// \param childEndIndex end updating at childEndIndex
    void UpdateIndices(int childStartIndex, int childEndIndex);

    /// Gets the item start index
    /// \return the item start index
    int GetStartIndex() const { return m_startIndex; }

    /// Gets the item end index
    /// \return the item end index
    int GetEndIndex() const { return m_endIndex; }

    /// Updates the marker name for an existing perf marker
    /// \param strMarkerName the new marker name for the item
    void UpdateMarkerName(const QString& strMarkerName);

    /// type of the trace table item
    TraceTableItemType itemType;

private:
    /// Disable copy constructor
    TraceTableItem(const TraceTableItem&);

    /// Disable default assignment operator
    const TraceTableItem& operator=(const TraceTableItem& obj);

    QList<TraceTableItem*> m_children;  ///< list of children of this item
    TraceTableItem* m_parent;           ///< the parent of this item

    QList<QVariant> m_data;             ///< list of column data
    int m_startIndex;                   ///< The item start index (-1 in case where index is irelevant)
    int m_endIndex;                     ///< The item end index (-1 in case where index is irelevant)
    acTimelineItem* m_pTimelineItem;    ///< the timeline item for this trace table item
    acTimelineItem* m_pDeviceBlock;     ///< the device block for this trace table item (can be null)
    IOccupancyInfoDataHandler* m_pOccupancyInfo;    ///< the occupancy info for this trace table item (can be null)
    QVariant m_uniqueId;                ///< the unique id of the item.  This is returned from the model data for the Qt::UserRole
};

/// QAbstractItemModel descendant used for the API trace table in the TraceView
class TraceTableModel : public QAbstractItemModel
{
    Q_OBJECT;

public:

    enum TraceTableColIndex
    {
        TRACE_INDEX_COLUMN = 0,
        TRACE_INTERFACE_COLUMN = 1,
        TRACE_PARAMETERS_COLUMN = 2,
        TRACE_RESULT_COLUMN = 3,
        TRACE_DEVICE_BLOCK_COLUMN = 4,
        TRACE_OCCUPANCY_COLUMN = 5,
        TRACE_CPU_TIME_COLUMN = 6,
        TRACE_DEVICE_TIME_COLUMN = 7,
        TRACE_COLUMN_COUNT = TRACE_DEVICE_TIME_COLUMN + 1
    };


    /// Initializes a new instance of the TraceTableModel class
    /// \param parent the parent object
    TraceTableModel(QObject* parent);

    /// Destructor
    ~TraceTableModel();

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
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;

    /// Override ancestor method. Gets the parent index for the specified index
    /// \param index the index whose parent is needed
    /// \return the parent index for the specified index
    QModelIndex parent(const QModelIndex& index) const;

    /// Sets the visual properties for the TraceTableModel
    /// \param defaultForegroundColor the default color used for items in the table
    /// \param linkColor the color to use for painting hyper-links
    /// \param font the font used in the table
    void SetVisualProperties(const QColor& defaultForegroundColor, const QColor& linkColor, const QFont& font);

    /// Build the list of headers that should be displayed:
    void BuildHeaderData();

    /// Adds an API item to the maps of trace items (this map will later be added to the model, in InitializeModel)
    /// \param strAPIPrefix an API-specific prefix used to uniquely identify each trace item
    /// \param strApiName the name of the api
    /// \param pApiInfo struct containing info for the api
    /// \param pTimelineItem the item's timeline item
    /// \param pDeviceBlock the device block for the api, can be NULL
    /// \param pOccupancyInfo the occupancy info for the api, can be NULL
    /// \return the TraceTableItem instance added to the model
    TraceTableItem* AddTraceItem(const QString& strAPIPrefix, const QString& strApiName, IAPIInfoDataHandler* pApiInfo, acTimelineItem* pTimelineItem, acTimelineItem* pDeviceBlock, IOccupancyInfoDataHandler* pOccupancyInfo);

    /// Adds a top level item to the trace items table:
    /// \param strAPIPrefix an API-specific prefix used to uniquely identify each trace item
    /// \param strApiName the name of the api
    /// \param pApiInfo struct containing info for the api
    /// \param pTimelineItem the item's timeline item
    /// \param pDeviceBlock the device block for the api, can be NULL
    /// \param pOccupancyInfo the occupancy info for the api, can be NULL
    /// \return the TraceTableItem instance added to the model
    TraceTableItem* AddTopLevelTraceItem(const QString& strAPIPrefix, const QString& strApiName, IAPIInfoDataHandler* pApiInfo, acTimelineItem* pTimelineItem, acTimelineItem* pDeviceBlock, IOccupancyInfoDataHandler* pOccupancyInfo);

    /// Adds a perf marker item to the maps of trace items (this map will later be added to the model, in InitializeModel)
    /// \param strAPIPrefix an API-specific prefix used to uniquely identify each trace item
    /// \param strMarkerName the name of the marker
    /// \param pMarkerEntry struct containing info for the marker
    TraceTableItem* AddTraceItem(const QString& strAPIPrefix, const QString& strMarkerName, IPerfMarkerInfoDataHandler* pMarkerEntry);

    /// Gets the device block data for the specified row
    /// \param index the index of the row
    /// \return the timeline item representing the device block
    acTimelineItem* GetDeviceBlock(const QModelIndex& index);

    /// Gets the occupancy data for the specified row
    /// \param index the index of the row
    /// \return the occupancy info
    IOccupancyInfoDataHandler* GetOccupancyItem(const QModelIndex& index);

    /// Closing the last opened perf marker item:
    /// \param pTimelineItem the timeline item that matches the requested perf marker item
    /// \return the table item
    TraceTableItem* CloseLastOpenedPerfMarker(acTimelineItem* pTimelineItem);

    /// Initialize the model: construct the tree structure from the maps of API, HSA, and markers:
    /// \return true iff the initialization was successful
    bool InitializeModel();

    /// Return true iff the root has grandchildren:
    /// \return the flag stating if the root has grandchildren
    bool ShouldExpandBeEnabled()const {return m_shouldExpandBeEnabled;}

    /// Sets the api calls number for the thread related to this table model:
    /// \param apiNum the api calls number for this thread
    void SetAPICallsNumber(unsigned int apiNum);

    /// get the number of Api Calls Trace Items to be set
    int GetReservedApiCallsTraceItems() const {return m_reservedApiCallsTraceItems;}

    /// Export the model data to a CSV file
    /// \param outputFilePath the CSV output file path
    /// \return true if the export succeeded
    bool ExportToCSV(const QString& outputFilePath);

    /// Return true when there is no API or perf markers in the table:
    bool IsEmpty() const { return (m_apiCallsTraceItemsMap.empty() && m_perfMarkersTraceItemsMap.isEmpty()); }

    ///types and definitions
private:
    /// Maps containing the perf markers trace items:
    using TimeRange = QPair<quint64, quint64>;
    using ApiCallsTraceMapItr = std::map<quint64, TraceTableItem*>::iterator;
    using PerfMarkersMapItr = QMap<TimeRange, TraceTableItem*>::iterator;

    ///Methods
private:

    /// This method :
    /// 1. calculates next TraceTableItem to be connected with its parent during table initialization
    /// 2. advances input api and markers  iterators
    /// \param markersIter inout marker iterator, advanced each time this method called unless it reaches end
    /// \param markersIterEnd marker end iterator
    /// \param apiIter inout api iterator, advanced each time this method called unless it reaches end
    /// \param apiIterEnd api end iterator
    /// \return TraceTableItem* pointer to next TraceTableItem that shall be connected to it's parent  root or  marker
    TraceTableItem* CalculateNextTraceTableItem(PerfMarkersMapItr& markersIter, const PerfMarkersMapItr& markersIterEnd,
                                                ApiCallsTraceMapItr& apiIter, const ApiCallsTraceMapItr& apiIterEnd, TraceTableItemType& itemType) const;


    /// This method returns items parent , by default it's a root item,unless time overlapping marker item is found
    /// \param pNextItemToAdd item for which parent needs to be found
    /// \return TraceTableItem* pointer to parent for a given item
    TraceTableItem* GetNextItemParent(const TraceTableItem* pNextItemToAdd, const TraceTableItemType& itemType) const;

private:

    QStringList                m_headerData;             ///< the header data for this model
    TraceTableItem*            m_pRootItem;              ///< the root item of the trace table (tree)
    QColor                     m_defaultForegroundColor; ///< the default foreground (font) color for this model
    QColor                     m_linkColor;              ///< the link color for this model
    QFont                      m_font;                   ///< the font used for this model
    QFont                      m_underlineFont;          ///< the underlined font used for this model

    QStack<TraceTableItem*> m_openedPerfMarkerItemsStack;

    /// Maps containing the API call trace items:
    std::map<quint64, TraceTableItem*> m_apiCallsTraceItemsMap;

    /// number of reserved items for m_apiCallsTraceItemsMap -
    int m_reservedApiCallsTraceItems;

    /// Maps containing the perf markers trace items:
    QMap<TimeRange, TraceTableItem*> m_perfMarkersTraceItemsMap;


    /// holds all markers sorted by their time range intervals
    boost::icl::split_interval_map<quint64, TraceTableItem*, boost::icl::partial_absorber, std::less, boost::icl::inplace_max> m_markerIntervals;

    /// structure for holding marker information
    struct Marker
    {
        /// pointer to the marker
        TraceTableItem* pPerfMarkerItem;

        /// pointer to the parent of the marker
        TraceTableItem* pParentMarker;

        /// Constructor
        Marker() : pPerfMarkerItem(nullptr),
            pParentMarker(nullptr)
        {}
    };

    /// list of the perf markers
    std::vector<Marker> m_PerfmarkerList;

    /// Was the model initialized already?
    bool m_isInitialized;

    /// Does the root item has grandchildren?
    bool m_shouldExpandBeEnabled;
};

/// QTreeView descendant that hosts an API Trace table
class TraceTable : public QTreeView
{
    Q_OBJECT

public:

    /// Initializes a new instance of the TraceTable class.
    /// \param parent parent widget
    /// \param threadId the thread id for this trace table
    TraceTable(QWidget* parent, unsigned int threadId);

    /// Destructor
    virtual ~TraceTable();

    /// Gets the thread Id for this trace table
    /// \return the thread Id for this trace table
    unsigned int GetThreadId() const { return m_threadId; }


    // Edit actions:
    virtual void onUpdateEdit_Copy(bool& isEnabled);
    virtual void onUpdateEdit_SelectAll(bool& isEnabled) {isEnabled = true;};

    /// Contain true iff one of the root node children has children:
    /// \return the flag stating if the root node has grandchildren
    bool ShouldExpandBeEnabled() const;

    /// get the number of selected rows
    /// \returns the number of selected rows
    int NumOfSelectedRows() const;

    /// Export the table table content into a CSV file
    /// \param outputFilePath the CSV output file path
    /// \return true if the export succeeded
    bool ExportToCSV(const QString& outputFilePath);

    /// Set the list of columns that should be hidden:
    /// \param the list of hidden columns
    void SetHiddenColumnsList(const QList<TraceTableModel::TraceTableColIndex>& hiddenColumnsList);

public slots:

    /// Copies any selected text to the clipboard:
    virtual void OnEditCopy();

    /// Select all:
    virtual void OnEditSelectAll();


protected:
    /// Overridden handler to handle resizing of the column headers
    /// \param event the event params
    void resizeEvent(QResizeEvent* event);

private:
    /// Get the fill weight for the specified section (column)
    /// \param sectionIndex the index of the section whose fill weight is needed
    /// \return the the fill weight for the specified section (column)
    float GetSectionFillWeight(int sectionIndex);

    unsigned int m_threadId; ///< thread id of the api trace in this table

};


#endif // _TRACE_TABLE_H_
