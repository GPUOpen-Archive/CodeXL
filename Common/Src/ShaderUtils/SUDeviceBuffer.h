//=====================================================================
// Copyright 2010-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SUDeviceBuffer.h
/// \brief  This file defines SUDeviceBuffer class
///
//=====================================================================
//=====================================================================
// $Id: //devtools/main/Common/Src/ShaderUtils/SUDeviceBuffer.h#5 $
// Last checkin:   $DateTime: 2016/04/18 06:01:26 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569612 $
//=====================================================================

#ifndef _SU_DEVICE_BUFFER_H_
#define _SU_DEVICE_BUFFER_H_

#include "SUCommon.h"
#include "SUBuffer.h"
#include <string>

namespace ShaderUtils
{
/// SUDeviceBuffer defines the interface for handling ShaderDebugger device buffers.
/// These buffers encapsulate the handling of the underlying API specific buffers.
class SUDeviceBuffer
{
protected:
    /// Constructor for SUDeviceBuffer.
    SUDeviceBuffer();

public:
    /// Destructor for SUDeviceBuffer.
    virtual ~SUDeviceBuffer();

    /// Create a buffer.
    /// \param[in] bufferDesc The descriptor for the buffer to create.
    /// \return True if successful, otherwise false.
    virtual bool Create(const BufferDesc& bufferDesc) = 0;

    /// Get a data buffer from the device buffer and write to pBuffer
    /// \param[inout] pBuffer The buffer to which the device buffer will be updated.
    /// \return A pointer to the input buffer if successful, NULL otherwise.
    virtual SUBuffer* GetDataBuffer(SUBuffer* pBuffer = NULL) = 0;

    /// Get a pointer to the API-specific device buffer.
    /// \return A pointer to the API-specific device buffer.
    virtual const void* GetDeviceBuffer() const = 0;

    /// Clear the destination buffer.
    /// \return True if successful, otherwise false.
    virtual bool Clear() = 0;

    /// Get Device buffer description.
    /// \param[out] pDesc Output description.
    void GetDeviceBufferDesc(BufferDesc* pDesc) const;

    /// Get the buffer format.
    /// \return The buffer format.
    BufferFormat GetBufferFormat() const { return m_Desc.m_Format; }

    /// Get the buffer type.
    /// \return The buffer type.
    BufferType GetBufferType() const { return m_Desc.m_Type; }

    /// Get the buffer usage.
    /// \return The buffer usage.
    BufferUsage GetBufferUsage() const { return m_Desc.m_Usage; }

    /// Get the buffer shader type.
    /// \return The buffer shader type.
    ShaderType GetBufferShaderType() const { return m_Desc.m_ShaderType; }

    /// Dump the contents of the buffer to a file.
    /// This functionality is intended for debugging use only.
    /// \param[in] strFilePath The filename path to dump the buffer to.
    /// \return True if successful, otherwise false.
    virtual bool DumpBuffer(const std::string& strFilePath) const = 0;

    /// Update the contents of the buffer
    /// \param[in] pData      the input data
    /// \param[in] bufferSize the input data size
    /// \return true if successful, false otherwise
    virtual bool UpdateData(void*  pData,
                            size_t bufferSize) = 0;


protected:
    /// The buffer description.
    BufferDesc m_Desc;

private:
    /// Disabling copy constructor
    /// \param rhs  the input obj
    SUDeviceBuffer(const SUDeviceBuffer& rhs);

    /// Disabling assignment operator
    /// \param rhs  the input obj
    /// \param self object
    SUDeviceBuffer& operator=(const SUDeviceBuffer& rhs);
};

} // namespace ShaderUtils

#endif // _SU_DEVICE_BUFFER_H_
