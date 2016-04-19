//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PjsWriter.cpp
/// \brief This file provides the interface to write a PJS (Pre-JIT Symbol) file.
///
//==================================================================================

#include <AMDTProfilingAgentsData/inc/Windows/PjsWriter.h>

PjsWriter::PjsWriter(const wchar_t* pPJSFileName)
{
    m_fileName[0] = L'\0';

    if (NULL != pPJSFileName)
    {
        wcscpy(m_fileName, pPJSFileName);
    }

    m_recordCount = 0;
}

PjsWriter::~PjsWriter()
{
    Close();
}


void PjsWriter::Close()
{
    if (m_fileStream.isOpened())
    {
        // skip 12 bytes (8-byte signature, 4 byte version number)
        m_fileStream.seekCurrentPosition(CrtFile::ORIGIN_BEGIN, 12);
        m_fileStream.write(m_recordCount);
        m_fileStream.close();
    }
}
bool PjsWriter::WriteFileHeader(const wchar_t* pPreJitModuleName, gtVAddr loadAddr)
{
    bool ret = false;

    if (NULL != pPreJitModuleName)
    {
        if (m_fileStream.open(m_fileName, FMODE_TEXT("w+b")))
        {
            // write 8 byte file signature
            m_fileStream.write(PJSFILESIGNATURE, 8);

            // Write file version
            gtUInt32 tData = PREJITSYMBOLFILEVERSION;
            m_fileStream.write(tData);

            // Write dummy record here, we will modify it when closing file
            m_recordCount = 0;
            m_fileStream.write(m_recordCount);

            // write load address, use 8 byte value for all platforms.
            gtUInt64 tAddress = loadAddr;
            m_fileStream.write(tAddress);

            // write ngened module name length, and name string
            tData = static_cast<gtUInt32>(wcslen(pPreJitModuleName));
            m_fileStream.write(tData);
            m_fileStream.write(pPreJitModuleName, sizeof(wchar_t) * tData);

            ret = true;
        }
    }

    return ret;
}

bool PjsWriter::AddRecord(gtVAddr startAddr, gtUInt32 size, const char* pSymbolName)
{
    bool ret = false;

    if (NULL != pSymbolName && m_fileStream.isOpened())
    {
        gtUInt64 tAddress = startAddr;
        m_fileStream.write(tAddress);

        gtUInt32 tData = size;
        m_fileStream.write(tData);

        tData = static_cast<gtUInt32>(strlen(pSymbolName));
        m_fileStream.write(tData);
        m_fileStream.write(pSymbolName, sizeof(char) * tData);

        m_recordCount++;
        ret = true;
    }

    return ret;
}
