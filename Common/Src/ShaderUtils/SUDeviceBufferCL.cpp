//=====================================================================
// Copyright (c) 2010 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/Common/Src/ShaderUtils/SUDeviceBufferCL.cpp $
/// \version $Revision: #5 $
/// \brief  This file defines SUDeviceBufferCL
//
//=====================================================================
// $Id: //devtools/main/Common/Src/ShaderUtils/SUDeviceBufferCL.cpp#5 $
// Last checkin:   $DateTime: 2016/04/14 04:43:34 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569084 $
//=====================================================================

#include <cstring>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "SUDeviceBufferCL.h"
#include "DispatchTable_Internal.h"

// This include must be last (after direct or indirect includes of OpenCL)
#include "CLGuardian.h"

using namespace ShaderUtils;
using namespace AMD_ShaderDebugger;
using namespace std;


void SUDeviceBufferCL::InitAllMembers()
{
    m_Device = 0;
    m_Context = 0;
    m_Queue = 0;
    m_nSize = 0;
    m_Type = BT_Unknown;
    m_bManageMemory = false;
}


SUDeviceBufferCL::SUDeviceBufferCL(cl_device_id device,
                                   cl_context context,
                                   cl_command_queue queue)
    : SUDeviceBuffer()
{
    InitAllMembers();

    m_Device = device;
    m_Context = context;
    m_Queue = queue;
}


SUDeviceBufferCL::SUDeviceBufferCL(cl_device_id           device,
                                   cl_context             context,
                                   cl_command_queue       queue,
                                   vector<unsigned char>& value,
                                   size_t                 size,
                                   BufferDesc             desc)
    : SUDeviceBuffer()
{
    m_Device = device;
    m_Context = context;
    m_Queue = queue;

    m_Value = value;
    m_nSize = size;
    m_Desc = desc;

    // check sizes match except for local buffer arguments
    SU_Assert(m_Desc.m_Type == BT_LocalBuffer || value.size() == size);

    // never have to deallocate memory as buffer is users memory or
    // it is a simple value argument
    m_bManageMemory = false;
}


SUDeviceBufferCL::~SUDeviceBufferCL()
{
    Destroy();
}


