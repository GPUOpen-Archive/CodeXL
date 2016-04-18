//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscSourceCodeViewer.h
///
//==================================================================================

#ifndef vscSourceCodeViewer_h__
#define vscSourceCodeViewer_h__
#include "CodeXLVSPackageCoreDefs.h"
#include <Include/Public/CoreInterfaces/IVscSourceCodeViewerOwner.h>

void vscSourceCodeViewerOwner_SetOwner(const IVscSourceCodeViewerOwner* pOwner);

#endif // vscSourceCodeViewer_h__
