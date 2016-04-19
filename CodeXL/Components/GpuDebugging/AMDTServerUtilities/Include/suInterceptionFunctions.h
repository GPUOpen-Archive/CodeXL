//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suInterceptionFunctions.h
///
//==================================================================================

//------------------------------ suInterceptionFunctions.h ------------------------------

#ifndef __SUINTERCEPTIONFUNCTIONS_H
#define __SUINTERCEPTIONFUNCTIONS_H

// Forward declarations:
class gtString;

// Local:
#include <AMDTServerUtilities/Include/suSpiesUtilitiesDLLBuild.h>

void SU_API suTestFunction();
bool SU_API suGetSpiesDirectory(gtString& spiesDirectory, bool isRunningInStandaloneMode);
bool SU_API suHandleCRInSources(unsigned int sourceCount, const char* const* sources, unsigned int* sourceLengths, gtASCIIString& o_modifiedSource);


#endif //__SUINTERCEPTIONFUNCTIONS_H

