//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpTranslateCB.h
///
//==================================================================================

#ifndef _TPTRANSLATECB_H_
#define _TPTRANSLATECB_H_

#define INITGUID  // Include this #define to use SystemTraceControlGuid/EventTraceGuid  in Evntrace.h.

// System Headers
#include <Windows.h>
#include <evntrace.h>
#include <evntcons.h>
#include <tdh.h>

// Base headers
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtList.h>

// OS Wrappers headers
#include <AMDTOSWrappers/Include/osProcess.h>

// Project Headers
#include <tpInternalDataTypes.h>
#include <tpTranslateDataTypes.h>
#include <AMDTThreadProfileDataTypes.h>

//
//  Macros
//

#define TP_TRACE_EVENT_INFO_MAXSIZE         4096
#define TP_EVENT_PROPERTY_BUFFER_MAXSIZE    4096

//
// Callbacks
//

ULONG WINAPI EtwProcessBufferCB(PEVENT_TRACE_LOGFILE pBuffer);
void WINAPI EtwProcessEventCB(PEVENT_TRACE pEventTrace);
void WINAPI EtwProcessEventRecordCB(PEVENT_RECORD pEventRecord);
void WINAPI EtwProcessMetaEventRecordsCB(PEVENT_RECORD pEventRecord);
void WINAPI EtwProcessCSwitchEventRecordsCB(PEVENT_RECORD pEventRecord);

//
// Helper functions
//

AMDTResult GetTdhTraceEventInfo(PEVENT_RECORD pEventRecord, PTRACE_EVENT_INFO* ppTraceEventInfo);

AMDTResult ProcessTraceEventRecordMOF(PEVENT_RECORD pEventRecord,
                                      PTRACE_EVENT_INFO pTraceEventInfo);
AMDTResult ProcessMetaTraceEventRecordsMOF(PEVENT_RECORD pEventRecord,
                                           PTRACE_EVENT_INFO pTraceEventInfo);
AMDTResult ProcessCSwitchTraceEventRecordsMOF(PEVENT_RECORD pEventRecord,
                                              PTRACE_EVENT_INFO pTraceEventInfo);

AMDTResult SettranslateHandle(AMDTThreadProfileDataHandle dataHhandle,
                              PTRACE_EVENT_INFO pTraceEventInfo,
                              PBYTE pPropertyBuffer);

AMDTResult GetTraceEventInfo(PEVENT_RECORD pEventRecord, PTRACE_EVENT_INFO pTraceEventInfo, ThreadProfileEventInfo& info);
void GetTraceEventType(GUID guid, UCHAR opcode, ThreadProfileEvent& event, ThreadProfileEventType& eventType);
char* GetTraceEventSring(ThreadProfileEvent event);
char* GetTraceEventTypeSring(ThreadProfileEventType event);

AMDTResult IsMetaEvent(PEVENT_RECORD pEventRecord);
AMDTResult IsCSwitchEvent(PEVENT_RECORD pEventRecord);

int ProcessInfoRecord(PEVENT_RECORD pEventRecord,
                      PTRACE_EVENT_INFO pTraceEventInfo,
                      ThreadProfileEventGeneric& genericRec);

AMDTResult ProcessInfoRecordData(PEVENT_RECORD pEvent,
                                 PTRACE_EVENT_INFO pInfo,
                                 USHORT i,
                                 LPWSTR pStructureName,
                                 USHORT StructIndex,
                                 ThreadProfileEventGeneric& genericRec);

int ProcessProcessRecord(PEVENT_RECORD pEventRecord,
                         PTRACE_EVENT_INFO pTraceEventInfo,
                         ThreadProfileEventProcess& procesRec);

AMDTResult ProcessProcessRecordData(PEVENT_RECORD pEvent,
                                    PTRACE_EVENT_INFO pInfo,
                                    USHORT i,
                                    LPWSTR pStructureName,
                                    USHORT StructIndex,
                                    ThreadProfileEventProcess& processRec);

int ProcessImageRecord(PEVENT_RECORD pEventRecord,
                       PTRACE_EVENT_INFO pTraceEventInfo,
                       ThreadProfileEventImage& procesRec);

AMDTResult ProcessImageRecordData(PEVENT_RECORD pEvent,
                                  PTRACE_EVENT_INFO pInfo,
                                  USHORT i,
                                  LPWSTR pStructureName,
                                  USHORT StructIndex,
                                  ThreadProfileEventImage& imageRec);

