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

#ifndef _AMDTTIMERHELPER_H
#define _AMDTTIMERHELPER_H

#include <AMDTDriverInternal.h>

// AllocateAndInitClientData
//
// Allocate and Initialise Client Data
int AllocateAndInitClientData(ClientData** ppClientData,
                              uint32 clientId,
                              cpumask_t affinity,
                              const ProfileConfig* config);

// IntialiseCoreData
//
// Initialise and  per core data
int IntializeCoreData(CoreData* pCoreClientData,
                      int index,
                      bool* isFirstConfig,
                      uint32 clientId,
                      uint32 coreId,
                      ProfileConfig* config);

// FreeClientData
//
// Free Memmory set by client data
void FreeClientData(ClientData* pClientData);

// FreeCoreData
//
// Free Memory created for each core
void FreeCoreData(CoreData* pCoreClientData);

#endif // _AMDTTIMERHELPER_H
