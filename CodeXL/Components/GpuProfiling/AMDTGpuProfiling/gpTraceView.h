//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/gpSessionView.h $
/// \version $Revision: #40 $
/// \brief  This file contains gpSessionView class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/gpSessionView.h#40 $
// Last checkin:   $DateTime: 2015/07/05 04:24:05 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 533050 $
//=====================================================================

#ifndef __GPTRACEVIEW_H_
#define __GPTRACEVIEW_H_

#include <QMainWindow>
#include <QMap>
#include <QList>
#include <QSplitter>
#include <QTabWidget>
#include <QAbstractTableModel>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/SharedSessionWindow.h>

// Backend:
#include "CLAPIInfo.h"

// Local:
#include <AMDTGpuProfiling/gpBaseSessionView.h>

// forward declaration
class gpTimeline;
class gpNavigationRibbon;
class acTimelineItem;
class acAPITimelineItem;
class acTabWidget;
class SessionTreeNodeData;
class gpTraceDataContainer;
class acRibbonManager;
class gpTraceSummaryWidget;
class ProfileSessionDataItem;
class gpDetailedDataRibbon;
class acTimelineBranch;
class gpRibbonDataCalculator;


enum gpSessionViewIndex
{
    GP_TIMELINE_PAGE_INDEX,
    GP_SUMMARY_PAGE_INDEX

};

/// This class is used to map an item in the table, with items in the timeline
class APICallId
{
public:
    APICallId();

    /// < operator To enable the usage of APICallId as a map key
    bool operator<(const APICallId& other) const;

    osThreadId m_tid;
    QString m_queueName;
    int m_callIndex;
};
class APIUIInfo
{
public:

    APIUIInfo()
    {
        m_pAPITimelineItem = nullptr;
        m_pDeviceBlock = nullptr;
    }
    acAPITimelineItem* m_pAPITimelineItem;
    acTimelineItem* m_pDeviceBlock;
};

/// UI for Application Trace GPUSessionTreeItemData view
class gpTraceView : public gpBaseSessionView
{
    Q_OBJECT

public:

    /// Initializes a new instance of the gpSessionView class.
    /// \param parent the parent widget
    gpTraceView(QWidget* pParent);

    /// Destructor
    ~gpTraceView();

    /// Display the session file. This function should be implemented for session views with multiple children
    /// \param sessionFilePath the file path for the requested session
    /// \param sessionInnerPage the item type describing the inner view to open, or AF_TREE_ITEM_ITEM_NONE when the root is supposed to open
    /// \param [out] errorMessage when the display fails, errorMessage should contain a message for the user
    virtual bool DisplaySession(const osFilePath& sessionFilePath, afTreeItemType sessionInnerPage, QString& errorMessage);

    /// Set the profile data container
    void SetProfileDataContainer(gpTraceDataContainer* pTraceSessionDataContainer) { m_pSessionDataContainer = pTraceSessionDataContainer; };

    /// Sets the timeline item for an API call
    /// \param apiId the API call id (thread id + API index)
    /// \param the timeline item describing the API call
    void SetAPICallTimelineItem(APICallId apiId, acAPITimelineItem* pTimelineItem);

    /// Sets the timeline item for an API call
    /// \param apiId the API call id (thread id + API index)
    /// \param the timeline item describing the API gpu item
    void SetGPUTimelineItem(APICallId apiId, acTimelineItem* pGPUTimelineItem);


    /// Sets the timeline item for an API call
    /// \param apiId the API call id (thread id + API index)
    /// \return the timeline item describing the API item
    acAPITimelineItem* GetAPITimelineItem(APICallId apiId);

    /// Zoom the timeline to the requested timeline item
    /// \param pTimelineItem the item to zoom into
    void ZoomToItem(ProfileSessionDataItem* pDataItem);

    /// Set the data model
    virtual void SetProfileDataModel(gpTraceDataModel* pSessionDataModel);

