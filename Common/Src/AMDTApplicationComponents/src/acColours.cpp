//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acColours.cpp
///
//==================================================================================

//------------------------------ acColours.cpp ------------------------------

// ----------------------------------------------------------------------------------
// File Name:            acColours
// General Description:
//   Contains default colors values definitions.
//
// Author:               Yaki Tebeka
// Creation Date:        12/4/2005
// ----------------------------------------------------------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTApplicationComponents/Include/acColours.h>

// Initialize color values:
// Graphic remedy branding:
// QColor acQGREMEDY_ORANGE_COLOUR(255, 51, 0, 0xff);

// AMD Nov 2012 branding:
// QColor acQAMD_GREEN_COLOUR(0, 153, 102, 0xff);                      // #009966
// QColor acQAMD_RED_COLOUR(211, 25, 25, 0xff);                        // #D31919
// QColor acQAMD_ORANGE_COLOUR(252, 101, 0, 0xff);                     // #FC6500
// QColor acQAMD_PURPLE_COLOUR(118, 125, 197, 0xff);                   // #767DC5
// QColor acQAMD_GRAY_COLOUR(128, 127, 130, 0xff);                     // #807F82
// QColor acQAMD_AQUA_COLOUR(0, 130, 155, 0xff);                       // #00829B
// QColor acQAMD_YELLOW_COLOUR(252, 209, 22,  0xff);                   // #FCD116

// AMD Sep 2013 branding:
QColor acQAMD_RED_PRIMARY_COLOUR(237, 28, 36, 0xff);                // #ED1C24
QColor acQAMD_PURPLE_PRIMARY_COLOUR(129, 41, 144, 0xff);            // #812990
QColor acQAMD_CYAN_PRIMARY_COLOUR(0, 170, 181, 0xff);               // #00AAB5
QColor acQAMD_GREEN_PRIMARY_COLOUR(166, 203, 57, 0xff);             // #A6CB39
QColor acQAMD_ORANGE_PRIMARY_COLOUR(242, 101, 34, 0xff);            // #F26522
QColor acQAMD_RED_OVERLAP_COLOUR(177, 17, 22, 0xff);                // #B11116
QColor acQAMD_PURPLE_OVERLAP_COLOUR(77, 32, 122, 0xff);             // #4D207A
QColor acQAMD_CYAN_OVERLAP_COLOUR(0, 124, 151, 0xff);               // #007C97
QColor acQAMD_GREEN_OVERLAP_COLOUR(103, 174, 62, 0xff);             // #67AE3E
QColor acQAMD_ORANGE_OVERLAP_COLOUR(199, 74, 27, 0xff);             // #C74A1B
QColor acQAMD_GRAY1_COLOUR(99, 100, 102, 0xff);                     // #636466
QColor acQAMD_GRAY2_COLOUR(157, 159, 162, 0xff);                    // #9D9FA2
QColor acQAMD_GRAY3_COLOUR(199, 200, 202, 0xff);                    // #C7C8CA
QColor acQAMD_GRAY_LIGHT_COLOUR(248, 250, 252, 0xff);               // #F8FAFC
QColor acQAMD_CYAN_SELECTION_BKG_COLOUR(166, 183, 189, 0xff);       // #A6B7BD

