//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apFileType.h
///
//==================================================================================

//------------------------------ apFileType.h ------------------------------

#ifndef __APFILETYPE
#define __APFILETYPE

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>


// Describes file types:
typedef enum
{
    AP_BMP_FILE,    // BMP file - Windows or OS/2 Bitmap File.
    AP_TIFF_FILE,   // TIFF file - Tagged Image File Format.
    AP_JPEG_FILE,   // JPEG file - Independent JPEG Group file format.
    AP_PNG_FILE,    // PNG file - Portable Network Graphics.
    AP_CSV_FILE,    // CSV file - Spreadsheet file (Comma separated values).
    AP_HTML_FILE    // HTML file - Hyper-Text Markup Language (The scripting language of the World Wide Web).
} apFileType;


AP_API bool apFileTypeToFileExtensionString(apFileType fileType, gtString& extensionString);
AP_API bool apFileExtensionStringToFileType(const gtString& extensionString, apFileType& fileType);
AP_API bool apFileTypeWildcard(apFileType fileType, gtString& wildcardString);




#endif  // __APFILETYPE
