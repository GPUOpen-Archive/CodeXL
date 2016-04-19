//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscSourceCodeViewer.cpp
///
//==================================================================================

#include <Include\Public\vscSourceCodeViewer.h>
#include <src/vspSourceCodeViewer.h>

void vscSourceCodeViewerOwner_SetOwner(const IVscSourceCodeViewerOwner* pOwner)
{
    vspSourceCodeViewer& pInstance = vspSourceCodeViewer::instance();
    pInstance.setOwner(pOwner);
}
