//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdDeprecationStatisticsView.h
///
//==================================================================================

//------------------------------ gdDeprecationStatisticsView.h ------------------------------

#ifndef __GDDEPRECATEDFUNCTIONCALLSSTATISTICSVIEW
#define __GDDEPRECATEDFUNCTIONCALLSSTATISTICSVIEW

// Infra:
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>
#include <AMDTAPIClasses/Include/apFunctionDeprecation.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionId.h>
#include <AMDTAPIClasses/Include/apAPIVersion.h>
#include <AMDTAPIClasses/Include/apStatistics.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/views/gdStatisticsViewBase.h>


// ----------------------------------------------------------------------------------
// Class Name:           GD_API gdDeprecationStatisticsView: public gdStatisticsViewBase
// General Description: This class represents a deprecation function calls statistics
//                      viewer. This viewer is added to the Statistics viewer as one of the notebook pages.
// Author:      Sigal Algranaty
// Date:        25/2/2009
// ----------------------------------------------------------------------------------
class GD_API gdDeprecationStatisticsView: public gdStatisticsViewBase
{
public:
    gdDeprecationStatisticsView(QWidget* pParent);
    virtual ~gdDeprecationStatisticsView();

    // Updates the view with the current statistics:
    virtual bool updateFunctionCallsStatisticsList(const apStatistics& currentStatistics);

    // Overrides apIEventsObserver:
    virtual const wchar_t* eventObserverName() const { return L"DeprecationStatisticsView"; };

    /// Returns string to be used in a tool tip
    /// \param itemIndex description of item index
    virtual gtString getItemTooltip(int itemIndex);
    virtual const char* saveStatisticsDataFileName() override;

    bool getItemDeprecationVersions(int itemIndex, apAPIVersion& deprecatedAtVersion, apAPIVersion& removedAtVersion);
    bool getItemDeprecationStatus(int itemIndex, apFunctionDeprecationStatus& functionDeprecationStatus);
    bool getItemFunctionId(int itemIndex, int& functionID);

protected:
    virtual void initListCtrlColumns();

    // initialize the images list
    virtual void initializeImageList();

    void addTotalItemToList();
    void addMoreItemToList();
    void addGLSLDeprecationItems();

    bool addFunctionToList(const apFunctionCallStatistics& functionStatistics, apFunctionDeprecationStatus deprecationStatus);

protected:
    // Counter for total amount of function calls:
    gtUInt64 _totalAmountOfDeprecatedFunctionCalls;

    // Holds the total amount of function call in frame:
    gtUInt64 _totalAmountOfFunctionCallsInFrame;
};


#endif  // __GDDEPRECATEDFUNCTIONCALLSSTATISTICSVIEW
