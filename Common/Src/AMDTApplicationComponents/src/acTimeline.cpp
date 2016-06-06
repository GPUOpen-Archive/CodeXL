//==================================================================================
// Copyright (c) 2011 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acTimeline.cpp
///
//==================================================================================

#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <QHelpEvent>
#include <QEvent>
#include <QPainter>
#include <QBoxLayout>
#include <QToolTip>
#include <QDateTime>
#include <QStyle>
#include <QLabel>
#include <QDialog>

#include <limits>
#include <cmath>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>

// Local:
#include <AMDTApplicationComponents/Include/acDisplay.h>
#include <AMDTApplicationComponents/Include/Timeline/acTimeline.h>
#include <Include/Timeline/acTimelineFiltersDialog.h>

#define AC_DEFAULT_TIMELINE_BRANCH_HEIGHT 25

acTimeline::acTimeline(QWidget* parent, Qt::WindowFlags flags) : QWidget(parent, flags),
    m_nStartTime(std::numeric_limits<quint64>::max()),
    m_nFullRange(0),
    m_nVisibleStartTime(m_nStartTime),
    m_nVisibleRange(0),
    m_dInvRange(0),
    m_nRenderWidth(0),
    m_nTitleWidth(0),
    m_dPivot(0.5f),
    m_dZoomPivot(0.5f),
    m_dStartSelectionPivot(-1.0f),
    m_dEndSelectionPivot(-1.0f),
    m_dZoomFactor(1.0f),
    m_dMaxZoom(std::numeric_limits<int>::max() / 100),  //  this number is divided by 100 to prevent overflow
    m_nOffset(0),
    m_nVOffset(0),
    m_bScaleHScrollbar(false),
    m_dHScrollbarScaleValue(0),
    m_bUpdatingHScrollBar(false),
    m_dLastUserZoomTime(0),
    m_dUserZoomVelocity(0),
    m_bSelectDragging(false),
    m_bStartDrag(false),
    m_nStartDragX(0),
    m_nBranchIndentation(10),
    m_nDefaultBranchHeight(AC_DEFAULT_TIMELINE_BRANCH_HEIGHT),
    m_nTitleSectionLeftSpace(4),
    m_nTitleSectionRightSpace(4),
    m_nTimelineSectionRightSpace(2),
    m_bGradientPainting(true),
    m_bRoundedRectangles(true),
    m_bPivotMouseTracking(true),
    m_bShowZoomHint(true),
    m_showToolTip(true),
    m_shouldVScrollToEnd(false),
    m_pGrid(nullptr),
    m_pSelectedBranch(nullptr),
    m_pSelectedItem(nullptr),
    m_lastSizeWhenCacheWasCleared(0, 0),
    m_shouldDisplayChildrenInParentBranch(false),
    m_pHScrollBar(nullptr),
    m_pVScrollBar(nullptr)
{
    setMouseTracking(true);

    m_pGrid = new(std::nothrow) acTimelineGrid(this);
    Q_ASSERT(m_pGrid != nullptr);

    if (m_pGrid != nullptr)
    {
        m_pGrid->setGeometry(0, 0, m_pGrid->width(), 40);
        m_pGrid->setGridLabel(tr("ms"));
        m_pGrid->setDurationHintLabel(tr("%1 ms"));
        m_pGrid->setScalingFactor(1000000);
        updateGrid();
    }

    m_pHScrollBar = new(std::nothrow) QScrollBar(Qt::Horizontal, this);
    Q_ASSERT(m_pHScrollBar != nullptr);

    if (m_pHScrollBar != nullptr)
    {
        connect(m_pHScrollBar, SIGNAL(valueChanged(int)),
                this, SLOT(hScrollBarValueChanged(int)));
    }

    m_pVScrollBar = new(std::nothrow) QScrollBar(Qt::Vertical, this);
    Q_ASSERT(m_pVScrollBar != nullptr);

    if (m_pVScrollBar != nullptr)
    {
        connect(m_pVScrollBar, SIGNAL(valueChanged(int)),
                this, SLOT(vScrollBarValueChanged(int)));
    }

    reset();
    setFocusPolicy(Qt::StrongFocus);

    // Adapt the branch height to the display DPI
    m_nDefaultBranchHeight = acScaleSignedPixelSizeToDisplayDPI(AC_DEFAULT_TIMELINE_BRANCH_HEIGHT);
}

acTimeline::~acTimeline()
{
    clearBranches();
    clearMarkers();

    delete m_pGrid;
    delete m_pHScrollBar;
    delete m_pVScrollBar;

    m_pGrid = nullptr;
    m_pHScrollBar = nullptr;
    m_pVScrollBar = nullptr;
}

void acTimeline::clearBranches()
{
    for (QList<acTimelineBranch*>::iterator i = m_subBranches.begin(); i != m_subBranches.end(); ++i)
    {
        delete(*i);
        (*i) = NULL;
    }

    m_subBranches.clear();
}

bool acTimeline::addBranch(acTimelineBranch* branch)
{
    // bail out if nullptr is passed in
    if (branch == nullptr)
    {
        return false;
    }

    // Draw / do not draw children on parent branch
    branch->SetDrawChildren(m_shouldDisplayChildrenInParentBranch);

    branch->setParentTimeline(this);

    // update timeline start time and range
    if (m_nStartTime > branch->startTime())
    {
        setStartTime(branch->startTime());
    }

    if (branch->endTime() - m_nStartTime > m_nFullRange)
    {
        setFullRange(branch->endTime() - m_nStartTime);
    }

    connectSlotsToBranchSignals(branch);

    m_subBranches.push_back(branch);

    recalcTitleWidth();
    updateScrollBars(QFlags<Qt::Orientation>(Qt::Vertical));
    resetRowIndex();
    update();

    return true;
}

void acTimeline::connectSlotsToBranchSignals(acTimelineBranch* branch)
{
    //add signals to slots
    connect(branch, SIGNAL(branchRangeChanged()),
            this, SLOT(branchRangeChanged()));
    connect(branch, SIGNAL(branchAdded(acTimelineBranch*)),
            this, SLOT(branchAdded(acTimelineBranch*)));
    connect(branch, SIGNAL(branchTextChanged()),
            this, SLOT(branchTextChanged()));
    connect(branch, SIGNAL(branchFoldedChanged()),
            this, SLOT(branchFoldedChanged()));
    connect(branch, SIGNAL(branchSelectedChanged()),
            this, SLOT(branchSelectedChanged()));

    for (int i = 0; i < branch->subBranchCount(); ++i)
    {
        connectSlotsToBranchSignals(branch->getSubBranch(i));
    }


}

