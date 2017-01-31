//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file BytecodeToSource.cpp
///
//==================================================================================

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <sys/types.h>
#include <map>
#include <BytecodeToSource.h>

char UNKNOWN_METHOD[] = "unknown method";
char UNKNOWN_SIGNATURE[] = "unknown signature";
char UNKNOWN_SOURCE_FILE[] = "unknown source file";

//
// Builds method id mapping table (BytecodeToSource). Each entry has the form:
//    <methodid, pcstart, pcend, nameoffset, signatureoffset,
//     sourcefilenameoffset, linenumbertableoffset>
// The name, signature and source file name offsets are offsets into a
// string table. This function also populates the string table and a list
// of line number tables.
//
void buildBytecodeToSourceTable(jvmtiEnv*             pJvmtiEnv,
                                std::vector<void*>&   globalAddressRanges,
                                std::vector<void*>&   stringTable,
                                jint&                 stringTableSize,
                                std::vector<void*>&   lineNumberTables,
                                jint&                 lineNumberTableSize,
                                std::vector<jint>&    lineNumberTableEntryCounts)
{
    std::map<jmethodID, TableOffsets> id2tableOffsets;
    stringTableSize = 0;
    lineNumberTableSize = 0;

    jint methodSectionSize = (int)globalAddressRanges.size() * sizeof(AddressRange) + 2 * sizeof(jint);

    // fprintf(stderr,"methodSectionSize = %d\n", methodSectionSize);
    // fprintf(stderr,"globalAddressRanges size = %d\n", globalAddressRanges.size());

    for (int i = 0; i < (int)globalAddressRanges.size(); i++)
    {
        AddressRange* range = (AddressRange*)globalAddressRanges.at(i);

        if (NULL == range)
        {
            continue;
        }

        jmethodID id = (jmethodID)range->methodId;

        // We haven't seen this method id before. Create a new entry for it.
        if (id2tableOffsets.find(id) == id2tableOffsets.end())
        {
            char*                  methodName = NULL;
            char*                  methodSignature = NULL;
            char*                  sourceName = NULL;
            jclass                 methodDeclaringClass;
            jvmtiLineNumberEntry*  pLineNumberTable = NULL;
            jint                   entryCount = 0;
            jlong                  lineNumberTableOffset = 0;
            jlong                  methodNameOffset = 0;
            jlong                  methodSignatureOffset = 0;
            jlong                  sourceFileNameOffset = 0;

            // Get all of the information associated with this method id
            pJvmtiEnv->GetMethodName(id,
                                     &methodName,
                                     &methodSignature,
                                     NULL);

            pJvmtiEnv->GetMethodDeclaringClass(id, &methodDeclaringClass);
            pJvmtiEnv->GetSourceFileName(methodDeclaringClass, &sourceName);

            if (methodName == NULL)
            {
                methodName = UNKNOWN_METHOD;
            }

            if (methodSignature == NULL)
            {
                methodSignature = UNKNOWN_SIGNATURE;
            }

            if (sourceName == NULL)
            {
                sourceName = UNKNOWN_SOURCE_FILE;
            }

            // Add the name, signature and source file name to the string table
            stringTable.push_back((void*)methodName);
            stringTable.push_back((void*)methodSignature);
            stringTable.push_back((void*)sourceName);

            pJvmtiEnv->GetLineNumberTable(id,
                                          &entryCount,
                                          &pLineNumberTable);

            // Add the line number table for this method to list
            // of line number tables and compute the offset
            // of this method's line number table in that list.
            lineNumberTables.push_back((void*)pLineNumberTable);
            lineNumberTableEntryCounts.push_back(entryCount);
            lineNumberTableOffset = lineNumberTableSize;
            lineNumberTableSize += sizeof(jint) + entryCount * sizeof(jvmtiLineNumberEntry);

            // Compute the offsets of this method's name,
            // signature and source file name in the string table
            methodNameOffset = stringTableSize;
            jint methodNameSize = (jint)strlen(methodName) + sizeof(char);
            jint methodSignatureSize = (jint)strlen(methodSignature) + sizeof(char);
            jint sourceFileNameSize = (jint)strlen(sourceName) + sizeof(char);
            methodSignatureOffset = methodNameOffset + methodNameSize;
            sourceFileNameOffset = methodSignatureOffset + methodSignatureSize;
            stringTableSize += methodNameSize + methodSignatureSize + sourceFileNameSize;

            // Write the string table and line number table offsets
            // for this entry
            range->methodNameOffset      = methodNameOffset;
            range->methodSignatureOffset = methodSignatureOffset;
            range->sourceNameOffset      = sourceFileNameOffset;
            range->lineNumberTableOffset = methodSectionSize + lineNumberTableOffset;
            TableOffsets t;
            t.methodNameOffset      = methodNameOffset;
            t.sourceNameOffset      = sourceFileNameOffset;
            t.methodSignatureOffset = methodSignatureOffset;
            t.lineNumberTableOffset = methodSectionSize + lineNumberTableOffset;
            id2tableOffsets[id]     = t;
        }
        else
        {
            // We have seen this method id before.
            // Just update the string table and line number table offsets
            TableOffsets t               = id2tableOffsets.find(id)->second;
            range->methodNameOffset      = t.methodNameOffset;
            range->methodSignatureOffset = t.methodSignatureOffset;
            range->lineNumberTableOffset = t.lineNumberTableOffset;
            range->sourceNameOffset      = t.sourceNameOffset;
        }
    }
}