bool SUDeviceBufferCL::Create(const BufferDesc& bufferDesc)
{
    cl_int ret;

    Destroy();

    m_Desc = bufferDesc;

    if (m_Desc.m_Type == BT_Value)
    {
        // value vector holds actual argument value
        m_Value.resize(m_Desc.m_nSize);
        m_nSize = m_Value.size();
    }
    else if (m_Desc.m_Type == BT_LocalBuffer)
    {
        // there is no argument value, only a size
        m_Value.clear();
        m_nSize = m_Desc.m_nSize;
    }
    else if (m_Desc.m_Type == BT_Buffer)
    {
        // adjust the size of the buffer for image style formats
        switch (m_Desc.m_Format)
        {
            case BF_RGBA_32F: // fall through
            case BF_XYZW_32F:
                m_Desc.m_nSize = 4 * sizeof(float)
                                 * max(m_Desc.m_nWidth, (unsigned int) 1)
                                 * max(m_Desc.m_nHeight, (unsigned int) 1)
                                 * max(m_Desc.m_nDepth, (unsigned int) 1) ;
                break;

            case BF_RGBA_8:
                m_Desc.m_nSize = 4 * sizeof(unsigned char)
                                 * max(m_Desc.m_nWidth, (unsigned int) 1)
                                 * max(m_Desc.m_nHeight, (unsigned int) 1)
                                 * max(m_Desc.m_nDepth, (unsigned int) 1) ;
                break;

            case BF_R_32:
                m_Desc.m_nSize = sizeof(unsigned int)
                                 * max(m_Desc.m_nWidth, (unsigned int) 1)
                                 * max(m_Desc.m_nHeight, (unsigned int) 1)
                                 * max(m_Desc.m_nDepth, (unsigned int) 1) ;
                break;

            case BF_R_8:
                m_Desc.m_nSize = sizeof(unsigned char)
                                 * max(m_Desc.m_nWidth, (unsigned int) 1)
                                 * max(m_Desc.m_nHeight, (unsigned int) 1)
                                 * max(m_Desc.m_nDepth, (unsigned int) 1) ;
                break;

            case BF_RAW:
                // do nothing as m_Desc.m_nSize should contain correct size
                break;

            default:
                SU_Break("Unsupported CL buffer format");
                return false;
        }

        cl_mem mem = g_CLDispatchTable.CreateBuffer(m_Context,
                                                    CL_MEM_READ_WRITE,
                                                    m_Desc.m_nSize,
                                                    NULL,
                                                    &ret);
        m_Desc.m_AccessType = BA_ReadWrite;

        if (ret != CL_SUCCESS)
        {
            SU_Break("Could not allocate CL memory");
            return false;
        }

        // save memory handle
        m_Value.assign(reinterpret_cast<const unsigned char*>(&mem),
                       reinterpret_cast<const unsigned char*>(&mem) + sizeof(cl_mem));

        m_nSize = m_Value.size();
    }
    else
    {
        cl_image_format format;

        // fill in image format
        switch (m_Desc.m_Format)
        {
            case BF_RGBA_32F: // fall through
            case BF_XYZW_32F:
                format.image_channel_order = CL_RGBA;
                format.image_channel_data_type = CL_FLOAT;
                break;

            case BF_RGBA_8:
                format.image_channel_order = CL_RGBA;
                format.image_channel_data_type = CL_UNSIGNED_INT8;
                break;

            case BF_R_32:
                format.image_channel_order = CL_R;
                format.image_channel_data_type = CL_UNSIGNED_INT32;
                break;

            case BF_R_8:
                format.image_channel_order = CL_R;
                format.image_channel_data_type = CL_UNSIGNED_INT8;
                break;

            case BF_RAW:   // no such thing as a raw image format so fall through to error
            default:
                SU_Break("Unsupported CL buffer format");
                return false;
        }

        cl_mem mem;

        // allocate the memory
        switch (m_Desc.m_Type)
        {
            case BT_Texture_1D:
                mem = g_CLDispatchTable.CreateImage2D(m_Context,
                                                      CL_MEM_READ_WRITE,
                                                      &format,
                                                      m_Desc.m_nWidth,
                                                      1,
                                                      0,
                                                      NULL,
                                                      &ret);
                m_Desc.m_AccessType = BA_ReadWrite;

                if (ret != CL_SUCCESS)
                {
                    SU_Break("Could not allocate CL memory");
                    return false;
                }

                break;

            case BT_Texture_2D:
                mem = g_CLDispatchTable.CreateImage2D(m_Context,
                                                      CL_MEM_READ_WRITE,
                                                      &format,
                                                      m_Desc.m_nWidth,
                                                      m_Desc.m_nHeight,
                                                      0,
                                                      NULL,
                                                      &ret);
                m_Desc.m_AccessType = BA_ReadWrite;

                if (ret != CL_SUCCESS)
                {
                    SU_Break("Could not allocate CL memory");
                    return false;
                }

                break;

            case BT_Texture_3D:
                mem = g_CLDispatchTable.CreateImage3D(m_Context,
                                                      CL_MEM_READ_WRITE,
                                                      &format,
                                                      m_Desc.m_nWidth,
                                                      m_Desc.m_nHeight,
                                                      m_Desc.m_nDepth,
                                                      0,
                                                      0,
                                                      NULL,
                                                      &ret);
                m_Desc.m_AccessType = BA_ReadWrite;

                if (ret != CL_SUCCESS)
                {
                    SU_Break("Could not allocate CL memory");
                    return false;
                }

                break;

            default:
                SU_Break("Unsupported CL buffer type");
                return false;
        }

        // save memory handle
        m_Value.assign(reinterpret_cast<const unsigned char*>(&mem),
                       reinterpret_cast<const unsigned char*>(&mem) + sizeof(cl_mem));

        m_nSize = m_Value.size();
    }

    if (m_Desc.m_Type != BT_Value &&
        m_Desc.m_Type != BT_LocalBuffer)
    {
        // only manage memory for real buffer types
        m_bManageMemory = true;
    }

    return Clear();
}


