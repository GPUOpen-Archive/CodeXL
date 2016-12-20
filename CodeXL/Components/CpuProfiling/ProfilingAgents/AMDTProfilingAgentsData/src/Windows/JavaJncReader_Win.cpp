//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file JavaJncReader_Win.cpp
///
//==================================================================================

#include <stdio.h>

#include <JvmsParser.h>

#pragma warning(push)
#pragma warning(disable : 4311 4312 4718) //ignore qt warnings
#include "JavaJncReader_Win.h"
#pragma warning(pop)

static FILE* debugFP = nullptr;

JavaJncReader::JavaJncReader()
{
    Clear();

    m_SrcName = L"UnknownJITSource";
}

JavaJncReader::~JavaJncReader()
{
    Close();
}

#if 0

bool JavaJncReader::Open(const wchar_t* pImageName)
{
    bool bRet = false;

    do
    {
        if (!pImageName)
        {
            break;
        }

        m_pExecutable = new PeFile(pImageName);

        if (nullptr == m_pExecutable)
        {
            break;
        }

        if (!m_pExecutable->Open())
        {
            delete m_pExecutable;
            m_pExecutable = nullptr;
            break;
        }

        // If the open succeeds, eventually return TRUE
        // Must return true even if a JNC file does not
        // contain a .data section (BUG153481)
        bRet = true;

        //get the start of the native stuff
        gtUByte* pCode = nullptr;
        gtRVAddr sectionStartRVAddr, sectionEndRVAddr;

        //get the data section, if available.
        pCode = m_pExecutable->GetSectionStartCodeBytes(1, &sectionStartRVAddr, &sectionEndRVAddr);

        if (nullptr == pCode)
        {
            break;
        }

        //Skip stuff
        wchar_t temp [1024];
        DWORD strLength;
        // length for javaSrcname;
        memcpy(&strLength, pCode, sizeof(unsigned int));
        pCode += sizeof(unsigned int);

        if (strLength < MAX_PATH)
        {
            wcscpy_s(temp, MAX_PATH, reinterpret_cast<wchar_t*>(pCode));
            m_SrcName = temp;
        }

        pCode += strLength;
        // length for class name
        memcpy(&strLength, pCode , sizeof(DWORD));
        pCode += sizeof(unsigned int) + strLength;
        //  lenght for jncFuncName
        memcpy(&strLength, pCode, sizeof(DWORD));
        pCode += sizeof(unsigned int);

        if (strLength < 1024)
        {
            wcscpy_s(temp, 1024, reinterpret_cast<wchar_t*>(pCode));
            m_FuncName = temp;
        }

        pCode += strLength;
        // load address UInt64
        m_LoadAddr = *((gtUInt64*)pCode);
        pCode += sizeof(gtUInt64);

        DWORD offsetLineCnt = 0;
        memcpy(&offsetLineCnt, pCode, sizeof(DWORD));
        pCode += sizeof(DWORD);

        //read line number map
        // Note: the key of map is the offset, the data is line number
        DWORD codeOffset;
        DWORD line = 0xFFFFFFFF;
        DWORD maxline = 0;

        for (unsigned int i = 0; i < offsetLineCnt; i++)
        {
            memcpy(&codeOffset, pCode, sizeof(DWORD));

            // In the JVMTA agent with version earlier than (FILEVERSION 1,3,1,6 or PRODUCTVERSION 2,69.300,0)
            // JVMTI was writing RVA into .jnc file rather than offset, for source line mapping.
            // To keep consistency, starting from JVMTIA (version 1.3.1.6), we will write offset.
            // In future, we should remove the following two lines.
            // -Lei 04/05/2007
            if (codeOffset >= m_LoadAddr)
            {
                codeOffset = static_cast<DWORD>(static_cast<gtUInt64>(codeOffset) - m_LoadAddr);
            }

            pCode += sizeof(DWORD);
            DWORD lineNum;
            memcpy(&lineNum, pCode, sizeof(DWORD));
            pCode += sizeof(DWORD);
            m_lineMap[codeOffset] = lineNum;

            if (lineNum < line)
            {
                line = lineNum;
            }
        }

    }
    while (0);

    return bRet;
}

#endif

bool JavaJncReader::Open(const wchar_t* jncFile)
{
    if (nullptr != jncFile)
    {
        m_pExecutable = nullptr;
        m_pExecutable = new PeFile(jncFile);

        if (nullptr == m_pExecutable)
        {
            return false;
        }

        if (!m_pExecutable->Open())
        {
            delete m_pExecutable;
            m_pExecutable = nullptr;
            return false;
        }

        gtUInt32 version = m_pExecutable->GetImageVersion();

        if (version < 0x40000)
        {
            // older JNC file format without inline info
            return OpenWithoutInline(jncFile);
        }
        else
        {
            // JNC with inline info
            return OpenWithInline(jncFile);
        }
    }

    return false;
}


