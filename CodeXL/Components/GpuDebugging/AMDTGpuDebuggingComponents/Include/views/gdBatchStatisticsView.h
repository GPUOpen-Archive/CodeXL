//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdBatchStatisticsView.h
///
//==================================================================================

//------------------------------ gdBatchStatisticsView.h ------------------------------

#ifndef __GDBATCHSTATISTICSVIEW
#define __GDBATCHSTATISTICSVIEW

// Infra:
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>
#include <AMDTAPIClasses/Include/apStatistics.h>
#include <AMDTAPIClasses/Include/apExecutionMode.h>
#include <AMDTAPIClasses/Include/apRenderPrimitivesStatistics.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/views/gdStatisticsViewBase.h>


// ----------------------------------------------------------------------------------
// Class Name:           GD_API gdBatchStatisticsView: public gdStatisticsViewBase
// General Description: This class represents a batch statistics viewer. This viewer is added to the Statistics
//                      viewer as one of the notebook pages.
// Author:               Sigal Algranaty
// Creation Date:        14/5/2009
// ----------------------------------------------------------------------------------
class GD_API gdBatchStatisticsView: public gdStatisticsViewBase
{
public:
    gdBatchStatisticsView(QWidget* pParent);
    virtual ~gdBatchStatisticsView();

    // Updates the view with the current statistics:
    virtual bool updateFunctionCallsStatisticsList(const apStatistics& currentStatistics);

    // Collect data in ranges flag:
    void setShowDetailedData(bool collect) {_showDetailedData = collect;};
    bool showDetailedDataEnabled() {return _showDetailedDataEnabled;};
    bool shouldShowDetailedData() {return _showDetailedData;};

    // Retrieve data for an item:
    bool getItemInformation(int itemIndex, int& minAmountOfVertices, int& maxAmountOfVertices, float& percentageOfVertices, float& percentageOfBatches, afIconType& iconType);

    // Item chart color:
    virtual bool getItemChartColor(int itemIndex, int& amountOfCurrentItemsForColorSelection, bool useSavedColors, unsigned long& color);

    virtual gtString getItemTooltip(int itemIndex);

    virtual const char* saveStatisticsDataFileName() override;

    virtual const wchar_t* eventObserverName() const { return L"BatchStatisticsView"; };

protected:
    virtual void initListCtrlColumns();
    virtual void initializeImageList();

    void addTotalItemToList();

    // Add the items by batch size:
    void addDetailedBatches(const apRenderPrimitivesStatistics& renderPrimitivesStatistics);
    void addBatchesByRange(const apRenderPrimitivesStatistics& renderPrimitivesStatistics);
    void addBatchToListControl(int minVertices, int maxVertices, gtUInt64 amountOfBatches, gtUInt64 totalAmountOfVertices, bool byRange);

    typedef std::pair<gtUInt64, int> gdBatchCountAndSize;
    typedef std::pair<int, int> gdBatchSizeRange;
    bool createRangesVector(gtList<gdBatchCountAndSize>& listOfStatistics, const apRenderPrimitivesStatistics& renderStatistics, gtVector<gdBatchSizeRange>& rangesVector);
    bool addStatisticsItemToSlice(gtUInt64& currentSliceVerticesAmount, const gtUInt64& verticesPerSlice, int verticesPerCall, const apRenderPrimitivesStatistics& renderStatistics, gtList<int>& visitedAmountsOfVertices, int& minRange, int& maxRange);

private:
    // Should we show the detailed data, or collect the batched by range:
    bool _showDetailedData;
    bool _showDetailedDataEnabled;

    // Contain total amount of vertices and batches:
    gtUInt64 _totalAmountOfBatches;
    gtUInt64 _totalAmountOfVertices;

    // Map from batch item index to color index:
    static const int _pStaticBatchColorIndices[];
};


#endif  // __GDBATCHSTATISTICSVIEW


