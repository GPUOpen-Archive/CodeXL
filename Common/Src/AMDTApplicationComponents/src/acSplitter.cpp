//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acSplitter.cpp
///
//==================================================================================

//------------------------------ acSplitter.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acSplitter.h>

#define AC_Str_SolidBGStyleSplitterStyle    "QSplitter::handle {background-color:#%1;}"

#define AC_Str_VerticalSplitterStyle          "QSplitter::handle {" \
    "background-color: qlineargradient(spread : pad, x1 : 0, y1 : 0, x2 : 0, y2 : 1," \
    "stop : 0 rgba(255, 255, 255, 0)," \
    "stop : 0.407273 rgba(200, 200, 200, 255)," \
    "stop : 0.4825 rgba(150, 150, 155, 235)," \
    "stop : 1 rgba(255, 255, 255, 0));" \
    "image: url(: / images / splitter.png);" \
    "}"

acSplitter::acSplitter(QWidget* pParent) : QSplitter(pParent)
{

}

acSplitter::acSplitter(Qt::Orientation orientation, QWidget* pParent) : QSplitter(orientation, pParent)
{
    // Set the splitter style sheet:
    QString cssBGStr = QString(AC_Str_SolidBGStyleSplitterStyle).arg(acGetSystemDefaultBackgroundColorAsHexQString());
    setStyleSheet(cssBGStr);
}

void acSplitter::MoveSplitter(int index, int position)
{
    // Call base class implementation:
    QSplitter::moveSplitter(index, position);
}
