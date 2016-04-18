//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file JclWriter.cpp
/// \brief Implements the JclWriter class.
///
//==================================================================================

#include <AMDTProfilingAgentsData/inc/JclWriter.h>
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #ifndef _GLIBCXX_TR1_INTTYPES_H
        #define _GLIBCXX_TR1_INTTYPES_H 1
        #include <tr1/cinttypes>
    #endif // _GLIBCXX_TR1_INTTYPES_H
#endif
#include <inttypes.h>
//
//    Macros
//

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define JCL_MUTEX_INIT()        InitializeCriticalSection(&m_criticalSection)
    #define JCL_MUTEX_LOCK()        EnterCriticalSection (&m_criticalSection);
    #define JCL_MUTEX_UNLOCK()      LeaveCriticalSection (&m_criticalSection);
    #define JCL_MUTEX_DESTROY()     DeleteCriticalSection(&m_criticalSection)
#else
    #define JCL_MUTEX_INIT()        pthread_mutex_init(&m_criticalSection, NULL)
    #define JCL_MUTEX_LOCK()        pthread_mutex_lock(&m_criticalSection)
    #define JCL_MUTEX_UNLOCK()      pthread_mutex_unlock(&m_criticalSection)
    #define JCL_MUTEX_DESTROY()     pthread_mutex_destroy(&m_criticalSection)
#endif

#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
    static bool gJclVerbose = 0;
#endif

JclWriter::JclWriter(const wchar_t* pJclName, const wchar_t* pAppName, int pid, bool closeOnRecordWrite)
{
    m_recordCount = 0;
    m_pid = pid;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    InitializeCriticalSection(&m_criticalSection);

    wcsncpy_s(m_fileName, OS_MAX_PATH, pJclName, _TRUNCATE);
#else
    // Linux
    pthread_mutexattr_t  attr;

    pthread_mutexattr_init(&attr);
    int ret = pthread_mutex_init(&m_criticalSection, &attr);

    if (0 != ret)
    {
        printf("error pthread_mutex_init... \n");
    }

    wcsncpy(m_fileName, pJclName, OS_MAX_PATH);
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    m_lenAppName = static_cast<unsigned int>(wcslen(pAppName));
    m_pAppName = new wchar_t[m_lenAppName + 1U];

    if (0U != m_lenAppName)
    {
        memcpy(m_pAppName, pAppName, sizeof(wchar_t) * m_lenAppName);
    }

    m_pAppName[m_lenAppName] = L'\0';

    m_closeOnRecordWrite = closeOnRecordWrite;

#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD

    if (gJclVerbose)
    {
        printf("/*** JCLWriter::JCLWriter ***/\n");
        wprintf(L"file name is %ws\n", m_fileName);
        wprintf(L"javaAppName is %ws\n", m_pAppName);
    }

#endif
}


JclWriter::~JclWriter()
{

    //rewrite the header, so the record count will be recorded
    if (m_fileStream.isOpened())
    {
        // Here we re-write the header just to JCL records count field (JclHeader::numRecords).
        // No need to update the header as the JclHeader::numRecords is unused.
        // m_fileStream.seekCurrentPosition(CrtFile::ORIGIN_BEGIN, 0);
        // WriteHeader();

        m_fileStream.close();
    }

    JCL_MUTEX_DESTROY();

    delete [] m_pAppName;
}

bool JclWriter::Initialize()
{
    bool ret = false;

    JCL_MUTEX_LOCK();

    if (m_fileStream.open(m_fileName, WINDOWS_SWITCH(FMODE_TEXT("wb"), FMODE_TEXT("wb, ccs=UTF-8"))))
    {
        WriteHeader();
        ret = true;
    }

    // Close the file
    if (m_closeOnRecordWrite)
    {
        m_fileStream.close();
    }

    JCL_MUTEX_UNLOCK();

    return ret;
}

