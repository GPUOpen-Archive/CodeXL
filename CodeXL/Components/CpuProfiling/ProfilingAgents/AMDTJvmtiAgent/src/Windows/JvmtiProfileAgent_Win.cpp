//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file JvmtiProfileAgent_Win.cpp
/// \brief JVMTI Java Profile Agent - Windows specific implementation.
///
//==================================================================================

#include <AMDTProfilingAgentsData/inc/JncWriter.h>
#include <JvmtiProfileAgent.h>

extern int gJvmtiVerbose;

static jint computeInlineInfoSize(jvmtiCompiledMethodLoadInlineRecord* record);

void JNICALL writeInlineInfo(jvmtiEnv*      pJvmtiEnv,
                             jint           codeSize,
                             const void*    codeAddr,
                             methodInfo*    mInfo,
                             jmethodID      method,
                             const void*    compileInfo,
                             JncWriter&     jncWriter);

static void BuildInlineAddressRanges(
    jvmtiCompiledMethodLoadInlineRecord*  record,
    vector<void*>&                        globalAddressRanges,
    jint&                                 bytecodeToSourceTableSize,
    const void*                           jittedCodeAddr,
    jint                                  jittedCodeSize);

static unsigned int CreateInlineDataBlob(
    jvmtiCompiledMethodLoadInlineRecord* record,
    jint                                 inlineInfoSize,
    void*&                               inlineInfoBlob);

#if 0
    void DumpStringTable(unsigned int stringTableSize, void* stringTableBuf /*FILE* f=NULL*/);

    bool GetStringFromOffset(unsigned int offset, string& str, unsigned int stringTableSize, void* stringTableBuf);
#endif


// Had to implement this since Microsoft doesn't create parent directories with mkdir
//
int RecursiveMkdir(const wchar_t* pDir)
{
    int ret = 0;
    wchar_t tempDir[FILENAME_MAX] = { L'\0' };
    wchar_t* pNextDir = const_cast<wchar_t*>(pDir);

    memset(tempDir, 0, (FILENAME_MAX) * sizeof(wchar_t));

    while ((pNextDir = wcschr(pNextDir, L'\\')) != nullptr)
    {
        pNextDir ++;
        wcsncpy_s(tempDir, FILENAME_MAX, pDir, (pNextDir - pDir));
        _wmkdir(tempDir);
    }

    if (0 == ret)
    {
        ret = _wmkdir(pDir);
    }

    return ret;
}

void GetCmdline()
{
    gJvmCmd = GetCommandLine();

    for (unsigned int i = 0; i < gJvmCmd.length(); i++)
    {
        if (gJvmCmd[i] == L'\0')
        {
            gJvmCmd[i] = L'_';
        }
    }

    size_t endOfCommand = gJvmCmd.find(L"CXLJvmti");

    if (endOfCommand >= 0)
    {
        gJvmCmd.erase(0, endOfCommand);
        size_t index = gJvmCmd.find(L"dll\" ");

        if (index > 0)
        {
            gJvmCmd.erase(0, index + 5);
        }
    }
} // getCmdline