AC_API const QColor& acGetAMDColorScaleColor(acAMDColorHue hue, unsigned int indexInSeries)
{
    switch (hue)
    {
        case AC_AMD_RED:
        {
            indexInSeries %= 4;

            if (0 == indexInSeries)
            {
                return acQAMD_RED_OVERLAP_COLOUR;
            }
            else if (1 == indexInSeries)
            {
                static const QColor amdMidRed(207, 22, 29, 0xff);
                return amdMidRed;
            }
            else if (2 == indexInSeries)
            {
                return acQAMD_RED_PRIMARY_COLOUR;
            }
            else
            {
                static const QColor amdGreyRed(197, 93, 98, 0xff);
                return amdGreyRed;
            }
        }
        break;

        case AC_AMD_PURPLE:
        {
            indexInSeries %= 4;

            if (0 == indexInSeries)
            {
                return acQAMD_PURPLE_OVERLAP_COLOUR;
            }
            else if (1 == indexInSeries)
            {
                static const QColor amdMidPurple(103, 36, 133, 0xff);
                return amdMidPurple;
            }
            else if (2 == indexInSeries)
            {
                return acQAMD_PURPLE_PRIMARY_COLOUR;
            }
            else
            {
                static const QColor amdGreyPurple(143, 100, 152, 0xff);
                return amdGreyPurple;
            }
        }
        break;

        case AC_AMD_CYAN:
        {
            indexInSeries %= 4;

            if (0 == indexInSeries)
            {
                return acQAMD_CYAN_OVERLAP_COLOUR;
            }
            else if (1 == indexInSeries)
            {
                static const QColor amdMidCyan(0, 147, 166, 0xff);
                return amdMidCyan;
            }
            else if (2 == indexInSeries)
            {
                return acQAMD_CYAN_PRIMARY_COLOUR;
            }
            else
            {
                static const QColor amdGreyCyan(78, 164, 171, 0xff);
                return amdGreyCyan;
            }
        }
        break;

        case AC_AMD_GREEN:
        {
            indexInSeries %= 4;

            if (0 == indexInSeries)
            {
                // The green color is fairly light, use a darker version instead of the lighter version:
                static const QColor amdDarkGreen(51, 87, 31, 0xff);
                return amdDarkGreen;
            }
            else if (1 == indexInSeries)
            {
                return acQAMD_GREEN_OVERLAP_COLOUR;
            }
            else if (2 == indexInSeries)
            {
                static const QColor amdMidGreen(134, 188, 59, 0xff);
                return amdMidGreen;
            }
            else
            {
                return acQAMD_GREEN_PRIMARY_COLOUR;
            }
        }
        break;

        case AC_AMD_ORANGE:
        {
            indexInSeries %= 4;

            if (0 == indexInSeries)
            {
                return acQAMD_ORANGE_OVERLAP_COLOUR;
            }
            else if (1 == indexInSeries)
            {
                static const QColor amdMidOrange(220, 87, 30, 0xff);
                return amdMidOrange;
            }
            else if (2 == indexInSeries)
            {
                return acQAMD_ORANGE_PRIMARY_COLOUR;
            }
            else
            {
                static const QColor amdGreyOrange(199, 130, 97, 0xff);
                return amdGreyOrange;
            }
        }
        break;

        case AC_AMD_GRAY:
        {
            indexInSeries %= 4;

            if (0 == indexInSeries)
            {
                return acQAMD_GRAY1_COLOUR;
            }
            else if (1 == indexInSeries)
            {
                return acQAMD_GRAY3_COLOUR;
            }
            else if (2 == indexInSeries)
            {
                return acQAMD_GRAY2_COLOUR;
            }
            else
            {
                return acQAMD_GRAY_LIGHT_COLOUR;
            }
        }
        break;

        default:
            break;
    }

    static const QColor failureColor(Qt::black);
    return failureColor;
}

AC_API const QColor& acGetCodeXLColorScaleColor(acCodeXLColorHue hue, unsigned int indexInSeries)
{
    switch (hue)
    {
        case AC_CODEXL_BLUE:
        {
            indexInSeries %= 5;

            if (0 == indexInSeries)
            {
                static const QColor blue1(47, 63, 79, 0xff);
                return blue1;
            }
            else if (1 == indexInSeries)
            {
                static const QColor blue2(15, 95, 159, 0xff);
                return blue2;
            }
            else if (2 == indexInSeries)
            {
                static const QColor blue3(79, 95, 127, 0xff);
                return blue3;
            }
            else if (3 == indexInSeries)
            {
                static const QColor blue4(0, 79, 143, 0xff);
                return blue4;
            }
            else
            {
                static const QColor blue5(47, 79, 111, 0xff);
                return blue5;
            }
        }
        break;

        case AC_CODEXL_MAGENTA:
        {
            indexInSeries %= 4;

            if (0 == indexInSeries)
            {
                static const QColor magenta1(239, 111, 207, 0xff);
                return magenta1;
            }
            else if (1 == indexInSeries)
            {
                static const QColor magenta2(255, 223, 239, 0xff);
                return magenta2;
            }
            else if (2 == indexInSeries)
            {
                static const QColor magenta3(239, 0, 159, 0xff);
                return magenta3;
            }
            else
            {
                static const QColor magenta5(255, 191, 223, 0xff);
                return magenta5;
            }
        }
        break;

        default:
            break;
    }

    static const QColor failureColor(Qt::black);
    return failureColor;
}

