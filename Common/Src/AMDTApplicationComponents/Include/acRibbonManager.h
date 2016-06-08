//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acRibbonManager.h
///
//==================================================================================

//------------------------------ acRibbonManager.h ------------------------------

#ifndef _ACRIBBONMANAGER_H_
#define _ACRIBBONMANAGER_H_

// Qt
#include <qtIgnoreCompilerWarnings.h>
#include <QWidget>
#include <QSplitter>
#include <QLabel>
#include <QScrollArea>

// Local
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>

class QVBoxLayout;
class acNavigationChart;

#define RIBBON_MANAGER_SPACE 2

class acRibbonTooltipEventFilter : public QObject
{
public:
    acRibbonTooltipEventFilter() {};
    ~acRibbonTooltipEventFilter() {};

    bool eventFilter(QObject* object, QEvent* event);
};

class acRibbonEventFilter : public QObject
{
public:
    acRibbonEventFilter() {};
    ~acRibbonEventFilter() {};

    bool eventFilter(QObject* object, QEvent* event);
};

class acRibbonManagerButton : public QLabel
{
    Q_OBJECT
public:
    acRibbonManagerButton(QWidget* pOwningWidget);
    ~acRibbonManagerButton();

signals:
    /// button pressed with the owning widget that is to be hidden/shown
    void ButtonPressed(QWidget*);

protected:
    /// Overrides QLabel (in order to enable button like behavior with minimal size):
    virtual void enterEvent(QEvent* pEvent);

    /// Overrides QLabel (in order to enable button like behavior with minimal size):
    virtual void leaveEvent(QEvent* pEvent);

    /// Overrides QLabel (in order to enable button like behavior with minimal size):
    virtual bool event(QEvent* pEvent);

    /// sets if the label is in mouse press event
    bool m_isInMousePressEvent;

    /// sets that the mouse is over the control
    bool m_mouseIsInLabel;

    /// the widget that should be hidden or displayed by pressing this button
    QWidget* m_pOwningWidget;
};

class AC_API acRibbonData
{
public:
    acRibbonData();

    /// is the ribbon visible
    bool m_isVisible;

    /// the Button of the ribbon
    QLabel* m_pButton;

    /// The label of the ribbon
    QLabel* m_pLabel;

    /// The widget that is the ribbon
    QWidget* m_pRibbon;

    /// The wrapper widget that helps us calculate the sizes
    QWidget* m_pWrapper;

    /// last open size
    int m_lastOpenSize;

    /// is fixed size (can not be resized)
    bool m_isFixedSize;

    /// is the ribbon enabled;
    bool m_isEnabled;

    /// ratio height. if it is fixed size, this size must be used. if it is positive also it will be used as as
    // if it is a negative number then the ratio will be used.
    /// this is the ribbon height, not including the banner
    int m_ratioHeight;
};

class AC_API acIRibbonTime
{
public:
    /// string to display as the tool tip for a specific time
    virtual QString GetTooltip(double iTime, QMouseEvent* pMouseEvent) = 0;
};

class acRibbonWrapper : public QWidget
{
public:
    acRibbonWrapper(QWidget* childWidget);
    virtual ~acRibbonWrapper();

    /// Overridden QWidget method that returns the preferred size of this widget.
    /// \return the preferred size of this widget.
    virtual QSize sizeHint() const;

protected:
    QWidget* m_pChildWidget;
};

class AC_API acRibbonManager : public QWidget
{
    Q_OBJECT

    friend class acRibbonEventFilter;
    friend class acRibbonTooltipEventFilter;

public:
    acRibbonManager(QWidget* pParent);
    ~acRibbonManager();

    /// Add a ribbon to the manager
    /// \param pRibbon added ribbon
    /// \param ribbonName ribbon name
    /// \param isFixedSize is the ribbon fixed size
    /// \param hasCloseButton does the ribbon has a close button
    /// \param isOpen is the ribbon opened when first presented
    void AddRibbon(QWidget* pRibbon, QString ribbonName, int initialHeight, bool isFixedSize = false, bool hasCloseButton = true, bool isOpen = true);

    /// Set the ribbon that relative to it the Bound Frame is drawn
    /// it can be set to nullptr which means no Bound frame is drawn
    /// \param pBoundWidgetController is the controlling ribbon and must be a acNavigation type
    ///  widget that update the bounding span in the frame control widget and sends the BoundingChanged(int& low, int& high) signal
    /// \param pRibbonBottom the ribbon the defines the lower bound of the frame
    void SetBoundFrameControlRibbons(acNavigationChart* pBoundWidgetController, QWidget* pControllerRibbon, QWidget* pRibbonBottom);

    /// change a ribbon name
    /// \param pRibbon added ribbon
    /// \param newRibbonName new ribbon name
    void ChangeRibbonName(QWidget* pRibbon, QString newRibbonName);

    /// change when adding a ribbon we might not know its initial height but may know it before the vie wis displayed
    /// \param pRibbon added ribbon
    /// \param newInitialHeight new ribbon height
    void ChangeRibbonInitialHeight(QWidget* pRibbon, int newInitialHeight);

protected:
    /// get the index of a ribbon by its widget. -1 = widget not found
    /// \param pWidget widget to look for as a ribbon
    int IndexOfRibbon(QWidget* pWidget);

    /// Close a ribbon and increase the size of other ribbons accordingly
    void CloseRibbon(int index);

