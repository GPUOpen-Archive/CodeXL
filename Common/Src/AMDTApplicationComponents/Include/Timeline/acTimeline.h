//==================================================================================
// Copyright (c) 2011 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acTimeline.h
///
//==================================================================================

#ifndef _ACTIMELINE_H_
#define _ACTIMELINE_H_

// Qt:
#include <QScrollBar>

// Local:
#include <AMDTApplicationComponents/Include/Timeline/acTimelineGrid.h>
#include <AMDTApplicationComponents/Include/Timeline/acTimelineBranch.h>
#include <AMDTApplicationComponents/Include/Timeline/acTimelineItem.h>

/// Constant for the horizontal space used to draw the tree marks (plus/minus box used to expand collapse a row).
static const int ACTIMELINE_TreeMarkSpace = 12;
class QLabel;

/// Timeline widget.
class AC_API acTimeline : public QWidget
{
    Q_OBJECT
public:
    /// Construct/initialize a new instance of the acTimeline class.
    /// \param parent the parent widget
    /// \param flags the window flags
    acTimeline(QWidget* parent, Qt::WindowFlags flags = 0);


    /// Destroys an instance of the acTimeline class.
    virtual ~acTimeline();

    /// Gets the height in pixels of the top of the timeline where the grid is painted.
    /// \return the height in pixels of the top of the timeline where the grid is painted.
    int timelineSectionTopSpace() const;

    /// Sets the width in pixels of the left margin of the title section area.
    /// \param newTitleSectionLeftSpace the width in pixels of the left margin of the title section area.
    void setTitleSectionLeftSpace(const int newTitleSectionLeftSpace);

    /// Gets the width in pixels of the left margin of the title section area.
    /// \return the width in pixels of the left margin of the title section area.
    int titleSectionLeftSpace() const { return m_nTitleSectionLeftSpace; }

    /// Sets the width in pixels of the right margin of the title section area.
    /// \param newTitleSectionRightSpace the width in pixels of the right margin of the title section area.
    void setTitleSectionRightSpace(const int newTitleSectionRightSpace);

    /// Gets the width in pixels of the right margin of the title section area.
    /// \return the width in pixels of the right margin of the title section area.
    int titleSectionRightSpace() const { return m_nTitleSectionRightSpace; }

    /// Sets the width in pixels of the right margin of the timeline.
    /// \param newTimelineSectionRightSpace the width in pixels of the right margin of the timeline.
    void setTimelineSectionRightSpace(const int newTimelineSectionRightSpace);

    /// Gets the width in pixels of the right margin of the timeline.
    /// \return the width in pixels of the right margin of the timeline.
    int timelineSectionRightSpace() const { return m_nTimelineSectionRightSpace; }

    /// Sets the width in pixels of the indentation used for each sub branch.
    /// \param newBranchIndentation the width in pixels of the indentation used for each sub branch.
    void setBranchIndentation(const int newBranchIndentation) { m_nBranchIndentation = newBranchIndentation; }

    /// Gets the width in pixels of the indentation used for each sub branch.
    /// \return the width in pixels of the indentation used for each sub branch.
    int branchIndentation() const { return m_nBranchIndentation; }

    /// Sets the timestamp for the start of the timeline.
    /// \param newStartTime the start of the timeline.
    void setStartTime(const quint64 newStartTime);

    /// Gets the timestamp for the start of the timeline.
    /// \return the start of the timeline.
    quint64 startTime() const { return m_nStartTime; }

    /// Sets the full range of the timeline.
    /// \param newFullRange the full range of the timeline.
    void setFullRange(const quint64 newFullRange);

    /// Gets the full range of the timeline.
    /// \return the full range of the timeline.
    quint64 fullRange() const { return m_nFullRange; }

    /// Gets the timestamp for the start of the visible portion of the timeline.
    /// \return the timestamp for the start of the visible portion of the timeline.
    quint64 visibleStartTime() const { return m_nVisibleStartTime; }

    /// Gets the range of the visible portion of the timeline.
    /// \return the range of the visible portion of the timeline.
    quint64 visibleRange() const { return m_nVisibleRange; }

    /// Sets the zoom factor for the timeline.  Setting the zoom factor also affects the render width and the offset.
    /// \param newZoomFactor the new zoom factor.
    void setZoomFactor(const double newZoomFactor);

    /// Gets the zoom factor for the timeline.
    /// \return the zoom factor for the timeline.
    double zoomFactor() const { return m_dZoomFactor; }