bool JavaJncReader::OpenWithoutInline(const wchar_t* jncFile)
{
    if (nullptr == jncFile)
    {
        return false;
    }

    if (nullptr == m_pExecutable)
    {
        m_pExecutable = new PeFile(jncFile);

        if (nullptr == m_pExecutable)
        {
            return false;
        }

        if (!m_pExecutable->Open())
        {
            delete m_pExecutable;
            m_pExecutable = nullptr;
            return false;
        }
    }

    unsigned int sectionNum = m_pExecutable->GetSectionsCount();
    bool symAvail = false;

    if (sectionNum > 1)
    {
        //locate the Java symbol section
        m_pData = m_pExecutable->GetSectionBytes(m_pExecutable->LookupSectionIndex(".JVASRC"));

        if (nullptr == m_pData)
        {
            //Need to try to read data from older jnc files.
            m_pData = m_pExecutable->GetSectionBytes(m_pExecutable->LookupSectionIndex(".data"));
        }

        if (nullptr != m_pData)
        {
            for (unsigned int i = 1; i < sectionNum; i++)
            {
                if (m_pExecutable->GetSectionBytes(i) == m_pData)
                {
                    symAvail = true;
                    break;
                }
            }
        }
    }

    if ((sectionNum > JNC_ILMAP))
    {
        //gtString msg(L"JNC file was not properly formed.\nDisassembly will not be available.");
        return false;
    }

    m_jncFilePath = jncFile;

    // Get the code section
    m_pCode = m_pExecutable->GetSectionBytes(JNC_CODE);

    gtVAddr imageBase = m_pExecutable->GetImageBase();

    if (symAvail)
    {
        gtUInt32 offsetLineCnt;

        // Got the data section when checking
        gtUByte* pTemp = (gtUByte*)m_pData;

        //Skip stuff (Not used now. Might need in the future)
        gtUInt32 strLength;
        // length for javaSrcname;
        memcpy(&strLength, pTemp, sizeof(unsigned int));
        pTemp += sizeof(unsigned int) + strLength;
        // length for class name
        memcpy(&strLength, pTemp , sizeof(gtUInt32));
        pTemp += sizeof(unsigned int) + strLength;
        //  lenght for jncFuncName
        memcpy(&strLength, pTemp, sizeof(gtUInt32));
        pTemp += sizeof(unsigned int) + strLength;
        // load address gtUInt64
        pTemp += sizeof(gtUInt64);
        memcpy(&offsetLineCnt, pTemp, sizeof(gtUInt32));
        pTemp += sizeof(gtUInt32);

        // read line number map
        // Note: the key of map is the offset, the data is line number
        gtUInt32 codeOffset;
        unsigned int startLineNum = 0xFFFFFFFF;
        unsigned int endLineNum = 0;

        for (unsigned int i = 0; i < offsetLineCnt; i++)
        {
            memcpy(&codeOffset, pTemp, sizeof(gtUInt32));

            // In the JVMTA agent with version earlier than (FILEVERSION 1,3,1,6 or PRODUCTVERSION 2,69.300,0)
            // JVMTI was writing RVA into .jnc file rather than offset, for soruce line mapping.
            // To keep consistency, starting from JVMTIA (version 1.3.1.6), we will write offset.
            // In future, we should remove the following two lines.
            // -Lei 04/05/2007
            if (static_cast<gtVAddr>(codeOffset) >= imageBase)
            {
                codeOffset = static_cast<gtUInt32>(static_cast<gtVAddr>(codeOffset) - imageBase);
            }

            pTemp += sizeof(gtUInt32);
            gtUInt32 lineNum;
            memcpy(&lineNum, pTemp, sizeof(gtUInt32));
            pTemp += sizeof(gtUInt32);
            m_lineMap[codeOffset] = lineNum;

            if (lineNum < startLineNum)
            {
                startLineNum = lineNum;
            }

            if (lineNum > endLineNum)
            {
                endLineNum = lineNum;
            }
        }
    }

    return true;
}

