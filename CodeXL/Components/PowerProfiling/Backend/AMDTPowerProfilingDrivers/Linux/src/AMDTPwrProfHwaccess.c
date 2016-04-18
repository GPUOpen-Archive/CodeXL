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

// SYSTEM INCLUDES
#include <asm/io.h>

// PROJECT INCLUDES
#include <AMDTDriverTypedefs.h>
#include <AMDTHelpers.h>
#include <AMDTPwrProfAsm.h>
#include <AMDTPwrProfCpuid.h>
#include <AMDTPwrProfHwaccess.h>

// LOCAL DEFINES
// Northbridge Configuration Register
#define NB_CFG_MSR      0xC001001FU
// Bit 46, EnableCf8ExtCfg: enable CF8 extended configuration cycles.
#define ENABLE_CF8_EXT_CFG_MASK 0x400000000000ULL
#define PCI_ADDR_PORT       0xCF8
#define PCI_DATA_PORT       0xCFC

// Read CPU id.
void ReadCPUID(uint32 index, uint32 index2, uint32* peax, uint32* pebx, uint32* pecx, uint32* pedx)
{
    uint32 a = 0;
    uint32 b = 0;
    uint32 c = 0;
    uint32 d = 0;
    asm(
        "cpuid\n\t"
        :"=b"(b),
        "=d"(d),
        "=c"(c),
        "=a"(a)
        :"a"(index), "c"(index2)
    );
    /*asm
      ("cpuid" : "=a" (a), "=b" (b), "=c" (c), "=d" (d)
       : "a" (index), "c" (index2));*/
    DRVPRINT("cpuid a=%u, b=%u, c=%u,d =%u \n", a, b, c, d);
    *peax = a;
    *pebx = b;
    *pecx = c;
    *pedx = d;
}

// Read from MSR
void readMSR(uint32 index, uint64_t* pValue, uint64_t password)
{
    uint32 value[2];
    uint32* pPwHalfs = (uint32*)(&password);
    rdmsrpw(index, value[0], value[1], pPwHalfs[0], pPwHalfs[1]);
    *(uint32*)pValue = value[0];
    *(((uint32*)pValue) + 1) = value[1];
}

// Write to MSR
void writeMSR(uint32 index, uint64_t value, uint64_t password)
{
    uint32* pHalfs = (uint32*)(&value);
    uint32* pPwHalfs = (uint32*)(&password);
    wrmsrpw(index, pHalfs[0], pHalfs[1], pPwHalfs[0], pPwHalfs[1]);
}

// Read specified Model Specific Registers
uint32 ReadMSR(uint32 addr, uint64_t* val)
{
    readMSR(addr, val, 0);
    DRVPRINT(" ReadMSR addr 0x%x , value %llu \n" , addr, *val);
    return STATUS_SUCCESS;
}

// Write specified Model Specific Registers
uint32 WriteMSR(uint32 addr, uint64_t val)
{
    DRVPRINT(" WriteMSR addr 0x%x , value %llu \n" , addr, val);
    writeMSR(addr, val, 0);
    return STATUS_SUCCESS;
}

// Read data from the specified MMIO address
uint32 ReadMMIO(uint32 addr, uint64_t* val, bool map)
{
    uint32 byteCount ;
    void __iomem* virtualAddress;
    uint32* p;
    byteCount = 4;
    DRVPRINT(" READMMIO addr 0x%x \n ", addr);
    if (map)
    {
        if ((virtualAddress = ioremap(addr, byteCount)) == NULL)
        {
            DRVPRINT(" ioremap failed \n");
            return STATUS_INVALID_PARAMETER;
        }
    }
    else
    {
        p = &addr;
        virtualAddress = (void*)p;
    }
    DRVPRINT(" READMMIO virtual address %p \n", virtualAddress);
    if (virtualAddress)
    {
        *(uint32*)val = read_dword(virtualAddress);
        if (map)
        {
            iounmap(virtualAddress);
        }
        return STATUS_SUCCESS;
    }
    printk(" WARNING , Invalid MMOIO Address \n");
    return STATUS_INVALID_PARAMETER;
}

// Write data to the specified MMIO address
uint32 WriteMMIO(uint32 addr, uint32 data, bool map)
{
    uint32 byteCount;
    void* virtualAddress ;
    uint32* p;
    DRVPRINT(" WRITEMMIO addr %x, data %u \n ", addr, data);
    byteCount = 4;
    if (map)
    {
        virtualAddress = ioremap(addr, byteCount);
    }
    else
    {
        p = &addr;
        virtualAddress = (void*)p;
    }
    DRVPRINT(" WRITEMMIO virtual address %p \n ", virtualAddress);
    if (virtualAddress != NULL)
    {
        //write_dword( virtualAddress, *(uint32 *)pData );
        iowrite32(data, virtualAddress);
        if (map)
        {
            iounmap(virtualAddress);
        }
        return STATUS_SUCCESS;
    }
    return STATUS_INVALID_PARAMETER;
}

// Get Address for extended configuration address space.
static uint32 GetExtendedConfigurationSpaceAddress(unsigned int bus, unsigned int device, unsigned  func, unsigned int reg)
{
    PciExtendedConfigurationSpaceAddress pciAddr;
    pciAddr.address = 0U;
    pciAddr.element.configEn = 1U;
    pciAddr.element.extRegNo = (reg >> 8) & 0xFU;
    pciAddr.element.regNo = reg & 0x00FCU;
    pciAddr.element.busNo = bus;
    pciAddr.element.device = device;
    pciAddr.element.function = func;
    return pciAddr.address;
}

// Read data from specified PCI32 configuration address.
uint32 ReadPCI32(unsigned int bus, unsigned int device, unsigned int func, unsigned int reg, uint32* data)
{
    uint32 address = GetExtendedConfigurationSpaceAddress(bus, device, func, reg);
    DRVPRINT(" bus %u , device %u , func %u , reg %u , address %u \n", bus, device, func, reg, address);
    return ReadPCI(address , data);
}

// Read data from the specified PCI configuration address
uint32 ReadPCI(uint32 address, uint32* val)
{
    outl(address, PCI_ADDR_PORT);
    *val = inl(PCI_DATA_PORT);
    DRVPRINT("READPCI  address 0x%x  , value %u \n", address, *val);
    return STATUS_SUCCESS;
}

// Write data to the specified PCI configuration address
uint32 WritePCI(uint32 address, uint32 data)
{
    outl(address, PCI_ADDR_PORT);
    outl(data, PCI_DATA_PORT);
    return STATUS_SUCCESS;
}

// Write data to PCI32  address space.
uint32 WritePCI32(uint32 address, uint32 data, uint32 mask)
{
    uint32 old_address;
    uint32 current_data;
    // Save previous address
    old_address = inl(PCI_ADDR_PORT);
    // Set new address
    outl(PCI_ADDR_PORT, address);
    // Read current data
    current_data = inl(PCI_DATA_PORT);
    // Clear bits we're going to change
    current_data &= (mask);
    // Add new data bits
    current_data |= (data);
    // Write data back
    outl(PCI_DATA_PORT, current_data);
    // Restore previous address
    outl(PCI_ADDR_PORT, old_address);
    return STATUS_SUCCESS;
}

// Enable PCI configuration space.
void EnableExtendedPCIConfigSpace(void)
{
    // Enable Access to the PCI Extended Configuration Space
    uint64 nbcfgData;
    ReadMSR(NB_CFG_MSR, &nbcfgData);
    nbcfgData |= ENABLE_CF8_EXT_CFG_MASK;
    WriteMSR(NB_CFG_MSR, nbcfgData);
}
