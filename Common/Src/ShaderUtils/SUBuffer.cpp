//=====================================================================
// Copyright (c) 2010 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/Common/Src/ShaderUtils/SUBuffer.cpp $
/// \version $Revision: #11 $
/// \brief  This class defines base buffer object that are used in
///         ShaderDebugger and APP Profiler.
//
//=====================================================================
// $Id: //devtools/main/Common/Src/ShaderUtils/SUBuffer.cpp#11 $
// Last checkin:   $DateTime: 2016/04/14 04:43:34 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569084 $
//=====================================================================

#include <cstring>
#include <cstdlib>
#include <cassert>
#include "SUBuffer.h"

using namespace ShaderUtils;
using namespace std;

SUBuffer::SUBuffer()
    :  m_nPointSize(0),
       m_nSubResourceSize(0),
       m_nSubResourceCount(0),
       m_nSize(0),
       m_pData(NULL)
{
    memset(&m_Desc, 0, sizeof(m_Desc));
}


SUBuffer::SUBuffer(const SUBuffer& obj)
{
    m_Desc               = obj.m_Desc;
    m_nPointSize         = obj.m_nPointSize;
    m_nSubResourceCount  = obj.m_nSubResourceCount;
    m_nSubResourceSize   = obj.m_nSubResourceSize;
    m_nSize              = obj.m_nSize;

    m_pData              = (unsigned char*) malloc(m_nSize);
    memcpy(m_pData, obj.m_pData, m_nSize);
}


SUBuffer& SUBuffer::operator=(const SUBuffer& obj)
{
    if (this != &obj)
    {
        m_Desc               = obj.m_Desc;
        m_nPointSize         = obj.m_nPointSize;
        m_nSubResourceCount  = obj.m_nSubResourceCount;
        m_nSubResourceSize   = obj.m_nSubResourceSize;
        m_nSize              = obj.m_nSize;

        SU_SAFE_FREE(m_pData);
        m_pData              = (unsigned char*) malloc(m_nSize);
        memcpy(m_pData, obj.m_pData, m_nSize);
    }

    return *this;
}


SUBuffer::~SUBuffer()
{
    SU_SAFE_FREE(m_pData);
}


bool SUBuffer::Create(ShaderType shaderType,
                      unsigned int nWidth,
                      unsigned int nHeight,
                      unsigned int nPitch,
                      BufferType type,
                      BufferFormat format,
                      BufferUsage usage,
                      void* pData)
{
    BufferDesc desc;
    desc.m_ShaderType = shaderType;
    desc.m_Type = type;
    desc.m_Format = format;
    desc.m_Usage = usage;
    desc.m_nWidth = nWidth;
    desc.m_nHeight = nHeight;
    desc.m_nDepth = 1;// dwDepth;
    desc.m_nArraySize = 1;// dwArraySize;
    desc.m_nPitch = nPitch;
    desc.m_nSampleCount = 1;// dwSampleCount;
    desc.m_nSampleQuality = 0;// dwSampleQuality;

    return Create(desc, pData);
}