int acTimeline::timelineSectionTopSpace() const
{
    int retVal = 0;

    if (m_pGrid != NULL)
    {
        if (m_pGrid->isVisible())
        {
            retVal = m_pGrid->height();
        }
    }

    return retVal;
}

void acTimeline::setTitleSectionLeftSpace(const int newTitleSectionLeftSpace)
{
    m_nTitleSectionLeftSpace = newTitleSectionLeftSpace;
    recalcTitleWidth();
}

void acTimeline::setTitleSectionRightSpace(const int newTitleSectionRightSpace)
{
    m_nTitleSectionRightSpace = newTitleSectionRightSpace;
    recalcTitleWidth();
}

void acTimeline::setTimelineSectionRightSpace(const int newTimelineSectionRightSpace)
{
    m_nTimelineSectionRightSpace = newTimelineSectionRightSpace;
}

void acTimeline::setSelectedBranch(acTimelineBranch* branch)
{
    if (m_pSelectedBranch != branch)
    {
        if (m_pSelectedBranch != nullptr)
        {
            m_pSelectedBranch->setSelected(false);
        }

        m_pSelectedBranch = branch;

        if (m_pSelectedBranch != nullptr)
        {
            m_pSelectedBranch->setSelected(true);
        }
    }
}

void acTimeline::setStartTime(const quint64 newStartTime)
{
    // TODO investigate why does setting the start time also affect the visible start time (should do this only if visible start time is now out range)
    m_nStartTime = m_nVisibleStartTime = newStartTime;
    updateGrid();
}

void acTimeline::setFullRange(const quint64 newFullRange)
{
    // TODO investigate why does setting the full range also affect the visible range (should do this only if visible range is now out full range)
    m_nFullRange = m_nVisibleRange = newFullRange;

    if (m_nFullRange != 0)
    {
        m_dInvRange = 1 / (double)m_nFullRange;
    }
    else
    {
        m_dInvRange = 0;
    }

    updateGrid();
}

acTimelineBranch* acTimeline::getBranchFromY(int y) const
{
    if (y >= timelineSectionTopSpace())
    {
        int adjustedY = y - timelineSectionTopSpace() + m_nVOffset;
        int totalHeight = 0;

        for (QList<acTimelineBranch*>::const_iterator i = m_subBranches.begin(); i != m_subBranches.end(); ++i)
        {
            if ((*i)->IsVisible() == true)
            {
                int branchHeight = (*i)->cumulativeHeight();

                if (totalHeight + branchHeight > adjustedY)
                {
                    return (*i)->getSubBranchFromY(adjustedY - totalHeight);
                }

                totalHeight += branchHeight;
            }
        }
    }

    return nullptr;
}

acTimelineBranch* acTimeline::getBranchFromText(const QString& branchText, bool partialMatch, bool shouldRecurse) const
{
    for (QList<acTimelineBranch*>::const_iterator i = m_subBranches.begin(); i != m_subBranches.end(); ++i)
    {
        acTimelineBranch* branch = (*i);
        QString thisBranchText = branch->text();

        if (partialMatch)
        {
            if (thisBranchText.contains(branchText))
            {
                return branch;
            }
        }
        else
        {
            if (thisBranchText == branchText)
            {
                return branch;
            }
        }

        if (shouldRecurse)
        {
            acTimelineBranch* subbranch = branch->getSubBranchFromText(branchText, partialMatch);

            if (subbranch != nullptr)
            {
                return subbranch;
            }
        }
    }

    return nullptr;
}

acTimelineItem* acTimeline::getTimelineItem(int x, int y) const
{
    acTimelineBranch* branch = getBranchFromY(y);

    if (branch != nullptr)
    {
        return branch->getTimelineItem(x, y);
    }

    return nullptr;
}

void acTimeline::ShowItem(acTimelineItem* item, ItemDisplayOpt itemDisplayType)
{
    if (item == nullptr)
    {
        return;
    }

    acTimelineBranch* parentBranch = item->parentBranch();

    if (parentBranch == nullptr)
    {
        return;
    }

    blockSignals(true);
    int localRowWidth = rowWidth();
    double invRowWidth = 1.0 / localRowWidth;

    // calculate block width to be used if zooming
    double blockWidth = (double)localRowWidth;

    if (itemDisplayType == ZOOM_FIT_TEXT)
    {
        // determine the width needed to display the test comfortably (20% larger than the width of the text)
        float newBlockWidth = fontMetrics().width(item->text()) * 1.2f;

        if (newBlockWidth < blockWidth)
        {
            blockWidth = newBlockWidth;
        }
    }

    quint64 itemStartTime = item->startTime();
    quint64 itemEndTime = item->endTime();
    quint64 itemLength = itemEndTime - itemStartTime;

    // the width of the item as a percent of the total width of the timeline
    double itemWidthPercent = (itemEndTime - itemStartTime) * m_dInvRange;

    // the start position of the item as a percent of the width of the timeline
    double itemStartPercent = (itemStartTime - m_nStartTime) * m_dInvRange;

    bool shouldChangeZoomFactor = ((itemDisplayType == ZOOM_FULL_WIDTH) || (itemDisplayType == ZOOM_FIT_TEXT));

    // if an item was reported to have no duration (end time == start time)
    // then don't zoom, just position the pivot to the item
    if (itemWidthPercent != 0 && shouldChangeZoomFactor)
    {
        // zoom into the blockWidth (which was set according to the zoom type)
        double zoomF = (blockWidth / itemWidthPercent) * invRowWidth;

        if (zoomF < 1.0001)
        {
            setZoomFactor(1.0);
            setPivot(itemStartPercent + ((itemWidthPercent * m_nRenderWidth * 0.5) * invRowWidth));
            return;
        }
        else
        {
            setZoomFactor(zoomF);
        }
    }

    // now adjust the offset
    qint64 localOffset = (quint64)(m_nRenderWidth * itemStartPercent);

    if (localOffset < 0)
    {
        localOffset = 0;
    }

    setOffset(localOffset);

    if (offset() != localOffset)
    {
        // offset wasn't set:
        // localOffset must have fallen out of range, we can't place item at beginning
        // move pivot line to item
        double pos = getXCoordOfTime(itemStartTime) + (itemWidthPercent * m_nRenderWidth * 0.5);
        setPivot(pos * invRowWidth);
    }
    else
    {
        // move pivot to the center of the block
        setPivot((itemWidthPercent * m_nRenderWidth * 0.5) * invRowWidth);
    }

    blockSignals(false);


    if ((itemDisplayType == ZOOM_FIT_TEXT) || (itemDisplayType == HORIZONTAL_CENTER))
    {

        // Calculating the new visible range for this item :
        // Item should be centered , so we compute new visible start time by reducing half of visible range and half of item length
        // from the item relative start time
        const quint64 visibleRangeTime = visibleRange();
        const quint64 startTimeOnGrid = itemStartTime - startTime();
        const quint64 halfVisibleRange = visibleRangeTime / 2 - itemLength / 2;
        const quint64 newVisibleStartTime = startTimeOnGrid > halfVisibleRange ? startTimeOnGrid - halfVisibleRange : 0;
        SetVisibleRange(newVisibleStartTime, visibleRangeTime);

    }

    // make sure the branch is in view
    showBranch(parentBranch);

    // make sure the branch is selected
    setSelectedBranch(parentBranch);
}