    /// Open a ribbon and decrease the size of other ribbons accordingly
    void OpenRibbon(int index);

    /// Check A special case of disabling the splitter before the first ribbon of the group of close ribbons the continue until the last ribbon
    void CheckGroupEndRibbonsCase(QWidget* pWidget);

    /// handle the resize event to store the last open size
    void resizeEvent(QResizeEvent* event);

    /// handle the mouse move event. this is not an override version of the mouseMoveEvent of QWidget!
    void MouseMoveEvent(QObject* object, QEvent* event);

    /// handle the mouse leave event. this is not an override version of the mouseLeave of QWidget!
    void MouseLeaveEvent(QObject* object, QEvent* event);

    /// Check if there is a ribbon under the global position
    /// \return the index of the ribbon. - 1 not found
    int IsRibbonUnderPosition(QPoint& globalPos);

    /// handle the Mouse Dbl click event. this is not an override version of the mouseDblClickEvent of QWidget!
    void MouseDblClickEvent(QObject* object);

    /// install event filter on a widget and all its children qwidgets
    void RecursiveInstallEventFilter(QWidget* pWidget);

    /// get a ribbon bottom coordinate in the ribbon manager client coordinates
    int GetRibbonBottomCoordinate(QWidget* pRibbon);

    /// get the bottom coordinate of the controller axis in the ribbon manager client coordinates. if there is no controller, return 0 as the top of the ribbon manager
    /// since this is used as ref point to stretch a drawing of something up to this point.
    int GetControllerAxisBottomCoordinate();

    /// reposition the bounding frame
    void RepositionBoundingFrame();

    /// hide tooltip information
    void HideTooltip();

    /// Show the tooltip
    /// \param tooltipPos tooltip position
    /// \param shouldEmitSyncSignal should emit the time line signal
    void ShowTooltip(QObject* object, QEvent* event, double coordTime, QPoint& tooltipPos, bool shouldEmitSyncSignal);

    /// toggle the state of a ribbon open->close close->open
    void ToggleRibbonState(int index);

    /// Enable/disable last ribbon
    /// \param is in open action mark last open ribbon as enabled, close ribbon, mark last ribbon as disabled
    void LastRibbonCheck(bool openAction);

    /// add height to a ribbon if max height allows it and return true. if not reduce from delta what was added and return false
    bool AddHeightToRibbon(QList<int>& ribbonsSizes, int ribbonIndex, int& deltaSize);

    /// Set initial sizes of the ribbons
    void RecalcHeights(int height);

    /// Store the last visible heights and ratios
    void StoreLastOpenHeights(QList<int>& currentSizes);

    /// pass the mouse click below the tooltip controls;
    /// \param pEvent mouse click event
    void PassMouseClickBelowToolTip(QEvent* pEvent);

signals:
    /// Signal to set the width of the elements of ribbons that have time line components
    /// \params legendWidth the left side of the ribbon is the legend or the tree part width
    /// \param timelineWidth the width of the timeline
    void SetElementsNewWidth(int legendWidth, int timelineWidth);

    /// Signal for the tooltip line used to show sync on other ribbons
    /// \param visible if the line should be displayed or not
    /// \param time of the line
    void ShowTimeLine(bool visible, double timePos);

protected slots:
    /// Handle the changing of the Bound change
    void OnBoundChanged(int& lowBound, int& highBound);

    /// handle pressing and open/close button
    void OnButtonPressed(QWidget*);

    /// Handle the movement of the splitter handles
    void OnSplitterMoved(int pos, int index);

    /// handle movement of the scrollbar
    void OnSliderMoved(int value);

    /// handle a timeline change to show a sync point if needed
    /// \param visible if the line should be displayed or not
    /// \param time of the line
    void OnShowTimeLine(bool visible, double timePos);

private:
    /// Main layout of the ribbons
    QVBoxLayout* m_pMainLayout;

    /// White space that allows us to handle the closer of ribbons better (placed as last ribbon)
    QWidget* m_pWhiteSpace;

    /// main splitter that the widgets are added to
    QSplitter* m_pSplitter;

    /// Bound ribbon that is used to draw the Bound To
    QWidget* m_pBoundFrameRibbonBottom;

    /// ribbon that holds the controller
    QWidget* m_pControllerRibbon;

    /// the controls that send the bounds update
    acNavigationChart* m_pBoundsController;

    /// the low and high ranges of the Bound bounds
    int m_lowBoundRange;
    /// -1 high Bound means the entire Bound
    int m_highBoundRange;

    /// line widget of tracking
    QWidget* m_pBoundingBoxLeftLine;
    QWidget* m_pBoundingBoxRightLine;
    QWidget* m_pBoundingBoxBottomLine;

    /// visibility vector for the added widgets
    QVector<acRibbonData> m_ribbonDataVector;

    /// the tooltip line
    QWidget* m_pTooltipLine;

    /// vector of labels to be displayed with the tooltip line
    QVector<QLabel*> m_tooltipLabels;

    /// mouse tracking event filter
    acRibbonEventFilter* m_pEventFilter;

    /// tooltip widgets event filter
    acRibbonTooltipEventFilter* m_pTooltipEventFilter;

    /// Scroll are for the ribbon
    QScrollArea* m_pScrollArea;
};

#endif // _ACRIBBONMANAGER_H_