//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osTransferableObjectCreatorsBase.h
///
//=====================================================================

//------------------------------ osTransferableObjectCreatorsBase.h ------------------------------

#ifndef __OSTRANSFERABLEOBJECTCREATORSBASE
#define __OSTRANSFERABLEOBJECTCREATORSBASE

// Local:
#include <AMDTOSWrappers/Include/osOSWrappersDLLBuild.h>
#include <AMDTOSWrappers/Include/osTransferableObject.h>


// ----------------------------------------------------------------------------------
// Class Name:           OS_API osTransferableObjectCreatorsBase
// General Description:
//   Base class for all template instances of osTransferableObjectsCreator
//
// Author:      AMD Developer Tools Team
// Creation Date:        31/1/2004
// ----------------------------------------------------------------------------------
class OS_API osTransferableObjectCreatorsBase
{
public:
    osTransferableObjectCreatorsBase();
    virtual ~osTransferableObjectCreatorsBase();

    virtual osTransferableObjectCreatorsBase* cloneCreator() = 0;
    virtual osTransferableObjectType transferableObjectType() = 0;
    virtual osTransferableObject* createTransferableObject() = 0;
};


#endif  // __OSTRANSFERABLEOBJECTCREATORSBASE
