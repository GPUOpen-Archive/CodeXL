//==================================================================================
// Copyright (c) 2011 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acTimelineBranch.cpp
///
//==================================================================================

#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <QPainter>

#include <limits>

#include <AMDTApplicationComponents/Include/Timeline/acTimeline.h>
#include <AMDTApplicationComponents/Include/Timeline/acTimelineBranch.h>
#include <AMDTApplicationComponents/Include/Timeline/acTimelineItem.h>

acTimelineBranch::acTimelineBranch() : QObject(),
    m_strText(""),
    m_pParentTimeline(nullptr),
    m_pParentBranch(nullptr),
    m_nStartTime(std::numeric_limits<quint64>::max()),
    m_nEndTime(std::numeric_limits<quint64>::min()),
    m_bFolded(false),
    m_bSelected(false),
    m_bSorted(true),
    m_nTitleWidth(0),
    m_nDepth(0),
    m_nRowIndex(0),
    m_nTop(0),
    m_bHeightSet(false),
    m_nHeight(0),
    m_pTag(nullptr),
    m_bMaskEnabled(true),
    m_bVisible(true),
    m_pMask(new(std::nothrow) acTimelineMaskBuffer()),
    m_shouldDrawChildren(false)
{
    Q_ASSERT(m_pMask != nullptr);

    if (m_pMask == nullptr)
    {
        m_bMaskEnabled = false;
    }

    m_bgColor = QColor(207, 214, 229);

}

acTimelineBranch::~acTimelineBranch()
{
    // free all items and clear the list
    for (QList<acTimelineItem*>::iterator i = m_timelineItems.begin(); i != m_timelineItems.end(); ++i)
    {
        delete(*i);
        (*i) = nullptr;
    }

    m_timelineItems.clear();

    // free all branches and clear the list
    for (QList<acTimelineBranch*>::iterator i = m_subBranches.begin(); i != m_subBranches.end(); ++i)
    {
        delete(*i);
        (*i) = nullptr;
    }

    m_subBranches.clear();
    delete m_pMask;
    m_pMask = nullptr;
}

void acTimelineBranch::setParentTimeline(acTimeline* newParentTimeline)
{
    Q_ASSERT(m_pParentTimeline == nullptr);

    m_pParentTimeline = newParentTimeline;

    for (QList<acTimelineBranch*>::const_iterator i = m_subBranches.begin(); i != m_subBranches.end(); ++i)
    {
        (*i)->setParentTimeline(newParentTimeline);
    }

    if (m_pParentTimeline != nullptr)
    {
        if (!m_bHeightSet)
        {
            m_nHeight = m_pParentTimeline->defaultBranchHeight();
        }

        recalcTitleWidth();

        connect(m_pParentTimeline, SIGNAL(zoomFactorChanged()),
                this, SLOT(zoomFactorOrOffsetChanged()));
        connect(m_pParentTimeline, SIGNAL(offsetChanged()),
                this, SLOT(zoomFactorOrOffsetChanged()));
    }
}

void acTimelineBranch::setDepth(const unsigned int newDepth)
{
    m_nDepth = newDepth;

    for (QList<acTimelineBranch*>::iterator i = m_subBranches.begin(); i != m_subBranches.end(); ++i)
    {
        (*i)->setDepth(m_nDepth + 1);
    }

    recalcTitleWidth();
}

void acTimelineBranch::setFolded(const bool newFolded)
{
    if (m_bFolded != newFolded)
    {
        m_bFolded = newFolded;
        clearDrawCache(false);

        emit branchFoldedChanged();
    }
}

void acTimelineBranch::setSelected(const bool newSelected)
{
    if (m_bSelected != newSelected)
    {
        m_bSelected = newSelected;

        emit branchSelectedChanged();
    }
}

bool acTimelineBranch::allParentsUnfolded() const
{
    if (m_pParentBranch != nullptr)
    {
        return (!m_pParentBranch->isFolded()) && m_pParentBranch->allParentsUnfolded();
    }
    else
    {
        return true;
    }
}

unsigned int acTimelineBranch::rowCount() const
{
    unsigned int totalCount = 1;

    for (QList<acTimelineBranch*>::const_iterator i = m_subBranches.begin(); i != m_subBranches.end(); ++i)
    {
        totalCount += (*i)->rowCount();
    }

    return totalCount;
}

