//=====================================================================
// Copyright 2010-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SUBuffer.h
/// \brief  This class defines base buffer object that are used in
///         ShaderDebugger and APP Profiler.
///
//=====================================================================
//=====================================================================
// $Id: //devtools/main/Common/Src/ShaderUtils/SUBuffer.h#11 $
// Last checkin:   $DateTime: 2016/04/18 06:01:26 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569612 $
//=====================================================================

#ifndef _SU_BUFFER_H_
#define _SU_BUFFER_H_

#include "SUCommon.h"
#include <cstdlib>

namespace ShaderUtils
{
/// The access type of a buffer.
typedef enum
{
    BA_Unknown = 0,         ///< Unknown access type.
    BA_Read = 0x1,          ///< Read access type.
    BA_Write = 0x2,         ///< Write access type.
    BA_ReadWrite = 0x3      ///< Read and Write access type.
} BufferAccess;

/// The type of a buffer.
typedef enum
{
    BT_Unknown,          ///< Unknown buffer type.
    BT_Buffer,           ///< Generic buffer.
    BT_Texture_1D,       ///< 1D texture.
    BT_Texture_2D,       ///< 2D texture.
    BT_Texture_CubeMap,  ///< Cube-map texture.
    BT_Texture_3D,       ///< 3D texture.
    BT_Value,            ///< A single value. Not really a buffer type but it can be bound to a kernel.
    BT_LocalBuffer       ///< An LDS buffer. Not really a buffer type but it can be bound to a kernel.
} BufferType;

/// The format of a buffer.
typedef enum
{
    BF_Unknown,          ///< Unknown buffer format.
    BF_RGBA_32F,         ///< Four-channel RGBA 32-bit floating point.
    BF_RGBA_16,          ///< Four-channel RGBA 16-bit integer
    BF_RGBA_8,           ///< Four-channel RGBA 8-bit integer.
    BF_RGB_32,           ///< Three-channel 32-bit integer.
    BF_RGB_16,           ///< Three-channel 16-bit integer.
    BF_RGB_8,            ///< Three-channel 8-bit integer.
    BF_RG_32,            ///< Two-channel 32-bit integer.
    BF_RG_16,            ///< Two-channel 16-bit integer.
    BF_RG_8,             ///< Two-channel 8-bit integer.
    BF_R_32,             ///< Single-channel 32-bit integer.
    BF_R_16,             ///< Single-channel 16-bit integer.
    BF_R_8,              ///< Single-channel 8-bit integer.
    BF_RAW,              ///< Raw buffer format.
    BF_XYZW_32F          ///< Four-channel vertex 32-bit floating point.
} BufferFormat;

/// The usage of a buffer.
typedef enum
{
    BU_Unknown,          ///< Unknown buffer usage.
    BU_Results,          ///< Results buffer.
    BU_ExecMask          ///< Execution mask (aka draw mask) buffer.
} BufferUsage;

/// A structure describing a buffer.
typedef struct
{
    BufferType     m_Type;             ///< The type of buffer.
    BufferFormat   m_Format;           ///< The format of the buffer.
    BufferUsage    m_Usage;            ///< The usage of the buffer.
    BufferAccess   m_AccessType;       ///< The access type of the buffer.
    ShaderType     m_ShaderType;       ///< The type of shader using the buffer.
    unsigned int   m_nWidth;           ///< The width of the buffer.
    unsigned int   m_nHeight;          ///< The height of the buffer.
    unsigned int   m_nDepth;           ///< The depth of the buffer.
    unsigned int   m_nStride;          ///< The stride of the buffer. Used only when m_Type is BT_Buffer.
    size_t         m_nSize;            ///< The size of the buffer in bytes. Used only when m_Type is BT_Buffer or BT_Value.
    unsigned int   m_nArraySize;       ///< The number of buffers in the array.
    unsigned int   m_nFirstArraySlice; ///< The first slice in the array.
    unsigned int   m_nLastArraySlice;  ///< The last slice in the array.
    size_t         m_nPitch;           ///< The pitch of the buffer in bytes.
    unsigned int   m_nSampleCount;     ///< The sample count for multi-sampled texture buffers.
    unsigned int   m_nSampleQuality;   ///< The sample quality for multi-sampled texture buffers.
} BufferDesc;

/// Equality operator for comparing instances of BufferDesc.
inline bool operator==(const BufferDesc& lhs, const BufferDesc& rhs)
{
    return (lhs.m_Type             == rhs.m_Type) &&
           (lhs.m_Format           == rhs.m_Format) &&
           (lhs.m_Usage            == rhs.m_Usage) &&
           (lhs.m_AccessType       == rhs.m_AccessType) &&
           (lhs.m_ShaderType       == rhs.m_ShaderType) &&
           (lhs.m_nWidth           == rhs.m_nWidth) &&
           (lhs.m_nHeight          == rhs.m_nHeight) &&
           (lhs.m_nDepth           == rhs.m_nDepth) &&
           (lhs.m_nStride          == rhs.m_nStride) &&
           (lhs.m_nSize            == rhs.m_nSize) &&
           (lhs.m_nArraySize       == rhs.m_nArraySize) &&
           (lhs.m_nFirstArraySlice == rhs.m_nFirstArraySlice) &&
           (lhs.m_nLastArraySlice  == rhs.m_nLastArraySlice) &&
           (lhs.m_nPitch           == rhs.m_nPitch) &&
           (lhs.m_nSampleCount     == rhs.m_nSampleCount) &&
           (lhs.m_nSampleQuality   == rhs.m_nSampleQuality);
}

/// Inequality operator for comparing instances of BufferDesc.
inline bool operator!=(const BufferDesc& lhs, const BufferDesc& rhs)
{
    return !(lhs == rhs);
}

/// Get the thread ID type used by the specified buffer type.
/// \param[in]  bufferType The buffer type.
/// \return    The thread ID type used by the buffer type if successful, otherwise TID_Unknown.
ThreadIDType GetThreadIDType(BufferType bufferType);

/// Buffer encapsulates the handling of patched shader results & draw mask buffers in a platform agnostic manner.
class SUBuffer
{
public:
    /// Constructor for SUBuffer.
    SUBuffer();