bool SUBuffer::Create(BufferDesc& desc,
                      void* pData)
{
    SU_Assert(m_pData == NULL);

    m_Desc = desc;

    switch (GetFormat())
    {
        case BF_RGBA_32F: m_nPointSize = 4 * sizeof(float);         break;

        case BF_RGBA_16:  m_nPointSize = 4 * sizeof(unsigned short); break;

        case BF_RGBA_8:   m_nPointSize = 4 * sizeof(unsigned char); break;

        case BF_RGB_32:   m_nPointSize = 3 * sizeof(unsigned int);  break;

        case BF_RGB_16:   m_nPointSize = 3 * sizeof(unsigned short); break;

        case BF_RGB_8:    m_nPointSize = 3 * sizeof(unsigned char); break;

        case BF_RG_32:    m_nPointSize = 2 * sizeof(unsigned int);  break;

        case BF_RG_16:    m_nPointSize = 2 * sizeof(unsigned short); break;

        case BF_RG_8:     m_nPointSize = 2 * sizeof(unsigned char); break;

        case BF_R_32:     m_nPointSize = sizeof(unsigned int);      break;

        case BF_R_16:     m_nPointSize = sizeof(unsigned short);    break;

        case BF_R_8:      m_nPointSize = sizeof(unsigned char);     break;

        case BF_RAW:      m_nPointSize = sizeof(unsigned char);     break;

        case BF_XYZW_32F: m_nPointSize = 4 * sizeof(float);         break;

        default:          SU_Break("m_Format is valid");            return false;
    }

    if (GetFormat() == BF_RAW)
    {
        m_nSubResourceSize = desc.m_nSize;
    }
    else
    {
        size_t nPitch = m_Desc.m_nWidth * m_nPointSize;

        if (m_Desc.m_nPitch == 0)
        {
            m_Desc.m_nPitch = nPitch;
        }

        m_nSubResourceSize = m_Desc.m_nPitch * m_Desc.m_nHeight;
    }

    m_nSubResourceCount = m_Desc.m_nArraySize * m_Desc.m_nDepth; // * ((m_Desc.m_nSampleCount == 1) ? 1 : (m_Desc.m_nSampleCount + 1));
    m_nSize = m_nSubResourceSize * m_nSubResourceCount;
    m_pData = (unsigned char*) malloc(m_nSize);
    SU_Assert(m_pData != NULL);

    if (m_pData == NULL)
    {
        SU_TODO("Implement a Logger")
        //Log(logERROR, "SUBuffer::Create failed. Could not malloc %i bytes\n", m_nSize);
        m_Desc.m_nWidth = m_Desc.m_nHeight = 0;
        m_nSize = 0;
        return false;
    }

    memset(m_pData, 0, m_nSize);

    if (pData != NULL)
    {
        return UpdateData(pData, 0, 0, 1);
    }

    return true;
}


bool SUBuffer::UpdateData(void* pData,
                          size_t nSourcePitch,
                          unsigned int nSubResource,
                          int nSubResourceCount)
{
    SU_Assert(m_pData != NULL);

    if (m_pData == NULL)
    {
        return false;
    }

    SU_Assert(pData != NULL);

    if (pData == NULL)
    {
        return false;
    }

    SU_Assert(nSubResource < m_nSubResourceCount);

    if (nSubResource >= m_nSubResourceCount)
    {
        return false;
    }

    SU_Assert((nSubResourceCount == -1) || ((nSubResourceCount != 0) && ((nSubResource + nSubResourceCount) <= m_nSubResourceCount)));

    if ((nSubResourceCount != -1) && ((nSubResourceCount == 0) || ((nSubResource + nSubResourceCount) > m_nSubResourceCount)))
    {
        return false;
    }

    if (nSubResourceCount == -1)
    {
        nSubResourceCount = m_nSubResourceCount - nSubResource;
    }

    unsigned char* pDest = GetSubResource(nSubResource);
    size_t nSize = nSubResourceCount * m_nSubResourceSize;

    if (nSourcePitch == 0 && m_Desc.m_nPitch != 0)
    {
        nSourcePitch = m_Desc.m_nWidth * m_nPointSize;
    }

    if (nSourcePitch == m_Desc.m_nPitch)
    {
        // We can copy all the data at once
        memcpy(pDest, pData, nSize);
    }
    else
    {
        // Copy line by line to pack the data
        unsigned char* pSrc = (unsigned char*) pData;
        unsigned int nLines = m_Desc.m_nHeight * nSubResourceCount;

        for (unsigned int i = 0; i < nLines; i++)
        {
            memcpy(pDest, pSrc, m_Desc.m_nPitch);
            pSrc += nSourcePitch;
            pDest += m_Desc.m_nPitch;
        }
    }

    return true;
}


