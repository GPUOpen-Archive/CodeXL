//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdWin32LoadLibraryStub.cpp
///
//==================================================================================

//------------------------------ pdWin32LoadLibraryStub.cpp ------------------------------

// C:
#include <stddef.h>

// Local:
#include <src/pdWin32LoadLibraryStub.h>


// ---------------------------------------------------------------------------
// Name:        pdWin32LoadLibraryStub::pdWin32LoadLibraryStub
// Description: Constructor
// Author:      Yaki Tebeka
// Date:        27/11/2003
// Implementation Notes:
//   Fills the assembly instruction codes.
// ---------------------------------------------------------------------------
pdWin32LoadLibraryStub::pdWin32LoadLibraryStub()
    : _PUSH_instr(0x68),
      _MOV_EAX_instr(0xB8),
      _CALL_EAX_instr(0xD0FF),
      _INT_3_instr(0xCC)
{
}


// ---------------------------------------------------------------------------
// Name:        pdWin32LoadLibraryStub::initialize
// Description: Fills the missing data in this assembly code.
//
// Arguments:   dllPath - The path of the DLL to be loaded.
//              myAddress - The address in which this code will reside when executed.
//                          (In the executed process address space)
//
// Author:      Yaki Tebeka
// Date:        27/11/2003
//
// Implementation Notes:
//   a. Fills the DLL path
//   b. Fills the assembly instructions arguments.
// ---------------------------------------------------------------------------
void pdWin32LoadLibraryStub::initialize(const char* dllPath, DWORD myAddress)
{
    // Fill the dll path:
    strcpy_s(_data_DllPath, MAX_PATH, dllPath);

    // Fill the PUSH argument (the address of _data_DllPath):
    _PUSH_argument = myAddress + offsetof(pdWin32LoadLibraryStub, _data_DllPath);

    // Fill the move EAX argument:
    _MOV_EAX_argument = GetProcAddress(GetModuleHandle(L"KERNEL32.DLL"), "LoadLibraryW");
}
