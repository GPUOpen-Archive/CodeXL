//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file BytecodeToSource.h
///
//==================================================================================

#ifndef _BYTECODETOSOURCE_H
#define _BYTECODETOSOURCE_H

#include <vector>
#include <jvmti.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

using namespace std;

struct AddressRange
{
    gtVAddr   methodId;
    gtUInt64  pcStart;
    gtUInt64  pcEnd;
    jlong     methodNameOffset;
    jlong     methodSignatureOffset;
    jlong     sourceNameOffset;
    jlong     lineNumberTableOffset;
};

struct TableOffsets
{
    jlong methodNameOffset;
    jlong methodSignatureOffset;
    jlong sourceNameOffset;
    jlong lineNumberTableOffset;
};

extern void buildBytecodeToSourceTable(jvmtiEnv* pJvmtiEnv,
                                       vector<void*>& globalAddressRanges,
                                       vector<void*>& stringTable,
                                       jint& stringTableSize,
                                       vector<void*>& lineNumberTables,
                                       jint& lineNumberTableSize,
                                       vector<jint>& lineNumberTableEntryCounts);

extern void createMethodTableBlob(jvmtiEnv* pJvmtiEnv,
                                  vector<void*>& methodTable,
                                  jint methodTableSize,
                                  vector<void*>& lineNumberTables,
                                  jint lineNumberTableSize,
                                  vector<jint>& lineNumberTableEntryCounts,
                                  void*& methodTableBlob,
                                  jint& methodTableBlobSize);

extern void createStringTableBlob(vector<void*>& stringTable,
                                  jint& stringTableSize,
                                  void*& stringTableBlob);

extern void insertAddressRange(jmethodID methodId,
                               const void* jittedCodeAddr,
                               jint jittedCodeSize,
                               vector<void*>& globalAddressRanges,
                               jint& bytecodeToSourceTableSize);

extern int createJNCMethodLoadLineInfoBlob(jvmtiEnv* pJvmtiEnv,
                                           jmethodID* pMethodId,
                                           const jvmtiAddrLocationMap* pLocMap,
                                           jint locMapSize,
                                           void*& lineInfoBlob,
                                           jint& lineinfoBlobSize);

extern void freeMethodTables(jvmtiEnv* pJvmtiEnv,
                             vector<void*>& methodTables,
                             vector<void*>& lineNumberTables);

#endif // _BYTECODETOSOURCE_H
