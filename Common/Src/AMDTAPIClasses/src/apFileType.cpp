//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apFileType.cpp
///
//==================================================================================

//------------------------------ apFileType.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTAPIClasses/Include/apFileType.h>
#include <AMDTAPIClasses/Include/apStringConstants.h>


// ---------------------------------------------------------------------------
// Name:        apFileTypeToFileExtensionString
// Description: Translates apFileType to a file extension string.
// Arguments:   fileType - The input file type.
//              extensionString - The output extension string.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        18/1/2005
// ---------------------------------------------------------------------------
bool apFileTypeToFileExtensionString(apFileType fileType, gtString& extensionString)
{
    bool retVal = true;
    extensionString.makeEmpty();

    switch (fileType)
    {
        case AP_BMP_FILE:
            extensionString = L"bmp";
            break;

        case AP_TIFF_FILE:
            extensionString = L"tif";
            break;

        case AP_JPEG_FILE:
            extensionString = L"jpg";
            break;

        case AP_PNG_FILE:
            extensionString = L"png";
            break;

        case AP_CSV_FILE:
            extensionString = L"csv";
            break;

        case AP_HTML_FILE:
            extensionString = L"html";
            break;

        default:
            // Unknown file type:
            retVal = false;
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apFileTypeWildcard
// Description: Translates apFileType to a wildcard string that can be
//              used in File Dialogs.
// Arguments:   fileType - The input file type.
//              wildcardString - The output wildcard string.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        5/1/2008
// ---------------------------------------------------------------------------
bool apFileTypeWildcard(apFileType fileType, gtString& wildcardString)
{
    bool retVal = true;
    wildcardString.makeEmpty();

    switch (fileType)
    {
        case AP_BMP_FILE:
            wildcardString = AP_STR_BmpWildcard;
            break;

        case AP_TIFF_FILE:
            wildcardString = AP_STR_TiffWildcard;
            break;

        case AP_JPEG_FILE:
            wildcardString = AP_STR_JpegWildcard;
            break;

        case AP_PNG_FILE:
            wildcardString = AP_STR_PngWildcard;
            break;

        case AP_CSV_FILE:
            wildcardString = AP_STR_CsvWildcard;
            break;

        default:
        {
            GT_ASSERT_EX(false, L"Unknown file type!");

            // Unknown file type:
            retVal = false;
        }
        break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apFileExtensionStringToFileType
// Description: Translates a file extension string to apFileType.
// Arguments:   fileType - The output file type.
//              extensionString - The input extension string.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        24/1/2005
// ---------------------------------------------------------------------------
bool apFileExtensionStringToFileType(const gtString& extensionString, apFileType& fileType)
{
    bool retVal = true;

    gtString tmpString = extensionString;
    tmpString.toLowerCase();

    if (tmpString == L"bmp")
    {
        fileType = AP_BMP_FILE;
    }
    else if (tmpString == L"tif")
    {
        fileType = AP_TIFF_FILE;
    }
    else if (tmpString == L"jpg")
    {
        fileType = AP_JPEG_FILE;
    }
    else if (tmpString == L"png")
    {
        fileType = AP_PNG_FILE;
    }
    else if (tmpString == L"csv")
    {
        fileType = AP_CSV_FILE;
    }
    else if ((tmpString == L"html") || (tmpString == L"htm"))
    {
        fileType = AP_HTML_FILE;
    }

    return retVal;
}
