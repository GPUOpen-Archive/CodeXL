#ifndef ISAParserTester_h__
#define ISAParserTester_h__

// Google test.
#include <gtest/gtest.h>

// C++.
#include <string>

// Backend.
#include <AMDTBackEnd/Include/beProgramBuilderOpenCL.h>
#include <AMDTBackEnd/Include/beInclude.h>
#include <AMDTBackEnd/Include/beBackend.h>
#include <AMDTBackEnd/Emulator/Parser/ISAParser.h>

using ::testing::Test;

class ISAParserTester : public Test
{
public:
    ISAParserTester() {}
    ~ISAParserTester() {}

    /// --------------------------------------------------------
    /// \brief Name:        TestIsaParsing
    /// \brief Description: Tests parsing of the given ISA code.
    /// --------------------------------------------------------
    bool TestIsaParsing(const std::string& isa);

    /// --------------------------------------------------------
    /// \brief Name:        TestIsaLineSplitting
    /// \brief Description: Tests splitting of the given ISA source code line.
    /// \Note: the input should only contain ISA instructions (other sections that
    ///        usually appear in the compiler's output should be removed.
    /// --------------------------------------------------------
    bool TestIsaLineSplitting(const std::string& isaInstructions);

private:
    ParserISA m_isaParser;
};
#endif // ISAParserTester_h__