    /// Copy constructor
    /// \param[in] obj The buffer to copy.
    SUBuffer(const SUBuffer& obj);

    /// Assignment operator=
    /// \param[in] obj The buffer to copy.
    SUBuffer& operator=(const SUBuffer& obj);

    /// Destructor for SUBuffer.
    virtual ~SUBuffer();

    /// Create the underlying buffer.
    /// \param[in] shaderType  The type of shader being debugged.
    /// \param[in] nWidth      The buffer width.
    /// \param[in] nHeight     The buffer height.
    /// \param[in] nPitch      The buffer pitch.
    /// \param[in] type        The buffer type.
    /// \param[in] format      The buffer format.
    /// \param[in] usage       The buffer usage.
    /// \param[in] pData       Pointer to initial data to copy to the buffer [Optional]. Defaults to NULL.
    /// \return                True if successful, otherwise false.
    virtual bool Create(ShaderType shaderType,
                        unsigned int nWidth,
                        unsigned int nHeight,
                        unsigned int nPitch,
                        BufferType type,
                        BufferFormat format,
                        BufferUsage usage,
                        void* pData = NULL);

    /// Create the underlying buffer.
    /// \param[in] desc        The buffer description.
    /// \param[in] pData       Pointer to initial data to copy to the buffer [Optional]. Defaults to NULL.
    /// \return                True if successful, otherwise false.
    bool Create(BufferDesc& desc,
                void* pData = NULL);

