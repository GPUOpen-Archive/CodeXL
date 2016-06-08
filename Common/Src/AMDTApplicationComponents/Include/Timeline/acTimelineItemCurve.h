//==================================================================================
// Copyright (c) 2011 - 2016  , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acTimelineItemCurve.h
///
//==================================================================================

#ifndef _ACTIMELINEITEMCURVE_H_
#define _ACTIMELINEITEMCURVE_H_


#include <QMap>
#include <QList>
#include <QWidget>
#include <QPainter>


// Local:
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>
#include <AMDTApplicationComponents/Include/Timeline/acTimelineItem.h>

/// class that draw a line graph along the timeline using a set of points
class AC_API acTimelineItemCurve : public acTimelineItem
{
    Q_OBJECT

public:
    acTimelineItemCurve();

    virtual void draw(QPainter& painter, const int branchRowTop, const int branchHeight);

    void addTimestampValuePair(quint64 timeStamp, float value);

    void setSolid(const bool newValue) { m_bSolid = newValue; }

private:

    float m_fMaxValue;
    double m_dInvMaxValue;
    quint64 m_nMinTime;
    quint64 m_nMaxTime;
    QRect m_clipBound;
    bool m_bSolid;

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4189)
#pragma warning(disable : 4800)
#pragma warning(disable : 4251)
#pragma warning(disable : 4512)
#endif
    QVector<QPoint> m_pointBuffer;
    QMap<quint64, float> m_samplePairs; ///< map of the samples to plot along the curve
#ifdef _WIN32
#pragma warning(pop)
#endif

};

#endif // _ACTIMELINEITEMCURVE_H_
