//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apStateVariablesSnapShot.cpp
///
//==================================================================================

//------------------------------ apStateVariablesSnapShot.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apStateVariablesSnapShot.h>
#include <AMDTBaseTools/Include/gtAssert.h>

apNotSupportedParameter apStateVariablesSnapShot::_staticNotSupportedParameter;
apNotAvailableParameter apStateVariablesSnapShot::_staticNotAvailableParameter;

// ---------------------------------------------------------------------------
// Name:        apStateVariablesSnapShot::apStateVariablesSnapShot
// Description: Constructor.
// Author:  AMD Developer Tools Team
// Date:        26/6/2008
// ---------------------------------------------------------------------------
apStateVariablesSnapShot::apStateVariablesSnapShot(): _isReadFromFile(false), _pStateVariablesFilteredIdsVector(NULL)
{
    // Initialize per state variable vectors:
    for (int i = 0; i < apOpenGLStateVariablesAmount; i++)
    {
        _stateVariableValues[i] = NULL;
        _isStateVariableSupported[i] = false;
        _isStateVariableAvailable[i] = false;
    }
}

// ---------------------------------------------------------------------------
// Name:        apStateVariablesSnapShot::apStateVariablesSnapShot
// Description: Destructor.
// Author:  AMD Developer Tools Team
// Date:        26/6/2008
// ---------------------------------------------------------------------------
apStateVariablesSnapShot::~apStateVariablesSnapShot()
{
    clearContextDataSnapshot();

    // Delete the static not supported parameter:
    /*if (_pStaticNotSupportedParameter != NULL)
    {
        delete _pStaticNotSupportedParameter;
        _pStaticNotSupportedParameter = NULL;
    }

    // Delete the static not available parameter:
    if (_pStaticNotAvailableParameter != NULL)
    {
        delete _pStaticNotAvailableParameter;
        _pStaticNotAvailableParameter = NULL;
    }*/
}

// ---------------------------------------------------------------------------
// Name:        apStateVariableValue::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        26/6/2008
// ---------------------------------------------------------------------------
osTransferableObjectType apStateVariablesSnapShot::type() const
{
    return OS_TOBJ_ID_STATE_VARIABLE_SNAPSHOT;
}

// ---------------------------------------------------------------------------
// Name:        gsStateVariablesSnapshot::clearContextDataSnapshot
// Description: Makes the _stateVariableValues vector empty.
// Author:  AMD Developer Tools Team
// Date:        17/7/2004
// ---------------------------------------------------------------------------
void apStateVariablesSnapShot::clearContextDataSnapshot()
{
    for (int i = 0; i < apOpenGLStateVariablesAmount; i++)
    {
        apParameter* pParameter = _stateVariableValues[i];

        if ((pParameter != NULL) && (pParameter != &_staticNotAvailableParameter) && (pParameter != &_staticNotSupportedParameter))
        {
            delete _stateVariableValues[i];
            _stateVariableValues[i] = NULL;
        }

        _stateVariableValues[i] = NULL;
    }
}


// ---------------------------------------------------------------------------
// Name:        apStateVariablesSnapShot::setIsReadFromFile
// Description: Mark if this snapshot is read from file or not
// Arguments: bool isReadFromFile
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        29/6/2008
// ---------------------------------------------------------------------------
void apStateVariablesSnapShot::setIsReadFromFile(bool isReadFromFile)
{
    _isReadFromFile = isReadFromFile;
}

// ---------------------------------------------------------------------------
// Name:        apStateVariablesSnapShot::apStateVariablesSnapShot
// Description: Writes this class data into a channel.
// Author:  AMD Developer Tools Team
// Date:        15/6/2004
// ---------------------------------------------------------------------------
bool apStateVariablesSnapShot::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    (void)(ipcChannel); // unused
    return true;
}

// ---------------------------------------------------------------------------
// Name:        apStateVariablesSnapShot::apStateVariablesSnapShot
// Description: Reads this class data from a channel.
// Author:  AMD Developer Tools Team
// Date:        15/6/2004
// ---------------------------------------------------------------------------
bool apStateVariablesSnapShot::readSelfFromChannel(osChannel& ipcChannel)
{
    (void)(ipcChannel); // unused
    return true;
}

