//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpGlobals.cpp
///
//==================================================================================

//FIXME - also defined in tpInternalDataTypes.h
#define INITGUID  // Include this #define to use SystemTraceControlGuid/EventTraceGuid  in Evntrace.h.

// System headers
#include <windows.h>
#include <evntrace.h>
#include <evntcons.h>
#include <tdh.h>

// Project headers
#include <AMDTThreadProfileApi.h>
#include <tpInternalDataTypes.h>
#include <tpCollect.h>


//
// Globals
//

// GUIDs for NT kernel logger events
//  https://msdn.microsoft.com/en-us/library/windows/desktop/aa364085(v=vs.85).aspx
//  https://famellee.wordpress.com/2012/08/17/a-fight-with-etw-and-nt-kernel-logger/
//
// Type - https://msdn.microsoft.com/en-us/library/windows/desktop/dd765164(v=vs.85).aspx
// Trace Info - https://msdn.microsoft.com/en-us/library/windows/desktop/dd392329(v=vs.85).aspx
// StackWalk_Event class  https://msdn.microsoft.com/en-us/library/windows/desktop/dd392323(v=vs.85).aspx
//
// Note: All the kernel events use MOF to publish the format of the event data.
// MOF approach to consume trace data
//      https://msdn.microsoft.com/en-us/library/windows/desktop/aa364114(v=vs.85).aspx
// Trace Data Helper(TDH) approach
//      https://msdn.microsoft.com/en-us/library/windows/desktop/ee441328(v=vs.85).aspx
//      https://msdn.microsoft.com/en-us/library/windows/desktop/aa364115(v=vs.85).aspx
//

const GUID g_imageGuid = { 0x2cb15d1d, 0x5fc1, 0x11d2, { 0xab, 0xe1, 0x00, 0xa0, 0xc9, 0x11, 0xf5, 0x18 } };
const GUID g_processGuid = { 0x3d6fa8d0, 0xfe05, 0x11d0, { 0x9d, 0xda, 0x00, 0xc0, 0x4f, 0xd7, 0xba, 0x7c } };
const GUID g_threadGuid = { 0x3d6fa8d1, 0xfe05, 0x11d0, { 0x9d, 0xda, 0x00, 0xc0, 0x4f, 0xd7, 0xba, 0x7c } };
const GUID g_stackWalkGuid = { 0xdef2fe46, 0x7bd6, 0x4b80, { 0xbd, 0x94, 0xf5, 0x7f, 0xe2, 0x0d, 0x0c, 0xe3 } };
const GUID g_perfInfoGuid = { 0xce1dbfb4, 0x137e, 0x4da6, { 0x87, 0xb0, 0x3f, 0x59, 0xaa, 0x10, 0x2c, 0xbc } };

//const wchar_t *g_pLogFilePath = L"c:\\temp\\test2.etl";

// ThreadProfileSession     g_tpSession;
