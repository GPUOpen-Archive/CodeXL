//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apOSRelatedParameters.h
///
//==================================================================================

//------------------------------ apOSRelatedParameters.h ------------------------------

#ifndef __APOSRELATEDPARAMETERS_H
#define __APOSRELATEDPARAMETERS_H

// Standard C:
#include <stdarg.h>

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osTransferableObject.h>

// Linux generic variant only:
#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT)
    // Yaki 7/2/2007:
    // We copy this definition from xorg/vdif.h, since vdiff makes also other defines in a way
    // that collides with other libraries (example, it defines BOOL without checking if it was
    // not already defined.
    #define CARD32 unsigned int

    #include <X11/Xlib.h>
    #include <X11/X.h>
#elif (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
    #include <AMDTOSWrappers/Include/osOSDefinitions.h>
#endif

// Local:
#include <AMDTAPIClasses/Include/apBasicParameters.h>



// -------------------  Win32 parameters ---------------------

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apWin32BOOLParameter : public apParameter
// General Description: Represents a Win32 BOOL parameter.
// Author:  AMD Developer Tools Team
// Creation Date:        28/4/2004
// ----------------------------------------------------------------------------------
class AP_API apWin32BOOLParameter : public apParameter
{
public:
    // Self functions:
    apWin32BOOLParameter(BOOL value = FALSE) : _value(value) {};
    BOOL value() const { return _value; };

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

private:
    // The parameter value:
    BOOL _value;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apWin32FLOATParameter : public apParameter
// General Description: Represents a Win32 FLOAT parameter.
// Author:  AMD Developer Tools Team
// Creation Date:        30/10/2005
// ----------------------------------------------------------------------------------
class AP_API apWin32FLOATParameter : public apParameter
{
public:
    // Self functions:
    apWin32FLOATParameter(FLOAT value = 0.0f) : _value(value) {};
    FLOAT value() const { return _value; };

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

private:
    // The parameter value:
    FLOAT _value;
};

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apWin32UINTParameter : public apParameter
// General Description: Represents a Win32 UINT parameter.
// Author:  AMD Developer Tools Team
// Creation Date:        28/4/2004
// ----------------------------------------------------------------------------------
class AP_API apWin32UINTParameter : public apParameter
{
public:
    // Self functions:
    apWin32UINTParameter(UINT value = FALSE) : _value(value) {};
    UINT value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual void valueAsHexString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

private:
    // The parameter value:
    UINT _value;
};


// ----------------------------------------------------------------------------------
// Class Name:          AP_API apWin32INTParameter : public apParameter
// General Description: Represents a Win32 INT parameter.
// Author:  AMD Developer Tools Team
// Creation Date:       22/3/2009
// ----------------------------------------------------------------------------------
class AP_API apWin32INTParameter : public apParameter
{
public:
    // Self functions:
    apWin32INTParameter(INT value = FALSE) : _value(value) {};
    INT value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual void valueAsHexString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

private:
    // The parameter value:
    INT _value;
};

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apWin32DWORDParameter : public apParameter
// General Description: Represents a Win32 DWORD parameter.
// Author:  AMD Developer Tools Team
// Creation Date:        28/4/2004
// ----------------------------------------------------------------------------------
class AP_API apWin32DWORDParameter : public apParameter
{
public:
    // Self functions:
    apWin32DWORDParameter(DWORD value = FALSE) : _value(value) {};
    DWORD value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual void valueAsHexString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

private:
    // The parameter value:
    DWORD _value;
};

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apWin32HANDLEParameter : public apParameter
// General Description: Represents a Win32 HANDLE parameter.
// Author:  AMD Developer Tools Team
// Creation Date:        8/9/2008
// ----------------------------------------------------------------------------------
class AP_API apWin32HANDLEParameter : public apParameter
{
public:
    // Self functions:
    apWin32HANDLEParameter(HANDLE value = FALSE) : _value(value) {};
    HANDLE value() const { return _value; };

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

private:
    // The parameter value:
    HANDLE _value;
};

// -------------------  Linux generic variant parameters ---------------------

#elif (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT)


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apX11BoolParamter : public apParameter
// General Description: Represents an X11 Bool parameter, as defined in /usr/include/X11/Xlib.h.
// Author:  AMD Developer Tools Team
// Creation Date:        7/2/2007
// ----------------------------------------------------------------------------------
class AP_API apX11BoolParamter : public apParameter
{
public:
    // Self functions:
    apX11BoolParamter(Bool value = False) : _value(value) {};
    Bool value() const { return _value; };

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

private:
    // The parameter value:
    Bool _value;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apXOrgCARD32Paramter : public apParameter
// General Description: Represents an XOrg CARD32 parameter, as defined in /usr/include/xorg/vdif.h.
// Author:  AMD Developer Tools Team
// Creation Date:        7/2/2007
// ----------------------------------------------------------------------------------
class AP_API apXOrgCARD32Paramter : public apParameter
{
public:
    // Self functions:
    apXOrgCARD32Paramter(CARD32 value = 0) : _value(value) {};
    CARD32 value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual void valueAsHexString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

private:
    // The parameter value:
    CARD32 _value;
};

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apXIDParamter : public apParameter
// General Description: Represents an XID parameter, as defined in /usr/include/X11/X.h.
// Author:  AMD Developer Tools Team
// Creation Date:        5/5/2007
// ----------------------------------------------------------------------------------
class AP_API apXIDParamter : public apParameter
{
public:
    // Self functions:
    apXIDParamter(XID value = 0) : _value(value) {};
    XID value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual void valueAsHexString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

private:
    // The parameter value:
    XID _value;
};

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apLongBitfieldParameter : public apParameter
// General Description: Represents an unsigned long parameter (used in glX as mask).
// Author:  AMD Developer Tools Team
// Date:        6/5/2008
// ----------------------------------------------------------------------------------
class AP_API apLongBitfieldParameter : public apParameter
{
public:
    // Self functions:
    apLongBitfieldParameter(unsigned long value = 0) : _value(value) {};
    unsigned long value() const { return _value; };

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

private:
    // The parameter value:
    unsigned long _value;
};
#elif (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
// Mac-only parameters

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apEAGLRenderingAPIParameter : public apParameter
// General Description: Represents a EAGLRenderingAPI parameter. This enum is used
//                      to create EAGL contexts, and currently (13/07/09) has only 2
//                      values: one for OpenGL ES 1.1 and one for OpenGL ES 2.0
// Author:  AMD Developer Tools Team
// Creation Date:       13/7/2009
// ----------------------------------------------------------------------------------
class AP_API apEAGLRenderingAPIParameter : public apParameter
{
public:
    // Self functions:
    apEAGLRenderingAPIParameter(EAGLRenderingAPI value = 0) : _value(value) {};
    EAGLRenderingAPI value() const { return _value; };

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

private:
    // The parameter value:
    EAGLRenderingAPI _value;
};

#endif // AMDT_BUILD_TARGET 


#endif //__APOSRELATEDPARAMETERS_H

