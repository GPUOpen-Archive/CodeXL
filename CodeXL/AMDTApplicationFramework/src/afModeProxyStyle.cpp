//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afModeProxyStyle.cpp
///
//==================================================================================

// Qt
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTApplicationFramework/src/afModeProxyStyle.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>

// ---------------------------------------------------------------------------
// Name:        afModeProxyStyle::afModeProxyStyle
// Description: constructor
// Return Val:
// Author:      Gilad Yarnitzky
// Date:        18/7/2012
// ---------------------------------------------------------------------------
afModeProxyStyle::afModeProxyStyle(QStyle* pOldStyle) : m_pOriginalStyle(pOldStyle)
{

}


// ---------------------------------------------------------------------------
// Name:        afModeProxyStyle::~afModeProxyStyle
// Description: destructor
// Return Val:
// Author:      Gilad Yarnitzky
// Date:        18/7/2012
// ---------------------------------------------------------------------------
afModeProxyStyle::~afModeProxyStyle()
{

}


// ---------------------------------------------------------------------------
// Name:        afModeProxyStyle::drawComplexControl
// Description: Draw the control
// Author:      Gilad Yarnitzky
// Date:        18/7/2012
// ---------------------------------------------------------------------------
void afModeProxyStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const
{
    // Use the default style to draw the control
    if (nullptr != m_pOriginalStyle)
    {
        m_pOriginalStyle->drawComplexControl(control, option, painter, widget);
    }

    // Draw the extra information if needed:
    const QToolButton* pToolButton = dynamic_cast<QToolButton*>((QWidget*)widget);

    if (nullptr != pToolButton)
    {
        QAction* pButtonAction = pToolButton->defaultAction();
        GT_IF_WITH_ASSERT(nullptr != pButtonAction)
        {
            if (pButtonAction->isChecked() && !pButtonAction->isEnabled())
            {
                // Draw the extra frame if it is checked and disabled:
                QRect widgetRect(QPoint(0, 0), widget->size());
                widgetRect.adjust(0, 0, -1, -1);

                QPen drawingPen(Qt::gray);
                drawingPen.setWidth(1);

                painter->setPen(drawingPen);
                painter->drawRoundedRect(widgetRect, 5, 5);
            }
        }
    }
}


