//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apPointerParameterCreator.cpp
///
//==================================================================================

//------------------------------ apPointerParameterCreator.cpp ------------------------------

// Local:
#include <AMDTAPIClasses/Include/apPointerParameterCreator.h>


// ---------------------------------------------------------------------------
// Name:        apPointerParameterCreator::apPointerParameterCreator
// Description: Constructor
// Arguments:   pointedObjectType - The type of the object that is pointed by
//                                  the apPointerParameter:
// Author:  AMD Developer Tools Team
// Date:        6/5/2004
// ---------------------------------------------------------------------------
apPointerParameterCreator::apPointerParameterCreator(osTransferableObjectType pointedObjectType)
    : _pointedObjectType(pointedObjectType)
{
}


// ---------------------------------------------------------------------------
// Name:        apPointerParameterCreator::~apPointerParameterCreator
// Description: Destructor.
// Author:  AMD Developer Tools Team
// Date:        23/11/2006
// ---------------------------------------------------------------------------
apPointerParameterCreator::~apPointerParameterCreator()
{
}


// ---------------------------------------------------------------------------
// Name:        apPointerParameterCreator::cloneCreator
// Description: Returns another instance of this creator object.
//              (Notice - this is not an instance of the transferable object)
// Author:  AMD Developer Tools Team
// Date:        1/2/2004
// ---------------------------------------------------------------------------
osTransferableObjectCreatorsBase* apPointerParameterCreator::cloneCreator()
{
    return new apPointerParameterCreator(_pointedObjectType);
}


// ---------------------------------------------------------------------------
// Name:        apPointerParameterCreator::createTransferableObject
// Description: Creates the specific class instance of the transferable object
//              (This is the aim of this class)
// Author:  AMD Developer Tools Team
// Date:        1/2/2004
// ---------------------------------------------------------------------------
osTransferableObject* apPointerParameterCreator::createTransferableObject()
{
    return new apPointerParameter(NULL, _pointedObjectType);
}


// ---------------------------------------------------------------------------
// Name:        apPointerParameterCreator::transferableObjectType
// Description: Returns the type of the transferable class that this creator
//              creates - OS_TOBJ_ID_POINTER_PARAMETER.
// Author:  AMD Developer Tools Team
// Date:        1/2/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apPointerParameterCreator::transferableObjectType()
{
    return _pointedObjectType;
}
