//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLXParameters.h
///
//==================================================================================

//------------------------------ apGLXParameters.h ------------------------------

#ifndef __APGLXPARAMETERS_H
#define __APGLXPARAMETERS_H

// Forward declarations:
class osChannel;

// Infra:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Local:
#include <AMDTAPIClasses/Include/apBasicParameters.h>
#include <AMDTAPIClasses/Include/apOpenGLParameters.h>


// ----------------------------------------------------------------------------------
// Class Name:          AP_API apGLXenumParameter : public apOpenGLParameter
// General Description: Represents a glX-specific enum value. This class needs to be
//                      separate from apGLenumParameter since the is some reuse of values
//                      (mostly in the 0x000080## range)
// Author:  AMD Developer Tools Team
// Creation Date:       6/7/2009
// ----------------------------------------------------------------------------------
class AP_API apGLXenumParameter : public apOpenGLParameter
{
public:
    // Self functions:
    apGLXenumParameter(int val = 0) : _value(val) {};
    int value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

    // Overrides apOpenGLParameter:
    virtual void setValueFromInt(GLint val);
    virtual void setValueFromFloat(GLfloat val);

private:
    // The parameter value:
    int _value;
};


#endif //__APGLXPARAMETERS_H