//
// Writes method id mapping table (BytecodeToSource) to memory in the format in which
// it will later be written to the JNC file.
//
// Format of the method table blob
//     <Header>
//         sizeof(methodtable)
//         sizeof(lineNumberTables)
//     <MethodTable>
//         methodtableentry-1
//         methodtableentry-2
//           .
//         methodtableentry-k
//     <LineNumberTable-1>
//         numentries (say k1)
//         linenumberentry-1
//         linenumberentry-2
//           .
//         linenumberentry-k1
//     <LineNumberTable-2>
//           .
//     <LineNumberTable-n>
//         numentries
//         linenumberentry1
//         linenumberentry2
//           .
//         linenumberentry-kn
//
void createMethodTableBlob(jvmtiEnv*             pJvmtiEnv,
                           std::vector<void*>&   methodTable,
                           jint                  methodTableSize,
                           std::vector<void*>&   lineNumberTables,
                           jint                  lineNumberTableSize,
                           std::vector<jint>&    lineNumberTableEntryCounts,
                           void*&                methodTableBlob,
                           jint&                 methodTableBlobSize)
{
    GT_UNREFERENCED_PARAMETER(pJvmtiEnv);
    int offset = 0;

    methodTableBlobSize = 2 * sizeof(jint) + methodTableSize + lineNumberTableSize;
    methodTableBlob = malloc(methodTableBlobSize);

    // Copy the header information
    memcpy((char*)methodTableBlob + offset, &methodTableSize, sizeof(jint));
    offset += sizeof(jint);

    memcpy((char*)methodTableBlob + offset, &lineNumberTableSize, sizeof(jint));
    offset += sizeof(jint);

    // Copy each method table entry
    for (size_t i = 0, e = methodTable.size(); i < e; i++)
    {
        AddressRange* entry = static_cast<AddressRange*>(methodTable.at(i));
        memcpy(static_cast<char*>(methodTableBlob) + offset, entry, sizeof(AddressRange));
        offset += sizeof(AddressRange);
    }

    // Copy each line number table. Each table is prefixed
    // with a header indicating the number of entries.
    for (size_t i = 0, e = lineNumberTables.size(); i < e; i++)
    {
        jvmtiLineNumberEntry* plineNumberTable = static_cast<jvmtiLineNumberEntry*>(lineNumberTables.at(i));
        jint entryCount = lineNumberTableEntryCounts.at(i);

        memcpy((char*)methodTableBlob + offset, &entryCount, sizeof(jint));
        offset += sizeof(jint);

        for (int j = 0; j < entryCount; j++)
        {
            memcpy(static_cast<char*>(methodTableBlob) + offset, &plineNumberTable[j], sizeof(jvmtiLineNumberEntry));
            offset += sizeof(jvmtiLineNumberEntry);
        }
    }
}


//
// Writes the string table and its header information to a blob in memory
// which will eventually be written to the JNC file. The string table is
// preceded by its size in bytes. This information is needed for postprocessing
// tools.
//
void createStringTableBlob(std::vector<void*>&  stringTable,
                           jint&                stringTableSize,
                           void*&               stringTableBlob)
{
    stringTableBlob = malloc(sizeof(char) * stringTableSize + sizeof(jint));
    int offset = 0;

    memcpy((char*)stringTableBlob + offset, &stringTableSize, sizeof(jint));
    offset += sizeof(jint);

    for (unsigned int i = 0; i < stringTable.size(); i++)
    {
        char* str = (char*)stringTable.at(i);
        strcpy((char*)stringTableBlob + offset, str);
        offset += (int)strlen(str) + 1;
    }

    stringTableSize += sizeof(jint);
}