    /// Sets the maximum zoom factor.
    /// \param newMaxZoom the new maximum zoom factor.
    void setMaxZoom(const double newMaxZoom) { m_dMaxZoom = newMaxZoom; }

    /// Gets the maximum zoom factor.
    /// \return the maximum zoom factor.
    double maxZoom() const { return m_dMaxZoom; }

    /// Sets the pivot position. Pivot represents the location of the vertical line as a fraction of the timeline screen width.  Valid range is 0 to 1.
    /// \param newPivot the pivot position.
    void setPivot(const double newPivot);

    /// Gets the pivot position. Pivot represents the location of the vertical line as a fraction of the timeline screen width.  Valid range is 0 to 1.
    /// \return the pivot position.
    double pivot() const { return m_dPivot; }

    /// Sets the zoom pivot position.
    /// \param newZoomPivot the zoom pivot position.
    void setZoomPivot(const double newZoomPivot);

    /// Gets the zoom pivot position.
    /// \return the zoom pivot position.
    double zoomPivot() const { return m_dZoomPivot; }

    /// Sets the default branch height for branches added to this timeline.
    /// \param newDefaultBranchHeight the default branch height for branches added to this timeline.
    void setDefaultBranchHeight(const int newDefaultBranchHeight) { m_nDefaultBranchHeight = newDefaultBranchHeight; }

    /// Gets the default branch height for branches added to this timeline.
    /// \return the default branch height for branches added to this timeline.
    int defaultBranchHeight() const { return m_nDefaultBranchHeight; }

    /// Gets the position of the mouse cursor over the widget.
    /// \return the position of the mouse cursor over the widget.
    QPoint mouseLocation() const { return m_mouseLocation; }

    /// Gets a flag indicating whether or not the user is currently mouse-dragging the timeline.
    /// \return a flag indicating whether or not the user is currently mouse-dragging the timeline.
    bool mouseDragging() const { return m_bStartDrag; }

    /// Gets a flag indicating whether or not the user is currently mouse-drag-selecting the timeline.
    /// \return a flag indicating whether or not the user is currently mouse-drag-selecting the timeline.
    bool mouseSelectDragging() const { return m_bSelectDragging; }

    /// Gets the width of the title section of this timeline.
    /// \return the width of the title section of this timeline.
    int titleWidth() const { return m_nTitleWidth; }

    /// Gets the row width for this timeline.  This is the area containing actual timeline items (excluding the title section at the left of the timeline).
    /// \return the row width for this timeline.  This is the area containing actual timeline items (excluding the title section at the left of the timeline).
    int rowWidth() const;

    /// Gets the total height of all branches, taking into account whether or not branches are folded.
    /// \return the height in pixels of all branches.
    int cumulativeBranchHeight();

    /// Sets a flag indicating whether or not the timeline is painted with gradient colors.
    /// \param newGradientPainting a flag indicating whether or not the timeline is painted with gradient colors.
    void setGradientPainting(const bool newGradientPainting) { m_bGradientPainting = newGradientPainting; }

    /// Gets a flag indicating whether or not the timeline is painted with gradient colors.
    /// \return a flag indicating whether or not the timeline is painted with gradient colors.
    bool gradientPainting() const { return m_bGradientPainting; }

    /// Sets a flag indicating whether or not the timeline is painted with rounded rectangles.
    /// \param newRoundedRectangles a flag indicating whether or not the timeline is painted with rounded rectangles.
    void setRoundedRectangles(const bool newRoundedRectangles) { m_bRoundedRectangles = newRoundedRectangles; }

    /// Gets a flag indicating whether or not the timeline is painted with rounded rectangles.
    /// \return a flag indicating whether or not the timeline is painted with rounded rectangles.
    bool roundedRectangles() const { return m_bRoundedRectangles; }

    /// Sets a flag indicating whether or not the pivot position follows the mouse cursor.
    /// \param newPivotMouseTracking a flag indicating whether or not the pivot position follows the mouse cursor.
    void setPivotMouseTracking(const bool newPivotMouseTracking) { m_bPivotMouseTracking = newPivotMouseTracking; };

    /// Gets a flag indicating whether or not the pivot position follows the mouse cursor.
    /// \return a flag indicating whether or not the pivot position follows the mouse cursor.
    bool pivotMouseTracking() { return m_bPivotMouseTracking; } // can we just use QWidget::mouseTracking for this????

