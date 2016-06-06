//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apFunctionCall.cpp
///
//==================================================================================

//------------------------------ apFunctionCall.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apMonitoredFunctionsManager.h>
#include <AMDTAPIClasses/Include/apFunctionCall.h>


// ---------------------------------------------------------------------------
// Name:        apFunctionCall::apFunctionCall
// Description: Constructor.
// Arguments:   functionId - The function ID
// Author:  AMD Developer Tools Team
// Date:        9/4/2004
// ---------------------------------------------------------------------------
apFunctionCall::apFunctionCall(apMonitoredFunctionId functionId)
    : _functionId(functionId), _functionRedundancyStatus(AP_REDUNDANCY_UNKNOWN), _functionDeprecationStatus(AP_DEPRECATION_NONE)
{
}


// ---------------------------------------------------------------------------
// Name:        apFunctionCall::~apFunctionCall
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        8/4/2004
// ---------------------------------------------------------------------------
apFunctionCall::~apFunctionCall()
{
    // Delete the function arguments:
    while (!_arguments.empty())
    {
        // Get the current argument:
        const apParameter* pCurrentArgument = _arguments.back();
        _arguments.pop_back();

        // Delete it:
        delete pCurrentArgument;
    }

    // Delete the additional parameters:
    while (!(_additionalDataParameters.empty()))
    {
        // Delete the current pseudo parameter:
        apPseudoParameter* pCurrentParam = (apPseudoParameter*)(_additionalDataParameters.front());
        _additionalDataParameters.pop_front();
        delete pCurrentParam;
    }
}


// ---------------------------------------------------------------------------
// Name:        apFunctionCall::addArgument
// Description: Adds an argument to this function call.
// Arguments:   apArgument - AutoPtr that holds the input argument.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
void apFunctionCall::addArgument(gtAutoPtr<apParameter>& apArgument)
{
    // Get the argument ownership from the AutoPtr:
    apParameter* pArgument = apArgument.releasePointedObjectOwnership();

    // Add the argument to the arguments list:
    _arguments.push_back(pArgument);
}


// ---------------------------------------------------------------------------
// Name:        apFunctionCall::addAdditionalDataParameter
// Description: Adds an additional data parameter to this function call.
// Arguments:   apDataParameter - AutoPtr that holds the input data parameter.
// Author:  AMD Developer Tools Team
// Date:        6/1/2005
// ---------------------------------------------------------------------------
void apFunctionCall::addAdditionalDataParameter(gtAutoPtr<apPseudoParameter>& apDataParameter)
{
    // Get the pseudo parameter ownership from the AutoPtr:
    apPseudoParameter* pPseudoParameter = apDataParameter.releasePointedObjectOwnership();

    // Add the argument to the arguments list:
    _additionalDataParameters.push_back(pPseudoParameter);
}


// ---------------------------------------------------------------------------
// Name:        apFunctionCall::asString
// Description: Returns a displayable string that represents this function call.
//              The string is of the form: funcName(arg1, arg2, ... , argn)
// Author:  AMD Developer Tools Team
// Date:        15/12/2004
// ---------------------------------------------------------------------------
void apFunctionCall::asString(gtString& funcCallAsString) const
{
    funcCallAsString.makeEmpty();

    // Get the function name:
    static apMonitoredFunctionsManager& monitoredFuncMgr = apMonitoredFunctionsManager::instance();
    funcCallAsString += monitoredFuncMgr.monitoredFunctionName(_functionId);

    // Start the function arguments section:
    funcCallAsString += '(';

    // Iterate the function arguments:
    gtList<const apParameter*>::const_iterator iter = _arguments.begin();
    gtList<const apParameter*>::const_iterator endIter = _arguments.end();

    gtString currentArgumentValueAsString;

    while (iter != endIter)
    {
        // Get the current argument value (as a string):
        (*(*iter)).valueAsString(currentArgumentValueAsString);

        // Add it to the function call string:
        funcCallAsString += currentArgumentValueAsString;
        iter++;

        // Add the "," only if it is NOT the last parameter
        if (iter != endIter)
        {
            funcCallAsString += L", ";
        }
    }

    // End the function arguments section:
    funcCallAsString += L")";
}


// ---------------------------------------------------------------------------
// Name:        apFunctionCall::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        9/4/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apFunctionCall::type() const
{
    return OS_TOBJ_ID_FUNCTION_CALL;
}


