//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file RunInfo.cpp
///
//==================================================================================

#include "RunInfoWriter.h"
#include "RunInfoReader.h"

HRESULT fnWriteRIFile(const wchar_t* pRIFilePath, const RunInfo* pRIInfo)
{
    HRESULT hr;

    if (pRIFilePath == NULL || pRIInfo == NULL)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        RunInfoWriter riWriter;
        hr = riWriter.Write(pRIFilePath, pRIInfo);
    }

    return hr;
}

HRESULT fnReadRIFile(const wchar_t* pRIFilePath, RunInfo* pRIInfo)
{
    HRESULT hr;

    if (pRIFilePath == NULL || pRIInfo == NULL)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        RunInfoReader riReader;
        hr = riReader.Read(pRIFilePath);

        if (hr == S_OK)
        {
            hr = riReader.GetRunInfoData(pRIInfo);
        }
    }

    return hr;
}
