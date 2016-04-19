//=====================================================================
// Copyright (c) 2013 Advanced Micro Devices, Inc. All rights reserved.
//
/// \file $File: $
/// \version $Revision: $
/// \brief
//
//=====================================================================
// $Id: $
// Last checkin:   $DateTime: $
// Last edited by: $Author: $
// Change list:    $Change: $
//=====================================================================

#ifndef _JNCJVMTITYPES_H_
#define _JNCJVMTITYPES_H_

#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtASCIIString.h>

//
//    Typedefs
//

#ifndef _JAVA_JVMTI_H_

    typedef int jint;
    typedef gtInt64 jlong;
    typedef gtVAddr jmethodID;
    typedef gtInt64 jlocation;

#endif // _JAVA_JVMTI_H_


// JNPCStackInfo
//
struct JncPcStackInfo
{
    gtUInt64    pc;              // the pc address for this compiled method
    jint        numstackframes;  // number of methods on the stack
    jmethodID*  methods;         // array of numstackframes method ids
    jint*       bcis;            // array of numstackframes bytecode indices
};

typedef gtMap<gtUInt64, JncPcStackInfo*> JncPcStackInfoMap;

// JncMethodLoadLineInfo
//
struct JncMethodLoadLineInfo
{
    jint             numinfos;  // number of pc descriptors in this method
    JncPcStackInfo*  pcinfo;    // array of numpcs pc descriptors
};

// JncJvmtiLineNumberEntry
//
struct JncJvmtiLineNumberEntry
{
    jlocation  start_location;
    jint       line_number;

    bool operator<(const struct JncJvmtiLineNumberEntry& rhs) const
    {
        return start_location < rhs.start_location;
    }
};

typedef gtVector<JncJvmtiLineNumberEntry> JncJvmtiLineNumberEntryVec;


// JncMethod
//
struct JncMethod
{
    jmethodID                   id;
    string                      name;
    string                      signature;
    string                      sourceName;
    JncJvmtiLineNumberEntryVec  lineNumberVec;
};

typedef gtMap<jmethodID, JncMethod> JNCMethodMap;


// JncAddressRange
//
struct JncAddressRange
{
    jmethodID  id;
    gtUInt64   pc_start;
    gtUInt64   pc_end;

    bool operator<(const struct JncAddressRange& rhs) const
    {
        return pc_start < rhs.pc_start;
    }
};

typedef gtVector<JncAddressRange> JncAddressRangeVec;

#endif // _JNCJVMTITYPES_H_