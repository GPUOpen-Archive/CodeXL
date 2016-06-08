//==================================================================================
// Copyright (c) 2011 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acTimelineGrid.cpp
///
//==================================================================================

// Qt:
#include <QPainter>

// Local:
#include <AMDTApplicationComponents/Include/Timeline/acTimelineGrid.h>


#define AC_DEFAULT_AXIS_LABEL_PRECISION 3

// TODO: check the casts (to double)
// TODO: Perhaps remove concept of visible vs. full (start & range).  Let client set that using only start/range???

acTimelineGrid::acTimelineGrid(QWidget* parent, Qt::WindowFlags flags) : QWidget(parent, flags),
    m_strGridLabel(tr("Scale")),
    m_nLargeDivisionCount(10),
    m_nSmallDivisionCount(5),
    m_bAutoCalculateLargeDivisionCount(true),
    m_nStartTime(0),
    m_nFullRange(0),
    m_nVisibleStartTime(0),
    m_nVisibleRange(0),
    m_nSelectedTime(0),
    m_nEndSelectedTime(0),
    m_bShowTimeHint(true),
    m_strDurationHintLabel(tr("%1 units")),
    m_nGridLabelSpace(ACTIMELINEGRID_DefaultGridLabelSpace),
    m_nGridSpace(0),
    m_nGridRightXPosition(0),
    m_nScalingFactor(1),
    m_dInverseScalingFactor(1),
    m_fMaxTextWidth(0),
    m_nGridShortLineHeight(0),
    m_nGridLongLineHeight(0),
    m_nGridNumTextHeight(0),
    m_bPaintEndTime(true),
    m_precision(AC_DEFAULT_AXIS_LABEL_PRECISION),
    m_rightMargins(0)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

void acTimelineGrid::setGridLabel(const QString newGridLabel)
{
    m_strGridLabel = newGridLabel;
}

void acTimelineGrid::ensureValidVisibleRange()
{
    if (m_nVisibleStartTime < m_nStartTime)
    {
        m_nVisibleStartTime = m_nStartTime;
    }

    quint64 nfullEndTime = m_nStartTime + m_nFullRange;

    if (m_nVisibleStartTime > nfullEndTime)
    {
        m_nVisibleStartTime = nfullEndTime;
    }

    if (m_nSelectedTime > nfullEndTime)
    {
        m_nSelectedTime = nfullEndTime;
    }

    if (m_nEndSelectedTime > nfullEndTime)
    {
        m_nEndSelectedTime = nfullEndTime;
    }

    // ensure that the visible portion is still in range -- if not adjust it
    if (m_nVisibleStartTime + m_nVisibleRange > nfullEndTime)
    {
        m_nVisibleRange = nfullEndTime - m_nVisibleStartTime;
    }
}

void acTimelineGrid::setStartTime(const quint64 newStartTime)
{
    m_nStartTime = newStartTime;

    ensureValidVisibleRange();
}

void acTimelineGrid::setVisibleStartTime(const quint64 newVisibleStartTime)
{
    m_nVisibleStartTime = newVisibleStartTime;

    ensureValidVisibleRange();
}

void acTimelineGrid::setFullRange(const quint64 newFullRange)
{
    m_nFullRange = newFullRange;

    ensureValidVisibleRange();

    QString numString;
    numString.setNum((m_nStartTime + m_nFullRange) * m_dInverseScalingFactor, 'f', m_precision);

    QFontMetrics fontMet = fontMetrics();
    m_fMaxTextWidth = fontMet.width(numString) * 1.75F;  // 75% larger for adequate buffer between two time markers
    m_nGridNumTextHeight = fontMet.lineSpacing(); //TODO:  this is a weird place to do this

    calcLargeDivisionCount();
}

void acTimelineGrid::setVisibleRange(const quint64 newVisibleRange)
{
    m_nVisibleRange = newVisibleRange;

    ensureValidVisibleRange();
}

void acTimelineGrid::setSelectedTime(const quint64 newSelectedTime)
{
    m_nSelectedTime = newSelectedTime;

    ensureValidVisibleRange();
}

