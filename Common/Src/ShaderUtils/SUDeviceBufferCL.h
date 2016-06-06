//=====================================================================
// Copyright 2010-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SUDeviceBufferCL.h
/// \brief  This file defines SUDeviceBufferCL
///
//=====================================================================
//=====================================================================
// $Id: //devtools/main/Common/Src/ShaderUtils/SUDeviceBufferCL.h#5 $
// Last checkin:   $DateTime: 2016/04/18 06:01:26 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569612 $
//=====================================================================

#ifndef _SU_DEVICE_BUFFER_CL_H_
#define _SU_DEVICE_BUFFER_CL_H_

#include <vector>

#include "CL/cl.h"
#include "SUDeviceBuffer.h"


namespace ShaderUtils
{
/// SUDeviceBufferCL defines the interface for handling ShaderDebuggerCL device buffers.
///  These buffers encapsulate the handling of the underlying OpenCL kernel arguments.
///  TODO rename this class to something the better describes it.
class SUDeviceBufferCL : public SUDeviceBuffer
{
private:
    /// Convenience function for constructors to set all members to NULL.
    void InitAllMembers();

public:
    /// Constructor for SUDeviceBufferCL.
    /// \param[in] device  The Open CL device.
    /// \param[in] context The Open CL context.
    SUDeviceBufferCL(cl_device_id device,
                     cl_context context,
                     cl_command_queue queue);

    /// Constructor for SUDeviceBufferCL.
    /// \param[in] device  The Open CL device.
    /// \param[in] context The Open CL context.
    /// \param[in] value   The Open CL argument value.
    /// \param[in] size    The size of the Open CL argument value.
    /// \param[in] type    The Open CL argument type.
    SUDeviceBufferCL(cl_device_id                device,
                     cl_context                  context,
                     cl_command_queue            queue,
                     std::vector<unsigned char>& value,
                     size_t                      size,
                     BufferDesc                  desc);

    /// Destructor for SUDeviceBufferCL.
    virtual ~SUDeviceBufferCL();

    virtual bool Create(const BufferDesc& bufferDesc);

    virtual bool Destroy();

    virtual SUBuffer* GetDataBuffer(SUBuffer* pBuffer = NULL);

    virtual bool Clear();

    virtual bool DumpBuffer(const std::string& strFilePath) const;

    virtual const void* GetDeviceBuffer() const;

    /// Get the size of the buffer (CL argument) value pointed to by GetDeviceBuffer() in bytes.
    /// \return The size of the buffer (CL argument) value in bytes.
    virtual size_t GetArgumentSize() const;

    /// Update the contents of the buffer
    /// \param[in] pData      the input data
    /// \param[in] bufferSize the input data size
    /// \return true if successful, false otherwise
    virtual bool UpdateData(void*  pData,
                            size_t bufferSize);

protected:
    /// The CL device.
    cl_device_id m_Device;

    /// The CL context this memory is to be bound to
    cl_context m_Context;

    /// The CL queue to use when accessing this memory
    cl_command_queue m_Queue;

    /// The CL memory bound to context or argument value.
    std::vector<unsigned char> m_Value;

    /// The size of the argument. Should match m_Value.size()
    /// except for local memory arguments when m_Value.size() == 0
    size_t m_nSize;

    /// The CL buffer type.
    BufferType m_Type;

    /// Whether we have to deal with deallocating memory
    bool m_bManageMemory;
};

} // namespace ShaderUtils

#endif // _SU_DEVICE_BUFFER_CL_H_
