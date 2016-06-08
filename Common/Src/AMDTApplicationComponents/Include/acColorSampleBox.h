//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acColorSampleBox.h
///
//==================================================================================

//------------------------------ acColorSampleBox.h ------------------------------

#ifndef __ACCOLORSAMPLEBOX
#define __ACCOLORSAMPLEBOX

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

// Local:
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:          acColorSampleBox : public QWidget
// General Description: A box with a background, painted with a color
// Author:              Sigal Algranaty
// Creation Date:       10/6/2012
// ----------------------------------------------------------------------------------
class AC_API acColorSampleBox : public QWidget
{
public:
    // Constructor:
    acColorSampleBox(QWidget* parent, QSize size);

    // Destructor:
    ~acColorSampleBox();

public:

    // Public access functions:
    void setColourSample(const QRgb& color);
    void clearColourSample();

protected:

    // Override QWidget:
    virtual void paintEvent(QPaintEvent* pEvent);

private:

    // This QPixmap object will hold the color sample background
    QPixmap* m_pBackgroundPixmap;

    // Is the background color set:
    bool m_isColorSet;

    // The current background color:
    QColor m_currentColor;

};

#endif  // __acColorSampleBox