bool JavaJncReader::OpenWithInline(const wchar_t* jncFile)
{
    bool bRet = false;

    if (nullptr == jncFile)
    {
        return false;
    }

    if (nullptr == m_pExecutable)
    {
        m_pExecutable = new PeFile(jncFile);

        if (nullptr == m_pExecutable)
        {
            return false;
        }

        if (!m_pExecutable->Open())
        {
            delete m_pExecutable;
            m_pExecutable = nullptr;
            return false;
        }
    }

    unsigned int sectionNum = m_pExecutable->GetSectionsCount();
    bool symAvail = false;

    if (sectionNum > 1)
    {
        m_LoadAddr = m_pExecutable->GetCodeBase();

        //locate the Java symbol section
        m_pData = m_pExecutable->GetSectionBytes(m_pExecutable->LookupSectionIndex(".JVASRC"));

        if (nullptr == m_pData)
        {
            //Need to try to read data from older jnc files.
            m_pData = m_pExecutable->GetSectionBytes(m_pExecutable->LookupSectionIndex(".data"));
        }

        if (nullptr != m_pData)
        {
            for (unsigned int i = 1; i < sectionNum; i++)
            {
                if (m_pExecutable->GetSectionBytes(i) == m_pData)
                {
                    symAvail = true;
                    break;
                }
            }
        }

    }

    if ((sectionNum > JNC_ILMAP))
    {
        //gtString msg(L"JNC file was not properly formed.\nDisassembly will not be available.");
        return false;
    }

    m_jncFilePath = jncFile;

    // Get the code section
    m_pCode = m_pExecutable->GetSectionBytes(JNC_CODE);

    if (symAvail)
    {
        // Got the data section when checking
        gtUByte* pTemp = (gtUByte*)m_pData;

        // JIT load address
        m_LoadAddr = *(gtUInt64*)pTemp;
        pTemp += sizeof(m_LoadAddr);

        DWORD sectionSize = *(DWORD*)pTemp;
        pTemp += sizeof(DWORD);

        // read the sizes...
        DWORD s1 = *(DWORD*)pTemp;
        pTemp += sizeof(DWORD);

        // skip the section type - PC2BC
        DWORD type1 = *(DWORD*)pTemp;
        pTemp += sizeof(DWORD);
        m_pPc2bcBuf = (gtUByte*) pTemp;
        pTemp += s1 - sizeof(DWORD);// sizeof(DWORD) is already advanced

        DWORD s2 = *(DWORD*)pTemp;
        pTemp += sizeof(DWORD);

        // skip the section type - METHOD_TABLE
        DWORD type2 = *(DWORD*)pTemp;
        pTemp += sizeof(DWORD);
        m_pBc2srcBuf = (gtUByte*) pTemp;
        pTemp += s2 - sizeof(DWORD);// sizeof(DWORD) is already advanced

        DWORD s3 = *(DWORD*)pTemp;
        pTemp += sizeof(DWORD);

        // skip the section type - STRING_TABLE
        DWORD type3 = *(DWORD*)pTemp;
        pTemp += sizeof(DWORD);
        m_string_table_buf = (gtUByte*) pTemp;

        GT_UNREFERENCED_PARAMETER(sectionSize);
        GT_UNREFERENCED_PARAMETER(s3);
        GT_UNREFERENCED_PARAMETER(type1);
        GT_UNREFERENCED_PARAMETER(type2);
        GT_UNREFERENCED_PARAMETER(type3);

        bRet = _processInlineSections();
    }

    return bRet;
}


bool JavaJncReader::_processInlineSections()
{
    //if (nullptr == debugFP) {
    //    debugFP = fopen("c:\\Temp\\java-debug.txt", "w");
    //}

    // Process the stringtable
    if (nullptr == m_string_table_buf)
    {
        // JNC Files for Native methods won't have stringtable, bc2src and pc2bc sections..
        return true;
    }

    if (!_process_stringtable_section())
    {
        delete m_pExecutable;
        m_pExecutable = nullptr;
        return false;
    }

    // DumpStringTable(debugFP);

    // Process .bc2src
    if (nullptr == m_pBc2srcBuf)
    {
        delete m_pExecutable;
        m_pExecutable = nullptr;
        return false;
    }

    if (!_process_bc2src_section())
    {
        delete m_pExecutable;
        m_pExecutable = nullptr;
        return false;
    }

    //DumpJncMethodMap(debugFP);
    //DumpAddressRangeTable(debugFP);

    // Process the pc2bc
    if (nullptr == m_pPc2bcBuf)
    {
        // OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Warning: file %hs does not have pc-to-byte code information", fileName);
        delete m_pExecutable;
        m_pExecutable = nullptr;
        return false;
    }

    if (!_process_pc2bc_section())
    {
        delete m_pExecutable;
        m_pExecutable = nullptr;
        return false;
    }

    //DumpJncPcStackInfoMap(debugFP);

    // Setup inline information map
    if (!_process_inline_map())
    {
        delete m_pExecutable;
        m_pExecutable = nullptr;
        return false;
    }

    // DEBUG:  dump the java inline entries
    //DumpJavaInlineMap(debugFP);

    // Identify the main method for this JNC
    if (m_jncPcStackInfoMap.empty())
    {
        // We should be able to show the disassembly
        return true;
    }

    JncPcStackInfoMap::iterator it = m_jncPcStackInfoMap.begin();
    m_mainMethodId = it->second->methods[it->second->numstackframes - 1];

    //m_methodName   = m_jncMethodMap[m_mainMethodId].name;
    //m_srcFile      = m_jncMethodMap[m_mainMethodId].sourceName;

    //if (m_jncMethodMap[m_mainMethodId].lineNumberVec.size() > 0)
    //{
    //    m_startLine = m_jncMethodMap[m_mainMethodId].lineNumberVec[0].line_number;

    //    m_possibleEndLine =
    //        m_jncMethodMap[m_mainMethodId].lineNumberVec[m_jncMethodMap[m_mainMethodId].lineNumberVec.size() - 1].line_number;
    //}

    //m_mainInlineDepth = it->second->numstackframes;

    for (unsigned int i = 0; i < m_addressRangeTable.size(); i++)
    {
        if (m_addressRangeTable[i].id == m_mainMethodId)
        {
            m_addressRangeTable[i].pc_start = m_LoadAddr;
        }
    }

    return true;
}

