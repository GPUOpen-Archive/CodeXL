//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Interface to a draw call object
//==============================================================================

#ifndef I_DRAW_CALL_H
#define I_DRAW_CALL_H

#include "AMDTBaseTools/Include/gtASCIIString.h"
#include "CommonTypes.h"

//=====================================================================
/// Common interface that layers can use to execute a draw call.
/** IDrawCall abstract class
* The purpose of this class is to provide a common interface that
* layers can use to execute a draw call without having to know the
* specifics of which graphics API is being used.
* It is intented that a derived class will be implement for each API
* which adds additional functions and member variables for storing
* parameters made to a drawcall and for storing and recalling which
* entrypoint the DrawCall was created for.*/
//=====================================================================
class IDrawCall
{
public:

    //------------------------------------------------------------------
    /// Constructor
    //------------------------------------------------------------------
    IDrawCall() {};

    //------------------------------------------------------------------
    /// Destructor
    //------------------------------------------------------------------
    virtual ~IDrawCall() {};

    //------------------------------------------------------------------
    /// Simple method to all common classes to execute a drawcall that
    /// was previously defined in an API-specific derived class.
    //------------------------------------------------------------------
    virtual void Execute() = 0;

    //------------------------------------------------------------------
    /// returns the Draw call name and parameters in XML format
    //------------------------------------------------------------------
    virtual gtASCIIString GetXML() = 0;

    //------------------------------------------------------------------
    /// returns a hash for the draw call
    //------------------------------------------------------------------
    virtual gtASCIIString GetHash() = 0;

#ifdef GDT_INTERNAL
    //------------------------------------------------------------------
    /// Calculate a CRC based from the current draw call's pixel shader
    ///
    /// \return the calculated CRC as a 64-bit int, or 0 if no pixel shader
    /// is available for the current draw call
    //------------------------------------------------------------------
    virtual UINT64 GetShaderCRC(PIPELINE_STAGE shaderType) = 0;
#endif //GDT_INTERNAL

    //------------------------------------------------------------------
    /// returns hash categories
    //------------------------------------------------------------------
    virtual gtASCIIString GetHashCategories() = 0;

    //------------------------------------------------------------------
    /// Is this an actual draw call or some other operation like a clear?
    //------------------------------------------------------------------
    virtual bool IsDraw() = 0;

    //------------------------------------------------------------------
    /// Is this an actual dispatch call or some other operation like a clear?
    //------------------------------------------------------------------
    virtual bool IsDispatch() { return false; };
};

#endif //I_DRAW_CALL_H
