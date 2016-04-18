//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsRenderPrimitiveType.h
///
//==================================================================================

//------------------------------ gsRenderPrimitiveType.h ------------------------------

#ifndef __GSRENDERPRIMITIVETYPE_H
#define __GSRENDERPRIMITIVETYPE_H

// ----------------------------------------------------------------------------------
// Enum: gsRenderPrimitiveType
// General Description: Is used for render primitives counters. Define a counted render
//                      primitives type
// Author:              Sigal Algranaty
// Creation Date:       6/7/2009
// ----------------------------------------------------------------------------------
enum gsRenderPrimitiveType
{
    GS_VERTICES = 0, // Must remain first!
    GS_POINTS = 1,
    GS_LINES = 2,
    GS_TRIANGLES = 3,
    GS_PRIMITIVES = 4, // Must remain last!
    GS_AMOUNT_OF_PRIMITIVE_TYPES = 5
};

#endif //__GSRENDERPRIMITIVETYPE_H