void acTimelineGrid::setEndSelectedTime(const quint64 newEndSelectedTime)
{
    m_nEndSelectedTime = newEndSelectedTime;

    ensureValidVisibleRange();
}

void acTimelineGrid::setDurationHintLabel(const QString newDurationHintLabel)
{
    m_strDurationHintLabel = newDurationHintLabel;
}

void acTimelineGrid::setLargeDivisionCount(const unsigned int newLargeDivisionCount)
{
    m_bAutoCalculateLargeDivisionCount = false;
    m_nLargeDivisionCount = newLargeDivisionCount;
}

void acTimelineGrid::setSmallDivisionCount(const unsigned int newSmallDivisionCount)
{
    m_nSmallDivisionCount = newSmallDivisionCount;
}

void acTimelineGrid::setGridLabelSpace(const int newGridLabelSpace)
{
    m_nGridLabelSpace = newGridLabelSpace;
    calcLargeDivisionCount();
}

void acTimelineGrid::setScalingFactor(const quint64 newScalingFactor)
{
    m_nScalingFactor = newScalingFactor;

    if (m_nScalingFactor == 0)
    {
        m_nScalingFactor = 1;
    }

    m_dInverseScalingFactor = (double)1 / (double)m_nScalingFactor;
}

QSize acTimelineGrid::sizeHint() const
{
    QSize size(0, 50);
    return size;
}

void acTimelineGrid::paintEvent(QPaintEvent* /* event */)
{
    QPainter painter(this);
    painter.setPen(palette().foreground().color());

    QRect numRect = QRect(0, 0, m_nGridLabelSpace, height());

    //painter.drawText(numRect, Qt::AlignCenter, m_strGridLabel);

    if (m_nLargeDivisionCount == 0)
    {
        return;
    }

    double unitWidth = (double)m_nGridSpace / (double)m_nLargeDivisionCount;
    double unitRange = (double)m_nVisibleRange / (double)m_nLargeDivisionCount;
    m_nGridRightXPosition = 0;
    int unitWidthShortLine = (int)(unitWidth / m_nSmallDivisionCount);

    QPoint p0;
    QPoint p1;
    double gridNum;

    // Draw the grid
    for (unsigned int i = 0; i <= m_nLargeDivisionCount; i++)
    {
        unsigned int thisLinePos = (unsigned int)(i * unitWidth);

        p0.setX(m_nGridLabelSpace + thisLinePos);
        p0.setY(ms_GRID_MARGIN);
        p1.setX(m_nGridLabelSpace + thisLinePos);
        p1.setY(ms_GRID_MARGIN + m_nGridLongLineHeight);

        if (i == m_nLargeDivisionCount)
        {
            // next line makes up for the fact that some extra items ..?
            gridNum = (double)(m_nVisibleStartTime + m_nVisibleRange);

            // don't paint a full length end line, if it's not truly the end
            if (m_nVisibleStartTime + m_nVisibleRange == m_nStartTime + m_nFullRange)
            {
                p1.ry() += m_nGridNumTextHeight;
            }

            // if painting the last long line, store its X coordinate for later use as the right coordinate of the top line
            // also adjust the X coordinate to be the absolute end of the timeline
            p0.setX(width() - 1 - m_rightMargins);
            p1.setX(p0.x());
            m_nGridRightXPosition = p0.x();
        }
        else
        {
            gridNum = (double)m_nVisibleStartTime + (unitRange * i);

            if (i == 0)
            {
                if (m_nStartTime == m_nVisibleStartTime)
                {
                    p1.ry() += m_nGridNumTextHeight;
                }
            }
        }

        painter.drawLine(p0, p1);

        // draw grid numbers
        QString numString;
        numString.setNum(gridNum * m_dInverseScalingFactor, 'f', m_precision);
        int textWidth = fontMetrics().width(numString);

        int flags;

        if (i == 0)
        {
            numRect.setLeft(p0.x() + 2);
            flags = Qt::AlignLeft | Qt::AlignVCenter;
        }
        else if (i == m_nLargeDivisionCount)
        {
            if (!m_bPaintEndTime)
            {
                continue;
            }

            numRect.setLeft(p0.x() - textWidth - 2);
            flags = Qt::AlignRight | Qt::AlignVCenter;
        }
        else
        {
            numRect.setLeft(p0.x() - (textWidth / 2));
            flags = Qt::AlignHCenter | Qt::AlignVCenter;
        }

        numRect.setTop(m_nGridLongLineHeight + ms_GRID_MARGIN);
        numRect.setWidth(textWidth);
        numRect.setHeight(m_nGridNumTextHeight);

        painter.drawText(numRect, flags, numString);

        // don't draw more short lines after the last long line
        if (m_nGridRightXPosition == 0)
        {
            // Draw short line
            p1.setY(ms_GRID_MARGIN + m_nGridShortLineHeight);

            for (unsigned int j = 0; j < m_nSmallDivisionCount; j++)
            {
                p0.setX(m_nGridLabelSpace + thisLinePos + (j * unitWidthShortLine));
                p1.setX(p0.x());
                painter.drawLine(p0, p1);
            }
        }
    }

    drawTimeHints(painter);

    // draw top line last, using the pre-calculated right coordinate
    if (m_nGridRightXPosition > 0)
    {
        p0.setX(m_nGridLabelSpace);
        p0.setY(ms_GRID_MARGIN);
        p1.setX(m_nGridRightXPosition);
        p1.setY(ms_GRID_MARGIN);
        painter.drawLine(p0, p1);
    }

    // "ms" label is now positioned to the right of the grid
    numRect = QRect(m_nGridRightXPosition - m_rightMargins, 0, 20, height());
    painter.drawText(numRect, Qt::AlignCenter, m_strGridLabel);

}

