//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acDisplay.cpp
///
//==================================================================================

//------------------------------ acDisplay.cpp ------------------------------

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTApplicationComponents/Include/acDisplay.h>


// ---------------------------------------------------------------------------
// Name:        acGetApplicationDPI
// Description: Gets the main screen DPI and returns it. The value is cached
//              so that it won't change unexpectedly at runtime.
// Author:      Uri Shomroni
// Date:        22/9/2014
// ---------------------------------------------------------------------------
unsigned int acGetApplicationDPI()
{
    static unsigned int retVal = AC_BASE_DPI;

    static bool onlyOnce = true;

    if (onlyOnce)
    {
        onlyOnce = false;

        // Get the screen list
        QList<QScreen*> screensList = QGuiApplication::screens();
        GT_IF_WITH_ASSERT(!screensList.empty())
        {
            // Main screen turn on
            QScreen* pMainScreen = NULL;
            QList<QScreen*>::const_iterator screenIter = screensList.cbegin();
            QList<QScreen*>::const_iterator endIter = screensList.cend();

            while (NULL == pMainScreen && screenIter != endIter)
            {
                pMainScreen = *screenIter++;
            }

            GT_IF_WITH_ASSERT(NULL != pMainScreen)
            {
                qreal dpiX = pMainScreen->logicalDotsPerInchX();
                qreal dpiY = pMainScreen->logicalDotsPerInchY();
                GT_IF_WITH_ASSERT(0.0 < dpiX)
                {
                    retVal = (unsigned int)dpiX;
                    GT_ASSERT((unsigned int)dpiY == (unsigned int)dpiX);
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acScalePixelSizeToDisplayDPI
// Description: Scales a pixel size according to the display DPI
// Author:      Uri Shomroni
// Date:        22/9/2014
// ---------------------------------------------------------------------------
unsigned int acScalePixelSizeToDisplayDPI(unsigned int baseSize)
{
    unsigned int retVal = (acGetApplicationDPI() * baseSize) / AC_BASE_DPI;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acScaleSignedPixelSizeToDisplayDPI
// Description: Scales a pixel size according to the display DPI
// Author:      Uri Shomroni
// Date:        23/9/2014
// ---------------------------------------------------------------------------
int acScaleSignedPixelSizeToDisplayDPI(int baseSize)
{
    int retVal = ((int)acGetApplicationDPI() * baseSize) / AC_BASE_DPI;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acGetDisplayScalePercent
// Description: Calculates the main screen scaling and returns it. The value is cached
//              so that it won't change unexpectedly at runtime.
// Author:      Uri Shomroni
// Date:        22/9/2014
// ---------------------------------------------------------------------------
unsigned int acGetDisplayScalePercent()
{
    static unsigned int retVal = 100;

    bool onlyOnce = true;

    if (onlyOnce)
    {
        onlyOnce = false;
        retVal = acScalePixelSizeToDisplayDPI(100);
    }

    return retVal;
}