void JavaJncReader::Close()
{
    if (nullptr != m_pExecutable)
    {
        m_pExecutable->Close();
        delete m_pExecutable;
        m_pExecutable = nullptr;
    }

    _freePcStackInfo();
    Clear();
}

void
JavaJncReader::_freePcStackInfo()
{
    if (nullptr != m_pcinfo)
    {
        free(m_pcinfo);
        m_pcinfo = nullptr;
    }
}

void
JavaJncReader::Clear()
{
    m_string_table_size = 0;
    m_string_table_buf  = nullptr;
    m_pPc2bcBuf         = nullptr;
    m_pBc2srcBuf        = nullptr;

    m_pExecutable = nullptr;

    m_lineMap.clear();
    m_LoadAddr = 0;

    m_SrcName.makeEmpty();
    m_FuncName.makeEmpty();
    m_jncFilePath.makeEmpty();

    m_pData = nullptr;
    m_pCode = nullptr;

    m_pcinfo = nullptr;
    m_jncPcStackInfoMap.clear();

    m_javaInlineMap.clear();
    m_jncMethodMap.clear();
    m_mainMethodId = 0;
    m_addressRangeTable.clear();
}

gtString JavaJncReader::GetSourceName()
{
    return  m_SrcName;
}
gtString JavaJncReader::GetFunctionName()
{
    return m_FuncName;
}

gtUInt64 JavaJncReader::GetLoadAddr()
{
    return m_LoadAddr;
}

OffsetLinenumMap JavaJncReader::GetOffsetLines()
{
    return m_lineMap;
}

const gtUByte* JavaJncReader::GetCodeBytesOfTextSection(unsigned int* pSectionSize)
{
    if (nullptr == m_pExecutable)
    {
        return nullptr;
    }

    const gtUByte* pCode = m_pExecutable->GetSectionBytes(JNC_CODE);

    if (nullptr != pSectionSize)
    {
        gtRVAddr sectionStartRva, sectionEndRva;
        m_pExecutable->GetSectionRvaLimits(JNC_CODE, sectionStartRva, sectionEndRva);
        *pSectionSize = sectionEndRva - sectionStartRva;
    }

    return pCode;
}

void
JavaJncReader::_fillLineNumSrcMap(
    JNCInlineMap&       ilmap,
    int                 numFrames,
    void*               vbcisArray,
    void*               vmethodArray,
    gtUInt64  thisPc,
    gtUInt64  nextPc
)
{
    jint* bcisArray = (jint*) vbcisArray;
    jmethodID* methodArray = (jmethodID*) vmethodArray;

    int xx;
    numFrames -= 2;

    for (xx = numFrames; xx >= 0; xx--)
    {
        LineNumSrc lns;
        addrRanges addrRange;

        bool ret = _getSrcInfoFromBcAndMethodID(bcisArray[xx],
                                                methodArray[xx],
                                                lns.lineNum,
                                                lns.sourceFile);

        if (false == ret)
        {
            continue;
        }

        addrRange.startAddr = thisPc;
        addrRange.stopAddr  = nextPc;
        // lns.startAddr       = thisPc;
        // lns.stopAddr        = nextPc;
        lns.methodId        = methodArray[xx];
        lns.symName         = GetMethodName(methodArray[xx]);

        if ((gtUInt64) - 1 == nextPc)
        {
            for (unsigned int j = 1; j < m_addressRangeTable.size(); j++)
            {
                if ((m_addressRangeTable[j].id == methodArray[xx])
                    && (thisPc >= (gtUInt64) m_addressRangeTable[j].pc_start)
                    && (thisPc <= (gtUInt64) m_addressRangeTable[j].pc_end))
                {
                    addrRange.startAddr = (gtUInt64) m_addressRangeTable[j].pc_start;
                    addrRange.stopAddr  = (gtUInt64) m_addressRangeTable[j].pc_end;
                    break;
                }
            }
        }

        lns.addrs.push_back(addrRange);
        ilmap.insert(JNCInlineMap::value_type(thisPc, lns));
    }
}


