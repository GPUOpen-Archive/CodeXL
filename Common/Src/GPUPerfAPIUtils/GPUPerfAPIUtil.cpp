//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Defines the entrypoints to the GPUPerfAPIUtil library.
//==============================================================================

#include "GPUPerfAPI.h"
#include "GPUPerfAPIUtil.h"

#ifdef _WIN32
    #include <windows.h>
    #include <atlbase.h>

    #pragma warning( disable : 4996 )
#endif

#ifdef _LINUX
    #include <string.h>
    typedef int errno_t;

    extern errno_t fopen_s(FILE** pFile, const char* pFilename, const char* pMode);

#endif // _LINUX

GPA_Status GPA_EnableCountersFromFile(GPUPerfAPILoader* pLoader, const char* pFileName, gpa_uint32* pCountersRead, const char** ppErrorText, const char* pActiveSectionLabel)
{
    *ppErrorText = NULL;

    if (!pLoader || !pFileName || !pCountersRead)
    {
        *ppErrorText = "null pointer";
        return GPA_STATUS_ERROR_NULL_POINTER;
    }

    if (!pLoader->Loaded())
    {
        *ppErrorText = "loader not loaded";
        return GPA_STATUS_ERROR_FAILED;
    }

    (*pCountersRead) = 0;

    FILE* pFile;
    fopen_s(&pFile, pFileName, "r");

    if (!pFile)
    {
        *ppErrorText = "could not open file for reading";
        return GPA_STATUS_ERROR_NOT_FOUND;
    }

    bool usingSectionLabel;

    if (pActiveSectionLabel && strlen(pActiveSectionLabel))
    {
        usingSectionLabel = true;
    }
    else
    {
        usingSectionLabel = false;
    }

    char readBuf[2048];
    int scanResult = fscanf(pFile, "%s", readBuf);

    bool skippingSection = false;

    while (scanResult > 0)
    {
        if (usingSectionLabel && readBuf[0] == '[')
        {
            // handle [label] section
            char* closingBracket = strchr(readBuf, ']');

            if (closingBracket)
            {
                if (strncmp(readBuf + 1, pActiveSectionLabel, strlen(pActiveSectionLabel)) == 0)
                {
                    // matches, so read next string
                    fgets(readBuf, sizeof(readBuf), pFile);
                    scanResult = fscanf(pFile, "%s", &readBuf);
                    continue;
                }
                else
                {
                    // skip all text until next []
                    skippingSection = true;
                }
            }
            else
            {
                *ppErrorText = "did not find closing ] in a section label";
                fclose(pFile);
                return GPA_STATUS_ERROR_FAILED;
            }
        }

        if (readBuf[0] == ';' || skippingSection)
        {
            fgets(readBuf, sizeof(readBuf), pFile);
            scanResult = fscanf(pFile, "%s", &readBuf);
            continue;
        }

        // read a valid counter name, enable it

        (*pCountersRead)++;
        GPA_Status s = pLoader->GPA_EnableCounterStr(readBuf);

        if (s != GPA_STATUS_OK)
        {
            *ppErrorText = "could not enable counter";
            fclose(pFile);
            return GPA_STATUS_ERROR_FAILED;
        }

        scanResult = fscanf(pFile, "%s", &readBuf);
    }

    fclose(pFile);

    return GPA_STATUS_OK;
}
