//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief File contains classes that can release their ID3D11 pointers when the auto variable goes out of scope. Copied from GPS
//==============================================================================

#ifndef _DC_AUTO_H_
#define _DC_AUTO_H_

#include <windows.h>
#include <d3d11.h>
#include "..\Common\Defs.h"

/// \addtogroup DCCommandRecorder
// @{

/// This class should be used as an auto variable. It will acquire a pointer to the
/// immediate context and release it when it goes out of scope.
class ImmediateContext
{

public:

    /// Constructor
    /// \param pDev The device to get the immediate context from
    ImmediateContext(ID3D11Device* pDev)
        : m_pImmediateContext(NULL)
    {
        //PsAssert ( pDev != NULL );
        pDev->GetImmediateContext(&m_pImmediateContext);
        //PsAssert ( m_pImmediateContext != NULL );
    }

    /// Destructor
    ~ImmediateContext()
    {
        SAFE_RELEASE(m_pImmediateContext);
    }

    /// Get a pointer to the immediate context
    /// \return pointer to the immediate context
    ID3D11DeviceContext* Get()
    {
        return m_pImmediateContext;
    }

    /// Get a pointer to the immediate context
    /// \return pointer to the immediate context
    ID3D11DeviceContext* operator->()
    {
        return m_pImmediateContext;
    }

private:

    /// Store the immediate context
    ID3D11DeviceContext* m_pImmediateContext ;

};

// @}

#endif // _DC_AUTO_H_