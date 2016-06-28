//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdFunctionCallsStatisticsView.h
///
//==================================================================================

//------------------------------ gdFunctionCallsStatisticsView.h ------------------------------

#ifndef __GDFUNCTIONCALLSSTATISTICSVIEW
#define __GDFUNCTIONCALLSSTATISTICSVIEW

// Forward decelerations:
class afGlobalVariableChangedEvent;
class apExceptionEvent;

// Infra:
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionId.h>
#include <AMDTAPIClasses/Include/apStatistics.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afHTMLContent.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/views/gdStatisticsViewBase.h>

// ----------------------------------------------------------------------------------
// Class Name:           gdFunctionCallsStatisticsView: public gdStatisticsViewBase
// General Description:
//  A view that displays render frame's function calls statistical data.
//
// Author:               Avi Shapira
// Creation Date:        29/1/2006
// ----------------------------------------------------------------------------------
class GD_API gdFunctionCallsStatisticsView: public gdStatisticsViewBase
{
public:
    gdFunctionCallsStatisticsView(QWidget* pParent);
    virtual ~gdFunctionCallsStatisticsView();

    // Overrides gdStatisticsViewBase:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);
    virtual const wchar_t* eventObserverName() const { return L"FunctionCallsStatisticsView"; };

    // Update the statistics display with the updated statistics object:
    virtual bool updateFunctionCallsStatisticsList(const apStatistics& currentStatistics);

    /// Returns string to be used in a tool tip
    /// \param itemIndex description of item index
    virtual gtString getItemTooltip(int itemIndex);
    virtual const char* saveStatisticsDataFileName() override;

protected:
    virtual void initListCtrlColumns();
    virtual void initializeImageList();

    void onGlobalVariableChanged(const afGlobalVariableChangedEvent& event);

    bool addStatisticsItem(apMonitoredFunctionId functionId, const gtString& functionName, gtString& functionType, gtUInt64 amountOfCalls);
    void getStatisticsItemIcon(const gtString& functionName, afIconType& iconType, int& iconIndex) const;
    void addTotalItemToList();
    bool getFunctionTypeString(apMonitoredFunctionId functionId, gtString& functionTypeString);

protected:
    gtUInt64 _totalAmountOfFunctionCallsInFrame;
};


#endif  // __GDFUNCTIONCALLSSTATISTICSVIEW
