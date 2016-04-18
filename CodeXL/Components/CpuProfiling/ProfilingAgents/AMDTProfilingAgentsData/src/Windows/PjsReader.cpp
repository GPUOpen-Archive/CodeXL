//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PjsReader.cpp
/// \brief This file provides the interface to read a PJS (Pre-JIT Symbol) file.
///
//==================================================================================

#include <AMDTProfilingAgentsData/inc/Windows/PjsReader.h>

PjsReader::PjsReader()
{
    m_pModuleName = NULL;
    m_LoadAddr = 0;
    m_RecordRead = 0;
    m_RecordCount = 0;
}

PjsReader::~PjsReader()
{
    Close();

    if (NULL != m_pModuleName)
    {
        delete [] m_pModuleName;
        m_pModuleName = NULL;
    }
}

bool PjsReader::Open(const wchar_t* pFileName)
{
    bool ret = false;

    if (NULL != pFileName)
    {
        if (m_fileStream.open(pFileName, FMODE_TEXT("r+b")))
        {
            // read 8 byte file signature
            char signature[8];
            m_fileStream.read(signature);

            if (0 == memcmp(signature, PJSFILESIGNATURE, 8))
            {
                // read file version
                gtUInt32 tData;
                m_fileStream.read(tData);

                // read dummy record here, we will modify it when closing file
                m_RecordCount = 0;
                m_fileStream.read(m_RecordCount);

                // read load address, use 8 byte value for all platforms.
                m_fileStream.read(m_LoadAddr);

                // read ngened module name length, and name string
                m_fileStream.read(tData);
                m_pModuleName = new wchar_t [tData + 1];

                if (NULL != m_pModuleName)
                {
                    memset(m_pModuleName, 0, tData + 1);
                    m_fileStream.read(m_pModuleName, sizeof(wchar_t) * tData);
                    m_pModuleName[tData] = L'\0';

                    ret = true;
                }
            }
        }
    }

    return ret;
}

void PjsReader::Close()
{
    if (m_fileStream.isOpened())
    {
        m_fileStream.close();
    }
}

unsigned int PjsReader::GetNumberOfRecords()
{
    return m_RecordCount;
}

bool PjsReader::GetPreJITModuleName(wchar_t* pModuleName, unsigned int size)
{
    bool bRet = false;

    if (NULL != m_pModuleName)
    {
        size_t len = wcslen(m_pModuleName);

        if (size > len)
        {
            wcscpy(pModuleName, m_pModuleName);
            bRet = true;
        }
        else
        {
            wcsncpy(pModuleName, m_pModuleName, size - 1);
        }
    }

    return bRet;
}

gtUInt64 PjsReader::GetLoadAddress()
{
    return m_LoadAddr;
}

bool PjsReader::GetNextRecord(gtUInt64* pStartAddr, gtUInt32* pSize, char* pSymbol, gtUInt32 symbolLength)
{
    bool bRet = false;

    if (NULL != pStartAddr && NULL != pSize && NULL != pSymbol && m_RecordRead < m_RecordCount && m_fileStream.isOpened())
    {
        gtUInt64 tAddress;
        m_fileStream.read(tAddress);
        *pStartAddr = tAddress;

        m_fileStream.read(*pSize);

        gtUInt32 len;
        m_fileStream.read(len);

        if (symbolLength > len)
        {
            if (m_fileStream.read(pSymbol, sizeof(char) * len))
            {
                pSymbol[len] = '\0';
                bRet = true;
            }
        }

        m_RecordRead++;
    }

    return bRet;
}
