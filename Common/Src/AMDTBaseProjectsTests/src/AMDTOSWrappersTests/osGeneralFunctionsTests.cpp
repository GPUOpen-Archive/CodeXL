#pragma warning(disable : 4996)
#pragma warning(disable : 4100)
// Qt:
#include <QtWidgets>

#include <gtest/gtest.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include "osTCPSocketClient.h"
#include "osPortAddress.h"
#include <boost/asio.hpp>
#include "osTCPSocketServer.h"
#include "AMDTBaseTools/Include/gtAssert.h"
#include "osProcess.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <AMDTKernelAnalyzer/src/kaDataAnalyzerFunctions.h>
#include <AMDTKernelAnalyzer/src/kaProjectDataManager.h>


TEST(FillKernelNamesList, CheckAttributes)
{
    QString sourceCode = "__kernel       __attribute__     	((reqd_work_group_size(LOCAL_XRES, LOCAL_YRES, 1)))   	    void    	    advancedSeparableConvolution(__global uchar4 *input, __global float *row_filter, __global float *col_filter,__global uchar4 *output, uint nWidth,	uint nHeight,uint nExWidth){} \n";
    osFilePath filePath(LR"(C:\Users\rbober\Downloads\AdvancedConvolution_Kernels.cl)");
    vector<std::string> additionalMacros = {};
    gtVector<kaProjectDataManagerAnaylzeData> result;
    kaProjectDataManager::FillKernelNamesList(sourceCode, filePath, additionalMacros, result);
}
TEST(TestWave, ExpandMacros)
{
#define KERNEL_NAME1 "OpenCL1"
#define KERNEL_NAME2 "OpenCL2"

    string sourceCode = "__kernel void " KERNEL_NAME1 " () { } MACRO void " KERNEL_NAME2 " () { }";
    wstring fileName = L"dummy.cl";
    string result;
    std::vector<std::string> additionalMacros{ "MACRO=__kernel"};
    std::vector<PreProcessedToken> tokens;
    ExpandMacros(sourceCode, fileName, additionalMacros, tokens);
    vector<pair<string, size_t>> kernelNamesPostions;
    auto  token = tokens.begin();

    while (token != tokens.end())
    {
        if (token->value == "__kernel")
        {
            //skip  to return value
            while (++token != tokens.end() && std::isspace(token->value[0]));

            if (token != tokens.end())
            {
                //skip  spaces till kerenl name
                while (++token != tokens.end() && std::isspace(token->value[0]));

                if (token != tokens.end())
                {
                    kernelNamesPostions.push_back(make_pair(token->value, token->line));
                    ++token;
                }
            }
        }
        else
        {
            ++token;
        }
    }

    EXPECT_TRUE(kernelNamesPostions.size() == 2);
    EXPECT_TRUE(kernelNamesPostions[0].first == KERNEL_NAME1);
    EXPECT_TRUE(kernelNamesPostions[1].first == KERNEL_NAME2);

}
TEST(TestGetOperatingSystemVersionNumber, BuildNumberZero)
{
    int i = -1, j = -1, k = -1;
    EXPECT_TRUE(osGetOperatingSystemVersionNumber(i, j, k));
    EXPECT_TRUE(0 == k);

    i = -1, j = -1, k = -1;
    EXPECT_TRUE(osGetOperatingSystemVersionNumber(i, j, k));
    EXPECT_TRUE(0 == k);

    osWindowsVersion windowsVersion;
    EXPECT_TRUE(osGetWindowsVersion(windowsVersion));

}

TEST(TesstCreateDirectory, CreateOK)
{
#define DIR_ROOT1 LR"(C:\temp\AMD\)";
#define DIR_ROOT2 LR"(C:\temp\OutputDir32\)";
    gtString dirStr = DIR_ROOT1 LR"(CodeXL\Project3_AnalyzerOutput\ProgramCL2\OutputDir32\ruk\)";
    gtString dirStr2 = DIR_ROOT2 LR"(buk\duk)";
    osDirectory dir(dirStr);
    osDirectory dir2(dirStr2);
    EXPECT_TRUE(dir.create());
    EXPECT_TRUE(dir2.create());

    //cleanup
    dirStr = DIR_ROOT1;
    EXPECT_TRUE(osDirectory(dirStr).deleteRecursively());
    dirStr = DIR_ROOT2;
    EXPECT_TRUE(osDirectory(dirStr).deleteRecursively());


}

TEST(TesstGTAsciiString, EmptyStringOk)
{
    gtASCIIString str;
    std::string larg;

    for (int i = 0; i < 4096; ++i)
    {
        larg.push_back('a');
    }

    str.appendFormattedString("");
    str.appendFormattedString(larg.c_str());

}

TEST(TestSocketOccupied, IsOccupied)
{
    bool result = osIsLocalPortAvaiable(8080);
    GT_ASSERT(result);
}

TEST(TESTProcessUsageInfo, UsageOK)
{
    unsigned  int  processID = GetCurrentProcessId(), PageFaultCount;
    size_t WorkingSetSize,
           PeakWorkingSetSize,
           QuotaPeakPagedPoolUsage,
           QuotaPagedPoolUsage,
           QuotaPeakNonPagedPoolUsage,
           QuotaNonPagedPoolUsage,
           PagefileUsage,
           PeakPagefileUsage,
           PrivateUsage;
    bool res = osGetMemoryUsage(processID,
                                PageFaultCount,
                                WorkingSetSize,
                                PeakWorkingSetSize,
                                QuotaPeakPagedPoolUsage,
                                QuotaPagedPoolUsage,
                                QuotaPeakNonPagedPoolUsage,
                                QuotaNonPagedPoolUsage,
                                PagefileUsage,
                                PeakPagefileUsage,
                                PrivateUsage);
    EXPECT_TRUE(res);
}