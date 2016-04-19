//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file JvmtiProfileAgent_Lin.cpp
/// \brief JVMTI Java Profile Agent - Linux specific implementation.
///
//==================================================================================

// System Headers
#include <sys/types.h>
#include <sys/stat.h>
#include <asm/unistd.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cwchar>

#include <AMDTProfilingAgentsData/inc/JncWriter.h>
#include <JvmtiProfileAgent.h>

extern int gJvmtiVerbose;


//
//    Helper Routines
//

// getCmdline
//
void GetCmdline()
{
    char tmp[100] = { 0 };
    pid_t pid = gJvmPID;
    stringstream tstream;

    string jvmCmd = "";

    tstream << "/proc/" << pid << "/cmdline";
    string pidDir = tstream.rdbuf()->str();

    filebuf fb;

    if (NULL == fb.open(pidDir.c_str(), ios::in))
    {
        cout << "filebuf failed to open" << endl;
    }

    istream is(&fb);

    while (!is.eof())
    {
        is >> jvmCmd;

        if (gJvmtiVerbose)
        {
            cout << "getCmdline " << jvmCmd << endl;
        }
    }

    int end = jvmCmd.find("CXLJvmtiAgent\0");

    if (end > -1)
    {
        jvmCmd.erase(0, end);

        // Attempt to parse the string
        // -agentpath:/usr/local/bin/libAMDTJvmtiAgent64.so
        sprintf(tmp, "%s", jvmCmd.c_str());
        unsigned int tmpSize = strlen(tmp);

        if (jvmCmd.length() > tmpSize)
        {
            jvmCmd.erase(0, tmpSize + 1);
        }
    }

    for (unsigned int i = 0, e = jvmCmd.length(); i < e; i++)
    {
        if ('\0' == jvmCmd[i])
        {
            jvmCmd[i] = ' ';
        }
    }

    wchar_t  pTempWStr[TMP_MAX_PATH] = { L'\0' };
    memset(pTempWStr, 0, (TMP_MAX_PATH * sizeof(wchar_t)));
    mbstowcs(pTempWStr, jvmCmd.c_str(), (TMP_MAX_PATH - 1));
    gJvmCmd = pTempWStr;
} // getCmdline


//
//    dumper routines to dump the inline info
//

void PrintDummyRecord(jvmtiCompiledMethodLoadDummyRecord* record,
                      FILE*                               fp)
{
    if (NULL != record)
    {
        fprintf(fp, "Dummy record. message: %s\n", (char*)record->message);
    }
} // PrintDummyRecord


#define VERIFY_JVMTI_ERROR(errnum) if (errnum != JVMTI_ERROR_NONE) { return -1; }


// Print the PCStackInfo records
int PrintStackFrames(PCStackInfo*  record,
                     jvmtiEnv*     pJvmtiEnv,
                     FILE*         fp)
{
    if (record != NULL && record->methods != NULL)
    {
        int i;

        fprintf(fp, "Number of stack frames: %d\n", record->numstackframes);

        for (i = 0; i < record->numstackframes; i++)
        {
            jvmtiError err;
            char* methodName = NULL;
            char* className = NULL;
            char* methodSignature = NULL;
            char* classSignature = NULL;
            char* genericPtrMethod = NULL;
            char* genericPtrClass = NULL;
            jmethodID id;
            jclass declaringClassPtr;
            id = record->methods[i];

            err = pJvmtiEnv->GetMethodDeclaringClass(id, &declaringClassPtr);
            VERIFY_JVMTI_ERROR(err);

            err = pJvmtiEnv->GetClassSignature(declaringClassPtr,
                                               &classSignature,
                                               &genericPtrClass);
            VERIFY_JVMTI_ERROR(err);

            err = pJvmtiEnv->GetMethodName(id,
                                           &methodName,
                                           &methodSignature,
                                           &genericPtrMethod);
            VERIFY_JVMTI_ERROR(err);

            fprintf(fp, "\tcallstack frame : %d --- starts ---\n", i);
            fprintf(fp, "\t%s::%s %s %s @%d\n", classSignature, methodName,
                    methodSignature,
                    genericPtrMethod == NULL ? "" : genericPtrMethod,
                    record->bcis[i]);
            fprintf(fp, "\tcallstack frame : %d --- done ---\n", i);

            if (methodName != NULL)
            {
                err = pJvmtiEnv->Deallocate((unsigned char*)methodName);
                VERIFY_JVMTI_ERROR(err);
            }

            if (methodSignature != NULL)
            {
                err = pJvmtiEnv->Deallocate((unsigned char*)methodSignature);
                VERIFY_JVMTI_ERROR(err);
            }

            if (genericPtrMethod != NULL)
            {
                err = pJvmtiEnv->Deallocate((unsigned char*)genericPtrMethod);
                VERIFY_JVMTI_ERROR(err);
            }

            if (className != NULL)
            {
                err = pJvmtiEnv->Deallocate((unsigned char*)className);
                VERIFY_JVMTI_ERROR(err);
            }

            if (classSignature != NULL)
            {
                err = pJvmtiEnv->Deallocate((unsigned char*)classSignature);
                VERIFY_JVMTI_ERROR(err);
            }

            if (genericPtrClass != NULL)
            {
                err = pJvmtiEnv->Deallocate((unsigned char*)genericPtrClass);
                VERIFY_JVMTI_ERROR(err);
            }
        }
    }

    return 0;
} // PrintStackFrames


