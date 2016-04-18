//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CrtFile.h
///
//==================================================================================

#ifndef _CRTFILE_H_
#define _CRTFILE_H_

#include <cstdio>
#include <string>

#if AMDT_CPP_COMPILER == AMDT_VISUAL_CPP_COMPILER
    #pragma warning(disable : 4251) // class 'CrtFile' needs to have dll-interface
#endif

#define __UNICODE_TEXT(quote) L##quote

#ifdef _WIN32
    #define FMODE_TEXT(quote) __UNICODE_TEXT(quote)
#else
    #define FMODE_TEXT(quote) quote
#endif

class CrtFile
{
public:
    enum Origin
    {
        ORIGIN_BEGIN = SEEK_SET,
        ORIGIN_CURRENT = SEEK_CUR,
        ORIGIN_END = SEEK_END
    };

    CrtFile() : m_pFile(NULL) {}
    ~CrtFile() { if (isOpened()) { close(); } }

#ifdef _WIN32
    bool open(const wchar_t* pFileName, const wchar_t* pMode)
    {
        return 0 == _wfopen_s(&m_pFile, pFileName, pMode);
    }
#else
    bool open(const wchar_t* pFileName, const char* pMode)
    {
        char fileNameANSI[1024];
        fileNameANSI[0] = '\0';
        wcstombs(fileNameANSI, pFileName, sizeof(fileNameANSI));

        m_pFile = fopen(fileNameANSI, pMode);
        return NULL != m_pFile;
    }
#endif

    bool close() { int res = fclose(m_pFile); m_pFile = NULL; return 0 == res; }
    bool isOpened() const { return NULL != m_pFile; }

    bool isEndOfFile() { return 0 != feof(m_pFile); }

    template <typename Ty>
    bool read(Ty& data) { return read(&data, sizeof(Ty)); }
    bool read(void* pBuf, size_t size) { return 0 != fread(pBuf, size, 1, m_pFile); }

    template <typename Ty>
    bool write(const Ty& data) { return write(&data, sizeof(Ty)); }
    bool write(const void* pBuf, size_t size) { return 0 != fwrite(pBuf, size, 1, m_pFile); }

    void flush() { fflush(m_pFile); }

    bool seekCurrentPosition(Origin origin, long offset) { return 0 == fseek(m_pFile, offset, origin); }
    bool currentPosition(long& offset) { offset = ftell(m_pFile); return -1L != offset; }

    FILE* getHandler() { return m_pFile; }

private:
    FILE* m_pFile;
};

#endif // _CRTFILE_H_
