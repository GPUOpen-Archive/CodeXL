#pragma warning(disable : 4996)
#pragma warning(disable : 4100)


#include <iostream>
#include <gtest/gtest.h>
#include <inc/amd_ags.h>

using namespace std;
TEST(agsLib, GetVersion)
{
    AGSContext* agsContext = nullptr;
    AGSGPUInfo gpuInfo;
    AGSReturnCode rc = agsInit(&agsContext, nullptr, &gpuInfo);
    EXPECT_EQ(rc, AGS_SUCCESS);
    cout << "Radeon version " << gpuInfo.radeonSoftwareVersion << endl;
    rc = agsDeInit(agsContext);
    agsContext = nullptr;
    EXPECT_EQ(rc, AGS_SUCCESS);

}