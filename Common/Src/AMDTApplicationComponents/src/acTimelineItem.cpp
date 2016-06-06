//==================================================================================
// Copyright (c) 2011 - 2016  , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acTimelineItem.cpp
///
//==================================================================================

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>

// Local:
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/Timeline/acTimeline.h>
#include <AMDTApplicationComponents/Include/Timeline/acTimelineItem.h>
#include <AMDTApplicationComponents/Include/Timeline/acTimelineBranch.h>

QMap<QRgb, QRgb> acTimelineItem::m_lightenedColorMap;

acTimelineItem::acTimelineItem(quint64 startTime, quint64 endTime) : QObject(),
    m_pParentBranch(NULL),
    m_backgroundColor(Qt::black),
    m_lightenedBackgroundColor(Qt::black),
    m_foregroundColor(Qt::white),
    m_outlineColor(Qt::red),
    m_strText(""),
    m_nIndex(-1),
    m_nStartTime(startTime),
    m_nEndTime(endTime),
    m_dFractionalHeight(1.0f),
    m_dFractionalOffset(0),
    m_bLightenedBGColorSet(false),
    m_bLightenedBGColorCalculated(false),
    m_bVisible(false), //false until it is painted
    m_bUseFractionalOffset(false),
    m_itemShape(AC_TIMELINE_RECTANGLE),
    m_isSelected(false)
{
}

QRgb acTimelineItem::lightenColor(QRgb color, int amount)
{
    QMap<QRgb, QRgb>::iterator item = m_lightenedColorMap.find(color);

    if (item != m_lightenedColorMap.end())
    {
        return item.value();
    }
    else
    {
        QColor lightColor = QColor(color);
        lightColor = lightColor.lighter(amount);
        QRgb rgba = lightColor.rgba();
        m_lightenedColorMap[color] = rgba;
        return rgba;
    }
}

void acTimelineItem::DrawSelection(QPainter& painter)
{
    QPen pen = painter.pen();

    // Sanity check
    GT_IF_WITH_ASSERT(m_pParentBranch != nullptr && m_pParentBranch->parentTimeline() != nullptr)
    {
        if (IsSelected())
        {
            pen.setWidth(2);
            pen.setColor(Qt::yellow);
            painter.setPen(pen);

            if (m_rect.width() > 1)
            {
                if (m_rect.width() <= 4)
                {
                    painter.drawRect(m_rect);
                }
                else
                {
                    painter.drawPath(m_itemFramePath);
                }
            }
            else
            {
                painter.fillRect(m_rect.left(), m_rect.top(), 2, m_rect.height(), Qt::yellow);
            }
        }
        else  if (IsHighlighted())
        {
            // draw item under the mouse with its outline color (skipped if the user is mouse dragging)
            pen.setColor(Qt::yellow);
            pen.setWidth(1);
            painter.setPen(pen);
            painter.drawPath(m_itemFramePath);
        }
    }
}

void acTimelineItem::setParentBranch(acTimelineBranch* newParentBranch)
{
    // this is only expected to be called once -- changing the parent branch is not supported
    // TODO:  this should probably not be a public method
    Q_ASSERT(m_pParentBranch == NULL);

    if (m_pParentBranch == NULL)
    {
        m_pParentBranch = newParentBranch;
    }
}

void acTimelineItem::setBackgroundColor(const QColor& newColor)
{
    m_backgroundColor = newColor;
    m_bLightenedBGColorCalculated = false;
}

QColor acTimelineItem::lightenedBackgroundColor()
{
    if (!m_bLightenedBGColorSet && ! m_bLightenedBGColorCalculated)
    {
        m_lightenedBackgroundColor = lightenColor(m_backgroundColor.rgba(), 150);
        m_bLightenedBGColorCalculated = true;
    }

    return m_lightenedBackgroundColor;
}

void acTimelineItem::setLightenedBackgroundColor(const QColor& newColor)
{
    m_lightenedBackgroundColor = newColor;
    m_bLightenedBGColorSet = true;
}

