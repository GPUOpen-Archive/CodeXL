//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file JavaJncReader_Lin.cpp
/// \brief The JavaJncReader class to read the JNC files created by the
///        JVMTI profile agent created on Linux.
///
//==================================================================================

#include <stdio.h>
#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <JvmsParser.h>
#include "JavaJncReader_Lin.h"

static int gJncReaderVerbose = 0;

JavaJncReader::JavaJncReader()
{
    Clear();
}


JavaJncReader::~JavaJncReader()
{
    Close();
}


void
JavaJncReader::Clear()
{
    m_pExecutable = nullptr;
    m_sectionCounts     = 0;
    m_string_table_size = 0;

    m_string_table_buf  = nullptr;
    m_pCodeBuf          = nullptr;
    m_pPc2bcBuf         = nullptr;
    m_pBc2srcBuf        = nullptr;

    m_loadAddr          = 0;
    m_startLine         = 0;
    m_possibleEndLine   = 0;
    m_textOffset        = 0;
    m_textSize          = 0;
    m_mainInlineDepth   = 1;

    m_jncMethodMap.clear();
    m_addressRangeTable.clear();
    m_pcinfo = nullptr;
    m_jncPcStackInfoMap.clear();
    m_javaInlineMap.clear();

    m_lineMap.clear();
}


void
JavaJncReader::Close()
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
    if (!m_pcinfo || m_jncPcStackInfoMap.size() == 0)
    {
        return;
    }

    free(m_pcinfo);
    m_pcinfo = nullptr;
}


bool JavaJncReader::Open(const wchar_t* pWFileName)
{
    JncPcStackInfoMap::iterator it;
    char fileName[261];

    memset(fileName, 0, sizeof(fileName));

    if (nullptr == pWFileName)
    {
        return false;
    }

    wcstombs(fileName, pWFileName, 260);

    ExecutableFile* pExecutable = ExecutableFile::Open(pWFileName);

    if (nullptr == pExecutable)
    {
        return false;
    }

    m_sectionCounts =  pExecutable->GetSectionsCount();

    // Get The .text section
    unsigned codeSecIndex = pExecutable->LookupSectionIndex(".text");
    gtRVAddr codeStartRva = 0, codeEndRva = 0;
    pExecutable->GetSectionRvaLimits(codeSecIndex, codeStartRva, codeEndRva);

    m_pCodeBuf   = pExecutable->GetSectionBytes(codeSecIndex);
    m_loadAddr   = static_cast<gtRVAddr>(codeStartRva) + pExecutable->GetImageBase();
    m_textOffset = codeStartRva;
    m_textSize   = codeEndRva - codeStartRva;

    if (nullptr == m_pCodeBuf || m_textSize <= 0)
    {
        delete pExecutable;
        return false;
    }

    // Get the .stringtable
    m_string_table_buf = pExecutable->GetSectionBytes(pExecutable->LookupSectionIndex(".stringtable"));

    if (nullptr == m_string_table_buf)
    {
        // JNC Files for Native methods won't have stringtable, bc2src and pc2bc sections..
        m_pExecutable = pExecutable;
        return true;
    }

    if (!_process_stringtable_section())
    {
        delete pExecutable;
        m_pExecutable = nullptr;
        return false;
    }

    // Process .bc2src
    m_pBc2srcBuf = pExecutable->GetSectionBytes(pExecutable->LookupSectionIndex(".bc2src"));

    if (nullptr == m_pBc2srcBuf)
    {
        delete pExecutable;
        m_pExecutable = nullptr;
        return false;
    }

    if (!_process_bc2src_section())
    {
        delete pExecutable;
        m_pExecutable = nullptr;
        return false;
    }

    // Process the .pc2bc
    m_pPc2bcBuf = pExecutable->GetSectionBytes(pExecutable->LookupSectionIndex(".pc2bc"));

    if (nullptr == m_pPc2bcBuf)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Warning: file %hs does not have pc-to-byte code information", fileName);
        delete pExecutable;
        m_pExecutable = nullptr;
        return false;
    }

    if (!_process_pc2bc_section())
    {
        delete pExecutable;
        m_pExecutable = nullptr;
        return false;
    }

    // Setup inline information map
    if (!_process_inline_map())
    {
        delete pExecutable;
        m_pExecutable = nullptr;
        return false;
    }

    // DEBUG:  dump the java inline entries
    // DumpJavaInlineMap();

    // Identify the main method for this JNC
    if (m_jncPcStackInfoMap.empty())
    {
        // If there is no PC stack info, we should still be able to
        //  provide disassembly details. Hence return true;
        return true;
    }

    it = m_jncPcStackInfoMap.begin();
    m_mainMethodId = it->second->methods[it->second->numstackframes - 1];
    m_methodName   = m_jncMethodMap[m_mainMethodId].name;
    m_srcFile      = m_jncMethodMap[m_mainMethodId].sourceName;

    if (m_jncMethodMap[m_mainMethodId].lineNumberVec.size() > 0)
    {
        m_startLine = m_jncMethodMap[m_mainMethodId].lineNumberVec[0].line_number;

        m_possibleEndLine =
            m_jncMethodMap[m_mainMethodId].lineNumberVec[m_jncMethodMap[m_mainMethodId].lineNumberVec.size() - 1].line_number;
    }

    m_mainInlineDepth = it->second->numstackframes;

    for (unsigned int i = 0; i < m_addressRangeTable.size(); i++)
    {
        if (m_addressRangeTable[i].id == m_mainMethodId)
        {
            m_addressRangeTable[i].pc_start = m_loadAddr;
        }
    }

    m_pExecutable = pExecutable;
    return true;
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
        gtUInt64 nextPc = -1;

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


