//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file contains DirectCompute Utilities to stringify DC enums
//==============================================================================

#ifndef _DC_STRINGIFY_D3D11_ENUMS_H_
#define _DC_STRINGIFY_D3D11_ENUMS_H_

#include <string>
#include "d3d11.h"

/// \addtogroup DCServer
// @{
namespace DCUtils
{
std::string Stringify_D3D11_INPUT_CLASSIFICATION(D3D11_INPUT_CLASSIFICATION var);
std::string Stringify_D3D11_FILL_MODE(D3D11_FILL_MODE var);
std::string Stringify_D3D11_PRIMITIVE_TOPOLOGY(D3D11_PRIMITIVE_TOPOLOGY var);
std::string Stringify_D3D11_PRIMITIVE(D3D11_PRIMITIVE var);
std::string Stringify_D3D11_CULL_MODE(D3D11_CULL_MODE var);
std::string Stringify_D3D11_RESOURCE_DIMENSION(D3D11_RESOURCE_DIMENSION var);
std::string Stringify_D3D11_SRV_DIMENSION(D3D11_SRV_DIMENSION var);
std::string Stringify_D3D11_DSV_DIMENSION(D3D11_DSV_DIMENSION var);
std::string Stringify_D3D11_RTV_DIMENSION(D3D11_RTV_DIMENSION var);
std::string Stringify_D3D11_UAV_DIMENSION(D3D11_UAV_DIMENSION var);
std::string Stringify_D3D11_USAGE(D3D11_USAGE var);
std::string Stringify_D3D11_BIND_FLAG(D3D11_BIND_FLAG var);
std::string Stringify_D3D11_CPU_ACCESS_FLAG(D3D11_CPU_ACCESS_FLAG var);
std::string Stringify_D3D11_RESOURCE_MISC_FLAG(D3D11_RESOURCE_MISC_FLAG var);
std::string Stringify_D3D11_MAP(D3D11_MAP var);
std::string Stringify_D3D11_MAP_FLAG(D3D11_MAP_FLAG var);
std::string Stringify_D3D11_RAISE_FLAG(D3D11_RAISE_FLAG var);
std::string Stringify_D3D11_CLEAR_FLAG(D3D11_CLEAR_FLAG var);
std::string Stringify_D3D11_COMPARISON_FUNC(D3D11_COMPARISON_FUNC var);
std::string Stringify_D3D11_DEPTH_WRITE_MASK(D3D11_DEPTH_WRITE_MASK var);
std::string Stringify_D3D11_STENCIL_OP(D3D11_STENCIL_OP var);
std::string Stringify_D3D11_BLEND(D3D11_BLEND var);
std::string Stringify_D3D11_BLEND_OP(D3D11_BLEND_OP var);
std::string Stringify_D3D11_COLOR_WRITE_ENABLE(D3D11_COLOR_WRITE_ENABLE var);
std::string Stringify_D3D11_TEXTURECUBE_FACE(D3D11_TEXTURECUBE_FACE var);
std::string Stringify_D3D11_BUFFEREX_SRV_FLAG(D3D11_BUFFEREX_SRV_FLAG var);
std::string Stringify_D3D11_DSV_FLAG(D3D11_DSV_FLAG var);
std::string Stringify_D3D11_BUFFER_UAV_FLAG(D3D11_BUFFER_UAV_FLAG var);
std::string Stringify_D3D11_FILTER(D3D11_FILTER var);
std::string Stringify_D3D11_FILTER_TYPE(D3D11_FILTER_TYPE var);
std::string Stringify_D3D11_TEXTURE_ADDRESS_MODE(D3D11_TEXTURE_ADDRESS_MODE var);
std::string Stringify_D3D11_FORMAT_SUPPORT(D3D11_FORMAT_SUPPORT var);
std::string Stringify_D3D11_FORMAT_SUPPORT2(D3D11_FORMAT_SUPPORT2 var);
std::string Stringify_D3D11_ASYNC_GETDATA_FLAG(D3D11_ASYNC_GETDATA_FLAG var);
std::string Stringify_D3D11_QUERY(D3D11_QUERY var);
std::string Stringify_D3D11_QUERY_MISC_FLAG(D3D11_QUERY_MISC_FLAG var);
std::string Stringify_D3D11_COUNTER(D3D11_COUNTER var);
std::string Stringify_D3D11_COUNTER_TYPE(D3D11_COUNTER_TYPE var);
std::string Stringify_D3D11_STANDARD_MULTISAMPLE_QUALITY_LEVELS(D3D11_STANDARD_MULTISAMPLE_QUALITY_LEVELS var);
std::string Stringify_D3D11_DEVICE_CONTEXT_TYPE(D3D11_DEVICE_CONTEXT_TYPE var);
std::string Stringify_D3D11_FEATURE(D3D11_FEATURE var);
std::string Stringify_D3D11_CREATE_DEVICE_FLAG(D3D11_CREATE_DEVICE_FLAG var);
}
// @}

#endif // _DC_STRINGIFY_D3D11_ENUMS_H_
