//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file contains DirectCompute Utilities to stringify DC enums
//==============================================================================

#include "d3d11.h"
#include <string>
#include "DCStringifyD3d11Enums.h"

//-----------------------------------------------------------
// Stringify_D3D11_INPUT_CLASSIFICATION
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_INPUT_CLASSIFICATION(D3D11_INPUT_CLASSIFICATION var)
{
    switch (var)
    {
        case D3D11_INPUT_PER_VERTEX_DATA: return "D3D11_INPUT_PER_VERTEX_DATA";

        case D3D11_INPUT_PER_INSTANCE_DATA: return "D3D11_INPUT_PER_INSTANCE_DATA";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_FILL_MODE
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_FILL_MODE(D3D11_FILL_MODE var)
{
    switch (var)
    {
        case D3D11_FILL_WIREFRAME: return "D3D11_FILL_WIREFRAME";

        case D3D11_FILL_SOLID: return "D3D11_FILL_SOLID";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_PRIMITIVE_TOPOLOGY
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_PRIMITIVE_TOPOLOGY(D3D11_PRIMITIVE_TOPOLOGY var)
{
    switch (var)
    {
        case D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED: return "D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED";

        case D3D11_PRIMITIVE_TOPOLOGY_POINTLIST: return "D3D11_PRIMITIVE_TOPOLOGY_POINTLIST";

        case D3D11_PRIMITIVE_TOPOLOGY_LINELIST: return "D3D11_PRIMITIVE_TOPOLOGY_LINELIST";

        case D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP: return "D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP";

        case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST: return "D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST";

        case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP: return "D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP";

        case D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ: return "D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ";

        case D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ: return "D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ";

        case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ: return "D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ";

        case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ: return "D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ";

        case D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST: return "D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST";

        case D3D11_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST: return "D3D11_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST";

        case D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST: return "D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST";

        case D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST: return "D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST";

        case D3D11_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST: return "D3D11_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST";

        case D3D11_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST: return "D3D11_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST";

        case D3D11_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST: return "D3D11_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST";

        case D3D11_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST: return "D3D11_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST";

        case D3D11_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST: return "D3D11_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST";

        case D3D11_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST: return "D3D11_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST";

        case D3D11_PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST: return "D3D11_PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST";

        case D3D11_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST: return "D3D11_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST";

        case D3D11_PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST: return "D3D11_PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST";

        case D3D11_PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST: return "D3D11_PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST";

        case D3D11_PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST: return "D3D11_PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST";

        case D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST: return "D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST";

        case D3D11_PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST: return "D3D11_PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST";

        case D3D11_PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST: return "D3D11_PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST";

        case D3D11_PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST: return "D3D11_PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST";

        case D3D11_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST: return "D3D11_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST";

        case D3D11_PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST: return "D3D11_PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST";

        case D3D11_PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST: return "D3D11_PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST";

        case D3D11_PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST: return "D3D11_PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST";

        case D3D11_PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST: return "D3D11_PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST";

        case D3D11_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST: return "D3D11_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST";

        case D3D11_PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST: return "D3D11_PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST";

        case D3D11_PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST: return "D3D11_PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST";

        case D3D11_PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST: return "D3D11_PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST";

        case D3D11_PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST: return "D3D11_PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST";

        case D3D11_PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST: return "D3D11_PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST";

        case D3D11_PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST: return "D3D11_PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST";

        case D3D11_PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST: return "D3D11_PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_PRIMITIVE
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_PRIMITIVE(D3D11_PRIMITIVE var)
{
    switch (var)
    {
        case D3D11_PRIMITIVE_UNDEFINED: return "D3D11_PRIMITIVE_UNDEFINED";

        case D3D11_PRIMITIVE_POINT: return "D3D11_PRIMITIVE_POINT";

        case D3D11_PRIMITIVE_LINE: return "D3D11_PRIMITIVE_LINE";

        case D3D11_PRIMITIVE_TRIANGLE: return "D3D11_PRIMITIVE_TRIANGLE";

        case D3D11_PRIMITIVE_LINE_ADJ: return "D3D11_PRIMITIVE_LINE_ADJ";

        case D3D11_PRIMITIVE_TRIANGLE_ADJ: return "D3D11_PRIMITIVE_TRIANGLE_ADJ";

        case D3D11_PRIMITIVE_1_CONTROL_POINT_PATCH: return "D3D11_PRIMITIVE_1_CONTROL_POINT_PATCH";

        case D3D11_PRIMITIVE_2_CONTROL_POINT_PATCH: return "D3D11_PRIMITIVE_2_CONTROL_POINT_PATCH";

        case D3D11_PRIMITIVE_3_CONTROL_POINT_PATCH: return "D3D11_PRIMITIVE_3_CONTROL_POINT_PATCH";

        case D3D11_PRIMITIVE_4_CONTROL_POINT_PATCH: return "D3D11_PRIMITIVE_4_CONTROL_POINT_PATCH";

        case D3D11_PRIMITIVE_5_CONTROL_POINT_PATCH: return "D3D11_PRIMITIVE_5_CONTROL_POINT_PATCH";

        case D3D11_PRIMITIVE_6_CONTROL_POINT_PATCH: return "D3D11_PRIMITIVE_6_CONTROL_POINT_PATCH";

        case D3D11_PRIMITIVE_7_CONTROL_POINT_PATCH: return "D3D11_PRIMITIVE_7_CONTROL_POINT_PATCH";

        case D3D11_PRIMITIVE_8_CONTROL_POINT_PATCH: return "D3D11_PRIMITIVE_8_CONTROL_POINT_PATCH";

        case D3D11_PRIMITIVE_9_CONTROL_POINT_PATCH: return "D3D11_PRIMITIVE_9_CONTROL_POINT_PATCH";

        case D3D11_PRIMITIVE_10_CONTROL_POINT_PATCH: return "D3D11_PRIMITIVE_10_CONTROL_POINT_PATCH";

        case D3D11_PRIMITIVE_11_CONTROL_POINT_PATCH: return "D3D11_PRIMITIVE_11_CONTROL_POINT_PATCH";

        case D3D11_PRIMITIVE_12_CONTROL_POINT_PATCH: return "D3D11_PRIMITIVE_12_CONTROL_POINT_PATCH";

        case D3D11_PRIMITIVE_13_CONTROL_POINT_PATCH: return "D3D11_PRIMITIVE_13_CONTROL_POINT_PATCH";

        case D3D11_PRIMITIVE_14_CONTROL_POINT_PATCH: return "D3D11_PRIMITIVE_14_CONTROL_POINT_PATCH";

        case D3D11_PRIMITIVE_15_CONTROL_POINT_PATCH: return "D3D11_PRIMITIVE_15_CONTROL_POINT_PATCH";

        case D3D11_PRIMITIVE_16_CONTROL_POINT_PATCH: return "D3D11_PRIMITIVE_16_CONTROL_POINT_PATCH";

        case D3D11_PRIMITIVE_17_CONTROL_POINT_PATCH: return "D3D11_PRIMITIVE_17_CONTROL_POINT_PATCH";

        case D3D11_PRIMITIVE_18_CONTROL_POINT_PATCH: return "D3D11_PRIMITIVE_18_CONTROL_POINT_PATCH";

        case D3D11_PRIMITIVE_19_CONTROL_POINT_PATCH: return "D3D11_PRIMITIVE_19_CONTROL_POINT_PATCH";

        case D3D11_PRIMITIVE_20_CONTROL_POINT_PATCH: return "D3D11_PRIMITIVE_20_CONTROL_POINT_PATCH";

        case D3D11_PRIMITIVE_21_CONTROL_POINT_PATCH: return "D3D11_PRIMITIVE_21_CONTROL_POINT_PATCH";

        case D3D11_PRIMITIVE_22_CONTROL_POINT_PATCH: return "D3D11_PRIMITIVE_22_CONTROL_POINT_PATCH";

        case D3D11_PRIMITIVE_23_CONTROL_POINT_PATCH: return "D3D11_PRIMITIVE_23_CONTROL_POINT_PATCH";

        case D3D11_PRIMITIVE_24_CONTROL_POINT_PATCH: return "D3D11_PRIMITIVE_24_CONTROL_POINT_PATCH";

        case D3D11_PRIMITIVE_25_CONTROL_POINT_PATCH: return "D3D11_PRIMITIVE_25_CONTROL_POINT_PATCH";

        case D3D11_PRIMITIVE_26_CONTROL_POINT_PATCH: return "D3D11_PRIMITIVE_26_CONTROL_POINT_PATCH";

        case D3D11_PRIMITIVE_27_CONTROL_POINT_PATCH: return "D3D11_PRIMITIVE_27_CONTROL_POINT_PATCH";

        case D3D11_PRIMITIVE_28_CONTROL_POINT_PATCH: return "D3D11_PRIMITIVE_28_CONTROL_POINT_PATCH";

        case D3D11_PRIMITIVE_29_CONTROL_POINT_PATCH: return "D3D11_PRIMITIVE_29_CONTROL_POINT_PATCH";

        case D3D11_PRIMITIVE_30_CONTROL_POINT_PATCH: return "D3D11_PRIMITIVE_30_CONTROL_POINT_PATCH";

        case D3D11_PRIMITIVE_31_CONTROL_POINT_PATCH: return "D3D11_PRIMITIVE_31_CONTROL_POINT_PATCH";

        case D3D11_PRIMITIVE_32_CONTROL_POINT_PATCH: return "D3D11_PRIMITIVE_32_CONTROL_POINT_PATCH";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_CULL_MODE
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_CULL_MODE(D3D11_CULL_MODE var)
{
    switch (var)
    {
        case D3D11_CULL_NONE: return "D3D11_CULL_NONE";

        case D3D11_CULL_FRONT: return "D3D11_CULL_FRONT";

        case D3D11_CULL_BACK: return "D3D11_CULL_BACK";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_RESOURCE_DIMENSION
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_RESOURCE_DIMENSION(D3D11_RESOURCE_DIMENSION var)
{
    switch (var)
    {
        case D3D11_RESOURCE_DIMENSION_UNKNOWN: return "D3D11_RESOURCE_DIMENSION_UNKNOWN";

        case D3D11_RESOURCE_DIMENSION_BUFFER: return "D3D11_RESOURCE_DIMENSION_BUFFER";

        case D3D11_RESOURCE_DIMENSION_TEXTURE1D: return "D3D11_RESOURCE_DIMENSION_TEXTURE1D";

        case D3D11_RESOURCE_DIMENSION_TEXTURE2D: return "D3D11_RESOURCE_DIMENSION_TEXTURE2D";

        case D3D11_RESOURCE_DIMENSION_TEXTURE3D: return "D3D11_RESOURCE_DIMENSION_TEXTURE3D";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_SRV_DIMENSION
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_SRV_DIMENSION(D3D11_SRV_DIMENSION var)
{
    switch (var)
    {
        case D3D11_SRV_DIMENSION_UNKNOWN: return "D3D11_SRV_DIMENSION_UNKNOWN";

        case D3D11_SRV_DIMENSION_BUFFER: return "D3D11_SRV_DIMENSION_BUFFER";

        case D3D11_SRV_DIMENSION_TEXTURE1D: return "D3D11_SRV_DIMENSION_TEXTURE1D";

        case D3D11_SRV_DIMENSION_TEXTURE1DARRAY: return "D3D11_SRV_DIMENSION_TEXTURE1DARRAY";

        case D3D11_SRV_DIMENSION_TEXTURE2D: return "D3D11_SRV_DIMENSION_TEXTURE2D";

        case D3D11_SRV_DIMENSION_TEXTURE2DARRAY: return "D3D11_SRV_DIMENSION_TEXTURE2DARRAY";

        case D3D11_SRV_DIMENSION_TEXTURE2DMS: return "D3D11_SRV_DIMENSION_TEXTURE2DMS";

        case D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY: return "D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY";

        case D3D11_SRV_DIMENSION_TEXTURE3D: return "D3D11_SRV_DIMENSION_TEXTURE3D";

        case D3D11_SRV_DIMENSION_TEXTURECUBE: return "D3D11_SRV_DIMENSION_TEXTURECUBE";

        case D3D11_SRV_DIMENSION_TEXTURECUBEARRAY: return "D3D11_SRV_DIMENSION_TEXTURECUBEARRAY";

        case D3D11_SRV_DIMENSION_BUFFEREX: return "D3D11_SRV_DIMENSION_BUFFEREX";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_DSV_DIMENSION
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_DSV_DIMENSION(D3D11_DSV_DIMENSION var)
{
    switch (var)
    {
        case D3D11_DSV_DIMENSION_UNKNOWN: return "D3D11_DSV_DIMENSION_UNKNOWN";

        case D3D11_DSV_DIMENSION_TEXTURE1D: return "D3D11_DSV_DIMENSION_TEXTURE1D";

        case D3D11_DSV_DIMENSION_TEXTURE1DARRAY: return "D3D11_DSV_DIMENSION_TEXTURE1DARRAY";

        case D3D11_DSV_DIMENSION_TEXTURE2D: return "D3D11_DSV_DIMENSION_TEXTURE2D";

        case D3D11_DSV_DIMENSION_TEXTURE2DARRAY: return "D3D11_DSV_DIMENSION_TEXTURE2DARRAY";

        case D3D11_DSV_DIMENSION_TEXTURE2DMS: return "D3D11_DSV_DIMENSION_TEXTURE2DMS";

        case D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY: return "D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_RTV_DIMENSION
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_RTV_DIMENSION(D3D11_RTV_DIMENSION var)
{
    switch (var)
    {
        case D3D11_RTV_DIMENSION_UNKNOWN: return "D3D11_RTV_DIMENSION_UNKNOWN";

        case D3D11_RTV_DIMENSION_BUFFER: return "D3D11_RTV_DIMENSION_BUFFER";

        case D3D11_RTV_DIMENSION_TEXTURE1D: return "D3D11_RTV_DIMENSION_TEXTURE1D";

        case D3D11_RTV_DIMENSION_TEXTURE1DARRAY: return "D3D11_RTV_DIMENSION_TEXTURE1DARRAY";

        case D3D11_RTV_DIMENSION_TEXTURE2D: return "D3D11_RTV_DIMENSION_TEXTURE2D";

        case D3D11_RTV_DIMENSION_TEXTURE2DARRAY: return "D3D11_RTV_DIMENSION_TEXTURE2DARRAY";

        case D3D11_RTV_DIMENSION_TEXTURE2DMS: return "D3D11_RTV_DIMENSION_TEXTURE2DMS";

        case D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY: return "D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY";

        case D3D11_RTV_DIMENSION_TEXTURE3D: return "D3D11_RTV_DIMENSION_TEXTURE3D";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_UAV_DIMENSION
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_UAV_DIMENSION(D3D11_UAV_DIMENSION var)
{
    switch (var)
    {
        case D3D11_UAV_DIMENSION_UNKNOWN: return "D3D11_UAV_DIMENSION_UNKNOWN";

        case D3D11_UAV_DIMENSION_BUFFER: return "D3D11_UAV_DIMENSION_BUFFER";

        case D3D11_UAV_DIMENSION_TEXTURE1D: return "D3D11_UAV_DIMENSION_TEXTURE1D";

        case D3D11_UAV_DIMENSION_TEXTURE1DARRAY: return "D3D11_UAV_DIMENSION_TEXTURE1DARRAY";

        case D3D11_UAV_DIMENSION_TEXTURE2D: return "D3D11_UAV_DIMENSION_TEXTURE2D";

        case D3D11_UAV_DIMENSION_TEXTURE2DARRAY: return "D3D11_UAV_DIMENSION_TEXTURE2DARRAY";

        case D3D11_UAV_DIMENSION_TEXTURE3D: return "D3D11_UAV_DIMENSION_TEXTURE3D";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_USAGE
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_USAGE(D3D11_USAGE var)
{
    switch (var)
    {
        case D3D11_USAGE_DEFAULT: return "D3D11_USAGE_DEFAULT";

        case D3D11_USAGE_IMMUTABLE: return "D3D11_USAGE_IMMUTABLE";

        case D3D11_USAGE_DYNAMIC: return "D3D11_USAGE_DYNAMIC";

        case D3D11_USAGE_STAGING: return "D3D11_USAGE_STAGING";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_BIND_FLAG
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_BIND_FLAG(D3D11_BIND_FLAG var)
{
    switch (var)
    {
        case D3D11_BIND_VERTEX_BUFFER: return "D3D11_BIND_VERTEX_BUFFER";

        case D3D11_BIND_INDEX_BUFFER: return "D3D11_BIND_INDEX_BUFFER";

        case D3D11_BIND_CONSTANT_BUFFER: return "D3D11_BIND_CONSTANT_BUFFER";

        case D3D11_BIND_SHADER_RESOURCE: return "D3D11_BIND_SHADER_RESOURCE";

        case D3D11_BIND_STREAM_OUTPUT: return "D3D11_BIND_STREAM_OUTPUT";

        case D3D11_BIND_RENDER_TARGET: return "D3D11_BIND_RENDER_TARGET";

        case D3D11_BIND_DEPTH_STENCIL: return "D3D11_BIND_DEPTH_STENCIL";

        case D3D11_BIND_UNORDERED_ACCESS: return "D3D11_BIND_UNORDERED_ACCESS";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_CPU_ACCESS_FLAG
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_CPU_ACCESS_FLAG(D3D11_CPU_ACCESS_FLAG var)
{
    switch (var)
    {
        case D3D11_CPU_ACCESS_WRITE: return "D3D11_CPU_ACCESS_WRITE";

        case D3D11_CPU_ACCESS_READ: return "D3D11_CPU_ACCESS_READ";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_RESOURCE_MISC_FLAG
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_RESOURCE_MISC_FLAG(D3D11_RESOURCE_MISC_FLAG var)
{
    switch (var)
    {
        case D3D11_RESOURCE_MISC_GENERATE_MIPS: return "D3D11_RESOURCE_MISC_GENERATE_MIPS";

        case D3D11_RESOURCE_MISC_SHARED: return "D3D11_RESOURCE_MISC_SHARED";

        case D3D11_RESOURCE_MISC_TEXTURECUBE: return "D3D11_RESOURCE_MISC_TEXTURECUBE";

        case D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS: return "D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS";

        case D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS: return "D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS";

        case D3D11_RESOURCE_MISC_BUFFER_STRUCTURED: return "D3D11_RESOURCE_MISC_BUFFER_STRUCTURED";

        case D3D11_RESOURCE_MISC_RESOURCE_CLAMP: return "D3D11_RESOURCE_MISC_RESOURCE_CLAMP";

        case D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX: return "D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX";

        case D3D11_RESOURCE_MISC_GDI_COMPATIBLE: return "D3D11_RESOURCE_MISC_GDI_COMPATIBLE";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_MAP
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_MAP(D3D11_MAP var)
{
    switch (var)
    {
        case D3D11_MAP_READ: return "D3D11_MAP_READ";

        case D3D11_MAP_WRITE: return "D3D11_MAP_WRITE";

        case D3D11_MAP_READ_WRITE: return "D3D11_MAP_READ_WRITE";

        case D3D11_MAP_WRITE_DISCARD: return "D3D11_MAP_WRITE_DISCARD";

        case D3D11_MAP_WRITE_NO_OVERWRITE: return "D3D11_MAP_WRITE_NO_OVERWRITE";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_MAP_FLAG
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_MAP_FLAG(D3D11_MAP_FLAG var)
{
    switch (var)
    {
        case D3D11_MAP_FLAG_DO_NOT_WAIT: return "D3D11_MAP_FLAG_DO_NOT_WAIT";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_RAISE_FLAG
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_RAISE_FLAG(D3D11_RAISE_FLAG var)
{
    switch (var)
    {
        case D3D11_RAISE_FLAG_DRIVER_INTERNAL_ERROR: return "D3D11_RAISE_FLAG_DRIVER_INTERNAL_ERROR";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_CLEAR_FLAG
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_CLEAR_FLAG(D3D11_CLEAR_FLAG var)
{
    switch (var)
    {
        case D3D11_CLEAR_DEPTH: return "D3D11_CLEAR_DEPTH";

        case D3D11_CLEAR_STENCIL: return "D3D11_CLEAR_STENCIL";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_COMPARISON_FUNC
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_COMPARISON_FUNC(D3D11_COMPARISON_FUNC var)
{
    switch (var)
    {
        case D3D11_COMPARISON_NEVER: return "D3D11_COMPARISON_NEVER";

        case D3D11_COMPARISON_LESS: return "D3D11_COMPARISON_LESS";

        case D3D11_COMPARISON_EQUAL: return "D3D11_COMPARISON_EQUAL";

        case D3D11_COMPARISON_LESS_EQUAL: return "D3D11_COMPARISON_LESS_EQUAL";

        case D3D11_COMPARISON_GREATER: return "D3D11_COMPARISON_GREATER";

        case D3D11_COMPARISON_NOT_EQUAL: return "D3D11_COMPARISON_NOT_EQUAL";

        case D3D11_COMPARISON_GREATER_EQUAL: return "D3D11_COMPARISON_GREATER_EQUAL";

        case D3D11_COMPARISON_ALWAYS: return "D3D11_COMPARISON_ALWAYS";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_DEPTH_WRITE_MASK
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_DEPTH_WRITE_MASK(D3D11_DEPTH_WRITE_MASK var)
{
    switch (var)
    {
        case D3D11_DEPTH_WRITE_MASK_ZERO: return "D3D11_DEPTH_WRITE_MASK_ZERO";

        case D3D11_DEPTH_WRITE_MASK_ALL: return "D3D11_DEPTH_WRITE_MASK_ALL";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_STENCIL_OP
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_STENCIL_OP(D3D11_STENCIL_OP var)
{
    switch (var)
    {
        case D3D11_STENCIL_OP_KEEP: return "D3D11_STENCIL_OP_KEEP";

        case D3D11_STENCIL_OP_ZERO: return "D3D11_STENCIL_OP_ZERO";

        case D3D11_STENCIL_OP_REPLACE: return "D3D11_STENCIL_OP_REPLACE";

        case D3D11_STENCIL_OP_INCR_SAT: return "D3D11_STENCIL_OP_INCR_SAT";

        case D3D11_STENCIL_OP_DECR_SAT: return "D3D11_STENCIL_OP_DECR_SAT";

        case D3D11_STENCIL_OP_INVERT: return "D3D11_STENCIL_OP_INVERT";

        case D3D11_STENCIL_OP_INCR: return "D3D11_STENCIL_OP_INCR";

        case D3D11_STENCIL_OP_DECR: return "D3D11_STENCIL_OP_DECR";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_BLEND
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_BLEND(D3D11_BLEND var)
{
    switch (var)
    {
        case D3D11_BLEND_ZERO: return "D3D11_BLEND_ZERO";

        case D3D11_BLEND_ONE: return "D3D11_BLEND_ONE";

        case D3D11_BLEND_SRC_COLOR: return "D3D11_BLEND_SRC_COLOR";

        case D3D11_BLEND_INV_SRC_COLOR: return "D3D11_BLEND_INV_SRC_COLOR";

        case D3D11_BLEND_SRC_ALPHA: return "D3D11_BLEND_SRC_ALPHA";

        case D3D11_BLEND_INV_SRC_ALPHA: return "D3D11_BLEND_INV_SRC_ALPHA";

        case D3D11_BLEND_DEST_ALPHA: return "D3D11_BLEND_DEST_ALPHA";

        case D3D11_BLEND_INV_DEST_ALPHA: return "D3D11_BLEND_INV_DEST_ALPHA";

        case D3D11_BLEND_DEST_COLOR: return "D3D11_BLEND_DEST_COLOR";

        case D3D11_BLEND_INV_DEST_COLOR: return "D3D11_BLEND_INV_DEST_COLOR";

        case D3D11_BLEND_SRC_ALPHA_SAT: return "D3D11_BLEND_SRC_ALPHA_SAT";

        case D3D11_BLEND_BLEND_FACTOR: return "D3D11_BLEND_BLEND_FACTOR";

        case D3D11_BLEND_INV_BLEND_FACTOR: return "D3D11_BLEND_INV_BLEND_FACTOR";

        case D3D11_BLEND_SRC1_COLOR: return "D3D11_BLEND_SRC1_COLOR";

        case D3D11_BLEND_INV_SRC1_COLOR: return "D3D11_BLEND_INV_SRC1_COLOR";

        case D3D11_BLEND_SRC1_ALPHA: return "D3D11_BLEND_SRC1_ALPHA";

        case D3D11_BLEND_INV_SRC1_ALPHA: return "D3D11_BLEND_INV_SRC1_ALPHA";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_BLEND_OP
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_BLEND_OP(D3D11_BLEND_OP var)
{
    switch (var)
    {
        case D3D11_BLEND_OP_ADD: return "D3D11_BLEND_OP_ADD";

        case D3D11_BLEND_OP_SUBTRACT: return "D3D11_BLEND_OP_SUBTRACT";

        case D3D11_BLEND_OP_REV_SUBTRACT: return "D3D11_BLEND_OP_REV_SUBTRACT";

        case D3D11_BLEND_OP_MIN: return "D3D11_BLEND_OP_MIN";

        case D3D11_BLEND_OP_MAX: return "D3D11_BLEND_OP_MAX";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_COLOR_WRITE_ENABLE
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_COLOR_WRITE_ENABLE(D3D11_COLOR_WRITE_ENABLE var)
{
    switch (var)
    {
        case D3D11_COLOR_WRITE_ENABLE_RED: return "D3D11_COLOR_WRITE_ENABLE_RED";

        case D3D11_COLOR_WRITE_ENABLE_GREEN: return "D3D11_COLOR_WRITE_ENABLE_GREEN";

        case D3D11_COLOR_WRITE_ENABLE_BLUE: return "D3D11_COLOR_WRITE_ENABLE_BLUE";

        case D3D11_COLOR_WRITE_ENABLE_ALPHA: return "D3D11_COLOR_WRITE_ENABLE_ALPHA";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_TEXTURECUBE_FACE
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_TEXTURECUBE_FACE(D3D11_TEXTURECUBE_FACE var)
{
    switch (var)
    {
        case D3D11_TEXTURECUBE_FACE_POSITIVE_X: return "D3D11_TEXTURECUBE_FACE_POSITIVE_X";

        case D3D11_TEXTURECUBE_FACE_NEGATIVE_X: return "D3D11_TEXTURECUBE_FACE_NEGATIVE_X";

        case D3D11_TEXTURECUBE_FACE_POSITIVE_Y: return "D3D11_TEXTURECUBE_FACE_POSITIVE_Y";

        case D3D11_TEXTURECUBE_FACE_NEGATIVE_Y: return "D3D11_TEXTURECUBE_FACE_NEGATIVE_Y";

        case D3D11_TEXTURECUBE_FACE_POSITIVE_Z: return "D3D11_TEXTURECUBE_FACE_POSITIVE_Z";

        case D3D11_TEXTURECUBE_FACE_NEGATIVE_Z: return "D3D11_TEXTURECUBE_FACE_NEGATIVE_Z";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_BUFFEREX_SRV_FLAG
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_BUFFEREX_SRV_FLAG(D3D11_BUFFEREX_SRV_FLAG var)
{
    switch (var)
    {
        case D3D11_BUFFEREX_SRV_FLAG_RAW: return "D3D11_BUFFEREX_SRV_FLAG_RAW";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_DSV_FLAG
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_DSV_FLAG(D3D11_DSV_FLAG var)
{
    switch (var)
    {
        case D3D11_DSV_READ_ONLY_DEPTH: return "D3D11_DSV_READ_ONLY_DEPTH";

        case D3D11_DSV_READ_ONLY_STENCIL: return "D3D11_DSV_READ_ONLY_STENCIL";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_BUFFER_UAV_FLAG
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_BUFFER_UAV_FLAG(D3D11_BUFFER_UAV_FLAG var)
{
    switch (var)
    {
        case D3D11_BUFFER_UAV_FLAG_RAW: return "D3D11_BUFFER_UAV_FLAG_RAW";

        case D3D11_BUFFER_UAV_FLAG_APPEND: return "D3D11_BUFFER_UAV_FLAG_APPEND";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_FILTER
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_FILTER(D3D11_FILTER var)
{
    switch (var)
    {
        case D3D11_FILTER_MIN_MAG_MIP_POINT: return "D3D11_FILTER_MIN_MAG_MIP_POINT";

        case D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR: return "D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR";

        case D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT: return "D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT";

        case D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR: return "D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR";

        case D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT: return "D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT";

        case D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR: return "D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR";

        case D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT: return "D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT";

        case D3D11_FILTER_MIN_MAG_MIP_LINEAR: return "D3D11_FILTER_MIN_MAG_MIP_LINEAR";

        case D3D11_FILTER_ANISOTROPIC: return "D3D11_FILTER_ANISOTROPIC";

        case D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT: return "D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT";

        case D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR: return "D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR";

        case D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT: return "D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT";

        case D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR: return "D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR";

        case D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT: return "D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT";

        case D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR: return "D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR";

        case D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT: return "D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT";

        case D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR: return "D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR";

        case D3D11_FILTER_COMPARISON_ANISOTROPIC: return "D3D11_FILTER_COMPARISON_ANISOTROPIC";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_FILTER_TYPE
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_FILTER_TYPE(D3D11_FILTER_TYPE var)
{
    switch (var)
    {
        case D3D11_FILTER_TYPE_POINT: return "D3D11_FILTER_TYPE_POINT";

        case D3D11_FILTER_TYPE_LINEAR: return "D3D11_FILTER_TYPE_LINEAR";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_TEXTURE_ADDRESS_MODE
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_TEXTURE_ADDRESS_MODE(D3D11_TEXTURE_ADDRESS_MODE var)
{
    switch (var)
    {
        case D3D11_TEXTURE_ADDRESS_WRAP: return "D3D11_TEXTURE_ADDRESS_WRAP";

        case D3D11_TEXTURE_ADDRESS_MIRROR: return "D3D11_TEXTURE_ADDRESS_MIRROR";

        case D3D11_TEXTURE_ADDRESS_CLAMP: return "D3D11_TEXTURE_ADDRESS_CLAMP";

        case D3D11_TEXTURE_ADDRESS_BORDER: return "D3D11_TEXTURE_ADDRESS_BORDER";

        case D3D11_TEXTURE_ADDRESS_MIRROR_ONCE: return "D3D11_TEXTURE_ADDRESS_MIRROR_ONCE";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_FORMAT_SUPPORT
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_FORMAT_SUPPORT(D3D11_FORMAT_SUPPORT var)
{
    switch (var)
    {
        case D3D11_FORMAT_SUPPORT_BUFFER: return "D3D11_FORMAT_SUPPORT_BUFFER";

        case D3D11_FORMAT_SUPPORT_IA_VERTEX_BUFFER: return "D3D11_FORMAT_SUPPORT_IA_VERTEX_BUFFER";

        case D3D11_FORMAT_SUPPORT_IA_INDEX_BUFFER: return "D3D11_FORMAT_SUPPORT_IA_INDEX_BUFFER";

        case D3D11_FORMAT_SUPPORT_SO_BUFFER: return "D3D11_FORMAT_SUPPORT_SO_BUFFER";

        case D3D11_FORMAT_SUPPORT_TEXTURE1D: return "D3D11_FORMAT_SUPPORT_TEXTURE1D";

        case D3D11_FORMAT_SUPPORT_TEXTURE2D: return "D3D11_FORMAT_SUPPORT_TEXTURE2D";

        case D3D11_FORMAT_SUPPORT_TEXTURE3D: return "D3D11_FORMAT_SUPPORT_TEXTURE3D";

        case D3D11_FORMAT_SUPPORT_TEXTURECUBE: return "D3D11_FORMAT_SUPPORT_TEXTURECUBE";

        case D3D11_FORMAT_SUPPORT_SHADER_LOAD: return "D3D11_FORMAT_SUPPORT_SHADER_LOAD";

        case D3D11_FORMAT_SUPPORT_SHADER_SAMPLE: return "D3D11_FORMAT_SUPPORT_SHADER_SAMPLE";

        case D3D11_FORMAT_SUPPORT_SHADER_SAMPLE_COMPARISON: return "D3D11_FORMAT_SUPPORT_SHADER_SAMPLE_COMPARISON";

        case D3D11_FORMAT_SUPPORT_SHADER_SAMPLE_MONO_TEXT: return "D3D11_FORMAT_SUPPORT_SHADER_SAMPLE_MONO_TEXT";

        case D3D11_FORMAT_SUPPORT_MIP: return "D3D11_FORMAT_SUPPORT_MIP";

        case D3D11_FORMAT_SUPPORT_MIP_AUTOGEN: return "D3D11_FORMAT_SUPPORT_MIP_AUTOGEN";

        case D3D11_FORMAT_SUPPORT_RENDER_TARGET: return "D3D11_FORMAT_SUPPORT_RENDER_TARGET";

        case D3D11_FORMAT_SUPPORT_BLENDABLE: return "D3D11_FORMAT_SUPPORT_BLENDABLE";

        case D3D11_FORMAT_SUPPORT_DEPTH_STENCIL: return "D3D11_FORMAT_SUPPORT_DEPTH_STENCIL";

        case D3D11_FORMAT_SUPPORT_CPU_LOCKABLE: return "D3D11_FORMAT_SUPPORT_CPU_LOCKABLE";

        case D3D11_FORMAT_SUPPORT_MULTISAMPLE_RESOLVE: return "D3D11_FORMAT_SUPPORT_MULTISAMPLE_RESOLVE";

        case D3D11_FORMAT_SUPPORT_DISPLAY: return "D3D11_FORMAT_SUPPORT_DISPLAY";

        case D3D11_FORMAT_SUPPORT_CAST_WITHIN_BIT_LAYOUT: return "D3D11_FORMAT_SUPPORT_CAST_WITHIN_BIT_LAYOUT";

        case D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET: return "D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET";

        case D3D11_FORMAT_SUPPORT_MULTISAMPLE_LOAD: return "D3D11_FORMAT_SUPPORT_MULTISAMPLE_LOAD";

        case D3D11_FORMAT_SUPPORT_SHADER_GATHER: return "D3D11_FORMAT_SUPPORT_SHADER_GATHER";

        case D3D11_FORMAT_SUPPORT_BACK_BUFFER_CAST: return "D3D11_FORMAT_SUPPORT_BACK_BUFFER_CAST";

        case D3D11_FORMAT_SUPPORT_TYPED_UNORDERED_ACCESS_VIEW: return "D3D11_FORMAT_SUPPORT_TYPED_UNORDERED_ACCESS_VIEW";

        case D3D11_FORMAT_SUPPORT_SHADER_GATHER_COMPARISON: return "D3D11_FORMAT_SUPPORT_SHADER_GATHER_COMPARISON";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_FORMAT_SUPPORT2
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_FORMAT_SUPPORT2(D3D11_FORMAT_SUPPORT2 var)
{
    switch (var)
    {
        case D3D11_FORMAT_SUPPORT2_UAV_ATOMIC_ADD: return "D3D11_FORMAT_SUPPORT2_UAV_ATOMIC_ADD";

        case D3D11_FORMAT_SUPPORT2_UAV_ATOMIC_BITWISE_OPS: return "D3D11_FORMAT_SUPPORT2_UAV_ATOMIC_BITWISE_OPS";

        case D3D11_FORMAT_SUPPORT2_UAV_ATOMIC_COMPARE_STORE_OR_COMPARE_EXCHANGE: return "D3D11_FORMAT_SUPPORT2_UAV_ATOMIC_COMPARE_STORE_OR_COMPARE_EXCHANGE";

        case D3D11_FORMAT_SUPPORT2_UAV_ATOMIC_EXCHANGE: return "D3D11_FORMAT_SUPPORT2_UAV_ATOMIC_EXCHANGE";

        case D3D11_FORMAT_SUPPORT2_UAV_ATOMIC_SIGNED_MIN_OR_MAX: return "D3D11_FORMAT_SUPPORT2_UAV_ATOMIC_SIGNED_MIN_OR_MAX";

        case D3D11_FORMAT_SUPPORT2_UAV_ATOMIC_UNSIGNED_MIN_OR_MAX: return "D3D11_FORMAT_SUPPORT2_UAV_ATOMIC_UNSIGNED_MIN_OR_MAX";

        case D3D11_FORMAT_SUPPORT2_UAV_TYPED_LOAD: return "D3D11_FORMAT_SUPPORT2_UAV_TYPED_LOAD";

        case D3D11_FORMAT_SUPPORT2_UAV_TYPED_STORE: return "D3D11_FORMAT_SUPPORT2_UAV_TYPED_STORE";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_ASYNC_GETDATA_FLAG
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_ASYNC_GETDATA_FLAG(D3D11_ASYNC_GETDATA_FLAG var)
{
    switch (var)
    {
        case D3D11_ASYNC_GETDATA_DONOTFLUSH: return "D3D11_ASYNC_GETDATA_DONOTFLUSH";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_QUERY
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_QUERY(D3D11_QUERY var)
{
    switch (var)
    {
        case D3D11_QUERY_EVENT: return "D3D11_QUERY_EVENT";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_QUERY_MISC_FLAG
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_QUERY_MISC_FLAG(D3D11_QUERY_MISC_FLAG var)
{
    switch (var)
    {
        case D3D11_QUERY_MISC_PREDICATEHINT: return "D3D11_QUERY_MISC_PREDICATEHINT";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_COUNTER
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_COUNTER(D3D11_COUNTER var)
{
    switch (var)
    {
        case D3D11_COUNTER_DEVICE_DEPENDENT_0: return "D3D11_COUNTER_DEVICE_DEPENDENT_0";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_COUNTER_TYPE
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_COUNTER_TYPE(D3D11_COUNTER_TYPE var)
{
    switch (var)
    {
        case D3D11_COUNTER_TYPE_FLOAT32: return "D3D11_COUNTER_TYPE_FLOAT32";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_STANDARD_MULTISAMPLE_QUALITY_LEVELS
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_STANDARD_MULTISAMPLE_QUALITY_LEVELS(D3D11_STANDARD_MULTISAMPLE_QUALITY_LEVELS var)
{
    switch (var)
    {
        case D3D11_STANDARD_MULTISAMPLE_PATTERN: return "D3D11_STANDARD_MULTISAMPLE_PATTERN";

        case D3D11_CENTER_MULTISAMPLE_PATTERN: return "D3D11_CENTER_MULTISAMPLE_PATTERN";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_DEVICE_CONTEXT_TYPE
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_DEVICE_CONTEXT_TYPE(D3D11_DEVICE_CONTEXT_TYPE var)
{
    switch (var)
    {
        case D3D11_DEVICE_CONTEXT_IMMEDIATE: return "D3D11_DEVICE_CONTEXT_IMMEDIATE";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_FEATURE
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_FEATURE(D3D11_FEATURE var)
{
    switch (var)
    {
        case D3D11_FEATURE_THREADING: return "D3D11_FEATURE_THREADING";

        default: return "not found";
    }
}

//-----------------------------------------------------------
// Stringify_D3D11_CREATE_DEVICE_FLAG
//-----------------------------------------------------------
std::string DCUtils::Stringify_D3D11_CREATE_DEVICE_FLAG(D3D11_CREATE_DEVICE_FLAG var)
{
    switch (var)
    {
        case D3D11_CREATE_DEVICE_SINGLETHREADED: return "D3D11_CREATE_DEVICE_SINGLETHREADED";

        case D3D11_CREATE_DEVICE_DEBUG: return "D3D11_CREATE_DEVICE_DEBUG";

        case D3D11_CREATE_DEVICE_SWITCH_TO_REF: return "D3D11_CREATE_DEVICE_SWITCH_TO_REF";

        case D3D11_CREATE_DEVICE_PREVENT_INTERNAL_THREADING_OPTIMIZATIONS: return "D3D11_CREATE_DEVICE_PREVENT_INTERNAL_THREADING_OPTIMIZATIONS";

        case D3D11_CREATE_DEVICE_BGRA_SUPPORT: return "D3D11_CREATE_DEVICE_BGRA_SUPPORT";

        default: return "not found";
    }
}

