//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file JavaJncReader_Lin.h
/// \brief The JavaJncReader class to read the JNC files created by the
///        JVMTI profile agent created on Linux.
///
//==================================================================================

#ifndef _JAVAJNCREADER_LIN_H_
#define _JAVAJNCREADER_LIN_H_

#include <QMap>
#include "../../inc/ProfilingAgentsDataDLLBuild.h"
#include "../../inc/JncJvmtiTypes.h"
#include <AMDTExecutableFormat/inc/ExecutableFile.h>

#include <string>
#include <vector>
#include <map>
#include <list>

// Note: the key of map is the offset, the data is line number
typedef QMap<unsigned int, unsigned int> OffsetLinenumMap;

using namespace std;

#ifndef _ADDRESSRANGE_DEFINED
#define _ADDRESSRANGE_DEFINED
typedef struct _AddressRange
{
    jmethodID id;
    gtUInt64  pc_start;
    gtUInt64  pc_end;
    jlong     method_name_offset;
    jlong     method_signature_offset;
    jlong     source_name_offset;
    jlong     line_number_table_offset;
} AddressRange;
#endif // _ADDRESSRANGE_DEFINED

typedef struct
{
    gtUInt64  startAddr;
    gtUInt64  stopAddr;
} addrRanges;

typedef std::list<addrRanges> AddressRangeList;

typedef struct _LineNumSrc
{
    int               lineNum;
    string            sourceFile;
    string            symName;
    jmethodID         methodId;
    AddressRangeList  addrs;

    bool operator== (const struct _LineNumSrc& other)
    {
        return ((methodId == other.methodId) &&
                (sourceFile == other.sourceFile) &&
                //              (lineNum == other.lineNum) &&
                (symName == other.symName));
    }
} LineNumSrc;

// Key is starting PC
typedef std::multimap<gtUInt64, LineNumSrc> JNCInlineMap;

// Indexed by "parent" line number
// Has map of inlined function info for each "parent" line
typedef std::multimap<int, JNCInlineMap> JavaInlineMap;

class AGENTDATA_API JavaJncReader
{
public:
    JavaJncReader();

    virtual ~JavaJncReader();

    bool Open(const wchar_t* pFileName);
    void Close();
    void Clear();
    bool GetStringFromOffset(unsigned int offset, string& str);
    void DumpStringTable(FILE* f);
    void DumpJncMethodMap(FILE* f);
    void DumpAddressRangeTable(FILE* f);
    void DumpJncPcStackInfoMap(FILE* f);
    void DumpJavaInlineMap(void);

    bool GetSrcInfoFromAddressWithFile(gtUInt64  addr,
                                       int&                srcLine,
                                       string&             srcFile);

    bool GetSrcInfoFromAddress(gtUInt64  addr,
                               int&                 srcLine,
                               string&              srcFile,
                               int&                 inlineDepth);

    bool GetSymbolAndRangeFromAddr(gtUInt64  addr,
                                   string&    symName,
                                   gtUInt64&  startAddr,
                                   gtUInt64&  endAddr);

    string GetJittedFunctionName() { return m_methodName; }

    bool GetJittedStartEndAddr(gtUInt64& startAddr, gtUInt64& endAddr)
    {
        if (m_loadAddr == 0)
        {
            return false;
        }

        startAddr = m_loadAddr;
        endAddr   = m_loadAddr + m_textSize;

        return true;
    }

    bool GetJittedStartAddr(gtUInt64& startAddr)
    {
        if (m_loadAddr == 0)
        {
            return false;
        }

        startAddr = m_loadAddr;
        return true;
    }

    bool GetJittedFunctionSrcInfo(
        string& fileName,
        unsigned int& startLine,
        unsigned int& possibleEndLine)
    {
        if (m_loadAddr == 0)
        {
            return false;
        }

        fileName        = m_srcFile;
        startLine       = m_startLine;
        possibleEndLine = m_possibleEndLine;

        return true;
    }

    JavaInlineMap* GetInlineMap() { return &m_javaInlineMap; }

    const char* GetMethodName(jmethodID methodId);

    OffsetLinenumMap GetOffsetLines();
    OffsetLinenumMap GetOffsetLines(wstring funcName);

    const gtUByte* GetCodeBytesOfTextSection(unsigned int* pCodeSize);

private:
    bool _process_stringtable_section();
    bool _process_bc2src_section();
    bool _process_pc2bc_section();
    bool _process_inline_map();
    void _freePcStackInfo();

    jmethodID _getMethodFromAddr(gtUInt64 addr);

    int _getLineFromAddrVec(gtUInt64 addr,
                            JncJvmtiLineNumberEntryVec& vec);

    void _fillLineNumSrcMap(JNCInlineMap& ilmap,
                            int numFrames,
                            void* vbcisArray,
                            void* vmethodArray,
                            gtUInt64 thisPc,
                            gtUInt64 nextPc);

    ExecutableFile* m_pExecutable;
    unsigned int m_sectionCounts;

    unsigned int m_string_table_size;
    const gtUByte*  m_string_table_buf;

    const gtUByte*  m_pCodeBuf;
    const gtUByte*  m_pBc2srcBuf;
    const gtUByte*  m_pPc2bcBuf;

    bool _getSrcInfoFromBcAndMethodID(jint       bc,
                                      jmethodID  id,
                                      int&       srcLine,
                                      string&    srcFile);

    // String table stuff
    JNCMethodMap        m_jncMethodMap;
    jmethodID           m_mainMethodId;

    // Method table stuff
    JncAddressRangeVec  m_addressRangeTable;

    // Inline stuff
    void*               m_pcinfo;
    JncPcStackInfoMap   m_jncPcStackInfoMap;
    JavaInlineMap       m_javaInlineMap;

    gtVAddr               m_loadAddr;
    string              m_methodName;
    string              m_srcFile;
    bool                m_mainInlineDepth;
    unsigned int        m_startLine;
    unsigned int        m_possibleEndLine;

    gtUInt64              m_textOffset;
    gtUInt64              m_textSize;

    // Code offset - Src Line number map - for the source view
    OffsetLinenumMap    m_lineMap;
};

#endif // _JAVAJNCREADER_LIN_H_
