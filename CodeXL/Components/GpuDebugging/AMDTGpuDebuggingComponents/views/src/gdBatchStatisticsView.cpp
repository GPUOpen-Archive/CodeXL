//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdBatchStatisticsView.cpp
///
//==================================================================================

//------------------------------ gdBatchStatisticsView.cpp ------------------------------

// Qt
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/apRenderPrimitivesStatistics.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationComponents/Include/acIcons.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdBatchStatisticsView.h>
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdHTMLProperties.h>

// Amount of range shown in non detailed view:
#define GT_AMOUNT_OF_SLICES_IN_NON_DETAILED_VIEW 9


#define GD_STATISTICS_VIEWER_AMOUNT_OF_BATCH_COLORS 13
// The array represents the color indices in the following way:
// When the batch contain i items, and we want to know what is the color index for an
// item j, we take _pStaticBatchColorIndices[i-1][j]
const int gdBatchStatisticsView::_pStaticBatchColorIndices[] =
{
    0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    0, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    0,  6, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    0,  4,  8, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    0,  3,  6,  9, 12, -1, -1, -1, -1, -1, -1, -1, -1,
    0,  2,  5,  7, 10, 12, -1, -1, -1, -1, -1, -1, -1,
    0,  2,  4,  6,  8, 10, 12, -1, -1, -1, -1, -1, -1,
    0,  2,  3,  5,  7,  9, 10, 12, -1, -1, -1, -1, -1,
    0,  2,  3,  5,  6,  8,  9, 11, 12, -1, -1, -1, -1,
    0,  1,  2,  4,  5,  7,  8, 10, 11, 12, -1, -1, -1,
    0,  1,  2,  3,  5,  6,  7,  9, 10, 11, 12, -1, -1,
    0,  1,  2,  3,  4,  5,  7,  8,  9, 10, 11, 12, -1,
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12
};


// ---------------------------------------------------------------------------
// Name:        gdBatchStatisticsView::gdBatchStatisticsView
// Description: Constructor.
// Arguments:   parent - My parent window.
// Author:      Sigal Algranaty
// Date:        14/5/2009
// ---------------------------------------------------------------------------
gdBatchStatisticsView::gdBatchStatisticsView(QWidget* pParent)
    : gdStatisticsViewBase(pParent, GD_STATISTICS_VIEW_BATCH_INDEX, GD_STR_StatisticsViewerBatchStatisticsShortName),
      _showDetailedData(false), _showDetailedDataEnabled(false)
{
    // Call init function of base class:
    init();
}


// ---------------------------------------------------------------------------
// Name:        gdBatchStatisticsView::~gdBatchStatisticsView
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        14/5/2009
// ---------------------------------------------------------------------------
gdBatchStatisticsView::~gdBatchStatisticsView()
{
}


