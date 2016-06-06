//------------------------------ acQGanttChartWindow.h ------------------------------

#ifndef __ACQGANTTCHARTWINDOW_H
#define __ACQGANTTCHARTWINDOW_H

// AMDTControls:
#include <qcTimeline.h>

// Infra:
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>
#include <AMDTApplicationComponents/Include/acOpenGLChartWindow.h>

// ----------------------------------------------------------------------------------
// Class Name:          AC_API acGanttChartBackgroundSectionData
// General Description: Holds all the data for a section of the chart's background
// Author:               Uri Shomroni
// Creation Date:        23/2/2010
// ----------------------------------------------------------------------------------
struct AC_API acQGanttChartBackgroundSectionData
{
public:
    acQGanttChartBackgroundSectionData(const QColor& sectionColor)
        : _firstLine(-1), _lastLine(-1), _sectionColor(sectionColor) {};
    ~acQGanttChartBackgroundSectionData() {};

public:
    int _firstLine;
    int _lastLine;
    QColor _sectionColor;
    gtString _toolTip;
    gtString _label;
};

// ----------------------------------------------------------------------------------
// Class Name:          AC_API acGanttChartBarData
// General Description: Holds all the data for a single bar in the chart
// Author:              Uri Shomroni
// Creation Date:       23/2/2010
// ----------------------------------------------------------------------------------
struct AC_API acQGanttChartBarData
{
public:
    acQGanttChartBarData(gtUInt64 barStart, gtUInt64 barEnd, const QColor& barColor, int originalIndex, const gtString& label, const gtString& tooltip)
        : _barStart(barStart), _barEnd(barEnd), _barColor(barColor), _originalIndex(originalIndex), _label(label), _tooltip(tooltip), _barLine(-1) {};
    ~acQGanttChartBarData() {};

public:
    gtUInt64 _barStart;
    gtUInt64 _barEnd;
    QColor _barColor;
    int _originalIndex;
    gtString _label;
    gtString _tooltip;
    int _barLine;
};

// ----------------------------------------------------------------------------------
// Class Name:          AC_API acQGanttChartWindow : public wxGLCanvas
// General Description: A window drawing a gantt chart using OpenGL.
// Author:              Uri Shomroni
// Creation Date:       22/2/2010
// ----------------------------------------------------------------------------------
class AC_API acQGanttChartWindow : public qcTimeline
{
    Q_OBJECT

public:

    acQGanttChartWindow(QWidget* pParent, QColor bgColor);
    virtual ~acQGanttChartWindow();

    // Data loading:
    void clearAllData();
    void updateGanttChart();
    void setNumberOfBranches(int numberOfBranches);
    void setBranchData(int seriesIndex, const QColor& bgCol, const gtString& toolTip, const gtString& label);
    void addTimelineToBranch(int seriesIndex, gtUInt64 barStart, gtUInt64 barEnd, const QColor& barColor, int originalIndex, const gtString& label, const gtString& tooltip);
    void clearBranchData(int seriesIndex);

signals:

    /// Signal emitted when a branch item is clicked:
    void timelineClicked(int branchIndex, int timelineIndex);

protected slots:

    void itemClicked(qcTimelineItem* item);

protected:

    // Disallow use of my default constructor:
    acQGanttChartWindow();

    // Data calculation and construction:
    bool chartHasData() const;

private:

    // Background color:
    QColor _bgColor;

    // Chart data:
    gtVector<acQGanttChartBackgroundSectionData> _backgroundSectionsData;
    gtPtrVector<gtVector<acQGanttChartBarData>* > _barsDataByBranch;

};

#endif //__ACQGANTTCHARTWINDOW_H

