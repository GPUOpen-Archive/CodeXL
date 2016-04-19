//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Interface to a draw call object
//==============================================================================

#ifndef I_DXDRAW_CALL_H
#define I_DXDRAW_CALL_H

#include "AMDTBaseTools/Include/gtASCIIString.h"
#include "../Common/CommonTypes.h"
#include "../Common/IDrawCall.h"
#include "ShaderDebuggerCommon.h"

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
class IDXDrawCall : public IDrawCall
{
public:

    //------------------------------------------------------------------
    /// Constructor
    //------------------------------------------------------------------
    IDXDrawCall() {};

    //------------------------------------------------------------------
    /// Destructor
    //------------------------------------------------------------------
    virtual ~IDXDrawCall() {};

    //------------------------------------------------------------------
    /// Gets the thread info set by the Dispatch functions.
    /// \param rDispatchDimensions Output ref to the thread data.
    //------------------------------------------------------------------
    virtual void GetDispatchData(ShaderUtils::ThreadID&   rDispatchDimensions)
    {
        // Set the ThreadID to nothing
        rDispatchDimensions.x = 0;
        rDispatchDimensions.y = 0;
        rDispatchDimensions.z = 0;
        rDispatchDimensions.nSubResource = 0;
        rDispatchDimensions.type = ShaderUtils::TID_Unknown;
    }
};

#endif // I_DXDRAW_CALL_H