bool SUDeviceBufferCL::Destroy()
{
    // only release memory if it was ours to start with
    if (m_bManageMemory)
    {
        // use value as a memory handle
        cl_mem mem;
        SU_Assert(m_Value.size() == sizeof(cl_mem));
        copy(m_Value.begin(),
             m_Value.end(),
             reinterpret_cast<unsigned char*>(&mem));

        // release memory
        if (g_CLDispatchTable.ReleaseMemObject(mem) != CL_SUCCESS)
        {
            SU_Break("Could not release CL memory");
            return false;
        }

        m_bManageMemory = false;
    }

    m_Value.clear();
    m_nSize = 0;

    return true;
}

SUBuffer* SUDeviceBufferCL::GetDataBuffer(SUBuffer* pDataBuffer)
{
    SU_Assert(m_Desc.m_Type != BT_Value &&
              m_Desc.m_Type != BT_LocalBuffer);

    if (m_Desc.m_Type == BT_Value ||
        m_Desc.m_Type == BT_LocalBuffer)
    {
        SU_Break("Can't get buffer for value type buffers or local memory buffers");
        return NULL;
    }

    // make sure value is the correct size
    SU_Assert(m_Value.size() == sizeof(cl_mem));

    SU_Assert(m_Queue != 0);

    if (m_Queue == 0)
    {
        SU_TODO("Implement a Logger")
        //Log( logERROR, "SUDeviceBufferCL::GetDataBuffer() failed. m_Queue == 0\n" );
        return NULL;
    }

    // use value as a memory handle
    cl_mem mem;
    SU_Assert(m_Value.size() == sizeof(cl_mem));
    copy(m_Value.begin(),
         m_Value.end(),
         reinterpret_cast<unsigned char*>(&mem));

    if (mem == 0)
    {
        SU_TODO("Implement a Logger")
        //Log( logERROR, "SUDeviceBufferCL::GetDataBuffer() failed. mem == 0\n" );
        return NULL;
    }

    if (pDataBuffer == NULL)
    {
        pDataBuffer = new(std::nothrow) SUBuffer();
        SU_Assert(pDataBuffer != NULL);

        if (pDataBuffer == NULL)
        {
            //Log( logERROR, "SUDeviceBufferCL::GetDataBuffer() failed. pDataBuffer == NULL\n" );
            return NULL;
        }

        if (!pDataBuffer->Create(m_Desc))
        {
            SU_Break("pDataBuffer->Create failed");
            SU_TODO("Implement a Logger")
            //Log( logERROR, "SUDeviceBufferCL::GetDataBuffer() failed. pDataBuffer->Create failed.\n" );
            SU_SAFE_DELETE(pDataBuffer);
            return NULL;
        }
    }

    // map CL memory handle to CPU pointer
    cl_int ret;
    void* pData = NULL;
    size_t nRowPitch = 0;
    size_t nSlicePitch = 0;

    if (m_Desc.m_Type == BT_Buffer)
    {
        pData = g_CLDispatchTable.EnqueueMapBuffer(m_Queue,
                                                   mem,
                                                   CL_TRUE,
                                                   CL_MAP_READ,
                                                   0,
                                                   m_Desc.m_nSize,
                                                   0,
                                                   NULL,
                                                   NULL,
                                                   &ret);
    }
    else
    {
        size_t origin[3] = { 0, 0, 0 };
        size_t region[3] = { m_Desc.m_nWidth, m_Desc.m_nHeight, m_Desc.m_nDepth };
        pData = g_CLDispatchTable.EnqueueMapImage(m_Queue,
                                                  mem,
                                                  CL_TRUE,
                                                  CL_MAP_READ,
                                                  origin,
                                                  region,
                                                  &nRowPitch,
                                                  &nSlicePitch,
                                                  0,
                                                  NULL,
                                                  NULL,
                                                  &ret);
    }

    if (ret != CL_SUCCESS)
    {
        SU_Break("clEnqueueMapBuffer() failed");
        //Log( logERROR, "SUDeviceBufferCL::GetDataBuffer() failed. clEnqueueMapBuffer() failed.\n" );
        return NULL;
    }

    size_t nSourcePitch;

    switch (m_Desc.m_Type)
    {
        case BT_Buffer:
            nSourcePitch = 0;
            break;

        case BT_Texture_1D:
            nSourcePitch = 0;
            break;

        case BT_Texture_2D:
            nSourcePitch = nRowPitch;
            break;

        case BT_Texture_3D:
            nSourcePitch = nRowPitch;
            SU_TODO("correctly pass 3D texture info")
            //nSourcePitch = nSlicePitch;
            break;

        default:
            SU_Break("Unsupported CL buffer type");
            return NULL;
    }

    // update buffer
    SU_TODO("remove this casting and use size_t instead")
    pDataBuffer->UpdateData(pData, (unsigned int)nSourcePitch);

    // unmap memory
    ret = g_CLDispatchTable.EnqueueUnmapMemObject(m_Queue,
                                                  mem,
                                                  pData,
                                                  0,
                                                  NULL,
                                                  NULL);

    if (ret != CL_SUCCESS)
    {
        SU_Break("clEnqueueUnmapMemObject() failed");
        SU_TODO("Implement a Logger")
        //Log( logERROR, "SUDeviceBufferCL::GetDataBuffer() failed. clEnqueueUnmapMemObject() failed.\n" );
        return NULL;
    }

    ret += g_CLDispatchTable.Finish(m_Queue);

    return pDataBuffer;
}

