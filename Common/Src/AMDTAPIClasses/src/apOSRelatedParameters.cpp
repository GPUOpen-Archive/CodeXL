//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apOSRelatedParameters.cpp
///
//==================================================================================

//------------------------------ apOSRelatedParameters.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osTransferableObjectCreatorsManager.h>

// Local:
#include <AMDTAPIClasses/Include/apOSRelatedParameters.h>
#include <AMDTAPIClasses/Include/apStringConstants.h>


// Below are Win32 parameter types:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

// -----------------------------   apWin32BOOLParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apWin32BOOLParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apWin32BOOLParameter::type() const
{
    return OS_TOBJ_ID_WIN32_BOOL_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apWin32BOOLParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apWin32BOOLParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtInt32)_value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apWin32BOOLParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apWin32BOOLParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtInt32 valueAsInt32 = 0;
    ipcChannel >> valueAsInt32;
    _value = (BOOL)valueAsInt32;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apWin32BOOLParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        5/5/2004
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apWin32BOOLParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    long argumentValue = va_arg(pArgumentList , long);
    _value = (BOOL)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apWin32BOOLParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
void apWin32BOOLParameter::readValueFromPointer(void* pValue)
{
    _value = *((BOOL*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apWin32BOOLParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
gtSizeType apWin32BOOLParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(BOOL);
    return sizeOfValue;
}


// ---------------------------------------------------------------------------
// Name:        apWin32BOOLParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
void apWin32BOOLParameter::valueAsString(gtString& valueString) const
{
    if (_value == FALSE)
    {
        valueString = AP_STR_FALSE;
    }
    else
    {
        valueString = AP_STR_TRUE;
    }
}

// ---------------------------------------------------------------------------
// Name:        apWin32BOOLParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apWin32BOOLParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apWin32BOOLParameter* pParam  = (apWin32BOOLParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}


// -----------------------------   apWin32FLOATParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apWin32FLOATParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apWin32FLOATParameter::type() const
{
    return OS_TOBJ_ID_WIN32_FLOAT_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apWin32FLOATParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apWin32FLOATParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apWin32FLOATParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apWin32FLOATParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apWin32FLOATParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        5/5/2004
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apWin32FLOATParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    double argumentValue = va_arg(pArgumentList , double);
    _value = (FLOAT)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apWin32FLOATParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
void apWin32FLOATParameter::readValueFromPointer(void* pValue)
{
    _value = *((FLOAT*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apWin32FLOATParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
gtSizeType apWin32FLOATParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(FLOAT);
    return sizeOfValue;
}


// ---------------------------------------------------------------------------
// Name:        apWin32FLOATParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
void apWin32FLOATParameter::valueAsString(gtString& valueString) const
{
    valueString.makeEmpty();
    const wchar_t* pFloatParamFormatString = apGetFloatParamsFormatString();
    valueString.appendFormattedString(pFloatParamFormatString, _value);
}


// ---------------------------------------------------------------------------
// Name:        apWin32FLOATParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apWin32FLOATParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apWin32FLOATParameter* pParam  = (apWin32FLOATParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}


// -----------------------------   apWin32UINTParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apWin32UINTParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apWin32UINTParameter::type() const
{
    return OS_TOBJ_ID_WIN32_UINT_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apWin32UINTParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apWin32UINTParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt32)_value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apWin32UINTParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apWin32UINTParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt32 valueAsUInt32 = 0;
    ipcChannel >> valueAsUInt32;
    _value = (UINT)valueAsUInt32;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apWin32UINTParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        5/5/2004
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apWin32UINTParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    unsigned long argumentValue = va_arg(pArgumentList , unsigned long);
    _value = (UINT)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apWin32UINTParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
void apWin32UINTParameter::readValueFromPointer(void* pValue)
{
    _value = *((UINT*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apWin32UINTParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
gtSizeType apWin32UINTParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(UINT);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apWin32UINTParameter::valueAsHexString
// Description: Returns the parameter value as a string in hexadecimal display
// Arguments:   gtString& valueString
// Author:  AMD Developer Tools Team
// Date:        14/4/2011
// ---------------------------------------------------------------------------
void apWin32UINTParameter::valueAsHexString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(GT_UNSIGNED_INT_HEXADECIMAL_8_CHAR_FORMAT, (gtUInt32)_value);
}

// ---------------------------------------------------------------------------
// Name:        apWin32UINTParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
void apWin32UINTParameter::valueAsString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(L"%u", _value);
}


// ---------------------------------------------------------------------------
// Name:        apWin32UINTParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apWin32UINTParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apWin32UINTParameter* pParam  = (apWin32UINTParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}


// -----------------------------   apWin32INTParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apWin32INTParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        22/3/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apWin32INTParameter::type() const
{
    return OS_TOBJ_ID_WIN32_INT_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apWin32INTParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        22/3/2009
// ---------------------------------------------------------------------------
bool apWin32INTParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtInt32)_value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apWin32INTParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        22/3/2009
// ---------------------------------------------------------------------------
bool apWin32INTParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtInt32 valueAsInt32 = 0;
    ipcChannel >> valueAsInt32;
    _value = (INT)valueAsInt32;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apWin32INTParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        22/3/2009
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apWin32INTParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    unsigned long argumentValue = va_arg(pArgumentList , unsigned long);
    _value = (INT)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apWin32INTParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        22/3/2009
// ---------------------------------------------------------------------------
void apWin32INTParameter::readValueFromPointer(void* pValue)
{
    _value = *((UINT*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apWin32INTParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        22/3/2009
// ---------------------------------------------------------------------------
gtSizeType apWin32INTParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(UINT);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apWin32INTParameter::valueAsHexString
// Description: Returns the parameter value as a string in hexadecimal display
// Arguments:   gtString& valueString
// Author:  AMD Developer Tools Team
// Date:        14/4/2011
// ---------------------------------------------------------------------------
void apWin32INTParameter::valueAsHexString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(GT_UNSIGNED_INT_HEXADECIMAL_8_CHAR_FORMAT, (gtUInt32)_value);
}

// ---------------------------------------------------------------------------
// Name:        apWin32INTParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        22/3/2009
// ---------------------------------------------------------------------------
void apWin32INTParameter::valueAsString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(L"%d", _value);
}


// ---------------------------------------------------------------------------
// Name:        apWin32INTParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        22/3/2009
// ---------------------------------------------------------------------------
bool apWin32INTParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apWin32INTParameter* pParam  = (apWin32INTParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// -----------------------------   apWin32DWORDParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apWin32DWORDParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apWin32DWORDParameter::type() const
{
    return OS_TOBJ_ID_WIN32_DWORD_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apWin32DWORDParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apWin32DWORDParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt32)_value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apWin32DWORDParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apWin32DWORDParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt32 valueAsUInt32 = 0;
    ipcChannel >> valueAsUInt32;
    _value = (DWORD)valueAsUInt32;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apWin32DWORDParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        5/5/2004
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apWin32DWORDParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    unsigned long argumentValue = va_arg(pArgumentList , unsigned long);
    _value = (DWORD)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apWin32DWORDParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
void apWin32DWORDParameter::readValueFromPointer(void* pValue)
{
    _value = *((DWORD*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apWin32DWORDParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
gtSizeType apWin32DWORDParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(DWORD);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apWin32DWORDParameter::valueAsHexString
// Description: Returns the parameter value as a string in hexadecimal display
// Arguments:   gtString& valueString
// Author:  AMD Developer Tools Team
// Date:        14/4/2011
// ---------------------------------------------------------------------------
void apWin32DWORDParameter::valueAsHexString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(L"0x%0.16llx", (gtUInt64)_value);
}

// ---------------------------------------------------------------------------
// Name:        apWin32DWORDParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
void apWin32DWORDParameter::valueAsString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(L"%u", _value);
}


// ---------------------------------------------------------------------------
// Name:        apWin32FLOATParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apWin32DWORDParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apWin32DWORDParameter* pParam  = (apWin32DWORDParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// -----------------------------   apWin32HANDLEParameter ------------------------------

// ---------------------------------------------------------------------------
// Name:        apWin32HANDLEParameter::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        8/9/2008
// ---------------------------------------------------------------------------
osTransferableObjectType apWin32HANDLEParameter::type() const
{
    return OS_TOBJ_ID_WIN32_HANDLE_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apWin32HANDLEParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        8/9/2008
// ---------------------------------------------------------------------------
bool apWin32HANDLEParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt64)_value;
    return true;
}

// ---------------------------------------------------------------------------
// Name:        apWin32HANDLEParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        8/9/2008
// ---------------------------------------------------------------------------
bool apWin32HANDLEParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt64 valueAsUInt64 = 0;
    ipcChannel >> valueAsUInt64;
    _value = (HANDLE)valueAsUInt64;

    return true;
}

// ---------------------------------------------------------------------------
// Name:        apWin32HANDLEParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        8/9/2008
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apWin32HANDLEParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    size_t argumentValue = va_arg(pArgumentList , size_t);
    _value = HANDLE(argumentValue);
}

// ---------------------------------------------------------------------------
// Name:        apWin32HANDLEParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        8/9/2008
// ---------------------------------------------------------------------------
void apWin32HANDLEParameter::readValueFromPointer(void* pValue)
{
    _value = *((HANDLE*)(pValue));
}

// ---------------------------------------------------------------------------
// Name:        apWin32HANDLEParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        8/9/2008
// ---------------------------------------------------------------------------
gtSizeType apWin32HANDLEParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(HANDLE);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apWin32HANDLEParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        8/9/2008
// ---------------------------------------------------------------------------
void apWin32HANDLEParameter::valueAsString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(L"%lu", _value);
}

// ---------------------------------------------------------------------------
// Name:        apWin32HANDLEParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        8/9/2008
// ---------------------------------------------------------------------------
bool apWin32HANDLEParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apWin32HANDLEParameter* pParam  = (apWin32HANDLEParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// Below are Linux generic variant parameter types:
#elif (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT)


// ----------------------------- apX11BoolParamter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apX11BoolParamter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        7/1/2007
// ---------------------------------------------------------------------------
osTransferableObjectType apX11BoolParamter::type() const
{
    return OS_TOBJ_ID_X11_BOOL_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apX11BoolParamter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        7/1/2007
// ---------------------------------------------------------------------------
bool apX11BoolParamter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apX11BoolParamter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        7/1/2007
// ---------------------------------------------------------------------------
bool apX11BoolParamter::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apX11BoolParamter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        7/1/2007
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apX11BoolParamter::readValueFromArgumentsList(va_list& pArgumentList)
{
    long argumentValue = va_arg(pArgumentList , long);
    _value = (Bool)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apX11BoolParamter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        7/1/2007
// ---------------------------------------------------------------------------
void apX11BoolParamter::readValueFromPointer(void* pValue)
{
    _value = *((Bool*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apX11BoolParamter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        7/1/2007
// ---------------------------------------------------------------------------
gtSizeType apX11BoolParamter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(Bool);
    return sizeOfValue;
}


// ---------------------------------------------------------------------------
// Name:        apX11BoolParamter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        7/1/2007
// ---------------------------------------------------------------------------
void apX11BoolParamter::valueAsString(gtString& valueString) const
{
    if (_value == False)
    {
        valueString = AP_STR_False;
    }
    else
    {
        valueString = AP_STR_True;
    }
}

// ---------------------------------------------------------------------------
// Name:        apX11BoolParamter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apX11BoolParamter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apX11BoolParamter* pParam  = (apX11BoolParamter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// ----------------------------- apXOrgCARD32Paramter ------------------------------



// ---------------------------------------------------------------------------
// Name:        apXOrgCARD32Paramter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        7/1/2007
// ---------------------------------------------------------------------------
osTransferableObjectType apXOrgCARD32Paramter::type() const
{
    return OS_TOBJ_ID_XORG_CARD32_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apXOrgCARD32Paramter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        7/1/2007
// ---------------------------------------------------------------------------
bool apXOrgCARD32Paramter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apXOrgCARD32Paramter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        7/1/2007
// ---------------------------------------------------------------------------
bool apXOrgCARD32Paramter::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apXOrgCARD32Paramter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        7/1/2007
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apXOrgCARD32Paramter::readValueFromArgumentsList(va_list& pArgumentList)
{
    long argumentValue = va_arg(pArgumentList , long);
    _value = (CARD32)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apXOrgCARD32Paramter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        7/1/2007
// ---------------------------------------------------------------------------
void apXOrgCARD32Paramter::readValueFromPointer(void* pValue)
{
    _value = *((CARD32*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apXOrgCARD32Paramter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        7/1/2007
// ---------------------------------------------------------------------------
gtSizeType apXOrgCARD32Paramter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(CARD32);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apXOrgCARD32Paramter::valueAsHexString
// Description: Returns the parameter value as a string in hexadecimal display
// Arguments:   gtString& valueString
// Author:  AMD Developer Tools Team
// Date:        14/4/2011
// ---------------------------------------------------------------------------
void apXOrgCARD32Paramter::valueAsHexString(gtString& valueString) const
{
    (void)(valueString); // unused
    GT_ASSERT_EX(false, L"Implement me please");
}

// ---------------------------------------------------------------------------
// Name:        apXOrgCARD32Paramter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        7/1/2007
// ---------------------------------------------------------------------------
void apXOrgCARD32Paramter::valueAsString(gtString& valueString) const
{
    valueString = L"";
    valueString.appendFormattedString(L"%u", _value);
}

// ---------------------------------------------------------------------------
// Name:        apXOrgCARD32Paramter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apXOrgCARD32Paramter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apXOrgCARD32Paramter* pParam  = (apXOrgCARD32Paramter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// ----------------------------- apXIDParamter ------------------------------



// ---------------------------------------------------------------------------
// Name:        apXIDParamter::type()
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        5/5/2007
// ---------------------------------------------------------------------------
osTransferableObjectType apXIDParamter::type() const
{
    return OS_TOBJ_ID_XID_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apXIDParamter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        5/5/2007
// ---------------------------------------------------------------------------
bool apXIDParamter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apXIDParamter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        5/5/2007
// ---------------------------------------------------------------------------
bool apXIDParamter::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apXIDParamter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        5/5/2007
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apXIDParamter::readValueFromArgumentsList(va_list& pArgumentList)
{
    long argumentValue = va_arg(pArgumentList , long);
    _value = (XID)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apXIDParamter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        5/5/2007
// ---------------------------------------------------------------------------
void apXIDParamter::readValueFromPointer(void* pValue)
{
    _value = *((XID*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apXIDParamter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        5/5/2007
// ---------------------------------------------------------------------------
gtSizeType apXIDParamter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(XID);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apXIDParamter::valueAsHexString
// Description: Returns the parameter value as a string in hexadecimal display
// Arguments:   gtString& valueString
// Author:  AMD Developer Tools Team
// Date:        14/4/2011
// ---------------------------------------------------------------------------
void apXIDParamter::valueAsHexString(gtString& valueString) const
{
    (void)(valueString); // unused
    GT_ASSERT_EX(false, L"Implement me please");
}

// ---------------------------------------------------------------------------
// Name:        apXIDParamter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        5/5/2007
// ---------------------------------------------------------------------------
void apXIDParamter::valueAsString(gtString& valueString) const
{
    valueString = L"";
    valueString.appendFormattedString(L"%lu", _value);
}

// ---------------------------------------------------------------------------
// Name:        apXIDParamter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apXIDParamter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apXIDParamter* pParam  = (apXIDParamter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// -----------------------------   apLongBitfieldParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apLongBitfieldParameter::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        6/5/2008
// ---------------------------------------------------------------------------
osTransferableObjectType apLongBitfieldParameter::type() const
{
    return OS_TOBJ_ID_LONG_BITFIELD_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apLongBitfieldParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        6/5/2008
// ---------------------------------------------------------------------------
bool apLongBitfieldParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _value;
    return true;
}

// ---------------------------------------------------------------------------
// Name:        apLongBitfieldParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        6/5/2008
// ---------------------------------------------------------------------------
bool apLongBitfieldParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> _value;
    return true;
}

// ---------------------------------------------------------------------------
// Name:        apLongBitfieldParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        6/5/2008
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apLongBitfieldParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    unsigned long argumentValue = va_arg(pArgumentList , unsigned long);
    _value = argumentValue;
}

// ---------------------------------------------------------------------------
// Name:        apGLbitfieldParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        6/5/2008
// ---------------------------------------------------------------------------
void apLongBitfieldParameter::readValueFromPointer(void* pValue)
{
    _value = *((unsigned long*)(pValue));
}

// ---------------------------------------------------------------------------
// Name:        apGLbitfieldParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        6/5/2008
// ---------------------------------------------------------------------------
gtSizeType apLongBitfieldParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(unsigned long);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apLongBitfieldParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        6/5/2008
// ---------------------------------------------------------------------------
void apLongBitfieldParameter::valueAsString(gtString& valueString) const
{
    // We will display the hexadecimal value of the parameter.
    // Example: 0xFFFFDBEE
    valueString = L"";
    valueString.appendFormattedString(L"0x%lX", _value);
}

// ---------------------------------------------------------------------------
// Name:        apLongBitfieldParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apLongBitfieldParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apLongBitfieldParameter* pParam  = (apLongBitfieldParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}
#elif (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)

// ----------------------------- apEAGLRenderingAPIParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apEAGLRenderingAPIParameter::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        13/7/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apEAGLRenderingAPIParameter::type() const
{
    return OS_TOBJ_ID_EAGL_RENDERING_API_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apEAGLRenderingAPIParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        13/7/2009
// ---------------------------------------------------------------------------
bool apEAGLRenderingAPIParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _value;
    return true;
}

// ---------------------------------------------------------------------------
// Name:        apEAGLRenderingAPIParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        13/7/2009
// ---------------------------------------------------------------------------
bool apEAGLRenderingAPIParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> _value;
    return true;
}

// ---------------------------------------------------------------------------
// Name:        apEAGLRenderingAPIParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        13/7/2009
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apEAGLRenderingAPIParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    EAGLRenderingAPI argumentValue = va_arg(pArgumentList , EAGLRenderingAPI);
    _value = argumentValue;
}

// ---------------------------------------------------------------------------
// Name:        apGLbitfieldParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        13/7/2009
// ---------------------------------------------------------------------------
void apEAGLRenderingAPIParameter::readValueFromPointer(void* pValue)
{
    _value = *((EAGLRenderingAPI*)(pValue));
}

// ---------------------------------------------------------------------------
// Name:        apGLbitfieldParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        13/7/2009
// ---------------------------------------------------------------------------
gtSizeType apEAGLRenderingAPIParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(EAGLRenderingAPI);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apEAGLRenderingAPIParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        13/7/2009
// ---------------------------------------------------------------------------
void apEAGLRenderingAPIParameter::valueAsString(gtString& valueString) const
{
    switch (_value)
    {
        case kEAGLRenderingAPIOpenGLES1:
            valueString = L"kEAGLRenderingAPIOpenGLES1";
            break;

        case kEAGLRenderingAPIOpenGLES2:
            valueString = L"kkEAGLRenderingAPIOpenGLES2";
            break;

        default:
            GT_ASSERT(false);
            valueString = L"Unknown";
            valueString.appendFormattedString(L" (0x%X)", _value);
            break;
    }
}

// ---------------------------------------------------------------------------
// Name:        apEAGLRenderingAPIParameter::compareToOther
// Description: Compares this with other
// Author:  AMD Developer Tools Team
// Date:        13/7/2009
// ---------------------------------------------------------------------------
bool apEAGLRenderingAPIParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apEAGLRenderingAPIParameter* pParam  = (apEAGLRenderingAPIParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

#endif // AMDT_BUILD_TARGET







// ---------------------------------------------------------------------------
// Name:        apCreateParameterObject
// Description: Inputs an parameter type and creates the appropriate
//              apParameter class instance.
//              Notice that the caller is responsible for deleting the created
//              object.
// Arguments:   parameterType - The input parameter type.
// Return Val:  apParameter* - Will get a pointer to the created apParameter
//                             object (or NULL in case of failure).
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
apParameter* apCreateParameterObject(osTransferableObjectType parameterType)
{
    apParameter* retVal = NULL;

    // Create a transferable object of the input type:
    osTransferableObjectCreatorsManager& creatorsMgr = osTransferableObjectCreatorsManager::instance();
    gtAutoPtr<osTransferableObject> aptrTransferableObj;
    bool rc = creatorsMgr.createObject(parameterType, aptrTransferableObj);

    if (rc)
    {
        // Verify that it is an apParameter:
        if (aptrTransferableObj->isParameterObject())
        {
            // Down cast it to apParameter (and get its memory ownership):
            retVal = (apParameter*)(aptrTransferableObj.releasePointedObjectOwnership());
        }
    }

    return retVal;
}