    /// Gets the branch located at the specified Y coordinate.
    /// \param y the y coordinate.
    /// \return the branch located at the specified Y coordinate.
    acTimelineBranch* getBranchFromY(int y) const;

    /// Gets the first branch found with the specified text.
    /// \param branchText the branch text to find
    /// \param partialMatch flag indicating whether to perform a partial match or complete match
    /// \param shouldRecurse flag indicating whether or not to check child branches
    /// \return the first branch found with the specified text.  Returns NULL if branch can not be found
    acTimelineBranch* getBranchFromText(const QString& branchText, bool partialMatch, bool shouldRecurse = true) const;

    /// Gets the timeline item located at the specified X and Y coordinate.
    /// \param x the x coordinate.
    /// \param y the y coordinate.
    /// \return the timeline item located at the specified X and Y coordinate.
    acTimelineItem* getTimelineItem(int x, int y) const;

    /// Shows a timeline item, without changing the zoom level
    /// \param item the timeline item to show
    void ShowItem(acTimelineItem* item);

    /// Move the item visible range, so that the requested item is horizontally centered
    /// \param pItemToCenter the timeline item to center
    /// \param shouldSelect should the zoomed item be selected?
    void DisplayItemAtHorizontalCenter(acTimelineItem* pItemToCenter, bool shouldSelect);

    /// Zooms into a timeline item far enough so the the item's block is wide enough for its Text
    /// \param item the timeline item to zoom into
    /// \param shouldSelect should the zoomed item be selected?
    void ZoomToItem(acTimelineItem* item, bool shouldSelect);

    /// Zooms into a timeline item so that it takes the entire width of the timeline
    /// \param item the timeline item to zoom into
    void zoomToItemFull(acTimelineItem* item);

    /// Shows a timeline branch (scrolls the timeline vertically so that the branch is in view)
    /// \param branch the timeline branch to show
    void showBranch(acTimelineBranch* branch);

    /// Gets the X coordinate of the specified timestamp.
    /// \param time the specified time.
    /// \return the X coordinate of the specified timestamp.
    int getXCoordOfTime(quint64 time) const;

    /// Converts a fraction value from a visible fraction to a full timeline fraction.
    /// \param fractionOfVisiblePortion a fraction value representing a fraction of the visible portion of the timeline.
    /// \return a fraction value representing a fraction of the full timeline.
    double getFractionOfFullTimeline(double fractionOfVisiblePortion);

    /// Gets the full render width of the entire timeline, given the full range of the timeline and the current zoom factor.
    /// \return the full render width of the entire timeline, given the full range of the timeline and the current zoom factor.
    quint64 renderWidth() const { return m_nRenderWidth; }

    /// Gets the inverse of the full range of the timeline. This is computed and cached any time the full range changes.
    //// \return the inverse of the full range of the timeline. This is computed and cached any time the full range changes.
    double invRange() const { return m_dInvRange; }

    /// Gets the qcTimelineGrid grid instances associated with this timeline.
    /// \return the qcTimelineGrid grid instances associated with this timeline.
    acTimelineGrid* grid() const { return m_pGrid; }

    /// Sets the horizontal offset of the timeline.  It is an offset from the beginning of the timeline.
    /// \param newOffset the horizontal offset of the timeline.  It is an offset from the beginning of the timeline.
    void setOffset(const qint64 newOffset);

    /// Gets the horizontal offset of the timeline.  It is an offset from the beginning of the timeline.  Will always be zero when the timeline is fully zoomed out.
    /// \return the horizontal offset of the timeline.  It is an offset from the beginning of the timeline.  Will always be zero when the timeline is fully zoomed out.
    qint64 offset() { return m_nOffset; }

    /// Sets the vertical offset of the timeline (the vertical scroll bar value). Zero indicates not scrolled at all.
    /// \param newVOffset the vertical offset of the timeline
    void setVOffset(const int newVOffset);

    /// Gets the vertical offset of the timeline (zero, unless the timeline is scrolled vertically).
    /// \return the vertical offset of the timeline (zero, unless the timeline is scrolled vertically).
    int vOffset() const { return m_nVOffset; }

    /// Adds the specified branch to this timeline.  Add branch hands off ownership (memory-wise) of the branch to the timeline.
    /// Note: performance of the add operation is better when timeline items are added to a branch before the branch is added to the timeline.
    /// \param branch the branch to add to the timeline.
    /// \return true if the branch was added, false otherwise.
    bool addBranch(acTimelineBranch* branch);

