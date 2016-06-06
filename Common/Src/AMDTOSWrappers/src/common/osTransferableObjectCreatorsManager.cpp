//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osTransferableObjectCreatorsManager.cpp
///
//=====================================================================

//------------------------------ osTransferableObjectCreatorsManager.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTOSWrappers/Include/osTransferableObjectCreatorsBase.h>
#include <AMDTOSWrappers/Include/osTransferableObjectCreatorsManager.h>

// Static members initializations:
osTransferableObjectCreatorsManager* osTransferableObjectCreatorsManager::_pMySingleInstance = NULL;


// ---------------------------------------------------------------------------
// Name:        osTransferableObjectCreatorsManager::instance
// Description: Returns the single instance of this class.
//              (Creates it on the first call to this function).
// Author:      AMD Developer Tools Team
// Date:        31/1/2004
// ---------------------------------------------------------------------------
osTransferableObjectCreatorsManager& osTransferableObjectCreatorsManager::instance()
{
    // If my single instance was not created yet - create it:
    if (_pMySingleInstance == NULL)
    {
        _pMySingleInstance = new osTransferableObjectCreatorsManager;
    }

    return *_pMySingleInstance;
}


// ---------------------------------------------------------------------------
// Name:        osTransferableObjectCreatorsManager::osTransferableObjectCreatorsManager
// Description: Constructor.
// Author:      AMD Developer Tools Team
// Date:        8/2/2004
// ---------------------------------------------------------------------------
osTransferableObjectCreatorsManager::osTransferableObjectCreatorsManager()
{
    // Initialize the creators vector to contains NULLs:
    for (int i = 0; i < OS_AMOUNT_OF_TRANSFERABLE_OBJECT_TYPES; i++)
    {
        _idToTransferableObjCreator.push_back(NULL);
    }
}


// ---------------------------------------------------------------------------
// Name:        osTransferableObjectCreatorsManager::~osTransferableObjectCreatorsManager
// Description: Destructor.
// Author:      AMD Developer Tools Team
// Date:        31/1/2004
// ---------------------------------------------------------------------------
osTransferableObjectCreatorsManager::~osTransferableObjectCreatorsManager()
{
    // Delete the registered creator's clones:
    for (int i = 0; i < OS_AMOUNT_OF_TRANSFERABLE_OBJECT_TYPES; i++)
    {
        osTransferableObjectCreatorsBase* pCurrentCreatorClone = _idToTransferableObjCreator[i];
        delete pCurrentCreatorClone;
    }
}


// ---------------------------------------------------------------------------
// Name:        osTransferableObjectCreatorsManager::registerCreator
// Description: Registers a transferable object creator.
//              This creator can be used later to create a transferable object
//              from its Id.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        10/4/2004
// ---------------------------------------------------------------------------
void osTransferableObjectCreatorsManager::registerCreator(osTransferableObjectCreatorsBase& transferableObjectCreator)
{
    // Get the type of transferable object that this creator creates:
    osTransferableObjectType objectType = transferableObjectCreator.transferableObjectType();

    // Clone the creator:
    osTransferableObjectCreatorsBase* pCreatorClone = transferableObjectCreator.cloneCreator();
    GT_ASSERT(pCreatorClone);

    // Register the creator clone:
    if (_idToTransferableObjCreator[objectType] != NULL)
    {
        delete _idToTransferableObjCreator[objectType];
        gtString errMsg;
        errMsg.appendFormattedString(L"Registering two creators for object type %u", objectType);
        GT_ASSERT_EX(false, errMsg.asCharArray());
    }

    _idToTransferableObjCreator[objectType] = pCreatorClone;
}


// ---------------------------------------------------------------------------
// Name:        osTransferableObjectCreatorsManager::registerAliasCreator
// Description:
//   Registers an aliased creator.
//   An aliased creator enables you to register object A to be created whenever
//   object of type B is required (A is an alias for B).
//
// Arguments: aliasObjectType - The alias type (A)
//            transferableObjectCreator - A creator that creates objects of type B.
// Author:      AMD Developer Tools Team
// Date:        23/2/2006
// ---------------------------------------------------------------------------
void osTransferableObjectCreatorsManager::registerAliasCreator(osTransferableObjectType aliasObjectType,  osTransferableObjectCreatorsBase& transferableObjectCreator)
{
    // Clone the creator:
    osTransferableObjectCreatorsBase* pCreatorClone = transferableObjectCreator.cloneCreator();
    GT_ASSERT(pCreatorClone);

    // Register the creator clone to be the creator of the alias type:
    if (_idToTransferableObjCreator[aliasObjectType] != NULL)
    {
        delete _idToTransferableObjCreator[aliasObjectType];
    }

    _idToTransferableObjCreator[aliasObjectType] = pCreatorClone;
}


// ---------------------------------------------------------------------------
// Name:        osTransferableObjectCreatorsManager::createObject
// Description: Inputs a transferable object type, and creates an object of
//              this type.
// Arguments:   objectType - The type of the object to be created.
//              aptrCreatedObject - Will get the created object.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        26/4/2004
// ---------------------------------------------------------------------------
bool osTransferableObjectCreatorsManager::createObject(unsigned int objectType, gtAutoPtr<osTransferableObject>& aptrCreatedObject)
{
    bool retVal = false;

    // Verify that the input object type is in the right range:
    int amountOfResisteredTransferableObjectTypes = (int)_idToTransferableObjCreator.size();

    if (((int)objectType) < amountOfResisteredTransferableObjectTypes)
    {
        // Get the creator for this type:
        osTransferableObjectCreatorsBase* pCreator = _idToTransferableObjCreator[objectType];

        if (pCreator)
        {
            // Create the transferable object:
            osTransferableObject* pCreatedObj = pCreator->createTransferableObject();

            if (pCreatedObj)
            {
                // Output the created object:
                aptrCreatedObject = pCreatedObj;
                retVal = true;
            }
        }
    }

    return retVal;
}