// ---------------------------------------------------------------------------
// Name:        apFunctionCall::writeSelfIntoChannel
// Description: Writes this class into an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        9/4/2004
// ---------------------------------------------------------------------------
bool apFunctionCall::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    // Write the function ID:
    ipcChannel << (gtInt32)_functionId;

    // Write the function redundancy status:
    ipcChannel << (gtInt32)_functionRedundancyStatus;

    // Write the function deprecation status:
    ipcChannel << (gtInt32)_functionDeprecationStatus;

    // Write the function arguments amount:
    gtInt64 amountOfArguments = (gtInt64)_arguments.size();
    ipcChannel << amountOfArguments;

    // Write the function arguments:
    gtList<const apParameter*>::const_iterator iter = _arguments.begin();
    gtList<const apParameter*>::const_iterator endIter = _arguments.end();

    while (iter != endIter)
    {
        // Get the current parameter (as non const):
        const apParameter* pCurrentParameter = (apParameter*)(*iter);

        // Write the parameter:
        ipcChannel << *pCurrentParameter;
        iter++;
    }

    // Write the amount of additional data parameters:
    gtInt64 amountOfAdditionalParameters = (gtInt64)_additionalDataParameters.size();
    ipcChannel << amountOfAdditionalParameters;

    // Write the additional data parameters:
    gtList<const apPseudoParameter*>::const_iterator additionalDataIter = _additionalDataParameters.begin();
    gtList<const apPseudoParameter*>::const_iterator additionalDataEndIter = _additionalDataParameters.end();

    while (additionalDataIter != additionalDataEndIter)
    {
        // Get the current parameter (as non const):
        const apPseudoParameter* pCurrentDataParameter = (apPseudoParameter*)(*additionalDataIter);

        // Write the parameter:
        ipcChannel << *pCurrentDataParameter;
        additionalDataIter++;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apFunctionCall::readSelfFromChannel
// Description: Reads self from an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        9/4/2004
// ---------------------------------------------------------------------------
bool apFunctionCall::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    // Read the function ID:
    gtInt32 functionIdAsInt32 = 0;
    ipcChannel >> functionIdAsInt32;
    _functionId = (apMonitoredFunctionId)functionIdAsInt32;

    // Read the function redundancy status:
    gtInt32 functionRedundancyStatus;
    ipcChannel >> functionRedundancyStatus;
    _functionRedundancyStatus = (apFunctionRedundancyStatus)functionRedundancyStatus;

    // Read the function deprecation status:
    gtInt32 functionDeprecationStatus;
    ipcChannel >> functionDeprecationStatus;
    _functionDeprecationStatus = (apFunctionDeprecationStatus)functionDeprecationStatus;

    // Read the function arguments amount:
    gtInt64 amountOfArguments = 0;
    ipcChannel >> amountOfArguments;

    // Read the function arguments:
    for (int i = 0; i < amountOfArguments; i++)
    {
        // Read the current argument:
        gtAutoPtr<osTransferableObject> aptrObject;
        ipcChannel >> aptrObject;

        // Get the ownership of the argument memory:
        apParameter* pCreatedObject = (apParameter*)(aptrObject.releasePointedObjectOwnership());

        // Verify that the object was created succesfully:
        retVal = retVal && (pCreatedObject != NULL);

        if (retVal)
        {
            // Add the argument to our arguments list:
            _arguments.push_back(pCreatedObject);
        }
    }

    // Read amount of additional data parameters:
    gtInt64 amountOfAdditionalDataParameters = 0;
    ipcChannel >> amountOfAdditionalDataParameters;

    // Read additional data parameters:
    for (int i = 0; i < amountOfAdditionalDataParameters; i++)
    {
        // Read the current data parameter:
        gtAutoPtr<osTransferableObject> aptrObject;
        ipcChannel >> aptrObject;

        // Get the ownership of the argument memory:
        apPseudoParameter* pCreatedObject = (apPseudoParameter*)(aptrObject.releasePointedObjectOwnership());

        // Verify that the object was created succesfully:
        retVal = retVal && (pCreatedObject != NULL);

        if (retVal)
        {
            // Add the argument to our arguments list:
            _additionalDataParameters.push_back(pCreatedObject);
        }
    }

    return retVal;
}

