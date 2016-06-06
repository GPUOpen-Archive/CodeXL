//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file WinPackages.cpp
///
//==================================================================================

#include <windows.h>
#include <sddl.h>

#include <algorithm>
#include <AMDTOSWrappers/Include/osApplication.h>
#include "AMDTWinComponentExtUtils/include/WinPackages.h"
#include "AMDTWinComponentExtUtils/include/WinAppExp.h"
#include <shobjidl.h>
#include <collection.h>
#include <AppModel.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/include/osDebugLog.h>

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace std;
//using Windows.Foundation.UniversalApiContract.winmd;

LIBRARY_API int  GetAppInfo(gtList<WindowsStoreAppInfo>& outAppInfoList)
{
    return WinPackages::GetApplicationInfo(outAppInfoList);
}

[STAThread]
int WinPackages::GetApplicationInfo(AppInfoList& outAppInfoList)
{
    CoInitialize(nullptr);

    try
    {
        outAppInfoList.clear();
        auto packageManager = ref new Windows::Management::Deployment::PackageManager();
        auto packages = packageManager->FindPackages();


        std::for_each(Windows::Foundation::Collections::begin(packages), Windows::Foundation::Collections::end(packages),
                      [&packageManager, &outAppInfoList](Windows::ApplicationModel::Package ^ package)
        {

            try
            {
                if (false == package->IsDevelopmentMode && false == package->IsResourcePackage)
                {
                    WindowsStoreAppInfo appInfo;
                    appInfo.m_name = package->Id->Name->Data();
                    //appInfo.m_userModelId = package->Id->PublisherId->Data();
                    appInfo.m_userModelId = UserIdApp(package->Id->FamilyName->Data(), package->Id->PublisherId->Data());

                    if (package->InstalledLocation != nullptr && package->InstalledLocation->Path != nullptr)
                    {
                        appInfo.m_packageDirectory = package->InstalledLocation->Path->Data();
                    }

                    try
                    {
                        if (nullptr != package->Logo && nullptr != package->Logo->AbsoluteUri)
                        {
                            appInfo.m_logoPath = gtString(package->Logo->AbsoluteUri->Data());
                        }
                    }
                    catch (Exception^ ex)
                    {
                        OS_OUTPUT_DEBUG_LOG(ex->ToString()->Data(), OS_DEBUG_LOG_INFO);
                    }

                    outAppInfoList.push_back(appInfo);
                }
                else
                {
                    gtString msg = L"package is for development or  resource package :";
                    msg += package->Id->Name->Data();
                    OS_OUTPUT_DEBUG_LOG(msg.asCharArray(), OS_DEBUG_LOG_INFO);
                }

            }
            catch (Exception^ ex)
            {
                OS_OUTPUT_DEBUG_LOG(ex->ToString()->Data(), OS_DEBUG_LOG_INFO);
            }
        });

    }
    catch (AccessDeniedException^ ex)
    {
        OS_OUTPUT_DEBUG_LOG(ex->ToString()->Data(), OS_DEBUG_LOG_INFO);
        GT_ASSERT(false);
        return 1;
    }
    catch (Exception^ ex)
    {
        OS_OUTPUT_DEBUG_LOG(ex->ToString()->Data(), OS_DEBUG_LOG_INFO);
        GT_ASSERT(false);
        return 1;
    }

    CoUninitialize();
    return 0;
}

gtString WinPackages::UserIdApp(const gtString& FamilyName, const gtString& RelativeAppId)
{
    PCWSTR packageFamilyName = FamilyName.asCharArray();
    PCWSTR packageRelativeApplicationId = RelativeAppId.asCharArray();
    gtString appUserModelId;
    UINT32 length = 0;
    LONG rc = FormatApplicationUserModelId(packageFamilyName, packageRelativeApplicationId, &length, NULL);

    GT_IF_WITH_ASSERT(rc != ERROR_SUCCESS && rc == ERROR_INSUFFICIENT_BUFFER)
    {
        PWSTR applicationUserModelId = (PWSTR)malloc(length * sizeof(WCHAR));

        GT_IF_WITH_ASSERT(applicationUserModelId != nullptr)
        {
            rc = FormatApplicationUserModelId(packageFamilyName, packageRelativeApplicationId, &length, applicationUserModelId);

            GT_IF_WITH_ASSERT(rc == ERROR_SUCCESS)
            {
                appUserModelId = applicationUserModelId;
            }
            free(applicationUserModelId);
        }
    }

    return appUserModelId;
}