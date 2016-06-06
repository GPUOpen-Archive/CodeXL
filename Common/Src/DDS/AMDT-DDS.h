//==============================================================================
// Copyright (c) 2009-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  TODO
//==============================================================================

#pragma once
#ifndef DDS_H
#define DDS_H

namespace DDS
{
typedef enum
{
    Err_Ok = 0,
    Err_Unknown,
} DDS_Error;

typedef enum
{
    UnknownType = 0,
    Texture1D,
    Texture2D,
    Texture3D,
    TextureCubeMap,
} DDS_Type;

typedef enum
{
    UnknownFormat,
    RGBA_32F,
    RGBA_8,
    RG_8,
    RG_S8, // Signed 8 bit per channel.
    R_8,
    D_32F,
    R_32F,
    D24_S8,
    L8_A8
} DDS_Format;

DDS_Error SaveDDSToFile(UINT32 nWidth,
                        UINT32 nHeight,
                        UINT32 nDepth,
                        UINT32 nSubResourceCount,
                        UINT32 nMipMapCount,
                        DDS_Type eType,
                        DDS_Format eFormat,
                        void* pData,
                        size_t nSize,
                        const TCHAR* pszFileName);

DDS_Error SaveDDSToMemory(UINT32 nWidth,
                          UINT32 nHeight,
                          UINT32 nDepth,
                          UINT32 nSubResourceCount,
                          UINT32 nMipMapCount,
                          DDS_Type eType,
                          DDS_Format eFormat,
                          void* pData,
                          size_t nSize,
                          void** ppDDSData,
                          size_t* pnDDSSize);

} // namespace DDS

#endif // DDS_H
