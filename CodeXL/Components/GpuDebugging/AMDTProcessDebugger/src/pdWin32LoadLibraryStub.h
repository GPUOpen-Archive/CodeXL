//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdWin32LoadLibraryStub.h
///
//==================================================================================

//------------------------------ pdWin32LoadLibraryStub.h ------------------------------

#ifndef __PDWIN32LOADLIBRARYSTUB
#define __PDWIN32LOADLIBRARYSTUB

// Win32:
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

// Set the compiler packing alignment to 1 byte:
// (I.E: Do not align this struct).
#pragma pack( push, 1 )

// ----------------------------------------------------------------------------------
// Struct Name:          pdWin32LoadLibraryStub
//
// General Description:
//   An assembly code that:
//   a. Calls LoadLibrary on a given DLL path.
//   b. Triggers a breakpoint event.
//
//   It is packed as a struct for convenience reasons.
//
// Author:               Yaki Tebeka
// Creation Date:        27/11/2003
// ----------------------------------------------------------------------------------
struct pdWin32LoadLibraryStub
{
public:
    pdWin32LoadLibraryStub();
    void initialize(const char* dllPath, DWORD myAddress);

public:
    BYTE    _PUSH_instr;             // Will contain a push instruction.
    DWORD   _PUSH_argument;          // Will contain the push argument - a pointer to
    // the given dll path.

    BYTE    _MOV_EAX_instr;          // Will cotain a move EAX instruction.
    FARPROC _MOV_EAX_argument;       // Will contain the move EAX argument - a pointer to.
    // the KERNAL32.dll LoadLibrary function.

    WORD    _CALL_EAX_instr;         // Will contain a call EAX instruction - A call to
    // the KERNAL32.dll LoadLibrary function.

    BYTE    _INT_3_instr;            // Will contains a breakpoint event instruction.

    char    _data_DllPath[MAX_PATH]; // Will contains the given DLL path.
};

// Unset the compiler packing alignment:
#pragma pack( pop )

#endif  // __PDWIN32LOADLIBRARYSTUB
