//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspGRApiFunctions.cpp
///
//==================================================================================

//------------------------------ vspGRApiFunctions.cpp ------------------------------

#include "stdafx.h"

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtAutoPtr.h>
#include <AMDTAPIClasses/Include/apBreakPoint.h>

// Local:
#include <src/vspGRApiFunctions.h>
#include <src/vscBreakpointsManager.h>
#include <src/vspWindowsManager.h>
#include <src/vspImagesAndBuffersManager.h>

// Static members initialization:
IVscGRApiFunctionsOwner* vspGRApiFunctions::m_pOwner = NULL;

// ---------------------------------------------------------------------------
// Name:        vspGRApiFunctions::vspGRApiFunctions
// Description: Constructor
// Author:      Uri Shomroni
// Date:        28/9/2010
// ---------------------------------------------------------------------------
vspGRApiFunctions::vspGRApiFunctions()
{
}

// ---------------------------------------------------------------------------
// Name:        vspGRApiFunctions::~vspGRApiFunctions
// Description: Destructor
// Author:      Uri Shomroni
// Date:        28/9/2010
// ---------------------------------------------------------------------------
vspGRApiFunctions::~vspGRApiFunctions()
{
}

// ---------------------------------------------------------------------------
// Name:        vspGRApiFunctions::vspInstance
// Description: Gets the gaGRApiFunctions single instance and casts it to a vspGRApiFunctions.
//              If the instance does not yet exist, creates and registers it as this subclass.
// Author:      Uri Shomroni
// Date:        28/9/2010
// ---------------------------------------------------------------------------
vspGRApiFunctions& vspGRApiFunctions::vspInstance()
{
    // If no instance of gaGRApiFunctions was registered yet:
    if (!wasInstanceRegistered())
    {
        // Create and register an instance of this class:
        vspGRApiFunctions* pAPIFunctions = new vspGRApiFunctions;
        gaGRApiFunctions::registerInstance(*pAPIFunctions);
    }

    // Get the gaGRApiFunctions instance:
    gaGRApiFunctions& theAPIFunctions = instance();

    // Downcast it to our type and return.
    // Note that if the assertion in registerInstance() was encountered, this would probably lead
    // to a crash if a class-specific function is called:
    return (vspGRApiFunctions&)theAPIFunctions;
}

// ---------------------------------------------------------------------------
// Name:        vspGRApiFunctions::gaSetBreakpoint
// Description: Sets a debugged process breakpoint.
// Arguments:   breakpoint - The breakpoint to be set.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:
// ---------------------------------------------------------------------------
bool vspGRApiFunctions::gaSetBreakpoint(const apBreakPoint& breakpoint)
{
    bool retVal = vscBreakpointsManager::instance().setBreakpoint(breakpoint);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspGRApiFunctions::gaRemoveBreakpoint
// Description: Removes a debugged process breakpoint
// Arguments: int breakpointId
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        28/9/2010
// ---------------------------------------------------------------------------
bool vspGRApiFunctions::gaRemoveBreakpoint(int breakpointId)
{
    bool retVal = false;

    // Get the breakpoint information:
    gtAutoPtr<apBreakPoint> aptrBreakpoint;
    bool rcBkpt = gaGetBreakpoint(breakpointId, aptrBreakpoint);
    GT_IF_WITH_ASSERT(rcBkpt && (aptrBreakpoint.pointedObject() != NULL))
    {
        retVal = vscBreakpointsManager::instance().removeBreakpoint(*aptrBreakpoint);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspGRApiFunctions::gaRemoveAllBreakpoints
// Description: Removes all debugged process breakpoint.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        28/9/2010
// ---------------------------------------------------------------------------
bool vspGRApiFunctions::gaRemoveAllBreakpoints()
{
    bool retVal = false;

    // Remove all the registered breakpoints:
    int numberOfBreakpoints = 0;
    bool rcNum = gaGetAmountOfBreakpoints(numberOfBreakpoints);
    GT_IF_WITH_ASSERT(rcNum)
    {
        retVal = true;

        // TO_DO: find a better way to do this, this might be dangerours...
        // Remove each breakpoint:
        for (int i = numberOfBreakpoints - 1; i >= 0; i--)
        {
            bool rcRem = gaRemoveBreakpoint(i);
            GT_ASSERT(rcRem);

            retVal = retVal && rcRem;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspGRApiFunctions::gaSetHexDisplayMode
// Description: Set the hex mode
// Author:      Gilad Yarnitzky
// Date:        18/5/2011
// ---------------------------------------------------------------------------
bool vspGRApiFunctions::gaSetHexDisplayMode(bool hexDisplayMode)
{
    bool retVal = true;

    // Execute the base class implementation:
    retVal = gaGRApiFunctions::gaSetHexDisplayMode(hexDisplayMode);

    // Update multi watch views:
    vspWindowsManager::instance().updateMultiWatchViewHexMode(hexDisplayMode);

    // Update images and buffer views:
    vspImagesAndBuffersManager::instance().updateOpenedViewsHexMode(hexDisplayMode);

    GT_IF_WITH_ASSERT(retVal)
    {
        GT_IF_WITH_ASSERT(m_pOwner != NULL)
        {
            // Update visual studio:
            m_pOwner->SetHexDisplayMode();
            retVal = true;
        }
        else
        {
            retVal = false;
        }
    }

    return retVal;

}

// ---------------------------------------------------------------------------
// Name:        vspGRApiFunctions::setAPIBreakpoint
// Description: Wraps the common gaSetBreakpoint function, allowing it to be used
//              by the package facilities
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        29/9/2010
// ---------------------------------------------------------------------------
bool vspGRApiFunctions::setAPIBreakpoint(const apBreakPoint& breakpoint)
{
    bool retVal = gaGRApiFunctions::gaSetBreakpoint(breakpoint);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspGRApiFunctions::removeAPIBreakpoint
// Description: Wraps the common gaRemoveBreakpoint function, allowing it to be used
//              by the package facilities
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        19/10/2010
// ---------------------------------------------------------------------------
bool vspGRApiFunctions::removeAPIBreakpoint(int breakpointIndex)
{
    bool retVal = gaGRApiFunctions::gaRemoveBreakpoint(breakpointIndex);

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        vspGRApiFunctions::removeAllAPIBreakpoints
// Description: Wraps the common gaRemoveAllBreakpoints function, allowing it to be used
//              by the package facilities
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        19/10/2010
// ---------------------------------------------------------------------------
bool vspGRApiFunctions::removeAllAPIBreakpoints()
{
    bool retVal = gaGRApiFunctions::gaRemoveAllBreakpoints();

    return retVal;
}

void vspGRApiFunctions::setOwner(IVscGRApiFunctionsOwner* pOwner)
{
    m_pOwner = pOwner;
}

