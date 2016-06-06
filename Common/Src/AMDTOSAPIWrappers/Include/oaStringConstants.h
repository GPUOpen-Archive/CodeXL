//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaStringConstants.h
///
//=====================================================================

//------------------------------ oaStringConstants.h ------------------------------

#ifndef __OASTRINGCONSTANTS
#define __OASTRINGCONSTANTS

#define OA_STR_associatedFilePath L"Associated file path: "
#define OA_STR_rawFileVersion L"Raw file version: %d"
#define OA_STR_texelDataFormat L"Texel data format: "
#define OA_STR_texelDataType L"Texel data type: "
#define OA_STR_pageDimensions L"Page width: %d, page height: %d"
#define OA_STR_pagesAmount L"Pages amount: %d"
#define OA_STR_loadingRawDataFile L"Loading raw data file"
#define OA_STR_savingRawDataFile L"Saving raw data file"

// Vendor type:
#define OA_STR_VENDOR_ATI L"ATI"
#define OA_STR_VENDOR_NVIDIA L"NVIDIA"
#define OA_STR_VENDOR_INTEL L"Intel"
#define OA_STR_VENDOR_S3 L"S3"
#define OA_STR_VENDOR_MICROSOFT L"Microsoft"
#define OA_STR_VENDOR_MESA L"Microsoft"

// Driver access ADL entry points:
#define OA_STR_ADL_DRIVER_CREATE_FUNCTION   "ADL_Main_Control_Create"
#define OA_STR_ADL_DRIVER_VERSION_FUNCTION  "ADL_Graphics_Versions_Get"
#define OA_STR_ADL_DRIVER_DESTROY_FUNCTION  "ADL_Main_Control_Destroy"
#define OA_STR_ADL2_DRIVER_CREATE_FUNCTION  "ADL2_Main_Control_Create"
#define OA_STR_ADL2_DRIVER_VERSION_FUNCTION "ADL2_Graphics_VersionsX2_Get"
#define OA_STR_ADL2_DRIVER_VERSION_FUNCTION_LEGACY "ADL2_Graphics_Versions_Get"
#define OA_STR_ADL2_DRIVER_DESTROY_FUNCTION "ADL2_Main_Control_Destroy"

#endif  // __OASTRINGCONSTANTS