    /// Update data with the buffer.
    /// \param[in] pData             Pointer to data to copy into the buffer.
    /// \param[in] nSourcePitch      Pitch of the source data pointed to by pData.
    /// \param[in] nSubResource      Index of the first sub-resource to update.
    /// \param[in] nSubResourceCount Number of sub-resources to update [Optional]. Defaults to -1. When nSubResourceCount == -1, all sub-resources from dwSubResource to max are updated.
    /// \return                      True if successful, otherwise false.
    bool UpdateData(void* pData,
                    size_t nSourcePitch,
                    unsigned int nSubResource = 0,
                    int nSubResourceCount = -1);

    /// Merge another buffer into this buffer. A mask can be specified to only merge certain elements.
    /// There is also limited component selection. As examples:
    ///   - a 4 component buffer can have the zw values of the source copied to
    ///     the xy values of the destination by specifying
    ///      nSrcComponentOffset  = 2
    ///      nDestComponentOffset = 0
    ///      nNumComponents       = 2
    ///   - a 4 component buffer can have the y value of the source copied to
    ///     the w value of the destination by specifying
    ///      nSrcComponentOffset  = 1
    ///      nDestComponentOffset = 3
    ///      nNumComponents       = 1
    /// Components that are not copied from the source are left untouched in the destination (this buffer).
    /// \param[in] other                The buffer to merge into this buffer. It must be the same format
    ///                                  and size as this buffer.
    /// \param[in] mask                 The mask to specify which values to replace. Non zero values
    ///                                  means replace this buffer value with other buffer value.
    /// \param[in] bInvertMask          Whether the mask is inverted. Non zero value become zero values and vice versa.
    /// \param[in] nDestComponentOffset The component offset in the destination buffer.
    /// \param[in] nSrcComponentOffset  The component offset in the source buffer.
    /// \param[in] nNumComponents       The number fo components to merge. This must not be greater than
    ///                                 min( Number of source components - nSrcComponentOffset,
    ///                                      Number of destination components - nDestComponentOffset )
    ///                                 If this is zero then all components are merged.
    /// \return                         True if successful, otherwise false.
    bool Merge(const SUBuffer& other,
               const SUBuffer& mask,
               bool bInvertMask = false,
               size_t nDestComponentOffset = 0,
               size_t nSrcComponentOffset = 0,
               size_t nNumComponents = 0);

    /// Get the buffer description.
    /// \param[out]   desc  Reference to BufferDesc to copy the buffer description to.
    void GetDesc(BufferDesc& desc) const
    {
        desc = m_Desc;
    }

    /// Get the buffer width.
    /// \return The buffer width.
    unsigned int GetWidth() const
    {
        return m_Desc.m_nWidth;
    }

    /// Get the buffer height.
    /// \return The buffer height.
    unsigned int GetHeight() const
    {
        return m_Desc.m_nHeight;
    }

    /// Get the buffer depth.
    /// \return The buffer depth.
    unsigned int GetDepth() const
    {
        return m_Desc.m_nDepth;
    }

    /// Get the buffer array size.
    /// \return The buffer array size.
    unsigned int GetArraySize() const
    {
        return m_Desc.m_nArraySize;
    }

    /// Get the index of the first buffer array slice.
    /// \return The index of the first buffer array slice.
    unsigned int GetFirstArraySlice() const
    {
        return m_Desc.m_nFirstArraySlice;
    }

    /// Get the buffer width.
    /// \return The buffer width.
    unsigned int GetLastArraySlice() const
    {
        return m_Desc.m_nLastArraySlice;
    }

    /// Get the buffer type.
    /// \return The buffer type.
    BufferType GetType() const
    {
        return m_Desc.m_Type;
    }

    /// Get the buffer thread ID type.
    /// \return The buffer thread ID type.
    ThreadIDType GetThreadIDType() const;

    /// Get the buffer format.
    /// \return The buffer format.
    BufferFormat GetFormat() const
    {
        return m_Desc.m_Format;
    }

    /// Get the buffer usage.
    /// \return The buffer usage.
    BufferUsage GetUsage() const
    {
        return m_Desc.m_Usage;
    }

