//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsThreadLocalData.cpp
///
//==================================================================================

//------------------------------ gsThreadLocalData.cpp ------------------------------

// Local:
#include <src/gsThreadLocalData.h>


// ----------------------------------------------------------------------------------
// Class Name:           eadLocalData::gsThreadLocalData
// General Description: Constructor.
// Author:               Yaki Tebeka
// Creation Date:        12/4/2005
// ----------------------------------------------------------------------------------
gsThreadLocalData::gsThreadLocalData()
    : _hDC(NULL), _drawSurface(0), _readSurface(0), _spyContextId(0)
{
}