    /// Sets the selected branch for this timeline to the specified branch.
    /// \param branch the branch to select.
    void setSelectedBranch(acTimelineBranch* branch);

    /// Gets the currently selected branch for this timeline.
    /// \return the currently selected branch for this timeline.
    const acTimelineBranch* selectedBranch() const { return m_pSelectedBranch; }

    /// Adds a marker, which is a vertical line spanning the entire timeline.
    /// Will adjust the start time or range if the addenew marker is out of range.
    /// \param timeStamp the time stamp where the marker should be located.
    /// \param color the color of the marker.
    /// \return true if the marker was added, false otherwise.
    bool addMarker(quint64 timeStamp, QColor color);

    /// Removes a marker at the specified timestamp.  Does not affect the timeline's range
    /// (i.e will not contract the range if it was expanded when the marker was added).
    /// \param timeStamp the timestamp where the marker should be removed
    /// \return true if the marker was removed, false if there is no marker at the specified time stamp.
    bool removeMarker(quint64 timeStamp);

    /// Removes all markers from the timeline.
    bool clearMarkers();

    /// Clears all branches and resets the timeline to be empty.
    void reset();

    /// Sets the grid labels precision (amount of digits after the dot [3 by default])
    /// \param precision the new precision
    void SetGridLabelsPrecision(unsigned int precision);

    /// Set the visible time range
    /// \param startVisibleTime starting visible time
    /// \param visibleRange range of visible time
    void SetVisibleRange(const quint64 startVisibleTime, const quint64 visibleRange);

    QList<acTimelineBranch*>& GetBranches() { return m_subBranches; }

    /// Sets the visibility of time line tooltips
    void ShowTimeLineTooltips(bool shown) { m_showToolTip = shown; }

    /// Set should v scroll to end flag
    void ShouldScrollToEnd(bool scroll) { m_shouldVScrollToEnd = scroll; }

    /// returns pixel coord of a time value
    /// \param time value
    int TimeToPixel(double timeValue, bool checkBounds = true);

protected:
    /// Overridden QWidget method used to display tooltip hints.
    /// \param event the event parameters.
    bool event(QEvent* event);

    /// Overridden QWidget method called when this widget needs to be painted.
    /// \param event the event parameters.
    void paintEvent(QPaintEvent* event);

    /// Overridden QWidget method called when the mouse is pressed.
    /// \param event the event parameters.
    void mousePressEvent(QMouseEvent* event);

    /// Overridden QWidget method called when the mouse is released.
    /// \param event the event parameters.
    void mouseReleaseEvent(QMouseEvent* event);

    /// Overridden QWidget method called when the mouse is moved.
    /// \param event the event parameters.
    void mouseMoveEvent(QMouseEvent* event);

    /// Overridden QWidget method called when the mouse is double clicked.
    /// \param event the event parameters.
    void mouseDoubleClickEvent(QMouseEvent* event);

    /// Overridden QWidget method called when the mouse leaves the widget.
    /// \param event the event parameters.
    void leaveEvent(QEvent* event);

    /// Overridden QWidget method called when the mouse wheel is rolled.
    /// \param event the event parameters.
    void wheelEvent(QWheelEvent* event);

    /// Overridden QWidget method called when a key is pressed.
    /// \param event the event parameters.
    void keyPressEvent(QKeyEvent* event);

    /// Overridden QWidget method called when this widget is resized.
    /// \param event the event parameters.
    void resizeEvent(QResizeEvent* event);

    /// Recalculates the title width based on the current set of branches.
    void recalcTitleWidth();

signals:
    /// Signal emitted when a timeline branch is clicked.
    /// \param branch the timeline branch clicked.
    void branchClicked(acTimelineBranch* branch);

    /// Signal emitted when a timeline branch is double clicked.
    /// \param branch the timeline branch double clicked.
    void branchDoubleClicked(acTimelineBranch* branch);

    /// Signal emitted when a timeline item is clicked.
    /// \param item the timeline item clicked.
    void itemClicked(acTimelineItem* item);

    /// Signal emitted when a timeline item is double clicked.
    /// \param item the timeline item double clicked.
    void itemDoubleClicked(acTimelineItem* item);

    /// Signal emitted when the timeline's zoom factor changes.
    void zoomFactorChanged();

    /// Signal emitted when the timeline's offset changes.
    void offsetChanged();


signals:
    /// Called when the visibility filter changes.
    void VisibilityFilterChanged(QMap<QString, bool>& threadNameVisibilityMap);

private slots:

