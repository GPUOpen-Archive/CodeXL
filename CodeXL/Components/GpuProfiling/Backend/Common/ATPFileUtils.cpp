//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file ATPFileUtils.cpp
/// \brief  This file contains some functions shared between OCL and HSA ATP File modules
//==============================================================================

#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>

#include "FileUtils.h"
#include "StringUtils.h"

#include "ATPFileUtils.h"

void ReadExcludedAPIs(const std::string& strAPIFilterFile, std::set<std::string>& excludedAPIs)
{
    if (strAPIFilterFile.empty())
    {
        return;
    }

    std::vector<std::string> tmpArr;
    FileUtils::ReadFile(strAPIFilterFile, tmpArr, true);

    for (std::vector<std::string>::iterator it = tmpArr.begin(); it != tmpArr.end(); ++it)
    {
        // use set for better lookup performance
        std::string name = StringUtils::Trim(*it);

        if (!name.empty())
        {
            excludedAPIs.insert(name);
        }
    }
}

void WriteExcludedAPIs(SP_fileStream& sout, const char* pPrefix, std::set<std::string> excludedAPIs)
{
    // Write excluded APIs
    sout << pPrefix;
    sout << "ExcludedAPIs=";

    for (std::set<std::string>::const_iterator it = excludedAPIs.begin(); it != excludedAPIs.end(); ++it)
    {
        if (it != excludedAPIs.begin())
        {
            sout << ",";
        }

        sout << (*it).c_str();
    }

    sout << std::endl;
}