// Write the JNC file for compiled method
void JNICALL WriteJncFile(jvmtiEnv*                    pJvmtiEnv,
                          jmethodID                    method,
                          jint                         codeSize,
                          const void*                  codeAddr,
                          jint                         mapLength,
                          const jvmtiAddrLocationMap*  map,
                          wstring*                     jncFileName,
                          methodInfo*                  mInfo,
                          const void*                  compileInfo)
{
    JncWriter  jncWriter;
    char*     pCodeAddr = (char*)codeAddr;
    wstring   className;

    if (nullptr == mInfo)
    {
        return;
    }

    bool hasInlineInfo = false;
    jvmtiCompiledMethodLoadRecordHeader*  methodLoadRec =
        (jvmtiCompiledMethodLoadRecordHeader*)compileInfo;

    if (nullptr != methodLoadRec)
    {
        if (methodLoadRec->kind == JVMTI_CMLR_INLINE_INFO)
        {
            hasInlineInfo = true;
        }
        else
        {
            fprintf(stderr, "Invalid JVMTI_CMLR record type\n");
            return;
        }
    }

    if (gJvmtiVerbose)
    {
        fwprintf(stderr, L"In writeJNCFile, jncFileName is " WIDE_STR_FORMAT L"\n", jncFileName->c_str());

        if (hasInlineInfo)
        {
            fwprintf(stderr, L"writeJNCFile: found inline info.\n");
            methodLoadRec = (jvmtiCompiledMethodLoadRecordHeader*)compileInfo;
        }
    }

    wchar_t pTemp[OS_MAX_PATH] = { L'\0' };
    size_t retVal;
    mbstowcs_s(&retVal, pTemp, OS_MAX_PATH, mInfo->pClassSig, _TRUNCATE);
    className = pTemp;

    jncWriter.SetJNCFileName(jncFileName->c_str());

#if 0
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
    gtUInt64 jitLoadAddr = (gtUInt64) codeAddr;
#else
    // FIXME: why is codeSize added to jitLoadAddr
    unsigned int jitLoadAddr = (unsigned int)codeAddr + codeSize;
#endif

    jncWriter.SetJITStartAddr((gtUInt64)jitLoadAddr);
#else
    jncWriter.SetJITStartAddr((gtUInt64)codeAddr);
#endif

    memset(pTemp, 0, (OS_MAX_PATH * sizeof(wchar_t)));
    mbstowcs_s(&retVal, pTemp, OS_MAX_PATH, mInfo->pMethodName, _TRUNCATE);

    wchar_t pTempSrc[OS_MAX_PATH] = { L'\0' };
    mbstowcs_s(&retVal, pTempSrc, OS_MAX_PATH, mInfo->pSrcFile, _TRUNCATE);

    jncWriter.SetJITFuncName(className.c_str(),
                             pTemp,        // Method name
                             pTempSrc);    // java source file

    jvmtiLineNumberEntry*  tiLineTable = nullptr;
    JavaSrcLineInfo*       javaLineTable = nullptr;
    long                   numberLines = 0;

    pJvmtiEnv->GetLineNumberTable(method,
                                  &numberLines,
                                  &tiLineTable);

    if (gJvmtiVerbose)
    {
        fwprintf(stderr, L"In writeJNCFile, number of source lines is %d\n", numberLines);
    }

    if (true == hasInlineInfo)
    {
        // construct inlined method details amd write into JNC file
        writeInlineInfo(pJvmtiEnv,
                        codeSize,
                        codeAddr,
                        mInfo,
                        method,
                        compileInfo,
                        jncWriter);
    }
    else if ((mapLength > 0) && (numberLines > 0))
    {
        javaLineTable = new JavaSrcLineInfo[mapLength];

        if (nullptr != javaLineTable)
        {
            for (jint i = 0; i < mapLength; i++)
            {
                gtUIntPtr offset = reinterpret_cast<gtUIntPtr>(map[i].start_address) - reinterpret_cast<gtUIntPtr>(codeAddr);
                javaLineTable[i].addrOffset = static_cast<unsigned int>(offset);

                javaLineTable[i].lineNum = tiLineTable[numberLines - 1].line_number;

                for (long k = 1; k < numberLines; k++)
                {
                    if (map[i].location < tiLineTable[k].start_location)
                    {
                        javaLineTable[i].lineNum = tiLineTable[k - 1].line_number;
                        break;
                    }
                }

                jncWriter.WriteJITNativeCode((gtUByte*)pCodeAddr, codeSize, javaLineTable, mapLength);
                delete[] javaLineTable;
            }
        }
        else
        {
            jncWriter.WriteJITNativeCode((gtUByte*)pCodeAddr, codeSize);
        }
    }
    else
    {
        bool ret = jncWriter.WriteJITNativeCode((gtUByte*)pCodeAddr, codeSize);

        if (!ret && gJvmtiVerbose)
        {
            printf("failed to writeJITNativeCode\n");
        }
    }

    if (nullptr != tiLineTable)
    {
        pJvmtiEnv->Deallocate((unsigned char*)tiLineTable);
    }

    jncWriter.Close();
}