    /// Get the buffer sample count for MSAA render-targets.
    /// \return The buffer sample count for MSAA render-targets.
    unsigned int GetSampleCount() const
    {
        return m_Desc.m_nSampleCount;
    }

    /// Get the buffer sample quality for MSAA render-targets.
    /// \return The buffer sample quality for MSAA render-targets.
    unsigned int GetSampleQuality() const
    {
        return m_Desc.m_nSampleQuality;
    }

    /// Get the type of shader writing to this buffer.
    /// \return The type of shader writing to this buffer.
    ShaderType GetShaderType() const
    {
        return m_Desc.m_ShaderType;
    }

    /// Get the point size in bytes.
    /// \return The poitn size in bytes.
    size_t GetPointSize() const
    {
        return m_nPointSize;
    }

    /// Get the size of each sub-resource.
    /// \return The size of each sub-resource.
    size_t GetSubResourceSize() const
    {
        return m_nSubResourceSize;
    }

    /// Get the number of sub-resources.
    /// \return The number of sub-resources.
    unsigned int GetSubResourceCount() const
    {
        return m_nSubResourceCount;
    }

    /// Get the a pointer to the specified sub resource in the buffer data.
    /// \param[in] nSubResource  The index of the sub-resource to retrieve.
    /// \return    A pointer to the specified sub resource in the buffer data if successful, otherwise NULL.
    unsigned char* GetSubResource(unsigned int nSubResource) const;

    /// Get the buffer data size.
    /// \return The buffer data size.
    size_t GetSize() const
    {
        return m_nSize;
    }

    /// Get the buffer data pointer.
    /// \return The buffer data pointer.
    unsigned char* GetData()
    {
        return m_pData;
    }

    /// Get the buffer data pointer.
    /// \return The buffer data pointer.
    const unsigned char* GetData() const
    {
        return m_pData;
    }

    /// Is the buffer blank?
    /// \param[in] nSubResource  The sub-resource to query [Optional]. Defaults to 0.
    /// \return True if the buffer is blank, otherwise false.
    bool IsBlank(unsigned int nSubResource = 0) const;

    /// Get the number of non-zero items for the specified sub-resource.
    /// \param[in] nSubResource  The sub-resource to query [Optional]. Defaults to 0.
    /// \return The number of non-zero items.
    size_t GetActiveItemCount(unsigned int nSubResource = 0) const;

    /// Get the number of items in one sub-resource of the buffer.
    /// \return The number items.
    size_t GetItemCount() const;

    /// Is the specified thread ID a valid ID for this buffer?
    /// \param[in] threadID The thread ID to test.
    /// \return    True if the thread ID is valid, otherwise false.
    bool IsValidThread(ThreadID& threadID) const;

    /// Retrieve the value at the specified thread ID.
    /// \param[out]   fPoint   4-way vector to receive the value.
    /// \param[in]    threadID The thread ID to query.
    /// \return       True if successful, otherwise false.
    bool GetThreadValue(float fPoint[4], ThreadID& threadID) const;

    /// Retrieve the  value at the specified thread ID.
    /// \param[out]   cPoint   scalar to receive the value.
    /// \param[in]    threadID The thread ID to query.
    /// \return       True if successful, otherwise false.
    bool GetThreadValue(unsigned char& cPoint, ThreadID& threadID) const ;

    /// Retrieve the value at the specified thread ID.
    /// \param[out]   cPoint   4-way vector to receive the value.
    /// \param[in]    threadID The thread ID to query.
    /// \return       True if successful, otherwise false.
    bool GetThreadValue(unsigned char cPoint[4], ThreadID& threadID) const;

    /// Find a thread in the buffer with the specified value.
    /// \param[in]    fPoint         4-way vector to search for.
    /// \param[in]    nSubResource   The sub-resource to search in [Optional]. Defaults to -1. If -1 then every sub-resource is searched.
    /// \return       The ID of the first thread with the specified value if found, otherwise g_tID_Error.
    ThreadID FindThread(float fPoint[4], int nSubResource = -1) const ;