void acTimeline::ShowItem(acTimelineItem* item)
{
    ShowItem(item, NA_ZOOM_ITEM_TYPE);
}

void acTimeline::DisplayItemAtHorizontalCenter(acTimelineItem* pItemToCenter, bool shouldSelect)
{
    if (shouldSelect)
    {
        if (pItemToCenter != nullptr)
        {
            pItemToCenter->SetSelected(true);

            if ((m_pSelectedItem != nullptr) && (m_pSelectedItem != pItemToCenter))
            {
                m_pSelectedItem->ClearSelection();
            }

            m_pSelectedItem = pItemToCenter;
        }
    }

    ShowItem(pItemToCenter, HORIZONTAL_CENTER);
}

void acTimeline::ZoomToItem(acTimelineItem* pZoomedItem, bool shouldSelect)
{
    if (shouldSelect)
    {
        if (pZoomedItem != nullptr)
        {
            pZoomedItem->SetSelected(true);

            if ((m_pSelectedItem != nullptr) && (m_pSelectedItem != pZoomedItem))
            {
                m_pSelectedItem->ClearSelection();
            }

            m_pSelectedItem = pZoomedItem;
        }
    }

    ShowItem(pZoomedItem, ZOOM_FIT_TEXT);
}

void acTimeline::zoomToItemFull(acTimelineItem* item)
{
    ShowItem(item, ZOOM_FULL_WIDTH);
}

void acTimeline::showBranch(acTimelineBranch* branch)
{
    if (m_pVScrollBar->isVisible())
    {
        int branchTop = branch->top();
        int timelineTopSpace = timelineSectionTopSpace();
        int vscrollBarVal = branchTop + m_nVOffset - timelineTopSpace;
        vscrollBarVal = vscrollBarVal < 0 ? 0 : vscrollBarVal;
        vscrollBarVal = vscrollBarVal > m_pVScrollBar->maximum() ? m_pVScrollBar->maximum() : vscrollBarVal;
        m_pVScrollBar->setValue(vscrollBarVal);
    }
}

int acTimeline::getXCoordOfTime(quint64 time) const
{
    int val = (int)((((double)(time - m_nStartTime) * invRange()) * m_nRenderWidth) - m_nOffset);
    return val;
}

double acTimeline::getFractionOfFullTimeline(double fractionOfVisiblePortion)
{
    double inverseZoomFactor = 1.0 / m_dZoomFactor;
    return (m_nOffset / (double)m_nRenderWidth) + (fractionOfVisiblePortion * inverseZoomFactor);
}

void acTimeline::SetGridLabelsPrecision(unsigned int precision)
{
    if (m_pGrid != nullptr)
    {
        m_pGrid->SetGridLabelsPrecision(precision);
    }
}

void acTimeline::recalcTitleWidth()
{
    int originalWidth = ACTIMELINEGRID_DefaultGridLabelSpace;

    if (m_pGrid != nullptr)
    {
        originalWidth = m_pGrid->gridLabelSpace();
    }

    m_nTitleWidth = originalWidth;
    int branchTitleWidth;

    for (QList<acTimelineBranch*>::const_iterator i = m_subBranches.begin(); i != m_subBranches.end(); ++i)
    {
        branchTitleWidth = (*i)->titleWidth();

        if (branchTitleWidth > m_nTitleWidth)
        {
            m_nTitleWidth = branchTitleWidth;
        }
    }

    if (m_nTitleWidth > originalWidth)
    {
        m_nTitleWidth += m_nTitleSectionLeftSpace + m_nTitleSectionRightSpace + ACTIMELINE_TreeMarkSpace;

        if (m_pGrid != nullptr)
        {
            m_pGrid->setGridLabelSpace(m_nTitleWidth);
        }

        if (m_pHScrollBar != nullptr)
        {
            m_pHScrollBar->setGeometry(m_nTitleWidth, m_pHScrollBar->y(), rowWidth(), m_pHScrollBar->height());
        }
    }

    m_nRenderWidth = (quint64)(rowWidth() * m_dZoomFactor);
}

int acTimeline::rowWidth() const
{
    int rowWidth = width();
    int nonRowSpace = m_nTitleWidth + m_nTimelineSectionRightSpace;

    if (m_pVScrollBar != nullptr && m_pVScrollBar->isVisible())
    {
        nonRowSpace += m_pVScrollBar->width();
    }

    if (rowWidth > nonRowSpace)
    {
        rowWidth -= nonRowSpace;
    }
    else
    {
        rowWidth = 0;
    }

    return rowWidth;
}

int acTimeline::cumulativeBranchHeight()
{
    int totalBranchHeight = 0;

    for (QList<acTimelineBranch*>::const_iterator i = m_subBranches.begin(); i != m_subBranches.end(); ++i)
    {
        if ((*i)->IsVisible() == true)
        {
            totalBranchHeight += (*i)->cumulativeHeight();
        }
    }

    return totalBranchHeight;
}