int ProcessThreadRecord(PEVENT_RECORD pEventRecord,
                        PTRACE_EVENT_INFO pTraceEventInfo,
                        ThreadProfileEventThread& threadRec);

AMDTResult ProcessThreadRecordData(PEVENT_RECORD pEvent,
                                   PTRACE_EVENT_INFO pInfo,
                                   USHORT i,
                                   LPWSTR pStructureName,
                                   USHORT StructIndex,
                                   ThreadProfileEventThread& threadRec);

int ProcessCSwitchRecord(PEVENT_RECORD pEventRecord,
                         PTRACE_EVENT_INFO pTraceEventInfo,
                         ThreadProfileEventCSwitch& csRec);

AMDTResult ProcessCSwitchRecordData(PEVENT_RECORD pEvent,
                                    PTRACE_EVENT_INFO pInfo,
                                    USHORT i,
                                    LPWSTR pStructureName,
                                    USHORT StructIndex,
                                    ThreadProfileEventCSwitch& csRec);

int ProcessStackRecord(PEVENT_RECORD pEventRecord,
                       PTRACE_EVENT_INFO pTraceEventInfo,
                       ThreadProfileEventStack& csRec);

AMDTResult ProcessStackRecordData(PEVENT_RECORD pEvent,
                                  PTRACE_EVENT_INFO pInfo,
                                  USHORT i,
                                  LPWSTR pStructureName,
                                  USHORT StructIndex,
                                  ThreadProfileEventStack& stackRec);

// Helper functions on top of TDH interfaces
DWORD GetTdhProperty(PEVENT_RECORD pEvent,
                     DWORD descriptorCount,
                     PPROPERTY_DATA_DESCRIPTOR pDataDescriptor,
                     PDWORD pPropertySize,
                     PBYTE*  ppData);

DWORD GetTdhPropertyData(PEVENT_RECORD pEvent,
                         PTRACE_EVENT_INFO pInfo,
                         USHORT i,
                         PROPERTY_DATA_DESCRIPTOR* pDataDescriptor,
                         PDWORD pPropertySize,
                         PBYTE* ppData,
                         PUSHORT pArraySize);

DWORD GetTdhMapInfo(PEVENT_RECORD pEvent, LPWSTR pMapName, DWORD DecodingSource, PEVENT_MAP_INFO& pMapInfo);
void RemoveTrailingSpace(PEVENT_MAP_INFO pMapInfo);


// Helper functions to retrieve the data from MOF record
DWORD GetUnicodeString(USHORT InType, PBYTE pData, size_t& StringLength, LPWSTR& pString);
DWORD GetAnsiString(USHORT InType, PBYTE pData, size_t& StringLength, LPSTR& pString);
INT8 GetInt8(USHORT InType, PBYTE pData);
UINT8 GetUInt8(USHORT InType, PBYTE pData);
LONG GetInt32(USHORT InType, PBYTE pData);
ULONG GetUInt32(USHORT InType, PBYTE pData);
ULONGLONG GetUInt64(USHORT InType, PBYTE pData);
ULONGLONG GetPointer(USHORT InType, PBYTE pData);
ULONG GetPUlong(USHORT InType, PBYTE pData);
ULONGLONG GetPUlonglong(USHORT InType, PBYTE pData);
DWORD GetWbemsid(USHORT InType, PBYTE pData, DWORD dataSize);


// Print routines
void PrintInitialize();
AMDTResult PrintTraceEventRecordMOF(PEVENT_RECORD pEventRecord,
                                    PTRACE_EVENT_INFO pTraceEventInfo);

int PrintEventTraceData(PEVENT_RECORD pEvent,
                        PTRACE_EVENT_INFO pInfo,
                        USHORT i,
                        LPWSTR pStructureName,
                        USHORT StructIndex);

AMDTResult GetEventString(GUID guid, char* pEventString);
AMDTResult GetEventTypeString(int eventType, char* pEventString);

char* GetTraceEventSring(ThreadProfileEvent event);
char* GetTraceEventTypeSring(ThreadProfileEventType event);

DWORD FormatAndPrintData(PEVENT_RECORD pEvent,
                         USHORT InType,
                         USHORT OutType,
                         PBYTE pData,
                         DWORD DataSize,
                         PEVENT_MAP_INFO pMapInfo);

void PrintMapString(PEVENT_MAP_INFO pMapInfo, PBYTE pData);

#endif //_TPTRANSLATECB_H_