void acTimelineBranch::resetRowIndex(unsigned int& rowIndex)
{
    m_nRowIndex = rowIndex++;

    for (QList<acTimelineBranch*>::const_iterator i = m_subBranches.begin(); i != m_subBranches.end(); ++i)
    {
        (*i)->resetRowIndex(rowIndex);
    }
}

int acTimelineBranch::subBranchCount() const
{
    return m_subBranches.count();
}

acTimelineBranch* acTimelineBranch::getSubBranch(const int index) const
{
    if (index >= 0 && index < m_subBranches.count())
    {
        return m_subBranches[index];
    }

    return nullptr;
}

void acTimelineBranch::setHeight(const int newHeight)
{
    m_bHeightSet = true;
    m_nHeight = newHeight;
}

int acTimelineBranch::cumulativeHeight() const
{
    if (!allParentsUnfolded())
    {
        return 0;
    }

    int totalHeight = m_nHeight;

    for (QList< acTimelineBranch*>::const_iterator i = m_subBranches.begin(); i != m_subBranches.end(); ++i)
    {
        if ((*i)->IsVisible() == true)
        {
            totalHeight += (*i)->cumulativeHeight();
        }
    }

    return totalHeight;
}

void acTimelineBranch::setText(const QString newText)
{
    if (m_strText != newText)
    {
        m_strText = newText;
        recalcTitleWidth();
        emit branchTextChanged();
    }
}

void acTimelineBranch::setStartTime(const quint64 newStartTime)
{
    if (m_nStartTime != newStartTime)
    {
        m_nStartTime = newStartTime;

        emit branchRangeChanged();
    }
}

void acTimelineBranch::setEndTime(const quint64 newEndTime)
{
    if (m_nEndTime != newEndTime)
    {
        m_nEndTime = newEndTime;

        emit branchRangeChanged();
    }
}

acTimelineItem* acTimelineBranch::getTimelineItem(const acTimelineBranch* branch, const int x, const int y) const
{
    if (branch != nullptr)
    {
        // Search sub-branches first
        for (QList<acTimelineBranch*>::const_iterator i = branch->m_subBranches.begin(); i != branch->m_subBranches.end(); ++i)
        {
            if ((*i)->IsVisible() == true)

            {
                acTimelineItem* item = getTimelineItem(*i, x, y);

                if (item != nullptr)
                {
                    return item;
                }
            }
        }

        // Not found? search our own cached items
        QList<acTimelineItem*> itemsToCheck;

        if (branch->m_drawCache.count() > 0)
        {
            itemsToCheck = branch->m_drawCache;
        }
        else
        {
            itemsToCheck = branch->m_timelineItems;
        }

        acTimelineItem* retVal = nullptr;

        // iterate backwards to get overlapped items correctly in a sorted branch
        for (int i = itemsToCheck.size() - 1; i >= 0; --i)
        {
            retVal = itemsToCheck[i];

            if (retVal->isVisible())
            {
                QRect rect = retVal->drawRectangle();

                if (x >= rect.x() && x <= rect.x() + rect.width() && y >= rect.y() && y < rect.y() + rect.height())
                {
                    return retVal;
                }
            }
        }
    }

    return nullptr;
}

void acTimelineBranch::recalcTitleWidth()
{
    if (m_pParentTimeline != nullptr)
    {
        //recalc title width
        m_nTitleWidth = (m_pParentTimeline->branchIndentation() * m_nDepth) +                                       // take into account the indentation level
                        m_pParentTimeline->titleSectionLeftSpace() + m_pParentTimeline->titleSectionRightSpace() + // take into account the parent's left margin space
                        m_pParentTimeline->fontMetrics().width(m_strText);                                         // take into account this branch's text

        for (QList<acTimelineBranch*>::const_iterator i = m_subBranches.begin(); i != m_subBranches.end(); ++i)
        {
            if ((*i)->IsVisible() == true)
            {
                (*i)->recalcTitleWidth();
                int childWidth = (*i)->titleWidth();

                if (childWidth > m_nTitleWidth)
                {
                    m_nTitleWidth = childWidth;
                }
            }
        }
    }
    else
    {
        m_nTitleWidth = 0;
    }
}

int acTimelineBranch::itemCount() const
{
    return m_timelineItems.count();
}

acTimelineItem* acTimelineBranch::getTimelineItem(const int index) const
{
    if (index >= 0 && index < m_timelineItems.count())
    {
        return m_timelineItems[index];
    }

    return nullptr;
}