bool acTimeline::event(QEvent* event)
{
    if (event->type() == QEvent::ToolTip)
    {
        if (m_showToolTip)
        {
            QHelpEvent* helpEvent = dynamic_cast<QHelpEvent*>(event);
            acTimelineItem* item = getTimelineItem(helpEvent->x(), helpEvent->y());

            if (item != NULL)
            {
                QToolTip::showText(helpEvent->globalPos(), item->tooltipText());
            }
            else if (m_bShowZoomHint)
            {
                QToolTip::showText(helpEvent->globalPos(), tr("Use the mouse wheel or the plus/minus keys to zoom in and out"));
            }
            else
            {
                QToolTip::hideText();
                event->ignore();
            }
        }

        return true;
    }

    return QWidget::event(event);
}

void acTimeline::paintEvent(QPaintEvent* /* event */)
{
    int rw = rowWidth();
    int curHeight = height();
    int topSpace = timelineSectionTopSpace();

    int yOffset = topSpace - m_nVOffset;
    QPainter painter(this);

    int totalHeight = curHeight - topSpace;

    if (m_pHScrollBar != NULL && m_pHScrollBar->isVisible())
    {
        totalHeight -= m_pHScrollBar->height();
    }

    painter.setClipRect(0, topSpace, width(), totalHeight);

    // draw branches
    for (QList<acTimelineBranch*>::const_iterator i = m_subBranches.begin(); i != m_subBranches.end(); ++i)
    {
        if ((*i)->IsVisible() == true)
        {
            (*i)->draw(painter, yOffset);
        }
    }

    // draw markers
    for (QList<TimelineMarker*>::const_iterator i = m_markers.begin(); i != m_markers.end(); ++i)
    {
        TimelineMarker* marker = *i;

        if (marker->m_nTimeStamp >= m_nVisibleStartTime && marker->m_nTimeStamp <= m_nVisibleStartTime + m_nVisibleRange)
        {
            QPen pen = painter.pen();
            pen.setColor(marker->m_color);
            painter.setPen(pen);
            int xPos = m_nTitleWidth + getXCoordOfTime(marker->m_nTimeStamp);
            painter.drawLine(QPoint(xPos, topSpace), QPoint(xPos, curHeight));
        }
    }

    // draw pivot line
    if (m_showToolTip)
    {
        if (m_dPivot >= 0 && rw > 0)
        {
            QPen pen = painter.pen();
            pen.setColor(Qt::black);
            painter.setPen(pen);
            int xPos = m_nTitleWidth + (int)(rw * m_dPivot);
            painter.drawLine(QPoint(xPos, topSpace), QPoint(xPos, curHeight));
        }
    }

    // draw drag highlight rectangle
    if (m_bStartDrag && m_bSelectDragging)
    {
        int mouseX = m_mouseLocation.x();
        int localTitleWidth = m_nTitleWidth;

        QRect dragAreaRect;

        if (mouseX > m_nStartDragX)
        {
            dragAreaRect.setX(m_nStartDragX);
            dragAreaRect.setWidth(mouseX - m_nStartDragX);
        }
        else if (mouseX < localTitleWidth)
        {
            dragAreaRect.setX(localTitleWidth);
            dragAreaRect.setWidth(m_nStartDragX - localTitleWidth);
        }
        else
        {
            dragAreaRect.setX(mouseX);
            dragAreaRect.setWidth(m_nStartDragX - mouseX);
        }

        dragAreaRect.setY(topSpace);
        dragAreaRect.setHeight(height() - dragAreaRect.y());
        QColor dragColor = palette().color(QPalette::Highlight);
        dragColor.setAlpha(127);
        painter.fillRect(dragAreaRect, dragColor);
    }
}

void acTimeline::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        int mouseX = event->x();

        if (mouseX >= m_nTitleWidth)
        {
            m_bStartDrag = true;
            m_nStartDragX = mouseX;
            m_dStartSelectionPivot = (mouseX - m_nTitleWidth) / (double)rowWidth();
            updateGrid();
        }
    }
}

void acTimeline::mouseReleaseEvent(QMouseEvent* event)
{
    int mouseX = event->x();
    int mouseY = event->y();

    if (m_bStartDrag && event->modifiers() & Qt::ControlModifier)
    {
        zoomToSelectedArea(mouseX);
    }
    else
    {
        if (!m_bPivotMouseTracking && m_bStartDrag && mouseX == m_nStartDragX)
        {
            updatePivotFromMousePosX(mouseX);
            updateGrid();
        }

        acTimelineItem* item = getTimelineItem(mouseX, mouseY);

        acTimelineBranch* branch;

        if (item != NULL)
        {
            branch = item->parentBranch();
        }
        else
        {
            branch = getBranchFromY(mouseY);
        }

        setSelectedBranch(branch);

        if (event->button() == Qt::LeftButton)
        {
            if (m_pSelectedBranch != NULL)
            {
                QRect treeMarkRect = m_pSelectedBranch->treeMarkRect();

                if (treeMarkRect.isValid())
                {
                    // give the user some wiggle room -- allow clicking within two pixels on any side of the plus/minus sign
                    treeMarkRect.adjust(2, 2, 2, 2);

                    if (treeMarkRect.contains(mouseX, mouseY))
                    {
                        m_pSelectedBranch->setFolded(!m_pSelectedBranch->isFolded());
                    }
                }

                emit branchClicked(m_pSelectedBranch);

                if (item != NULL)
                {
                    // Clear the selection of the last item selected
                    if (m_pSelectedItem != nullptr)
                    {
                        m_pSelectedItem->ClearSelection();
                    }

                    // Toggle the clicked item selection flag
                    emit itemClicked(item);
                    item->SetSelected(!item->IsSelected());

                    // Set or clear the selected item pointer
                    if (item->IsSelected())
                    {
                        m_pSelectedItem = item;
                    }
                    else
                    {
                        m_pSelectedItem = nullptr;
                    }
                }
            }

            event->accept();
            update();
        }
    }

    setCursor(Qt::ArrowCursor); //restore cursor from mouse panning
    m_bStartDrag = false;
    m_bSelectDragging = false;
}

