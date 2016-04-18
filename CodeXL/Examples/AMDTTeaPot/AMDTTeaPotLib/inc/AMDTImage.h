//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTImage.h
///
//==================================================================================

//------------------------------ AMDTImage.h ----------------------------

#ifndef __AMDTIMAGE_H
#define __AMDTIMAGE_H

#include <string.h>
#include <stdio.h>

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef          char  s8;
typedef          short s16;
typedef          int   s32;

struct AMDTImage
{
public:
    AMDTImage(const char*);
    virtual ~AMDTImage();
    virtual void readImage(const char*) = 0;
    virtual void readImage()             = 0;

    u8* _data;
    char _fileLocation[MAX_PATH_LEN]; // this should be enough
};


struct AMDTBMPImage: public AMDTImage
{
#define SUPPORTED_BPP   (32)
#define BPP             ((4)*sizeof(char))
public:
    AMDTBMPImage(const char*);
    ~AMDTBMPImage() {}
    void readImage(const char*);
    void readImage();
    void toOGLFmt();
    int getWidth()
    {
        return _BITMAPINFOHEADER.width;
    }
    int getHeight()
    {
        return _BITMAPINFOHEADER.height;
    }
    int getDepth()
    {
        return _BITMAPINFOHEADER.bitspp;
    }

    struct BITMAPINFOHEADER
    {
        u32 header_sz;
        u32 width;
        u32 height;
        u16 nplanes;
        u16 bitspp;
        u32 compress_type;
        u32 bmp_bytesz;
        s32 hres;
        s32 vres;
        u32 ncolors;
        u32 nimpcolors;
    } _BITMAPINFOHEADER;
};

#endif // #ifndef __AMDTIMAGE_H