// ---------------------------------------------------------------------------
// Name:        gdBatchStatisticsView::initListCtrlColumns
// Description: Set the titles and widths of columns
// Return Val: void
// Author:      Sigal Algranaty
// Date:        20/5/2009
// ---------------------------------------------------------------------------
void gdBatchStatisticsView::initListCtrlColumns()
{
    // Add the columns to the list of columns:
    _listControlColumnTitles.push_back(GD_STR_BatchStatisticsViewerColumn1Title);
    _listControlColumnTitles.push_back(GD_STR_BatchStatisticsViewerColumn2Title);
    _listControlColumnTitles.push_back(GD_STR_BatchStatisticsViewerColumn3Title);
    _listControlColumnTitles.push_back(GD_STR_BatchStatisticsViewerColumn4Title);
    _listControlColumnTitles.push_back(GD_STR_BatchStatisticsViewerColumn5Title);

    // Add the width percentages:
    _listControlColumnWidths.push_back(0.30f);
    _listControlColumnWidths.push_back(0.20f);
    _listControlColumnWidths.push_back(0.15f);
    _listControlColumnWidths.push_back(0.20f);
    _listControlColumnWidths.push_back(0.15f);

    m_columnsPostfixes.push_back("");
    m_columnsPostfixes.push_back("");
    m_columnsPostfixes.push_back("%");
    m_columnsPostfixes.push_back("");
    m_columnsPostfixes.push_back("%");

    // Make sure thousand separators are removed:
    m_removeThousandSeparatorOnCopy = true;

    // Set the last column as the widest one:
    _widestColumnIndex = 4;
    _initialSortColumnIndex = 0;
    _sortInfo._sortOrder = Qt::AscendingOrder;
}
// ---------------------------------------------------------------------------
// Name:        gdBatchStatisticsView::updateFunctionCallsStatisticsList
// Description: Update the current statistics into the listCTRL
// Return Val: void
// Author:      Sigal Algranaty
// Date:        14/5/2009
// ---------------------------------------------------------------------------
bool gdBatchStatisticsView::updateFunctionCallsStatisticsList(const apStatistics& currentStatistics)
{
    (void)(currentStatistics);  // unused
    bool retVal = true;

    // Clear all counters and data:
    clearAllStatisticsItems();

    // Enable the list
    setEnabled(true);

    if (_activeContextId.isOpenGLContext())
    {
        // Get the render primitives statistics information:
        apRenderPrimitivesStatistics renderPrimitivesStatistics;
        bool rc = gaGetRenderPrimitivesStatistics(_activeContextId._contextId, renderPrimitivesStatistics);
        GT_IF_WITH_ASSERT(rc)
        {
            // Check if data by range is possible:
            int amountOfStatisticsItems = renderPrimitivesStatistics.getAmountOfItems();
            bool showDetailedDataEnabledChanged = _showDetailedDataEnabled;
            _showDetailedDataEnabled = (amountOfStatisticsItems > GT_AMOUNT_OF_SLICES_IN_NON_DETAILED_VIEW);
            showDetailedDataEnabledChanged = (showDetailedDataEnabledChanged != _showDetailedDataEnabled);

            if (showDetailedDataEnabledChanged)
            {
                _showDetailedData = !_showDetailedDataEnabled;
            }

            if (amountOfStatisticsItems > 0)
            {
                // Reset column width - since we change when there are no items:
                resetColumnsWidth();

                // Reset total counters:
                _totalAmountOfBatches = 0;
                _totalAmountOfVertices = 0;

                if (_showDetailedData)
                {
                    // Add the items by batch size:
                    addDetailedBatches(renderPrimitivesStatistics);
                }
                else
                {
                    // Add the items after collecting them by ranges:
                    addBatchesByRange(renderPrimitivesStatistics);
                }

                // Add the total items to the list:
                addTotalItemToList();

                // Sort the item in the list control:
                sortItems(0, _sortInfo._sortOrder);
            }
            else
            {
                addRow(AF_STR_NotAvailableA);
            }
        }
        else
        {
            addRow(AF_STR_NotAvailableA);
        }
    }
    else // !_activeContextId.isOpenGLContext()
    {
        addRow(GD_STR_BatchStatisticsViewerOnlyGLContexts);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdBatchStatisticsView::initializeImageList
// Description: Create and populate the image list for this item
// Author:      Sigal Algranaty
// Date:        14/5/2009
// ---------------------------------------------------------------------------
void gdBatchStatisticsView::initializeImageList()
{
    // Create the icons from the xpm files:
    QPixmap* pEmptyIcon = new QPixmap;
    acSetIconInPixmap(*pEmptyIcon, AC_ICON_EMPTY);

    QPixmap* pYellowWarningIcon = new QPixmap;
    acSetIconInPixmap(*pYellowWarningIcon, AC_ICON_WARNING_YELLOW);

    QPixmap* pInfoIcon = new QPixmap;
    acSetIconInPixmap(*pInfoIcon, AC_ICON_WARNING_INFO);

    // Add the icons to the list
    _listIconsVec.push_back(pEmptyIcon);            // 0
    _listIconsVec.push_back(pYellowWarningIcon);    // 1
    _listIconsVec.push_back(pInfoIcon);             // 2

}

// ---------------------------------------------------------------------------
// Name:        gdBatchStatisticsView::addTotalItemToList
// Description: Adds the "Total" item to the list control.
// Author:      Sigal Algranaty
// Date:        14/5/2009
// ---------------------------------------------------------------------------
void gdBatchStatisticsView::addTotalItemToList()
{
    // Get the amount of list items:
    int listSize = rowCount();

    if (listSize == 0)
    {
        // Add an item stating that there are not items:
        addEmptyListItem();
    }

    else
    {
        gtASCIIString numOfBatchesStr;
        gtASCIIString numOfVerticesStr;
        numOfVerticesStr.appendFormattedString("%llu", _totalAmountOfVertices);
        numOfBatchesStr.appendFormattedString("%llu", _totalAmountOfBatches);

        // Format the strings as number:
        numOfVerticesStr.addThousandSeperators();
        numOfBatchesStr.addThousandSeperators();

        QStringList list;
        list << AF_STR_TotalA;
        list << numOfBatchesStr.asCharArray();
        list << "100%";
        list << numOfVerticesStr.asCharArray();
        list << "100%";

        addRow(list, NULL);

        // Set the item appearance:
        setItemBold(rowCount() - 1);
    }
}


// ---------------------------------------------------------------------------
// Name:        gdBatchStatisticsView::addDetailedBatches
// Description: Adds the batches to the list view by details. Adds each batch
//              by itself
// Arguments: const apRenderPrimitivesStatistics& renderPrimitivesStatistics
// Return Val: void
// Author:      Sigal Algranaty
// Date:        14/5/2009
// ---------------------------------------------------------------------------
void gdBatchStatisticsView::addDetailedBatches(const apRenderPrimitivesStatistics& renderPrimitivesStatistics)
{
    // Get the statistics map:
    const gtMap<int, gtUInt64>& renderPrimitivesStatsMap = renderPrimitivesStatistics.getStatisticsMap();
    gtMap<int, gtUInt64>::const_iterator iter = renderPrimitivesStatsMap.begin();
    gtMap<int, gtUInt64>::const_iterator iterEnd = renderPrimitivesStatsMap.end();

    // Loop through the statistics items, and calculate total vertices and batches counters:
    for (; iter != iterEnd; iter++)
    {
        // Get the amount of calls for this number of vertices:
        int amountOfVertices = (*iter).first;
        gtUInt64 amountOfBatches = (*iter).second;

        if ((amountOfBatches > 0) && (amountOfVertices > 0))
        {
            _totalAmountOfBatches += amountOfBatches;
            _totalAmountOfVertices += amountOfBatches * amountOfVertices;
        }
    }

    // Reset the iterator:
    iter = renderPrimitivesStatsMap.begin();

    // Loop through the statistics items:
    for (; iter != iterEnd; iter++)
    {
        // Get the amount of calls for this number of vertices:
        int amountOfVertices = (*iter).first;
        gtUInt64 amountOfBatches = (*iter).second;

        // If this batch size is not 0 (and we are not using functions such as glEvalMesh which we
        // don't know the number of vertices for):
        if ((amountOfBatches > 0) && (amountOfVertices > 0))
        {
            // Calculate total amount of vertices:
            gtUInt64 totalAmountOfVertices = amountOfVertices * amountOfBatches;

            // Add this batch to the list control:
            addBatchToListControl(amountOfVertices, 0, amountOfBatches, totalAmountOfVertices, false);
        }
    }
}



// ---------------------------------------------------------------------------
// Name:        gdBatchStatisticsView::addBatchesByRange
// Description: Adds the batches to the list view by range.
//              The function separates the batches to ranges, and add them to the
//              list control by range
// Arguments: const apRenderPrimitivesStatistics& renderPrimitivesStatistics
// Return Val: void
// Author:      Sigal Algranaty
// Date:        14/5/2009
// ---------------------------------------------------------------------------
void gdBatchStatisticsView::addBatchesByRange(const apRenderPrimitivesStatistics& renderPrimitivesStatistics)
{
    // Get the statistics map:
    const gtMap<int, gtUInt64>& renderPrimitivesStatsMap = renderPrimitivesStatistics.getStatisticsMap();
    gtMap<int, gtUInt64>::const_iterator iterRanges = renderPrimitivesStatsMap.begin();
    gtMap<int, gtUInt64>::const_iterator iterEndRanges = renderPrimitivesStatsMap.end();
    gtList<pair<gtUInt64, int> > listOfStatistics;

    // Loop through the statistics items, and calculate total vertices and batches counters:
    for (; iterRanges != iterEndRanges; iterRanges++)
    {
        // Get the amount of calls for this number of vertices:
        int amountOfVertices = (*iterRanges).first;
        gtUInt64 amountOfBatches = (*iterRanges).second;

        if (amountOfBatches > 0)
        {
            _totalAmountOfBatches += amountOfBatches;
            _totalAmountOfVertices += amountOfBatches * amountOfVertices;

            // Insert this statistics to the list of statistics:
            pair<gtUInt64, int> currentStats;
            currentStats.first = amountOfBatches * amountOfVertices;
            currentStats.second = amountOfVertices;
            listOfStatistics.insert(listOfStatistics.begin(), currentStats);
        }
    }

    // Create a vector describes the order of the statistics added to the list control:
    gtVector<pair<int, int> > rangesVector;
    bool rc = createRangesVector(listOfStatistics, renderPrimitivesStatistics, rangesVector);
    GT_IF_WITH_ASSERT(rc)
    {
        // Loop through the ranges vector, and add each of the ranges:
        gtVector<pair<int, int> >::const_iterator iter = rangesVector.begin();
        gtVector<pair<int, int> >::const_iterator iterEnd = rangesVector.end();

        for (; iter != iterEnd; iter++)
        {
            // Get the min and max values for this range:
            int minVerticesAmount = (*iter).first;
            int maxVerticesAmount = (*iter).second;

            // Calculate amount of batches for this range:
            gtUInt64 amountOfBatches = 0;
            gtUInt64 totalAmountOfBatches = 0;

            for (int i = minVerticesAmount; i <= maxVerticesAmount; i++)
            {
                // Get batches amount for this vertices amount:
                gtUInt64 currentBatchAmount = renderPrimitivesStatistics.getBatchesAmount(i);
                totalAmountOfBatches += currentBatchAmount * i;
                amountOfBatches += currentBatchAmount;

            }

            // Add this batch range to the list control:
            addBatchToListControl(minVerticesAmount, maxVerticesAmount, amountOfBatches, totalAmountOfBatches, true);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdBatchStatisticsView::addBatchToListControl
// Description: Add a batch to the list control
// Arguments: int minVertices - the minimum amount of vertices for this range
//            int maxVertices - the maximum amount of vertices for this range
//            gtUInt64 amountOfBatches - amount of batches for this range
//            gtUInt64 amountOfVerticesForRange - total amount of vertices drawn by
//            this range
//            bool byRange - should show by range (if not, ignore maxVertices)
// Return Val: void
// Author:      Sigal Algranaty
// Date:        18/5/2009
// ---------------------------------------------------------------------------
void gdBatchStatisticsView::addBatchToListControl(int minVertices, int maxVertices, gtUInt64 amountOfBatches, gtUInt64 amountOfVerticesForRange, bool byRange)
{
    gtASCIIString rangeStr;
    gtASCIIString numOfBatchesStr;
    gtASCIIString numOfVerticesStr;
    gtASCIIString percentageOfBatchesStr;
    gtASCIIString percentageOfVerticesStr;

    // Calculate percentage of batches and vertices:
    float batchesPercentage = (float)amountOfBatches / (float)_totalAmountOfBatches * 100;
    float verticesPercentage = (float)amountOfVerticesForRange / (float)_totalAmountOfVertices * 100;

    // Format the range strings:
    gtASCIIString minRangeAsStr, maxRangeAsStr;
    minRangeAsStr.appendFormattedString("%d", minVertices);
    maxRangeAsStr.appendFormattedString("%d", maxVertices);
    minRangeAsStr.addThousandSeperators();
    maxRangeAsStr.addThousandSeperators();

    // Create the strings for the list control:
    if (byRange && (minVertices != maxVertices))
    {
        rangeStr.appendFormattedString("%s - %s", minRangeAsStr.asCharArray(), maxRangeAsStr.asCharArray());
    }
    else
    {
        rangeStr.appendFormattedString("%s", minRangeAsStr.asCharArray());
    }

    // Build the strings for the item:
    numOfVerticesStr.appendFormattedString("%llu", amountOfVerticesForRange);
    numOfBatchesStr.appendFormattedString("%llu", amountOfBatches);
    percentageOfVerticesStr.appendFormattedString("%.2f", verticesPercentage);
    percentageOfBatchesStr.appendFormattedString("%.2f", batchesPercentage);

    // Format the strings as number:
    numOfBatchesStr.addThousandSeperators();
    numOfVerticesStr.addThousandSeperators();

    // Calculate the vertices to batch rate:
    float verticesToBatchesRate = verticesPercentage / batchesPercentage;
    afIconType iconType = AF_ICON_NONE;
    int iconIndex = 0;

    // Check the icon type according to the optimized rate:
    if (verticesToBatchesRate > GD_HTML_PROPERTIES_VERTICES_TO_BATCHES_OPTIMIZED_RATE)
    {
        // Add information icon:
        iconType = AF_ICON_INFO;
        iconIndex = 0;
    }
    else
    {
        // Add warning icon:
        iconType = AF_ICON_WARNING1;
        iconIndex = 2;
    }

    QStringList list;
    list << rangeStr.asCharArray();
    list << numOfBatchesStr.asCharArray();
    list << percentageOfBatchesStr.asCharArray();
    list << numOfVerticesStr.asCharArray();
    list << percentageOfVerticesStr.asCharArray();

    // Get the icon:
    QPixmap* pPixmap = icon(iconIndex);

    // Add the item data:
    gdStatisticsViewItemData* pItemData = new gdStatisticsViewItemData;


    pItemData->_numOfBatches = amountOfBatches;
    pItemData->_numOfVertices = amountOfVerticesForRange;
    pItemData->_percentageOfBatches = batchesPercentage;
    pItemData->_percentageOfVertices = verticesPercentage;
    pItemData->_minRange = minVertices;
    pItemData->_maxRange = maxVertices;
    pItemData->_iconType = iconType;

    // Add the item:
    addRow(list, pItemData, false, Qt::Unchecked, pPixmap);
}


// ---------------------------------------------------------------------------
// Name:        gdBatchStatisticsView::createRangesVector
// Description:
// Arguments: gtList<pair<int, gtUInt64> > sortedListOfStats
//            const apRenderPrimitivesStatistics& renderStatistics - the statistics item
//            gtVector<pair<int
//            int> >& rangesVector
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        18/5/2009
// ---------------------------------------------------------------------------
bool gdBatchStatisticsView::createRangesVector(gtList<pair<gtUInt64, int> >& listOfStatistics, const apRenderPrimitivesStatistics& renderStatistics, gtVector<pair<int, int> >& rangesVector)
{
    bool retVal = true;
    // Check how many vertices are in each range:
    gtUInt64 amountOfVerticesPerSlice = _totalAmountOfVertices / GT_AMOUNT_OF_SLICES_IN_NON_DETAILED_VIEW;

    // Sort the list of statistics (we sort the list by batches):
    listOfStatistics.sort();

    // Create a list of amounts of vertices, that were already collected to ranges:
    gtList<int> visitedAmountsOfVertices;

    // Save the amount of vertices left to collect yet:
    gtUInt64 verticesLeftToCollect = _totalAmountOfVertices;

    // Loop the sorted statistics (in descending order):
    gtList<pair<gtUInt64, int> >::const_reverse_iterator iter = listOfStatistics.rbegin();
    gtList<pair<gtUInt64, int> >::const_reverse_iterator iterEnd = listOfStatistics.rend();

    for (; iter != iterEnd; iter++)
    {
        // Get the current item:
        int currentNumOfVertices = (*iter).second;

        gtUInt64 currentSliceVerticesAmount = 0;
        int currentMinRange = 0;
        int currentMaxRange = 0;

        // Check if item was already visited:
        gtList<int>::iterator findIter = gtFind(visitedAmountsOfVertices.begin(), visitedAmountsOfVertices.end(), currentNumOfVertices);

        if (findIter == visitedAmountsOfVertices.end())
        {
            bool rc = addStatisticsItemToSlice(currentSliceVerticesAmount, amountOfVerticesPerSlice, currentNumOfVertices, renderStatistics, visitedAmountsOfVertices, currentMinRange, currentMaxRange);
            GT_IF_WITH_ASSERT(rc)
            {
                // Add the current range to the vector of ranges:
                rangesVector.push_back(pair<int, int>(currentMinRange, currentMaxRange));

                // Reduce the amount of vertices added by this range:
                verticesLeftToCollect -= currentSliceVerticesAmount;

                // Check how many slices are left to add:
                int slicesAmount = (GT_AMOUNT_OF_SLICES_IN_NON_DETAILED_VIEW - rangesVector.size());

                if (slicesAmount > 0)
                {
                    // The last slice contain the left vertices:
                    if (slicesAmount == 1)
                    {
                        amountOfVerticesPerSlice = verticesLeftToCollect;
                    }
                    else
                    {
                        // Recalculate the amount of vertices per slice:
                        amountOfVerticesPerSlice = verticesLeftToCollect / slicesAmount;
                    }
                }
            }

            retVal = retVal && rc;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdBatchStatisticsView::addStatisticsItemToSlice
// Description: Recursive function that adds a statistics item to the current range
// Arguments: gtUInt64& currentSliceVerticesAmount
//            const gtUInt64& maxVerticesPerSlice
//            int verticesPerCall
//            const apRenderPrimitivesStatistics& renderStatistics
//            gtList<int>& visitedAmountsOfVertices
//            int& minRange
//            int& maxRange
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        19/5/2009
// ---------------------------------------------------------------------------
bool gdBatchStatisticsView::addStatisticsItemToSlice(gtUInt64& currentSliceVerticesAmount, const gtUInt64& maxVerticesPerSlice,
                                                     int verticesPerCall, const apRenderPrimitivesStatistics& renderStatistics,
                                                     gtList<int>& visitedAmountsOfVertices, int& minRange, int& maxRange)
{
    bool retVal = false;

    // Get the batches amount for this amount of vertices:
    gtUInt64 batchesAmount = renderStatistics.getBatchesAmount(verticesPerCall);

    // Calculate the vertices drawn for this statistics item:
    gtUInt64 verticesAmount  = verticesPerCall * batchesAmount;

    // Check if item overflow the slice size:
    bool itemOverFlow = ((currentSliceVerticesAmount + verticesAmount) <= maxVerticesPerSlice);
    bool itemIsFirstInRange = ((maxRange == 0) && (minRange == 0));

    // Check if item can be added to slice
    // If the item is the first in the range, then ofcourse we add it:
    if (itemOverFlow || itemIsFirstInRange)
    {
        // Update minimum and maximum range if needed:
        if (maxRange < verticesPerCall)
        {
            maxRange = verticesPerCall;
        }

        if ((minRange > verticesPerCall) || (minRange == 0))
        {
            minRange = verticesPerCall;
        }

        // Mark that the item was visited:
        visitedAmountsOfVertices.insert(visitedAmountsOfVertices.end(), verticesPerCall);

        // Update current slice vertices amount:
        currentSliceVerticesAmount += verticesAmount;

        // Try to add neighbors only for vertices amount different than 1:
        if (verticesPerCall != 1)
        {
            // Go left and find the next right neighbor:
            for (int j = verticesPerCall - 1; j > 1; j--)
            {
                // Get the current neighbor value:
                gtUInt64 neighborBatchesAmount = renderStatistics.getBatchesAmount(j);

                // If this is a 'real' neighbor:
                if (neighborBatchesAmount > 0)
                {
                    bool neighborIsAdded = false;
                    // Check if neighbor is visited:
                    gtList<int>::iterator findIter = gtFind(visitedAmountsOfVertices.begin(), visitedAmountsOfVertices.end(), j);

                    if (findIter == visitedAmountsOfVertices.end())
                    {
                        // The neighbor was not visited, add it:
                        neighborIsAdded = addStatisticsItemToSlice(currentSliceVerticesAmount, maxVerticesPerSlice, j, renderStatistics, visitedAmountsOfVertices, minRange, maxRange);
                    }

                    // If neighbor cannot be added, break the loop:
                    if (!neighborIsAdded)
                    {
                        break;
                    }
                }
            }

            // Get the last statistics item vertices amount:
            int maxAmountOfVertices = renderStatistics.getMaxVerticesAmount();

            // Go right and find the next right neighbor:
            for (int j = verticesPerCall + 1; j <= maxAmountOfVertices; j++)
            {
                // Get the current neighbor value:
                gtUInt64 neighborBatchesAmount = renderStatistics.getBatchesAmount(j);

                // If this is a 'real' neighbor:
                if (neighborBatchesAmount > 0)
                {
                    bool neighborIsAdded = false;
                    // Check if neighbor is visited:
                    // Check if neighbor is visited:
                    gtList<int>::iterator findIter = gtFind(visitedAmountsOfVertices.begin(), visitedAmountsOfVertices.end(), j);

                    if (findIter == visitedAmountsOfVertices.end())
                    {
                        // The neighbor was not visited, add it:
                        neighborIsAdded = addStatisticsItemToSlice(currentSliceVerticesAmount, maxVerticesPerSlice, j, renderStatistics, visitedAmountsOfVertices, minRange, maxRange);
                    }

                    // If neighbor cannot be added, break the loop:
                    if (!neighborIsAdded)
                    {
                        break;
                    }
                }
            }
        }

        // This item was added successfully:
        retVal = true;
    }
    else
    {
        // Item cannot be added, create a new range for it:
        retVal = false;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdBatchStatisticsView::getItemInformation
// Description: Retrieves an information for an item
// Arguments: int itemIndex
//            int &minAmountOfVertices
//            int &maxAmountOfVertices
//            float& verticesToBatchRate
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/5/2009
// ---------------------------------------------------------------------------
bool gdBatchStatisticsView::getItemInformation(int itemIndex, int& minAmountOfVertices, int& maxAmountOfVertices, float& percentageOfVertices, float& percentageOfBatches, afIconType& iconType)
{
    bool retVal = false;

    // If it is a 'real' item (not the total item):
    if (itemIndex < rowCount() - 1)
    {
        // Get the item data for this item:
        gdStatisticsViewItemData* pViewItemData = gdStatisticsViewBase::getItemData(itemIndex);
        GT_IF_WITH_ASSERT(pViewItemData != NULL)
        {
            // Get the item information:
            minAmountOfVertices = pViewItemData->_minRange;
            maxAmountOfVertices = pViewItemData->_maxRange;
            percentageOfBatches = pViewItemData->_percentageOfBatches;
            percentageOfVertices = pViewItemData->_percentageOfVertices;
            iconType = pViewItemData->_iconType;
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdBatchStatisticsView::getItemChartColor
// Description: Overrides base class - return an item chart color
// Arguments: int itemIndex
//            int amountOfItemsCuurentlyInChart
//            bool useSavedColors
//            unsigned long& color
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        26/5/2009
// ---------------------------------------------------------------------------
bool gdBatchStatisticsView::getItemChartColor(int itemIndex, int& amountOfCurrentItemsForColorSelection, bool useSavedColors, unsigned long& color)
{
    (void)(amountOfCurrentItemsForColorSelection);  // unused
    bool retVal = false;

    // Check if this item color was already set:
    if (useSavedColors)
    {
        long itemColor = gdStatisticsViewBase::getItemChartColor(itemIndex);

        if (itemColor >= 0)
        {
            color = (unsigned long)itemColor;
            retVal = true;
        }
    }

    if (!retVal)
    {
        // Get amount of colors:
        int amountOfColors = amountOfChartColors();

        // Batch view - when data is collected in ranges, get the item in array:
        bool showDetailedData = shouldShowDetailedData();

        int colorIndex = -1;

        // Get amount of items:
        int amountOfItems = rowCount();

        int itemIndexForColor = itemIndex;

        if ((showDetailedData) && (amountOfItems > amountOfColors))
        {
            // Check amount of item for each color:
            int amountOfItemsPerColor = (int)ceilf((float)amountOfItems / (float)amountOfColors);
            GT_IF_WITH_ASSERT(amountOfItemsPerColor > 0)
            {
                itemIndexForColor = (int)floorf((float)itemIndex / (float)amountOfItemsPerColor);
                int amountOfItemsForColor = GD_STATISTICS_VIEWER_AMOUNT_OF_BATCH_COLORS - 1;
                int indexInArray = amountOfItemsForColor * GD_STATISTICS_VIEWER_AMOUNT_OF_BATCH_COLORS + itemIndexForColor;
                colorIndex = _pStaticBatchColorIndices[indexInArray];
                color = _chartColors[colorIndex];
                retVal = true;
            }
        }
        else
        {
            // Get the row to get the items from:
            int rowInArray = amountOfItems - 1;

            if (rowInArray >= GD_STATISTICS_VIEWER_AMOUNT_OF_BATCH_COLORS)
            {
                rowInArray = GD_STATISTICS_VIEWER_AMOUNT_OF_BATCH_COLORS - 1;
            }

            // Get the index within the color - item array:
            int indexInArray = rowInArray * GD_STATISTICS_VIEWER_AMOUNT_OF_BATCH_COLORS + itemIndexForColor;
            colorIndex = _pStaticBatchColorIndices[indexInArray];
            color = _chartColors[colorIndex];
            retVal = true;
        }

        // Save the item chart color:
        setItemChartColor(itemIndex, color);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdBatchStatisticsView::getItemTooltip
// Description: Get an item tooltip
// Arguments: int itemIndex
// Return Val: gtString
// Author:      Sigal Algranaty
// Date:        24/6/2009
// ---------------------------------------------------------------------------
gtString gdBatchStatisticsView::getItemTooltip(int itemIndex)
{
    gtString retVal;

    // Get the item data:
    gdStatisticsViewItemData* pItemData = gdStatisticsViewBase::getItemData(itemIndex);

    if (pItemData != NULL)
    {
        // Convert the min, max ranges to formatted string:
        gtString minRangeAsString;
        gtString maxRangeAsString;
        minRangeAsString.appendFormattedString(L"%d", pItemData->_minRange);
        maxRangeAsString.appendFormattedString(L"%d", pItemData->_maxRange);
        minRangeAsString.addThousandSeperators();
        maxRangeAsString.addThousandSeperators();

        // Create the batch statistics item title:
        if ((pItemData->_maxRange != 0) && (pItemData->_minRange != pItemData->_maxRange))
        {
            retVal.appendFormattedString(GD_STR_BatchStatisticsViewerItemTooltip1, minRangeAsString.asCharArray(), maxRangeAsString.asCharArray(), pItemData->_percentageOfBatches);
        }
        else
        {
            retVal.appendFormattedString(GD_STR_BatchStatisticsViewerItemTooltip2, minRangeAsString.asCharArray(), pItemData->_percentageOfBatches);
        }
    }

    return retVal;

}

const char* gdBatchStatisticsView::saveStatisticsDataFileName()
{
    return GD_STR_saveVertexBatchStatisticsFileName;
}
