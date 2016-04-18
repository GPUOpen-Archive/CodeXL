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

#ifndef _COUNTER_ACCESS_INTERFACE_H_
#define _COUNTER_ACCESS_INTERFACE_H_

#include <AMDTDriverTypedefs.h>
#include <AMDTDriverInternal.h>

// CollectBasicCounters: Read basic counters such as sample id, sample spec, timestamp
bool CollectBasicCounters(CoreData* pCoreCfg,
                          uint32* pLength);

// CollectPerCoreCounters: Read core specific counters values
bool CollectPerCoreCounters(CoreData* pCoreCfg,
                            uint32* pLength);

// CollectNonCoreCounters: Read counter values which are independant of any core
bool CollectNonCoreCounters(CoreData* pCoreCfg,
                            uint32* pLength);
// InitializeGenericCounterAccess:
void InitializeGenericCounterAccess(uint32 core);

// CloseGenericCounterAccess
void CloseGenericCounterAccess(void);

#endif //_COUNTER_ACCESS_INTERFACE_H_