void JavaJncReader::DumpJavaInlineMap()
{
    JavaInlineMap::iterator mit = m_javaInlineMap.begin(), mitEnd = m_javaInlineMap.end();

    fwprintf(stderr, L"Dumping java inline map\n");

    for (; mit != mitEnd; ++mit)
    {
        fwprintf(stderr, L"  Parent line = %d\n", mit->first);
        JNCInlineMap::iterator ilmit = mit->second.begin(), ilmitEnd = mit->second.end();

        for (; ilmit != ilmitEnd; ++ilmit)
        {
            fwprintf(stderr, L"     methodId = 0x%lx (key=0x%lx)\n", ilmit->second.methodId, ilmit->first);
            fwprintf(stderr, L"      lineNum = %d\n", ilmit->second.lineNum);
            fwprintf(stderr, L"   sourceFile = %s\n", ilmit->second.sourceFile.c_str());
            fwprintf(stderr, L"      symName = %s\n", ilmit->second.symName.c_str());

            AddressRangeList::iterator listit = ilmit->second.addrs.begin(), listitEnd = ilmit->second.addrs.end();

            for (; listit != listitEnd; ++listit)
            {
                fwprintf(stderr, L"        address range 0x%lx - 0x%lx\n", (*listit).startAddr, (*listit).stopAddr);
            }
        }
    }

    fwprintf(stderr, L"Dumping java inline map (END)\n"); fflush(stdout);
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

        _getSrcInfoFromBcAndMethodID(bcisArray[xx],
                                     methodArray[xx],
                                     lns.lineNum,
                                     lns.sourceFile);

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

    m_string_table_size = sizeof(char) * size;

    // update the string buffer
    m_string_table_buf += sizeof(jint);

    return true;
}


bool
JavaJncReader::GetStringFromOffset(unsigned int offset, std::string& str)
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


void
JavaJncReader::DumpStringTable(FILE* f)
{
    std::string str;
    unsigned int offset = 0;

    fprintf(f, "DumpStringTable - (size:%u) \n", m_string_table_size);

    while (offset < m_string_table_size)
    {
        GetStringFromOffset(offset, str);
        fprintf(f, " %4u:%s\n", offset, str.c_str());
        offset += str.length() + 1;
    }

    fprintf(f, "DumpStringTable: END\n");
}