bool
JavaJncReader::_process_stringtable_section()
{
    jint size;

    // get the first four bytes - string buffer size
    size = *((jint*)m_string_table_buf);

    m_string_table_size = (jlong)(sizeof(char) * size);

    // update the string buffer
    m_string_table_buf += sizeof(jint);

    return true;
}


bool
JavaJncReader::GetStringFromOffset(jlong offset, std::string& str)
{
    char*  pChar = nullptr;

    if ((offset >= m_string_table_size)
        || (!m_string_table_buf))
    {
        return false;
    }

    pChar = (char*)m_string_table_buf + offset;
    str = pChar;
    return true;
}

const char*
JavaJncReader::GetMethodName(jmethodID methodId)
{
    JNCMethodMap::iterator it = m_jncMethodMap.find(methodId);

    if (it == m_jncMethodMap.end())
    {
        return "Unknown Method";
    }

    return it->second.name.c_str();
}

bool
JavaJncReader::_getSrcInfoFromBcAndMethodID(
    jint bc,
    jmethodID id,
    int& srcLine,
    std::string& srcFile
)
{
    bool ret = false;

    srcLine = -1;
    srcFile = "";

    // OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"bc=%d", bc);
    //  if (bc < 0) {
    //      return ret;
    //  }

    // Find method
    JNCMethodMap::iterator it = m_jncMethodMap.find(id);

    if (it == m_jncMethodMap.end())
    {
        return ret;
    }

    if ((0 > bc)
        && (m_jncMethodMap[m_mainMethodId].lineNumberVec.size() > 0)
        && (it->second.lineNumberVec.size() > 0))
    {
        srcLine = it->second.lineNumberVec[0].line_number;
        srcFile = it->second.sourceName;
        return true;
    }

    // Get Source line
    unsigned int num = it->second.lineNumberVec.size();

    for (int i = num - 1; i >= 0; i--)
    {
        srcLine = it->second.lineNumberVec[i].line_number;

        if (bc >= it->second.lineNumberVec[i].start_location)
        {
            break;
        }
    }

    // Get Source file
    if (srcLine != -1)
    {
        srcFile = it->second.sourceName;
    }

    ret = true;

    return ret;
}


