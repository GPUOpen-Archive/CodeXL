//=====================================================================
// Copyright 2014-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gtErrorString.cpp
/// \brief Implements a function that converts error codes to error description strings
///
//=====================================================================

// Local:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

const wchar_t* gtGetErrorString(const HRESULT errCode)
{
    switch (errCode)
    {
        case S_OK:
            return L"No error";

        case S_FALSE:
            return L"No Error";

        case E_FAIL:
            return L"Operation failed";

        case E_INVALIDARG:
            return L"One or more arguments are invalid";

        case E_OUTOFMEMORY:
            return L"Ran out of memory";

        case E_UNEXPECTED:
            return L"Unexpected error";

        case E_ACCESSDENIED:
            return L"Access denied";

        case E_HANDLE:
            return L"Invalid handle";

        case E_ABORT:
            return L"Operation aborted";

        case E_NOTIMPL:
            return L"Not implemented";

        case E_NOFILE:
            return L"File not found";

        case E_INVALIDPATH:
            return L"Path not found";

        case E_INVALIDDATA:
            return L"Invalid data";

        case E_NOTAVAILABLE:
            return L"Resource not available";

        case E_NODATA:
            return L"No data";

        case E_LOCKED:
            return L"Lock violation";

        case E_TIMEOUT:
            return L"Timeout";

        case E_PENDING:
            return L"The data necessary to complete this operation is not yet available.";

        case E_NOTSUPPORTED:
            return L"This operation is not supported.";

        default:
            return L"Unknown error.";

    }
}