    /// Selects the requested item in CPU\GPU trace table
    void SelectItemInTraceTables(ProfileSessionDataItem* pItem, bool setFocus = true);

    /// Handlers and UI update functions for the edit commands
    virtual void onUpdateEdit_Copy(bool& isEnabled);
    virtual void onUpdateEdit_SelectAll(bool& isEnabled);
    virtual void onUpdateEdit_Find(bool& isEnabled);
    virtual void onUpdateEdit_FindNext(bool& isEnabled);

    virtual void OnEditCopy();
    virtual void OnEditSelectAll();

public slots:

    /// Find
    virtual void onFindClick();

    /// Find Next
    virtual void onFindNext();

protected slots:

    /// Is handling the click of a table item
    /// \param clickedItem the item clicked
    void OnTableItemClick(const QModelIndex& clickedItem);

    void SyncSelectionInSummary(ProfileSessionDataItem* pDataItem);

    /// Is handling the double click of a table item
    /// \param clickedItem the item clicked
    void OnTableItemDoubleClick(const QModelIndex& clickedItem);

    acTimelineItem* SessionItemToTimelineItem(ProfileSessionDataItem* pDataItem);

    /// Received when timeline filters are changed
    /// \param threadNameVisibilityMap - a map of thread names and visibility
    void OnTimelineFilterChanged(QMap<QString, bool>& threadNameVisibilityMap);

    /// Sent when time range changes (either by zoom or scroll)
    void OnTimelineRangeChanged();

    /// Is handling a double click on a timeline item
    void OnTimelineItemActivated(acTimelineItem* pTimelineItem);

    /// handle the navigation bar layer visible change
    void OnNavigationLayerVisibilityChanged();

    void OnAfterReplot();
    void OnCPUTableRowChanged(const QModelIndex&, const QModelIndex&);
    void OnGPUTableRowChanged(const QModelIndex&, const QModelIndex&);

    void SyncSelectionInAllTables(ProfileSessionDataItem* pDataItem);

public slots:
    /// Sent when summary item is clicked - invokes selection in other views
    /// \param pItem - the clicked item
    void OnSummaryItemClicked(ProfileSessionDataItem* pItem);

protected:

    /// Clears the selection in CPU\GPU trace table
    void ClearSelectionInTraceTables(bool isCPU);

protected:
    /// the ribbon manager
    acRibbonManager* m_pRibbonManager;

    // the navigation ribbon that holds the navigation chart
    gpNavigationRibbon* m_pNavigationRibbon;

    /// draw calls ribbon
    gpDetailedDataRibbon* m_pDetailedDataRibbon;

    /// Frame data calculator
    gpRibbonDataCalculator* m_pFrameDataCalculator;

    /// The DX timeline
    gpTimeline* m_pTimeline;

    /// The tab-widget containing the CPU trace tables
    acTabWidget* m_pCPUTraceTablesTabWidget;

    /// The tab-widget containing the GPU trace tables
    acTabWidget* m_pGPUTraceTablesTabWidget;

    /// The tab-widget containing the Summary tables
    gpTraceSummaryWidget* m_pSummaryTableTabWidget;

    /// The session data container object
    gpTraceDataContainer* m_pSessionDataContainer;

    /// Map containing UI information for each of the API calls (timeline item, device block timeline item etc')
    QMap<APICallId, APIUIInfo> m_apiItemsUIMap;

    /// A vector of pairs containing m_pCPUTraceTablesTabWidget's tabs and the matching caption for show/hide according to timeline filters
    QVector<QPair<QString, QAbstractItemView*>> m_visibilityFiltersMap;

    /// Contain the displayed frame index
    int m_frameIndex;

    /// True if the session was already loaded
    bool m_isSessionLoaded;

    /// True if the user selected an item in the CPU table
    bool m_isCpuAPISelectionChangeInProgress;

    /// True if the user selected an item in the GPU table
    bool m_isGpuAPISelectionChangeInProgress;

};


#endif // __GPTRACEVIEW_H_