void acTimeline::mouseMoveEvent(QMouseEvent* event)
{
    int mouseX = event->x();
    int mouseY = event->y();

    int lastX = m_mouseLocation.x();

    m_mouseLocation.setX(mouseX);
    m_mouseLocation.setY(mouseY);

    if (m_bStartDrag)
    {
        m_bSelectDragging = (event->modifiers() & Qt::ControlModifier) != 0;

        if (m_bSelectDragging)
        {
            double endMousePos = mouseX - m_nTitleWidth;
            int localRowWidth = rowWidth();

            if (endMousePos > localRowWidth)
            {
                endMousePos = localRowWidth;
            }
            else if (endMousePos < 0)
            {
                endMousePos = 0;
            }

            m_dEndSelectionPivot = endMousePos / (double)localRowWidth;
            updateGrid();
            update();
        }
        else
        {
            m_dStartSelectionPivot = -1.0;
            m_dEndSelectionPivot = -1.0;
            int mouseMoveDistance = lastX - mouseX;

            if (mouseMoveDistance != 0)
            {
                if (m_dZoomFactor > 1.0 && !m_bSelectDragging)
                {
                    setCursor(Qt::SizeHorCursor); //TODO:  temp cursor for mouse panning

                    setOffset(m_nOffset + mouseMoveDistance);
                }

                updateGrid();
                update();
            }
        }
    }
    else if (m_bPivotMouseTracking)
    {
        if (updatePivotFromMousePosX(mouseX))
        {
            m_dStartSelectionPivot = -1.0;
            m_dEndSelectionPivot = -1.0;
            updateGrid();
            update();
        }
    }
}

void acTimeline::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        int mouseX = event->x();
        int mouseY = event->y();
        acTimelineItem* item = getTimelineItem(mouseX, mouseY);

        if (item != NULL)
        {
            emit itemDoubleClicked(item);
        }
        else
        {
            acTimelineBranch* branch = getBranchFromY(mouseY);

            if (branch != NULL)
            {
                QRect treeMarkRect = m_pSelectedBranch->treeMarkRect();
                bool emitSignal = true;

                if (treeMarkRect.isValid())
                {
                    // give the user some wiggle room -- allow clicking within two pixels on any side of the plus/minus sign
                    treeMarkRect.adjust(2, 2, 2, 2);

                    // don't emit the signal is the user double-clicked the tree expand/collapse control
                    if (treeMarkRect.contains(mouseX, mouseY))
                    {
                        emitSignal = false;
                    }
                }

                if (emitSignal)
                {
                    emit branchDoubleClicked(branch);
                }
            }
        }
    }
}

void acTimeline::leaveEvent(QEvent* /* event */)
{
    // clear the mouse location when the mouse leaves the control
    m_mouseLocation.setX(-1);
    m_mouseLocation.setY(-1);
    updateGrid();
    update();
}

void acTimeline::userZoom(int numTicks)
{
    m_bShowZoomHint = false;
    double zoomFactor = m_dZoomFactor;

    qint64 curMsecs = QDateTime::currentDateTime().toMSecsSinceEpoch();

    bool positiveVelocity = m_dUserZoomVelocity > 0;
    bool positiveAcceleration = numTicks > 0;

    // scale initial velocity to zoomFactor
    double initialVelocity = numTicks / 1000.0f * m_dZoomFactor;
    double constantAcceleration = numTicks / 200.0f;

    if (abs(curMsecs - m_dLastUserZoomTime) < 1000 && positiveVelocity == positiveAcceleration)
    {
        // accumulate velocity
        m_dUserZoomVelocity += constantAcceleration;
    }
    else
    {
        // reset velocity
        m_dUserZoomVelocity = constantAcceleration;
    }

    m_dLastUserZoomTime = curMsecs;

    zoomFactor += m_dUserZoomVelocity + initialVelocity;

    if (zoomFactor < 1)
    {
        zoomFactor = 1;
    }

    setZoomFactor(zoomFactor);
}

void acTimeline::wheelEvent(QWheelEvent* event)
{
    // don't zoom if the mouse cursor is over a visible scroll bar.  Qt has built-in support for scrolling using the wheel when the mouse is over a scrollbar.
    if (m_pHScrollBar != NULL && m_pHScrollBar->isVisible() && childAt(event->pos()) == m_pHScrollBar)
    {
        return;
    }

    if (m_pVScrollBar != NULL && m_pVScrollBar->isVisible() && childAt(event->pos()) == m_pVScrollBar)
    {
        return;
    }

    if (event->orientation() == Qt::Vertical)
    {
        userZoom(event->delta());
    }
}

void acTimeline::keyPressEvent(QKeyEvent* event)
{
    int zoomScale;
    int hScrollScale;
    int vScrollScale;

    // scroll or zoom more if CTRL key is pressed
    if (event->modifiers() & Qt::ControlModifier)
    {
        zoomScale = 600;
        hScrollScale = m_pHScrollBar->pageStep();
        vScrollScale = m_pVScrollBar->pageStep();
    }
    else
    {
        zoomScale = 120;
        hScrollScale = m_pHScrollBar->singleStep();
        vScrollScale = m_pVScrollBar->singleStep();
    }

    switch (event->key())
    {
        case Qt::Key_Plus:
            userZoom(zoomScale);
            break;

        case Qt::Key_Minus:
            userZoom(-zoomScale);
            break;

        case Qt::Key_Left:
            setOffset(m_nOffset - hScrollScale);
            break;

        case Qt::Key_Right:
            setOffset(m_nOffset + hScrollScale);
            break;

        case Qt::Key_Up:
            setVOffset(m_nVOffset - vScrollScale);
            break;

        case Qt::Key_Down:
            setVOffset(m_nVOffset + vScrollScale);
            break;

        default:
            QWidget::keyPressEvent(event);
    }

    // keep the focus on this widget after all the emits on the way.
    // We had it in the first place if we got here
    setFocus();
}

void acTimeline::resizeEvent(QResizeEvent* pEvent)
{
    if ((pEvent != NULL) && isVisible())
    {
        // Get the new size:
        QSize newSize = pEvent->size();

        // When the timeline is being hidden by its parent widget (tab widget, for example), the resize event is being called by Qt.
        // In this case, we do not want to reset the zoom setting of the timeline. In this case, the timeline gets minimal height (usually 1):
        if (newSize.height() > 10)
        {
            // Clear the draw cache only if the size had changed from the last time it was cleared:
            if (m_lastSizeWhenCacheWasCleared != newSize)
            {
                for (QList<acTimelineBranch*>::const_iterator i = m_subBranches.begin(); i != m_subBranches.end(); ++i)
                {
                    (*i)->clearDrawCache(true);
                }
            }

            // Set the current size as the cache clear size:
            m_lastSizeWhenCacheWasCleared = newSize;
        }
        
        // the first time we get a resize event we can set the v scroll bar info if needed
        if (m_shouldVScrollToEnd)
        {
            if (m_pVScrollBar != nullptr && m_pVScrollBar->isVisible())
            {
                m_nVOffset = m_pVScrollBar->maximum();
            }
            m_shouldVScrollToEnd = false;
        }

    }

    updateScrollBars(QFlags<Qt::Orientation>(Qt::Vertical | Qt::Horizontal));
    updateGridGeometry();
    updateGrid();
    update();

    QWidget::resizeEvent(pEvent);
}

