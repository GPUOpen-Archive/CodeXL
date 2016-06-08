//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acColorSampleBox.cpp
///
//==================================================================================

//------------------------------ acColorSampleBox.cpp ------------------------------

// Qt:
#include <QVBoxLayout>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>

// Local:
#include <AMDTApplicationComponents/Include/acColorSampleBox.h>

// BG xpm:
#include <AMDTApplicationComponents/Include/res/icons/image_background_pattern.xpm>

// ---------------------------------------------------------------------------
// Name:        acColorSampleBox::acColorSampleBox
// Description: Constructor
// Arguments:   QWidget* pParent
//              QSize size
// Author:      Sigal Algranaty
// Date:        21/6/2012
// ---------------------------------------------------------------------------
acColorSampleBox::acColorSampleBox(QWidget* pParent, QSize size)
    : QWidget(pParent),
      m_pBackgroundPixmap(NULL), m_isColorSet(false), m_currentColor(Qt::white)
{
    // Set my size:
    setFixedWidth(size.width());
    setFixedHeight(size.height());
    resize(size);

    setMouseTracking(false);

    // Generate the color sample background
    m_pBackgroundPixmap = new QPixmap(image_background_pattern);

}

// ---------------------------------------------------------------------------
// Name:        acColorSampleBox::~acColorSampleBox
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        21/6/2012
// ---------------------------------------------------------------------------
acColorSampleBox::~acColorSampleBox()
{
    // If background bitmap exists, release it
    if (m_pBackgroundPixmap)
    {
        // Release the object memory
        delete m_pBackgroundPixmap;

        // Point the object to NULL
        m_pBackgroundPixmap = NULL;
    }
}

// ---------------------------------------------------------------------------
// Name:        acColorSampleBox::paintEvent
// Description: Is called when the widget is painted
// Arguments:   QPaintEvent* pEvent
// Author:      Sigal Algranaty
// Date:        21/6/2012
// ---------------------------------------------------------------------------
void acColorSampleBox::paintEvent(QPaintEvent* pEvent)
{
    GT_UNREFERENCED_PARAMETER(pEvent);

    // Paint the background pixmap, and on top of it, draw a rectangle with the
    // background color:
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);


    // Create the painted rectangles:
    QRect borderRect = rect();

    // Color rect:
    QPoint topLeft, bottomRight;
    topLeft.setX(rect().left() + 1);
    topLeft.setY(rect().top() + 1);

    bottomRight.setX(rect().right() - 1);
    bottomRight.setY(rect().bottom() - 1);
    QRect colorRect(topLeft, bottomRight);

    GT_IF_WITH_ASSERT(m_pBackgroundPixmap != NULL)
    {
        // Border rect:
        p.fillRect(borderRect, Qt::black);

        p.drawPixmap(colorRect, *m_pBackgroundPixmap);

        if (m_isColorSet)
        {
            p.fillRect(colorRect, m_currentColor);
        }

    }


    p.save();

    p.restore();
}

// ---------------------------------------------------------------------------
// Name:        acColorSampleBox::clearColourSample
// Description: Clear the current color sample
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        21/6/2012
// ---------------------------------------------------------------------------
void acColorSampleBox::clearColourSample()
{
    m_isColorSet = false;
}


// ---------------------------------------------------------------------------
// Name:        acColorSampleBox::setColourSample
// Description: Sets the current color
// Arguments:   gtUByte red
//              gtUByte green
//              gtUByte blue
//              gtUByte alpha
// Author:      Sigal Algranaty
// Date:        21/6/2012
// ---------------------------------------------------------------------------
void acColorSampleBox::setColourSample(const QRgb& color)
{
    // Set the color:
    m_currentColor = QColor::fromRgba(color);

    // Mark the color as set:
    m_isColorSet = true;

    repaint();
}

