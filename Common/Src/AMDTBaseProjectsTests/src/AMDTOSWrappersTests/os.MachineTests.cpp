#pragma warning(disable : 4996)
#pragma warning(disable : 4100)


#include <gtest/gtest.h>
#include <boost/algorithm/string.hpp>
#include <AMDTOSWrappers/Include/osMachine.h>

TEST(osGetLocalMachineMemoryInformationStrings, GetStringsOK)
{
    gtUInt64 totalRamSizet = 0;
    gtUInt64 availRamSizet = 0;
    gtUInt64 totalPageSizet = 0;
    gtUInt64 availPageSizet = 0;
    gtUInt64 totalVirtualSizet = 0;
    gtUInt64 availVirtualSizet = 0;

    bool retVal = osGetLocalMachineMemoryInformation(totalRamSizet, availRamSizet, totalPageSizet, availPageSizet, totalVirtualSizet, availVirtualSizet);

    EXPECT_TRUE(retVal);
    EXPECT_GT(totalRamSizet, 0);
    EXPECT_GT(availRamSizet, 0);
    EXPECT_GT(totalPageSizet, 0);
    EXPECT_GT(availPageSizet, 0);
    EXPECT_GT(totalVirtualSizet, 0);
    EXPECT_GT(availVirtualSizet, 0);
    gtString totalRam, availRam, totalPage, availPage, totalVirtual, availVirtual;

    retVal =  osGetLocalMachineMemoryInformationStrings(totalRam, availRam, totalPage, availPage, totalVirtual, availVirtual);
    EXPECT_TRUE(boost::algorithm::contains(totalRam.asCharArray(), L" MB"));
    EXPECT_TRUE(boost::algorithm::contains(availRam.asCharArray(), L" MB"));
    EXPECT_TRUE(boost::algorithm::contains(totalPage.asCharArray(), L" MB"));
    EXPECT_TRUE(boost::algorithm::contains(availPage.asCharArray(), L" MB"));
    EXPECT_TRUE(boost::algorithm::contains(totalVirtual.asCharArray(), L" MB"));
    EXPECT_TRUE(boost::algorithm::contains(availVirtual.asCharArray(), L" MB"));
    EXPECT_TRUE(retVal);
}