bool JavaJncReader::_process_inline_map()
{
    std::string file;
    JncPcStackInfoMap::iterator it = m_jncPcStackInfoMap.begin(), itEnd = m_jncPcStackInfoMap.end();

    bool ret = true;

    for (; it != itEnd; ++it)
    {
        int numFrames = it->second->numstackframes;

        if (1 == numFrames)
        {
            continue;
        }

        gtUInt64 thisPc = (gtUInt64) it->second->pc;
        jint* bcisArray = it->second->bcis;
        jmethodID* methodArray = it->second->methods;
        JNCInlineMap ilmap;
        gtUInt64 nextPc = static_cast<gtUInt64>(-1);

        JncPcStackInfoMap::iterator nextit = it;
        ++nextit;

        if (nextit != itEnd)
        {
            nextPc = (gtUInt64) nextit->second->pc;
        }

        _fillLineNumSrcMap(ilmap,
                           numFrames,
                           bcisArray,
                           methodArray,
                           thisPc,
                           nextPc);

        int parentLine;
        std::string parentSource;
        int xx = numFrames - 1;

        _getSrcInfoFromBcAndMethodID(bcisArray[xx],
                                     methodArray[xx],
                                     parentLine,
                                     parentSource);

        JavaInlineMap::iterator jilmit = m_javaInlineMap.find(parentLine);
        std::pair<JavaInlineMap::iterator, JavaInlineMap::iterator> pairMapIt = m_javaInlineMap.equal_range(parentLine);
        JavaInlineMap::iterator itr = pairMapIt.first;

        while (itr != pairMapIt.second)
        {
            jilmit = itr;
            itr++;
        }

        if (jilmit == m_javaInlineMap.end())
        {
            // Parent line does not exist - add it
            m_javaInlineMap.insert(JavaInlineMap::value_type(parentLine, ilmap));
        }
        else
        {
            // Parent line exists - see if this stack already exists
            if (ilmap.size() == jilmit->second.size())
            {
                bool bMerged = false;

                while (!bMerged)
                {
                    JNCInlineMap::iterator newilmapit = ilmap.begin();
                    JNCInlineMap::iterator newmainit = jilmit->second.begin();

                    bool bIsEqual = true;

                    for (; (newilmapit != ilmap.end()) && (newmainit != jilmit->second.end()); ++newilmapit, ++newmainit)
                    {
                        if (!(newilmapit->second == newmainit->second))
                        {
                            bIsEqual = false;
                            break;
                        }
                    }

                    if (bIsEqual)
                    {
                        // Merge the address range of this stack with the existing stack
                        newilmapit = ilmap.begin();
                        newmainit = jilmit->second.begin();

                        for (; (newilmapit != ilmap.end()) && (newmainit != jilmit->second.end()); ++newilmapit, ++newmainit)
                        {
                            addrRanges ilAR = *(newilmapit->second.addrs.begin()); // will only be one of these
                            // multiple of these
                            bool bDidMerge = false;

                            AddressRangeList::iterator arIt = newmainit->second.addrs.begin(), arEnd = newmainit->second.addrs.end();

                            for (; arIt != arEnd; ++arIt)
                            {
                                if ((ilAR.startAddr >= (*arIt).startAddr) && (ilAR.startAddr <= (*arIt).stopAddr))
                                {
                                    (*arIt).startAddr = std::min((*arIt).startAddr, ilAR.startAddr);
                                    (*arIt).stopAddr = std::max((*arIt).stopAddr, ilAR.stopAddr);
                                    bDidMerge = true;

                                    // TODO: Need to check if adding this range should merge other ranges
                                    break;
                                }
                            }

                            if (!bDidMerge)
                            {
                                newmainit->second.addrs.push_back(ilAR);
                            }
                        }

                        bMerged = true;
                    }

                    if (bMerged)
                    {
                        break;
                    }

                    ++jilmit;

                    if ((jilmit == m_javaInlineMap.end()) || (jilmit->first != parentLine))
                    {
                        break;
                    }
                }

                if (!bMerged)
                {
                    // TODO: What about this case?
                    // printf("After while - no merge done!!\n"); fflush(stdout);
                }
            }
            else
            {
                m_javaInlineMap.insert(JavaInlineMap::value_type(parentLine, ilmap));
            } // if (ilmap.size() == jilmit->second.size())
        } // if (jilmit == m_javaInlineMap.end())
    } // for

    return ret;
}

bool
JavaJncReader::_process_pc2bc_section()
{
    // jint  info_size;
    jint  numpcs;

    gtUByte* tmpBuf = m_pPc2bcBuf;

    // info_size = *((jint *) tmpBuf);

    tmpBuf += sizeof(jint);
    numpcs =  *((jint*) tmpBuf);

    tmpBuf += sizeof(jint);

    m_pcinfo = malloc(sizeof(JncPcStackInfo) * numpcs);

    if (nullptr == m_pcinfo)
    {
        return false;
    }

    for (int j = 0; j < numpcs; j++)
    {
        jint        num_stack_frames;
        gtUInt64    pc;
        jmethodID*  method_id_array;
        jint*       bc_index_array;

        pc = *((gtUInt64*)tmpBuf);
        tmpBuf += sizeof(gtUInt64);

        num_stack_frames = *((jint*)tmpBuf);
        tmpBuf += sizeof(jint);

        // Set up array of methodids
        method_id_array = (jmethodID*)tmpBuf;
        tmpBuf += sizeof(jmethodID) * num_stack_frames;

        // Set up array of bytecode indices
        bc_index_array = (jint*)tmpBuf;
        tmpBuf += sizeof(jint) * num_stack_frames;

        ((JncPcStackInfo*)m_pcinfo)[j].pc             = pc;
        ((JncPcStackInfo*)m_pcinfo)[j].numstackframes = num_stack_frames;
        ((JncPcStackInfo*)m_pcinfo)[j].methods        = method_id_array;
        ((JncPcStackInfo*)m_pcinfo)[j].bcis           = bc_index_array;

        m_jncPcStackInfoMap[(unsigned long)pc] =
            &(((JncPcStackInfo*)m_pcinfo)[j]);
    }

    return true;
}

