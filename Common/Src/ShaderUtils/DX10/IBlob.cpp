//=====================================================================
// Copyright 2008-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file IBlob.cpp
///
//=====================================================================

//=====================================================================
// $Id: //devtools/main/Common/Src/ShaderUtils/DX10/IBlob.cpp#4 $
//
// Last checkin:  $DateTime: 2016/04/18 06:01:26 $
// Last edited by: $Author:  AMD Developer Tools Team
//=====================================================================

#include "IBlob.h"
#include <new>

/// A class implementing the IBlob interface.
class CDataBlob : public IBlob
{
public:
    /// Default constructor.
    CDataBlob();

    /// Destructor.
    virtual ~CDataBlob();

    virtual void* GetBufferPointer() const {return m_pData;};
    virtual size_t GetBufferSize() const {return m_nSize;};

    /// Allocate a buffer of specified size within the blob.
    /// \param[in] nSize The size of the blob to create.
    void* Allocate(size_t nSize);

    virtual ULONG Release();

private:
    void* m_pData;    ///< A pointer to the blob buffer.
    size_t m_nSize;   ///< The size of the blob buffer.
};

CDataBlob::CDataBlob()
{
    m_pData = NULL;
    m_nSize = 0;
}

CDataBlob::~CDataBlob()
{
    Release();
}

void* CDataBlob::Allocate(size_t nSize)
{
    Release();
    m_nSize = nSize;
    return (m_pData = malloc(nSize));
}

ULONG CDataBlob::Release()
{
    if (m_pData)
    {
        free(m_pData);
    }

    m_pData = NULL;
    m_nSize = 0;
    return 0;
}

IBlob* CreateBlob(size_t nSize)
{
    CDataBlob* pBlob = new(std::nothrow) CDataBlob();

    if (!pBlob->Allocate(nSize))
    {
        delete pBlob;
        pBlob = NULL;
    }

    return pBlob;
}

IBlob* CreateBlob(void* pData, size_t nSize)
{
    CDataBlob* pBlob = (CDataBlob*) CreateBlob(nSize);

    if (pBlob && pData)
    {
        memcpy(pBlob->GetBufferPointer(), pData, nSize);
    }

    return pBlob;
}
