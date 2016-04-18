//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileInputStream.cpp
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/Backend/AMDTCpuProfilingRawData/src/Translation/CpuProfileInputStream.cpp#4 $
// Last checkin:   $DateTime: 2016/04/14 02:12:02 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569056 $
//=====================================================================

#include <CpuProfileInputStream.h>

CpuProfileInputStream::CpuProfileInputStream()
{
    clear();
}


CpuProfileInputStream::~CpuProfileInputStream()
{
    close();
}


void CpuProfileInputStream::clear()
{
    if (m_fileStream.isOpened())
    {
        m_fileStream.close();
    }

    m_path.makeEmpty();
    memset(&m_bof, 0, sizeof(fpos_t));
    memset(&m_eof, 0, sizeof(fpos_t));
    m_isExtStream = false;
    m_sectionMap.clear();
}


bool CpuProfileInputStream::open(const gtString& path)
{
    if (path.isEmpty())
    {
        return false;
    }

    // Check if file is already open
    if (m_fileStream.isOpened())
    {
        if (path != m_path)
        {
            // Close and open a new file
            close();
        }
        else
        {
            return true;
        }
    }

    // Windows Note:
    // The profile files are opened with UTF-8 encoding.
    //
    if (!m_fileStream.open(path.asCharArray(), WINDOWS_SWITCH(FMODE_TEXT("r, ccs=UTF-8"), FMODE_TEXT("rb"))))
    {
        return false;
    }

    // Set path name
    m_path = path;

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS

    if (fwide(m_fileStream.getHandler(), 1) <= 0)
    {
        close();
        return false;
    }

    // Note: For Linux
    // Due to a bug in some version of gcc,
    // we add this to make sure that we are
    // starting from the beginning of the file.
    m_fileStream.seekCurrentPosition(CrtFile::ORIGIN_BEGIN, 0);
#endif
    getCurrentPosition(&m_bof);

    // Get length of file
    m_fileStream.seekCurrentPosition(CrtFile::ORIGIN_END, 0);
    getCurrentPosition(&m_eof);

    // Back to the beginning of file
    setCurrentPosition(&m_bof);
    return true;
}


void CpuProfileInputStream::close()
{
    if (m_fileStream.isOpened() && !m_isExtStream)
    {
        m_fileStream.close();
    }

    clear();
}


bool CpuProfileInputStream::resetStream()
{
    bool ret = false;

    if (m_fileStream.isOpened())
    {
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
        // NOTE:This is a hack for GCC-4.1.1
        // to force resetting the stream location
        long off;
        m_fileStream.currentPosition(off);
#endif

        ret = setCurrentPosition(&m_bof);
    }

    return ret;
}


void CpuProfileInputStream::setStream(CpuProfileInputStream* pReader)
{
    if (NULL != pReader)
    {
        m_path       = pReader->m_path;
        m_fileStream = pReader->m_fileStream;
        m_bof        = pReader->m_bof;
        m_eof        = pReader->m_eof;
        m_sectionMap = pReader->m_sectionMap;
        m_isExtStream = true;
    }
}


int CpuProfileInputStream::readLine(gtString& str)
{
    wchar_t buf = L'\0';
    str.makeEmpty();

    if (!m_fileStream.isOpened())
    {
        return -1;
    }

    while (!isEof())
    {
        buf = fgetwc(m_fileStream.getHandler());

        if (buf == (wchar_t) WEOF)
        {
            break;
        }

        if (buf != L'\n')
        {
            str += buf;
        }
        else
        {
            str += L'\0';
            break;
        }
    }

    return str.length();
}


bool CpuProfileInputStream::section(gtString& retStr, const gtString& line, const wchar_t* sep, int start, int stop)
{
    retStr.makeEmpty();
    int count = 0;
    int begin = 0;
    int end   = 0;

    // Sanity check
    if ((line.length() <= 0)
        || (start < 0)
        || (stop != 0 && start > stop))
    {
        return false;
    }

    // Find start
    if (start == 0)
    {
        begin = 0;
    }
    else
    {
        while (count < start)
        {
            begin = line.find(sep, begin);

            if (begin != -1)
            {
                count++;
                begin++;
            }
            else
            {
                return false;
            }
        }
    }

    // Find stop
    if (stop == 0)
    {
        line.getSubString(begin, line.length() - 1, retStr);
    }
    else
    {
        end = begin + 1;

        while (count < stop)
        {
            end = line.find(sep, end);

            if (end != -1)
            {
                count++;
                end++;
            }
            else
            {
                line.getSubString(begin, line.length() - 1, retStr);
                return !retStr.isEmpty();
            }
        }

        line.getSubString(begin, end - 2, retStr);
    }

    return !retStr.isEmpty();
}

bool CpuProfileInputStream::getCurrentPosition(fpos_t* pPos)
{
    return 0 == fgetpos(m_fileStream.getHandler(), pPos);
}

bool CpuProfileInputStream::setCurrentPosition(fpos_t* pPos)
{
    return 0 == fsetpos(m_fileStream.getHandler(), pPos);
}

void CpuProfileInputStream::markPos(const gtString& mark)
{
    if (!mark.isEmpty() && m_fileStream.isOpened() && !isEof())
    {
        fpos_t pos;
        getCurrentPosition(&pos);
        m_sectionMap.insert(SectionStreamPosMap::value_type(mark, pos));
    }
}