// ---------------------------------------------------------------------------
// Name:        apStateVariablesSnapShot::addValue
// Description: Adds a value to the state variables values vector
// Arguments:   int id - the parameter id
//              gtAutoPtr<apParameter>  pParamVal - an auto pointer to an apParameter object
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        26/6/2008
// ---------------------------------------------------------------------------
bool apStateVariablesSnapShot::addStateVariableValue(int id, apParameter* pParamVal)
{
    bool retVal = false;

    if (id >= 0 && id < apOpenGLStateVariablesAmount)
    {
        retVal = true;
        _stateVariableValues[id] = pParamVal;

        if ((pParamVal == NULL) || (pParamVal->type() == OS_TOBJ_ID_NOT_AVAILABLE_PARAMETER))
        {
            _isStateVariableAvailable[id] = false;
        }
        else
        {
            _isStateVariableAvailable[id] = true;
        }
    }

    return retVal;
};


// ---------------------------------------------------------------------------
// Name:        apStateVariablesSnapShot::addValue
// Description: This function adds a state variable value for a snapshot file object.
// Arguments: int id
//            gtString val
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/6/2008
// ---------------------------------------------------------------------------
bool apStateVariablesSnapShot::addStateVariableValue(int id, gtString val)
{
    bool retVal = false;

    if (_isReadFromFile)
    {
        if (id >= 0 && id < apOpenGLStateVariablesAmount)
        {
            // Set the parameter's value:
            gtASCIIString str(val.asASCIICharArray());
            apStringParameter* pStringParameter = new apStringParameter(str);


            _stateVariableValues[id] = pStringParameter;
            _isStateVariableAvailable[id] = true;

            retVal = true;
        }
    }

    return retVal;
}

/*// ---------------------------------------------------------------------------
// Name:        apStateVariablesSnapShot::getValue
// Description: Get the id's state variable's value
// Arguments: int id
//            gtString& val
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        26/6/2008
// ---------------------------------------------------------------------------
bool apStateVariablesSnapShot::getValue(int id, gtString& val)
{
    bool retVal = false;
    if (id >= 0 || id < (int)_snapshotValues.size())
    {
        val = _snapshotValues[id];
        retVal = true;
    }
    return retVal;
}*/
// ---------------------------------------------------------------------------
// Name:        apStateVariablesSnapShot::size
// Description: Adds a value to the state variables values vector
// Return Val: int  - TGhe state variables number.
// Author:  AMD Developer Tools Team
// Date:        26/6/2008
// ---------------------------------------------------------------------------
int apStateVariablesSnapShot::size() const
{
    return (int)apOpenGLStateVariablesAmount;
};

// ---------------------------------------------------------------------------
// Name:        apStateVariablesSnapShot::clear
// Description: Clears the values from the values vector
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        26/6/2008
// ---------------------------------------------------------------------------
void apStateVariablesSnapShot::clear()
{
    for (int i = 0; i < (int)apOpenGLStateVariablesAmount; i++)
    {
        _stateVariableValues[i] = NULL;
        _isStateVariableAvailable[i] = false;
    }
}

