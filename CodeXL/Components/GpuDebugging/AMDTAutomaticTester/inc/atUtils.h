//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file atUtils.h
///
//==================================================================================

//------------------------------ atUtils.h ------------------------------

#ifndef __ATUTILS_H
#define __ATUTILS_H

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osFilePath.h>

// Base / relative path lists:
void atGenerateBaseFilePathList(const gtString& i_addEnvVarPath, bool i_addSDKPath, gtVector<gtString>& o_basePaths);
bool atMatchFilePathToBasePaths(const gtString& i_filePath, const gtVector<gtString>& i_basePaths, osFilePath& o_matchedPath);

#endif //__ATUTILS_H

