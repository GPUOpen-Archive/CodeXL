#include <AMDTBackEnd/Include/beDriverUtils.h>
#include <ADLUtil.h>

beDriverUtils::beDriverUtils()
{
}


beDriverUtils::~beDriverUtils()
{
}

bool beDriverUtils::GetDriverPackagingVersion(gtString& driverPackagingVersion)
{
    bool ret = false;
    ADLVersionsInfo versionInfo;
    ADLUtil_Result rc = ADLUtil_GetVersionsInfo(versionInfo);

    if (rc == ADL_SUCCESS)
    {
        driverPackagingVersion.fromASCIIString(versionInfo.strDriverVer, ADL_MAX_PATH);
        ret = true;
    }

    return ret;
}
