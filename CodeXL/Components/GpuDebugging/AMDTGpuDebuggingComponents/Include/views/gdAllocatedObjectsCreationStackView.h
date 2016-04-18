//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdAllocatedObjectsCreationStackView.h
///
//==================================================================================

//------------------------------ gdAllocatedObjectsCreationStackView.h ------------------------------

#ifndef __GDALLOCATEDOBJECTSCREATIONSTACKVIEW_H
#define __GDALLOCATEDOBJECTSCREATIONSTACKVIEW_H

// Qt:
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/views/gdCallsStackListCtrl.h>

// Predeclarations:
class apAllocatedObject;

class GD_API gdAllocatedObjectsCreationStackView : public gdCallsStackListCtrl
{
    Q_OBJECT

public:

    gdAllocatedObjectsCreationStackView(QWidget* pParent, const gtASCIIString& columnHeader);
    ~gdAllocatedObjectsCreationStackView();

    bool getAndDisplayObjectCreationStack(const apAllocatedObject& obj);
};

#endif //__GDALLOCATEDOBJECTSCREATIONSTACKVIEW_H

