//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdAllocatedObjectsCreationStackView.cpp
///
//==================================================================================

//------------------------------ gdAllocatedObjectsCreationStackView.cpp ------------------------------

// Qt:
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra:
#include <AMDTAPIClasses/Include/apAllocatedObject.h>

// API functions:
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/views/gdAllocatedObjectsCreationStackView.h>


// ---------------------------------------------------------------------------
// Name:        gdAllocatedObjectsCreationStackView::gdAllocatedObjectsCreationStackView
// Description: Constructor
// Author:      Uri Shomroni
// Date:        26/10/2008
// ---------------------------------------------------------------------------
gdAllocatedObjectsCreationStackView::gdAllocatedObjectsCreationStackView(QWidget* pParent, const gtASCIIString& columnHeader):
    gdCallsStackListCtrl(pParent, false)
{
    if (!columnHeader.isEmpty())
    {
        QStringList headers;
        headers << columnHeader.asCharArray();
        initHeaders(headers, false);
        horizontalHeader()->show();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdAllocatedObjectsCreationStackView::~gdAllocatedObjectsCreationStackView
// Description: Destructor
// Author:      Uri Shomroni
// Date:        26/10/2008
// ---------------------------------------------------------------------------
gdAllocatedObjectsCreationStackView::~gdAllocatedObjectsCreationStackView()
{

}

// ---------------------------------------------------------------------------
// Name:        gdAllocatedObjectsCreationStackView::getAndDisplayObjectCreationStack
// Description: Gets and displays the allocated object's creation calls stack.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        26/10/2008
// ---------------------------------------------------------------------------
bool gdAllocatedObjectsCreationStackView::getAndDisplayObjectCreationStack(const apAllocatedObject& obj)
{
    bool retVal = false;

    int objAllocationId = obj.getAllocatedObjectId();

    osCallStack objCreationStack;
    retVal = gaGetAllocatedObjectCreationStack(objAllocationId, objCreationStack);

    // Note: we do not assert here as in some cases not having a calls stack is normal.
    // (eg profile mode, user chose not to collect creation stacks)
    if (retVal)
    {
        updateCallsStack(objCreationStack);
    }
    else
    {
        // Display a "Not available" message by sending an empty stack:
        objCreationStack.clearStack();
        updateCallsStack(objCreationStack);
    }

    return retVal;
}
