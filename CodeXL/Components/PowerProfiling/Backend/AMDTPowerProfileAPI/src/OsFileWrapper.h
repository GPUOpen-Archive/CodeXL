//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file OsFileWrapper.h
///
//==================================================================================

#ifndef _OS_FILE_WRAPPER_H_
#define _OS_FILE_WRAPPER_H_

#if ( defined (_WIN32) || defined (_WIN64) )
    #include <Windows.h>
#endif
#include <stdio.h>

bool PwrOpenFile(FILE** m_hFile, const char* pFileName, const char* pMode);
bool PwrReadFile(FILE* m_hFile, AMDTUInt8* pData, AMDTInt32 len);
bool PwrSeekFile(FILE* m_hFile, long size, AMDTInt32 mode);
bool PwrCloseFile(FILE* m_hFile);
bool PwrWriteFile(FILE* m_hFile, AMDTUInt8* pData, AMDTInt32 len);
AMDTUInt64 GetOsTimeStamp(void);

#endif //_OS_FILE_WRAPPER_H_