void acTimelineItem::setStartTime(const quint64 newStartTime)
{
    m_nStartTime = newStartTime;

    if ((m_nStartTime > m_nEndTime) && (m_nEndTime != std::numeric_limits<quint64>::min()))
    {
        m_nStartTime = m_nEndTime;
    }
}

void acTimelineItem::setEndTime(const quint64 newEndTime)
{
    m_nEndTime = newEndTime;

    if ((m_nEndTime < m_nStartTime) && (m_nStartTime != std::numeric_limits<quint64>::max()))
    {
        m_nEndTime = m_nStartTime;
    }
}

int acTimelineItem::index()
{
    if (m_nIndex == -1 && m_pParentBranch != NULL)
    {
        m_nIndex = m_pParentBranch->indexOfItem(this);
    }

    return m_nIndex;
}

void acTimelineItem::setFractionalOffset(const double newFractionalOffset)
{
    m_dFractionalOffset = newFractionalOffset;
    m_bUseFractionalOffset = true;

    if (m_dFractionalOffset < 0)
    {
        m_dFractionalOffset = 0;
    }
    else if (m_dFractionalOffset > 1.0f)
    {
        m_dFractionalOffset = 1.0f;
    }
}

void acTimelineItem::setFractionalHeight(const double newFractionalHeight)
{
    m_dFractionalHeight = newFractionalHeight;

    if (m_dFractionalHeight < 0)
    {
        m_dFractionalHeight = 0;
    }
    else if (m_dFractionalHeight > 1.0f)
    {
        m_dFractionalHeight = 1.0f;
    }
}

QString acTimelineItem::tooltipText() const
{
    acTimelineItemToolTip tooltip;
    tooltipItems(tooltip);

    QString retVal;
    QString row1 = "%1: %2";
    QString row2 = "%1";

    for (int i = 0; i < tooltip.count(); i++)
    {
        if (tooltip.getName(i).isEmpty())
        {
            retVal += row2.arg(tooltip.getValue(i));
        }
        else
        {
            retVal += row1.arg(tooltip.getName(i), tooltip.getValue(i));
        }

        if (i < tooltip.count() - 1)
        {
            retVal += "<br>";
        }
    }

    if (!tooltip.additionalText().isEmpty())
    {
        retVal += "\n" + tooltip.additionalText();
    }

    return retVal;
}

QString acTimelineItem::getDurationString(quint64 duration)
{
    double sec = 1e9;
    double millisec = 1e6;
    double microsec = 1e3;

    double fnum = static_cast<double>(duration);
    QString strNum;

    if (fnum > sec)
    {
        fnum /= sec;
        strNum = QString(tr("%1 seconds")).arg(fnum, 0, 'f', 3);
    }
    else if (fnum > millisec)
    {
        fnum /= millisec;
        strNum = QString(tr("%1 milliseconds")).arg(fnum, 0, 'f', 3);
    }
    else if (fnum > microsec)
    {
        fnum /= microsec;
        strNum = QString(tr("%1 microseconds")).arg(fnum, 0, 'f', 3);
    }
    else
    {
        strNum = QString(tr("%1 nanoseconds")).arg(duration);
    }

    return strNum;
}

void acTimelineItem::tooltipItems(acTimelineItemToolTip& tooltip) const
{
    int pos = m_strText.indexOf('_');

    if (pos > 0)
    {
        // Assuming this is an interface + call
        QString interfaceStr = m_strText.mid(0, pos);
        QString callStr = m_strText.mid(pos + 1, m_strText.length() - pos + 1);
        tooltip.add(QString("%1.%2").arg(interfaceStr).arg(callStr), "");
    }
    else
    {
        tooltip.add(tr("Name"), m_strText);
    }

    quint64 timelineStartTime = 0;

    if (m_pParentBranch != NULL)
    {
        acTimeline* timeline = m_pParentBranch->parentTimeline();

        if (timeline != NULL)
        {
            timelineStartTime = timeline->startTime();
        }
    }

    double fnum = (m_nStartTime - timelineStartTime) / 1e6; // convert to milliseconds
    QString strNum = QString(tr("%1 millisecond")).arg(fnum, 0, 'f', 3);
    tooltip.add(tr("Start Time"), strNum);

    fnum = (m_nEndTime - timelineStartTime) / 1e6; // convert to milliseconds
    strNum = QString(tr("%1 millisecond")).arg(fnum, 0, 'f', 3);
    tooltip.add(tr("End Time"), strNum);

    quint64 duration = m_nEndTime - m_nStartTime;
    QString strDuration = QString(tr("%1 millisecond")).arg(NanosecToTimeString(duration, true, false));
    tooltip.add(tr("Duration"), strDuration);
}

