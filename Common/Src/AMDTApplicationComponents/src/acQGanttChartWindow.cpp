//------------------------------ acQGanttChartWindow.cpp ------------------------------

// Ignore compiler warnings:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Qt:
#include <QtGui>

// wxWindows pre compiled header:
#include <AMDTApplicationComponents/Include/acWxWidgetsIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>

// Local:
#include <AMDTApplicationComponents/Include/acQGanttChartWindow.h>
#include <inc/acOpenGLChartWindowResources.h>

// ---------------------------------------------------------------------------
// Name:        acQGanttChartWindow::acQGanttChartWindow
// Description: Constructor
// Arguments:   QWidget* pParent
//              QColor bgColor
// Author:      Sigal Algranaty
// Date:        15/12/2011
// ---------------------------------------------------------------------------
acQGanttChartWindow::acQGanttChartWindow(QWidget* pParent, QColor bgColor)
    : qcTimeline(pParent)
{
    // Set the background color:
    _bgColor = bgColor;

    // Connect the item clicked signal to its slot:
    bool rc = connect(this, SIGNAL(itemClicked(qcTimelineItem*)), this, SLOT(itemClicked(qcTimelineItem*)));
    GT_ASSERT(rc);
}

// ---------------------------------------------------------------------------
// Name:        acQGanttChartWindow::~acQGanttChartWindow
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        15/12/2011
// ---------------------------------------------------------------------------
acQGanttChartWindow::~acQGanttChartWindow()
{
}

// ---------------------------------------------------------------------------
// Name:        acQGanttChartWindow::clearAllData
// Description: Clears all the data from the vectors
// Author:      Sigal Algranaty
// Date:        15/12/2011
// ---------------------------------------------------------------------------
void acQGanttChartWindow::clearAllData()
{
    _backgroundSectionsData.clear();
    _barsDataByBranch.deleteElementsAndClear();

    updateGanttChart();
}