bool SUDeviceBufferCL::Clear()
{
    return UpdateData(NULL, 0);
}


bool SUDeviceBufferCL::DumpBuffer(const std::string& strFilePath) const
{
    if (strFilePath.empty())
    {
        return false;
    }

    ofstream fp;

    fp.open(strFilePath.c_str(), ios::binary);

    if (!fp.is_open())
    {
        return false;
    }

    // write out data

    SU_TODO("write out the CL data buffer to a file")

    fp.close();

    return true;
}


const void* SUDeviceBufferCL::GetDeviceBuffer() const
{
    SU_Assert(this != NULL);

    if (m_Desc.m_Type == BT_LocalBuffer)
    {
        return NULL;
    }
    else
    {
        return reinterpret_cast<const void*>(&m_Value[0]);
    }
}


size_t SUDeviceBufferCL::GetArgumentSize() const
{
    SU_Assert(this != NULL);

    // check sizes match except for local buffer arguments
    SU_Assert(m_Desc.m_Type == BT_LocalBuffer ||
              m_Value.size() == m_nSize);

    return m_nSize;
}

bool SUDeviceBufferCL::UpdateData(void* pData, size_t bufferSize)
{
    if (BT_Value       == m_Desc.m_Type ||
        BT_LocalBuffer == m_Desc.m_Type)
    {
        // it's enough to clear the value as there's no memory buffer associated with it
#ifdef _DEBUG
        fill(m_Value.begin(), m_Value.end(), 0xaa);
#else
        fill(m_Value.begin(), m_Value.end(), 0);
#endif

        return true;
    }

    SU_Assert(m_Queue != 0);

    if (m_Queue == 0)
    {
        SU_TODO("Implement a Logger")
        //Log( logERROR, "SUDeviceBufferCL::UpdateData() failed. m_Queue == 0\n" );
        return false;
    }

    // use value as a memory handle
    cl_mem mem;
    SU_Assert(m_Value.size() == sizeof(cl_mem));
    copy(m_Value.begin(),
         m_Value.end(),
         reinterpret_cast<unsigned char*>(&mem));

    SU_Assert(mem != 0);

    if (mem == 0)
    {
        SU_TODO("Implement a Logger")
        //Log( logERROR, "SUDeviceBufferCL::UpdateData() failed. mem == 0\n" );
        return false;
    }

    // map CL memory handle to CPU pointer
    cl_int ret;
    void*  pMappedData = NULL;
    size_t nRowPitch   = 0;
    size_t nSlicePitch = 0;

    if (m_Desc.m_Type == BT_Buffer)
    {
        pMappedData = g_CLDispatchTable.EnqueueMapBuffer(m_Queue,
                                                         mem,
                                                         CL_TRUE,
                                                         CL_MAP_WRITE,
                                                         0,
                                                         m_Desc.m_nSize,
                                                         0,
                                                         NULL,
                                                         NULL,
                                                         &ret);
    }
    else
    {
        size_t origin[3] = { 0, 0, 0 };
        size_t region[3] = { m_Desc.m_nWidth, m_Desc.m_nHeight, m_Desc.m_nDepth };
        pMappedData = g_CLDispatchTable.EnqueueMapImage(m_Queue,
                                                        mem,
                                                        CL_TRUE,
                                                        CL_MAP_WRITE,
                                                        origin,
                                                        region,
                                                        &nRowPitch,
                                                        &nSlicePitch,
                                                        0,
                                                        NULL,
                                                        NULL,
                                                        &ret);
    }

    if (ret != CL_SUCCESS)
    {
        SU_Break("clEnqueueMapBuffer() failed");
        SU_TODO("Implement a Logger")
        //Log( logERROR, "SUDeviceBufferCL::UpdateData() failed. clEnqueueMapBuffer() failed.\n" );
        return false;
    }

    size_t nBufferSize;

    switch (m_Desc.m_Type)
    {
        case BT_Buffer:
            nBufferSize = m_Desc.m_nSize;
            break;

        case BT_Texture_1D:
            nBufferSize = nRowPitch;
            break;

        case BT_Texture_2D:
            nBufferSize = nRowPitch * m_Desc.m_nHeight;
            break;

        case BT_Texture_3D:
            nBufferSize = nSlicePitch * m_Desc.m_nDepth;
            break;

        default:
            SU_Break("Unsupported CL buffer type");
            return false;
    }

    if (NULL == pData)
    {
        // clear memory
#ifdef _DEBUG
        memset(pMappedData, 0xaa, nBufferSize);
#else
        memset(pMappedData, 0, nBufferSize);
#endif
    }
    else
    {
        // copy memory
        SU_Assert(bufferSize == nBufferSize);
        memcpy(pMappedData, pData, std::min(nBufferSize, bufferSize));
    }

    // unmap memory
    ret = g_CLDispatchTable.EnqueueUnmapMemObject(m_Queue,
                                                  mem,
                                                  pMappedData,
                                                  0,
                                                  NULL,
                                                  NULL);

    if (ret != CL_SUCCESS)
    {
        SU_Break("clEnqueueUnmapMemObject() failed");
        SU_TODO("Implement a Logger")
        //Log( logERROR, "SUDeviceBufferCL::UpdateData() failed. clEnqueueUnmapMemObject() failed.\n" );
        return false;
    }

    // wait for the unmap to finish executing
    if (g_CLDispatchTable.Finish(m_Queue) != CL_SUCCESS)
    {
        SU_Break("clFinish() failed");
        SU_TODO("Implement a Logger")
        //Log( logERROR, "SUDeviceBufferCL::UpdateData() failed. clFinish() failed.\n" );
        return false;
    }

    if (NULL != pData && nBufferSize != bufferSize)
    {
        // if we are copying a buffer (rather than clearing it), the buffer size passed into
        // the function should be the same as the one calculated in this function
        return false;
    }

    return true;
}