bool acTimelineItem::IsHighlighted() const
{
    bool retVal = false;

    // Sanity check
    GT_IF_WITH_ASSERT(m_pParentBranch != nullptr && m_pParentBranch->parentTimeline() != nullptr)
    {
        acTimeline* pTimeline = m_pParentBranch->parentTimeline();
        QPoint mouseLocation = pTimeline->mouseLocation();

        if (m_rect.contains(mouseLocation, false) && !pTimeline->mouseDragging() &&
            pTimeline->getTimelineItem(mouseLocation.x(), mouseLocation.y()) == this)
        {
            retVal = true;
        }
    }

    return retVal;
}

void acTimelineItem::draw(QPainter& painter, const int branchRowTop, const int branchHeight, bool drawGradientBG, bool shouldUpdateGeometry)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pParentBranch != nullptr && m_pParentBranch->parentTimeline() != nullptr)
    {
        acTimeline* pTimeline = m_pParentBranch->parentTimeline();

        // Calculate the item geometry. The function also set m_bVisible, so we need to check is the item is visible after the calculation
        bool roundLeftCorners = true;
        bool roundRightCorners = true;
        QRect originalRect = m_rect;
        CalculateItemRect(branchHeight, branchRowTop, roundLeftCorners, roundRightCorners);

        if (m_bVisible)
        {
            // Check if the parent branch mask
            CheckParentBranchMask();

            if (m_rect.isValid())
            {
                m_bVisible = true;

                // Draw the item background
                DrawItemBackground(painter, roundLeftCorners, roundRightCorners, drawGradientBG);

                // Draw the item selection frame is the item is selected
                if (shouldUpdateGeometry)
                {
                    // Draw the selection frame only for the real item, not for the item shadow
                    DrawSelection(painter);

                    // Draw the item text
                    DrawItemText(pTimeline, painter);
                }

                painter.restore();
            }
            else
            {
                m_bVisible = false;
            }
        }

        if (!shouldUpdateGeometry)
        {
            m_rect = originalRect;
        }
    }
}


void acTimelineItem::DrawItemBackground(QPainter& painter, bool roundLeftCorners, bool roundRightCorners, bool drawGradientBG)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pParentBranch != nullptr && m_pParentBranch->parentTimeline() != nullptr)
    {
        acTimeline* pTimeline = m_pParentBranch->parentTimeline();
        QColor blockColor = m_backgroundColor;
        QColor lightenedColor = lightenedBackgroundColor();

        if (!pTimeline->isEnabled())
        {
            blockColor.setAlpha(32);
            lightenedColor.setAlpha(32);
        }

        QPen pen = painter.pen();
        painter.save();

        int rectHeight = m_rect.height();
        int rectWidth = m_rect.width();

        if (m_rect.width() == 1)
        {
            pen.setColor(blockColor);
            painter.setPen(pen);
            painter.drawLine(m_rect.topLeft(), m_rect.bottomLeft());
        }
        else if (rectHeight == 1)
        {
            pen.setColor(blockColor);
            painter.setPen(pen);
            painter.drawLine(m_rect.topLeft(), m_rect.topRight());
        }
        else
        {
            // Clear the path
            m_itemFramePath = QPainterPath();

            if (pTimeline->roundedRectangles() && rectHeight > 4 && rectWidth > 4)
            {
                m_itemFramePath.addRoundedRect(m_rect, 4.0, 4.0);

                if (!roundLeftCorners)
                {
                    // create a new path with a non-rounded left edge and union it with the rounded path
                    // this causes square edges to be painted when a timeline item is partially scrolled off the left edge of the timeline
                    QRect newRect(m_rect);
                    newRect.setWidth(newRect.width() / 2);

                    if (newRect.isValid())
                    {
                        QPainterPath leftPath;
                        leftPath.addRect(newRect);
                        m_itemFramePath = m_itemFramePath.united(leftPath);
                    }
                }

                if (!roundRightCorners)
                {
                    // create a new path with a non-rounded right edge and union it with the rounded path
                    // this causes square edges to be painted when a timeline item is partially scrolled off the right edge of the timeline
                    QRect newRect(m_rect);
                    newRect.setX(newRect.x() + newRect.width() / 2);

                    if (newRect.isValid())
                    {
                        QPainterPath rightPath;
                        rightPath.addRect(newRect);
                        m_itemFramePath = m_itemFramePath.united(rightPath);
                    }
                }
            }
            else
            {
                m_itemFramePath.addRect(m_rect);
            }

            QBrush blockBrush;

            if (pTimeline->gradientPainting() && drawGradientBG)
            {
                QLinearGradient gradient(m_rect.topLeft(), m_rect.bottomRight());
                gradient.setColorAt(0.0, blockColor);
                gradient.setColorAt(1.0, lightenedColor);
                blockBrush = QBrush(gradient);
            }
            else
            {
                blockBrush = QBrush(blockColor);
            }

            painter.fillPath(m_itemFramePath, blockBrush);
        }
    }
}