bool CpuProfileInputStream::getPos(const gtString& mark, fpos_t* pPos)
{
    SectionStreamPosMap::iterator it = m_sectionMap.find(mark);
    bool ret = (it != m_sectionMap.end());

    if (ret)
    {
        *pPos = it->second;
    }

    return ret;
}


bool CpuProfileInputStream::parsePid(const gtString& line, int& index, ProcessIdType& pid)
{
    gtString str;
    bool ret = section(str, line, L",", index, index + 1);

    if (ret)
    {
        unsigned long tmp = 0UL;
        str.toUnsignedLongNumber(tmp);
        pid = static_cast<ProcessIdType>(tmp);
        index++;
    }

    return ret;
}


bool CpuProfileInputStream::parseUINT8(const gtString& line, int& index, gtUByte& val)
{
    gtString str;
    bool ret = section(str, line, L",", index, index + 1);

    if (ret)
    {
        unsigned int tmp = 0U;
        str.toUnsignedIntNumber(tmp);
        val = static_cast<gtUByte>(tmp);
        index++;
    }

    return ret;
}


bool CpuProfileInputStream::parseUINT16(const gtString& line, int& index, gtUInt16& val)
{
    gtString str;
    bool ret = section(str, line, L",", index, index + 1);

    if (ret)
    {
        unsigned int tmp = 0U;
        str.toUnsignedIntNumber(tmp);
        val = static_cast<gtUInt16>(tmp);
        index++;
    }

    return ret;
}


bool CpuProfileInputStream::parseUINT32(const gtString& line, int& index, gtUInt32& val)
{
    gtString str;
    bool ret = section(str, line, L",", index, index + 1);

    if (ret)
    {
        val = 0;
        str.toUnsignedIntNumber(val);
        index++;
    }

    return ret;
}


bool CpuProfileInputStream::parseUINT64(const gtString& line, int& index, gtUInt64& val)
{

    gtString str;
    bool ret = section(str, line, L",", index, index + 1);

    if (ret)
    {
        unsigned long long tmp = 0ULL;
        str.toUnsignedLongLongNumber(tmp);
        val = static_cast<gtUInt64>(tmp);
        index++;
    }

    return ret;
}


bool CpuProfileInputStream::parseTotalSample(const gtString& line, int& index,  gtUInt64& total)
{
    gtString str;
    bool ret = section(str, line, L",", index, index + 1);

    if (ret)
    {
        unsigned long long tmp = 0ULL;
        str.toUnsignedLongLongNumber(tmp);
        total = static_cast<gtUInt64>(tmp);
        index++;
    }

    return ret;
}


bool CpuProfileInputStream::parseNumEventSet(const gtString& line, int& index, unsigned long& numSet)
{
    gtString str;
    bool ret = section(str, line, L",", index, index + 1);

    if (ret)
    {
        numSet = 0UL;
        str.toUnsignedLongNumber(numSet);
        index++;
    }

    return ret;
}


bool CpuProfileInputStream::parseEvents(const gtString& line, int& index, EventEncodeVec& eventVec, CpuProfileSampleMap& sampMap)
{
    gtString buffer;
    unsigned int evIndex = 0;
    int k = index;

    // Get each [CPU INDEX] #SAMPLE for Version 6 or later;
    for (; section(buffer, line, L",", k, k + 1); k++)
    {
        SampleKey key;
        unsigned long count = 0;

        if (!section(buffer, line, L",", k, k + 1))
        {
            return false;
        }

        if (buffer[0] != L'[')
        {
            break;
        }

        swscanf(buffer.asCharArray(), L"[%u %u] %lu", &key.cpu, &evIndex, &count);

        if (evIndex >= eventVec.size())
        {
            return false;
        }

        // This is index, convert it to event;
        key.event = eventVec[evIndex].eventMask;
        sampMap.insert(CpuProfileSampleMap::value_type(key, count));
    }

    index = k;
    return true;
}


bool CpuProfileInputStream::parseIs32Bit(const gtString& line, int& index, bool& is32Bit)
{
    // Get 32-bit-flag:
    // 32-BIT-FLAG, for version 6 or higher;
    gtString threeTwoBitFlag;
    bool ret = section(threeTwoBitFlag, line, L",", index, index + 1);

    if (ret)
    {
        is32Bit = (L"1" == threeTwoBitFlag);
        index++;
    }

    return ret;
}


bool CpuProfileInputStream::parseHasCss(const gtString& line, int& index, bool& hasCss)
{
    gtString css;
    bool ret = section(css, line, L",", index, index + 1);

    if (ret)
    {
        hasCss = (L"1" == css);
        index++;
    }

    return ret;
}


bool CpuProfileInputStream::parseWstring(const gtString& line, int& index, gtString& name, bool lastIndex)
{
    int endIndex = index + 1;

    if (lastIndex)
    {
        endIndex = 0;
    }

    //Force a deep copy
    bool ret = section(name, line, L",", index, endIndex);
    index += static_cast<int>(ret);

    return ret;
}


bool CpuProfileInputStream::parseVaddr(const gtString& line, int& index, gtVAddr& addr)
{
    gtString addrStr;
    bool ret = section(addrStr, line, L",", index, index + 1);

    if (ret)
    {
        unsigned long long tmp = 0ULL;
        addrStr.toUnsignedLongLongNumber(tmp);
        addr = static_cast<gtVAddr>(tmp);
        index++;
    }

    return ret;
}
