//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspUnknown.h
///
//==================================================================================

//------------------------------ vspUnknown.h ------------------------------

#ifndef __VSPUNKNOWN_H
#define __VSPUNKNOWN_H


// ----------------------------------------------------------------------------------
// Class Name:           vspCUnknown
// General Description: Implements the IUnknown methods AddRef and Release. Note that
//                      we rename them (and don't inherit the interface to avoid moultiple
//                      inheritance issues (such as ambiguity when casting (IUnknown*)this).
// Author:               Uri Shomroni
// Creation Date:        15/9/2010
// ----------------------------------------------------------------------------------
class vspCUnknown
{
protected:
    vspCUnknown();
    virtual ~vspCUnknown();

    ////////////////////////////////////////////////////////////
    // IUnknown method implementations
    STDMETHOD_(ULONG, addRef)(void);
    STDMETHOD_(ULONG, release)(void);

protected:
    // Allow subclasses to query (but not modify) the reference count:
    ULONG getReferenceCount() const {return _referenceCount;};

private:
    ULONG _referenceCount;
};

#endif //__VSPUNKNOWN_H

