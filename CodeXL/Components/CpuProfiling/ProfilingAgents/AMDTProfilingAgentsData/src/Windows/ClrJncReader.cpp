//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ClrJncReader.cpp
///
//==================================================================================

#include <AMDTProfilingAgentsData/inc/Windows/ClrJncReader.h>

static bool CopyName(wchar_t* pDst, const wchar_t* pSrc, unsigned int maxLen);

ClrJncReader::ClrJncReader()
{
    m_LoadAddr = 0;
    m_ClassName[0] = '\0';
    m_ModuleName[0] = '\0';
    memset(m_FuncName, 0, (sizeof(wchar_t) * OS_MAX_PATH));
    m_ILoffset = 0;
    m_pILMetaData = nullptr;
    m_ILSize = 0;
    m_ILNativeMapCount = 0;
    m_pILNativeMap = nullptr;
    m_pExecutable = nullptr;
}


ClrJncReader::~ClrJncReader()
{
    Close();
}


bool ClrJncReader::Open(const wchar_t* pImageName)
{
    if (nullptr != pImageName)
    {
        m_pExecutable = ExecutableFile::Open(pImageName);
    }

    bool bRet = (nullptr != m_pExecutable) ? true : false;

    const gtUByte* pCode = nullptr;
    unsigned int offset = 0;
    // copySize is in bytes
    unsigned int copySize = 0;
    const gtUInt32* pData = nullptr;
    const gtUInt64* pUInt64 = nullptr;

    if (bRet)
    {
        pCode = m_pExecutable->GetSectionBytes(m_pExecutable->LookupSectionIndex(".CAIL"));

        if (nullptr != pCode)
        {
            // this is module name length
            pData = reinterpret_cast<const gtUInt32*>(pCode);
            offset += sizeof(gtUInt32);

            if (*pData < (OS_MAX_PATH * sizeof(wchar_t) - 1))
            {
                copySize = *pData;
            }
            else
            {
                copySize = (OS_MAX_PATH * sizeof(wchar_t) - 1);
            }

            memcpy(m_ModuleName, (pCode + offset), copySize);
            m_ModuleName[copySize / sizeof(wchar_t)] = L'\0';
            offset += *pData;

            //  class name;
            pData = reinterpret_cast<const gtUInt32*>(pCode + offset);

            if (*pData < (OS_MAX_PATH * sizeof(wchar_t) - 1))
            {
                copySize = *pData;
            }
            else
            {
                copySize = (OS_MAX_PATH * sizeof(wchar_t) - 1);
            }

            offset += sizeof(gtUInt32);
            memcpy(m_ClassName, (pCode + offset), copySize);
            m_ClassName[copySize / sizeof(wchar_t)] = L'\0';
            offset += *pData;

            //  function name;
            pData = reinterpret_cast<const gtUInt32*>(pCode + offset);

            if (*pData < (OS_MAX_PATH * sizeof(wchar_t) - 1))
            {
                copySize = *pData;
            }
            else
            {
                copySize = (OS_MAX_PATH * sizeof(wchar_t) - 1);
            }

            offset += sizeof(gtUInt32);
            memcpy(m_FuncName, (pCode + offset), copySize);
            m_FuncName[copySize / sizeof(wchar_t)] = L'\0';
            offset += *pData;

            // load address
            pUInt64 = reinterpret_cast<const gtUInt64*>(pCode + offset);
            m_LoadAddr = *pUInt64;
            offset += sizeof(gtUInt64);

            // this is offset to module image
            pData = reinterpret_cast<const gtUInt32*>(pCode + offset);
            m_ILoffset = (*pData);
            offset += sizeof(gtUInt32);

            // this is size of IL (including header)
            pData = reinterpret_cast<const gtUInt32*>(pCode + offset);
            m_ILSize = (*pData);
            offset += sizeof(gtUInt32);

            m_pILMetaData = pCode + offset;
        }
        else
        {
            bRet = false;
        }
    }

    if (bRet)
    {
        pCode = m_pExecutable->GetSectionBytes(m_pExecutable->LookupSectionIndex(".ILMAP"));

        if (nullptr != pCode)
        {
            offset = 0;

            // this is module (length) name
            pData = reinterpret_cast<const gtUInt32*>(pCode);
            offset += *pData + sizeof(gtUInt32);

            //  class name;
            pData = reinterpret_cast<const gtUInt32*>(pCode + offset);
            offset += *pData + sizeof(gtUInt32);

            //  function name;
            pData = reinterpret_cast<const gtUInt32*>(pCode + offset);
            offset += *pData + sizeof(gtUInt32);

            // load address
            offset += sizeof(gtUInt64);

            // this is count of IL To Native Mapping
            pData = reinterpret_cast<const gtUInt32*>(pCode + offset);
            m_ILNativeMapCount = (*pData);
            offset += sizeof(gtUInt32);

            // this is size of IL (including header)
            m_pILNativeMap = reinterpret_cast<const COR_DEBUG_IL_TO_NATIVE_MAP*>(pCode + offset);
        }
        else
        {
            bRet = false;
        }
    }

    return bRet;
}

