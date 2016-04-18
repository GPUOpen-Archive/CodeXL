//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Utilities implementation file for MDO.
///         Home of the MdoUtil namespace, containing utility functions used
///         throughout MDO. Here we also find reusable macros, includes, etc.
//==============================================================================

#include "mdoUtil.h"

// For logging/debugging
std::string MdoUtil::FormatWithCommas(UINT32 value)
{
    std::stringstream ss;
    ss.imbue(std::locale(""));
    ss << std::fixed << value;
    return ss.str();
}

// For logging/debugging
void MdoUtil::Print(const char* pStr)
{
    printf("%s", pStr);
    OutputDebugString(pStr);
}

// For logging/debugging
void MdoUtil::PrintLn(const char* pStr)
{
    printf("%s", pStr);
    printf("\n");
    OutputDebugString(pStr);
    OutputDebugString("\n");
}

// For logging/debugging
bool MdoUtil::DumpMemBufferToDiskArray(
    const std::string& name,
    const std::string& path,
    UINT32             bytesPerRow,
    const char*        pBuf,
    UINT32             byteCount)
{
    bool result = false;

    std::string fileName = path + name;

    std::ofstream s(fileName);

    s << "static const unsigned char ";
    s << name;
    s << "[] =  \n";
    s << "{";

    if (s.good())
    {
        for (UINT32 i = 0; i < byteCount; i++)
        {
            if (i % bytesPerRow == 0)
            {
                s << "\n    ";
            }

            UINT32 val = pBuf[i] & 0x000000FF;

            if (val > 0x0F)
            {
                s << "0x";
            }
            else
            {
                s << "0x0";
            }

            std::string valStr;

            do
            {
                valStr += "0123456789ABCDEF"[val % 16];
                val >>= 4;
            }
            while (val != 0);

            s << std::string(valStr.rbegin(), valStr.rend());

            if (i != byteCount - 1)
            {
                s << ", ";
            }
        }

        s << "\n};\n";

        result = true;
    }

    s.close();

    return result;
}

// For logging/debugging
void MdoUtil::DumpFileToDiskArray(
    const std::string& name,
    const std::string& path,
    const std::string& ext,
    UINT32             bytesPerRow)
{
    FILE* pFile = NULL;
    UINT32 bufSize = 0;
    char* pBuffer = NULL;

    std::string fileName = path + name + "." + ext;

    fopen_s(&pFile, fileName.c_str(), "rb");

    fseek(pFile, 0, SEEK_END);
    bufSize = ftell(pFile);
    rewind(pFile);

    pBuffer = new char[bufSize];

    fread(pBuffer, 1, bufSize, pFile);

    DumpMemBufferToDiskArray(name.c_str(), path.c_str(), bytesPerRow, static_cast<char*>(pBuffer), bufSize);

    fclose(pFile);

    delete[] pBuffer;
    pBuffer = NULL;
};
