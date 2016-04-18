//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vsdUnknown.h
///
//==================================================================================

//------------------------------ vsdUnknown.h ------------------------------

#ifndef __VSDUNKNOWN_H
#define __VSDUNKNOWN_H

// COM:
#define WIN32_LEAN_AND_MEAN
#define _ATL_APARTMENT_THREADED
#define _ATL_REGISTER_PER_USER
#include <atlbase.h>

// Local:
#include <CodeXLVSPackageDebugger/Include/vsdPackageDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:          VSD_API  vsdCUnknown
// General Description: Implements the IUnknown methods AddRef and Release. Note that
//                      we rename them (and don't inherit the interface to avoid multiple
//                      inheritance issues (such as ambiguity when casting (IUnknown*)this).
// Author:              Uri Shomroni
// Creation Date:       15/9/2010
// ----------------------------------------------------------------------------------
class VSD_API vsdCUnknown
{
protected:
    vsdCUnknown();
    virtual ~vsdCUnknown();

    ////////////////////////////////////////////////////////////
    // IUnknown method implementations
    STDMETHOD_(ULONG, addRef)(void);
    STDMETHOD_(ULONG, release)(void);

protected:
    // Allow subclasses to query (but not modify) the reference count:
    ULONG getReferenceCount() const {return m_referenceCount;};

private:
    ULONG m_referenceCount;
};

#endif //__VSDUNKNOWN_H

