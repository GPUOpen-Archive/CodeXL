//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscCoreInternalUtils.h
///
//==================================================================================

#ifndef vscCoreInternalUtils_h__
#define vscCoreInternalUtils_h__
#include <AMDTBaseTools/Include/gtString.h>
#include <wchar.h>

// Utilities to be used from within VSC code only.

// Allocates a new wchar_t buffer and copies the input string's content to the buffer.
wchar_t* vscAllocateAndCopy(const gtString& str);

char* vscAllocateAndCopy(const std::string& str);

std::string vscWstrToStr(std::wstring const& text);


#endif // vscCoreInternalUtils_h__
