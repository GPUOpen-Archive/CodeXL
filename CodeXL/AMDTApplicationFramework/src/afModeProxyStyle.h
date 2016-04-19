//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afModeProxyStyle.h
///
//==================================================================================

#ifndef __AFMODEPROXYSTYLE_H
#define __AFMODEPROXYSTYLE_H

// Qt
#include <QProxyStyle>

class afModeProxyStyle : public QProxyStyle
{
public:
    afModeProxyStyle(QStyle* pOldStyle);
    virtual ~afModeProxyStyle();

    virtual void    drawComplexControl(ComplexControl control, const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget = 0) const;
    QStyle* m_pOriginalStyle;
};

#endif //__AFMODEPROXYSTYLE_H