bool
JavaJncReader::_process_bc2src_section()
{
    bool ret = false;

    // Recreate and output bc2src section
    void* line_number_table_buf = nullptr;
    void* method_table_buf = nullptr;

    jint method_table_size;
    jint line_number_table_size;
    jint method_table_with_header_size;
    int offset;

    gtUByte* tmpBuf = m_pBc2srcBuf;

    method_table_size = *((jint*)tmpBuf);
    tmpBuf += sizeof(jint);

    // Only for debug purpose
    line_number_table_size = *((jint*)tmpBuf);
    // fprintf(stdout,
    //        "method_table_size %d line_number_table_size %d\n",
    //        method_table_size, line_number_table_size);

    method_table_with_header_size = 2 * sizeof(jint) + method_table_size;

    // Get Method table stuff
    method_table_buf = (void*)m_pBc2srcBuf;

    // Get Line number stuff
    tmpBuf = m_pBc2srcBuf;
    tmpBuf += method_table_with_header_size;
    line_number_table_buf = (void*)tmpBuf;

    // Populate information in JNCMethodMap
    offset = 2 * sizeof(jint);

    while (offset < method_table_with_header_size)
    {
        AddressRange range;
        memcpy(&range, (char*)method_table_buf + offset, sizeof(AddressRange));

        if (m_jncMethodMap.find(range.id) == m_jncMethodMap.end())
        {
            JncMethod jncMethod;
            jncMethod.id = range.id;

            GetStringFromOffset(range.method_name_offset,
                                jncMethod.name);

            GetStringFromOffset(range.method_signature_offset,
                                jncMethod.signature);

            GetStringFromOffset(range.source_name_offset,
                                jncMethod.sourceName);

            // fprintf(stderr,
            //        "DEBUG: offset = %u\n", range.line_number_table_offset);

            unsigned int numLineNumber =
                *((jint*)((char*)line_number_table_buf +
                          range.line_number_table_offset -
                          method_table_with_header_size));

            // fprintf(stderr,
            //        "DEBUG: numLineNumber = %u\n", numLineNumber);

            JncJvmtiLineNumberEntry* pEnt =
                (JncJvmtiLineNumberEntry*)((char*)line_number_table_buf +
                                           range.line_number_table_offset +
                                           sizeof(jint) -
                                           method_table_with_header_size);

            // for (unsigned int i = 0 ; i < numLineNumber; i++, pEnt++)
            for (unsigned int i = 0 ; i < numLineNumber; i++, pEnt++)
            {
                jncMethod.lineNumberVec.push_back(*pEnt);
                // fprintf(stderr, "pEnt=%p, start_loc=%d, line=%l\n",
                //        pEnt, pEnt->start_location, pEnt->line_number);
            }

            gtSort(jncMethod.lineNumberVec.begin(), jncMethod.lineNumberVec.end());

            m_jncMethodMap[range.id] = jncMethod;
        }

        JncAddressRange jncRng;
        jncRng.id       = range.id;
        jncRng.pc_start = range.pc_start;
        jncRng.pc_end   = range.pc_end;

        m_addressRangeTable.push_back(jncRng);
        offset += sizeof(AddressRange);
    }

    gtSort(m_addressRangeTable.begin(), m_addressRangeTable.end());
    ret = true;

    return ret;
}

void
JavaJncReader::DumpStringTable(FILE* f)
{
    if (nullptr == f)
    {
        return;
    }

    std::string str;
    unsigned int offset = 0;

    fprintf(f, "DumpStringTable - (size:%llu) \n", m_string_table_size);

    while (offset < m_string_table_size)
    {
        GetStringFromOffset(offset, str);
        fprintf(f, " %4u:%s\n", offset, str.c_str());
        offset += str.length() + 1;
    }

    fprintf(f, "DumpStringTable: END\n");
}

