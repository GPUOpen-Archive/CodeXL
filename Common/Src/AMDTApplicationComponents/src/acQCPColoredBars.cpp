//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acQCPColoredBars.cpp
///
//==================================================================================


#include <AMDTApplicationComponents/Include/acQCPColoredBars.h>
#include <AMDTBaseTools/Include/gtString.h>

acQCPColoredBars::acQCPColoredBars(QCPAxis* keyAxis, QCPAxis* valueAxis, const QVector<QColor>& colors)
    : QCPBars(keyAxis, valueAxis)
{
    m_pBarColors = colors;
}

void acQCPColoredBars::draw(QCPPainter* painter)
{
    if (!mKeyAxis || !mValueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return; }

    if (mData->isEmpty()) { return; }

    QCPBarDataMap::const_iterator it;
    int i = 0;

    for (it = mData->constBegin(); it != mData->constEnd(); ++it, ++i)
    {
        // skip bar if not visible in key axis range:
        if (it.key() + mWidth * 0.5 < mKeyAxis.data()->range().lower || it.key() - mWidth * 0.5 > mKeyAxis.data()->range().upper)
        {
            continue;
        }

        // check data validity if flag set:
#ifdef QCUSTOMPLOT_CHECK_DATA

        if (QCP::isInvalidData(it.value().key, it.value().value))
        {
            qDebug() << Q_FUNC_INFO << "Data point at" << it.key() << "of drawn range invalid." << "Plottable name:" << name();
        }

#endif
        QPolygonF barPolygon = getBarPolygon(it.key(), it.value().value);

        // draw bar fill:
        if (mainBrush().style() != Qt::NoBrush && mainBrush().color().alpha() != 0)
        {
            applyFillAntialiasingHint(painter);
            painter->setPen(Qt::NoPen);
            QColor color = m_pBarColors[i];
            painter->setBrush(QBrush(color, Qt::SolidPattern));
            painter->drawPolygon(barPolygon);
        }

        // draw bar line:
        if (mainPen().style() != Qt::NoPen && mainPen().color().alpha() != 0)
        {
            applyDefaultAntialiasingHint(painter);
            painter->setPen(mainPen());
            painter->setBrush(Qt::NoBrush);
            painter->drawPolyline(barPolygon);
        }

        // set value label above bar
        double d = it->value;
        gtString gtStr;
        gtStr.appendFormattedString(L"%.2f", d);
        gtStr.addThousandSeperators();
        QString str(gtStr.asASCIICharArray());
        int halfStrWidth = painter->fontMetrics().boundingRect(str).width() / 2;
        int x = ((barPolygon[1]).x() + (barPolygon[2]).x()) / 2 - halfStrWidth;
        int y = (barPolygon[1]).y() - 2;
        painter->setPen(Qt::black);
        painter->drawText(x, y, str);
    }
}