//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apAIDFunctions.h
///
//==================================================================================

//------------------------------ apAIDFunctions.h ------------------------------

#ifndef __APAIDFUNCTIONS
#define __APAIDFUNCTIONS

// This file contain utility functions for GRApiClasses module

// Infra:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>

bool AP_API apIsTextureUniformType(GLenum uniformType);
void AP_API apHandleXMLEscaping(gtString& string, bool toEscape);
bool AP_API apLookForFileInAdditionalDirectories(const osFilePath& targetFile, const gtString& additionalDir, osFilePath& foundPath);

#endif  // __APAIDFUNCTIONS
