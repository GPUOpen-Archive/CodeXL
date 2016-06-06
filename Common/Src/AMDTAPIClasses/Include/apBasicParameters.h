//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apBasicParameters.h
///
//==================================================================================

//------------------------------ apBasicParameters.h ------------------------------

#ifndef __APBASICPARAMETERS_H
#define __APBASICPARAMETERS_H

// Forward decelerations:
class apParameter;

// Standard C:
#include <stdarg.h>

// Infra:
#include <AMDTOSWrappers/Include/osTransferableObject.h>
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTAPIClasses/Include/apAPIVersion.h>

// Default values:
#define AP_DEFAULT_FLOATING_POINT_PRECISION 8

// Utility functions:
AP_API apParameter* apCreateParameterObject(osTransferableObjectType parameterType);

// Float and double parameters display precision:
AP_API void apSetFloatParamsDisplayPrecision(unsigned int maxSignificatDigitsAmount);
AP_API unsigned int apGetFloatParamsDisplayPrecision();
AP_API const wchar_t* apGetFloatParamsFormatString();


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apParameter : public osTransferableObject
//
// General Description:
//   Base class that represents a parameter:
//   - An argument that is passed to a function.
//   - OpenGL state variable.
//   - etc.
//   There is a sub class per parameter type.
//
// Author:  AMD Developer Tools Team
// Creation Date:        8/4/2004
// ----------------------------------------------------------------------------------
class AP_API apParameter : public osTransferableObject
{

public:

    // Overrides osTransferableObject:
    virtual bool isParameterObject() const;

    // Must be implemented by sub classes:
    virtual void readValueFromArgumentsList(va_list& pArgumentList) = 0;
    virtual void readValueFromPointer(void* pValue) = 0;
    virtual gtSizeType sizeofData() = 0;
    virtual void valueAsString(gtString& valueString) const = 0;
    virtual bool compareToOther(const apParameter& other)const = 0;

    // Can be implemented by sub classes:
    virtual bool isPseudoParameter() const;
    virtual bool isOpenGLParameter() const;

    // Should be implemented for each integer number parameter
    virtual void valueAsHexString(gtString& valueString) const;


};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apPointerParameter : public apParameter
// General Description:
//   Represents a pointer parameter (Type*).
//   Holds the pointer value and the pointed object type.
// Author:  AMD Developer Tools Team
// Creation Date:        28/4/2004
// ----------------------------------------------------------------------------------
class AP_API apPointerParameter : public apParameter
{
public:
    // Self functions:
    apPointerParameter(void* ptrValue = NULL, osTransferableObjectType pointedObjectType = OS_TOBJ_ID_INT_PARAMETER);
    osTransferableObjectType pointedObjectType() const { return _pointedObjectType; };
    osProcedureAddress64 pointerValue() const { return _ptrValue; };
    void setValue(osProcedureAddress64 ptrValue, osTransferableObjectType pointedObjectType) {_ptrValue = ptrValue; _pointedObjectType = pointedObjectType;};

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
    // Is the debugged app 64-bit?
    bool _is64BitPointer;

    // The pointer value:
    osProcedureAddress64 _ptrValue;

    // The parameter type:
    osTransferableObjectType _pointedObjectType;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apPointerToPointerParameter : public apParameter
// General Description:
//   Represents a pointer to a pointer parameter (Type**).
//   Holds the pointer pointer value and the pointed object type.
// Author:  AMD Developer Tools Team
// Creation Date:        28/4/2004
// ----------------------------------------------------------------------------------
class AP_API apPointerToPointerParameter : public apParameter
{
public:
    // Self functions:
    apPointerToPointerParameter(void** ptrPtrValue = NULL);
    osTransferableObjectType pointedObjectType() const { return _pointedObjectType; };
    osProcedureAddress64 pointerToPointerValue() const { return _ptrPtrValue; };

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
    // Is the debugged app 64-bit?
    bool _is64BitPointer;

    // The pointer to pointer value:
    osProcedureAddress64 _ptrPtrValue;

    // The parameter type:
    osTransferableObjectType _pointedObjectType;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apVectorParameter: public apParameter
// General Description:
//   A vector parameter - a vector holding 1 or more parameters of the
//   same type.
// Author:  AMD Developer Tools Team
// Creation Date:        13/7/2004
// ----------------------------------------------------------------------------------
class AP_API apVectorParameter: public apParameter
{
public:
    apVectorParameter();
    apVectorParameter(unsigned int size);
    virtual ~apVectorParameter();

    bool initialize(unsigned int size);
    unsigned int vectorSize() const { return _size; };
    void setItem(unsigned int i, gtAutoPtr<apParameter>& aptrParameter);
    const apParameter* operator[](unsigned int itemIndex) const;

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
    bool createElementsVector();
    void deleteElementsVector();

private:
    // Amount of vector elements:
    unsigned int _size;