acTimelineItem* acTimelineBranch::getTimelineItem(const int x, const int y) const
{
    if (m_bFolded)
    {
        // If this branch is folded, then we need to search the subbranches for this item
        return getTimelineItem(this, x, y);
    }
    else
    {
        acTimelineItem* retVal = nullptr;


        // Not found? search our own cached items
        QList<acTimelineItem*> itemsToCheck;

        if (m_drawCache.count() > 0)
        {
            itemsToCheck = m_drawCache;
        }
        else
        {
            itemsToCheck = m_timelineItems;
        }

        // iterate backwards to get overlapped items correctly in a sorted branch
        for (int i = itemsToCheck.size() - 1; i >= 0; --i)
        {
            retVal = itemsToCheck[i];

            if (retVal->isVisible())
            {
                QRect rect = retVal->drawRectangle();

                if (x >= rect.x() && x <= rect.x() + rect.width() && y >= rect.y() && y < rect.y() + rect.height())
                {
                    return retVal;
                }
            }
        }

        return nullptr;
    }
}

int acTimelineBranch::indexOfItem(acTimelineItem* item) const
{
    return m_timelineItems.indexOf(item);
}

acTimelineBranch* acTimelineBranch::getSubBranchFromRowIndex(const unsigned int rowIndex) const
{
    for (QList<acTimelineBranch*>::const_iterator i = m_subBranches.begin(); i != m_subBranches.end(); ++i)
    {
        if ((*i)->m_nRowIndex == rowIndex)
        {
            return *i;
        }

        if ((*i)->m_nRowIndex < rowIndex && (*i)->m_nRowIndex + (*i)->rowCount() > rowIndex)
        {
            return (*i)->getSubBranchFromRowIndex(rowIndex);
        }
    }

    return NULL;
}

acTimelineBranch* acTimelineBranch::getSubBranchFromText(const QString& branchText, bool partialMatch) const
{
    for (QList<acTimelineBranch*>::const_iterator i = m_subBranches.begin(); i != m_subBranches.end(); ++i)
    {
        if ((*i)->IsVisible() == true)
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

            acTimelineBranch* subbranch = branch->getSubBranchFromText(branchText, partialMatch);

            if (subbranch != nullptr)
            {
                return subbranch;
            }
        }
    }

    return nullptr;
}

acTimelineBranch* acTimelineBranch::getSubBranchFromY(const int y) const
{
    int totalHeight = m_nHeight;

    if (y < totalHeight)
    {
        return const_cast<acTimelineBranch*>(this);
    }

    for (QList<acTimelineBranch*>::const_iterator i = m_subBranches.begin(); i != m_subBranches.end(); ++i)
    {
        if ((*i)->IsVisible() == true)
        {
            int branchHeight = (*i)->cumulativeHeight();

            if (totalHeight + branchHeight > y)
            {
                return (*i)->getSubBranchFromY(y - totalHeight);
            }

            totalHeight += branchHeight;
        }
    }

    return NULL;
}

bool acTimelineBranch::addTimelineItem(acTimelineItem* item)
{
    if (item == nullptr)
    {
        return false;
    }

    // when items are added, keep track if this branch remains sorted
    if (m_bSorted && m_timelineItems.size() > 0)
    {
        quint64 lastItemStartTime = m_timelineItems.last()->startTime();

        if (lastItemStartTime > 0 && item->startTime() < lastItemStartTime)
        {
            m_bSorted = false;
        }
    }

    item->setParentBranch(this);
    m_timelineItems.push_back(item);

    emit branchItemAdded(item);

    bool rangeChanged = false;

    if (m_nStartTime > item->startTime() || (m_nStartTime == std::numeric_limits<quint64>::max()))
    {
        m_nStartTime = item->startTime();
        rangeChanged = true;
    }

    if (item->endTime() > m_nEndTime || (m_nEndTime == std::numeric_limits<quint64>::min()))
    {
        m_nEndTime = item->endTime();
        rangeChanged = true;
    }

    if (rangeChanged)
    {
        emit branchRangeChanged();
    }

    return true;
}

