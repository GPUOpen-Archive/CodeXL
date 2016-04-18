//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTImage.cpp
///
//==================================================================================

//------------------------------ AMDTImage.cpp ----------------------------

#include <string.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#if defined(__linux__)
    #include <unistd.h>
#endif  // #if defined(__linux__)


#include "AMDTMisc.h"
#include "AMDTDebug.h"
#include "AMDTImage.h"

#define SWAP(X, Y) do { (X) = (X)^(Y); (Y) = (X)^(Y); (X) = (X)^(Y); } while(0)

// ---------------------------------------------------------------------------
// Name:        AMDTImage::AMDTImage
// Description: Constructor.
// ---------------------------------------------------------------------------
AMDTImage::AMDTImage(
    const char* floc) :
    _data(NULL)
{
    ASSERT((strlen(floc) < MAX_PATH_LEN), "Image location path too long (%d)", (int)strlen(floc));
    strcpy(_fileLocation, floc);
}

// ---------------------------------------------------------------------------
// Name:        AMDTImage::~AMDTImage
// Description: Destructor.
// ---------------------------------------------------------------------------
AMDTImage::~AMDTImage()
{
    delete[] _data;
}

// ---------------------------------------------------------------------------
// Name:        AMDTBMPImage::AMDTBMPImage
// Description: Constructor.
// ---------------------------------------------------------------------------
AMDTBMPImage::AMDTBMPImage(
    const char* floc) :
    AMDTImage(floc)
{
}

// ---------------------------------------------------------------------------
// Name:        AMDTBMPImage::readImage
// Description: read the data of a 24-bit BMP file into memory
// ---------------------------------------------------------------------------
void AMDTBMPImage::readImage(
    const char* floc)
{
    ASSERT((strlen(floc) < MAX_PATH_LEN), "Image location path too long (%d)", (int)strlen(floc));
    strcpy(_fileLocation, floc);
    readImage();
}

// ---------------------------------------------------------------------------
// Name:        AMDTBMPImage::readImage
// Description: read the data of a BMP file into memory
// ---------------------------------------------------------------------------
void AMDTBMPImage::readImage()
{
    FILE*                pFile = NULL;
    unsigned int        rdret;

    ASSERT(((pFile = fopen(_fileLocation, "rb")) != NULL),                                                      "ERROR: file \"%s\" not found\n", _fileLocation);

    // Seek forward to width and height info
    fseek(pFile, 14, SEEK_CUR);

    // Read image info and assert limitations
    ASSERT(((rdret = fread(&_BITMAPINFOHEADER, sizeof(struct AMDTBMPImage::BITMAPINFOHEADER), 1, pFile)) == 1), "Couldn't read width from %s.\n", _fileLocation);
    ASSERT((_BITMAPINFOHEADER.nplanes == 1) ,                                                                       "Plane count from %s is not 1: %u\n", _fileLocation, _BITMAPINFOHEADER.nplanes);
    ASSERT((_BITMAPINFOHEADER.bitspp == SUPPORTED_BPP),                                                         "BPP from %s is not %d: %u\n", _fileLocation, SUPPORTED_BPP,  _BITMAPINFOHEADER.bitspp);

    int dloc = 0;
    int rdSiz = (1 << 20) + 4;
    int sum = 0;
    _data = new u8[_BITMAPINFOHEADER.bmp_bytesz + 4];

    do
    {
        dloc = fread(&_data[dloc], sizeof(char), rdSiz, pFile);
        sum += dloc;
    }
    while (!feof(pFile) && !ferror(pFile));
}

// ---------------------------------------------------------------------------
// Name:        AMDTBMPImage::toOGLFmt
// Description: Convert the BMP to openGL format
// ---------------------------------------------------------------------------
void AMDTBMPImage::toOGLFmt()
{
    ASSERT((_data != NULL),     "ERROR: No BMP data");
    ASSERT((getDepth() == 32),  "ERROR: Only supporting 32bit BMP images, have %d", getDepth());
    u8* data = new u8[_BITMAPINFOHEADER.bmp_bytesz + 4];

    int wLen = getWidth() * (getDepth() / 8);

    for (int h = 0 ; h < getHeight() ; h++)
    {
        for (int w = 0 ; w < getWidth() ; w++)
        {
            ((u32*)&data[wLen * (getHeight() - h - 1)])[getWidth() - w] = ((u32*)&_data[wLen * h])[w];
        }
    }

    delete[] _data;
    _data = data;
}