void acTimelineGrid::resizeEvent(QResizeEvent* event)
{
    m_nGridLongLineHeight = height() - m_nGridNumTextHeight - (ms_GRID_MARGIN * 2);
    m_nGridShortLineHeight = (int)(m_nGridLongLineHeight / 3);

    calcLargeDivisionCount();

    QWidget::resizeEvent(event);
}

void acTimelineGrid::drawTimeHints(QPainter& painter)
{
    if (m_bShowTimeHint)
    {
        QString startStrNum;

        startStrNum.setNum(m_nSelectedTime * m_dInverseScalingFactor, 'f', m_precision);

        addOutOfRangeCharacterToHintString(startStrNum, m_nSelectedTime);

        int startTextWidth = fontMetrics().width(startStrNum);
        QRect startHintRect = calcHintRect(startTextWidth, m_nSelectedTime);
        QColor hintColor = palette().color(QPalette::ToolTipBase);
        hintColor.setAlpha(192);

        if (m_nSelectedTime != m_nEndSelectedTime)
        {
            QString endStrNum;
            endStrNum.setNum(m_nEndSelectedTime * m_dInverseScalingFactor, 'f', m_precision);

            addOutOfRangeCharacterToHintString(endStrNum, m_nEndSelectedTime);
            int endTextWidth = fontMetrics().width(endStrNum);
            QRect endHintRect = calcHintRect(endTextWidth, m_nEndSelectedTime);

            quint64 diffTime;
            quint64 absDiffTime;

            if (m_nEndSelectedTime > m_nSelectedTime)
            {
                diffTime = m_nEndSelectedTime - m_nSelectedTime;
                absDiffTime = m_nSelectedTime + (diffTime / 2);
            }
            else
            {
                diffTime = m_nSelectedTime - m_nEndSelectedTime;
                absDiffTime = m_nEndSelectedTime + (diffTime / 2);
            }

            if ((startHintRect.left() == m_nGridLabelSpace && endHintRect.right() == m_nGridLabelSpace + m_nGridSpace - 1) ||
                (endHintRect.left() == m_nGridLabelSpace && startHintRect.right() == m_nGridLabelSpace + m_nGridSpace - 1))
            {
                absDiffTime = m_nVisibleStartTime + (m_nVisibleRange / 2);
            }

            QString diffStrNum;
            diffStrNum.setNum(diffTime * m_dInverseScalingFactor, 'f', m_precision);

            diffStrNum = m_strDurationHintLabel.arg(diffStrNum);
            int diffTextWidth = fontMetrics().width(diffStrNum);
            QRect diffHintRect = calcHintRect(diffTextWidth, absDiffTime);

            if (startHintRect.intersects(endHintRect) || startHintRect.intersects(diffHintRect)
                || endHintRect.intersects(diffHintRect))
            {
                if (m_nEndSelectedTime > m_nSelectedTime)
                {
                    startStrNum = startStrNum + " - " + endStrNum + " (" + diffStrNum + ")" ;
                }
                else
                {
                    startStrNum = endStrNum + " - " + startStrNum + " (" + diffStrNum + ")";
                }

                startTextWidth = fontMetrics().width(startStrNum);
                startHintRect = calcHintRect(startTextWidth, m_nSelectedTime);
            }
            else
            {
                painter.fillRect(endHintRect, hintColor);
                painter.drawText(endHintRect, Qt::AlignCenter, endStrNum);

                painter.fillRect(diffHintRect, hintColor);
                painter.drawText(diffHintRect, Qt::AlignCenter, diffStrNum);
            }
        }

        painter.fillRect(startHintRect, hintColor);
        painter.drawText(startHintRect, Qt::AlignCenter, startStrNum);

    }
}