    /// Called when the range (start/range) of a branch changes.
    void branchRangeChanged();

    /// Called when a branch is added.
    /// \param subBranch the branch which was added.
    void branchAdded(acTimelineBranch* subBranch);

    /// Called when the text of a branch changes.
    void branchTextChanged();

    /// Called when a branch is folded or unfolded.
    void branchFoldedChanged();

    /// Called when a branch is selected or unselected.
    void branchSelectedChanged();

    /// Called when the horizontal scroll bar value changes.
    /// \param value the new value of the horizontal scroll bar.
    void hScrollBarValueChanged(int value);

    /// Called when the vertical scroll bar value changes.
    /// \param value the new value of the vertical scroll bar.
    void vScrollBarValueChanged(int value);

    /// Connects this timeline's slots to the specified branch signals (recursively).
    /// \param branch the branch whose signals we want to connect to.
    void connectSlotsToBranchSignals(acTimelineBranch* branch);

private:

    // private enum which defines the different ways of displaying an item on the timeline
    enum ItemDisplayOpt
    {
        /// Do not zoom into the item
        NA_ZOOM_ITEM_TYPE = 0,

        /// Zoom in far enough so that the block is wide enough to fit its text
        ZOOM_FIT_TEXT,

        /// Zoom in far enough so that the block take the entire width
        ZOOM_FULL_WIDTH,

        /// Do not change the zoom factor of the item, only make sure that it's centered horizontally
        HORIZONTAL_CENTER
    };

    /// Nested structure to hold information about a timeline marker.
    struct TimelineMarker
    {
        /// Construct/initialize a new instance of the TimelineMarker struct.
        TimelineMarker(quint64 timeStamp, QColor color);

        quint64 m_nTimeStamp;  ///< The timestamp of the marker.
        QColor m_color;        ///< The color of the marker.
    };

    /// Resets the row index for all branches.
    void resetRowIndex();

    /// Computes the pivot from the zoom pivot.
    void updateZoomPivotFromPivot();

    /// Computes the zoom pivot from the pivot.
    void updatePivotFromZoomPivot();

    /// Updates the Pivot from the specified mouse X coordinate.
    /// \param mousePosX the mouse X coordinate where the pivot should be located.
    /// \return true if the pivot was updated, false otherwise.
    bool updatePivotFromMousePosX(int mousePosX);

    /// Always updates visibility and geometry for both the horizontal and vertical scroll bars. Optionally calls updateHorizontalScrollBar or updateVerticalScrollBar.
    /// \param scrollBarsToUpdate flag indicating which scroll bars to update.  Can be none, horizontal, vertical or both.
    void updateScrollBars(QFlags<Qt::Orientation> scrollBarsToUpdate);

    /// Updates the horizontal scroll bar (min/max val, value, step size).
    void updateHorizontalScrollBar();

    /// Updates the vertical scroll bar (min/max val, value, step size).
    void updateVerticalScrollBar();

    /// Zooms into the selected area
    /// \param mouseUpX the X coordinate where the mouse button was released.
    void zoomToSelectedArea(int mouseUpX);

    /// Updates the geometry of the time grid based on whether or not the vertical scroll bar is displayed.
    void updateGridGeometry();

    /// Updates the grid times (full and visible) as well as the selected times.
    void updateGrid();

    /// Zooms the timeline the specified number of ticks.
    /// A tick is equal to a mouse wheel tick (120 ticks per notch).
    /// \param numTicks the number of ticks to zoom.  A positive number indicates zoom in. A negative number indicates zoom out.
    void userZoom(int numTicks);

    /// Clears all branches.
    void clearBranches();


    /// Shows a timeline item, optionally zooming in to the item
    /// \param item the timeline item to show
    /// \param zoomType flag indicating how the timeline should zoom to the specified item
    void ShowItem(acTimelineItem* item, ItemDisplayOpt zoomType);

    quint64          m_nStartTime;                  ///< The timestamp for the start of the timeline grid.  Defaults to zero.
    quint64          m_nFullRange;                  ///< The full range of the timeline grid.  Defaults to zero.
    quint64          m_nVisibleStartTime;           ///< The timestamp for the start of the visible portion of the timeline grid.  Defaults to zero.
    quint64          m_nVisibleRange;               ///< The range of the visible portion of the timeline grid.  Defaults to zero.
    double           m_dInvRange;                   ///< Inverse of full range.

    quint64          m_nRenderWidth;                ///< The full width of the timeline given the current row width and zoom factor (row width * zoom factor).
    int              m_nTitleWidth;                 ///< The width of the title section (the area containing the branch titles).