void acTimeline::setZoomFactor(const double newZoomFactor)
{
    double localZoomFactor = newZoomFactor;

    if (localZoomFactor > m_dMaxZoom)
    {
        localZoomFactor = m_dMaxZoom;
    }

    if (m_dZoomFactor != localZoomFactor)
    {
        m_dZoomFactor = localZoomFactor;

        m_nRenderWidth = (quint64)(rowWidth() * m_dZoomFactor);

        if (m_nRenderWidth > (quint64)std::numeric_limits<int>::max())
        {
            m_bScaleHScrollbar = true;
            m_dHScrollbarScaleValue = (m_nRenderWidth / (double)std::numeric_limits<int>::max()) * 2.0;

        }
        else
        {
            m_bScaleHScrollbar = false;
        }

        double inverseZoomFactor = 1 / m_dZoomFactor;

        setOffset((qint64)((m_dZoomPivot - (m_dPivot * inverseZoomFactor)) * m_nRenderWidth));
        m_nVisibleRange = (quint64)(m_nFullRange * inverseZoomFactor);

        emit zoomFactorChanged();

        updateGrid();
        update();
    }
}

void acTimeline::setOffset(const qint64 newOffset)
{
    qint64 tempOffset = newOffset;
    int localRowWidth = rowWidth();

    if (m_nRenderWidth - tempOffset < (quint64)localRowWidth)
    {
        tempOffset = m_nRenderWidth - localRowWidth;
    }

    if (tempOffset < 0)
    {
        tempOffset = 0;
    }

    if (m_nOffset != tempOffset)
    {
        m_nOffset = tempOffset;

        updatePivotFromZoomPivot();
        updateScrollBars(QFlags<Qt::Orientation>(Qt::Horizontal));

        quint64 offset = 0;

        if (m_nOffset > 0 && m_nRenderWidth > 0 && m_nFullRange > 0)
        {
            offset = (quint64)(((double)m_nOffset / (double)m_nRenderWidth) * m_nFullRange);
        }

        m_nVisibleStartTime = m_nStartTime + offset;

        emit offsetChanged();

        updateGrid();
        update();
    }
}

void acTimeline::setVOffset(const int newVOffset)
{
    int tempOffset = newVOffset;

    if (tempOffset < 0)
    {
        tempOffset = 0;
    }

    if (m_pVScrollBar != NULL && m_pVScrollBar->isVisible())
    {
        int scrollMax = m_pVScrollBar->maximum();

        if (tempOffset > scrollMax)
        {
            tempOffset = scrollMax;
        }
    }
    else
    {
        tempOffset = 0;
    }

    if (m_nVOffset != tempOffset)
    {
        m_nVOffset = tempOffset;
        updateScrollBars(QFlags<Qt::Orientation>(Qt::Vertical));
        update();
    }
}

void acTimeline::setPivot(const double newPivot)
{
    m_dPivot = newPivot;
    updateZoomPivotFromPivot();
}

void acTimeline::setZoomPivot(const double newZoomPivot)
{
    m_dZoomPivot = newZoomPivot;
    updatePivotFromZoomPivot();
}

void acTimeline::updateZoomPivotFromPivot()
{
    double ratio = 1.0f / m_dZoomFactor;
    m_dZoomPivot = (m_nOffset / (double)m_nRenderWidth) + (m_dPivot * ratio);
    updateGrid();
    update();
}

void acTimeline::updatePivotFromZoomPivot()
{
    double ratio = 1.0f / m_dZoomFactor;
    m_dPivot = (m_dZoomPivot - (m_nOffset / (double)m_nRenderWidth)) / ratio;
    updateGrid();
    update();
}

bool acTimeline::updatePivotFromMousePosX(int mousePosX)
{
    mousePosX -= m_nTitleWidth;
    int curRowWidth = rowWidth();

    if (mousePosX < 0 || mousePosX > curRowWidth)
    {
        return false;
    }

    // Set vertical line position, update zoomPivot
    setPivot((double)mousePosX / (double)curRowWidth);
    return true;
}

void acTimeline::updateHorizontalScrollBar()
{
    if (m_pHScrollBar != NULL)
    {
        m_bUpdatingHScrollBar = true;
        int minHScrollVal = 0;
        int maxHScrollVal = 0;
        int newScrollVal = 0;
        int localRowWidth = rowWidth();

        if (m_bScaleHScrollbar)
        {
            maxHScrollVal = (int)((m_nRenderWidth - localRowWidth) / m_dHScrollbarScaleValue);
            newScrollVal = (int)(m_nOffset / m_dHScrollbarScaleValue);
        }
        else
        {
            maxHScrollVal = (int)(m_nRenderWidth - localRowWidth);
            newScrollVal = (int)m_nOffset;
        }

        if (newScrollVal < minHScrollVal)
        {
            newScrollVal = minHScrollVal;
        }

        if (newScrollVal > maxHScrollVal)
        {
            newScrollVal = maxHScrollVal;
        }

        int singleStepSize = localRowWidth / 2;

        if (singleStepSize < 1)
        {
            singleStepSize = 1;
        }

        int pageStepSize = localRowWidth;

        if (pageStepSize < 1)
        {
            pageStepSize = 1;
        }

        bool wasNotVisible = (m_pHScrollBar->minimum() == m_pHScrollBar->maximum());

        m_pHScrollBar->setSingleStep(singleStepSize);
        m_pHScrollBar->setPageStep(pageStepSize);
        m_pHScrollBar->setRange(minHScrollVal, maxHScrollVal);
        m_pHScrollBar->setValue(newScrollVal);

        // if the scroll bar was not visible before and now it is, there is a need to update the vertical scrollbar
        if (wasNotVisible && minHScrollVal < maxHScrollVal)
        {
            updateVerticalScrollBar();
        }

        m_bUpdatingHScrollBar = false;
    }
}