// Print a jvmtiCompiledMethodLoadInlineRecord
void PrintInlineInfoRecord(jvmtiCompiledMethodLoadInlineRecord*  record,
                           jvmtiEnv*                             jvmti,
                           FILE*                                 fp)
{
    if (record != NULL && record->pcinfo != NULL)
    {
        int numpcs = record->numpcs;
        int i;

        fprintf(fp, "Inline Info Record - num PCs (%d)\n", numpcs);

        for (i = 0; i < numpcs; i++)
        {
            PCStackInfo pcrecord = (record->pcinfo[i]);

            fprintf(fp, "---------------------------------------\n");
            fprintf(fp, "PC Descriptor: i(%d) (pc=0x%lx):\n", i, (unsigned long)(pcrecord.pc));

            PrintStackFrames(&pcrecord, jvmti, fp);
        }
    }
} // PrintInlineInfoRecord


// Print CompiledMethodLoadRecord records
void PrintCompileInfoRecords(jvmtiCompiledMethodLoadRecordHeader*  list,
                             jvmtiEnv*                             pJvmtiEnv,
                             FILE*                                 fp)
{
    jvmtiCompiledMethodLoadRecordHeader* curr = list;

    fprintf(fp, "\nPrinting Compile Info:-\n");

    while (curr != NULL)
    {
        switch (curr->kind)
        {
            case JVMTI_CMLR_DUMMY:
                fprintf(fp, "\nDummy Record:-\n");
                PrintDummyRecord((jvmtiCompiledMethodLoadDummyRecord*)curr, fp);
                break;

            case JVMTI_CMLR_INLINE_INFO:
                fprintf(fp, "\nInline Record:-\n");
                PrintInlineInfoRecord((jvmtiCompiledMethodLoadInlineRecord*)curr, pJvmtiEnv, fp);
                break;

            default:
                fprintf(fp, "Unrecognized Record: kind=%d\n", curr->kind);
                break;
        }

        curr = (jvmtiCompiledMethodLoadRecordHeader*)curr->next;
    }

    fprintf(fp, "\n\n");
} // PrintCompileInfoRecords


// computeInlineInfoSize
//
// Compute the total size of this jvmtiCompiledMethodLoadInlineRecord
//
static jint computeInlineInfoSize(jvmtiCompiledMethodLoadInlineRecord* record)
{
    jint size = 0;

    if (record != NULL && record->pcinfo != NULL)
    {
        size += sizeof(jvmtiCompiledMethodLoadInlineRecord);

        for (int i = 0; i < record->numpcs; i++)
        {
            PCStackInfo pcinfo = record->pcinfo[i];
            size += sizeof(PCStackInfo) +
                    (pcinfo.numstackframes * (sizeof(jmethodID) + sizeof(jint)));
        }
    }

    return size;
} // computeInlineInfoSize


