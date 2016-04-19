//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vsdAidOperators.h
///
//==================================================================================

//------------------------------ vsdAidOperators.h ------------------------------

#ifndef __VSDAIDOPERATORS_H
#define __VSDAIDOPERATORS_H

// Visual Studio:
#include <msdbg.h>

// Local:
#include <CodeXLVSPackageDebugger/Include/vsdPackageDLLBuild.h>

bool VSD_API vsdCompareGuids(const GUID& first, const GUID& second);
bool VSD_API operator< (const AD_PROCESS_ID& first, const AD_PROCESS_ID& second);

#endif //__VSDAIDOPERATORS_H

