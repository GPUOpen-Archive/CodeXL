//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdTotalStatisticsView.h
///
//==================================================================================

//------------------------------ gdTotalStatisticsView.h ------------------------------

#ifndef __GDTOTALSTATISTICSVIEW
#define __GDTOTALSTATISTICSVIEW

// Infra:
#include <AMDTAPIClasses/Include/apFunctionType.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>
#include <AMDTAPIClasses/Include/apStatistics.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdStatisticsViewBase.h>

// ----------------------------------------------------------------------------------
// Class Name:           GD_API gdTotalStatisticsView: public gdStatisticsViewBase
// General Description: This class represents a total statistics viewer. This viewer is added to the Statistics
//                      viewer as one of the notebook pages.
// Author:               Sigal Algranaty
// Creation Date:        20/7/2008
// ----------------------------------------------------------------------------------
class GD_API gdTotalStatisticsView: public gdStatisticsViewBase
{
public:

    gdTotalStatisticsView(QWidget* pParent);
    virtual ~gdTotalStatisticsView();

    // Clears the statistics:
    void clearAllStatisticsItems();

    // Updates the view with the current statistics:
    virtual bool updateFunctionCallsStatisticsList(const apStatistics& currentStatistics);

    // Get functions for item content:
    bool getItemNumOfCalls(int itemIndex, gtUInt64& numberOfCalls, bool& isItemAvailable);
    bool getItemFunctionType(int itemIndex, gdFuncCallsViewTypes& functionType);


    // Override for base class:
    virtual bool getItemChartColor(int itemIndex, int& amountOfCurrentItemsForColorSelection, bool useSavedColors, unsigned long& color);

    virtual gtString getItemTooltip(int itemIndex);

    // Overrides apIEventsObserver:
    virtual const wchar_t* eventObserverName() const { return L"TotalStatisticsView"; };

protected:

    virtual void initListCtrlColumns();
    virtual void setListHeadingToolTips();
    virtual void initializeImageList();


private:

    // Count the function by it's type:
    bool increaseCounterByType(const apFunctionCallStatistics& functionCallStatisticsData);

    // Add function type counters to list:
    bool addFunctionTypeCountersToList();

    // Initialized the function type to function calls amount mapping:
    void initFunctionTypeMapping();

    // Checks if a function type should be shown currently:
    void shouldShowFunctionType(gdFuncCallsViewTypes functionType, bool& shouldAddFunctionTypeItem, bool& shouldAddUnavailableItem, gtString& unavailableMessage);

private:

    bool addStatisticsItem(gtString functionTypeName, gtUInt64 amountOfCalls, gtUInt64 averageAmountOfCallsPerFrameForType, int icon, gdFuncCallsViewTypes functionType);
    bool addUnavailableItem(gtString functionTypeName, gdFuncCallsViewTypes functionType);
    void addTotalItemToList();

private:

    // Strings for function type;
    gtString* _pFunctionTypeStrings;

    // Contain the amount of calls for function types (the indexing is according to _functionTypeToFunctionAmountIndex):
    gtUInt64* _pFunctionTypeAmountsByType;

    // Contain the average amount of calls (per frame) for function types (the indexing is according to _functionTypeToFunctionAmountIndex):
    gtUInt64* _pFunctionTypeAverageAmountsByType;

    gtUInt64 _totalAmountOfFunctionCalls;
    gtUInt64 _totalAmountOfFunctionCallsInFullFrames;

    // Amount of full frames count:
    gtUInt64 _amountOfFullFrames;

};


#endif  // __gdTotalStatisticsView
