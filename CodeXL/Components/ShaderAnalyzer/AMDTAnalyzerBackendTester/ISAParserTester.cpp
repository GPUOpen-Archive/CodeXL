#include "ISAParserTester.h"

// Define the test fixture.
TEST_F(ISAParserTester, CL_TestIsaParsing)
{
    // This is the list of ISA files to be tested.
    const size_t NUM_OF_ISA_FILES = 2;
    const std::string ISA_FILES[NUM_OF_ISA_FILES] =
    {
        "C:\\Temp\\cl\\output\\clISA-Tonga.isa",
        "C:\\Temp\\cl\\output\\clISA-Bonaire.isa"
    };

    for (size_t i = 0; i < NUM_OF_ISA_FILES; i++)
    {
        // Read the file.
        std::ifstream t(ISA_FILES[i]);
        std::string isaAsText((std::istreambuf_iterator<char>(t)),
                              std::istreambuf_iterator<char>());

        bool isParsingSuccessful = TestIsaParsing(isaAsText);
        ASSERT_TRUE(isParsingSuccessful);
    }

}

bool ISAParserTester::TestIsaParsing(const std::string& isa)
{
    bool isaParseSuccess = m_isaParser.Parse(isa);

    if (isaParseSuccess)
    {
        std::vector<Instruction*> isaInstructions = m_isaParser.GetInstructions();
        size_t numOfInstructions = isaInstructions.size();

        if (numOfInstructions > 0)
        {
        }
    }

    return isaParseSuccess;
}

// Define the test fixture.
TEST_F(ISAParserTester, DX_TestIsaParsing)
{
    // This is the list of ISA files to be tested.
    const size_t NUM_OF_ISA_FILES = 2;
    const std::string ISA_FILES[NUM_OF_ISA_FILES] =
    {
        "C:\\Temp\\dx\\output\\dxISA-Tonga.isa",
        "C:\\Temp\\dx\\output\\dxISA-Bonaire.isa"
    };

    for (size_t i = 0; i < NUM_OF_ISA_FILES; i++)
    {
        // Read the file.
        std::ifstream t(ISA_FILES[i]);
        std::string isaAsText((std::istreambuf_iterator<char>(t)),
                              std::istreambuf_iterator<char>());

        bool isParsingSuccessful = TestIsaParsing(isaAsText);
        ASSERT_TRUE(isParsingSuccessful);
    }

}


// Define the test fixture.
TEST_F(ISAParserTester, CL_TestIsaLineSplitting)
{
    // This is the list of ISA files to be tested.
    const size_t NUM_OF_ISA_FILES = 1;
    const std::string ISA_FILES[NUM_OF_ISA_FILES] =
    { "C:\\Temp\\isaParser\\dxISA-Carrizo_instr_only.isa" };

    bool isIsaLineSplitSucessful = false;

    for (size_t i = 0; i < NUM_OF_ISA_FILES; i++)
    {
        // Read the file.
        std::ifstream fin(ISA_FILES[i]);

        // Test each ISA source code line.
        std::string isaLine;

        while (std::getline(fin, isaLine))
        {
            const size_t MIN_LINE_LEN = 32;

            if (isaLine.length() > MIN_LINE_LEN)
            {
                isIsaLineSplitSucessful = TestIsaLineSplitting(isaLine);
                ASSERT_TRUE(isIsaLineSplitSucessful);
            }
        }
    }
}
bool ISAParserTester::TestIsaLineSplitting(const std::string& isaLine)
{
    std::string instr;
    std::string params;
    std::string instrBinRep;
    std::string offset;

    bool ret = m_isaParser.SplitIsaLine(isaLine, instr, params, instrBinRep, offset);
    std::stringstream outStream;
    outStream << "ISA Line: " << isaLine << std::endl <<
              "> Instruction: " << instr << std::endl <<
              "> Parameters:  " << params << std::endl <<
              "> Binary form: " << instrBinRep << std::endl <<
              "> offset:      " << offset << std::endl;

    std::cout << outStream.str();

    return ret;
}

