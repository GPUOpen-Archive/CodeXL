//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file JclReader.cpp
/// \brief Implements the JclReader class.
///
//==================================================================================

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <JclReader.h>

static int gJclReaderVerbose = 0;

JclReader::JclReader(const wchar_t* jclName)
{
    wcsncpy(m_fileName, jclName, (OS_MAX_PATH - 1));
}


JclReader::~JclReader()
{
    Close();
}


void JclReader::Close()
{
    if (m_fileStream.isOpened())
    {
        m_fileStream.close();
    }
}

bool JclReader::Open()
{
    return m_fileStream.open(m_fileName, WINDOWS_SWITCH(FMODE_TEXT("r+b"), FMODE_TEXT("r+b, ccs=UTF-8")));
}

//
// If the JCL file is of a version that only has load records,
// then this will automatically return load records, otherwise
// it reads the file to determine the type, it depends on the
// caller to call the correct function to read the returned type
//
bool JclReader::ReadNextRecordType(JitRecordType* pRecordType)
{
    bool ret = false;

    if (NULL != pRecordType && m_fileStream.isOpened())
    {
        if ((int)JCL_VERSION > m_version)
        {
            *pRecordType = JIT_LOAD;
            ret = true;
        }
        else
        {
            gtUInt32 type;

            if (m_fileStream.read(type))
            {
                *pRecordType = static_cast<JitRecordType>(type);
                ret = true;
            }
        }
    }

    return ret;
}

bool JclReader::ReadLoadRecord(JitLoadRecord* pJitLoadRec)
{
    bool ret = true;

    if (NULL == pJitLoadRec || !m_fileStream.isOpened() || m_fileStream.isEndOfFile())
    {
        return false;
    }

    gtUInt32 buff_size;
    wchar_t temp_buffer[1024] = { L'\0' };
    memset(temp_buffer, 0, sizeof(temp_buffer));

    m_fileStream.read(pJitLoadRec->loadTimestamp);
    m_fileStream.read(pJitLoadRec->blockStartAddr);
    m_fileStream.read(pJitLoadRec->blockEndAddr);
    m_fileStream.read(pJitLoadRec->threadID);

    // read in java source file name string
    // Note: JCL writer already writes terminate into jcl file.
    m_fileStream.read(buff_size);

    if (0 != buff_size)
    {
        m_fileStream.read(temp_buffer, sizeof(wchar_t) * buff_size);
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        wcsncpy_s(pJitLoadRec->srcFileName, OS_MAX_PATH, temp_buffer, _TRUNCATE);
#else
        wcsncpy(pJitLoadRec->srcFileName, temp_buffer, (OS_MAX_PATH - 1));
#endif
        memset(temp_buffer, 0, sizeof(temp_buffer));
    }

    //read in class function name string
    m_fileStream.read(buff_size);

    if (0 != buff_size)
    {
        m_fileStream.read(temp_buffer, sizeof(wchar_t) * buff_size);
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        wcsncpy_s(pJitLoadRec->classFunctionName, OS_MAX_PATH, temp_buffer, _TRUNCATE);
#else
        wcsncpy(pJitLoadRec->classFunctionName, temp_buffer, (OS_MAX_PATH - 1));
#endif
        memset(temp_buffer, 0, sizeof(temp_buffer));
    }

    //read in jnc file name string
    m_fileStream.read(buff_size);

    if (0 != buff_size)
    {
        if (!m_fileStream.read(temp_buffer, sizeof(wchar_t) * buff_size))
        {
            return false;
        }

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        wcsncpy_s(pJitLoadRec->jncFileName, OS_MAX_PATH, temp_buffer, _TRUNCATE);
#else
        wcsncpy(pJitLoadRec->jncFileName, temp_buffer, (OS_MAX_PATH - 1));
#endif
    }

    if (gJclReaderVerbose)
    {
        printf("loadTimestamp  : %" PRId64 "\n", pJitLoadRec->loadTimestamp);
        printf("blockStartAddr : 0x%" PRId64 "\n", pJitLoadRec->blockStartAddr);
        printf("blockEndAddr   : 0x%" PRId64  "\n", pJitLoadRec->blockEndAddr);
        printf("threadID       : %d\n", pJitLoadRec->threadID);
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        wprintf(L"classFunctionName : %s\n", pJitLoadRec->classFunctionName);
        wprintf(L"jncFileName       : %s\n", pJitLoadRec->jncFileName);
#else
        wprintf(L"classFunctionName : %S\n", pJitLoadRec->classFunctionName);
        wprintf(L"jncFileName       : %S\n", pJitLoadRec->jncFileName);
#endif
    }

    return ret;
}


bool JclReader::ReadUnLoadRecord(JitUnloadRecord* pJitUnloadRec)
{
    bool ret = false;

    if (NULL != pJitUnloadRec && m_fileStream.isOpened() && !m_fileStream.isEndOfFile())
    {
        m_fileStream.read(pJitUnloadRec->unloadTimestamp);

        if (m_fileStream.read(pJitUnloadRec->blockStartAddr))
        {
            ret = true;

            if (gJclReaderVerbose)
            {
                printf("unloadTimestamp : %" PRId64  "\n", pJitUnloadRec->unloadTimestamp);
                printf("blockStartAddr  : 0x%" PRId64 "\n", pJitUnloadRec->blockStartAddr);
            }
        }
    }

    return ret;
}


bool JclReader::ReadHeader(std::wstring* pAppName)
{
    bool ret = false;

    if (NULL != pAppName && (m_fileStream.isOpened() || Open()))
    {
        // read the JclHeader::hdrInfo
        JclHeader hdr;
        m_fileStream.read(hdr.hdrInfo);

        //not a java jcl file
        if (0 == strncmp(hdr.hdrInfo.signature, JCL_HEADER_SIGNATURE, 15))
        {
            ret = true;

            m_version  = hdr.hdrInfo.version;
            m_is32Bit  = hdr.hdrInfo.is32Bit;

            //read the jit app name string
            gtUInt32 buff_size;
            m_fileStream.read(buff_size);

            while (buff_size > 0)
            {
                wchar_t smallBuff[OS_MAX_PATH + 1];
                gtInt32 tempSize = buff_size;

                memset(smallBuff, 0, ((OS_MAX_PATH + 1) * sizeof(wchar_t)));

                if (tempSize > OS_MAX_PATH)
                {
                    tempSize = OS_MAX_PATH;
                }

                if (!m_fileStream.read(smallBuff, sizeof(wchar_t) * tempSize))
                {
                    ret = false;
                }

                pAppName->append(smallBuff);
                buff_size -= tempSize;
            }
        }
    }

    return ret;
}