bool acTimelineBranch::addSubBranch(acTimelineBranch* subBranch)
{
    if (subBranch == nullptr)
    {
        return false;
    }

    connect(subBranch, SIGNAL(branchRangeChanged()),
            this, SLOT(subBranchRangeChanged()));
    connect(subBranch, SIGNAL(branchAdded(acTimelineBranch*)),
            this, SLOT(subBranchAdded(acTimelineBranch*)));
    connect(subBranch, SIGNAL(branchTextChanged()),
            this, SLOT(subBranchTextChanged()));

    subBranch->setParentTimeline(this->parentTimeline());
    subBranch->setDepth(this->m_nDepth + 1);

    bool rangeChanged = false;

    if (m_nStartTime > subBranch->m_nStartTime)
    {
        m_nStartTime = subBranch->m_nStartTime;
        rangeChanged = true;
    }

    if (subBranch->m_nEndTime > m_nEndTime)
    {
        m_nEndTime = subBranch->m_nEndTime;
        rangeChanged = true;
    }

    if (rangeChanged)
    {
        emit branchRangeChanged();
    }

    m_subBranches.push_back(subBranch);

    subBranch->m_pParentBranch = this;

    recalcTitleWidth();

    emit branchAdded(subBranch);

    return true;
}

void acTimelineBranch::subBranchRangeChanged()
{
    bool rangeChanged = false;

    acTimelineBranch* branch = qobject_cast<acTimelineBranch*>(sender());

    if (m_nStartTime > branch->startTime())
    {
        m_nStartTime = branch->startTime();
        rangeChanged = true;
    }

    if (branch->endTime() > m_nEndTime)
    {
        m_nEndTime = branch->endTime();
        rangeChanged = true;
    }

    if (rangeChanged)
    {
        emit branchRangeChanged();
    }
}

void acTimelineBranch::subBranchAdded(acTimelineBranch* subBranch)
{
    emit branchAdded(subBranch);
    recalcTitleWidth();
}

void acTimelineBranch::subBranchTextChanged()
{
    emit branchTextChanged();
    recalcTitleWidth();
}

void acTimelineBranch::zoomFactorOrOffsetChanged()
{
    clearDrawCache(false);
}