void acTimelineItem::DrawItemText(acTimeline* pTimeline, QPainter& painter)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pTimeline != nullptr)
    {
        QPen pen = painter.pen();
        QFontMetrics fontMet = pTimeline->fontMetrics();

        if (m_rect.width() > fontMet.averageCharWidth() && m_rect.height() >= fontMet.lineSpacing())
        {
            pen.setColor(m_foregroundColor);
            painter.setPen(pen);
            painter.drawText(m_rect, Qt::AlignCenter, m_strText);
        }
    }
}

void acTimelineItem::CheckParentBranchMask()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pParentBranch != nullptr && m_pParentBranch->parentTimeline() != nullptr)
    {
        acTimeline* pTimeline = m_pParentBranch->parentTimeline();
        int rowWidth = pTimeline->rowWidth();
        int titleWidth = pTimeline->titleWidth();

        // if (enable mask on the branch), check parent mask
        if (m_pParentBranch->isMaskEnabled())
        {
            int maskIndex = m_rect.x() - titleWidth;
            maskIndex = maskIndex > rowWidth - 1 ? rowWidth - 1 : maskIndex;
            maskIndex = maskIndex < 0 ? 0 : maskIndex;

            if (m_pParentBranch->getMaskBuffer()->checkMask(maskIndex, m_rect))
            {
                m_rect.setWidth(-1);
                m_rect.setHeight(-1);
                m_bVisible = false;
            }
            else
            {
                m_pParentBranch->getMaskBuffer()->setMask(maskIndex, m_rect);
            }
        }
    }
}

