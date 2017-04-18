//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/gpPerformanceCountersDataView.h $
/// \version $Revision: #40 $
/// \brief  This file contains gpPerformanceCountersDataView class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/gpPerformanceCountersDataView.h#40 $
// Last checkin:   $DateTime: 2015/07/05 04:24:05 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 533050 $
//=====================================================================

#ifndef __GPPERFORMANCECOUNTERSDATAVIEW_H_
#define __GPPERFORMANCECOUNTERSDATAVIEW_H_

#include <QMainWindow>
// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/SharedSessionWindow.h>

// Local:
#include <AMDTGpuProfiling/gpBaseSessionView.h>

#pragma message ("TODO: FA: should be retrieved from server")
struct FrameSummaryData
{
    unsigned int m_apiCallsCount;
    unsigned int m_drawCallsCount;
    float m_cpuTimeMS;
    float m_gpuTimeMS;
    float m_cpuTimeInDrawPercentage;
    float m_gpuBusyPercentage;
};

/// UI for performance counters data view
class gpPerformanceCountersDataView : public gpBaseSessionView
{
    Q_OBJECT

public:

    /// Initializes a new instance of the gpPerformanceCountersDataView class.
    /// \param parent the parent widget
    gpPerformanceCountersDataView(QWidget* pParent);

    /// Destructor
    ~gpPerformanceCountersDataView();

    /// Display the session file. This function should be implemented for session views with multiple children
    /// \param sessionFilePath the file path for the requested session
    /// \param sessionInnerPage the item type describing the inner view to open, or AF_TREE_ITEM_ITEM_NONE when the root is supposed to open
    virtual bool DisplaySession(const osFilePath& sessionFilePath, afTreeItemType sessionInnerPage);

    /// Add a call child element to the state bucket tree node
    /// callValuesStrings list of strings describing the call
    void AddDrawCallToStateBucket(const QStringList& callValuesStrings);

    /// Set the current summary data in the summary label
    void BuildSummaryData();

protected slots:

    void OnCountersSelectionClick();
    void OnDisplaySettingsClick();

protected:

    // Tree management functions. Will be replaced by QTreeView + model if performance will have to improve

    /// Get the top level item related to state bucket with index stateBucketIndex. The node is created if needed
    /// \param stateBucketIndex the state bucket index
    /// \return the created (or existing) state bucket top level item
    QTreeWidgetItem* GetStateBucketItem(int stateBucketIndex);

protected:

    QTreeWidget* m_pPerformanceCountersTree;
    QWidget* m_pSummaryWidget;
    QLabel* m_pCountersDescriptionLabel;
    QLabel* m_pSummaryLabel;
    QLabel* m_pDisplaySettingsLabel;

};


#endif // __GPPERFORMANCECOUNTERSDATAVIEW_H_