void acTimelineGrid::addOutOfRangeCharacterToHintString(QString& hintString, quint64 timeStamp)
{
    if (timeStamp < m_nVisibleStartTime)
    {
        hintString.prepend(' ').prepend(QChar(0x25C4));
    }
    else if (timeStamp > m_nVisibleStartTime + m_nVisibleRange)
    {
        hintString.append(' ').append(QChar(0x25BA));
    }
}

QRect acTimelineGrid::calcHintRect(int textWidth, quint64 time)
{
    QRect textRect;
    int x;

    if (time < m_nVisibleStartTime)
    {
        x = m_nGridLabelSpace;
    }
    else if (time > m_nVisibleStartTime + m_nVisibleRange)
    {
        x = m_nGridLabelSpace + m_nGridSpace - textWidth;
    }
    else
    {
        x = m_nGridLabelSpace + (int)(m_nGridSpace * ((double)(time - m_nVisibleStartTime) / m_nVisibleRange)) - (textWidth / 2);

        if (x < m_nGridLabelSpace)
        {
            x = m_nGridLabelSpace;
        }
        else if (x + textWidth > m_nGridLabelSpace + m_nGridSpace)
        {
            x = m_nGridLabelSpace + m_nGridSpace - textWidth;
        }
    }

    textRect.setX(x);
    //textRect.setY(ms_GRID_MARGIN + m_nGridLongLineHeight - m_nGridNumTextHeight);
    //textRect.setY(ms_GRID_MARGIN + m_nGridShortLineHeight);
    textRect.setY((m_nGridLongLineHeight - m_nGridNumTextHeight + m_nGridShortLineHeight + m_nGridNumTextHeight) / 2);
    textRect.setHeight(m_nGridNumTextHeight);
    textRect.setWidth(textWidth);

    return textRect;
}

void acTimelineGrid::calcLargeDivisionCount()
{
    m_nGridSpace = width() - m_nGridLabelSpace - m_rightMargins;

    if (m_nGridSpace >= 0)
    {
        if (m_bAutoCalculateLargeDivisionCount && m_fMaxTextWidth > 0)
        {
            m_nLargeDivisionCount = (int)(m_nGridSpace / m_fMaxTextWidth);
            m_bPaintEndTime = m_nLargeDivisionCount > 0;

            if (m_nLargeDivisionCount == 0)
            {
                m_nLargeDivisionCount = 1;
            }
        }
    }
    else
    {
        m_nGridSpace = 0;
        m_nLargeDivisionCount = 1;
        m_bPaintEndTime = false;
    }
}