void acTimelineBranch::draw(QPainter& painter, int& yOffset)
{
    m_nTop = yOffset;
    yOffset += m_nHeight;

    // Draw subbranches first, so that we can easily cull away branches whose parent is folded
    if (!m_bFolded)
    {
        // Draw sub branches
        for (QList<acTimelineBranch*>::const_iterator i = m_subBranches.begin(); i != m_subBranches.end(); ++i)
        {
            if ((*i)->IsVisible() == true)
            {
                (*i)->draw(painter, yOffset);
            }
        }
    }

    if (m_pParentTimeline == nullptr || m_nTop > m_pParentTimeline->height() || m_nTop + m_nHeight < m_pParentTimeline->timelineSectionTopSpace())
    {
        // Invisible, don't draw
        return;
    }

    // Draw row background
    // dilute based on depth
    unsigned int intensity = 230 + (m_nDepth * 5);
    intensity = intensity > 255 ? 255 : intensity;

    QRect rowRect = QRect(0, m_nTop, m_pParentTimeline->width(), m_nHeight);

    QPalette palette = m_pParentTimeline->palette();

    QColor bg = m_bSelected ? QColor::fromRgb(199, 200, 202) : m_bgColor;
    painter.fillRect(rowRect, bg);

    QPen pen = painter.pen();
    painter.save();

    if (m_bSelected)
    {
        pen.setColor(QColor::fromRgb(157, 159, 162));
        pen.setWidth(2);
        painter.setPen(pen);

        // draw entire row outline as highlighted
        painter.drawRect(rowRect);

        pen.setColor(palette.color(QPalette::HighlightedText));
        pen.setWidth(1);
        painter.setPen(pen);
    }
    else
    {
        pen.setColor(QColor(185, 185, 185));
        painter.setPen(pen);
        painter.drawLine(QPoint(0, rowRect.bottom()), QPoint(m_pParentTimeline->width(), rowRect.bottom()));
        pen.setColor(palette.color(QPalette::Text));
        painter.setPen(pen);
    }

    rowRect.setLeft((m_nDepth * m_pParentTimeline->branchIndentation()) + ACTIMELINE_TreeMarkSpace + m_pParentTimeline->titleSectionLeftSpace());
    rowRect.setRight(m_pParentTimeline->titleWidth() + m_pParentTimeline->titleSectionRightSpace());
    painter.drawText(rowRect, Qt::AlignVCenter | Qt::AlignLeft, m_strText);

    if (m_subBranches.size() > 0)
    {
        // TODO:  right now, this paints using a Windows look'n'feel.
        // Investigate using QStyle::drawPrimitive to draw the plus/minus marks in
        // the correct style

        // size of the edges of the square used for the tree mark rect
        const static int treeMarkBoxSize = 8;
        // position of the vertical or horizontal center of the tree mark rect
        const static int treemarkLinePosCenterOffset = treeMarkBoxSize / 2;
        // offset from the top or left side of the treemark rect where the horizontal or vertical line of the plus sign should begin
        const static int treemarkLineMarginTopLeftOffset = 2;
        // offset from the bottom or right side of the treemark rect where the horizontal or vertical line of the plus sign should end
        const static int treemarkLineMarginBottomRightOffset = treemarkLineMarginTopLeftOffset - 1;
        // color "borrowed" from .Net WinForms
        const static QRgb slateBlue = qRgb(106, 90, 205);
        // color "borrowed" from .Net WinForms
        const static QRgb royalBlue = qRgb(65, 105, 225);
        // color "borrowed" from C# version of the control
        const static QRgb treeMarkEdge = qRgb(145, 145, 145);

        // draw a white square
        m_treeMarkRect = QRect(rowRect.x() - ACTIMELINE_TreeMarkSpace, rowRect.y() + (rowRect.height() / 2) - 4, treeMarkBoxSize, treeMarkBoxSize);
        painter.fillRect(m_treeMarkRect, Qt::white);

        // draw the square border
        pen.setColor(treeMarkEdge);
        painter.setPen(pen);
        painter.drawRect(m_treeMarkRect);

        if (m_bFolded)
        {
            // if branch is folded, then draw the vertical bar for the plus sign
            pen.setColor(slateBlue);
            painter.setPen(pen);
            painter.drawLine(m_treeMarkRect.left() + treemarkLinePosCenterOffset,
                             m_treeMarkRect.top() + treemarkLineMarginTopLeftOffset,
                             m_treeMarkRect.left() + treemarkLinePosCenterOffset,
                             m_treeMarkRect.bottom() - treemarkLineMarginBottomRightOffset);
        }

        // always draw the horizontal bar for the plus/minus sign
        pen.setColor(royalBlue);
        painter.setPen(pen);
        painter.drawLine(m_treeMarkRect.left() + treemarkLineMarginTopLeftOffset,
                         m_treeMarkRect.top() + treemarkLinePosCenterOffset,
                         m_treeMarkRect.right() - treemarkLineMarginBottomRightOffset,
                         m_treeMarkRect.top() + treemarkLinePosCenterOffset);
    }
    else
    {
        // make m_treeMarkRect empty
        m_treeMarkRect.setWidth(0);
        m_treeMarkRect.setHeight(0);
    }

    if (m_bMaskEnabled)
    {
        m_pMask->resetMask(m_pParentTimeline->rowWidth());
    }

    // Draw items
    QList<acTimelineItem*> itemsForDrawing;
    bool usingCache = false;

    if (m_drawCache.count() > 0)
    {
        itemsForDrawing = m_drawCache;
        usingCache = true;
    }
    else
    {
        itemsForDrawing = m_timelineItems;
    }

    for (QList<acTimelineItem*>::const_iterator i = itemsForDrawing.begin(); i != itemsForDrawing.end(); ++i)
    {
        acTimelineItem* item = (*i);

        if (m_bSorted && (item->startTime() > m_pParentTimeline->visibleStartTime() + m_pParentTimeline->visibleRange()))
        {
            break;
        }

        item->draw(painter, m_nTop, m_nHeight, true, true);

        if (!usingCache && item->drawRectangle().width() > 0)
        {
            m_drawCache.push_back(item);
        }
    }

    if (m_bFolded || m_shouldDrawChildren)
    {
        // Only update the child's geometry if it is painted once, in the parent branch
        bool shouldUpdateChildGeometry = m_bFolded;

        // Draw sub branches item on itself
        DrawSubBranchItems(painter, m_nTop, m_nHeight, shouldUpdateChildGeometry);
    }

    painter.restore();
}