    double           m_dPivot;                      ///< Pivot represents the location of the vertical line as a fraction of the timeline screen width. Valid range is 0 to 1.
    double           m_dZoomPivot;                  ///< Zoom Pivot represents the location of the vertical line as a fraction of the full timeline width, taking into account the zoom factor. Valid range is 0 to 1.
    double           m_dStartSelectionPivot;        ///< The pivot point of the start of a selection (when ctrl-dragging to select a region).
    double           m_dEndSelectionPivot;          ///< The pivot point of the end of a selection (when ctrl-dragging to select a region).

    double           m_dZoomFactor;                 ///< The current zoom factor.
    double           m_dMaxZoom;                    ///< The max zoom factor.  Defaults to max int / 100.

    qint64           m_nOffset;                     ///< The current horizontal offset when zoomed in.
    int              m_nVOffset;                    ///< The current vertical offset when the total height of all branches exceeds the vertical available space.

    bool             m_bScaleHScrollbar;            ///< Flag indicating that the horizontal scroll bar is scaled.  Set to true when m_nRenderWidth exceeds max int.  Scaling is done to make sure value set to QScrollBar.sliderPosition doesn't overflow.
    double           m_dHScrollbarScaleValue;       ///< Scale value applied to horizontal scroll bar when it is scaled.  Set to (m_nRenderWidth / max int) * 2.  Scaling is done to make sure value set to QScrollBar.sliderPosition doesn't overflow.
    bool             m_bUpdatingHScrollBar;         ///< Flag indicating that the horizontal scroll bar is being updated.

    qint64           m_dLastUserZoomTime;           ///< Timestamp used when accumulating velocity of user zoom event (mouse wheel or keyboard).
    double           m_dUserZoomVelocity;           ///< Accumulated velocity of user zoom event (mouse wheel or keyboard).

    bool             m_bSelectDragging;             ///< Flag indicating that the user is ctrl-dragging to select a region.
    bool             m_bStartDrag;                  ///< Flag indicating that the user started a mouse drag operation.
    int              m_nStartDragX;                 ///< X coordinate of the start position of a mouse drag operation.
    QPoint           m_mouseLocation;               ///< Current position of the mouse cursor over the timeline.

    int              m_nBranchIndentation;          ///< Branch indentation size in pixels.  Defaults to 10.
    int              m_nDefaultBranchHeight;        ///< Default height of new branches added to this timeline. Defaults to 25.
    int              m_nTitleSectionLeftSpace;      ///< Margin at the left side of the branch title area.
    int              m_nTitleSectionRightSpace;     ///< Margin at the right side of the branch title area.
    int              m_nTimelineSectionRightSpace;  ///< Margin at the right side of the timeline control.
    bool             m_bGradientPainting;           ///< Flag indicating whether or not the timeline is painted with gradient colors.
    bool             m_bRoundedRectangles;          ///< Flag indicating whether or not the timeline is painted with rounded rectangles.
    bool             m_bPivotMouseTracking;         ///< Flag indicating whether or not the pivot marker tracks the mouse cursor.

    bool             m_bShowZoomHint;               ///< Flag indicating whether or not a tooltip appears to describe zooming ui interaction
    bool             m_showToolTip;                 ///< Flag indicating if tooltips are shown or not
    bool             m_shouldVScrollToEnd;          ///< Flag indicating if next time scroll update will cause auto scroll to end of V scroll
    acTimelineGrid*  m_pGrid;                       ///< The grid control associated with this timeline.

    acTimelineBranch* m_pSelectedBranch;            ///< The currently selected branch.
    acTimelineItem*   m_pSelectedItem;                ///< The currently selected item

    QSize             m_lastSizeWhenCacheWasCleared; ///< Contain the size of the timeline when the cache was recently cleared. Will be used to decide if the cache should be cleared again, or not.

protected:

    QList<acTimelineBranch*> m_subBranches;         ///< The list of top-level branches in the timeline.
    QList<TimelineMarker*>   m_markers;             ///< The list of markers that have been added to this timeline.

    bool m_shouldDisplayChildrenInParentBranch;     ///< Should the timeline items should be drawn on parent branch? (false by default)

    QScrollBar*      m_pHScrollBar;                 ///< The horizontal scroll bar.
    QScrollBar*      m_pVScrollBar;                 ///< The vertical scroll bar.
};

#endif // _QCTIMELINE_H_


