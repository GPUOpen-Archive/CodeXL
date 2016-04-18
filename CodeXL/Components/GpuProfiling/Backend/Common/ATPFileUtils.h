//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file contains some functions shared between OCL and HSA ATP File modules
//==============================================================================

#ifndef _ATP_FILE_UTILS_H
#define _ATP_FILE_UTILS_H

#include <set>

/// Read excluded APIs from API filter file
/// \param strAPIFilterFile API filter file
/// \param[out] excludedAPIs List of excluded APIs
void ReadExcludedAPIs(const std::string& strAPIFilterFile, std::set<std::string>& excludedAPIs);

/// Write excluded APIs to stream
/// \param sout stream used for writing
/// \param pPrefix the prefix to prepend before the key in the file header
/// \param excludedAPIs List of excluded APIs to write
void WriteExcludedAPIs(SP_fileStream& sout, const char* pPrefix, std::set<std::string> excludedAPIs);

#endif // _ATP_FILE_UTILS_H
