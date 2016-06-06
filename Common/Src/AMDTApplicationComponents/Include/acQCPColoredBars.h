//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acQCPColoredBars.h
///
//==================================================================================


#ifndef ACQCPCOLOREDBARS
#define ACQCPCOLOREDBARS

#include <qcustomplot.h>
//#include <qstring.h>
//#include <qvector.h>
#include <QtWidgets>
#include <QtCore>

#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>

class AC_API acQCPColoredBars : public QCPBars
{
public:
    acQCPColoredBars(QCPAxis* keyAxis, QCPAxis* valueAxis, const QVector<QColor>& barsData);

private:
    virtual void draw(QCPPainter* painter);
    QVector<QColor> m_pBarColors;
};

#endif //ACQCPCOLOREDBARS