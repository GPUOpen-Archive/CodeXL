//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file Auxil.h
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/inc/Auxil.h#17 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569613 $
//=============================================================

#ifndef _CPUPROF_AUXIL_H_
#define _CPUPROF_AUXIL_H_

#include <AMDTCpuProfilingRawData/inc/CpuProfileReader.h>
#include "inc/StdAfx.h"

class osFilePath;

// Prototypes
void AuxGetSymbolSearchPath(gtString& searchPath, gtString& serverList, gtString& cachePath);

// The purpose of this API is for 64-bit module under \windows\system32
bool AuxFileExists(QString fileNamePath);

bool AuxGetExecutablePath(QString& exePath,
                          CpuProfileReader& profileReader,
                          const QString& sessionDir,
                          const QString& processPath,
                          QWidget* pParent = nullptr,
                          CpuProfileModule* pModule = nullptr);

void AuxAppendSampleName(QString& sampleName, gtVAddr va);
bool AuxGetParentFunctionName(const CpuProfileModule* pModule, const CpuProfileFunction* pFunction, gtVAddr va, QString& name);

bool AuxIsWindowsSystemModule(const gtString& absolutePath);
bool AuxIsLinuxSystemModule(const gtString& absolutePath);
bool AuxIsSystemModule(const gtString& absolutePath);
bool AuxIsSystemModule(const osFilePath& modulePath);

#endif