template<typename BufferType, typename ComponentType, typename MaskType>
bool SUBuffer::MergeAs(const SUBuffer& other,
                       const SUBuffer& mask,
                       bool bInvertMask,
                       size_t nDestComponentOffset,
                       size_t nSrcComponentOffset,
                       size_t nNumComponents)
{
    // number of components per buffer sample
    size_t nNumBufferComponents = sizeof(BufferType) / sizeof(ComponentType);

    if (nNumComponents == 0)
    {
        // zero means copy all components
        nNumComponents = nNumBufferComponents;
    }

    SU_Assert(nNumBufferComponents * sizeof(ComponentType) == sizeof(BufferType));

    if (nNumBufferComponents * sizeof(ComponentType) != sizeof(BufferType))
    {
        return false;
    }

    // check we won't copy outside a sample
    if (nDestComponentOffset + nNumComponents > nNumBufferComponents ||
        nSrcComponentOffset + nNumComponents > nNumBufferComponents)
    {
        return false;
    }

    size_t nNumDestPadComponents = nNumBufferComponents - nDestComponentOffset - nNumComponents;
    size_t nNumSrcPadComponents  = nNumBufferComponents - nSrcComponentOffset - nNumComponents;

    // loop over each sub-resource merging each one
    for (unsigned int nSubResource = 0; nSubResource < GetSubResourceCount(); nSubResource++)
    {
        ComponentType* pData = reinterpret_cast<ComponentType*>(GetSubResource(nSubResource));
        const ComponentType* pDataEnd = reinterpret_cast<ComponentType*>(GetSubResource(nSubResource) + GetSubResourceSize());

        const ComponentType* pOtherData = reinterpret_cast<ComponentType*>(other.GetSubResource(nSubResource));

        const MaskType* pMaskData = reinterpret_cast<MaskType*>(mask.GetSubResource(0));

        while (pData < pDataEnd)
        {
            bool bMask = (*pMaskData != MaskType(0));

            if (bInvertMask)
            {
                bMask = !bMask;
            }

            if (bMask)
            {
                // offset into sample
                pData      += nDestComponentOffset;
                pOtherData += nSrcComponentOffset;

                // copy components
                for (size_t i = 0; i < nNumComponents; i++)
                {
                    *pData++ = *pOtherData++;
                }

                // move to next sample
                pData      += nNumDestPadComponents;
                pOtherData += nNumSrcPadComponents;
            }
            else
            {
                // move to next sample
                pData      += nNumBufferComponents;
                pOtherData += nNumBufferComponents;
            }

            pMaskData++;
        }
    }

    return true;
}


template<typename BufferType, typename ComponentType>
bool SUBuffer::MergeAs(const SUBuffer& other,
                       const SUBuffer& mask,
                       bool bInvertMask,
                       size_t nDestComponentOffset,
                       size_t nSrcComponentOffset,
                       size_t nNumComponents)

{
    switch (mask.GetFormat())
    {
        case BF_RGBA_8:
        case BF_RG_16:
        case BF_R_32:
            return MergeAs<BufferType, ComponentType, unsigned int>(other,
                                                                    mask,
                                                                    bInvertMask,
                                                                    nDestComponentOffset,
                                                                    nSrcComponentOffset,
                                                                    nNumComponents);
            break;

        case BF_RG_8:
        case BF_R_16:
            return MergeAs<BufferType, ComponentType, unsigned short>(other,
                                                                      mask,
                                                                      bInvertMask,
                                                                      nDestComponentOffset,
                                                                      nSrcComponentOffset,
                                                                      nNumComponents);
            break;

        case BF_R_8:
            return MergeAs<BufferType, ComponentType, unsigned char>(other,
                                                                     mask,
                                                                     bInvertMask,
                                                                     nDestComponentOffset,
                                                                     nSrcComponentOffset,
                                                                     nNumComponents);
            break;

        case BF_XYZW_32F:
        case BF_RGBA_32F:
            return MergeAs<BufferType, ComponentType, RGBA_32F>(other,
                                                                mask,
                                                                bInvertMask,
                                                                nDestComponentOffset,
                                                                nSrcComponentOffset,
                                                                nNumComponents);
            break;

        case BF_RGBA_16:
        case BF_RGB_32:
        case BF_RGB_16:
        case BF_RGB_8:
        case BF_RG_32:
            SU_TODO("Complete support for merging all buffer types")
            SU_Break("Buffer format not supported");
            return false;

        default:
            return false;
    }
}


