#include "DXBuilderTester.h"

// C++.
#include <fstream>
#include <streambuf>
#include <string>
#include <iostream>
#include <vector>
#include <sstream>

// Backend.
#include <AMDTBackEnd\Emulator\Parser\Instruction.h>

DXBuilderTester::DXBuilderTester() : m_pBackend(Backend::Instance())
{
}

DXBuilderTester::~DXBuilderTester()
{
}

bool DXBuilderTester::DX_TestIsaSizeExtraction(const std::string& isa, size_t& extractedSize)
{
    bool ret = false;
    extractedSize = 0;

    if (!isa.empty())
    {
        if (m_pBackend != nullptr)
        {
            beProgramBuilderDX* pDxBuilder = m_pBackend->theOpenDXBuilder();
            ret = pDxBuilder->GetIsaSize(isa, extractedSize);
        }
    }

    return ret;
}

// Define the test fixture.
TEST_F(DXBuilderTester, DX_TestIsaSizeExtraction)
{
    // This is the list of ISA files to be tested.
    const size_t NUM_OF_ISA_FILES = 2;
    const std::string ISA_FILES[NUM_OF_ISA_FILES] =
    {
        "C:\\Temp\\dx\\output\\dxISA-Tonga.isa",
        "C:\\Temp\\dx\\output\\dxISA-Bonaire.isa"
    };

    // The expected size in bytes.
    const size_t EXPECTED_SIZE[NUM_OF_ISA_FILES] = { 180, 140 };

    for (size_t i = 0; i < NUM_OF_ISA_FILES; i++)
    {
        // Read the file.
        std::ifstream t(ISA_FILES[i]);
        std::string isaAsText((std::istreambuf_iterator<char>(t)),
                              std::istreambuf_iterator<char>());

        // Do the test.
        size_t detectedSize = 0;
        bool isSizeExtractionSucceeded = DX_TestIsaSizeExtraction(isaAsText, detectedSize);
        ASSERT_TRUE(isSizeExtractionSucceeded);
        ASSERT_TRUE(detectedSize == EXPECTED_SIZE[i]);
    }
}


bool DXBuilderTester::DX_TestWavefrontSizeExtraction(const std::string& deviceName, size_t& extractedSize)
{
    bool ret = false;

    if (m_pBackend != nullptr)
    {
        beProgramBuilderDX* pDxBuilder = m_pBackend->theOpenDXBuilder();
        ret = pDxBuilder->GetWavefrontSize(deviceName, extractedSize);
    }

    return ret;
}