// ---------------------------------------------------------------------------
// Name:        acQGanttChartWindow::updateGanttChart
// Description: Update the gantt chart OpenGL data after loading series and other
//              data
// Author:      Sigal Algranaty
// Date:        15/12/2011
// ---------------------------------------------------------------------------
void acQGanttChartWindow::updateGanttChart()
{
    // Reset the chart:
    reset();

    // Collect the max and min values:
    gtUInt64 minValue = 0, maxValue = 0;

    // Add the required branches for the time line:
    for (int branchIndex = 0 ; branchIndex < (int)_backgroundSectionsData.size(); branchIndex++)
    {
        // Get the current branch data:
        const acQGanttChartBackgroundSectionData& ganttChartData = _backgroundSectionsData[branchIndex];

        // Create new branch:
        qcTimelineBranch* pTimelineBranch = new qcTimelineBranch;
        GT_ASSERT_ALLOCATION(pTimelineBranch);

        // Set the branch text:
        pTimelineBranch->setText(ganttChartData._label.asASCIICharArray());

        // Add the branch:
        addBranch(pTimelineBranch);

        // Get the vector of the chart bar data for this branch:
        gtVector<acQGanttChartBarData>* pBranchVector = _barsDataByBranch[branchIndex];
        GT_IF_WITH_ASSERT(pBranchVector != NULL)
        {
            // Add each of the branch time lines:
            for (int timelineIndex = 0; timelineIndex < (int)pBranchVector->size(); timelineIndex ++)
            {
                // Get the current command:
                acQGanttChartBarData currentTimelineData = (*pBranchVector)[timelineIndex];

                // Create new time line:
                gtUInt64 barStartInMs = currentTimelineData._barStart;
                gtUInt64 barEndInMs = currentTimelineData._barEnd;
                qcTimelineItem* pTimeLine = new qcTimelineItem(barStartInMs, barEndInMs);
                GT_ASSERT_ALLOCATION(pTimeLine);

                // Check if max value should increase:
                maxValue = max(currentTimelineData._barEnd, maxValue);

                // Set the time line bg color:
                pTimeLine->setBackgroundColor(currentTimelineData._barColor);
                pTimeLine->setText(currentTimelineData._label.asASCIICharArray());

                // Add this time line to the branch:
                pTimelineBranch->addTimelineItem(pTimeLine);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acQGanttChartWindow::setNumberOfBranches
// Description: Sets the number of branches in the chart by assigning that many
//              items in the relevant vectors
// Author:      Sigal Algranaty
// Date:        15/12/2011
// ---------------------------------------------------------------------------
void acQGanttChartWindow::setNumberOfBranches(int numberOfBranches)
{
    // Clear any former data:
    _backgroundSectionsData.clear();
    _barsDataByBranch.deleteElementsAndClear();

    // Items at negative line numbers will always be off screen:
    acQGanttChartBackgroundSectionData defaultBackgroundSection(_bgColor);

    // Add the data:
    for (int i = 0; i < numberOfBranches; i++)
    {
        // Add a background data:
        _backgroundSectionsData.push_back(defaultBackgroundSection);

        // Create a new branch data:
        gtVector<acQGanttChartBarData>* pCurrentBranchVec = new gtVector<acQGanttChartBarData>;
        GT_ASSERT_ALLOCATION(pCurrentBranchVec);
        _barsDataByBranch.push_back(pCurrentBranchVec);
    }
}

// ---------------------------------------------------------------------------
// Name:        acQGanttChartWindow::setSeriesData
// Description: Sets the background color of a series
// Author:      Sigal Algranaty
// Date:        15/12/2011
// ---------------------------------------------------------------------------
void acQGanttChartWindow::setBranchData(int seriesIndex, const QColor& bgCol, const gtString& toolTip, const gtString& label)
{
    // Sanity check:
    int numberOfBackgroundSections = (int)_backgroundSectionsData.size();
    GT_IF_WITH_ASSERT((seriesIndex >= 0) && (seriesIndex < numberOfBackgroundSections))
    {
        // Set the color and tooltip:
        acQGanttChartBackgroundSectionData& sectionBackgroundData = _backgroundSectionsData[seriesIndex];
        sectionBackgroundData._sectionColor = bgCol;
        sectionBackgroundData._toolTip = toolTip;
        sectionBackgroundData._label = label;
    }
}

// ---------------------------------------------------------------------------
// Name:        acQGanttChartWindow::addBarToSeries
// Description: Adds a bar to a series
// Author:      Sigal Algranaty
// Date:        15/12/2011
// ---------------------------------------------------------------------------
void acQGanttChartWindow::addTimelineToBranch(int branchIndex, gtUInt64 barStart, gtUInt64 barEnd, const QColor& barColor, int originalIndex, const gtString& label, const gtString& tooltip)
{
    // Sanity check index:
    int numberOfBranches = (int)_barsDataByBranch.size();
    GT_IF_WITH_ASSERT((branchIndex >= 0) && (branchIndex < numberOfBranches))
    {
        // Sanity check pointer:
        gtVector<acQGanttChartBarData>* pBranchVector = _barsDataByBranch[branchIndex];
        GT_IF_WITH_ASSERT(pBranchVector != NULL)
        {
            // Make sure the bar will be visible:
            if (barStart < barEnd)
            {
                // Create and add the item:
                acQGanttChartBarData newBarData(barStart, barEnd, barColor, originalIndex, label, tooltip);
                pBranchVector->push_back(newBarData);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acQGanttChartWindow::clearSeriesData
// Description: Clear all the bars from a series
// Author:      Sigal Algranaty
// Date:        15/12/2011
// ---------------------------------------------------------------------------
void acQGanttChartWindow::clearBranchData(int seriesIndex)
{
    // Sanity check index:
    int numberOfSeries = (int)_barsDataByBranch.size();
    GT_IF_WITH_ASSERT((seriesIndex >= 0) && (seriesIndex < numberOfSeries))
    {
        // Sanity check pointer:
        gtVector<acQGanttChartBarData>* pSeriesVector = _barsDataByBranch[seriesIndex];
        GT_IF_WITH_ASSERT(pSeriesVector != NULL)
        {
            // Clear the vector:
            pSeriesVector->clear();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acQGanttChartWindow::chartHasData
// Description: Returns true iff the chart has any data
// Author:      Uri Shomroni
// Date:        15/3/2010
// ---------------------------------------------------------------------------
bool acQGanttChartWindow::chartHasData() const
{
    bool retVal = false;

    // Go over all the series:
    int numberOfSeries = (int)_barsDataByBranch.size();

    for (int i = 0; i < numberOfSeries; i++)
    {
        // Pointer sanity check:
        const gtVector<acQGanttChartBarData>* pCurrentSeries = _barsDataByBranch[i];
        GT_IF_WITH_ASSERT(pCurrentSeries != NULL)
        {
            // If this series has at least one bar:
            if (pCurrentSeries->size() > 0)
            {
                // There is data in the chart, no need to look further:
                retVal = true;
                break;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        acQGanttChartWindow::itemClicked
// Description:
// Arguments:   qcTimelineItem* pTimelineItem
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        20/12/2011
// ---------------------------------------------------------------------------
void acQGanttChartWindow::itemClicked(qcTimelineItem* pTimelineItem)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pTimelineItem != NULL)
    {
        // Get the timeline index:
        int timelineIndex = pTimelineItem->index();

        // Get the parent branch:
        qcTimelineBranch* pTimelineBranch = pTimelineItem->parentBranch();
        GT_IF_WITH_ASSERT(pTimelineBranch != NULL)
        {
            // Check what is this timeline item, and which branch / timeline index it represents:
            int branchIndex = pTimelineBranch->rowIndex();

            // Emit the timeline clicked signal:
            emit timelineClicked(branchIndex, timelineIndex);
        }
    }
}
