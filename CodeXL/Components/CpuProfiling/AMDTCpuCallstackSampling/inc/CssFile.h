//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CssFile.h
///
//==================================================================================

#ifndef _CSSFILE_H_
#define _CSSFILE_H_

#include <AMDTBaseTools/Include/AMDTDefinitions.h>

#if AMDT_CPP_COMPILER == AMDT_VISUAL_CPP_COMPILER
    #define CSS_FILE_SIGNATURE  FCC('CACS')  // 0x53434143
#else
    #define CSS_FILE_SIGNATURE  0x53434143U
#endif

#define CALL_SITE_FILE_VERSION  0x00010001  // major version (high 16 bits) 1, minor version (low 16 bits) 1
#define CALL_PATH_FILE_VERSION  0x00020001  // major version (high 16 bits) 2, minor version (low 16 bits) 1
#define CALL_GRAPH_FILE_VERSION 0x00020002  // major version (high 16 bits) 2, minor version (low 16 bits) 2

#if AMDT_CPP_COMPILER == AMDT_VISUAL_CPP_COMPILER
    #pragma warning(push)
    #pragma warning(disable : 4201) // nameless struct/union
#endif
struct CssFileHeader
{
    gtUInt32 m_signature;
    union
    {
        struct
        {
            gtUInt16 m_versionMinor;
            gtUInt16 m_versionMajor;
        };
        gtUInt32  m_versionValue;
    };
    gtUInt64 m_processId;
    gtUInt32 m_modInfoOffset;
    gtUInt32 m_modItemCount;
    gtUInt32 m_callEdgeCount;
};
#if AMDT_CPP_COMPILER == AMDT_VISUAL_CPP_COMPILER
    #pragma warning(pop)
#endif