void JclWriter::WriteHeader()
{
    JclHeader header;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    strncpy_s(header.hdrInfo.signature, 16, JCL_HEADER_SIGNATURE, _TRUNCATE);
#else
    snprintf(header.hdrInfo.signature, 16, "%s", JCL_HEADER_SIGNATURE);
#endif

    header.hdrInfo.version    = JCL_VERSION;
    header.hdrInfo.processID  = m_pid;
    header.hdrInfo.numRecords = m_recordCount;
    header.hdrInfo.is32Bit = static_cast<gtInt32>(AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE);

    m_fileStream.write(header.hdrInfo);

    // write string appName
    gtUInt32 size = m_lenAppName + 1U;
    m_fileStream.write(size);
    m_fileStream.write(m_pAppName, sizeof(wchar_t) * size);

    m_fileStream.flush();
}

bool JclWriter::WriteLoadRecord(JitLoadRecord* pRecord)
{
    bool ret = false;

    if (NULL != pRecord)
    {
        JCL_MUTEX_LOCK();

        if (m_closeOnRecordWrite && !m_fileStream.isOpened())
        {
            m_fileStream.open(m_fileName, WINDOWS_SWITCH(FMODE_TEXT("ab"), FMODE_TEXT("ab, ccs=UTF-8")));
        }

        if (m_fileStream.isOpened())
        {
#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD

            if (gJclVerbose)
            {
                printf("/*** writeLoadRecord ***/\n");
                printf("loadTimestamp %" PRId64 "\n", pRecord->loadTimestamp);
                printf("blockStartAddr %" PRIx64 "\n", pRecord->blockStartAddr);
                printf("blockEndAddr %" PRIx64 "\n", pRecord->blockEndAddr);
                printf("threadID %d\n", pRecord->threadID);
                wprintf(L"classFunctionName %ws\n", pRecord->classFunctionName);
                wprintf(L"jncFileName %ws\n", pRecord->jncFileName);
            }

#endif

            gtInt32 type = JIT_LOAD;
            gtUInt32 size = 0;

            m_fileStream.write(type);
            m_fileStream.write(pRecord->loadTimestamp);
            m_fileStream.write(pRecord->blockStartAddr);
            m_fileStream.write(pRecord->blockEndAddr);
            m_fileStream.write(pRecord->threadID);

            // write string source file name
            size = static_cast<gtUInt32>(wcslen(pRecord->srcFileName) + 1);
            m_fileStream.write(size);
            m_fileStream.write(pRecord->srcFileName, sizeof(wchar_t) * size);

            // write string class::function name
            size = static_cast<gtUInt32>(wcslen(pRecord->classFunctionName) + 1);
            m_fileStream.write(size);
            m_fileStream.write(pRecord->classFunctionName, sizeof(wchar_t) * size);

            // write jnc file name
            size = static_cast<gtUInt32>(wcslen(pRecord->jncFileName) + 1);
            m_fileStream.write(size);
            m_fileStream.write(pRecord->jncFileName, sizeof(wchar_t) * size);

            // flush every time, in case the java app gets terminated...
            m_recordCount++;
            m_fileStream.flush();

            if (m_closeOnRecordWrite)
            {
                m_fileStream.close();
            }

            ret = true;
        }

        JCL_MUTEX_UNLOCK();
    }

    return ret;
}

bool JclWriter::WriteUnloadRecord(JitUnloadRecord* pRecord)
{
    bool ret = false;

    if (NULL != pRecord)
    {
        JCL_MUTEX_LOCK();

        if (m_closeOnRecordWrite && !m_fileStream.isOpened())
        {
            m_fileStream.open(m_fileName, WINDOWS_SWITCH(FMODE_TEXT("ab"), FMODE_TEXT("ab, ccs=UTF-8")));
        }

        if (m_fileStream.isOpened())
        {
#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD

            if (gJclVerbose)
            {
                printf("/*** writeUnloadRecord ***/\n");
                printf("unloadTimestamp %" PRIu64 "\n", pRecord->unloadTimestamp);
                printf("blockStartAddr %" PRIx64 "\n", pRecord->blockStartAddr);
            }

#endif

            gtInt32 type = JIT_UNLOAD;

            m_fileStream.write(type);
            m_fileStream.write(pRecord->unloadTimestamp);
            m_fileStream.write(pRecord->blockStartAddr);

            // flush every time, in case the java app gets terminated...
            m_recordCount++;
            m_fileStream.flush();

            if (m_closeOnRecordWrite)
            {
                m_fileStream.close();
            }

            ret = true;
        }

        JCL_MUTEX_UNLOCK();
    }

    return ret;
}
