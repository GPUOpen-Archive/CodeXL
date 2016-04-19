//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file OclJncReader.cpp
///
//==================================================================================

#ifdef TBI

#include <AMDTExecutableFormat/inc/SymbolEngine.h>
#include <OclJncReader.h>

OclJncReader::OclJncReader()
{
    m_LoadAddr = 0;
    m_ClassName[0] = '\0';
    m_ModuleName[0] = '\0';
    memset(m_FuncName, 0, (sizeof(wchar_t) * _MAX_PATH));
    m_pJitLineMap = NULL;
}


OclJncReader::~OclJncReader()
{
    Close();
}

HRESULT OclJncReader::Disassemble(gtVAddr address,  char* disassembly, gtUInt32* numBytes, bool* isPCRelative, gtUInt32* dispVal)
{
    BYTE error_code;
    UIInstInfoType temp_struct;
    unsigned strlength = 255;
    //  char dasmstring[256];
    disassembly[0] = '\0';
    gtRVAddr codeOffset = static_cast<gtRVAddr>(address - m_LoadAddr);

    // Check if pCode points to correct segment
    if ((codeOffset < m_sectionStartVAddr) || (codeOffset > m_sectionEndVAddr))
    {
        m_pCode = m_pe.GetCodeBytes(codeOffset, &m_sectionStartVAddr, &m_sectionEndVAddr);

        if (NULL == m_pCode)
        {
            return S_FALSE;
        }
    }

    codeOffset -= m_sectionStartVAddr;

    HRESULT hr = m_dasm.UIDisassemble(&m_pCode[codeOffset], &strlength, (BYTE*)disassembly, &temp_struct, &error_code);

    if (S_OK == hr)
    {
        *isPCRelative = (temp_struct.bIsPCRelative && temp_struct.bHasDispData);
        *numBytes = temp_struct.NumBytesUsed;

        if (temp_struct.bHasDispData)
        {
            *dispVal = temp_struct.DispDataValue;
        }
        else
        {
            *dispVal = 0;
        }
    }

    return hr;
}

void OclJncReader::GetCodeBytes(gtVAddr address, gtUByte* bytes, gtUInt32 size)
{
    gtRVAddr codeOffset = static_cast<gtRVAddr>(address - m_LoadAddr);

    // Check if address is within range of code in pCode.  Reload if not.
    if ((codeOffset < m_sectionStartVAddr) || (codeOffset > m_sectionEndVAddr))
    {
        m_pCode = m_pe.GetCodeBytes(codeOffset, &m_sectionStartVAddr, &m_sectionEndVAddr);

        if (NULL == m_pCode)
        {
            return;
        }
    }

    codeOffset -= m_sectionStartVAddr;

    memcpy(bytes, &m_pCode[codeOffset], size);
}


bool OclJncReader::Open(const wchar_t* pImageName, gtVAddr loadAddr)
{
    bool bRet = false;

    m_pJitLineMap = pSymEngine->GetpLineMap();
    m_LoadAddr = loadAddr;

    do
    {
        if (!pImageName)
        {
            break;
        }

        if (!m_pe.Open(pImageName))
        {
            break;
        }

        m_numSections = m_pe.GetSectionNum();
        m_codeBaseVAddr = m_pe.GetCodeBaseVAddr();

        m_pCode = m_pe.GetSectionStartCodeBytes(0, &m_sectionStartVAddr, &m_sectionEndVAddr);

        if (m_pe.Is64Bit())
        {
            m_dasm.SetLongMode(TRUE);
        }

        bRet = true;

        break;

    }
    while (0);

    return bRet;
}

void OclJncReader::Close()
{
    m_pe.Close();
}

unsigned int OclJncReader::GetSectionNum()
{
    return m_pe.GetSectionNum();
}



bool OclJncReader::DoesSectionExist(char* pSectionName)
{
    bool bRet = false;

    if (pSectionName)
    {
        if (NULL != m_pe.GetCodeBytes(pSectionName))
        {
            bRet = true;
        }
    }

    return bRet;
}


bool OclJncReader::GetModuleName(wchar_t* pModuleName, unsigned int strLength)
{
    bool bRet = false;

    if (pModuleName && (strLength > wcslen(m_ModuleName)))
    {
        wcscpy_s(pModuleName, strLength, m_ModuleName);
        bRet = true;
    }

    return bRet;
}


bool OclJncReader::GetClassName(wchar_t* pClassName, unsigned int strLength)
{
    bool bRet = false;

    if (pClassName && (strLength > wcslen(this->m_ClassName)))
    {
        wcscpy_s(pClassName, strLength, m_ClassName);
        bRet = true;
    }

    return bRet;
}

bool OclJncReader::GetFunctionName(wchar_t* pFuncName, unsigned int strLength)
{
    bool bRet = false;

    if (pFuncName && (strLength > wcslen(m_FuncName)))
    {
        wcscpy_s(pFuncName, strLength, m_FuncName);
        bRet = true;
    }

    return bRet;
}

gtUInt64 OclJncReader::GetJITLoadAddress()
{
    return m_LoadAddr;
}

gtUByte* OclJncReader::GetCodeBytesOfTextSection(unsigned int* pSectionOffset, unsigned int* pSectionSize)
{
    gtUByte* pData = NULL;
    pData = m_pe.GetCodeBytes(".text", pSectionSize);

    if (pData)
    {
        *pSectionOffset = m_pe.GetCodeOffset();
    }

    return pData;
}

#endif // TBI
