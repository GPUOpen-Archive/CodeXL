//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osTransferableObjectCreator.h
///
//=====================================================================

//------------------------------ osTransferableObjectCreator.h ------------------------------

#ifndef __OSTRANSFERABLEOBJECTCREATOR
#define __OSTRANSFERABLEOBJECTCREATOR

// Local:
#include <AMDTOSWrappers/Include/osTransferableObject.h>
#include <AMDTOSWrappers/Include/osTransferableObjectCreatorsBase.h>


// ----------------------------------------------------------------------------------
// Class Name:           osTransferableObjectCreator
//
// General Description:
//   Template class that is responsible for creating a given (as a template
//   parameter) transferable object class instances.
//
//  Template parameters:
//    TransferableObjectType - The type of the transferable object.
//
//   Example:
//     To define a creator for class Foo - write the following code:
//     osTransferableObjectCreator<Foo> myCreator;
//
//     To create an instance of the Foo class - write the following code:
//     Foo* fooIntance = myCreator.createTransferableObject();
//
// Author:      AMD Developer Tools Team
// Creation Date:        31/1/2004
// ----------------------------------------------------------------------------------
template <class TransferableObjectType>
class osTransferableObjectCreator : public osTransferableObjectCreatorsBase
{
public:
    // Overrides osTransferableObjectCreatorsBase:

    // ---------------------------------------------------------------------------
    // Name:        cloneCreator
    // Description: Returns another instance of this creator object.
    //              (Notice - this is not an instance of the transferable object)
    // Return Val:  virtual apIPCChannelTransferableObject*
    // Author:      AMD Developer Tools Team
    // Date:        1/2/2004
    // ---------------------------------------------------------------------------
    virtual osTransferableObjectCreatorsBase* cloneCreator()
    {
        return new osTransferableObjectCreator<TransferableObjectType>;
    };

    // ---------------------------------------------------------------------------
    // Name:        createTransferableObject
    // Description: Creates the specific class instance of the transferable object
    //              (This is the aim of this class)
    // Author:      AMD Developer Tools Team
    // Date:        1/2/2004
    // ---------------------------------------------------------------------------
    virtual osTransferableObject* createTransferableObject()
    {
        return new TransferableObjectType;
    };


    // ---------------------------------------------------------------------------
    // Name:        int transferableObjectType
    // Description: Returns the type of the transferable class that this creator
    //              creates.
    // Author:      AMD Developer Tools Team
    // Date:        1/2/2004
    // ---------------------------------------------------------------------------
    virtual osTransferableObjectType transferableObjectType()
    {
        TransferableObjectType classInstance;
        return classInstance.type();
    }
};


#endif  // __OSTRANSFERABLEOBJECTCREATOR
