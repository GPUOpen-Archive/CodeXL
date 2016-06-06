//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osEnvironmentVariable.h
///
//=====================================================================

//------------------------------ osEnvironmentVariable.h ------------------------------

#ifndef __OSENVIRONMENTVARIABLE
#define __OSENVIRONMENTVARIABLE
//std
#include <vector>

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTOSWrappers/Include/osEnvironmentVariable.h>
#include <AMDTOSWrappers/Include/osOSWrappersDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:           osEnvironmentVariable
// General Description: Represents an environment variable.
// Author:      AMD Developer Tools Team
// Creation Date:        6/9/2005
// ----------------------------------------------------------------------------------
struct OS_API osEnvironmentVariable
{
    osEnvironmentVariable(const gtString& varName, const gtString& varValue) : _name(varName), _value(varValue) {}
    osEnvironmentVariable() {}
    // The environment variable name:
    gtString _name;

    // The environment variable value:
    gtString _value;
};

// Defines the scope for spawning a child process using RAII.
// In the CTOR the relevant environment variables are being set.
// In the DTOR the environment variables which were set in the CTOR are being unset.
// Note: if C++11 is used we will be able to use move semantics instead of copy for the
// environment variables vector.
class OS_API osEnvVarScope
{
public:
    osEnvVarScope(const std::vector<osEnvironmentVariable>& envVars);

    virtual ~osEnvVarScope();

private:
    std::vector<osEnvironmentVariable> mEnvVars;
};
#endif  // __OSENVIRONMENTVARIABLE