void insertAddressRange(jmethodID             methodId,
                        const void*           jittedCodeAddr,
                        jint                  jittedCodeSize,
                        std::vector<void*>&   globalAddressRanges,
                        jint&                 bytecodeToSourceTableSize)
{
    AddressRange* range = new AddressRange();

    range->methodId = (gtVAddr)methodId;
    range->pcStart  = (gtUInt64)jittedCodeAddr;
    range->pcEnd    = (gtUInt64)((char*)jittedCodeAddr + jittedCodeSize);
    globalAddressRanges.push_back((void*)range);

    bytecodeToSourceTableSize = (jint)globalAddressRanges.size() * sizeof(AddressRange);
}


//
// Structure of inline information blob
//
//     total_size
//     numpcs
//     pcinfo<1>
//     .
//     .
//     pcinfo<n>
//
// where each pcinfo<i> has the form:
//     pc
//     numstackframes
//     methodid0
//     methodid1
//       .
//       .
//     methodid(numstackframes-1)
//     bcindex0
//     bcindex1
//       .
//       .
//     bcindex(numstackframes-1)
//
int createJNCMethodLoadLineInfoBlob(jvmtiEnv*                    pJvmtiEnv,
                                    jmethodID*                   pMethodId,
                                    const jvmtiAddrLocationMap*  pLocMap,
                                    jint                         locMapSize,
                                    void*&                       lineInfoBlob,
                                    jint&                        lineInfoBlobSize)
{
    GT_UNREFERENCED_PARAMETER(pJvmtiEnv);

    if (NULL == pLocMap)
    {
        return -1;
    }

    unsigned int sizeOfEachPcStackInfo = sizeof(void*) +
                                         sizeof(jint) +
                                         sizeof(jmethodID) +
                                         sizeof(jint);

    lineInfoBlobSize = (sizeOfEachPcStackInfo * locMapSize) +
                       sizeof(jint) +  // numpcs
                       sizeof(jint);   // totalSize

    lineInfoBlob = malloc(lineInfoBlobSize);

    if (NULL == lineInfoBlob)
    {
        return -1;
    }

    int offset = 0;

    // Set totalSize
    memcpy((char*)lineInfoBlob + offset, &lineInfoBlobSize, sizeof(jint));
    offset += sizeof(jint);

    // Set number of PCs
    memcpy((char*)lineInfoBlob + offset, &locMapSize, sizeof(jint));
    offset += sizeof(jint);

    // Set each JncPcStackInfo
    for (int i = 0 ; i < locMapSize; i++)
    {
        // pc
        memcpy((char*)lineInfoBlob + offset, &(pLocMap[i].start_address), sizeof(void*));
        offset += sizeof(void*);

        // numstackframes = 1;
        jint numstackframe = 1;
        memcpy((char*)lineInfoBlob + offset, &numstackframe, sizeof(jint));
        offset += sizeof(jint);

        // methods
        memcpy((char*)lineInfoBlob + offset, pMethodId, sizeof(jmethodID));
        offset += sizeof(jmethodID);

        // bcis
        memcpy((char*)lineInfoBlob + offset, &(pLocMap[i].location), sizeof(jint));
        offset += sizeof(jint);
    }

    return 0;
}

void freeMethodTables(jvmtiEnv* pJvmtiEnv, std::vector<void*>& methodTable, std::vector<void*>& lineNumberTables)
{
    for (int i = 0; i < (int)methodTable.size(); i++)
    {
        AddressRange* range = (AddressRange*)methodTable.at(i);

        if (range != NULL)
        {
            free(range);
            range = NULL;
        }
    }

    methodTable.clear();

    for (unsigned int i = 0; i < lineNumberTables.size(); i++)
    {
        jvmtiLineNumberEntry* plineNumberTable = (jvmtiLineNumberEntry*)lineNumberTables.at(i);

        if (NULL != plineNumberTable)
        {
            pJvmtiEnv->Deallocate((unsigned char*)plineNumberTable);
        }
    }

    lineNumberTables.clear();
}
