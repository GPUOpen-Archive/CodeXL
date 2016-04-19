//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file RawDataReader.h
///
//==================================================================================

#ifndef _RAW_DATA_READER_H_
#define _RAW_DATA_READER_H_
#ifndef LINUX
    #include <windows.h>
#endif

#include <AMDTRawDataFileHeader.h>
#include <AMDTDefinitions.h>


#include <stdlib.h>
#include <cstdio>
/****************************************************************************/

// SectionInfo
//
// This structure holds the section type and section data information
// Depending on the m_secType, m_pSecHdrData will point to various section data.
struct SectionInfo
{
    //ctionType m_secType;
    AMDTUInt64 m_secType;
    void* m_pSecHdrData;
};

// SectionTabData
//
// This structure holds the number of sections and section table
// All of the sections are optional.
struct SectionTabData
{
    AMDTUInt32 m_SecCnt;
    SectionInfo* m_pSecInfo;
};

// RawDataHandle
//
// An unified file handler to hold complete raw file information
struct RawDataHandle
{
    //File header
    RawFileHeader m_fileHdr;

    //For additional section table header. This field is valid only if m_sectionTabCnt > 1
    //SectionHrdTableInfo* m_pTableHdrInfo;

    //For each SectionHrdTableInfo there is header section
    //There could be more than one tables.
    SectionTabData* m_pTabInfo;

    //Raw Sample data
    void* m_pRawData;
};

class RawDataReader
{
public:
    RawDataReader();
    virtual ~RawDataReader();

    //PrepareRawBufferList
    AMDTResult PrepareRawBufferList(AMDTUInt16 coreCnt);

    //Close the raw file
    AMDTResult Close();

    // RawDataInitialize() will open the raw file and read the configuration records.
    AMDTResult RawDataInitialize(const wchar_t* pFileName, bool isOnline);

    //RawDataClose will close the raw file
    AMDTResult RawDataClose();

    // ReleaseBufferList
    AMDTResult ReleaseBufferList();

    // ResetBufferList buffer list
    AMDTResult ResetBufferList();

    //GetProfileCfgInfo
    AMDTResult GetProfileCfgInfo(ProfileConfigList* pPwrCfgTabInfo);

    //GetSampleSectionInfo
    AMDTResult GetSampleSectionInfo(SectionSampleInfo* pSampleInfo);

    //GetSessionDuration - Read the duration for which profile was run
    AMDTResult GetSessionDuration(AMDTUInt64* pDuration);

    //GetSessionTimeStamps - Read the start time and end session time
    AMDTResult GetSessionTimeStamps(AMDTUInt64* pStart, AMDTUInt64* pEnd);
    AMDTResult GetSessionProfileStartTimeStamp(AMDTUInt64* pStart) { return GetSessionTimeStamps(pStart, NULL); }

    AMDTUInt64 GetSessionProfileStartPerfCounter(void) { return m_rawDataHdl.m_fileHdr.m_startPerfCounter; }
    AMDTUInt64 GetSessionPerfFreq(void) { return m_rawDataHdl.m_fileHdr.m_perfFreq; }

    //GetRawFileNextChunkInfo-Read the chunk marker record and get the chunk information
    AMDTResult GetRawFileNextChunkInfo(void* pBaseAddress, AMDTUInt64* pChunkAddress, AMDTUInt64* pSize, AMDTUInt32* pRecCount);

    //GetRecordStartOffset- Get the offset of raw record data in the raw file
    AMDTResult GetRecordStartOffset(AMDTUInt32* pOffset) { *pOffset = m_rawDataHdl.m_fileHdr.m_rawDataOffset ; return AMDT_STATUS_OK;}

    //GetRawFileSize - get the sizeof raw records in bytes
    AMDTUInt32 GetRawFileSize() {return m_fileLen;}

    //GetTargetCoreCount- Get total number of cores & cus in target machine
    AMDTUInt16 GetTargetCoreCuCount() {return m_rawDataHdl.m_fileHdr.m_targetCoreCuCnt;}

    //GetTargetFamily
    AMDTUInt16 GetTargetFamily() { return (AMDTUInt16)m_rawDataHdl.m_fileHdr.m_family; }

    //GetTargetModel
    AMDTUInt16 GetTargetModel() { return (AMDTUInt16)m_rawDataHdl.m_fileHdr.m_model; }

    //CheckRawAttributeMask
    bool CheckRawAttributeMask(AMDTUInt16 attributeId, AMDTUInt64* pEventMask);

protected:
    RawDataHandle m_rawDataHdl;

private:
    //Read the file header section
    AMDTResult ReadFileHeader();

    //Read each sections
    AMDTResult ReadSections();

    //Reset the reader
    AMDTResult Reset();

    //Clear all file handle memory
    AMDTResult ReleaseFileHandle(RawDataHandle* pFileHld);

    //ReadFileChunk will read a chunk of data into a buffer
    AMDTResult ReadFileChunk(AMDTUInt8** pBuffer);

    RawFileHeader m_fileHdr;
    SectionHrdTableInfo* m_pHeaderTableInfo;
    FILE* m_hRawFile;
    AMDTUInt8* m_pBufferHeader;
    RawBufferInfo* m_pBufferList;
    AMDTUInt16 m_bufferCnt;
    AMDTUInt32 m_offset;
    AMDTUInt32 m_fileLen;
    AMDTUInt32 m_curr;
    AMDTUInt32 m_rec_off;

    AMDTUInt32 m_onlineOffset;
    AMDTUInt8* m_pOnlineBuffer;

    //Map file
    AMDTUInt8* m_mapAddress;

};

#endif //_RAW_DATA_READER_H_

