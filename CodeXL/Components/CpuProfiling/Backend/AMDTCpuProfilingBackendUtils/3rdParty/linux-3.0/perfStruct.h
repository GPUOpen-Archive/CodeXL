//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file perfStruct.h
///
//==================================================================================

#ifndef __PERFSTRUCT_H_
#define __PERFSTRUCT_H_

typedef unsigned int u32;
typedef unsigned long long u64;
typedef __signed__ long long s64;

#define HEADER_FEAT_BITS    256
#define BITS_PER_BYTE           8
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
#define BITS_TO_LONGS(nr)       DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(long))
#define DECLARE_BITMAP(name,bits) unsigned long name[BITS_TO_LONGS(bits)]


#define PERF_MAGIC (*(u64 *)"PERFFILE")

// From util/header.h
struct perf_file_section
{
    u64 offset;
    u64 size;
};

/* From header.c */
struct perf_file_attr
{
    struct perf_event_attr      attr;
    struct perf_file_section    ids;
};

// From util/header.h
struct perf_file_header
{
    u64             magic;
    u64             size;
    u64             attr_size;
    struct perf_file_section    attrs;
    struct perf_file_section    data;
    struct perf_file_section    event_types;
    DECLARE_BITMAP(adds_features, HEADER_FEAT_BITS);
};


// From util/header.h
struct perf_header
{
    int         frozen;
    bool            needs_swap;
    s64         attr_offset;
    u64         data_offset;
    u64         data_size;
    u64         event_offset;
    u64         event_size;
    DECLARE_BITMAP(adds_features, HEADER_FEAT_BITS);
};

// From util/event.h
#define MAX_EVENT_NAME 64

struct perf_trace_event_type
{
    u64 event_id;
    char    name[MAX_EVENT_NAME];
};

#endif //__PERFSTRUCT_H_