//
// General Description
// The CSS (Call-Stack Sampling) file format is required for saving the CSS related data recorded in the CPU profiling session.
// This data is first gathered into different objects (described in details in the Call Graph Traversal ADD), and then these objects
// are serialized and written into the CSS file.
// Without the CSS file, the CSS data would need to be recomputed from the Profiling Database (the recorded profiling raw-data) -
// an operation that usually takes a significant amount of time, in the better case, and it may even be lost, in the worst case.
//
//
// 1. Overall Layout
// +--------------------+
// | Header             |
// +--------------------+
// | Call-Stacks Array  |
// +--------------------+
// | Leaf Nodes Array   |
// +--------------------+
// | Call-Sites Array   |
// +--------------------+
// | Functions Array    |
// +--------------------+
// | Symbols Array      |
// +--------------------+
// | Source Files Array |
// +--------------------+
// | Modules Array      |
// +--------------------+
//
//
// 2. Header
//
// *** Header Record Format ***
// +--------------------+----------+--------------------------------------------------------------------+
// | Field              | Type     | Description                                                        |
// +====================+==========+====================================================================+
// | Signature          | gtUInt32 | FourCC identifying the file type [Must be FCC('CACS')]             |
// +--------------------+----------+--------------------------------------------------------------------+
// | Version.Minor      | gtUInt16 | File format minor version number [Must be 1]                       |
// +--------------------+----------+--------------------------------------------------------------------+
// | Version.Major      | gtUInt16 | File format major version number [Must be 2]                       |
// +--------------------+----------+--------------------------------------------------------------------+
// | ProcessId          | gtUInt64 | The OS's Process ID of the sampled process                         |
// +--------------------+----------+--------------------------------------------------------------------+
// | ModulesArrayOffset | gtUInt32 | Offset, in bytes, into the file, of the Modules Array              |
// +--------------------+----------+--------------------------------------------------------------------+
// | ModulesCount       | gtUInt32 | The number of modules in the Modules Array                         |
// +--------------------+----------+--------------------------------------------------------------------+
// | CallStacksCount    | gtUInt32 | The number of call-stacks in the Call-Stacks Array                 |
// +--------------------+----------+--------------------------------------------------------------------+
// |                    | gtUInt32 | Reserved                                                           |
// +--------------------+----------+--------------------------------------------------------------------+
// | CharacterSetSize   | gtUInt64 | The size, in bytes, of the Character Set used (char, wchar_t, ...) |
// +--------------------+----------+--------------------------------------------------------------------+
//
//
// 3. Call-Stacks Array
// Each Call-Stack is composed of an ordered list of call-sites (not including the leaf nodes), a list of leaf nodes, and a list of
// sampled events associated with it.
//
// *** CallStack Record Format ***
// +------------------------+--------------------+----------------------------------------------+
// | Field                  | Type               | Description                                  |
// +========================+====================+==============================================+
// | ID                     | gtUInt64           | Unique Identification number                 |
// +------------------------+--------------------+----------------------------------------------+
// | CountSites             | gtUInt64           | Number of call-sites                         |
// +------------------------+--------------------+----------------------------------------------+
// | CallSites[CountSites]  | CallStack_CallSite | The call-sites composing this call-stack     |
// +------------------------+--------------------+----------------------------------------------+
// | CountEvents            | gtUInt64           | Number of event types                        |
// +------------------------+--------------------+----------------------------------------------+
// | Events[CountEvents]    | CallStack_Event    | The sampled events for this call-stack       |
// +------------------------+--------------------+----------------------------------------------+
// | SelfTicks              | gtUInt64           | Self ticks                                   |
// +------------------------+--------------------+----------------------------------------------+
// | TimesObserved          | gtUInt64           | Number of times this call-stack was observed |
// +------------------------+--------------------+----------------------------------------------+
// | CountLeaves            | gtUInt64           | Number of Leaf Nodes                         |
// +------------------------+--------------------+----------------------------------------------+
// | LeafNodes[CountLeaves] | CallStack_LeafNode | The Leaf Nodes composing this call-stack     |
// +------------------------+--------------------+----------------------------------------------+
//
// *** CallStack_CallSite Record Format ***
// +----------------+----------+----------------------------------------------------------------------------------------------+
// | Field          | Type     | Description                                                                                  |
// +================+==========+==============================================================================================+
// | ModuleBaseAddr | gtUInt64 | The base virtual address of the module in which the parent function of the call-site resides |
// +----------------+----------+----------------------------------------------------------------------------------------------+
// | Address        | gtUInt64 | The call-site's traversed address                                                            |
// +----------------+----------+----------------------------------------------------------------------------------------------+
// | ID             | gtUInt64 | Unique Identification number                                                                 |
// +----------------+----------+----------------------------------------------------------------------------------------------+
//
// *** CallStack_Event Record Format ***
// +-------+----------+--------------------------------------------+
// | Field | Type     | Description                                |
// +=======+==========+============================================+
// | ID    | gtUInt64 | Unique Identification number               |
// +-------+----------+--------------------------------------------+
// | Count | gtUInt64 | The number of times this event was sampled |
// +-------+----------+--------------------------------------------+
//
// *** CallStack_LeafNode Record Format ***
// +-------+----------+------------------------------+
// | Field | Type     | Description                  |
// +=======+==========+==============================+
// | ID    | gtUInt64 | Unique Identification number |
// +-------+----------+------------------------------+
//
//
// 4. Leaf Nodes Array
// A Leaf Node is a call-site that was sampled while in the profiling session recording. This call-site is the beginning (top) of
// a call-stack.
// The array is preceded by a gtUInt64 of the number of elements within.
//
// *** LeafNode Record Format ***
// +---------------------------+-------------------+----------------------------------------------------------------------------+
// | Field                     | Type              | Description                                                                |
// +===========================+===================+============================================================================+
// | ModuleBaseAddr            | gtUInt64          | The base virtual address of the module in which the parent function of the |
// |                           |                   | Leaf Node resides                                                          |
// +---------------------------+-------------------+----------------------------------------------------------------------------+
// | Address                   | gtUInt64          | The Leaf Node's traversed address                                          |
// +---------------------------+-------------------+----------------------------------------------------------------------------+
// | TimesObserved             | gtUInt64          | Number of times this call-stack was observed                               |
// +---------------------------+-------------------+----------------------------------------------------------------------------+
// | ID                        | gtUInt64          | Unique Identification number                                               |
// +---------------------------+-------------------+----------------------------------------------------------------------------+
// | FunctionID                | gtUInt64          | Parent function ID                                                         |
// +---------------------------+-------------------+----------------------------------------------------------------------------+
// | SourceLine                | gtUInt64          | Associated source file's line number                                       |
// +---------------------------+-------------------+----------------------------------------------------------------------------+
// | SourceFileID              | gtUInt64          | Owning source file ID                                                      |
// +---------------------------+-------------------+----------------------------------------------------------------------------+
// | CountStacks               | gtUInt64          | Number of call-stacks                                                      |
// +---------------------------+-------------------+----------------------------------------------------------------------------+
// | Stacks[CountStacks]       | LeafNode_Stack    | The call-stacks associated with this Leaf Node                             |
// +---------------------------+-------------------+----------------------------------------------------------------------------+
// | CountEvents               | gtUInt64          | Number of event types                                                      |
// +---------------------------+-------------------+----------------------------------------------------------------------------+
// | Events[CountEvents]       | LeafNode_Event    | The sampled events for this Leaf Node                                      |
// +---------------------------+-------------------+----------------------------------------------------------------------------+
// | CountEvThreads            | gtUInt64          | Number of threads sampled                                                  |
// +---------------------------+-------------------+----------------------------------------------------------------------------+
// | EvThreads[CountEvThreads] | LeafNode_EvThread | The thread specific sampling information                                   |
// +---------------------------+-------------------+----------------------------------------------------------------------------+
// | CountEvStacks             | gtUInt64          | Number of aggregated call-stacks                                           |
// +---------------------------+-------------------+----------------------------------------------------------------------------+
// | EvStacks[CountEvStacks]   | LeafNode_EvStack  | The call-stacks by aggregated by event type                                |
// +---------------------------+-------------------+----------------------------------------------------------------------------+
//
// *** LeafNode_CallStack Record Format ***
// +-------+----------+------------------------------+
// | Field | Type     | Description                  |
// +=======+==========+==============================+
// | ID    | gtUInt64 | Unique Identification number |
// +-------+----------+------------------------------+
//
// *** LeafNode_Event Record Format ***
// +-------+----------+--------------------------------------------+
// | Field | Type     | Description                                |
// +=======+==========+============================================+
// | ID    | gtUInt64 | Unique Identification number               |
// +-------+----------+--------------------------------------------+
// | Count | gtUInt64 | The number of times this event was sampled |
// +-------+----------+--------------------------------------------+
//
// *** LeafNode_EvThread Record Format ***
// +----------+----------+-------------------------------------------+
// | Field    | Type     | Description                               |
// +==========+==========+===========================================+
// | EventID  | gtUInt64 | Event Identification number               |
// +----------+----------+-------------------------------------------+
// | Count    | gtUInt64 | The sampled event's count for this thread |
// +----------+----------+-------------------------------------------+
// | ThreadID | gtUInt64 | The OS's Thread ID                        |
// +----------+----------+-------------------------------------------+
//
// *** LeafNode_EvStack Record Format ***
// +---------+----------+-------------------------------------------+
// | Field   | Type     | Description                               |
// +=========+==========+===========================================+
// | StackID | gtUInt64 | Call-stack Identification number          |
// +---------+----------+-------------------------------------------+
// | EventID | gtUInt64 | Event Identification number               |
// +---------+----------+-------------------------------------------+
// | Count   | gtUInt64 | The sampled event's count for this thread |
// +---------+----------+-------------------------------------------+
//
//
// 5. Call-Sites Array
// A Call-Site is essentially a traversed address within a call-stack. The call-site may be associated with more than one call-stack.
// The array is preceded by a gtUInt64 of the number of elements within.
//
// *** CallSite Record Format ***
// +---------------------+----------------+--------------------------------------------------------------------------------------+
// | Field               | Type           | Description                                                                          |
// +=====================+================+======================================================================================+
// | ModuleBaseAddr      | gtUInt64       | The base virtual address of the module in which the parent function of the call-site |
// |                     |                | resides                                                                              |
// +---------------------+----------------+--------------------------------------------------------------------------------------+
// | Address             | gtUInt64       | The call-site's traversed address                                                    |
// +---------------------+----------------+--------------------------------------------------------------------------------------+
// | ID                  | gtUInt64       | Unique Identification number                                                         |
// +---------------------+----------------+--------------------------------------------------------------------------------------+
// | TimesObserved       | gtUInt64       | Number of times this call-stack was observed                                         |
// +---------------------+----------------+--------------------------------------------------------------------------------------+
// | FunctionID          | gtUInt64       | Parent function ID                                                                   |
// +---------------------+----------------+--------------------------------------------------------------------------------------+
// | SourceLine          | gtUInt64       | Associated source file's line number                                                 |
// +---------------------+----------------+--------------------------------------------------------------------------------------+
// | SourceFileID        | gtUInt64       | Owning source file ID                                                                |
// +---------------------+----------------+--------------------------------------------------------------------------------------+
// | CountStacks         | gtUInt64       | Number of call-stacks                                                                |
// +---------------------+----------------+--------------------------------------------------------------------------------------+
// | Stacks[CountStacks] | CallSite_Stack | The call-stacks associated with this call-site                                       |
// +---------------------+----------------+--------------------------------------------------------------------------------------+
//
// *** CallSite_CallStack Record Format ***
// +-------+----------+------------------------------+
// | Field | Type     | Description                  |
// +=======+==========+==============================+
// | ID    | gtUInt64 | Unique Identification number |
// +-------+----------+------------------------------+
//
//
// 6. Functions Array
// This array lists the sampled functions information.
// The array is preceded by a gtUInt64 of the number of elements within.
//
// *** Function Record Format ***
// +----------------+----------+----------------------------------------------------------------------+
// | Field          | Type     | Description                                                          |
// +================+==========+======================================================================+
// | ModuleBaseAddr | gtUInt64 | The base virtual address of the module in which the function resides |
// +----------------+----------+----------------------------------------------------------------------+
// | Address        | gtUInt64 | The function's starting address                                      |
// +----------------+----------+----------------------------------------------------------------------+
// | ID             | gtUInt64 | Unique Identification number                                         |
// +----------------+----------+----------------------------------------------------------------------+
// | SourceLine     | gtUInt64 | Associated source file's line number                                 |
// +----------------+----------+----------------------------------------------------------------------+
// | SourceFileID   | gtUInt64 | Owning source file ID                                                |
// +----------------+----------+----------------------------------------------------------------------+
// | SymbolID       | gtUInt64 | Matching symbols ID                                                  |
// +----------------+----------+----------------------------------------------------------------------+
//
//
// 7. Symbols Array
// This array lists the symbols information of the matching the sampled function.
// The array is preceded by a gtUInt64 of the number of elements within.
//
// *** Symbol Record Format ***
// +-------+--------------------+---------------------------------------------------------------------------------------+
// | Field | Type               | Description                                                                           |
// +=======+====================+=======================================================================================+
// | ID    | gtUInt64           | Unique Identification number                                                          |
// +-------+--------------------+---------------------------------------------------------------------------------------+
// | Size  | gtUInt64           | Size, in bytes, of the following Name                                                 |
// +-------+--------------------+---------------------------------------------------------------------------------------+
// | Name  | Character-Set Type | The symbol's name (zero terminated, and should be converted to the Character Set type |
// |       |                    | declared in the file's Header record)                                                 |
// +-------+--------------------+---------------------------------------------------------------------------------------+
//
//
// 8. Source Files Array
// This array lists the source files' information in which the sampled functions reside.
// The array is preceded by a gtUInt64 of the number of elements within.
//
// *** SourceFile Record Format ***
// +-------+--------------------+--------------------------------------------------------------------------------------------+
// | Field | Type               | Description                                                                                |
// +=======+====================+============================================================================================+
// | ID    | gtUInt64           | Unique Identification number                                                               |
// +-------+--------------------+--------------------------------------------------------------------------------------------+
// | Size  | gtUInt64           | Size, in bytes, of the following Name                                                      |
// +-------+--------------------+--------------------------------------------------------------------------------------------+
// | Name  | Character-Set Type | The source file's name (zero terminated, and should be converted to the Character Set type |
// |       |                    | declared in the file's Header record)                                                      |
// +-------+--------------------+--------------------------------------------------------------------------------------------+
//
//
// 9. Modules Array
// This array lists the sampled modules information.
//
// *** Module Record Format ***
// +----------------+--------------------+---------------------------------------------------------------------------------------+
// | Field          | Type               | Description                                                                           |
// +================+====================+=======================================================================================+
// | ModuleBaseAddr | gtUInt64           | The module's base virtual address                                                     |
// +----------------+--------------------+---------------------------------------------------------------------------------------+
// | Size           | gtUInt64           | Size, in bytes, of the following Name                                                 |
// +----------------+--------------------+---------------------------------------------------------------------------------------+
// | Name           | Character-Set Type | The module's file name (zero terminated, and should be converted to the Character Set |
// |                |                    | type declared in the file's Header record)                                            |
// +----------------+--------------------+---------------------------------------------------------------------------------------+
//

#endif // _CSSFILE_H_
