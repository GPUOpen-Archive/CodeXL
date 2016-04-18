//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PjsHeader.h
///
//==================================================================================

#ifndef _PJSHEADER_H_
#define _PJSHEADER_H_

#include <AMDTOSWrappers/Include/osOSDefinitions.h>

#define PREJITSYMBOLFILEVERSION 1
#define PJSFILESIGNATURE    "CAPREJIT"

//  Basically the PJS file contains a header and body sections.
//  The PJS file header contains:
//          File Signature CAPREJIT
//          Filer version (DWORD);
//          NumberOfRecords (DWORD);
//          ModuleLoadAddress (QWORD) -- Regardless for 32-bit/64-bit
//          ModuleNameLength(DWORD);
//          ModuleName (no terminate character)
//  The PJS body section contains certain records, each record contains:
//          StartAddressOffset  (QWORD)
//          Size                (DWORD)
//          lengthOfSymbol      (DWORD)
//          symbol

#endif // _PJSHEADER_H_
