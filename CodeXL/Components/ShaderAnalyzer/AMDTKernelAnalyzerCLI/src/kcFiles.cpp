//=====================================================================
// Copyright 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author  GPU Developer Tools
/// \file    $File: //devtools/main/CodeXL/Components/ShaderAnalyzer/AMDTKernelAnalyzerCLI/src/kcFiles.cpp $
/// \version $Revision: #2 $
/// \brief   File read/write/&c. utilities.
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/ShaderAnalyzer/AMDTKernelAnalyzerCLI/src/kcFiles.cpp#2 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================

// C++.
#include <sstream>

// Infra.
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osDirectory.h>

// Local.
#include <AMDTKernelAnalyzerCLI/src/kcFiles.h>

#ifndef _WIN32
    #pragma GCC diagnostic ignored "-Wunused-variable"
    #include <dirent.h>
#endif

using namespace std;

bool KAUtils::ReadProgramSource(const string& inputFile, string& programSource)
{
    std::ifstream input;

#ifndef _WIN32
    // test if the input file is a directory.
    // On some Linux machine ifstream open will be valid if it is a directory
    // but will not allow to read data which will cause a crash when trying to read the data
    DIR* pDir;

    pDir = opendir(inputFile.c_str());

    if (pDir != NULL)
    {
        (void) closedir(pDir);
        return false;
    }

#endif

    // Open (at e)nd of file.  We want the length below.
    // Open binary because it's faster & other bits of code deal OK with CRLF, LF etc.
    input.open(inputFile.c_str(), ios::ate | ios::binary);

    if (!input)
    {
        return false;
    }

    std::ifstream::pos_type fileSize = 0;

    fileSize = input.tellg();

    if (fileSize == static_cast<std::ifstream::pos_type>(0))
    {
        input.close();
        return false;
    }

    input.seekg(0, ios::beg);

    programSource.resize(size_t(fileSize));
    input.read(&programSource[0], fileSize);

    input.close();
    return true;
}


bool KAUtils::WriteBinaryFile(const std::string& fileName, const std::vector<char>& content, LoggingCallBackFunc_t pCallback)
{
    bool ret = false;
    ofstream output;
    output.open(fileName.c_str(), ios::binary);

    if (output.is_open() && !content.empty())
    {
        output.write(&content[0], content.size());
        output.close();
        ret = true;
    }
    else
    {
        std::stringstream log;
        log << "Error: Unable to open " << fileName << " for write.\n";

        if (pCallback != nullptr)
        {
            pCallback(log.str());
        }
    }

    return ret;
}

bool KAUtils::WriteTextFile(const std::string& fileName, const std::string& content, LoggingCallBackFunc_t pCallback)
{
    bool ret = false;
    ofstream output;
    output.open(fileName.c_str());

    if (output.is_open())
    {
        output << content << std::endl;
        output.close();
        ret = true;
    }
    else
    {
        std::stringstream log;
        log << "Error: Unable to open " << fileName << " for write.\n";

        if (pCallback != nullptr)
        {
            pCallback(log.str());
        }
    }

    return ret;
}