OffsetLinenumMap
JavaJncReader::GetOffsetLines(std::wstring funcName)
{
    int line = -1;
    std::string file;

    if (! m_lineMap.empty())
    {
        return m_lineMap;
    }

    //fprintf(debugFP, "============================\n");
    //DumpJncPcStackInfoMap(debugFP);

    //if (gJncReaderVerbose)
    //{
    //fprintf (stderr, "CJNCReader::GetOffsetLines  (size:%lu) n",
    //           m_jncPcStackInfoMap.size());
    //}

    JncPcStackInfoMap::iterator it, itEnd;

    for (it = m_jncPcStackInfoMap.begin(), itEnd = m_jncPcStackInfoMap.end(); it != itEnd; ++it)
    {
        gtUInt64  pc = it->first;

        if (nullptr != debugFP)
        {
            fprintf(debugFP,
                    "loadaddr: 0x%llx, pc = 0x%llx, stack size = %u\n",
                    m_LoadAddr, pc, it->second->numstackframes);
        }

        int i = 0;

        for (i = 0 ; i < it->second->numstackframes; i++)
        {
            if (m_jncMethodMap[it->second->methods[i]].name.empty())
            {
                if (debugFP != nullptr)
                {
                    fprintf(debugFP, "Empty methods name..\n");
                }
                continue;
            }

            std::wstring tmpStr(m_jncMethodMap[it->second->methods[i]].name.begin(),
                                m_jncMethodMap[it->second->methods[i]].name.end());

            // parse method signature
            char parsedMethodSig[OS_MAX_PATH] = { 0 };
            parseMethodSignature(nullptr, m_jncMethodMap[it->second->methods[i]].signature.data(), parsedMethodSig);
            // append parsed signature to tmpStr
            tmpStr.append(parsedMethodSig, parsedMethodSig + strlen(parsedMethodSig));

            if ((!funcName.empty()) && wcscmp(funcName.c_str(), tmpStr.c_str()))
            {
                continue;
            }

            bool rv = _getSrcInfoFromBcAndMethodID(it->second->bcis[i],
                                                   it->second->methods[i],
                                                   line,
                                                   file);

            if (false == rv)
            {
                continue;
            }

            // Add it to offset line table
            unsigned int codeOffset = (unsigned int)(pc - m_LoadAddr);
            m_lineMap[codeOffset] = line;

            // if (gJncReaderVerbose)
            if (nullptr != debugFP)
            {
                fprintf(debugFP, "    Frame:%2u, bcis:%4d, method:(%lld) %s (%d,%s)\n",
                        i, it->second->bcis[i],
                        it->second->methods[i],
                        m_jncMethodMap[it->second->methods[i]].name.c_str(),
                        line, file.c_str());
            }
        }
    }

    return m_lineMap;
} // GetOffsetLines(wstring funcName)

void
JavaJncReader::DumpJncMethodMap(FILE* f)
{
    if (nullptr == f)
    {
        return;
    }

    fprintf(f, "DumpJncMethodMap - (size:%lu)\n", m_jncMethodMap.size());

    for (JNCMethodMap::iterator it = m_jncMethodMap.begin(), itEnd = m_jncMethodMap.end(); it != itEnd; ++it)
    {
        fprintf(f,
                "id:%4llu, name:%s, signature:%s, sourceName:%s, numLineNumberEntry:%u\n",
                it->second.id,
                it->second.name.c_str(),
                it->second.signature.c_str(),
                it->second.sourceName.c_str(),
                it->second.lineNumberVec.size());

        for (unsigned int i = 0; i < it->second.lineNumberVec.size(); i++)
        {
            // JncJvmtiLineNumberEntry le = (*it).second.lineNumberVec[i];

            fprintf(f, "    loc:%lld, line:%d\n",
                    it->second.lineNumberVec[i].start_location,
                    it->second.lineNumberVec[i].line_number);
        }
    }

    fprintf(f, "DumpJncMethodMap: END\n");

    return;
}

void
JavaJncReader::DumpAddressRangeTable(FILE* f)
{
    if (nullptr == f)
    {
        return;
    }

    fprintf(f, "DumpAddressRangeTable - (size:%u)\n",
            m_addressRangeTable.size());

    for (unsigned int i = 0; i < m_addressRangeTable.size(); i++)
    {
        fprintf(f, "id:%4llu, pc_start:0x%llx, pc_end:0x%llx, name:%s\n",
                m_addressRangeTable[i].id,
                m_addressRangeTable[i].pc_start,
                m_addressRangeTable[i].pc_end,
                GetMethodName(m_addressRangeTable[i].id));
    }

    fprintf(f, "DumpAddressRangeTable: END\n");

    return;
}

void
JavaJncReader::DumpJncPcStackInfoMap(FILE* f)
{
    if (nullptr == f)
    {
        return;
    }

    int line = -1;
    std::string file;

    fprintf(f, "DumpJncPcStackInfoMap - (size:%u)\n",
            m_jncPcStackInfoMap.size());

    JncPcStackInfoMap::iterator it, itEnd;

    for (it = m_jncPcStackInfoMap.begin(), itEnd = m_jncPcStackInfoMap.end(); it != itEnd; ++it)
    {
        fprintf(f, "pc = 0x%llx, stack size = %u\n",
                it->first, it->second->numstackframes);

        for (int i = 0 ; i < it->second->numstackframes; i++)
        {
            bool ret = _getSrcInfoFromBcAndMethodID(it->second->bcis[i],
                                                    it->second->methods[i],
                                                    line,
                                                    file);

            if (false == ret)
            {
                continue;
            }

            fprintf(f, "    FRAME:%2u, bcis:%4d, method:(%lld) %s (%d,%s)\n",
                    i, it->second->bcis[i],
                    it->second->methods[i],
                    m_jncMethodMap[it->second->methods[i]].name.c_str(),
                    line, file.c_str());
        }
    }

    fprintf(f, "DumpJncPcStackInfoMap - END\n");

    return;
}