void acTimeline::updateVerticalScrollBar()
{
    if (m_pVScrollBar != NULL)
    {
        int totalDrawHeight = cumulativeBranchHeight();
        int availableSpace = height() - timelineSectionTopSpace();

        if (totalDrawHeight > availableSpace && availableSpace > 0)
        {
            int maxVScrollVal = totalDrawHeight - availableSpace;

            if (m_pHScrollBar != nullptr && m_pHScrollBar->isVisible())
            {
                maxVScrollVal += m_pHScrollBar->height();
            }

            m_pVScrollBar->setPageStep(availableSpace);
            m_pVScrollBar->setSingleStep(m_nDefaultBranchHeight);
            m_pVScrollBar->setMaximum(maxVScrollVal);
            m_pVScrollBar->setValue(m_nVOffset);
        }
        else
        {
            m_pVScrollBar->setValue(0);
        }
    }
}

void acTimeline::zoomToSelectedArea(int mouseUpX)
{
    m_dStartSelectionPivot = -1.0;
    m_dEndSelectionPivot = -1.0;
    int start, end;
    int localRowWidth = rowWidth();

    if (mouseUpX > m_nStartDragX)
    {
        if (mouseUpX > m_nTitleWidth + localRowWidth)
        {
            mouseUpX = m_nTitleWidth + localRowWidth;
        }

        start = m_nStartDragX;
        end = mouseUpX;
    }
    else
    {
        if (mouseUpX < m_nTitleWidth)
        {
            mouseUpX = m_nTitleWidth;
        }

        start = mouseUpX;
        end = m_nStartDragX;
    }

    start -= m_nTitleWidth;
    end -= m_nTitleWidth;

    if (start >= end || start < 0 || end < 0)
    {
        return;
    }

    double inverseRowWidth = 1.0 / localRowWidth;
    double fractionStart = (double)start * inverseRowWidth;
    double fractionEnd = (double)end * inverseRowWidth;

    double timelineFractionStart = getFractionOfFullTimeline(fractionStart);
    double timelineFractionEnd = getFractionOfFullTimeline(fractionEnd);

    // compute zoom factor
    double selectedRange = timelineFractionEnd - timelineFractionStart;
    double zoomFactor = 1.0f / selectedRange;

    if (zoomFactor > m_dMaxZoom)
    {
        zoomFactor = m_dMaxZoom;
    }

    setZoomFactor(zoomFactor);
    setOffset((qint64)(m_nRenderWidth * timelineFractionStart));
    setPivot(0.5f);
}

bool acTimeline::addMarker(quint64 timeStamp, QColor color)
{
    if (timeStamp < m_nStartTime)
    {
        setStartTime(timeStamp);
    }

    if (timeStamp > m_nStartTime + m_nFullRange)
    {
        setFullRange(timeStamp - m_nStartTime);
    }

    TimelineMarker* marker = new(std::nothrow) TimelineMarker(timeStamp, color);

    if (marker == nullptr)
    {
        return false;
    }

    m_markers.append(marker);
    update();
    return true;
}

bool acTimeline::removeMarker(quint64 timeStamp)
{
    for (int i = 0; i < m_markers.count(); i++)
    {
        if (m_markers[i]->m_nTimeStamp == timeStamp)
        {
            TimelineMarker* marker = m_markers.takeAt(i);
            delete marker;
            marker = nullptr;
            update();
            return true;
        }
    }

    return false;
}

bool acTimeline::clearMarkers()
{
    for (int i = 0; i < m_markers.count(); i++)
    {
        TimelineMarker* marker = m_markers.takeAt(i);
        delete marker;
        marker = nullptr;
    }

    m_markers.clear();
    update();
    return true;
}

void acTimeline::reset()
{
    m_nStartTime = std::numeric_limits<quint64>::max();
    m_nFullRange = 0;
    m_nVisibleStartTime = m_nStartTime;
    m_nVisibleRange = 0;
    m_dInvRange = 0;

    m_nTitleWidth = 0;

    m_dPivot = 0.5f;
    m_dZoomPivot = 0.5f;
    m_dStartSelectionPivot = -1.0;
    m_dEndSelectionPivot = -1.0;

    m_bScaleHScrollbar = false;
    m_dHScrollbarScaleValue = 0;

    m_nVOffset = 0;

    m_dLastUserZoomTime = 0;
    m_dUserZoomVelocity = 0;
    m_nOffset = 0;
    m_dZoomFactor = 1.0f;

    m_bSelectDragging = false;
    m_bStartDrag = false;
    m_nStartDragX = 0;
    m_mouseLocation.setX(0);
    m_mouseLocation.setY(0);

    updateGrid();
    recalcTitleWidth();

    updateScrollBars(QFlags<Qt::Orientation>());

    m_pSelectedBranch = nullptr;

    clearMarkers();
    clearBranches();
}

void acTimeline::branchRangeChanged()
{
    // New item added which caused range and start time to change
    // Update Range and StartTime

    acTimelineBranch* branch = qobject_cast<acTimelineBranch*>(sender());
    Q_ASSERT(branch != nullptr);

    if (branch == nullptr)
    {
        return;
    }

    if (m_nStartTime > branch->startTime())
    {
        setStartTime(branch->startTime());
        updateGrid();
    }

    // Update Range
    if (branch->endTime() - m_nStartTime > m_nFullRange)
    {
        setFullRange(branch->endTime() - m_nStartTime);
        updateGrid();
    }

    /*
       this.BeginUpdate();
       this.ZoomFactor = 1;
       this.Offset = 0;
       this.RenderWidth = this.RowWidth;
       this.Pivot = 0.5;

       this.EndUpdate();
    */
}

void acTimeline::branchAdded(acTimelineBranch* subBranch)
{
    connectSlotsToBranchSignals(subBranch);
    updateScrollBars(QFlags<Qt::Orientation>(Qt::Vertical));
    resetRowIndex();
    recalcTitleWidth();
    update();
}

void acTimeline::branchTextChanged()
{
    recalcTitleWidth();
}

void acTimeline::branchFoldedChanged()
{
    updateScrollBars(QFlags<Qt::Orientation>(Qt::Vertical));
}

void acTimeline::branchSelectedChanged()
{
    setSelectedBranch(qobject_cast<acTimelineBranch*>(sender()));
}

void acTimeline::hScrollBarValueChanged(int value)
{
    if (!m_bUpdatingHScrollBar)
    {
        if (std::abs(m_dZoomFactor - 1.0) < 0.0001)
        {
            setOffset(0);
        }
        else
        {
            if (m_bScaleHScrollbar)
            {
                setOffset((qint64)(value * m_dHScrollbarScaleValue));
            }
            else
            {
                setOffset(value);
            }
        }
    }
}