// Write the JNC file for methods with no information
int WriteNativeToJncFile(const char*   name,
                         const void*   codeAddr,
                         jint          codeSize,
                         wstring*      jncFileName)
{
    JncWriter jncWriter;
    char* pCodeAddr = (char*)codeAddr;
    wstring class_name = L"Native Code::";

    if (gJvmtiVerbose)
    {
        fwprintf(stderr, L"In writeJNCFile, method: " CSTR_FORMAT L" jncFileName: " WIDE_STR_FORMAT L"\n",
                 name, jncFileName->c_str());
    }

    gtUInt64  jitLoadAddr = 0;
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
    jitLoadAddr = (gtUInt64) codeAddr;
#else
    //FIXME: why is code_size added to jitLoadAddr
    jitLoadAddr = (unsigned int)codeAddr + codeSize;
#endif

    jncWriter.SetJNCFileName(jncFileName->c_str());
    // jncWriter.SetJITStartAddr(jitLoadAddr);
    jncWriter.SetJITStartAddr((gtUInt64)codeAddr);

    wchar_t pTemp[OS_MAX_PATH] = { L'\0' };
    size_t retVal;
    mbstowcs_s(&retVal, pTemp, OS_MAX_PATH, name, _TRUNCATE);

    jncWriter.SetJITFuncName(L"Native Code::", pTemp);

    bool ret = jncWriter.WriteJITNativeCode((unsigned char*)pCodeAddr, codeSize);

    if (!ret && gJvmtiVerbose)
    {
        printf("failed to writeJITNativeCode\n");
    }

    jncWriter.Close();

    return 0;
}

void JNICALL writeInlineInfo(jvmtiEnv*      pJvmtiEnv,
                             jint           codeSize,
                             const void*    codeAddr,
                             methodInfo*    mInfo,
                             jmethodID      method,
                             const void*    compileInfo,
                             JncWriter&     jncWriter)
{
    GT_UNREFERENCED_PARAMETER(method);

    jvmtiCompiledMethodLoadInlineRecord* inlineRec = nullptr;

    if ((nullptr == mInfo) || (nullptr == compileInfo) || (nullptr == codeAddr))
    {
        return;
    }

    // Build Sections
    vector<void*>  globalAddressRanges;
    jint           bytecodeToSourceTableSize = 0;

    // Build the table to map the inlined method id's to PC address ranges
    inlineRec = (jvmtiCompiledMethodLoadInlineRecord*)compileInfo;

    BuildInlineAddressRanges(inlineRec,
                             globalAddressRanges,
                             bytecodeToSourceTableSize,
                             codeAddr,
                             codeSize);

    vector<void*>  stringTable;
    vector<void*>  lineNumberTables;
    vector<jint>   lineNumberTableEntryCounts;
    jint           stringTableSize = 0;
    jint           lineNumberTableSize = 0;

    buildBytecodeToSourceTable(pJvmtiEnv,
                               globalAddressRanges,
                               stringTable,
                               stringTableSize,
                               lineNumberTables,
                               lineNumberTableSize,
                               lineNumberTableEntryCounts);

    // Create Blobs
    void* stringTableBlob;
    createStringTableBlob(stringTable,
                          stringTableSize,
                          stringTableBlob);

    //DumpStringTable(stringTableSize, stringTableBlob);

    void*  methodTableBlob = nullptr;
    jint   methodTableBlobSize = 0;

    createMethodTableBlob(pJvmtiEnv,
                          globalAddressRanges,
                          bytecodeToSourceTableSize,
                          lineNumberTables,
                          lineNumberTableSize,
                          lineNumberTableEntryCounts,
                          methodTableBlob,
                          methodTableBlobSize);

    void*  lineInfoBlob = nullptr;
    jint   lineInfoBlobSize = 0;

    lineInfoBlobSize = computeInlineInfoSize(inlineRec);

    lineInfoBlobSize = CreateInlineDataBlob(inlineRec,
                                            lineInfoBlobSize,
                                            lineInfoBlob);

    if (nullptr != lineInfoBlob)
    {
        jncWriter.WriteJITNativeCode((gtUByte*)codeAddr,
                                     codeSize,
                                     lineInfoBlob,
                                     lineInfoBlobSize,
                                     methodTableBlob,
                                     methodTableBlobSize,
                                     stringTableBlob,
                                     stringTableSize);

        free(lineInfoBlob);
        lineInfoBlob = nullptr;
    }

    if (nullptr != methodTableBlob)
    {
        free(methodTableBlob);
        methodTableBlob = nullptr;
    }

    if (nullptr != stringTableBlob)
    {
        free(stringTableBlob);
        stringTableBlob = nullptr;
    }

    freeMethodTables(pJvmtiEnv, globalAddressRanges, lineNumberTables);
} // writeInlineInfo