bool
JavaJncReader::_process_bc2src_section()
{
    bool ret = false;

    // Recreate and output bc2src section
    const gtUByte* line_number_table_buf = nullptr;
    const gtUByte* method_table_buf = nullptr;

    jint method_table_size;
    // jint line_number_table_size;
    jint method_table_with_header_size;
    int offset;

    const gtUByte* tmpBuf = m_pBc2srcBuf;

    method_table_size = *reinterpret_cast<const jint*>(tmpBuf);
    tmpBuf += sizeof(jint);
    // line_number_table_size = *((jint *)tmpBuf);

    // fprintf(stdout,
    //        "method_table_size %d line_number_table_size %d\n",
    //         method_table_size, line_number_table_size);

    method_table_with_header_size = 2 * sizeof(jint) + method_table_size;

    // Get Method table stuff
    method_table_buf = m_pBc2srcBuf;

    // Get Line number stuff
    tmpBuf = m_pBc2srcBuf;
    tmpBuf += method_table_with_header_size;
    line_number_table_buf = tmpBuf;

    // Populate information in JNCMethodMap
    offset = 2 * sizeof(jint);

    while (offset < method_table_with_header_size)
    {
        AddressRange range;
        memcpy(&range, method_table_buf + offset, sizeof(AddressRange));

        if (m_jncMethodMap.find(range.id) == m_jncMethodMap.end())
        {
            JncMethod jncMethod;
            jncMethod.id = range.id;

            GetStringFromOffset(range.method_name_offset, jncMethod.name);
            GetStringFromOffset(range.method_signature_offset, jncMethod.signature);
            GetStringFromOffset(range.source_name_offset, jncMethod.sourceName);

            //fprintf(stderr,
            //        "DEBUG: offset = %u\n", range.line_number_table_offset);

            unsigned int numLineNumber = *reinterpret_cast<const jint*>(line_number_table_buf +
                                                                        range.line_number_table_offset -
                                                                        method_table_with_header_size);

            // fprintf(stderr,
            //        "DEBUG: numLineNumber = %u\n", numLineNumber);

            const JncJvmtiLineNumberEntry* pEnt = reinterpret_cast<const JncJvmtiLineNumberEntry*>(line_number_table_buf +
                                                  range.line_number_table_offset +
                                                  sizeof(jint) -
                                                  method_table_with_header_size);

            for (unsigned int i = 0 ; i < numLineNumber; i++, pEnt++)
            {
                jncMethod.lineNumberVec.push_back(*pEnt);
                // fprintf(stderr, "pEnt=%p, start_loc=%d, line=%d\n",
                //         pEnt, pEnt->start_location, pEnt->line_number);
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


const char*
JavaJncReader::GetMethodName(jmethodID methodId)
{
    JNCMethodMap::iterator it = m_jncMethodMap.find((jmethodID)methodId);

    if (it == m_jncMethodMap.end())
    {
        return "Unknown Method";
    }

    return it->second.name.c_str();
}


void
JavaJncReader::DumpJncMethodMap(FILE* f)
{
    (void)(f); // unused
#if 0
    fprintf(f, "DumpJncMethodMap - (size:%lu)\n", m_jncMethodMap.size());

    for (JNCMethodMap::iterator it = m_jncMethodMap.begin(), itEnd = m_jncMethodMap.end(); it != itEnd; ++it)
    {
        fprintf(f,
                "id:%4lu, name:%s, signature:%s, sourceName:%s, numLineNumberEntry:%u\n",
                it->second.id,
                it->second.name.c_str(),
                it->second.signature.c_str(),
                it->second.sourceName.c_str(),
                it->second.lineNumberVec.size());

        for (unsigned int i = 0; i < it->second.lineNumberVec.size(); i++)
        {
            fprintf(f, "    loc:%4u, line:%4u\n",
                    it->second.lineNumberVec[i].start_location,
                    it->second.lineNumberVec[i].line_number);
        }
    }

    fprintf(f, "DumpJncMethodMap: END\n");
#endif //0

    return;
}


// GetOffsetLines()
//
OffsetLinenumMap
JavaJncReader::GetOffsetLines()
{
    int line = -1;
    std::string file;

    if (! m_lineMap.empty())
    {
        return m_lineMap;
    }

    // fprintf (stderr, "CJNCReader::GetOffsetLines  (size:%u) n",
    //         m_jncPcStackInfoMap.size());

    for (JncPcStackInfoMap::iterator it = m_jncPcStackInfoMap.begin(), itEnd = m_jncPcStackInfoMap.end(); it != itEnd; ++it)
    {
        gtUInt64  pc = it->first;

        if (gJncReaderVerbose)
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"loadaddr: 0x%llx, size:%ld, pc = 0x%llx, stack size = %u",
                                       m_loadAddr, m_textSize, it->first, it->second->numstackframes);
        }

        int i = 0;

        for (i = 0 ; i < it->second->numstackframes; i++)
        {
            _getSrcInfoFromBcAndMethodID(it->second->bcis[i],
                                         it->second->methods[i],
                                         line,
                                         file);

            // Add it to offset line table
            unsigned int codeOffset = pc - m_loadAddr;
            m_lineMap[codeOffset] = line;

            // printf ("    Frame:%2u, bcis:%4d, method:(%d) %s (%d,%s)\n",
            //          i, it->second->bcis[i],
            //          it->second->methods[i],
            //          m_jncMethodMap[it->second->methods[i]].name.c_str(),
            //          line, file.c_str());
        }
    }

    // printf ("CJNcReader::GetOffsetLines: END \n");

    return m_lineMap;
} // GetOffsetLines


// GetOffsetLines(wstring funcName)
//
OffsetLinenumMap
JavaJncReader::GetOffsetLines(std::wstring funcName)
{
    int line = -1;
    std::string file;

    if (! m_lineMap.empty())
    {
        return m_lineMap;
    }

    if (gJncReaderVerbose)
    {
        fprintf(stderr, "CJNCReader::GetOffsetLines  (size:%lu) n",
                m_jncPcStackInfoMap.size());
    }

    JncPcStackInfoMap::iterator it = m_jncPcStackInfoMap.begin();
    JncPcStackInfoMap::iterator itEnd = m_jncPcStackInfoMap.end();

    for (; it != itEnd; it++)
    {
        uint64_t  pc = it->first;

        if (gJncReaderVerbose)
        {
            fprintf(stderr,
                    "loadaddr: 0x%lx, size:%ld, pc = 0x%lx, stack size = %u\n",
                    m_loadAddr, m_textSize,
                    it->first, it->second->numstackframes);
        }

        int i = 0;

        for (i = 0 ; i < it->second->numstackframes; i++)
        {
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

            _getSrcInfoFromBcAndMethodID(it->second->bcis[i],
                                         it->second->methods[i],
                                         line,
                                         file);

            // Add it to offset line table
            unsigned int codeOffset = pc - m_loadAddr;
            m_lineMap[codeOffset] = line;

            if (gJncReaderVerbose)
            {
                fprintf(stderr, "    Frame:%2u, bcis:%4d, method:(%lx) %s (%d,%s)\n",
                        i, it->second->bcis[i],
                        (uint64_t)it->second->methods[i],
                        m_jncMethodMap[it->second->methods[i]].name.c_str(),
                        line, file.c_str());
            }
        }
    }

    return m_lineMap;
} // GetOffsetLines(wstring funcName)


void
JavaJncReader::DumpAddressRangeTable(FILE* f)
{
    (void)(f); // unused
#if 0
    fprintf(f, "DumpAddressRangeTable - (size:%u)\n",
            m_addressRangeTable.size());

    for (unsigned int i = 0; i < m_addressRangeTable.size(); i++)
    {
        fprintf(f, "id:%4lu, pc_start:0x%lx, pc_end:0x%lx, name:%s\n",
                m_addressRangeTable[i].id,
                m_addressRangeTable[i].pc_start,
                m_addressRangeTable[i].pc_end,
                GetMethodName((jmethodID)m_addressRangeTable[i].id));
    }

    fprintf(f, "DumpAddressRangeTable: END\n");
#endif //0

    return;
}


bool
JavaJncReader::_process_pc2bc_section()
{
    // jint  info_size;
    jint  numpcs;

    gtUByte* tmpBuf = const_cast<gtUByte*>(m_pPc2bcBuf);

    // info_size = *((jint *) tmpBuf);

    tmpBuf += sizeof(jint);
    numpcs = *reinterpret_cast<const jint*>(tmpBuf);

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

        pc = *reinterpret_cast<gtUInt64*>(tmpBuf);
        tmpBuf += sizeof(void*);

        num_stack_frames = *reinterpret_cast<jint*>(tmpBuf);
        tmpBuf += sizeof(jint);

        // Set up array of methodids and bytecode indices
        method_id_array = reinterpret_cast<jmethodID*>(tmpBuf);
        tmpBuf += sizeof(jmethodID) * num_stack_frames;

        // Set up array of methodids and bytecode indices
        bc_index_array = reinterpret_cast<jint*>(tmpBuf);
        tmpBuf += sizeof(jint) * num_stack_frames;

        ((JncPcStackInfo*)m_pcinfo)[j].pc             = pc;
        ((JncPcStackInfo*)m_pcinfo)[j].numstackframes = num_stack_frames;
        ((JncPcStackInfo*)m_pcinfo)[j].methods        = method_id_array;
        ((JncPcStackInfo*)m_pcinfo)[j].bcis           = bc_index_array;

        m_jncPcStackInfoMap[static_cast<size_t>(pc)] = &(((JncPcStackInfo*)m_pcinfo)[j]);
    }

    return true;
}


void
JavaJncReader::DumpJncPcStackInfoMap(FILE* f)
{
    (void)(f); // unused
#if 0
    int line = -1;
    std::string file;

    fprintf(f, "DumpJncPcStackInfoMap - (size:%u) n",
            m_jncPcStackInfoMap.size());

    for (JncPcStackInfoMap::iterator it = m_jncPcStackInfoMap.begin(), itEnd = m_jncPcStackInfoMap.end(); it != itEnd; ++it)
    {
        fprintf(f, "pc = 0x%llx, stack size = %u\n",
                it->first, it->second->numstackframes);

        for (int i = 0 ; i < it->second->numstackframes; i++)
        {
            _getSrcInfoFromBcAndMethodID(it->second->bcis[i],
                                         it->second->methods[i],
                                         line,
                                         file);

            fprintf(f, "    Frame:%2u, bcis:%4d, method:(%d) %s (%d,%s)\n",
                    i, it->second->bcis[i],
                    it->second->methods[i],
                    m_jncMethodMap[it->second->methods[i]].name.c_str(),
                    line, file.c_str());
        }
    }

    fprintf(f, "DumpJncPcStackInfoMap - END\n");
#endif // 0

    return;
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


jmethodID
JavaJncReader::_getMethodFromAddr(gtUInt64 addr)
{
    jmethodID id = (jmethodID) - 1;

    for (unsigned int i = 0; i < m_addressRangeTable.size(); i++)
    {
        if ((addr >= m_addressRangeTable[i].pc_start)
            && (addr <= m_addressRangeTable[i].pc_end))
        {
            id = m_addressRangeTable[i].id;
            break;
        }
    }

    return id;
}


int
JavaJncReader::_getLineFromAddrVec(
    gtUInt64           addr,
    JncJvmtiLineNumberEntryVec&  vec
)
{
    int ret = 0;

    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_EXTENSIVE, L"Looking for addr %lld", addr);

    for (unsigned int i = 0; i < vec.size(); i++)
    {
        if ((jlocation)addr >= vec[i].start_location)
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_EXTENSIVE, L" Found: addr=0x%llx, start_loc=0x%llx, line=%d\n",
                                       addr, (gtUInt64) vec[i].start_location, vec[i].line_number);

            ret = vec[i].line_number;
        }

        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_EXTENSIVE, L"    loc:%4ld, line:%4u\n", vec[i].start_location, vec[i].line_number);
    }

    return ret;
}


