//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdStateChangeStatisticsView.h
///
//==================================================================================

//------------------------------ gdStateChangeStatisticsView.h ------------------------------

#ifndef __gdStateChangeStatisticsView
#define __gdStateChangeStatisticsView

struct apFunctionCallStatistics;
struct apEnumeratorUsageStatistics;

// Infra:

// Local:
#include <AMDTGpuDebuggingComponents/Include/views/gdStatisticsViewBase.h>


// ----------------------------------------------------------------------------------
// Class Name:           GD_API gdStateChangeStatisticsView: public gdStatisticsViewBase
// General Description: This class represents a detailed statistics viewer. This viewer is added to the Statistics
//                      viewer as one of the notebook pages.
// Author:               Sigal Algranaty
// Creation Date:        20/7/2008
// ----------------------------------------------------------------------------------
class GD_API gdStateChangeStatisticsView: public gdStatisticsViewBase
{
public:
    gdStateChangeStatisticsView(QWidget* pParent);
    virtual ~gdStateChangeStatisticsView();

    // Updates the view with the current statistics:
    virtual bool updateFunctionCallsStatisticsList(const apStatistics& currentStatistics);

    // Return an item properties:
    bool getItemProperties(int itemIndex, gtUInt64& effectiveCallsAmount, gtUInt64& redundantCallsAmount, long& effectiveColor, long& redundantColor);

    // Overrides apIEventsObserver:
    virtual const wchar_t* eventObserverName() const { return L"StateChangeStatisticsView"; };

public slots:

    virtual void onSaveStatisticsData();

protected:

    virtual void initListCtrlColumns();
    virtual void initializeImageList();

    void addTotalItemToList();

    bool addFunctionToList(const apFunctionCallStatistics& functionStatistics);
    bool addFunctionToList(const apFunctionCallStatistics& functionStatistics, const apEnumeratorUsageStatistics& functionEnumStatistics);
    void getFunctionIcon(float percentageRedundantCalls, int& iconIndex, afIconType& iconType);

protected:

    // Counter for total amount of function calls:
    gtUInt64 _totalAmountOfStateChangeFunctionCalls;
    gtUInt64 _totalAmountOfEffectiveCalls;
    gtUInt64 _totalAmountOfRedundantCalls;

};


#endif  // __gdStateChangeStatisticsView
