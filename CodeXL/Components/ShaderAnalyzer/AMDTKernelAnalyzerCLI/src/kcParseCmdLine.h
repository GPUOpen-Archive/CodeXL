// -*- C++ -*-
//=====================================================================
// Copyright 2011 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/ShaderAnalyzer/AMDTKernelAnalyzerCLI/src/kcParseCmdLine.h $
/// \version $Revision: #1 $
/// \brief  This file contains the function to parse the command
///         line using Boost.
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/ShaderAnalyzer/AMDTKernelAnalyzerCLI/src/kcParseCmdLine.h#1 $
// Last checkin:   $DateTime: 2016/02/28 16:32:28 $
// Last edited by: $Author: igal $
// Change list:    $Change: 561710 $
//=====================================================================

// Reduced from APPProfiler\Backend\sprofile\ParseCmdLine.h

#ifndef _PARSE_CMD_LINE_H_
#define _PARSE_CMD_LINE_H_

class Config;

/// Parse the command line arguments
/// \param[in]  argc      the number of arguments
/// \param[in]  argv      the array of argument strings
/// \param[out] configOut the output config structure
/// \return true if successful, false otherwise
bool ParseCmdLine(int argc, char* argv[], Config& configOut);

#endif