#if 0
// hasInlinedMethods
//
// Checks whether a compiled method contains inlined methods
//
static int hasInlinedMethods(jvmtiCompiledMethodLoadInlineRecord* record)
{
    if (record != NULL && record->pcinfo != NULL)
    {
        for (int i = 0; i < record->numpcs; i++)
        {
            PCStackInfo pcinfo = record->pcinfo[i];

            if (pcinfo.numstackframes > 1)
            {
                return 1;
            }
        }
    }

    return 0;
}
#endif // 0


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
    if (NULL == record)
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
                newelt.pcEnd    = (gtUInt64)((unsigned long)pcinfo.pc + 1);

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
                                (gtUInt64)((unsigned long)jittedCodeAddr + (unsigned long)jittedCodeSize);
                        }

                        globalAddressRanges.push_back((void*)completedMethod);
                        localStackFrames.pop_back();
                    }

                    // Add method with new id to local stack
                    AddressRange newInvocation;
                    newInvocation.methodId = (gtVAddr)inlinedMethodId;
                    newInvocation.pcStart  = (gtUInt64)pcinfo.pc;
                    newInvocation.pcEnd    = (gtUInt64)pcinfo.pc;
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
                    (gtUInt64)((unsigned long)jittedCodeAddr + (unsigned long)jittedCodeSize);
            }

            globalAddressRanges.push_back((void*)completedMethod);
            localStackFrames.pop_back();
        }
    }

    bytecodeToSourceTableSize = globalAddressRanges.size() * sizeof(AddressRange);
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
    // TODO: error check

    // Copy the total size of the inline data blob
    memcpy((char*)inlineInfoBlob + offset, &inlineInfoSize, sizeof(jint));
    offset += sizeof(jint);

    // Copy the number of PCs
    memcpy((char*)inlineInfoBlob + offset, &(record->numpcs), sizeof(jint));
    offset += sizeof(jint);

    // For every PC, write the pcinfo; Which contains
    // PC, numstackframes, numstackframes times methoid and bcindex
    for (int i = 0; i < record->numpcs; i++)
    {
        PCStackInfo pcinfo = record->pcinfo[i];
        jmethodID* methodIdArray = pcinfo.methods;
        jint* bcIndexArray = pcinfo.bcis;
        memcpy((char*)inlineInfoBlob + offset, &pcinfo.pc, sizeof(void*));
        offset += sizeof(void*);
        memcpy((char*)inlineInfoBlob + offset, &pcinfo.numstackframes, sizeof(jint));
        offset += sizeof(jint);

        for (int j = 0; j < pcinfo.numstackframes; j++)
        {
            jmethodID inlinedMethodId = methodIdArray[j];
            memcpy((char*)inlineInfoBlob + offset, &inlinedMethodId, sizeof(jmethodID));
            offset += sizeof(jmethodID);
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


// WriteJNC
//
static int WriteJNC(wstring*      jncFileName,
                    char const*   symbolName,
                    const void*   jittedCodeAddr,
                    unsigned int  jittedCodeSize,
                    const void*   pc2bcBlob,
                    unsigned int  pc2bcBlobSize,
                    void*         methodTableBlob,
                    unsigned int  methodTableBlobSize,
                    void*         stringTableBlob,
                    unsigned int  stringTableBlobSize)
{
    int ret = -1;
    JncWriter jncWriter;

    // fprintf(stderr,"WriteJNC: symbolName = %s\n", symbolName);

    if (NULL == jncFileName)
    {
        return -1;
    }

    // string jncFile(jncFileName->begin(), jncFileName->end());
    // fprintf(stderr,"WriteJNC: jnc-file-name = %s\n", jncFile.c_str());
    // wprintf("WriteJNC (wstring): jnc-file-name = %S\n", jncFileName->c_str());

    char jncfile[TMP_MAX_PATH] = { '\0' };
    wcstombs(jncfile, jncFileName->c_str(), (TMP_MAX_PATH - 1));
    // fprintf(stderr,"WriteJNC: jnc-file = %s %S \n",
    //        jncfile, jncFileName->c_str());

    ret = jncWriter.init(jncfile,
                         symbolName,
                         jittedCodeAddr,
                         jittedCodeSize);

    if (0 != ret)
    {
        return ret;
    }

    if (pc2bcBlob != NULL && pc2bcBlobSize > 0)
    {
        ret = jncWriter.addSection((const char*)".pc2bc",
                                   (const void*)pc2bcBlob,
                                   pc2bcBlobSize,
                                   0,     // addr
                                   false); // is progbits

        if (0 != ret)
        {
            return ret;
        }
    }

    if (stringTableBlob != NULL && stringTableBlobSize > 0)
    {
        ret = jncWriter.addSection((const char*)".stringtable",
                                   (const void*)stringTableBlob,
                                   stringTableBlobSize,
                                   0, // addr
                                   false);

        if (0 != ret)
        {
            return ret;
        }
    }

    if (methodTableBlob != NULL && methodTableBlobSize > 0)
    {
        ret = jncWriter.addSection((const char*)".bc2src",
                                   (const void*) methodTableBlob,
                                   methodTableBlobSize,
                                   0,
                                   false);

        if (0 != ret)
        {
            return ret;
        }
    }

    return jncWriter.write();
} // WriteJNC


// WriteJncFile
//
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
    string strName;

    if ((NULL == jncFileName) || (NULL == mInfo))
    {
        return;
    }

    bool hasInlineInfo = false;
    jvmtiCompiledMethodLoadRecordHeader*  methodLoadRec =
        (jvmtiCompiledMethodLoadRecordHeader*)compileInfo;
    jvmtiCompiledMethodLoadInlineRecord* inlineRec = NULL;

    if (NULL != methodLoadRec)
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

    strName = mInfo->pClassSig;
    strName += "::";
    strName += mInfo->pMethodName;

    if (gJvmtiVerbose)
    {
        fprintf(stderr, "writeJNCFile: addr = %p, methodName = %s\n", codeAddr, mInfo->pMethodName);
        fprintf(stderr, "Java Src file : %s\n", mInfo->pSrcFile);

        // Get the Line Number table
        jvmtiLineNumberEntry* pLineNumberTable = NULL;
        jint entryCount = 0;
        pJvmtiEnv->GetLineNumberTable(method,
                                      &entryCount,
                                      &pLineNumberTable);

        // Print the Line number table entries
        if (pLineNumberTable)
        {
            fprintf(stderr, "Dump jvmtiLineNumberEntry\n");

            for (jint i = 0 ; i < entryCount; i++)
            {
                fprintf(stderr, "%3u, loc:%lu, line:%u\n",
                        i,
                        pLineNumberTable[i].start_location,
                        pLineNumberTable[i].line_number);
            }
        }

        // Address Location
        if (map)
        {
            fprintf(stderr, "Dump Address Location\n");

            for (jint i = 0 ; i < mapLength; i++)
            {
                fprintf(stderr, "%3u, start_addr:%p, location:%lu\n",
                        i,
                        map[i].start_address,
                        map[i].location);
            }
        }

        if (hasInlineInfo)
        {
            fprintf(stderr, "writeJNCFile: found inline info.\n");

            methodLoadRec = (jvmtiCompiledMethodLoadRecordHeader*)compileInfo;

            PrintCompileInfoRecords(methodLoadRec,
                                    pJvmtiEnv,
                                    stderr);
        }

    } // gJvmtiVerbose

    // We need to create the following sections
    //     - .string_table
    //     - .bc2src
    //     - .pc2bc

    // Build Sections
    vector<void*>  globalAddressRanges;
    jint           bytecodeToSourceTableSize = 0;

    if (! hasInlineInfo)
    {
        insertAddressRange(method,
                           codeAddr,
                           codeSize,
                           globalAddressRanges,
                           bytecodeToSourceTableSize);
    }
    else
    {
        // Build the table to map the inlined method id's to PC address ranges
        inlineRec = (jvmtiCompiledMethodLoadInlineRecord*)compileInfo;

        BuildInlineAddressRanges(inlineRec,
                                 globalAddressRanges,
                                 bytecodeToSourceTableSize,
                                 codeAddr,
                                 codeSize);
    }

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

    void*  methodTableBlob = NULL;
    jint   methodTableBlobSize = 0;

    createMethodTableBlob(pJvmtiEnv,
                          globalAddressRanges,
                          bytecodeToSourceTableSize,
                          lineNumberTables,
                          lineNumberTableSize,
                          lineNumberTableEntryCounts,
                          methodTableBlob,
                          methodTableBlobSize);

    void*  lineInfoBlob = NULL;
    jint   lineInfoBlobSize;

    if (! hasInlineInfo)
    {
        createJNCMethodLoadLineInfoBlob(pJvmtiEnv,
                                        &method,
                                        map,
                                        mapLength,
                                        lineInfoBlob,
                                        lineInfoBlobSize);
    }
    else
    {
        lineInfoBlobSize = computeInlineInfoSize(inlineRec);
        lineInfoBlobSize = CreateInlineDataBlob(inlineRec,
                                                lineInfoBlobSize,
                                                lineInfoBlob);
    }

    // Write JNC file
    WriteJNC(jncFileName,
             strName.c_str(),
             codeAddr,
             codeSize,
             lineInfoBlob,
             lineInfoBlobSize,
             methodTableBlob,
             methodTableBlobSize,
             stringTableBlob,
             stringTableSize);

    // Free the allocated memory
    if (NULL != lineInfoBlob)
    {
        free(lineInfoBlob);
        lineInfoBlob = NULL;
    }

    if (NULL != methodTableBlob)
    {
        free(methodTableBlob);
        methodTableBlob = NULL;
    }

    if (NULL != stringTableBlob)
    {
        free(stringTableBlob);
        stringTableBlob = NULL;
    }

    freeMethodTables(pJvmtiEnv, globalAddressRanges, lineNumberTables);
} // WriteJncFile


// WriteNativeToJncFile
//
int WriteNativeToJncFile(char const*   symbolName,
                         const void*   jittedCodeAddr,
                         jint          jittedCodeSize,
                         wstring*      jncFileName)
{
    JncWriter jncWriter;

    if (NULL == jncFileName)
    {
        return -1;
    }

    // string jncFile(jncFileName->begin(), jncFileName->end());
    char jncfile[TMP_MAX_PATH] = { '\0' };
    wcstombs(jncfile, jncFileName->c_str(), (TMP_MAX_PATH - 1));
    // fprintf(stderr,"writeNativeToJNCFile: jnc-file = %s\n", jncfile);

    int ret = jncWriter.init(jncfile,
                             symbolName,
                             jittedCodeAddr,
                             jittedCodeSize);

    if (0 != ret)
    {
        return -1;
    }

    return jncWriter.write();
} // WriteNativeToJncFile
