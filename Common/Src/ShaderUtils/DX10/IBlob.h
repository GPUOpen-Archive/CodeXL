//=====================================================================
// Copyright 2008-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file IBlob.h
///
//=====================================================================

//=====================================================================
// $Id: //devtools/main/Common/Src/ShaderUtils/DX10/IBlob.h#4 $
//
// Last checkin:  $DateTime: 2016/04/18 06:01:26 $
// Last edited by: $Author:  AMD Developer Tools Team
//=====================================================================

#pragma once
#ifndef IDATABLOB_H
#define IDATABLOB_H

#include <Windows.h>

/// IBlob replicates the functionality of ID3D10Blob without requiring D3D10.
class IBlob
{
protected:
    /// Default constructor. Declared as protected to prevent it from being created directly.
    IBlob() { ; };

    /// Destructor.
    virtual ~IBlob() { ; };

public:
    /// Release the memory allocated for the blob.
    virtual ULONG Release() = 0;

    /// Retrieve a pointer to the blob buffer.
    /// \return A pointer to the blob.
    virtual void* GetBufferPointer() const = 0;

    /// Retrieve the size of the blob buffer.
    /// \return The size to the blob.
    virtual size_t GetBufferSize() const = 0;
};

/// Factory method for creating a blob object of specified size.
/// \param[in] nSize The size of the blob to create.
/// \return A pointer to the blob if successful, otherwise false.
IBlob* CreateBlob(size_t nSize);

/// Factory method for creating a blob object of specified size with initial data.
/// \param[in] pData A pointer to data with which to initialise the blob.
/// \param[in] nSize The size of the blob to create.
/// \return A pointer to the blob if successful, otherwise false.
IBlob* CreateBlob(void* pData, size_t nSize);

#endif //  IDATABLOB_H
