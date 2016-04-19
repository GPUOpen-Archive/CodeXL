//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file JavaJncReader_Win.h
///
//==================================================================================

#ifndef _JAVAJNCREADER_WIN_H_
#define _JAVAJNCREADER_WIN_H_

#include "../../inc/ProfilingAgentsDataDLLBuild.h"
#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTExecutableFormat/inc/PeFile.h>
#include "JncJvmtiTypes.h"
#include <QString>
#include <QMap>

#include <QList>

#include <string>
#include <vector>
#include <map>
#include <list>

#ifndef _ADDRESSRANGE_DEFINED
#define _ADDRESSRANGE_DEFINED

typedef struct _AddressRange
{
    jmethodID  id;
    gtUInt64   pc_start;
    gtUInt64   pc_end;
    jlong      method_name_offset;
    jlong      method_signature_offset;
    jlong      source_name_offset;
    jlong      line_number_table_offset;
} AddressRange;

#endif // _ADDRESSRANGE_DEFINED


// Note: the key of map is the offset, the data is line number
typedef QMap<unsigned int, unsigned int> OffsetLinenumMap;

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
// Reader class to read the JNC file written by the CodeXL's JVMTI Agent.

class AGENTDATA_API JavaJncReader
{
public:
    JavaJncReader();
    ~JavaJncReader();

    enum JncSection
    {
        JNC_CODE = 0,   //.text
        JNC_DATA,       //.JVASRC
        JNC_CLRIL,      //.CAIL
        JNC_ILMAP       //.ILMAP
    };

    bool Open(const wchar_t* pImageName);
    void Close();
    void Clear();

    unsigned int GetSectionNum();
    QString GetSourceName();
    QString GetFunctionName();
    gtUInt64 GetLoadAddr();

    OffsetLinenumMap GetOffsetLines();
    bool Is64Bit() const { return m_pExecutable->Is64Bit(); }
    QString GetJncFilePath() { return m_jncFilePath; }

    const gtUByte* GetCodeBytesOfTextSection(unsigned int* pSectionSize);

    JavaInlineMap* GetInlineMap() { return &m_javaInlineMap; }

    bool GetStringFromOffset(jlong offset, string& str);

    void DumpStringTable(FILE* f);
    void DumpJncMethodMap(FILE* f);
    void DumpAddressRangeTable(FILE* f);
    void DumpJncPcStackInfoMap(FILE* f);
    void DumpJavaInlineMap(FILE* f);

    bool GetJittedStartAddr(gtUInt64& startAddr)
    {
        if (m_LoadAddr == 0)
        {
            return false;
        }

        startAddr = m_LoadAddr;
        return true;
    }

    int GetJNCVersion()
    {
        int version = 0;

        if (NULL == m_pExecutable)
        {
            version = 0x3; // previous version without inline;
        }
        else
        {
            gtUInt32 imgVersion = m_pExecutable->GetImageVersion();
            version = static_cast<int>(imgVersion >> 16);
        }

        return version;
    }

    OffsetLinenumMap GetOffsetLines(wstring funcName);

private:
    bool _process_stringtable_section();
    bool _process_bc2src_section();
    bool _process_pc2bc_section();
    bool _process_inline_map();
    void _freePcStackInfo();

    bool _processInlineSections();

    void _fillLineNumSrcMap(JNCInlineMap& ilmap,
                            int numFrames,
                            void* vbcisArray,
                            void* vmethodArray,
                            gtUInt64 thisPc,
                            gtUInt64 nextPc);

    bool _getSrcInfoFromBcAndMethodID(jint       bc,
                                      jmethodID  id,
                                      int&       srcLine,
                                      string&    srcFile);

    const char* GetMethodName(jmethodID methodId);

    bool DoesSectionExist(char* pSectionName);

    bool OpenWithoutInline(const wchar_t* jncFile);
    bool OpenWithInline(const wchar_t* jncFile);

    jlong               m_string_table_size;
    gtUByte*            m_string_table_buf;

    gtUByte*            m_pBc2srcBuf;
    gtUByte*            m_pPc2bcBuf;

    PeFile*             m_pExecutable;
    OffsetLinenumMap    m_lineMap;
    gtUInt64            m_LoadAddr;
    QString             m_SrcName;
    QString             m_FuncName;

    QString             m_jncFilePath;
    const gtUByte*      m_pData;
    const gtUByte*      m_pCode;

    // Inline stuff
    void*               m_pcinfo;
    JncPcStackInfoMap   m_jncPcStackInfoMap;
    JavaInlineMap       m_javaInlineMap;

    // String table stuff
    JNCMethodMap        m_jncMethodMap;
    jmethodID           m_mainMethodId;

    // Method table stuff
    JncAddressRangeVec  m_addressRangeTable;
};

#endif // _JAVAJNCREADER_WIN_H_