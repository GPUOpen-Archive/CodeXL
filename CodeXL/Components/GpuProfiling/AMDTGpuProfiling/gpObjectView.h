//=====================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/gpObjectView.h $
/// \version $Revision: #1 $
/// \brief  This file contains gpObjectView class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/gpObjectView.h#1 $
// Last checkin:   $DateTime: 2015/07/05 04:24:05 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 533050 $
//=====================================================================

#ifndef __GPOBJECTVIEW_H_
#define __GPOBJECTVIEW_H_

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
#include <AMDTGpuProfiling/gpObjectDataModel.h>
#include <AMDTGpuProfiling/gpObjectModels.h>

// forward declaration
class gpTimeline;
class acTimelineItem;
class acAPITimelineItem;
class acTabWidget;
class SessionTreeNodeData;
class gpObjectDataContainer;
class gpObjectSummaryWidget;
class ProfileSessionDataItem;

class APICallId;
class APIUIInfo;

/// UI for Application Object GPUSessionTreeItemData view
class gpObjectView : public gpBaseSessionView
{
    Q_OBJECT

public:

    /// Initializes a new instance of the gpSessionView class.
    /// \param parent the parent widget
    gpObjectView(QWidget* pParent);

    /// Destructor
    ~gpObjectView();

    /// Display the session file. This function should be implemented for session views with multiple children
    /// \param sessionFilePath the file path for the requested session
    /// \param sessionInnerPage the item type describing the inner view to open, or AF_TREE_ITEM_ITEM_NONE when the root is supposed to open
    /// \param [out] errorMessage when the display fails, errorMessage should contain a message for the user
    virtual bool DisplaySession(const osFilePath& sessionFilePath, afTreeItemType sessionInnerPage, QString& errorMessage);

    /// Set the profile data container
    void SetProfileDataContainer(gpObjectDataContainer* pObjectSessionDataContainer) { m_pSessionDataContainer = pObjectSessionDataContainer; };

    /// Sets the timeline item for an API call
    /// \param apiId the API call id (thread id + API index)
    /// \param the timeline item describing the API call
    //    void SetAPICallTimelineItem(APICallId apiId, acAPITimelineItem* pTimelineItem);

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
    /// \param shouldSelect should the zoomed item be selected?
    void ZoomToItem(acTimelineItem* pTimelineItem, bool shouldSelect);

    /// Set the data model
    virtual void SetProfileObjectDataModel(gpObjectDataModel* pSessionDataModel);

signals:
    void viewObjSelected(QString strObjSelected);

protected slots:

    /// handle object tree item selection
    void OnTreeObjSelected(const QItemSelection& newSelection, const QItemSelection& oldSelection);

    void OnObjDisplayed(QString strObjDisplayed);

    void OnIncludeDestroyedChanged(int iCheckState);
    void OnGroupDestroyedChanged(int iCheckState);

public slots:
    /// Sent when summary item is clicked - invokes selection in other views
    /// \param pItem - the clicked item
    void OnSummaryItemClicked(ProfileSessionDataItem* pItem);

protected:

    /// Create the necessary preparations before displaying the trace view
    /// * Parse the frame file name and get the frame index
    /// * Make sure that the file exists, and contain the data from the server
    /// * Parse the file
    bool PrepareObjectFile();

protected:
    /// The tab-widget containing the Summary tables
    gpObjectSummaryWidget* m_pSummaryTableTabWidget;

    /// The session data container object
    gpObjectDataContainer* m_pSessionDataContainer;

    /// Map containing UI information for each of the API calls (timeline item, device block timeline item etc')
    QMap<APICallId, APIUIInfo> m_apiItemsUIMap;

    /// Contain the displayed frame index
    int m_frameIndex;

private:
    int            m_iIncludeDestroyed = true;

    int            m_iGroupDestroyed = false;

    QTreeView*      m_pObjTreeView = nullptr;

    QTreeView*      m_pObjDBaseTreeView = nullptr;

    ObjTreeModel*   m_oTreeModel = nullptr;

    ObjDatabaseModel* m_oDBaseModel = nullptr;

    QLabel*         m_pLblDeviceInfo = nullptr;

    QLabel*         m_pLblTypeInfo = nullptr;

    QLabel*         m_pLblTagDataInfo = nullptr;

    QTextEdit*      m_pTxtSelectedObject = nullptr;

    QHBoxLayout*    pMainLayout = nullptr;
    QHBoxLayout*    pDeviceLayout = nullptr;
    QHBoxLayout*    pTypeLayout = nullptr;
    QHBoxLayout*    pObjectItemLayout = nullptr;
    QVBoxLayout*    pLLayout = nullptr;
    QVBoxLayout*    pRLayout = nullptr;
    QVBoxLayout*    pFilteringLayout = nullptr;
    QGroupBox*      pLGroupBox = nullptr;
    QGroupBox*      pRGroupBox = nullptr;
    QGroupBox*      pGroupBoxFiltering = nullptr;

    QCheckBox*      pChkIncludeDestroyed = nullptr;
    QCheckBox*      pChkGroupDestroyed = nullptr;

    QLabel*         plblDevice = nullptr;
    QLabel*         plblType = nullptr;

    QPushButton*    pBtnFindUsage = nullptr;

    void expandSelectTree(QTreeView* treeViewSelect);
};


#endif // __GPOBJECTVIEW_H_