bool SUBuffer::Merge(const SUBuffer& other,
                     const SUBuffer& mask,
                     bool bInvertMask,
                     size_t nDestComponentOffset,
                     size_t nSrcComponentOffset,
                     size_t nNumComponents)
{
    // check all dimensions match and other and this are same format
    if (other.GetWidth()  != mask.GetWidth() ||
        other.GetHeight() != mask.GetHeight() ||
        other.GetDepth()  != mask.GetDepth() ||
        this->m_Desc      != other.m_Desc)
    {
        return false;
    }

    switch (GetFormat())
    {
        case BF_RGBA_8:
            return MergeAs<unsigned int, unsigned char>(other,
                                                        mask,
                                                        bInvertMask,
                                                        nDestComponentOffset,
                                                        nSrcComponentOffset,
                                                        nNumComponents);
            break;

        case BF_RG_16:
            return MergeAs<unsigned int, unsigned short>(other,
                                                         mask,
                                                         bInvertMask,
                                                         nDestComponentOffset,
                                                         nSrcComponentOffset,
                                                         nNumComponents);
            break;

        case BF_R_32:
            return MergeAs<unsigned int, unsigned int>(other,
                                                       mask,
                                                       bInvertMask,
                                                       nDestComponentOffset,
                                                       nSrcComponentOffset,
                                                       nNumComponents);
            break;

        case BF_RG_8:
            return MergeAs<unsigned short, unsigned char>(other,
                                                          mask,
                                                          bInvertMask,
                                                          nDestComponentOffset,
                                                          nSrcComponentOffset,
                                                          nNumComponents);
            break;

        case BF_R_16:
            return MergeAs<unsigned short, unsigned short>(other,
                                                           mask,
                                                           bInvertMask,
                                                           nDestComponentOffset,
                                                           nSrcComponentOffset,
                                                           nNumComponents);
            break;

        case BF_R_8:
            return MergeAs<unsigned char, unsigned char>(other,
                                                         mask,
                                                         bInvertMask,
                                                         nDestComponentOffset,
                                                         nSrcComponentOffset,
                                                         nNumComponents);
            break;

        case BF_XYZW_32F:
        case BF_RGBA_32F:
            return MergeAs<RGBA_32F, float>(other,
                                            mask,
                                            bInvertMask,
                                            nDestComponentOffset,
                                            nSrcComponentOffset,
                                            nNumComponents);
            break;

        case BF_RGBA_16:
        case BF_RGB_32:
        case BF_RGB_16:
        case BF_RGB_8:
        case BF_RG_32:
            SU_TODO("Complete support for merging all buffer types");
            SU_Break("Buffer format not supported");
            return false;

        default:
            return false;
    }
}


unsigned char* SUBuffer::GetSubResource(unsigned int nSubResource) const
{
    SU_Assert(nSubResource < m_nSubResourceCount);

    if (nSubResource >= m_nSubResourceCount)
    {
        return NULL;
    }

    return m_pData + (nSubResource * m_nSubResourceSize);
}


bool SUBuffer::IsBlank(unsigned int nSubResource) const
{
    if (this == NULL)
    {
        return true;
    }

    const unsigned char* pData = GetSubResource(nSubResource);

    const unsigned char* pDataEnd = pData + GetSubResourceSize();

    while (pData < pDataEnd)
    {
        if (*pData++ != 0)
        {
            return false;
        }
    }

    return true;
}


template<typename T>
size_t SUBuffer::CountActiveThreadsAs(unsigned int nSubResource) const
{
    const T* pData = (T*) GetSubResource(nSubResource);
    const T* pDataEnd = (T*)(GetSubResource(nSubResource) + GetSubResourceSize());

    unsigned int nActiveThreadCount = 0;

    while (pData < pDataEnd)
    {
        if (*pData++ != 0)
        {
            nActiveThreadCount++;
        }
    }

    return nActiveThreadCount;
}


size_t SUBuffer::GetActiveItemCount(unsigned int nSubResource) const
{
    if (this == NULL)
    {
        return 0;
    }

    size_t nActiveThreadCount = 0;

    BufferFormat format = GetFormat();

    if (format == BF_R_8)
    {
        nActiveThreadCount = CountActiveThreadsAs<unsigned char*>(nSubResource);
    }
    else if ((format == BF_R_16) || (format == BF_RG_8))
    {
        nActiveThreadCount = CountActiveThreadsAs<unsigned short*>(nSubResource);
    }
    else if ((format == BF_R_32) || (format == BF_RG_16) || (format == BF_RGBA_8))
    {
        nActiveThreadCount = CountActiveThreadsAs<unsigned int*>(nSubResource);
    }
    else if (format == BF_RGBA_32F || format == BF_XYZW_32F)
    {
        // Float 0 is also unsigned int 0 so we can do it this way for speed
        const unsigned int* pData = (unsigned int*) GetSubResource(nSubResource);
        const unsigned int* pDataEnd = (unsigned int*)(GetSubResource(nSubResource) + GetSubResourceSize());

        while (pData < pDataEnd)
        {
            if (pData[0] != 0 || pData[1] != 0 || pData[2] != 0 || pData[3] != 0)
            {
                nActiveThreadCount++;
            }

            pData += 4;
        }
    }
    else if ((format == BF_RGBA_16) || (format == BF_RGB_32) || (format == BF_RGB_16) || (format == BF_RGB_8) || (format == BF_RG_32))
    {
        SU_TODO("Complete support for merging all buffer types");
    }

    return nActiveThreadCount;
}