void acTimeline::vScrollBarValueChanged(int value)
{
    setVOffset(value);
}

void acTimeline::resetRowIndex()
{
    unsigned int index = 0;

    for (QList<acTimelineBranch*>::const_iterator i = m_subBranches.begin(); i != m_subBranches.end(); ++i)
    {
        (*i)->resetRowIndex(index);
    }
}

void acTimeline::updateGrid()
{
    if (m_pGrid != nullptr)
    {
        m_pGrid->setStartTime(0);
        m_pGrid->setFullRange(m_nFullRange);
        quint64 adjustedStartTime = m_nVisibleStartTime - m_nStartTime;
        m_pGrid->setVisibleStartTime(adjustedStartTime);
        m_pGrid->setVisibleRange(m_nVisibleRange);

        double startPivot;
        double endPivot;

        if (m_dEndSelectionPivot != -1.0)
        {
            startPivot = m_dStartSelectionPivot;
            endPivot = m_dEndSelectionPivot;
        }
        else
        {
            startPivot = m_dPivot;
            endPivot = m_dPivot;
        }

        quint64 time = adjustedStartTime + (quint64)(startPivot * m_nVisibleRange);
        m_pGrid->setSelectedTime(time);

        time = adjustedStartTime + (quint64)(endPivot * m_nVisibleRange);
        m_pGrid->setEndSelectedTime(time);

        m_pGrid->update();
    }
}

void acTimeline::updateGridGeometry()
{
    if (m_pGrid != nullptr)
    {
        int vScrollBarSpace = 0;

        if (m_pVScrollBar != nullptr && m_pVScrollBar->isVisible())
        {
            vScrollBarSpace = m_pVScrollBar->width();
        }

        m_pGrid->setGeometry(m_pGrid->x(), m_pGrid->y(), width() - m_nTimelineSectionRightSpace - vScrollBarSpace, m_pGrid->height());
    }
}

void acTimeline::updateScrollBars(QFlags<Qt::Orientation> scrollBarsToUpdate)
{
    if (m_pHScrollBar != nullptr && m_pVScrollBar != nullptr)
    {
        quint64 oldRenderWidth = m_nRenderWidth;

        if (oldRenderWidth < 1)
        {
            oldRenderWidth = 1;
        }

        bool hScrollBarVis = m_dZoomFactor > 1.0;
        m_pHScrollBar->setVisible(hScrollBarVis);

        int scrollHeight = style()->pixelMetric(QStyle::PM_ScrollBarExtent);

        int totalDrawHeight = cumulativeBranchHeight();
        int topSpace = timelineSectionTopSpace();
        int availableSpace = height() - topSpace;

        if (hScrollBarVis)
        {
            availableSpace -= scrollHeight;
        }

        bool vScrollBarWasVis = m_pVScrollBar->isVisible();
        bool vScrollBarVis = totalDrawHeight > availableSpace && availableSpace > 0;

        m_pVScrollBar->setVisible(vScrollBarVis);

        if (vScrollBarVis != vScrollBarWasVis)
        {
            recalcTitleWidth();
            updateGridGeometry();
        }

        m_pVScrollBar->setGeometry(width() - scrollHeight, topSpace, scrollHeight, availableSpace);
        m_pHScrollBar->setGeometry(m_nTitleWidth, height() - scrollHeight, rowWidth(), scrollHeight);

        m_nRenderWidth = (quint64)(rowWidth() * m_dZoomFactor);
        double factor = m_nRenderWidth / (double)oldRenderWidth;
        setOffset((quint64)(m_nOffset * factor));

        if (m_nRenderWidth == 0)
        {
            setZoomFactor(1.0f);
        }

        if (scrollBarsToUpdate.testFlag(Qt::Vertical))
        {
            updateVerticalScrollBar();
        }

        if (scrollBarsToUpdate.testFlag(Qt::Horizontal))
        {
            updateHorizontalScrollBar();
        }
    }
}

void acTimeline::SetVisibleRange(const quint64 startVisibleTime, const quint64 visibleRange)
{
    int localRowWidth = rowWidth();

    // Set the visible range:
    GT_IF_WITH_ASSERT(startVisibleTime < m_nFullRange)
    {
        m_nVisibleStartTime = startVisibleTime + m_nStartTime;
        m_nVisibleRange = visibleRange;

        if (m_nVisibleRange > m_nFullRange)
        {
            m_nVisibleRange = m_nFullRange;
        }

        // sanity check
        if (m_nVisibleStartTime + m_nVisibleRange > m_nFullRange + m_nStartTime)
        {
            m_nVisibleRange = m_nFullRange - m_nVisibleStartTime;
        }

        // Set the zoom factors
        m_dZoomFactor = m_nFullRange * 1.0 / m_nVisibleRange;
        m_nRenderWidth = localRowWidth * m_dZoomFactor;

        // Update the scrollbars
        double scrollRangeAsDouble = 1.0;

        if (m_bScaleHScrollbar)
        {
            scrollRangeAsDouble = ((m_nRenderWidth - localRowWidth) * 1.0 / m_dHScrollbarScaleValue);
        }
        else
        {
            scrollRangeAsDouble = ((m_nRenderWidth - localRowWidth) * 1.0);
        }

        double dVisibleRange = m_nFullRange - m_nVisibleRange;
        m_nOffset = (int)((m_nVisibleStartTime - m_nStartTime) * scrollRangeAsDouble / dVisibleRange);
        updateScrollBars(QFlags<Qt::Orientation>(Qt::Horizontal));

        emit zoomFactorChanged();

        updateGrid();
        update();
    }

}

int acTimeline::TimeToPixel(double timeValue, bool checkBounds)
{
    // default return is offscreen
    int retPixel = -9999;

    int drawingWidth = rowWidth();

    // calculate drawing pixels
    double drawingPixel = visibleRange() / (drawingWidth * 1.0);

    if (m_pGrid != nullptr)
    {
        retPixel = (int)((timeValue - m_pGrid->visibleStartTime()) / drawingPixel);

        if (checkBounds)
        {
            // check left bound
            if (retPixel < 0)
            {
                retPixel = 0;
            }

            // check right bound
            if (retPixel > drawingWidth)
            {
                retPixel = drawingWidth;
            }
        }

        retPixel += titleWidth();
    }

    return retPixel;
}

acTimeline::TimelineMarker::TimelineMarker(quint64 timeStamp, QColor color) :
    m_nTimeStamp(timeStamp), m_color(color)
{
}
