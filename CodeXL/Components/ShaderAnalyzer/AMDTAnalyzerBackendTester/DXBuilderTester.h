#ifndef DXBuilderTester_h__
#define DXBuilderTester_h__

// Google test.
#include <gtest/gtest.h>

// C++.
#include <string>

// Backend.
#include <AMDTBackEnd/Include/beProgramBuilderOpenCL.h>
#include <AMDTBackEnd/Include/beInclude.h>
#include <AMDTBackEnd/Include/beBackend.h>
#include <AMDTBackEnd/Include/beProgramBuilderDX.h>
#include <AMDTBackEnd/Emulator/Parser/ISAParser.h>


using ::testing::Test;

class DXBuilderTester : public Test
{
public:
    DXBuilderTester();

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        TestIsaSizeExtraction
    /// \brief Description: Tests beProgramBuilderDX's GetIsaSize function.
    /// -----------------------------------------------------------------------------------------------
    bool DX_TestIsaSizeExtraction(const std::string& isa, size_t& extractedSize);

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        TestIsaSizeExtraction
    /// \brief Description: Tests beProgramBuilderDX's GetWavefrontSize function.
    /// -----------------------------------------------------------------------------------------------
    bool DX_TestWavefrontSizeExtraction(const std::string& deviceName, size_t& extractedSize);

    ~DXBuilderTester();
private:
    Backend* m_pBackend;
};

#endif // DXBuilderTester_h__