size_t SUBuffer::GetItemCount() const
{
    if (this == NULL)
    {
        return 0;
    }

    BufferFormat format = GetFormat();
    size_t subResourceSize = GetSubResourceSize();

    switch (format)
    {
        case BF_R_8:
            return subResourceSize;

        case BF_R_16:
        case BF_RG_8:
            return subResourceSize >> 1;

        case BF_R_32:
        case BF_RG_16:
        case BF_RGBA_8:
            return subResourceSize >> 2;

        case BF_RG_32:
        case BF_RGBA_16:
            return subResourceSize >> 3;

        case BF_RGBA_32F:
        case BF_XYZW_32F:
            return subResourceSize >> 4;

        case BF_RGB_8:
            return subResourceSize * 3;

        case BF_RGB_16:
            return subResourceSize * 6;

        case BF_RGB_32:
            return subResourceSize * 12;

        default:
            break;
    }

    return 0;
}


ThreadIDType SUBuffer::GetThreadIDType() const
{
    if (GetType() == BT_Buffer)
    {
        if (GetDepth() > 1)
        {
            return TID_3D;
        }
        else if (GetHeight() > 1)
        {
            return TID_2D;
        }
        else
        {
            return TID_1D;
        }
    }
    else
    {
        return ::GetThreadIDType(GetType());
    }
}


bool SUBuffer::IsValidThread(ThreadID& threadID) const
{
    if (!IsValid(threadID))
    {
        return false;
    }

    if (GetThreadIDType() != threadID.type)
    {
        return false;
    }

    switch (threadID.type)
    {
        case TID_3D:

            if (threadID.z >= (int) GetDepth())
            {
                return false;
            }

        // Fall-through

        case TID_2D:

            if (threadID.y >= (int) GetHeight())
            {
                return false;
            }

        // Fall-through

        case TID_1D:

            if (threadID.x >= (int) GetWidth())
            {
                return false;
            }

            break;

        default:
        {
            break;
        }
    }

    if (threadID.nSubResource >= (int) GetSubResourceCount())
    {
        return false;
    }

    return true;
}


bool SUBuffer::GetThreadValue(float fPoint[4],
                              ThreadID& threadID) const
{
    if (this == NULL)
    {
        return false;
    }

    if (GetFormat() != BF_RGBA_32F && GetFormat() != BF_XYZW_32F)
    {
        return false;
    }

    if (!IsValidThread(threadID))
    {
        return false;
    }

    if (m_pData == NULL)
    {
        return false;
    }

    unsigned int nSubResource = (threadID.nSubResource > 0) ? threadID.nSubResource : 0;
    int x = (threadID.x > 0) ? threadID.x : 0;
    int y = (threadID.y > 0) ? threadID.y : 0;
    int z = (threadID.z > 0) ? threadID.z : 0;

    float* pfPoint = (float*)(GetSubResource(nSubResource) +
                              (((z * GetHeight() + y) * GetWidth()) + x) * m_nPointSize);

    fPoint[0] = *pfPoint++;
    fPoint[1] = *pfPoint++;
    fPoint[2] = *pfPoint++;
    fPoint[3] = *pfPoint++;

    return true;
}


bool SUBuffer::GetThreadValue(unsigned char& cPoint,
                              ThreadID& threadID) const
{
    if (this == NULL)
    {
        return false;
    }

    if (GetFormat() != BF_R_8)
    {
        return false;
    }

    if (!IsValidThread(threadID))
    {
        return false;
    }

    if (GetSubResource(threadID.nSubResource) == NULL)
    {
        return false;
    }

    unsigned int nSubResource = (threadID.nSubResource > 0) ? threadID.nSubResource : 0;
    int x = (threadID.x > 0) ? threadID.x : 0;
    int y = (threadID.y > 0) ? threadID.y : 0;
    int z = (threadID.z > 0) ? threadID.z : 0;

    unsigned char* pcPoint = (unsigned char*)(GetSubResource(nSubResource) +
                                              (((z * GetHeight() + y) * GetWidth()) + x) * m_nPointSize);

    cPoint = *pcPoint;

    return true;
}