void acTimelineItem::CalculateItemRect(const int branchHeight, const int branchRowTop, bool& roundLeftCorners, bool& roundRightCorners)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pParentBranch != nullptr && m_pParentBranch->parentTimeline() != nullptr)
    {
        acTimeline* pTimeline = m_pParentBranch->parentTimeline();

        int itemX = 0, itemY = 0, itemWidth = -1, itemHeight = -1;
        m_rect.setWidth(-1);
        m_rect.setHeight(-1);
        m_bVisible = false;

        // Check visibility
        if (m_nEndTime >= pTimeline->visibleStartTime() && (m_nStartTime <= (pTimeline->visibleStartTime() + pTimeline->visibleRange())))
        {
            m_bVisible = true;
            // calculate width
            double dblWidth = (m_nEndTime - m_nStartTime) * pTimeline->invRange() * pTimeline->renderWidth();
            itemWidth = 1;

            if (dblWidth > 1)
            {
                itemWidth = (int)dblWidth;
            }

            // calculate top and height
            itemHeight = branchHeight;
            itemY = branchRowTop + 1;

            if (m_dFractionalHeight > 0 && m_dFractionalHeight < 1.0)
            {
                int pixelsDropped = (int)(itemHeight * (1.0 - m_dFractionalHeight));
                itemHeight -= pixelsDropped;
                itemY += pixelsDropped / 2;
            }

            if (m_bUseFractionalOffset)
            {
                int pixelOffset = (int)(branchHeight * m_dFractionalOffset);
                itemY = branchRowTop + 1 + pixelOffset;

                if (itemY + itemHeight > branchRowTop + 1 + branchHeight)
                {
                    itemHeight = branchHeight - pixelOffset;
                }
            }

            int titleWidth = pTimeline->titleWidth();
            itemX = titleWidth + pTimeline->getXCoordOfTime(m_nStartTime);

            if (itemX + itemWidth <= titleWidth)
            {
                m_bVisible = false;
            }
            else
            {
                roundLeftCorners = true;
                roundRightCorners = true;

                if (itemX < titleWidth && itemX + itemWidth > titleWidth)
                {
                    // trim left -- don't use a rounded edge when painting the left edge
                    itemWidth = itemX + itemWidth - titleWidth;
                    itemX = titleWidth;
                    roundLeftCorners = false;
                }

                int rowWidth = pTimeline->rowWidth();

                if (itemX + itemWidth > titleWidth + rowWidth)
                {
                    // trim right -- don't use a rounded edge when painting the right edge
                    itemWidth = titleWidth + rowWidth - itemX;
                    roundRightCorners = false;
                }

                // For items with the same start and end time, draw a point:
                if (m_itemShape != AC_TIMELINE_RECTANGLE)
                {
                    itemWidth = 2;

                    if (m_itemShape == AC_TIMELINE_DOT)
                    {
                        itemHeight = 4;
                        itemY = itemY + branchHeight - itemHeight;
                    }
                }
            }

            // set rect coordinates
            m_rect.setX(itemX);
            m_rect.setY(itemY);
            m_rect.setWidth(itemWidth);
            m_rect.setHeight(itemHeight - 2);
        }
    }
}

acAPITimelineItem::acAPITimelineItem(quint64 startTime, quint64 endTime, int apiIndex) : acTimelineItem(startTime, endTime),
    m_nApiIndex(apiIndex)
{
}

void acAPITimelineItem::tooltipItems(acTimelineItemToolTip& tooltip) const
{
    acTimelineItem::tooltipItems(tooltip);
    QString indexString;
    indexString.setNum(m_nApiIndex);
    tooltip.add(tr("Call Index"), indexString);
}

acTimelineItemToolTip::acTimelineItemToolTip() : QObject()
{
}

acTimelineItemToolTip::~acTimelineItemToolTip()
{
    clear();
}

int acTimelineItemToolTip::count() const
{
    return m_tooltipItems.count();
}

bool acTimelineItemToolTip::isValid() const
{
    return m_tooltipItems.count() > 0 || !m_strAdditionalText.isEmpty();
}

bool acTimelineItemToolTip::add(QString name, QString value)
{
    ToolTipItem* newToolTipItem = new(std::nothrow) ToolTipItem(name, value);
    Q_ASSERT(newToolTipItem != NULL);

    if (newToolTipItem != NULL)
    {
        m_tooltipItems.push_back(newToolTipItem);
        return true;
    }

    return false;
}

bool acTimelineItemToolTip::remove(int index)
{
    if (index >= 0 && index < m_tooltipItems.count())
    {
        ToolTipItem* item = m_tooltipItems.takeAt(index);
        delete item;
        item = NULL;
        return true;
    }

    return false;
}

QString acTimelineItemToolTip::getName(int index) const
{
    if (index >= 0 && index < m_tooltipItems.count())
    {
        return m_tooltipItems[index]->m_strName;
    }

    return NULL;
}

QString acTimelineItemToolTip::getValue(int index) const
{
    if (index >= 0 && index < m_tooltipItems.count())
    {
        return m_tooltipItems[index]->m_strValue;
    }

    return NULL;
}

void acTimelineItemToolTip::clear()
{
    for (QList<ToolTipItem*>::iterator i = m_tooltipItems.begin(); i != m_tooltipItems.end(); ++i)
    {
        delete(*i);
        (*i) = NULL;
    }

    m_tooltipItems.clear();
    m_strAdditionalText = "";
}
