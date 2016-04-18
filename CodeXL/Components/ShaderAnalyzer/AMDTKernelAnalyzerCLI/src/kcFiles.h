//=====================================================================
// Copyright 2011 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author  GPU Developer Tools
/// \version $Revision: #2 $
/// \brief   File read/write/&c. utilities.
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/ShaderAnalyzer/AMDTKernelAnalyzerCLI/src/kcFiles.h#2 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $

#ifndef KAUtils_Files_H
#define KAUtils_Files_H

// C++.
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// Local.
#include <AMDTKernelAnalyzerCLI/src/kcDataTypes.h>

using namespace std;

class KAUtils
{

public:
    /// read named file into a string.
    /// \param[in]  inputFile     name of file.
    /// \param[out] programSource content of file.
    /// \returns                  success.
    static bool ReadProgramSource(const std::string& inputFile, std::string& programSource);

    /// Write a binary file.
    /// \param[in]  fileName   the name of the file to be created
    /// \param[in]  content    the contents
    /// \param[in]  pCallback  callback for logging
    static bool WriteBinaryFile(const std::string& fileName, const std::vector<char>& content, LoggingCallBackFunc_t pCallback);

    /// Write a text file.
    /// \param[in]  fileName   the name of the file to be created
    /// \param[in]  content    the contents
    /// \param[in]  pCallback  callback for logging
    static bool WriteTextFile(const std::string& fileName, const std::string& content, LoggingCallBackFunc_t pCallback);

private:
    KAUtils(const KAUtils&);
    KAUtils() {}
    ~KAUtils() {}
};

#endif //  KAUtils_Files_H