void ClrJncReader::Close()
{
    if (nullptr != m_pExecutable)
    {
        delete m_pExecutable;
        m_pExecutable = nullptr;
    }
}

unsigned int ClrJncReader::GetSectionNum() const
{
    return (nullptr != m_pExecutable) ? m_pExecutable->GetSectionsCount() : 0U;
}

bool ClrJncReader::DoesSectionExist(char* pSectionName)
{
    bool bRet = false;

    if (nullptr != pSectionName && nullptr != m_pExecutable)
    {
        if (m_pExecutable->LookupSectionIndex(pSectionName) < m_pExecutable->GetSectionsCount())
        {
            bRet = true;
        }
    }

    return bRet;
}


bool ClrJncReader::GetModuleName(wchar_t* pModuleName, unsigned int strLength) const
{
    return CopyName(pModuleName, m_ModuleName, strLength);
}


bool ClrJncReader::GetClassName(wchar_t* pClassName, unsigned int strLength) const
{
    return CopyName(pClassName, m_ClassName, strLength);
}

bool ClrJncReader::GetFunctionName(wchar_t* pFuncName, unsigned int strLength) const
{
    return CopyName(pFuncName, m_FuncName, strLength);
}

gtUInt64 ClrJncReader::GetJITLoadAddress() const
{
    return m_LoadAddr;
}

bool ClrJncReader::GetILInfo(unsigned int* pILOffsetToImage, unsigned int* pILSize) const
{
    bool bRet = false;

    if (0U != m_ILoffset)
    {
        *pILOffsetToImage = m_ILoffset;
        *pILSize = m_ILSize;
        bRet = true;
    }

    return bRet;
}

const gtUByte* ClrJncReader::GetCodeBytesOfTextSection(unsigned int* pSectionOffset, unsigned int* pSectionSize) const
{
    const gtUByte* pData = nullptr;

    if (nullptr != m_pExecutable)
    {
        unsigned numSections = m_pExecutable->GetSectionsCount();
        unsigned sectionIndex = m_pExecutable->LookupSectionIndex(".text");

        if (sectionIndex < numSections)
        {
            pData = m_pExecutable->GetSectionBytes(sectionIndex);

            gtRVAddr sectionStartRva, sectionEndRva;
            m_pExecutable->GetSectionRvaLimits(sectionIndex, sectionStartRva, sectionEndRva);

            if (nullptr != pSectionSize)
            {
                *pSectionSize = sectionEndRva - sectionStartRva;
            }

            if (nullptr != pSectionOffset)
            {
                *pSectionOffset = sectionStartRva;
            }
        }
    }

    return pData;
}

const gtUByte* ClrJncReader::GetILMetaData() const
{
    return m_pILMetaData;
}


unsigned int ClrJncReader::GetILNativeMapCount() const
{
    return m_ILNativeMapCount;
}

const COR_DEBUG_IL_TO_NATIVE_MAP* ClrJncReader::GetILNativeMapInfo() const
{
    return m_pILNativeMap;

}

static bool CopyName(wchar_t* pDst, const wchar_t* pSrc, unsigned int maxLen)
{
    bool bRet = false;

    if (nullptr != pDst)
    {
        size_t srcLen = wcslen(pSrc);

        if (maxLen > srcLen)
        {
            memcpy(pDst, pSrc, sizeof(wchar_t) * srcLen);
            pDst[srcLen] = L'\0';
            bRet = true;
        }
    }

    return bRet;
}