// computeInlineInfoSize
//
// Compute the total size of this jvmtiCompiledMethodLoadInlineRecord
//
static jint computeInlineInfoSize(jvmtiCompiledMethodLoadInlineRecord* record)
{
    jint size = 0;

    if (record != nullptr && record->pcinfo != nullptr)
    {
        for (int i = 0; i < record->numpcs; i++)
        {
            PCStackInfo pcinfo = record->pcinfo[i];

            // PC + Number-of-StackFrames + Method-IDs + BCIs
            size += sizeof(gtUInt64) + sizeof(jint) + (pcinfo.numstackframes * (sizeof(gtVAddr) + sizeof(jint)));
        }
    }

    return size;
} // computeInlineInfoSize


// BuildInlineAddressRanges
//
// Scans the jvmtiCompiledMethodLoadInlineRecord object and builds
// a table of PC address ranges for each method id. This is the first
// step to building the method id mapping table (BytecodeToSource).
//
static void BuildInlineAddressRanges(
    jvmtiCompiledMethodLoadInlineRecord*  record,
    vector<void*>&                        globalAddressRanges,
    jint&                                 bytecodeToSourceTableSize,
    const void*                           jittedCodeAddr,
    jint                                  jittedCodeSize)
{
    if (nullptr == record)
    {
        return;
    }

    vector<AddressRange> localStackFrames;

    for (int i = 0; i < record->numpcs; i++)
    {
        PCStackInfo pcinfo = record->pcinfo[i];
        jmethodID* methodIdArray = pcinfo.methods;
        int numStackFrames = pcinfo.numstackframes;

        for (int j = 0; j < numStackFrames; j++)
        {
            jmethodID inlinedMethodId = methodIdArray[numStackFrames - 1 - j];

            // Add new element to list
            if (j > (int)localStackFrames.size() - 1)
            {
                AddressRange newelt;
                newelt.methodId = (gtVAddr)inlinedMethodId;
                newelt.pcStart  = (gtUInt64)pcinfo.pc;
                newelt.pcEnd    = (gtUInt64)((unsigned long long)pcinfo.pc + 1);

                localStackFrames.push_back(newelt);
            }
            else
            {
                AddressRange* localStackMethod = &localStackFrames.at(j);

                if (localStackMethod->methodId == (gtVAddr)inlinedMethodId)
                {
                    localStackMethod->pcEnd = (gtUInt64)pcinfo.pc;
                }
                else
                {
                    // remove the method with different id and all its descendents
                    // from the local stack. Send these completed methods to the
                    // global stack.
                    // We should never reach this case, but just in case...
                    while ((int)localStackFrames.size() > j)
                    {
                        AddressRange* completedMethod = new AddressRange();

                        completedMethod->methodId = localStackFrames.back().methodId;
                        completedMethod->pcStart = localStackFrames.back().pcStart;

                        if (i < record->numpcs - 1)
                        {
                            completedMethod->pcEnd = (gtUInt64)pcinfo.pc;
                        }
                        else
                        {
                            completedMethod->pcEnd =
                                (gtUInt64)((unsigned long long)jittedCodeAddr + (unsigned long)jittedCodeSize);
                        }

                        globalAddressRanges.push_back((void*)completedMethod);
                        localStackFrames.pop_back();
                    }

                    // Add method with new id to local stack
                    AddressRange newInvocation;
                    newInvocation.methodId = (gtVAddr)inlinedMethodId;
                    newInvocation.pcStart = (gtUInt64)pcinfo.pc;
                    newInvocation.pcEnd = (gtUInt64)pcinfo.pc;
                    localStackFrames.push_back(newInvocation);
                }
            }
        }

        // Send all done elements to global list
        int finalStackSize = (i < record->numpcs - 1) ? pcinfo.numstackframes : 0;

        while ((int)localStackFrames.size() > finalStackSize)
        {
            AddressRange* completedMethod = new AddressRange();

            completedMethod->methodId = localStackFrames.back().methodId;
            completedMethod->pcStart = localStackFrames.back().pcStart;

            if (i < record->numpcs - 1)
            {
                completedMethod->pcEnd = (gtUInt64)pcinfo.pc;
            }
            else
            {
                completedMethod->pcEnd =
                    (gtUInt64)((unsigned long long)jittedCodeAddr + (unsigned long)jittedCodeSize);
            }

            globalAddressRanges.push_back((void*)completedMethod);
            localStackFrames.pop_back();
        }
    }

    bytecodeToSourceTableSize = (jint)(globalAddressRanges.size() * sizeof(AddressRange));
} // BuildInlineAddressRanges


