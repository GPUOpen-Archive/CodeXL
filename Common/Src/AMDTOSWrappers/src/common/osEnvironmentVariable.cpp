#include <AMDTOSWrappers/Include/osEnvironmentVariable.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTBaseTools/Include/gtAssert.h>

osEnvVarScope::osEnvVarScope(const std::vector<osEnvironmentVariable>& envVars) : mEnvVars(envVars.begin(), envVars.end())
{
    bool isOk = false;

    for (size_t i = 0; i < envVars.size(); i++)
    {
        isOk = osSetCurrentProcessEnvVariable(envVars[i]);
        GT_ASSERT(isOk);
    }
}

osEnvVarScope::~osEnvVarScope()
{
    bool isOk = false;

    for (size_t i = 0; i < mEnvVars.size(); i++)
    {
        isOk = osRemoveCurrentProcessEnvVariable(mEnvVars[i]._name);
        GT_ASSERT(isOk);
    }

}