bool
JavaJncReader::GetSrcInfoFromAddressWithFile(
    gtUInt64            addr,
    int&                srcLine,
    std::string&        srcFile
)
{
    // Check if this address belongs to the code section of this module
    if ((addr < m_loadAddr)
        || (addr >= (m_loadAddr + m_textSize)))
    {
        return false;
    }

    if (m_jncMethodMap.size() == 0)
    {
        return false;
    }

    // If the address is the beginning of the function, just return the info
    // NOTE: This is often the case, so this block helps speed things up.
    if (addr == m_loadAddr)
    {
        // Get last item in the MethodMap
        srcFile = m_srcFile;
        srcLine = m_startLine;
        return true;
    }

    // Search the PCStackInfoMap for the appropriate data
    bool bFound = false;
    JncPcStackInfoMap::reverse_iterator rit = m_jncPcStackInfoMap.rbegin(), rend = m_jncPcStackInfoMap.rend();

    for (; rit != rend; ++rit)
    {
        if (addr >= rit->first)
        {
            bFound = true;
            break;
        }
    }

    // If not found, we assume that this address belongs
    // to the first line of the function
    if (!bFound)
    {
        srcFile = m_srcFile;
        srcLine = m_startLine;
    }
    else
    {
        int level = rit->second->numstackframes - 1;
        bool foundit = false;

        for (level = 0; level < rit->second->numstackframes; level++)
        {
            std::string file;
            _getSrcInfoFromBcAndMethodID(rit->second->bcis[level],
                                         rit->second->methods[level],
                                         srcLine,
                                         file);

            if (srcFile == file)
            {
                foundit = true;
                break;
            }
        }

        if (!foundit)
        {
            return false;
        }
    }

    return true;
}


