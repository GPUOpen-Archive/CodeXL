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

#ifndef _PWR_PROF_DRV_HWACCESS_H_
#define _PWR_PROF_DRV_HWACCESS_H_

// LOCAL INCLUDES
#include <AMDTDriverTypedefs.h>

// LOCAL DEFINES
typedef union _PciExtendedConfigurationSpaceAddress
{
    // \brief The elements that make up a PCI address in PCI config space
    struct
    {
        // base register address to access
        unsigned int regNo    : 8;
        // function number to access
        unsigned int function : 3;
        // device number to access
        unsigned int device   : 5;
        // bus number to access
        unsigned int busNo    : 8;
        // extended register number to access
        unsigned int extRegNo : 4;
        // reserved, must be 0
        unsigned int reserved : 3;
        // Configuration space enable, 1 = IO read and write accesses to
        // IOCFC are translated into configuration cycles at the
        // configuration address
        unsigned int configEn : 1;
    } element;
    // The equivalent IO-space configuration address made up of each \ref Element
    uint32 address;
} PciExtendedConfigurationSpaceAddress;

// LOCAL FUNCTIONS

// Read from MSR
uint32 ReadMSR(uint32 addr, uint64_t* val);
// Write to MSR
uint32 WriteMSR(uint32 addr, uint64_t val);
uint32 ReadTSC(uint64_t* p);
// Read data from the specified PCI configuration address
uint32 ReadPCI(uint32 addr , uint32* val);
// Read data from specified PCI32 configuration address.
uint32 ReadPCI32(unsigned int bus, unsigned int device, unsigned int func, unsigned int reg, uint32* data);
// Write data to the specified PCI configuration address
uint32 WritePCI(uint32 addr, uint32 data);
// Write data to PCI32  address space.
uint32 WritePCI32(uint32 addr, uint32 data , uint32 mask);
// Read data from the specified MMIO address
uint32 ReadMMIO(uint32 addr, uint64_t* val, bool map);
// Write data to the specified MMIO address
uint32 WriteMMIO(uint32 addr , uint32 data, bool map);
// Read CPU id.
void ReadCPUID(uint32 index, uint32 index2, uint32* peax, uint32* pebx, uint32* pecx, uint32* pedx);
// Enable PCI configuration space.
void EnableExtendedPCIConfigSpace(void);

// OS specific routines.

// A couple of defines to help isolate CPU features
#define FEATUREFLAGS_GPE_BIT 0x00000200
#define GPE_SUPPORTED(featureFlags) (featureFlags & FEATUREFLAGS_GPE_BIT)
// and one to help with TLB flushes
#define CR4_PGE_BIT 0x00000040

#endif // _PWR_PROF_DRV_HWACCESS_H_