// ---------------------------------------------------------------------------
// Name:        apStateVariablesSnapShot::compareToOther
// Description: Compares 2 snapshots, and return the ids with the different values
// Arguments:   const apStateVariablesSnapShot& otherVersion - the snapshot to be compared with
//              gtVector<int>& diff - the ids which are different
//              bool& areEqual - are the 2 vectors equal
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/6/2008
// ---------------------------------------------------------------------------
bool apStateVariablesSnapShot::compareToOther(const apStateVariablesSnapShot& otherSnapshot, bool& areValuesEqual) const
{
    bool retVal = false;

    if (_pStateVariablesFilteredIdsVector != NULL)
    {
        retVal = compareOnlyFilteredToOther(otherSnapshot, areValuesEqual);
    }
    else
    {
        retVal = compareAllToOther(otherSnapshot, areValuesEqual);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apStateVariablesSnapShot::compareAllToOther
// Description: Compare all state variables one to another - this is called once the snapshot is not filtered one
// Arguments: const apStateVariablesSnapShot& otherSnapshot
//            bool& areValuesEqual
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        14/9/2008
// ---------------------------------------------------------------------------
bool apStateVariablesSnapShot::compareAllToOther(const apStateVariablesSnapShot& otherSnapshot, bool& areValuesEqual) const
{
    bool retVal = false;

    // Get the sizes of the both vectors:
    int stateVariableAmount = (int)this->size();
    int otherStateVariablesAmount = (int)otherSnapshot.size();

    // Initialize the equality:
    areValuesEqual = true;

    GT_IF_WITH_ASSERT(stateVariableAmount == otherStateVariablesAmount)
    {
        // Iterate the filtered state variables:
        for (int variableId = 0; variableId < otherStateVariablesAmount; variableId++)
        {
            // Compare only supported state variables:
            bool isStateVariableCurrentlySupported = isStateVariableSupported(variableId);

            if (isStateVariableCurrentlySupported == true)
            {
                // Get the current state variable value:
                const apParameter* pParam1 = NULL, *pParam2 = NULL;
                bool rc1 = getStateVariableValue(variableId, pParam1);
                bool rc2 = otherSnapshot.getStateVariableValue(variableId, pParam2);

                if (rc1 && rc2 && (pParam1 != NULL) && (pParam2 != NULL))
                {
                    // Compare the parameters values one to another:
                    areValuesEqual = pParam1->compareToOther(*pParam2);

                    // If values are different, stop comparing:
                    if (!areValuesEqual)
                    {
                        break;
                    }
                }
            }
        }

        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apStateVariablesSnapShot::compareOnlyFilteredToOther
// Description: Compare only the filtered state variables one to another - this is called once the snapshot is a filtered one
// Arguments: const apStateVariablesSnapShot& otherSnapshot
//            bool& areValuesEqual
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        14/9/2008
// ---------------------------------------------------------------------------
bool apStateVariablesSnapShot::compareOnlyFilteredToOther(const apStateVariablesSnapShot& otherSnapshot, bool& areValuesEqual) const
{
    bool retVal = false;

    // Initialize the equality:
    areValuesEqual = true;

    // Get the other snapshot filter vector:
    const gtVector<apOpenGLStateVariableId>* pOtherStateVariablesFilter = otherSnapshot._pStateVariablesFilteredIdsVector;

    GT_IF_WITH_ASSERT(_pStateVariablesFilteredIdsVector != NULL && pOtherStateVariablesFilter != NULL)
    {
        // Get the sizes of the both vectors:
        size_t filteredStateVariableAmount = _pStateVariablesFilteredIdsVector->size();
        size_t otherFilteredStateVariablesAmount = otherSnapshot._pStateVariablesFilteredIdsVector->size();

        GT_IF_WITH_ASSERT(filteredStateVariableAmount == otherFilteredStateVariablesAmount)
        {
            // Iterate the filtered state variables:
            for (size_t j = 0; j < otherFilteredStateVariablesAmount; j++)
            {
                // Get the current filtered state variable id:
                int currentVariableId = (*_pStateVariablesFilteredIdsVector)[j];

                // Get the current state variable value:
                const apParameter* pParam1 = NULL, *pParam2 = NULL;
                bool rc1 = getStateVariableValue(currentVariableId, pParam1);
                bool rc2 = otherSnapshot.getStateVariableValue(currentVariableId, pParam2);

                GT_IF_WITH_ASSERT(rc1 && rc2 && (pParam1 != NULL) && (pParam2 != NULL))
                {
                    // Compare the parameters values one to another:
                    areValuesEqual = pParam1->compareToOther(*pParam2);

                    // If values are different, stop comparing:
                    if (!areValuesEqual)
                    {
                        break;
                    }
                }
            }

            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apStateVariablesSnapShot::getStringValue
// Description: returns a string value of the requested state variable value
// Arguments: int id
//            gtString& val
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/6/2008
// ---------------------------------------------------------------------------
bool apStateVariablesSnapShot::getStringValue(int id, gtString& val) const
{
    bool retVal = false;

    if (id >= 0 && id < (int)apOpenGLStateVariablesAmount)
    {
        if (_isStateVariableAvailable[id])
        {
            apParameter* pParam = _stateVariableValues[id];
            GT_IF_WITH_ASSERT(pParam != NULL)
            {
                pParam->valueAsString(val);
                retVal = true;
            }
        }
        else
        {
            _staticNotAvailableParameter.valueAsString(val);
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsStateVariablesSnapshot::getStateVariableValue
// Description: Returns the value of a state variable as stored in this snapshot.
// Arguments:   stateVariableId - The id of the requested state variable.
//              pStateVariableValue - Will get the state variable value.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        17/7/2004
// ---------------------------------------------------------------------------
bool apStateVariablesSnapShot::getStateVariableValue(int stateVariableId,
                                                     const apParameter*& pStateVariableValue) const
{
    bool retVal = false;
    pStateVariableValue = NULL;

#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
    GT_IF_WITH_ASSERT((stateVariableId >= 0) && (stateVariableId < (int)apOpenGLStateVariablesAmount))
#endif
    {
        if (_stateVariableValues[stateVariableId] != NULL)
        {
            if (_isStateVariableAvailable[stateVariableId] == true)
            {
                pStateVariableValue = _stateVariableValues[stateVariableId];
            }
            else
            {
                pStateVariableValue = &_staticNotAvailableParameter;
            }

            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apStateVariablesSnapShot::isStateVariableSupported
// Description: Checks if the the state variable should be treated
// Arguments: int id - the state variable id
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        4/9/2008
// ---------------------------------------------------------------------------
bool apStateVariablesSnapShot::isStateVariableSupported(int id) const
{
    bool retVal = false;

    // Sanity check - the state variable id is within the right range:
#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
    GT_IF_WITH_ASSERT((id >= 0) && (id < apOpenGLStateVariablesAmount))
#endif
    {
        // If the snapshot treats only filtered ids:
        if (_pStateVariablesFilteredIdsVector != NULL)
        {
            // Search the id within the filtered ids list:
            for (size_t i = 0; i < _pStateVariablesFilteredIdsVector->size(); i++)
            {
                if ((*_pStateVariablesFilteredIdsVector)[i] == id)
                {
                    retVal = _isStateVariableSupported[id];
                    break;
                }
            }
        }
        else
        {
            // Take the answer from the supported state variables list:
            retVal = _isStateVariableSupported[id];
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apStateVariablesSnapShot::supportOnlyFilteredStateVariableIds
// Description: Set the snapshot relation to state variables - should the snapshot treat all state variables, or if it only needs to treat
//              filtered ids:
// Arguments: gtVector<apOpenGLStateVariableId>* pFilteredStateVariableIds - a pointer to the vector of filtered ids
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        4/9/2008
// ---------------------------------------------------------------------------
bool apStateVariablesSnapShot::supportOnlyFilteredStateVariableIds(gtVector<apOpenGLStateVariableId>* pFilteredStateVariableIds)
{
    bool retVal = false;

    // If the snapshot should currently treat only specific state variables:
    if (pFilteredStateVariableIds != NULL)
    {
        pFilteredStateVariableIds = pFilteredStateVariableIds;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apStateVariablesSnapShot::deleteParameter
// Description: Deletes a parameter if the parameter is not the static not available
//              parameter.
// Arguments: apParameter* pParam
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        16/9/2008
// ---------------------------------------------------------------------------
void apStateVariablesSnapShot::deleteParameter(apParameter* pParam)
{
    if ((pParam != &apStateVariablesSnapShot::_staticNotAvailableParameter) && (pParam != &_staticNotSupportedParameter))
    {
        delete pParam;
    }
}