bool SUBuffer::GetThreadValue(unsigned char cPoint[4],
                              ThreadID& threadID) const
{
    if (this == NULL)
    {
        return false;
    }

    if (GetFormat() != BF_RGBA_8)
    {
        return false;
    }

    if (!IsValidThread(threadID))
    {
        return false;
    }

    if (GetSubResource(threadID.nSubResource) == NULL)
    {
        return false;
    }

    unsigned int nSubResource = (threadID.nSubResource > 0) ? threadID.nSubResource : 0;
    int x = (threadID.x > 0) ? threadID.x : 0;
    int y = (threadID.y > 0) ? threadID.y : 0;
    int z = (threadID.z > 0) ? threadID.z : 0;

    unsigned char* pcPoint = (unsigned char*)(GetSubResource(nSubResource) +
                                              (((z * GetHeight() + y) * GetWidth()) + x) * m_nPointSize);

    cPoint[0] = *pcPoint++;
    cPoint[1] = *pcPoint++;
    cPoint[2] = *pcPoint++;
    cPoint[3] = *pcPoint++;

    return true;
}


ThreadID SUBuffer::FindThread(float fPoint[4],
                              int nSubResource) const
{
    ThreadID threadID;

    if (this != NULL && (GetFormat() == BF_RGBA_32F || GetFormat() == BF_XYZW_32F))
    {
        SU_TODO("set thread dimension more intelligently")

        if (GetFormat() == BF_XYZW_32F)
        {
            threadID.type = TID_1D;
        }

        float fThisPoint[4];
        int nStartSubResource = (nSubResource == -1) ? 0 : nSubResource;
        int nEndSubResource = (nSubResource == -1) ? GetSubResourceCount() - 1 : nSubResource;

        for (threadID.nSubResource = nStartSubResource; threadID.nSubResource <= nEndSubResource; threadID.nSubResource++)
        {
            for (threadID.x = 0; (unsigned int) threadID.x < GetWidth(); threadID.x++)
            {
                for (threadID.y = 0; (unsigned int) threadID.y < GetHeight(); threadID.y++)
                {
                    if (GetThreadValue(fThisPoint, threadID) && memcmp(fPoint, fThisPoint, sizeof(fThisPoint)) == 0)
                    {
                        return threadID;
                    }
                }
            }
        }
    }

    return g_tID_Error;
}


ThreadID SUBuffer::FindThread(unsigned char cPoint,
                              int nSubResource) const
{
    ThreadID threadID;

    if (this != NULL && GetFormat() == BF_R_8)
    {
        unsigned char cThisPoint = 0;
        int nStartSubResource = (nSubResource == -1) ? 0 : nSubResource;
        int nEndSubResource = (nSubResource == -1) ? GetSubResourceCount() - 1 : nSubResource;

        for (threadID.nSubResource = nStartSubResource; threadID.nSubResource <= nEndSubResource; threadID.nSubResource++)
        {
            for (threadID.x = 0; (unsigned int) threadID.x < GetWidth(); threadID.x++)
            {
                for (threadID.y = 0; (unsigned int) threadID.y < GetHeight(); threadID.y++)
                {
                    if (GetThreadValue(cThisPoint, threadID) && cPoint == cThisPoint)
                    {
                        return threadID;
                    }
                }
            }
        }
    }

    return g_tID_Error;
}


ThreadID SUBuffer::FindThread(unsigned char cPoint[4],
                              int nSubResource) const
{
    ThreadID threadID;

    if (this != NULL && GetFormat() == BF_RGBA_8)
    {
        unsigned char cThisPoint[4];
        int nStartSubResource = (nSubResource == -1) ? 0 : nSubResource;
        int nEndSubResource = (nSubResource == -1) ? GetSubResourceCount() - 1 : nSubResource;

        for (threadID.nSubResource = nStartSubResource; threadID.nSubResource <= nEndSubResource; threadID.nSubResource++)
        {
            for (threadID.x = 0; (unsigned int) threadID.x < GetWidth(); threadID.x++)
            {
                for (threadID.y = 0; (unsigned int) threadID.y < GetHeight(); threadID.y++)
                {
                    if (GetThreadValue(cThisPoint, threadID) && memcmp(cPoint, cThisPoint, sizeof(cThisPoint)) == 0)
                    {
                        return threadID;
                    }
                }
            }
        }
    }

    return g_tID_Error;
}

ThreadIDType ShaderUtils::GetThreadIDType(BufferType bufferType)
{
    switch (bufferType)
    {
        case BT_Buffer:          return TID_1D;

        case BT_Texture_1D:      return TID_1D;

        case BT_Texture_2D:      return TID_2D;

        case BT_Texture_CubeMap: return TID_2D;

        case BT_Texture_3D:      return TID_3D;

        default:                 return TID_2D;
    }
}
