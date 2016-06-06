//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file DX10Assembler.h
///
//=====================================================================
#pragma once
#include "d3d10_1.h"

HRESULT D3D10AssembleShader(LPCSTR pSrcData, ID3D10Blob** ppShader, ID3D10Blob** ppErrorMsgs);
typedef BOOL(* D3D10AssembleShaderProc)(LPCSTR pSrcData, ID3D10Blob** ppShader, ID3D10Blob** ppErrorMsgs);