void acTimelineBranch::DrawSubBranchItems(QPainter& painter, const int yOffset, const int branchHeight, bool shouldUpdateChildGeometry)
{
    if (m_pParentTimeline != nullptr)
    {
        for (QList<acTimelineBranch*>::const_iterator i = m_subBranches.begin(); i != m_subBranches.end(); ++i)
        {
            if ((*i)->IsVisible() == true)
            {
                (*i)->DrawSubBranchItems(painter, yOffset, branchHeight, shouldUpdateChildGeometry);
            }
        }

        if (m_bMaskEnabled)
        {
            m_pMask->resetMask(m_pParentTimeline->rowWidth());
        }

        // Draw items
        QList<acTimelineItem*> itemsForDrawing;
        bool usingCache = false;

        if (m_drawCache.count() > 0)
        {
            itemsForDrawing = m_drawCache;
            usingCache = true;
        }
        else
        {
            itemsForDrawing = m_timelineItems;
        }


        for (QList<acTimelineItem*>::const_iterator i = itemsForDrawing.begin(); i != itemsForDrawing.end(); ++i)
        {
            if (m_bSorted && ((*i)->startTime() > m_pParentTimeline->visibleStartTime() + m_pParentTimeline->visibleRange()))
            {
                break;
            }

            QColor origColor = (*i)->backgroundColor();
            QColor newColor = origColor;

            newColor.setAlpha(50);

            (*i)->setBackgroundColor(newColor);

            (*i)->draw(painter, yOffset, branchHeight, false, shouldUpdateChildGeometry);

            if (!usingCache && (*i)->drawRectangle().width() > 0)
            {
                m_drawCache.push_back(*i);
            }

            (*i)->setBackgroundColor(origColor);
        }
    }
}

void acTimelineBranch::clearDrawCache(bool clearSubBranchCache)
{
    m_drawCache.clear();

    if (clearSubBranchCache)
    {
        for (QList<acTimelineBranch*>::const_iterator i = m_subBranches.begin(); i != m_subBranches.end(); ++i)
        {
            if ((*i)->IsVisible() == true)
            {
                (*i)->clearDrawCache(clearSubBranchCache); // NZ only visible?
            }
        }
    }
}

acTimelineMaskBuffer::~acTimelineMaskBuffer()
{
    for (QMap<int, QMap<int, int*> >::iterator i = m_maskBuffer.begin(); i != m_maskBuffer.end(); ++i)
    {
        for (QMap<int, int*>::iterator j = i.value().begin(); j != i.value().end(); ++j)
        {
            delete[] j.value();
            j.value() = nullptr;
        }
    }
}

bool acTimelineMaskBuffer::setMask(int index, QRect rect)
{
    if (index < 0 || index > m_nBufferWidth)
    {
        return false;
    }

    int mask = rect.width() << 1;
    mask |= 0x1;
    int rectHeight = rect.height();
    int rectTop = rect.top();

    if (!m_maskBuffer.contains(rectHeight) || !m_maskBuffer[rectHeight].contains(rectTop))
    {
        QMap<int, int*> offsetMap;
        offsetMap[rectTop] = new(std::nothrow) int[m_nBufferWidth];
        Q_ASSERT(offsetMap[rectTop] != nullptr);

        if (offsetMap[rectTop] == nullptr)
        {
            return false;
        }

        memset(offsetMap[rectTop], 0, m_nBufferWidth * sizeof(int));

        m_maskBuffer[rectHeight] = offsetMap;
    }

    m_maskBuffer[rectHeight][rectTop][index] = mask;
    return true;
}

bool acTimelineMaskBuffer::checkMask(int index, QRect rect)
{
    if (index < 0 || index > m_nBufferWidth)
    {
        return false;
    }

    int rectHeight = rect.height();
    int rectTop = rect.top();

    if (!m_maskBuffer.contains(rectHeight) || !m_maskBuffer[rectHeight].contains(rectTop) || m_maskBuffer[rectHeight][rectTop] == nullptr)
    {
        return false;
    }

    int mask = m_maskBuffer[rectHeight][rectTop][index];
    bool isSet = (mask & 0x1) == 1;

    if (isSet)
    {
        int setLen = mask >> 1;

        if (setLen < rect.width())
        {
            isSet = false;
        }
    }

    return isSet;
}

void acTimelineMaskBuffer::resetMask(int rowWidth)
{
    if (m_nBufferWidth < rowWidth)
    {
        m_nBufferWidth = rowWidth;
    }

    for (QMap<int, QMap<int, int*> >::iterator i = m_maskBuffer.begin(); i != m_maskBuffer.end(); ++i)
    {
        for (QMap<int, int*>::iterator j = i.value().begin(); j != i.value().end(); ++j)
        {
            delete[] j.value();
            j.value() = nullptr;
        }

        i.value().clear();
    }

    m_maskBuffer.clear();
}

void acTimelineBranch::setVisibility(bool isVisible)
{
    m_bVisible = isVisible;
}
bool acTimelineBranch::IsVisible()const
{
    return m_bVisible;
}
