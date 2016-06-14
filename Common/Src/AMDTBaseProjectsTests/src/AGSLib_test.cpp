#pragma warning(disable : 4996)
#pragma warning(disable : 4100)


#include <gtest/gtest.h>
#include <inc/amd_ags.h>

TEST(agsLib, GetVersion)
{
    AGSContext* context = nullptr;
    
    AGSGPUInfo gpuInfo;
    AGSReturnCode rc = agsInit(&context, nullptr, &gpuInfo);
    EXPECT_EQ(rc, AGS_SUCCESS);

}