bool
JavaJncReader::GetSrcInfoFromAddress(
    gtUInt64            addr,
    int&                srcLine,
    std::string&        srcFile,
    int&                inlineDepth
)
{
    // Check if this address belongs to the code section of this module
    if ((addr < m_loadAddr)
        || (addr >= (m_loadAddr + m_textSize)))
    {
        return false;
    }

    if (m_jncMethodMap.size() == 0)
    {
        return false;
    }

    // If the address is the beginning of the function, just return the info
    // NOTE: This is often the case, so this block helps speed things up.
    if (addr == m_loadAddr)
    {
        // Get last item in the MethodMap
        srcFile     = m_srcFile;
        srcLine     = m_startLine;
        inlineDepth = m_mainInlineDepth;

        return true;
    }

    // Search the PCStackInfoMap for the appropriate data
    bool bFound = false;
    JncPcStackInfoMap::reverse_iterator rit = m_jncPcStackInfoMap.rbegin(), rend = m_jncPcStackInfoMap.rend();

    for (; rit !=  rend; ++rit)
    {
        if (addr >= rit->first)
        {
            bFound = true;
            break;
        }
    }

    // If not found, we assume that this address belongs
    // to the first line of the function
    if (!bFound)
    {
        srcFile     = m_srcFile;
        srcLine     = m_startLine;
        inlineDepth = m_mainInlineDepth;
    }
    else
    {
        int level = rit->second->numstackframes - 1;

        if (level > 0)
        {
            inlineDepth = level + 1;
        }

        _getSrcInfoFromBcAndMethodID(rit->second->bcis[level],
                                     rit->second->methods[level],
                                     srcLine,
                                     srcFile);
    }

    return true;
}


bool
JavaJncReader::GetSymbolAndRangeFromAddr(
    gtUInt64     addr,
    std::string& symName,
    gtUInt64&    startAddr,
    gtUInt64&    endAddr
)
{
    bool bRet = false;

    // NOTE: This is 1 because the first item in the table is the main
    //       method for the JNC
    //
    for (unsigned int i = 1; i < m_addressRangeTable.size(); i++)
    {
        // Check if address is within the range
        if ((addr >= m_addressRangeTable[i].pc_start)
            && (addr <= m_addressRangeTable[i].pc_end))
        {
            symName   = GetMethodName((jmethodID)m_addressRangeTable[i].id);
            startAddr = (gtUInt64) m_addressRangeTable[i].pc_start,
            endAddr   = (gtUInt64) m_addressRangeTable[i].pc_end,
            bRet      = true;
        }

        // In case there are duplicate range we use the inner most one
        // TODO: Verify this part
        if ((addr < m_addressRangeTable[i].pc_start))
        {
            break;
        }
    }

    return bRet;
}

const gtUByte* JavaJncReader::GetCodeBytesOfTextSection(unsigned int* pSize)
{
    if (nullptr != pSize)
    {
        *pSize = m_textSize;
    }

    return m_pCodeBuf;
}
