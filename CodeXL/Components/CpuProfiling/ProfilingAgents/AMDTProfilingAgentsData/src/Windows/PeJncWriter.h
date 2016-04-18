//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PeJncWriter.h
/// \brief This file contains an interface to write JIT Native Code file in PE format.
///
//==================================================================================

#ifndef _PEJNCWRITER_H_
#define _PEJNCWRITER_H_

#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <ProfilingAgents/Utils/CrtFile.h>

#define ADDRESSALIGNMENT    0x100
#define SIZEOFALLHEADERS    0x300
#define FILEPAGESIZE        0x100

// Update this if format is changed.
#define MAJORIMAGEVERSION   0x3
#define MAJORIMAGEVERSION_WITH_INLINE   0x4

enum JAVASRC_SECTIONS
{
    PC2BC = 0xAA,
    METHOD_TABLE = 0xBB,
    STRING_TABLE = 0xCC,
    JAVASRC_SECTION_COUNT = 3
};

struct JavaSrcLineInfo
{
    unsigned int addrOffset;
    unsigned int lineNum;
};

class PeJncWriter
{
public:
    PeJncWriter();
    ~PeJncWriter();

    void SetJNCFileName(const wchar_t* pFileName);
    void SetJITFuncName(const wchar_t* pClassName, const wchar_t* pFuncName, const wchar_t* pJavaSourceFile = NULL);
    void SetJITStartAddr(gtUInt64 addr);

    // Write JIT Native Code for Java Source Line Info.
    bool WriteJITNativeCode(const gtUByte* pJITNativeCode, unsigned int size,
                            JavaSrcLineInfo* pSrcInfo = NULL, unsigned int srcEntryCnt = 0);

    bool WriteJITNativeCode(const gtUByte* pJITNativeCode,
                            unsigned int   size,
                            const void*    pc2bc_blob,
                            unsigned int   pc2bc_blob_size,
                            void*          methodtableblob,
                            unsigned int   methodtableblobsize,
                            void*          stringtableblob,
                            unsigned int   stringtableblobsize);

    void SetJITModuleName(wchar_t* pModName);
    bool WriteCLRJITNativeCode(const gtUByte* pJITNativeCode, unsigned int size,
                               const gtUByte* pILCode, unsigned int ilSize, unsigned int ilOffsetToImageBase,
                               gtUInt32* pMap = NULL, gtUInt32 mapEntries = 0);

    void Close();

private:
    void WriteHeaders();
    void WriteMSDOSHeader();
    void WriteSectionHeaders();
    void WriteNTHeader64();
    void WriteNTHeader32();
    void WriteTextSection(const gtUByte* pJITNativeCode);
    void CalculateSectionSize(unsigned int additionalSize = 0);

    void WriteString(const wchar_t* pString);
    void WriteAlignmentPadding();

private:

    wchar_t m_jncFileName[OS_MAX_FNAME];
    wchar_t m_jncClassName[OS_MAX_FNAME];
    wchar_t m_jncFuncName[OS_MAX_FNAME];
    wchar_t m_javaSrcName[OS_MAX_FNAME];
    wchar_t m_clrModuleName[OS_MAX_FNAME];

    gtUInt32     m_ilNativeMapEntryCount;
    gtUInt32     m_ilSize;
    unsigned int m_codeSize;
    unsigned int m_dataSize;
    unsigned int m_javaSrcEntryCnt;
    gtUInt64     m_jitLoadAddr;
    unsigned int m_methodOffsetOfImage;

    // the following members will be used to calculate aligned size;
    unsigned int m_sizeOfAlignedCodeSect;
    unsigned int m_sizeOfAlignedJavaSrcSect;
    unsigned int m_sizeOfAlignedILSect;
    unsigned int m_sizeOfAlignedOffSetMap;

    CrtFile m_fileStream;
    unsigned short m_majorVersion;
};

#endif // _PEJNCWRITER_H_
