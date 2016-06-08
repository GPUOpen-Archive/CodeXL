//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apPointerParameterCreator.h
///
//==================================================================================

//------------------------------ apPointerParameterCreator.h ------------------------------

#ifndef __APPOINTERPARAMETERCREATOR
#define __APPOINTERPARAMETERCREATOR

// Infra:
#include <AMDTOSWrappers/Include/osTransferableObjectCreatorsBase.h>

// Local:
#include <AMDTAPIClasses/Include/apBasicParameters.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apPointerParameterCreator : public osTransferableObjectCreatorsBase
// General Description: Responsible for creating apPointerParameter objects.
// Author:  AMD Developer Tools Team
// Creation Date:        6/5/2004
// ----------------------------------------------------------------------------------
class AP_API apPointerParameterCreator : public osTransferableObjectCreatorsBase
{
public:
    apPointerParameterCreator(osTransferableObjectType pointedObjectType);
    virtual ~apPointerParameterCreator();

    // Overrides apTransferableObjectCreatorsBase:
    virtual osTransferableObjectCreatorsBase* cloneCreator();
    virtual osTransferableObject* createTransferableObject();
    virtual osTransferableObjectType transferableObjectType();

private:
    // The type of the object that is pointed by the apPointerParameter:
    osTransferableObjectType _pointedObjectType;
};


#endif  // __APPOINTERPARAMETERCREATOR
