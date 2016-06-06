//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apStateVariablesSnapShot.h
///
//==================================================================================

//------------------------------ apStateVariablesSnapShot.h ------------------------------

#ifndef __apStateVariablesSnapShot
#define __apStateVariablesSnapShot

// Infra:
#include <AMDTOSWrappers/Include/osTransferableObject.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTAPIClasses/Include/apOpenGLStateVariableId.h>
#include <AMDTAPIClasses/Include/apParameters.h>
#include <AMDTBaseTools/Include/gtVector.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apStateVariablesSnapShot
// General Description:
//   Represents the program state variables snapshot in certain time
// Author:  AMD Developer Tools Team
// Creation Date:        26/6/2008
// ----------------------------------------------------------------------------------
class AP_API apStateVariablesSnapShot : public osTransferableObject
{
public:
    apStateVariablesSnapShot();
    ~apStateVariablesSnapShot();

    virtual osTransferableObjectType type() const;
    bool compareToOther(const apStateVariablesSnapShot& otherSnapshot, bool& areValuesEqual) const;

    void clearContextDataSnapshot();

    // Overrides osTransferableObject:
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Get / Add value
    bool addStateVariableValue(int id, apParameter* pParamVal);
    bool addStateVariableValue(int id, gtString val);
    bool getStringValue(int id, gtString& val) const;
    bool getStateVariableValue(int stateVariableId, const apParameter*& pStateVariableValue) const;

    // Set that this snapshot is read from file:
    void setIsReadFromFile(bool isReadFromFile);

    // Should the state variable be treated:
    bool isStateVariableSupported(int id) const;

    // Set the snapshot relation to state variables - should the snapshot treat all state variables, or if it only needs to treat
    // filtered ids:
    bool supportOnlyFilteredStateVariableIds(gtVector<apOpenGLStateVariableId>* pFilteredStateVariableIds);

    int size() const;
    void clear();

    static apNotAvailableParameter _staticNotAvailableParameter;
    static apNotSupportedParameter _staticNotSupportedParameter;
    static void deleteParameter(apParameter* pParam);

private:
    bool compareOnlyFilteredToOther(const apStateVariablesSnapShot& otherSnapshot, bool& areValuesEqual) const;
    bool compareAllToOther(const apStateVariablesSnapShot& otherSnapshot, bool& areValuesEqual) const;

protected:
    // Holds the state variable values:
    apParameter* _stateVariableValues[apOpenGLStateVariablesAmount];

    // Index i contains true iff state variable i is supported by the context that
    // was current when updateStateVariablesSupport was called:
    bool _isStateVariableSupported[apOpenGLStateVariablesAmount];


    // Index i contains true iff state variable i is available currently:
    bool _isStateVariableAvailable[apOpenGLStateVariablesAmount];

    // Was the snapshot values read from file (it means that the values are strings):
    bool _isReadFromFile;

    // Vector containing the filtered state variable ids currently needed to be updated and compared:
    const gtVector<apOpenGLStateVariableId>* _pStateVariablesFilteredIdsVector;
};


#endif  // __APSTATEVARIABLESSNAPSHOT
