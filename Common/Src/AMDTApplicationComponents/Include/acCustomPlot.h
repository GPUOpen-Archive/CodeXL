//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acCustomPlot.h
///
//==================================================================================


#ifndef ACCUSTOMPLOT_H
#define ACCUSTOMPLOT_H

// QCustomPlot:
#include <qcustomplot.h>

// Local:
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>

class acCustomPlot;


// ----------------------------------------------------------------------------------
// Class Name:          acCustomPlotDropManager : public QObject
// General Description: This class is used for emitting the drop signal whenever an
//                      acCustomPlot object is dropped.
//                      acCustomPlot cannot emit these signals, since we have many instances
//                      of it, and we want the user to only connect to one object.
// Author:              Sigal Algranaty
// Creation Date:       28/12/2014
// ----------------------------------------------------------------------------------
class AC_API acCustomPlotDropManager : public QObject
{
    Q_OBJECT

public:

    // Singleton:
    static acCustomPlotDropManager& Instance();

    /// Emit a plot dropped signal:
    void EmitPlotDropped(acCustomPlot* pPlotDropped, acCustomPlot* pDraggedPlot);

signals :

    /// Signals for drag & drop:
    void PlotDropped(acCustomPlot* pPlotDropped, acCustomPlot* pDraggedPlot);

protected:

    // My single instance:
    static acCustomPlotDropManager* m_psMySingleInstance;

};


// ----------------------------------------------------------------------------------
// Class Name:          acCustomPlot : public QCustomPlot
// General Description: Inherits QCustomPlot. Is used for implementing drag & drop functionality
//                      In Qt, in order to catch and handle drag & drop events, the object should be derived,
//                      and drag and drop events should be override. This class is implementing this feature.
//                      The class overrides dragEnterEvent, dragMoveEvent, dropEvent and mousePressEvent.
//                      See these functions documentation for more details
// Author:              Sigal Algranaty
// Creation Date:       28/12/2014
// ----------------------------------------------------------------------------------
class AC_API acCustomPlot : public QCustomPlot
{
    Q_OBJECT

public:

    acCustomPlot(QWidget* pParent);
    virtual ~acCustomPlot();

    /// Enable / disable drag drop functionality for this object:
    /// \param isEnabled true iff drag and drop is enabled
    void EnableDragAndDrop(bool isEnabled);

    /// Are we dragging the plot now?
    bool Dragging() { return m_isDragging; }

    /// Highlight / un-highlight the plot:
    void SetHighlighted(bool isHighlighted);

signals:

    /// Is triggered when the mouse enters the plot area:
    void PlotEntered(acCustomPlot* pPlot);

    /// Is triggered when the mouse leaves the plot area:
    void PlotLeave(acCustomPlot* pPlot);

protected:

    /// Override QWidget (will be used for implementing drag & drop):

    /// Accept drag event only if it is a custom plot item drag:
    /// \param pEvent Qt event with the drag information
    void dragEnterEvent(QDragEnterEvent* pEvent);

    /// Accept drag event only if it is a custom plot item drag:
    /// \param pEvent Qt event with the drag information
    void dragMoveEvent(QDragMoveEvent* pEvent);

    /// Is called when an item is dropped (only after QDragEnterEvent was accepted). We do not handle
    /// the drop here, we emit a signal. The user of acCustomPlot can handle it according to its needs.
    void dropEvent(QDropEvent* pEvent);

    /// Override QCustomPlot. Is used for the drag and drop implementation
    void mousePressEvent(QMouseEvent* pEvent);

    /// Override QCustomPlot. Is used to set the drag flag:
    void mouseReleaseEvent(QMouseEvent* pEvent);

    /// Overrides QCustomPlot (sending a signal for users):
    virtual void enterEvent(QEvent* pEvent);

    /// Overrides QCustomPlot (sending a signal for users):
    virtual void leaveEvent(QEvent* pEvent);


protected:

    /// True iff drag and drop is enabled for this plot:
    bool m_isDragAndDropEnabled;

    /// True iff currently the user is dragging the plot:
    bool m_isDragging;

    /// True iff the background of the plot is highlighted:
    bool m_isHighlighted;

    /// Static members for the plots highlight:
    static QPixmap* m_spHighlightBackgroundPixmap;
    static QPixmap* m_spBackgroundPixmap;
};


#endif