#define AC_WARNING_SCALE_COLOR_COUNT 10
AC_API const QColor& acGetWarningScaleColor(int percentage)
{
    static QColor warningColors[AC_WARNING_SCALE_COLOR_COUNT];
    static bool firstTime = true;

    if (firstTime)
    {
        warningColors[0] = acQGREEN_WARNING_COLOUR;
        warningColors[1] = acQGREEN_WARNING_COLOUR;
        warningColors[2] = acQGREEN_WARNING_COLOUR;
        warningColors[3] = acQYELLOW_WARNING_COLOUR;
        warningColors[4] = acQYELLOW_WARNING_COLOUR;
        warningColors[5] = acQYELLOW_WARNING_COLOUR;
        warningColors[6] = acQORANGE_WARNING_COLOUR;
        warningColors[7] = acQORANGE_WARNING_COLOUR;
        warningColors[8] = acQORANGE_WARNING_COLOUR;
        warningColors[9] = acQRED_WARNING_COLOUR;
        acBlendIntoWithAlpha(warningColors[1], warningColors[3], 85);
        acBlendIntoWithAlpha(warningColors[2], warningColors[3], 170);
        acBlendIntoWithAlpha(warningColors[4], warningColors[6], 85);
        acBlendIntoWithAlpha(warningColors[5], warningColors[6], 170);
        acBlendIntoWithAlpha(warningColors[7], warningColors[9], 85);
        acBlendIntoWithAlpha(warningColors[8], warningColors[9], 170);

        firstTime = false;
    }

    GT_ASSERT((0 <= percentage) && (100 >= percentage));

    if (0 > percentage)
    {
        percentage = 0;
    }
    else if (100 < percentage)
    {
        percentage = 100;
    }

    static const int halfRange = 100 / 2;
    static const int stepSize10 = 1000 / AC_WARNING_SCALE_COLOR_COUNT;
    int idx = (((percentage - halfRange) * 1000 / (1000 + stepSize10 + 1)) + halfRange) / AC_WARNING_SCALE_COLOR_COUNT; // This gives a correct gamut of colors

    return warningColors[idx];
}

// Other colors:
QColor acQGREEN_WARNING_COLOUR(51, 204, 51, 0xff);
QColor acQYELLOW_WARNING_COLOUR(255, 255, 102, 0xff);               // Same color as used in CodeXLAppCode/res/icons/warning_icon_yellow.xpm
QColor acQORANGE_WARNING_COLOUR(255, 204, 51, 0xff);                // Same color as used in CodeXLAppCode/res/icons/warning_icon_orange.xpm
QColor acQRED_WARNING_COLOUR(255, 102, 51, 0xff);                   // Same color as used in CodeXLAppCode/res/icons/warning_icon_red.xpm
QColor acQLIST_HIGHLIGHT_COLOUR(255, 255, 112, 0xff);
QColor acQLIST_EDITABLE_ITEM_COLOR(120, 120, 120, 0xff);
QColor acQGREY_TEXT_COLOUR(120, 120, 120, 0xff);
QColor acQRAW_FILE_NOT_IN_SCOPE_COLOR(251, 221, 64, 0xff);
QColor acQRAW_FILE_ABOVE_RANGE_COLOR(175, 135, 249, 0xff);
QColor acQRAW_FILE_BELOW_RANGE_COLOR(244, 54, 76, 0xff);
QColor acQRAW_FILE_TOP_RANGE_COLOR(255, 255, 255, 0xff);
QColor acGRAY_BG_COLOR(188, 200, 216, 0xff);
QColor acYELLOW_INFO_COLOUR(255, 255, 183, 0xff);
QColor acRED_NUMBER_COLOUR(128, 0, 0, 0xff);
QColor acDARK_PURPLE(0x48, 0x00, 0xff, 0xff);
QColor acLIGHT_PURPLE(0x7d, 0x18, 0xff, 0xe5);
QColor acDARK_GREEN(0x00, 0x7f, 0x00, 0xff);
QColor acLIGHT_GREEN(0x00, 0xff, 0x21, 0xff);

QColor acPATH_SELECTED_COLOR(255, 200, 0, 0xff);
QColor acPATH_HOVER_COLOR(252, 90, 0, 0xff);

AC_API void acReadableNegativeColor(QColor& color)
{
    // Negate the color:
    int r = 255 - color.red();
    int g = 255 - color.green();
    int b = 255 - color.blue();

    // For mid-gray colors, the negative is too close to the original. Use black instead.
    if ((135 < r) || (119 > r) ||
        (135 < g) || (119 > g) ||
        (135 < b) || (119 > b))
    {
        color.setRed(r);
        color.setGreen(g);
        color.setBlue(b);
    }
    else
    {
        color = Qt::black;
    }
}

AC_API void acBlendInto(QColor& base, const QColor& top)
{
    acBlendIntoWithAlpha(base, top, top.alpha());
}

AC_API void acBlendIntoWithAlpha(QColor& base, const QColor& top, int topA)
{
    int topAc = 255 - topA;
    int newR = (base.red() * topAc + top.red() * topA) / 255;
    int newG = (base.green() * topAc + top.green() * topA) / 255;
    int newB = (base.blue() * topAc + top.blue() * topA) / 255;

    base.setRgb(newR, newG, newB, base.alpha());
}
