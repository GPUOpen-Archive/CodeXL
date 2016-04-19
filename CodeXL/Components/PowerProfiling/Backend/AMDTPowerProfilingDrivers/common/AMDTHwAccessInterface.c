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

#include <AMDTPwrProfAttributes.h>
#include <AMDTDriverTypedefs.h>
#include <AMDTHelpers.h>

// AccessMMIO: Read/Write to the MMIO address space.
// This method is used to called from user space
bool AccessMMIO(ACCESS_MMIO* pMMIO)
{
    bool ret = false;
    uint64 res = 0;
    uint64 map = 0;

    // DRVPRINT("PWRPROF: AccessMMIO R/W: %d, addr:0x%x data:0x%x\n", pMMIO->m_isReadAccess, pMMIO->m_addr, pMMIO->m_data);

    // Memory map for the address.
    ret = HelpMapMMIOSpace(pMMIO->m_addr,
                           sizeof(uint32),
                           &map,
                           &res);

    if (true == ret)
    {
        if (1 == pMMIO->m_isReadAccess)
        {
            pMMIO->m_data = READ_DWORD(map);
        }
        else
        {
            WRITE_DWORD(map, pMMIO->m_data);
        }

        HelpUnmapMMIOSpace(map, sizeof(uint32));
    }

    return ret;
}

