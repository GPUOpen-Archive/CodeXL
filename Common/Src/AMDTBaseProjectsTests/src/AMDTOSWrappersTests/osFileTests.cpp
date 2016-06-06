#pragma warning(disable : 4996)
#pragma warning(disable : 4100)


#include <gtest/gtest.h>
#include <AMDTOSWrappers/Include/osFile.h>

TEST(osFileIsExecutable, IsExecutableCheck)
{
    gtString path = LR"(c:\temp\NoSuchFile.txt)";
    osFilePath filePath = path;
    osFile myFile(filePath);
    bool res = myFile.IsExecutable();
    EXPECT_FALSE(res);
    
    wchar_t currentProcessPathBuf[MAX_PATH] = {};
    EXPECT_NE(0, GetModuleFileName(nullptr, currentProcessPathBuf, MAX_PATH));
    path = currentProcessPathBuf;
    filePath = path;
    myFile.setPath(filePath);
    res = myFile.IsExecutable();
    EXPECT_TRUE(res);

}
