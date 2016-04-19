//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileInputStream.h
///
//==================================================================================

#ifndef _CPUPROFILEINPUTSTREAM_H_
#define _CPUPROFILEINPUTSTREAM_H_
//CpuProfileInputStream

#include "CpuProfilingRawDataDLLBuild.h"
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <ProfilingAgents/Utils/CrtFile.h>
#include "CpuProfileSample.h"

// key is module name (or, PROCESSDATA, MODDATA),
// data is offset of the file
typedef gtMap<gtString, fpos_t> SectionStreamPosMap;

/****************************
 * class CpuProfileInputStream
 *
 * Description:
 * This is the base class for all readers.
 * It provides base functionality for file access.
 */
class CP_RAWDATA_API CpuProfileInputStream
{
public:
    CpuProfileInputStream();

    virtual ~CpuProfileInputStream();

    void clear();

    /* Note that this function is re-entrant.
     * opening a new file will simply close the old file.
     */
    virtual bool open(const gtString& path);
    virtual void close();

    bool isOpen() const { return m_fileStream.isOpened(); }
    bool isEof() { return m_fileStream.isEndOfFile(); }

    /* Set current input stream to the same
     * location as in another reader */
    void setStream(CpuProfileInputStream* pReader);

    /* Reset input stream to the beginning */
    bool resetStream();

protected:
    /* Read next line from opened input file stream */
    int readLine(gtString& str);

    /* String tokenize function based on QString::section() */
    bool section(gtString& retStr, const gtString& line, const wchar_t* sep, int start, int stop = 0);

    // Wrapper for input stream seekg API
    bool getCurrentPosition(fpos_t* pPos);
    bool setCurrentPosition(fpos_t* pPos);

    /******************************************
     * Stream Position Map APIs
     * The map is used to store stream position
     * of each section once discovered.
     */

    /* Store the input file stream position for future access. */
    void markPos(const gtString& mark);

    /* Returns the previously found file stream position for fast access */
    bool getPos(const gtString& mark, fpos_t* pPos);


    /******************************************
     * Parser Functions
     * These functions parse a comma-separated input string
     * and retrieve a substring from the specified index, then
     * convert and store the appropriate value into the
     * output arguments.
     *
     * NOTE:
     * The argument "Index" will be incremented if parsing is successful.
     * Intended for internal use only.
     */
    bool parsePid(const gtString& line, int& index, ProcessIdType& pid);
    bool parseTotalSample(const gtString& line, int& index, gtUInt64& total);
    bool parseNumEventSet(const gtString& line, int& index, unsigned long& numSet);
    bool parseEvents(const gtString& line, int& index, EventEncodeVec& eventVec, CpuProfileSampleMap& sampMap);
    bool parseIs32Bit(const gtString& line, int& index, bool& is32Bit);
    bool parseHasCss(const gtString& line, int& index, bool& hasCss);

    //If lastIndex is true, the rest of the line is returned, even if it
    //  contains delimiters
    bool parseWstring(const gtString& line, int& index, gtString& name, bool lastIndex = false);

    bool parseVaddr(const gtString& line, int& index, gtVAddr& addr);
    bool parseUINT8(const gtString& line, int& index, gtUByte& val);
    bool parseUINT16(const gtString& line, int& index, gtUInt16& val);
    bool parseUINT32(const gtString& line, int& index, gtUInt32& val);
    bool parseUINT64(const gtString& line, int& index, gtUInt64& val);

protected:
    gtString m_path;
    fpos_t m_bof;
    fpos_t m_eof;
    bool m_isExtStream;
    SectionStreamPosMap m_sectionMap;

private:
    CrtFile m_fileStream;
};

#endif // _CPUPROFILEINPUTSTREAM_H_
