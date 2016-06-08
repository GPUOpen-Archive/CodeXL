//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apFunctionCall.h
///
//==================================================================================

//------------------------------ apFunctionCall.h ------------------------------

#ifndef __APFUNCTIONCALL
#define __APFUNCTIONCALL


// Infra
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtAutoPtr.h>
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTAPIClasses/Include/apFunctionType.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>
#include <AMDTOSWrappers/Include/osTransferableObject.h>

// Local:
#include <AMDTAPIClasses/Include/apFunctionDeprecation.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionId.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apFunctionCall : public osTransferableObject
//
// General Description:
//   Represents an OpenGL / other function call.
//
// Author:  AMD Developer Tools Team
// Creation Date:        8/4/2004
// ----------------------------------------------------------------------------------
class AP_API apFunctionCall : public osTransferableObject
{
public:
    apFunctionCall(apMonitoredFunctionId functionId = apMonitoredFunctionsAmount);
    ~apFunctionCall();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Set functions:
    void asString(gtString& funcCallAsString) const;
    apMonitoredFunctionId functionId() const { return _functionId; };
    void addArgument(gtAutoPtr<apParameter>& apArgument);
    const gtList<const apParameter*>& arguments() const { return _arguments; };

    // Additional parameters that are attached to this function call:
    // (Associated texture, associated shader, associated program, etc)
    void addAdditionalDataParameter(gtAutoPtr<apPseudoParameter>& apDataParameter);
    const gtList<const apPseudoParameter*>& additionalDataParameters() const { return _additionalDataParameters; };

    // Function redundancy status:
    void setRedundanctStatus(apFunctionRedundancyStatus redundancyStatus) { _functionRedundancyStatus = redundancyStatus; };
    apFunctionRedundancyStatus getRedundanctStatus() const { return _functionRedundancyStatus; };

    // Function deprecation status:
    void setDeprecationStatus(apFunctionDeprecationStatus deprecationStatus) { _functionDeprecationStatus = deprecationStatus; };
    apFunctionDeprecationStatus getDeprecationStatus() const { return _functionDeprecationStatus; };

private:
    // The function Id:
    apMonitoredFunctionId _functionId;

    // Redundancy status:
    apFunctionRedundancyStatus _functionRedundancyStatus;

    // Deprecation status:
    apFunctionDeprecationStatus _functionDeprecationStatus;

    // Contains the function arguments
    gtList<const apParameter*> _arguments;

    // Contains additional data that is attached to this function:
    gtList<const apPseudoParameter*> _additionalDataParameters;
};


#endif  // __APFUNCTIONCALL