    // The vector itself:
    gtAutoPtr<apParameter>* _pParametersVector;

    // The pointer value (this pointer value is display ONLY when the user
    // by mistake, used a function with an expected vector arguments, with
    // size 0 and pointer which is not null.
    osProcedureAddress64 _ptrValue;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apMatrixParameter: public apParameter
// General Description:
//   A matrix parameter - an N x M  matrix holding parameters of the same type.
//
// Author:  AMD Developer Tools Team
// Creation Date:        13/7/2004
// ----------------------------------------------------------------------------------
class AP_API apMatrixParameter: public apParameter
{
public:
    apMatrixParameter();
    apMatrixParameter(unsigned int sizeN, unsigned int sizeM);
    virtual ~apMatrixParameter();

    void matrixSize(unsigned int& sizeN, unsigned int& sizeM) const { sizeN = _sizeN; sizeM = _sizeM; };
    void setItem(unsigned int indexN, unsigned int indexM, gtAutoPtr<apParameter>& aptrParameter);
    const apParameter* element(unsigned int indexN, unsigned int indexM) const;

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
    bool initialize(unsigned int sizeN, unsigned int sizeM);
    bool createElementsMatrix();
    void deleteElementsMatrix();

private:
    // The matrix size (N x M):
    unsigned int _sizeN;
    unsigned int _sizeM;

    // The matrix itself:
    gtAutoPtr<apParameter>* _pParametersMatrix;
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apZeroTerminatedListParameter : public apParameter
// General Description: Represents a list of parameters, styled as an array of
//                      {P0, V0, P1, V1, ..., Pn, Vn, 0}, where Px != 0 and V ?= 0.
//                      Class must be inherited to allow:
//                      1. Different sized payload
//                      2. Smarter parsing of the parameters (i.e. show V by its P)
// Author:  AMD Developer Tools Team
// Creation Date:       28/9/2014
// ----------------------------------------------------------------------------------
class AP_API apZeroTerminatedListParameter : public apParameter
{
public:
    apZeroTerminatedListParameter();
    virtual ~apZeroTerminatedListParameter();

    // Overrides osTransferableObject:
    // virtual osTransferableObjectType type() const; // Force the child classes to define unique types
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual void valueAsHexString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

protected:
    virtual unsigned int sizeOfArrayElement() const = 0;
    virtual unsigned int sizeOfArray(const void* pArray) const;
    virtual bool fillParamNameAndValue(const void* i_pArrayItem1, const void* i_pArrayItem2, gtAutoPtr<apParameter>& o_aptrArrayItem1, gtAutoPtr<apParameter>& o_aptrArrayItem2) const = 0;

private:
    bool createElementsVector(unsigned int size);
    void deleteElementsVector();

private:
    // Amount of vector elements. Note that a "1" value denotes an empty array {0} and a "0" value denotes
    // a NULL pointer - and both have a NULL parameters vector.
    unsigned int m_size;

    // The vector itself:
    gtAutoPtr<apParameter>* m_pParametersVector;

    // The pointer value (this pointer value is display ONLY when the user
    // by mistake, used a function with an expected vector arguments, with
    // size 0 and pointer which is not null.
    osProcedureAddress64 m_ptrValue;
};
// ----------------------------------------------------------------------------------
// Class Name:          AP_API apBitfieldParameter : public apParameter
// General Description: Represents a bitfield parameter. This class must be inherited
//                      To customize the specific bitfield values
// Author:  AMD Developer Tools Team
// Creation Date:       28/9/2014
// ----------------------------------------------------------------------------------
class AP_API apBitfieldParameter : public apParameter
{
public:
    apBitfieldParameter();
    virtual ~apBitfieldParameter();

    // Overrides osTransferableObject:
    // virtual osTransferableObjectType type() const; // Force the child classes to define unique types
    // virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const; // Force the child classes to define unique I/O functions
    // virtual bool readSelfFromChannel(osChannel& ipcChannel); // Force the child classes to define unique I/O functions

    // Overrides apParameter:
    // virtual void readValueFromArgumentsList(va_list& pArgumentList); // Force the child classes to define unique I/O functions
    // virtual void readValueFromPointer(void* pValue); // Force the child classes to define unique I/O functions
    // virtual gtSizeType sizeofData(); // Force the child classes to define unique I/O functions
    virtual void valueAsString(gtString& valueString) const;
    // virtual bool compareToOther(const apParameter& other)const;

protected:
    virtual bool getSpecialValue(gtString& o_string) const;
    virtual gtUInt64 getBitfieldAsUInt64() const = 0;
    virtual bool getBitName(int bitIndex, gtString& o_string) const = 0;
};

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apNotAvailableParameter : public apParameter
// General Description: Represents a "N/A" parameter.
// Author:  AMD Developer Tools Team
// Creation Date:        28/4/2004
// ----------------------------------------------------------------------------------
class AP_API apNotAvailableParameter : public apParameter
{
public:
    // Self functions:
    apNotAvailableParameter() {};

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
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apNotSupportedParameter : public apParameter
// General Description: Represents a "Not supported" parameter.
// Author:  AMD Developer Tools Team
// Creation Date:        22/2/2006
// ----------------------------------------------------------------------------------
class AP_API apNotSupportedParameter : public apParameter
{
public:
    enum apUnsupportReason
    {
        AP_NOT_SUPPORTED_BY_HARDWARE
    };