    /// Find a thread in the buffer with the specified value.
    /// \param[in]    cPoint         scalar to search for.
    /// \param[in]    nSubResource   The sub-resource to search in [Optional]. Defaults to -1. If -1 then every sub-resource is searched.
    /// \return       The ID of the first thread with the specified value if found, otherwise g_tID_Error.
    ThreadID FindThread(unsigned char cPoint, int nSubResource = -1) const;

    /// Find a thread in the buffer with the specified value.
    /// \param[in]    cPoint         4-way vector to search for.
    /// \param[in]    nSubResource   The sub-resource to search in [Optional]. Defaults to -1. If -1 then every sub-resource is searched.
    /// \return       The ID of the first thread with the specified value if found, otherwise g_tID_Error.
    ThreadID FindThread(unsigned char cPoint[4], int nSubResource = -1) const;

protected:
    BufferDesc   m_Desc;                ///< The buffer description.
    size_t       m_nPointSize;          ///< The size of each item in the buffer in bytes.
    size_t       m_nSubResourceSize;    ///< The size of each sub-resource.
    unsigned int m_nSubResourceCount;   ///< The number of sub-resources.
    size_t       m_nSize;               ///< The size of the data buffer.
    // TODO should this be void * ????
    unsigned char* m_pData;             ///< The data buffer.

    /// Count the number of non-zero items in the sub-resource treating data as type T.
    /// \param[in] nSubResource  The sub-resource to query.
    /// \return The number of non-zero items.
    template<typename T>
    size_t CountActiveThreadsAs(unsigned int nSubResource) const;

    /// Merge another buffer into this buffer taking types of both buffer and mask. Helper for Merge().
    /// \param[in] other                The buffer to merge into this buffer.
    /// \param[in] mask                 The mask to specify which values to replace. Non zero values means replace this buffer value with other buffer value.
    /// \param[in] bInvertMask          Whether the mask is inverted. Non zero value become zero values and vice versa.
    /// \param[in] nDestComponentOffset The component offset in the destination buffer.
    /// \param[in] nSrcComponentOffset  The component offset in the source buffer.
    /// \param[in] nNumComponents       The number fo components to merge. This must not be greater than
    ///                                 min( Number of source components - nSrcComponentOffset,
    ///                                      Number of destintion components - nDestComponentOffset )
    /// \return                      True if successful, otherwise false.
    template<typename BufferType, typename ComponentType, typename MaskType>
    bool MergeAs(const SUBuffer& other,
                 const SUBuffer& mask,
                 bool bInvertMask,
                 size_t nDestComponentOffset,
                 size_t nSrcComponentOffset,
                 size_t nNumComponents);

    /// Merge another buffer into this buffer taking type of buffer and mask. Helper for Merge().
    /// \param[in] other                The buffer to merge into this buffer.
    /// \param[in] mask                 The mask to specify which values to replace. Non zero values means replace this buffer value with other buffer value.
    /// \param[in] bInvertMask          Whether the mask is inverted. Non zero value become zero values and vice versa.
    /// \param[in] nDestComponentOffset The component offset in the destination buffer.
    /// \param[in] nSrcComponentOffset  The component offset in the source buffer.
    /// \param[in] nNumComponents       The number fo components to merge. This must not be greater than
    ///                                 min( Number of source components - nSrcComponentOffset,
    ///                                      Number of destintion components - nDestComponentOffset )
    /// \return                      True if successful, otherwise false.
    template<typename BufferType, typename ComponentType>
    bool MergeAs(const SUBuffer& other,
                 const SUBuffer& mask,
                 bool bInvertMask,
                 size_t nDestComponentOffset,
                 size_t nSrcComponentOffset,
                 size_t nNumComponents);
};

}

#endif //_SU_BUFFER_H_
