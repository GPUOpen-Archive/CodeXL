//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apStateVariableValue.h
///
//==================================================================================

//------------------------------ apStateVariableValue.h ------------------------------

#ifndef __APSTATEVARIABLEVALUE
#define __APSTATEVARIABLEVALUE

// Infra:
#include <AMDTOSWrappers/Include/osTransferableObject.h>

// Local:
#include <AMDTAPIClasses/Include/apBasicParameters.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apStateVariableValue
// General Description:
//   Represents a state variable value
// Author:  AMD Developer Tools Team
// Creation Date:        26/6/2008
// ----------------------------------------------------------------------------------
class AP_API apStateVariableValue : public osTransferableObject
{
public:
    apStateVariableValue();
    apStateVariableValue(int id, apParameter* pValue);
    ~apStateVariableValue();


    virtual osTransferableObjectType type() const;

    // Get / Set value:
    apParameter* getValue() const { return _pValue; };
    void setValue(apParameter* pValue) { _pValue = pValue; };

    // Get / Set ID:
    int getID() const { return _id; };
    void setID(int id) { _id = id; };

    // Overrides osTransferableObject:
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

private:

    int          _id;
    apParameter* _pValue;
};


#endif  // __APSTATEVARIABLEVALUE
