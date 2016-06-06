//==================================================================================
// Copyright (c) 2011 - 2016  , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acTimelineItemCurve.cpp
///
//==================================================================================

// Local:
#include <AMDTApplicationComponents/Include/Timeline/acTimeline.h>
#include <AMDTApplicationComponents/Include/Timeline/acTimelineBranch.h>
#include <AMDTApplicationComponents/Include/Timeline/acTimelineItemCurve.h>

acTimelineItemCurve::acTimelineItemCurve() :
    acTimelineItem(0, 0),
    m_fMaxValue(std::numeric_limits<float>::min()),
    m_dInvMaxValue(0),
    m_nMinTime(std::numeric_limits<quint64>::max()),
    m_nMaxTime(std::numeric_limits<quint64>::min()),
    m_bSolid(true)
{
    //propagatable = false
}

void acTimelineItemCurve::draw(QPainter& painter, const int branchRowTop, const int branchHeight)
{

    acTimeline* timeline = m_pParentBranch->parentTimeline();
    int titleWidth = timeline->titleWidth();


    m_pointBuffer.clear();


    //if (m_pointBuffer.empty() || m_pointBuffer.count() != m_samplePairs.count())
    {
        //for(QList<quint64>::const_iterator it = m_samplePairs.keys().constBegin(); it != m_samplePairs.keys().constEnd(); ++it)
        for (QMap<quint64, float>::const_iterator it = m_samplePairs.constBegin(); it != m_samplePairs.constEnd(); ++it)
            //quint64 key;
            //foreach(key, m_samplePairs.keys())
        {
            //if (key >= timeline->visibleStartTime() && key <= timeline->visibleStartTime() + timeline->visibleRange())
            {
                int x = timeline->getXCoordOfTime(it.key());
                double val = it.value() * m_dInvMaxValue;
                QPoint p = QPoint(x + titleWidth, (int)(branchRowTop + (branchHeight * val)));
                m_pointBuffer.push_back(p);
            }
        }
    }

    QPen pen = painter.pen();
    painter.save();

    painter.setClipRect(QRect(titleWidth, branchRowTop, timeline->rowWidth(), branchHeight));
    pen.setColor(backgroundColor());
    painter.setPen(pen);

    //painter.drawLines(m_pointBuffer);
    QPoint point;
    QPainterPath path;
    bool first = true;

    if (m_bSolid)
    {
        path.moveTo(timeline->getXCoordOfTime(m_nMinTime) + titleWidth, branchRowTop + branchHeight);
        first = false;
    }

    for (QVector<QPoint>::const_iterator it = m_pointBuffer.constBegin(); it != m_pointBuffer.end(); ++it)
        //foreach(point, m_pointBuffer)
    {
        if (first)
        {
            path.moveTo(*it);
            first = false;
        }
        else
        {
            path.lineTo(*it);
        }
    }

    painter.drawPath(path);

    if (m_bSolid)
    {
        path.lineTo(timeline->getXCoordOfTime(m_nMaxTime) + titleWidth, branchRowTop + branchHeight);
        path.lineTo(timeline->getXCoordOfTime(m_nMinTime) + titleWidth, branchRowTop + branchHeight);
        painter.fillPath(path, QBrush(backgroundColor()));
    }

    painter.restore();
}

void acTimelineItemCurve::addTimestampValuePair(quint64 timeStamp, float value)
{
    if (timeStamp > m_nMaxTime)
    {
        m_nMaxTime = timeStamp;
    }

    if (timeStamp < m_nMinTime)
    {
        m_nMinTime = timeStamp;
    }

    if (value > m_fMaxValue)
    {
        m_fMaxValue = value;
        m_dInvMaxValue = 1 / (double)m_fMaxValue;
    }

    m_samplePairs[timeStamp] = value;
}

