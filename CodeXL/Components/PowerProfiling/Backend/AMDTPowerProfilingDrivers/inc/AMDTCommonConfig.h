//===============================================================================
//
// Copyright(c) 2015 Advanced Micro Devices, Inc.All Rights Reserved
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
//=================================================================================
#ifndef _RAWDATAFILEHEADER_H
#define _RAWDATAFILEHEADER_H
#include <AMDTDriverInternal.h>
#include <AMDTRawDataFileHeader.h>

// FillSmuAccessData
bool FillSmuAccessData(SmuList* srcList, SmuList* destList);

// ConfigureSourceProfiling: Configuration for source code profiling
void ConfigureSourceProfiling(CoreData* pCoreData);

// CloseSourceProfiling: Close the configuration for source code profiling
void CloseSourceProfiling(CoreData* pCoreData);

// ConfigureSmu:
void ConfigureSmu(SmuList* pList, bool isOn);

// SetSmuAccessState: Se this flag is there is any failure while accesing smu
// it and any of the SMU configured for the profiling.
void SetSmuAccessState(bool state);

// GetSmuAccessState: If all configured SMU are accisible this will send true.
bool GetSmuAccessState(void);
#endif //_RAWDATAFILEHEADER_H