// CreateInlineDataBlob
//
// Writes the jvmtiCompiledMethodLoadInlineRecord to a blob in memory that will
// eventually be written out to a JNC file. The blob has the following format:
//
// Structure of inline information blob
//
//   totalSize
//   numpcs
//   pcinfo<1>
//   ..
//   pcinfo<numpcs>
//
// where each pcinfo<i> has the form:
//    PC
//    numstackframes
//    methodid-0
//    methodid-1
//    ...
//    methodid-(numstackframes - 1)
//    bcindex-0
//    bcindex-1
//    ...
//    bcindex-(numstackframes - 1)
//
static unsigned int CreateInlineDataBlob(
    jvmtiCompiledMethodLoadInlineRecord* record,
    jint                                 inlineInfoSize,
    void*&                               inlineInfoBlob)
{
    jint totalSize = 0;
    int offset = 0;

    totalSize = inlineInfoSize + (2 * sizeof(jint));

    inlineInfoBlob = malloc(totalSize);

    if (nullptr == inlineInfoBlob)
    {
        return 0;
    }

    // Copy the total size of the inline data blob
    memcpy((char*)inlineInfoBlob + offset, &inlineInfoSize, sizeof(jint));
    offset += sizeof(jint);

    // Copy the number of PCs
    memcpy((char*)inlineInfoBlob + offset, &(record->numpcs), sizeof(jint));
    offset += sizeof(jint);

    // For every PC, write the pcinfo; Which contains
    // PC, numstackframes, numstackframes times methoid and bcindex
    //
    for (int i = 0; i < record->numpcs; i++)
    {
        PCStackInfo pcinfo = record->pcinfo[i];
        jmethodID* methodIdArray = pcinfo.methods;
        jint* bcIndexArray = pcinfo.bcis;

        gtUInt64 pc = (gtUInt64) pcinfo.pc;
        memcpy((char*)inlineInfoBlob + offset, &pc, sizeof(gtUInt64));
        offset += sizeof(gtUInt64);

        memcpy((char*)inlineInfoBlob + offset, &pcinfo.numstackframes, sizeof(jint));
        offset += sizeof(jint);

        for (int j = 0; j < pcinfo.numstackframes; j++)
        {
            gtVAddr inlinedMethodId = (gtVAddr)methodIdArray[j];
            memcpy((char*)inlineInfoBlob + offset, &inlinedMethodId, sizeof(gtVAddr));
            offset += sizeof(gtVAddr);
        }

        for (int j = 0; j < pcinfo.numstackframes; j++)
        {
            jint bcIndex = bcIndexArray[j];
            memcpy((char*)inlineInfoBlob + offset, &bcIndex, sizeof(jint));
            offset += sizeof(jint);
        }
    }

    return totalSize;
} // CreateInlineDataBlob

#if 0
bool GetStringFromOffset(unsigned int offset, string& str, unsigned int stringTableSize, void* stringTableBuf)
{
    char*  pChar = nullptr;

    if ((offset >= stringTableSize) || (!stringTableBuf))
    {
        return false;
    }

    pChar = (char*)stringTableBuf + offset;
    str = string(pChar);
    return true;
}

void DumpStringTable(unsigned int stringTableSize, void* stringTableBuf /*FILE* f=NULL*/)
{
    string str;
    unsigned int offset = 0;

    char* p = (char*)stringTableBuf;
    p += 4 * sizeof(char);

    while (offset < stringTableSize)
    {
        std::wstring wstr;

        GetStringFromOffset(offset, str, stringTableSize, p);
        wstr.assign(str.begin(), str.end());

        //fprintf(f, " %4u:%s\n", offset, str.c_str());

        offset += (unsigned int)(str.length() + 1);
    }

    //fprintf(f, "DumpStringTable: END\n");
}
#endif
