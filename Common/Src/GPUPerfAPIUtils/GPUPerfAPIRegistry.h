//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Functions for setting driver-specific registry keys or env variables
///         to enable perf counter collection
//==============================================================================

#ifndef _GPUPERFAPI_REGISTRY_H_
#define _GPUPERFAPI_REGISTRY_H_

#include <string>

/// Set environment variable to enable Soft CP mode in the HSA runtime
/// \param shouldPrintDbgMsg Print out debug message to console
/// \return True if operation succeeded
bool SetHSASoftCPEnvVar(bool shouldPrintDbgMsg);

/// Unset environment variable to enable Soft CP mode in the HSA runtime
/// \param shouldPrintDbgMsg Print out debug message to console
/// \return True if operation succeeded
bool UnsetHSASoftCPEnvVar(bool shouldPrintDbgMsg);

/// Set environment variable to enable Soft CP mode in the HSA runtime
/// \param strErrorMsg error message, if regienv var could not be set
/// \return True if operation succeeded
bool SetHSASoftCPEnvVar(std::string& strErrorMsg);

/// Unset environment variable to enable Soft CP mode in the HSA runtime
/// \param strErrorMsg error message, if env var could not be unset
/// \return True if operation succeeded
bool UnsetHSASoftCPEnvVar(std::string& strErrorMsg);

#endif // _GPUPERFAPI_REGISTRY_H_