    // Self functions:
    apNotSupportedParameter(apUnsupportReason unsupportReason = AP_NOT_SUPPORTED_BY_HARDWARE);

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
    // The reason for the parameter to be not supported:
    apUnsupportReason _unsupportReason;
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apRemovedParameter : public apParameter
// General Description: Used to represent a parameter that was removed from OpenGL
//                      (3.1 or higher) by the deprecation model.
// Author:  AMD Developer Tools Team
// Creation Date:       4/1/2010
// ----------------------------------------------------------------------------------
class AP_API apRemovedParameter : public apParameter
{
public:
    apRemovedParameter(apAPIVersion removedVersion = AP_GL_VERSION_NONE);

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

    apAPIVersion removedVersion();

private:
    apAPIVersion _removedVersion;
};


// -------------------  Pseudo parameters ---------------------


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apPseudoParameter : public apParameter
// General Description:
//   Represents a pseudo parameter - a parameter that is not a part of the original
//   function parameters.
//   Pseudo parameters enable attaching additional information to function calls.
//
// Author:  AMD Developer Tools Team
// Creation Date:        6/1/2005
// ----------------------------------------------------------------------------------
class AP_API apPseudoParameter : public apParameter
{
    // Overrides apParameter:
    virtual bool isPseudoParameter() const;
};



// -------------------  C / C++ parameter types  ---------------------



// ----------------------------------------------------------------------------------
// Class Name:           AP_API apFloatParameter : public apParameter
// General Description: Represents a float parameter.
// Author:  AMD Developer Tools Team
// Creation Date:        28/4/2004
// ----------------------------------------------------------------------------------
class AP_API apFloatParameter : public apParameter
{
public:
    // Self functions:
    apFloatParameter(float value = 0.0f) : _value(value) {};
    float value() const { return _value; };

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
    float _value;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apIntParameter : public apParameter
// General Description: Represents an int parameter.
// Author:  AMD Developer Tools Team
// Creation Date:        28/4/2004
// ----------------------------------------------------------------------------------
class AP_API apIntParameter : public apParameter
{
public:
    // Self functions:
    apIntParameter(int value = 0) : _value(value) {};
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
    virtual void valueAsHexString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

private:
    // The parameter value:
    int _value;
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apUnsignedIntParameter : public apParameter
// General Description: Represents an unsigned int parameter.
// Author:  AMD Developer Tools Team
// Creation Date:       22/3/2009
// ----------------------------------------------------------------------------------
class AP_API apUnsignedIntParameter : public apParameter
{
public:
    // Self functions:
    apUnsignedIntParameter(unsigned int value = 0) : _value(value) {};
    unsigned int value() const { return _value; };

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

protected:
    // The parameter value:
    unsigned int _value;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apSizeTParameter : public apParameter
// General Description: Represents an size_t parameter.
// Author:  AMD Developer Tools Team
// Creation Date:        8/11/2009
// ----------------------------------------------------------------------------------
class AP_API apSizeTParameter : public apParameter
{
public:
    // Self functions:
    apSizeTParameter(size_t value = 0) : _value(value) {};
    size_t value() const { return _value; };

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

protected:
    // The parameter value:
    size_t _value;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apBytesSizeParameter : public apSizeTParameter
// General Description: Represents a size in bytes parameter.
// Author:  AMD Developer Tools Team
// Creation Date:        11/3/2010
// ----------------------------------------------------------------------------------
class AP_API apBytesSizeParameter : public apSizeTParameter
{
public:
    // Self functions:
    apBytesSizeParameter(size_t value = 0) : apSizeTParameter(value) {};

    // Overrides apSizeTParameter:
    virtual osTransferableObjectType type() const;
    virtual void valueAsString(gtString& valueString) const;
    virtual void valueAsHexString(gtString& valueString) const;

};

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apStringParameter : public apParameter
// General Description: Represents a string parameter.
// Author:  AMD Developer Tools Team
// Creation Date:        28/4/2004
// ----------------------------------------------------------------------------------
class AP_API apStringParameter : public apParameter
{
public:
    // Self functions:
    apStringParameter() {};
    apStringParameter(gtASCIIString& value) : _value(value) {};
    const gtASCIIString& value() const { return _value; };

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
    void setValue(const gtASCIIString& val) {_value = val;};

private:
    // The parameter value:
    gtASCIIString _value;
};


#endif //__APBASICPARAMETERS_H

