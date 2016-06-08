//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apBasicParameters.cpp
///
//==================================================================================

//------------------------------ apBasicParameters.cpp ------------------------------


// ---------------------------- The use of stdarg ------------------------
//
//    a. We decided to use stdarg for logging the called function argument
//       values because of efficiency reasons.
//    b. In Microsoft implementation of stdarg, the C calling convention
//       force us to use:
//       - int for char, and short types
//       - double for float types
//       - pointer for array types
//       (See an appropriate comment at Microsoft stdarg.h file).
//
// ------------------------------------------------------------------------

// Standard C:
#include <stdarg.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apBasicParameters.h>
#include <AMDTAPIClasses/Include/apStringConstants.h>


// Holds float and double parameters display precision:
static unsigned int s_floatParametersDisplayPrecision = AP_DEFAULT_FLOATING_POINT_PRECISION;

// Holds printf style string that represent the current float and double display precisions:
static wchar_t s_floatParametersDisplayPrecisionStr[255];


// ---------------------------------------------------------------------------
// Name:        apSetFloatParamsDisplayPrecision
// Description: Sets float parameters display precision.
// Arguments: maxSignificatDigitsAmount - The maximum number of significant
//                                        digits printed. Trailing zeros are truncated.
// Author:  AMD Developer Tools Team
// Date:        23/9/2007
// ---------------------------------------------------------------------------
void apSetFloatParamsDisplayPrecision(unsigned int maxSignificatDigitsAmount)
{
    // Log the updated precision value:
    s_floatParametersDisplayPrecision = maxSignificatDigitsAmount;

    // Update the printf style format string:
    // Example format string that represents 6 max signigicant digits is: "%.6g")
    swprintf(s_floatParametersDisplayPrecisionStr, 255, L"%%.%dg", s_floatParametersDisplayPrecision);
}


// ---------------------------------------------------------------------------
// Name:        apGetFloatParamsDisplayPrecision
// Description: Returns the current float parameters display precision.
// Author:  AMD Developer Tools Team
// Date:        23/9/2007
// ---------------------------------------------------------------------------
unsigned int apGetFloatParamsDisplayPrecision()
{
    return s_floatParametersDisplayPrecision;
}


// ---------------------------------------------------------------------------
// Name:        apGetFloatParamsFormatString
// Description: Returns a pointer to a printf style format string that
//              represents the current float parameters display precision.
// Author:  AMD Developer Tools Team
// Date:        23/9/2007
// ---------------------------------------------------------------------------
const wchar_t* apGetFloatParamsFormatString()
{
    return s_floatParametersDisplayPrecisionStr;
}


// ----------------------------- apParameter --------------------------------

// ---------------------------------------------------------------------------
// Name:        apParameter::isParameterObject
// Description: Returns true iff this is a sub-class of apParameter.
// Author:  AMD Developer Tools Team
// Date:        5/5/2004
// Implementation notes:
//   By implementing it in apParameter, all sub-classes inherit this implementation
//   that answers "true".
// ---------------------------------------------------------------------------
bool apParameter::isParameterObject() const
{
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apParameter::isPseudoParameter
// Description: By default, apParameter sub-classes are not pseudo parameters.
//              (See apPseudoParameter).
// Return Val:  bool - false - I am not a pseudo parameter.
// Author:  AMD Developer Tools Team
// Date:        6/1/2005
// ---------------------------------------------------------------------------
bool apParameter::isPseudoParameter() const
{
    return false;
}


// ---------------------------------------------------------------------------
// Name:        apParameter::isOpenGLParameter
// Description: By default, apParameter sub-classes are not OpenGL parameters.
//              (See apOpenGLParameter).
// Return Val:  bool - false - I am not an OpenGL parameter.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
bool apParameter::isOpenGLParameter() const
{
    return false;
}


// ---------------------------------------------------------------------------
// Name:        apParameter::valueAsHexString
// Description: By default hexadecimal display is just the string display
// Arguments:   gtString& valueString
// Author:  AMD Developer Tools Team
// Date:        14/4/2011
// ---------------------------------------------------------------------------
void apParameter::valueAsHexString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueAsString(valueString);
}

// -----------------------------   apPointerParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apPointerParameter::apPointerParameter
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        24/12/2009
// ---------------------------------------------------------------------------
apPointerParameter::apPointerParameter(void* ptrValue, osTransferableObjectType pointedObjectType):
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
    _is64BitPointer(false),
#elif AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
    _is64BitPointer(true),
#else
#error Unknown address space size!
#endif
    _ptrValue((osProcedureAddress64)ptrValue), _pointedObjectType(pointedObjectType)
{

}

// ---------------------------------------------------------------------------
// Name:        apPointerParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apPointerParameter::type() const
{
    return OS_TOBJ_ID_POINTER_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apPointerParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apPointerParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _is64BitPointer;
    ipcChannel << (gtUInt64)_ptrValue;
    ipcChannel << (gtInt32)_pointedObjectType;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apPointerParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apPointerParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> _is64BitPointer;
    gtUInt64 ptrValueAsUInt64 = 0;
    ipcChannel >> ptrValueAsUInt64;
    _ptrValue = (osProcedureAddress64)ptrValueAsUInt64;
    gtInt32 pointedObjectTypeAsInt32 = 0;
    ipcChannel >> pointedObjectTypeAsInt32;
    _pointedObjectType = (osTransferableObjectType)pointedObjectTypeAsInt32;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apPointerParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        5/5/2004
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apPointerParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    _ptrValue = (osProcedureAddress64)(va_arg(pArgumentList , void*));

    // We cannot know the data type. We initialize it to be int:
    _pointedObjectType = OS_TOBJ_ID_INT_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apPointerParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
void apPointerParameter::readValueFromPointer(void* pValue)
{
    _ptrValue = (osProcedureAddress64)(*((void**)(pValue)));

    // We cannot know the data type. We initialize it to be int:
    _pointedObjectType = OS_TOBJ_ID_INT_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apPointerParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
gtSizeType apPointerParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(void*);
    return sizeOfValue;
}


// ---------------------------------------------------------------------------
// Name:        apPointerParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
void apPointerParameter::valueAsString(gtString& valueString) const
{
    valueString.makeEmpty();

    if (_is64BitPointer)
    {
        valueString.appendFormattedString(GT_64_BIT_POINTER_FORMAT_UPPERCASE, _ptrValue);
    }
    else
    {
        gtUInt32 ptrValueAsUInt32 = (gtUInt32)_ptrValue;
        valueString.appendFormattedString(GT_32_BIT_POINTER_FORMAT_UPPERCASE, ptrValueAsUInt32);
    }
}


// ---------------------------------------------------------------------------
// Name:        apPointerParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apPointerParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apPointerParameter* pPointerParameter = (apPointerParameter*)(&other);
        GT_IF_WITH_ASSERT(pPointerParameter != NULL)
        {
            retVal = (pPointerParameter->_ptrValue == _ptrValue);
        }
    }

    return retVal;
}

// -----------------------------   apPointerToPointerParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apPointerToPointerParameter::apPointerToPointerParameter
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        24/12/2009
// ---------------------------------------------------------------------------
apPointerToPointerParameter::apPointerToPointerParameter(void** ptrPtrValue):
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
    _is64BitPointer(false),
#elif AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
    _is64BitPointer(true),
#else
#error Unknown address space size!
#endif
    _ptrPtrValue((osProcedureAddress64)ptrPtrValue), _pointedObjectType(OS_TOBJ_ID_INT_PARAMETER)
{

};

// ---------------------------------------------------------------------------
// Name:        apPointerToPointerParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apPointerToPointerParameter::type() const
{
    return OS_TOBJ_ID_POINTER_TO_POINTER_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apPointerToPointerParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apPointerToPointerParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _is64BitPointer;
    ipcChannel << (gtUInt64)_ptrPtrValue;
    ipcChannel << (gtInt32)_pointedObjectType;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apPointerToPointerParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apPointerToPointerParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> _is64BitPointer;
    gtUInt64 ptrValueAsUInt64 = 0;
    ipcChannel >> ptrValueAsUInt64;
    _ptrPtrValue = (osProcedureAddress64)ptrValueAsUInt64;
    gtInt32 pointedObjectTypeAsInt32 = 0;
    ipcChannel >> pointedObjectTypeAsInt32;
    _pointedObjectType = (osTransferableObjectType)pointedObjectTypeAsInt32;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apPointerToPointerParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        5/5/2004
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apPointerToPointerParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    void* pValue = va_arg(pArgumentList , void*);
    _ptrPtrValue = (osProcedureAddress64)pValue;

    // We cannot know the data type. We initialize it to be int:
    _pointedObjectType = OS_TOBJ_ID_INT_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apPointerToPointerParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
void apPointerToPointerParameter::readValueFromPointer(void* pValue)
{
    _ptrPtrValue = (osProcedureAddress64)(*((void***)(pValue)));

    // We cannot know the data type. We initialize it to be int:
    _pointedObjectType = OS_TOBJ_ID_INT_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apPointerToPointerParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
gtSizeType apPointerToPointerParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(void*);
    return sizeOfValue;
}


// ---------------------------------------------------------------------------
// Name:        apPointerToPointerParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
void apPointerToPointerParameter::valueAsString(gtString& valueString) const
{
    valueString.makeEmpty();

    if (_is64BitPointer)
    {
        valueString.appendFormattedString(GT_64_BIT_POINTER_FORMAT_UPPERCASE, _ptrPtrValue);
    }
    else
    {
        gtUInt32 ptrPtrValueAsUInt32 = (gtUInt32)_ptrPtrValue;
        valueString.appendFormattedString(GT_32_BIT_POINTER_FORMAT_UPPERCASE, ptrPtrValueAsUInt32);
    }
}

// ---------------------------------------------------------------------------
// Name:        apPointerToPointerParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apPointerToPointerParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apPointerToPointerParameter* pPointerToPointerParameter  = (apPointerToPointerParameter*)(&other);
        GT_IF_WITH_ASSERT(pPointerToPointerParameter != NULL)
        {
            retVal = (pPointerToPointerParameter->_ptrPtrValue == _ptrPtrValue);
        }
    }

    return retVal;
}

// -----------------------------  apVectorParameter  ------------------------------


// ---------------------------------------------------------------------------
// Name:        apVectorParameter::apVectorParameter
// Description: Default constructor - initialize an empty vector.
// Author:  AMD Developer Tools Team
// Date:        15/7/2004
// ---------------------------------------------------------------------------
apVectorParameter::apVectorParameter()
    : _size(0), _pParametersVector(NULL), _ptrValue(OS_NULL_PROCEDURE_ADDRESS_64)
{
}


// ---------------------------------------------------------------------------
// Name:        apVectorParameter::apVectorParameter
// Description: Constructor
// Arguments:   size - The vector size.
// Author:  AMD Developer Tools Team
// Date:        13/7/2004
// ---------------------------------------------------------------------------
apVectorParameter::apVectorParameter(unsigned int size)
    : _size(size), _pParametersVector(NULL)
{
    // Create the C vector that will hold this vector elements:
    bool rc = createElementsVector();
    GT_ASSERT(rc);
}


// ---------------------------------------------------------------------------
// Name:        apVectorParameter::~apVectorParameter
// Description: Destructor.
// Author:  AMD Developer Tools Team
// Date:        13/7/2004
// ---------------------------------------------------------------------------
apVectorParameter::~apVectorParameter()
{
    deleteElementsVector();
}


// ---------------------------------------------------------------------------
// Name:        apVectorParameter::setItem
// Description: Sets the value of the i'th vector element.
// Author:  AMD Developer Tools Team
// Date:        13/7/2004
// ---------------------------------------------------------------------------
void apVectorParameter::setItem(unsigned int i, gtAutoPtr<apParameter>& aptrParameter)
{
    if (_pParametersVector)
    {
        // Index range check:
        GT_IF_WITH_ASSERT(i < _size)
        {
            _pParametersVector[i] = aptrParameter;
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        apVectorParameter::operator[]
// Description: Return the vector it's item.
// Author:  AMD Developer Tools Team
// Date:        13/7/2004
// ---------------------------------------------------------------------------
const apParameter* apVectorParameter::operator[](unsigned int itemIndex) const
{
    const apParameter* retVal = NULL;

    // Index range check:
    GT_IF_WITH_ASSERT(itemIndex < _size)
    {
        retVal = _pParametersVector[itemIndex].pointedObject();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apVectorParameter::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        13/7/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apVectorParameter::type() const
{
    return OS_TOBJ_ID_VECTOR_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apVectorParameter::writeSelfIntoChannel
// Description: Writes self into a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        13/7/2004
// ---------------------------------------------------------------------------
bool apVectorParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    // Write the vector size:
    ipcChannel << (gtUInt32)_size;

    if (_size > 0)
    {
        // Write the vector elements:
        for (unsigned int i = 0; i < _size; i++)
        {
            if (_pParametersVector[i].pointedObject() != NULL)
            {
                // Write the current element:
                ipcChannel << *(_pParametersVector[i]);
            }
            else
            {
                // Write a N/A parameter into the channel:
                apNotAvailableParameter naParam;
                ipcChannel << naParam;

                // Mark that an error occured:
                retVal = false;
            }
        }
    }
    else
    {
        ipcChannel << (gtUInt64)_ptrValue;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apVectorParameter::readSelfFromChannel
// Description: Reads self from a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        13/7/2004
// ---------------------------------------------------------------------------
bool apVectorParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    // If the elements vector exists - delete it:
    if (_pParametersVector)
    {
        deleteElementsVector();
    }

    // Read the vector size:
    gtUInt32 sizeAsUInt32 = 0;
    ipcChannel >> sizeAsUInt32;
    _size = (unsigned int)sizeAsUInt32;

    if (_size > 0)
    {
        // Create the elements vector:
        bool rc = createElementsVector();

        if (rc)
        {
            // Read the vector elements:
            for (unsigned int i = 0; i < _size; i++)
            {
                // Read the parameter (as transferable object) from the channel:
                gtAutoPtr<osTransferableObject> aptrReadTransferableObj;
                ipcChannel >> aptrReadTransferableObj;

                // Verify that we read a parameter object:
                if (aptrReadTransferableObj->isParameterObject())
                {
                    // Set the vector's i'th element to contain this parameter:
                    _pParametersVector[i] = (apParameter*)(aptrReadTransferableObj.releasePointedObjectOwnership());
                }
                else
                {
                    retVal = false;
                }
            }
        }
    }
    else
    {
        gtUInt64 ptrValueAsUInt64 = 0;
        ipcChannel >> ptrValueAsUInt64;
        _ptrValue = (osProcedureAddress64)ptrValueAsUInt64;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apVectorParameter::readValueFromArgumentsList
// Description: Reads my value from an argument list.
// Author:  AMD Developer Tools Team
// Date:        13/7/2004
// Implementation notes:
//   We assume that the argument list contains the following arguments:
//   a. Elements type (as osTransferableObjectType).
//   b. The amount of vector elements
//   c. A pointer to a "C" array containing the vector elements.
// ---------------------------------------------------------------------------
void apVectorParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    // Read the vector elements type:
    osTransferableObjectType elementsType = (osTransferableObjectType)(va_arg(pArgumentList, int));

    // Get the amount of vector elements:
    int amountOfVectorElements = va_arg(pArgumentList , int);

    // Get a pointer to the "C" vector that contains the values:
    void* pCVector = va_arg(pArgumentList , void*);

    if (amountOfVectorElements > 0)
    {
        if (pCVector != NULL)
        {
            // Resize the vector:
            initialize(amountOfVectorElements);

            // Fill the vector elements:
            void* pCurrentElement = pCVector;

            for (int i = 0; i < amountOfVectorElements; i++)
            {
                bool succeededToReadElement = false;

                // Create an apParameter that will wrap the current element:
                apParameter* pParameterObj = apCreateParameterObject(elementsType);

                if (pParameterObj)
                {
                    // Initialize the item:
                    pParameterObj->readValueFromPointer(pCurrentElement);

                    // Get the next item pointer:
                    gtSizeType elementCSize = pParameterObj->sizeofData();
                    pCurrentElement = (gtByte*)pCurrentElement + elementCSize;

                    // Insert the item into this vector:
                    gtAutoPtr<apParameter> aptrParameter = pParameterObj;
                    setItem(i, aptrParameter);
                    succeededToReadElement = true;
                }

                // Sanity check:
                if (!succeededToReadElement)
                {
                    GT_ASSERT(0);
                    break;
                }
            }
        }
        else
        {
            _size = 0;
        }
    }
    else
    {
        // If the user declared 0 as the vector size, save the original pointer value, to display it:
        _ptrValue = (osProcedureAddress64)pCVector;
        _size = 0;
    }
}


// ---------------------------------------------------------------------------
// Name:        apVectorParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
void apVectorParameter::readValueFromPointer(void* pValue)
{
    (void)(pValue); // unused
    // Should not be called !
    GT_ASSERT(0);
}


// ---------------------------------------------------------------------------
// Name:        apVectorParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
gtSizeType apVectorParameter::sizeofData()
{
    // Should not be called !
    GT_ASSERT(0);
    return 4;
}

// ---------------------------------------------------------------------------
// Name:        apVectorParameter::valueAsHexString
// Description: Return my value as string with hexadecimal display
// Arguments:   gtString& valueString
// Author:  AMD Developer Tools Team
// Date:        14/4/2011
// ---------------------------------------------------------------------------
void apVectorParameter::valueAsHexString(gtString& valueString) const
{
    valueString.makeEmpty();

    if (_size > 0)
    {
        valueString = '{';

        if (_pParametersVector)
        {
            // Iterate the vector elements:
            for (unsigned int i = 0; i < _size; i++)
            {
                // Get the current element value (as a string):
                gtString currentElementValueString;
                const apParameter* pCurrentElem = _pParametersVector[i].pointedObject();

                if (pCurrentElem)
                {
                    pCurrentElem->valueAsHexString(currentElementValueString);
                }
                else
                {
                    currentElementValueString = AP_STR_NotAvailable;
                }

                // Add it to the output string:
                valueString += currentElementValueString;

                if (i < (_size - 1))
                {
                    // Add separator:
                    valueString += L", ";
                }
            }
        }

        valueString += '}';
    }
    else
    {
        // Write the pointer value:
        gtUInt32 ptrValueAsUInt32 = (gtUInt32)_ptrValue;
        valueString.appendFormattedString(GT_32_BIT_POINTER_FORMAT_UPPERCASE, ptrValueAsUInt32);
    }
}

// ---------------------------------------------------------------------------
// Name:        apVectorParameter::valueAsString
// Description: Returns my value as a string
// Author:  AMD Developer Tools Team
// Date:        13/7/2004
// ---------------------------------------------------------------------------
void apVectorParameter::valueAsString(gtString& valueString) const
{
    if (_size > 0)
    {
        valueString = '{';

        if (_pParametersVector)
        {
            // Iterate the vector elements:
            for (unsigned int i = 0; i < _size; i++)
            {
                // Get the current element value (as a string):
                gtString currentElementValueString;
                const apParameter* pCurrentElem = _pParametersVector[i].pointedObject();

                if (pCurrentElem)
                {
                    pCurrentElem->valueAsString(currentElementValueString);
                }
                else
                {
                    currentElementValueString = AP_STR_NotAvailable;
                }

                // Add it to the output string:
                valueString += currentElementValueString;

                if (i < (_size - 1))
                {
                    // Add separator:
                    valueString += L", ";
                }
            }
        }

        valueString += '}';
    }
    else
    {
        // Write the pointer value:
        gtUInt32 ptrValueAsUInt32 = (gtUInt32)_ptrValue;
        valueString.appendFormattedString(GT_32_BIT_POINTER_FORMAT_UPPERCASE, ptrValueAsUInt32);
    }
}


// ---------------------------------------------------------------------------
// Name:        apVectorParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apVectorParameter::compareToOther(const apParameter& other)const
{
    bool retVal = true;

    // Compare types:
    if (this->type() != other.type())
    {
        retVal = false;
    }
    else
    {
        // Verify that both vector sizes are identical:
        const apVectorParameter& otherAsVec = (const apVectorParameter&)other;
        unsigned int otherSize = otherAsVec.vectorSize();

        if (otherSize != _size)
        {
            GT_ASSERT(false);
            retVal = false;
        }
        else
        {
            // Iterate the vector elements:
            for (unsigned int i = 0; i < _size; i++)
            {
                // Get the current vectors items:
                const apParameter* pThisVecItem = _pParametersVector[i].pointedObject();
                const apParameter* pOtherVecItem = otherAsVec[i];

                if ((pThisVecItem == NULL) || (pOtherVecItem == NULL))
                {
                    GT_ASSERT(false);
                    retVal = false;
                }
                else
                {
                    retVal = pThisVecItem->compareToOther(*pOtherVecItem);

                    if (retVal == false)
                    {
                        break;
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apVectorParameter::initialize
// Description: Initialize the vector.
// Arguments:   size - The vector new size.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        26/7/2004
// ---------------------------------------------------------------------------
bool apVectorParameter::initialize(unsigned int size)
{
    bool retVal = false;

    // Delete the elements vector (if exists):
    deleteElementsVector();

    // Set the vector size:
    _size = size;

    // Create the elements vector:
    retVal = createElementsVector();

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apVectorParameter::createElementsVector
// Description: Creates the C vector that holds this vector elements.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        13/7/2004
// ---------------------------------------------------------------------------
bool apVectorParameter::createElementsVector()
{
    bool retVal = false;

    // Allocate the new vector:
    _pParametersVector = new gtAutoPtr<apParameter>[_size];

    if (NULL != _pParametersVector)
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apVectorParameter::deleteElementsVector
// Description: Deletes the elements vector.
// Author:  AMD Developer Tools Team
// Date:        15/7/2004
// ---------------------------------------------------------------------------
void apVectorParameter::deleteElementsVector()
{
    if (_pParametersVector)
    {
        // Delete the vector itself (Being a vector of gtAutoPtrs, this will cause the contents to be deleted as well):
        delete[] _pParametersVector;
    }
}




// -----------------------------   apMatrixParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apMatrixParameter::apMatrixParameter
// Description: Default constructor - initialize an empty matrix.
// Author:  AMD Developer Tools Team
// Date:        15/7/2004
// ---------------------------------------------------------------------------
apMatrixParameter::apMatrixParameter()
    : _sizeN(0), _sizeM(0), _pParametersMatrix(NULL)
{
}


// ---------------------------------------------------------------------------
// Name:        apMatrixParameter::apMatrixParameter
// Description: Constructor
// Arguments:   sizeN, sizeM - The matrix size (N x M).
// Author:  AMD Developer Tools Team
// Date:        18/7/2004
// ---------------------------------------------------------------------------
apMatrixParameter::apMatrixParameter(unsigned int sizeN, unsigned int sizeM)
    : _sizeN(sizeN), _sizeM(sizeM), _pParametersMatrix(NULL)
{
    bool rc = false;

    // Sanity check:
    if ((sizeN > 0) && (sizeN < 5) && (sizeM > 0) && (sizeM < 5))
    {
        // Create the C vector that will hold this matrix elements:
        rc = createElementsMatrix();
    }

    GT_ASSERT(rc);
}


// ---------------------------------------------------------------------------
// Name:        apMatrixParameter::~apMatrixParameter
// Description: Destructor.
// Author:  AMD Developer Tools Team
// Date:        18/7/2004
// ---------------------------------------------------------------------------
apMatrixParameter::~apMatrixParameter()
{
    deleteElementsMatrix();
}


// ---------------------------------------------------------------------------
// Name:        apMatrixParameter::setItem
// Description: Sets the value of the matrix element.
// Arguments:   indexN, indexM - Element position in the matrix.
//              aptrParameter - The element to be set.
// Author:  AMD Developer Tools Team
// Date:        18/7/2004
// ---------------------------------------------------------------------------
void apMatrixParameter::setItem(unsigned int indexN, unsigned int indexM, gtAutoPtr<apParameter>& aptrParameter)
{
    GT_IF_WITH_ASSERT(_pParametersMatrix != NULL)
    {
        // Indices range check:
        GT_IF_WITH_ASSERT((indexN < _sizeN) && (indexM < _sizeM))
        {
            unsigned int itemVecIndex = (_sizeM * indexN) + indexM;
            GT_IF_WITH_ASSERT(_sizeN * _sizeM > itemVecIndex)
            {
                _pParametersMatrix[itemVecIndex] = aptrParameter;
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        apMatrixParameter::element
// Description: Access method to the matrix elements.
// Arguments:   indexN, indexM - The requested element index.
// Return Val:  const apParameter* - Will get the requested matrix element.
// Author:  AMD Developer Tools Team
// Date:        18/7/2004
// ---------------------------------------------------------------------------
const apParameter* apMatrixParameter::element(unsigned int indexN, unsigned int indexM) const
{
    const apParameter* retVal = NULL;

    GT_IF_WITH_ASSERT(_pParametersMatrix != NULL)
    {
        // Indices range check:
        GT_IF_WITH_ASSERT((indexN < _sizeN) && (indexM < _sizeM))
        {
            unsigned int itemVecIndex = (_sizeM * indexN) + indexM;
            GT_IF_WITH_ASSERT(_sizeN * _sizeM > itemVecIndex)
            {
                retVal = _pParametersMatrix[itemVecIndex].pointedObject();
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apMatrixParameter::type
// Description: Return my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        18/7/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apMatrixParameter::type() const
{
    return OS_TOBJ_ID_MATRIX_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apMatrixParameter::writeSelfIntoChannel
// Description: Write this parameter into a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        18/7/2004
// ---------------------------------------------------------------------------
bool apMatrixParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    // Write the matrix size:
    ipcChannel << (gtUInt32)_sizeN;
    ipcChannel << (gtUInt32)_sizeM;

    // Write the matrix elements:
    for (unsigned int i = 0; i < _sizeN; i++)
    {
        for (unsigned int j = 0; j < _sizeM; j++)
        {
            unsigned int itemVecIndex = (_sizeM * i) + j;

            if ((_sizeN * _sizeM > itemVecIndex) && (_pParametersMatrix[itemVecIndex].pointedObject() != NULL))
            {
                // Write the current element:
                ipcChannel << *(_pParametersMatrix[itemVecIndex]);
            }
            else
            {
                retVal = false;
            }
        }
    }

    GT_RETURN_WITH_ASSERT(retVal);
}


// ---------------------------------------------------------------------------
// Name:        apMatrixParameter::readSelfFromChannel
// Description: Reads this parameter from a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        18/7/2004
// ---------------------------------------------------------------------------
bool apMatrixParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    // If the elements matrix exists - delete it:
    if (NULL != _pParametersMatrix)
    {
        deleteElementsMatrix();
    }

    // Read the matrix size:
    gtUInt32 sizeNAsUInt32 = 0;
    ipcChannel >> sizeNAsUInt32;
    _sizeN = (unsigned int)sizeNAsUInt32;
    gtUInt32 sizeMAsUInt32 = 0;
    ipcChannel >> sizeMAsUInt32;
    _sizeM = (unsigned int)sizeMAsUInt32;

    // Create the elements matrix:
    bool rc = createElementsMatrix();

    if (rc)
    {
        // Read the matrix elements:
        for (unsigned int i = 0; i < _sizeN; i++)
        {
            for (unsigned int j = 0; j < _sizeM; j++)
            {
                // Read the parameter (as transferable object) from the channel:
                gtAutoPtr<osTransferableObject> aptrReadTransferableObj;
                ipcChannel >> aptrReadTransferableObj;

                // Verify that we read a parameter object:
                if (aptrReadTransferableObj->isParameterObject())
                {
                    // Set the matrix appropriate element to contain this parameter:
                    unsigned int itemVecIndex = (_sizeM * i) + j;

                    if (_sizeN * _sizeM > itemVecIndex)
                    {
                        _pParametersMatrix[itemVecIndex] = (apParameter*)(aptrReadTransferableObj.releasePointedObjectOwnership());
                    }
                }
                else
                {
                    retVal = false;
                }
            }
        }
    }

    GT_ASSERT(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apMatrixParameter::readValueFromArgumentsList
// Description: Reads my value from an argument list.
// Author:  AMD Developer Tools Team
// Date:        1/8/2004
// Implementation notes:
//   We assume that the argument list contains the following arguments:
//   a. Elements type (as osTransferableObjectType).
//   b. The matrix sizes (as two integers).
//   c. A pointer to a "C" array containing the matrix elements.
// ---------------------------------------------------------------------------
void apMatrixParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    // Read the matrix elements type:
    osTransferableObjectType elementsType = (osTransferableObjectType)(va_arg(pArgumentList, int));

    // Get the matrix size:
    int sizeN = va_arg(pArgumentList , int);
    int sizeM = va_arg(pArgumentList , int);

    // Get a pointer to the "C" vector that contains the values:
    void* pCVector = va_arg(pArgumentList , void*);

    if (pCVector != NULL)
    {
        // Resize the vector:
        initialize(sizeN, sizeM);

        // Fill the vector elements:
        bool goOn = true;
        void* pCurrentElement = pCVector;

        for (int i = 0; (i < sizeN) && goOn; i++)
        {
            for (int j = 0; (j < sizeM) && goOn; j++)
            {
                bool succeededToReadElement = false;

                // Create an apParameter that will wrap the current element:
                apParameter* pParameterObj = apCreateParameterObject(elementsType);

                if (NULL != pParameterObj)
                {
                    // Initialize the item:
                    pParameterObj->readValueFromPointer(pCurrentElement);

                    // Get the next item pointer:
                    gtSizeType elementCSize = pParameterObj->sizeofData();
                    pCurrentElement = (gtByte*)pCurrentElement + elementCSize;

                    // Insert the item into this vector:
                    gtAutoPtr<apParameter> aptrParameter = pParameterObj;
                    setItem(i, j, aptrParameter);
                    succeededToReadElement = true;
                }

                // Sanity check:
                if (!succeededToReadElement)
                {
                    GT_ASSERT(0);
                    goOn = false;
                }
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        apMatrixParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
void apMatrixParameter::readValueFromPointer(void* pValue)
{
    (void)(pValue); // unused
    // Should not be called !
    GT_ASSERT(0);
}


// ---------------------------------------------------------------------------
// Name:        apMatrixParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
gtSizeType apMatrixParameter::sizeofData()
{
    // Should not be called !
    GT_ASSERT(0);
    return 4;
}

// ---------------------------------------------------------------------------
// Name:        apMatrixParameter::valueAsHexString
// Description: Returns my value as a string with hexadecimal display
// Arguments:   gtString& valueString
// Return Val:  void
// Author:  AMD Developer Tools Team
// Date:        14/4/2011
// ---------------------------------------------------------------------------
void apMatrixParameter::valueAsHexString(gtString& valueString) const
{
    valueString.makeEmpty();

    if (NULL != _pParametersMatrix)
    {
        // Iterate the vector elements:
        for (unsigned int i = 0; i < _sizeN; i++)
        {
            valueString += '{';

            for (unsigned int j = 0; j < _sizeM; j++)
            {
                // Get the current element value (as a string):
                gtString currentElementValueString;
                const apParameter* pCurrentElem = element(i, j);

                if (pCurrentElem)
                {
                    pCurrentElem->valueAsHexString(currentElementValueString);
                }
                else
                {
                    currentElementValueString = AP_STR_NotAvailable;
                }

                // Add it to the output string:
                valueString += currentElementValueString;

                if (j < (_sizeM - 1))
                {
                    // Add separator:
                    valueString += L", ";
                }
            }

            valueString += '}';

            if (i < (_sizeN - 1))
            {
                valueString += L"\n";
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        apMatrixParameter::valueAsString
// Description: Returns my value as a string.
// Author:  AMD Developer Tools Team
// Date:        18/7/2004
// ---------------------------------------------------------------------------
void apMatrixParameter::valueAsString(gtString& valueString) const
{
    valueString.makeEmpty();

    if (NULL != _pParametersMatrix)
    {
        // Iterate the vector elements:
        for (unsigned int i = 0; i < _sizeN; i++)
        {
            valueString += '{';

            for (unsigned int j = 0; j < _sizeM; j++)
            {
                // Get the current element value (as a string):
                gtString currentElementValueString;
                const apParameter* pCurrentElem = element(i, j);

                if (pCurrentElem)
                {
                    pCurrentElem->valueAsString(currentElementValueString);
                }
                else
                {
                    currentElementValueString = AP_STR_NotAvailable;
                }

                // Add it to the output string:
                valueString += currentElementValueString;

                if (j < (_sizeM - 1))
                {
                    // Add separator:
                    valueString += L", ";
                }
            }

            valueString += '}';

            if (i < (_sizeN - 1))
            {
                valueString += L"\n";
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        apMatrixParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apMatrixParameter::compareToOther(const apParameter& other)const
{
    bool retVal = true;

    // Compare types:
    if (this->type() != other.type())
    {
        retVal = false;
    }
    else
    {
        // Verify that both matrix sizes are identical:
        const apMatrixParameter& otherAsMat = (const apMatrixParameter&)other;
        unsigned int otherSizeN = 0;
        unsigned int otherSizeM = 0;
        otherAsMat.matrixSize(otherSizeN, otherSizeM);

        if ((otherSizeN != _sizeN) || (otherSizeM != _sizeM))
        {
            GT_ASSERT(false);
            retVal = false;
        }
        else
        {
            // Iterate the vector elements:
            for (unsigned int i = 0; i < _sizeN; i++)
            {
                for (unsigned int j = 0; j < _sizeM; j++)
                {
                    // Get the current matrix items:
                    const apParameter* pThisMatItem = element(i, j);
                    const apParameter* pOtherMatItem = otherAsMat.element(i, j);

                    if ((pThisMatItem == NULL) || (pOtherMatItem == NULL))
                    {
                        GT_ASSERT(false);
                        retVal = false;
                    }
                    else
                    {
                        retVal = pThisMatItem->compareToOther(*pOtherMatItem);

                        if (!retVal)
                        {
                            // Values are not equal, no need to keep on iterating:
                            break;
                        }
                    }
                }

                if (!retVal)
                {
                    // Values are not equal, no need to keep on iterating:
                    break;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apMatrixParameter::initialize
// Description: Initialize the matrix.
// Arguments:   sizeN, sizeM - The matrix new size.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        26/7/2004
// ---------------------------------------------------------------------------
bool apMatrixParameter::initialize(unsigned int sizeN, unsigned int sizeM)
{
    bool retVal = false;

    // Delete the elements matrix (if exists):
    deleteElementsMatrix();

    // Set the vector size:
    _sizeN = sizeN;
    _sizeM = sizeM;

    // Create the elements matrix:
    retVal = createElementsMatrix();

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apMatrixParameter::createElementsMatrix
// Description: Creates the C matrix that will hold the elements.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        18/7/2004
// ---------------------------------------------------------------------------
bool apMatrixParameter::createElementsMatrix()
{
    bool retVal = true;

    // Allocate the new matrix (the matrix will be represented as a vector):
    int vecSize = _sizeN * _sizeM;

    if (0 < vecSize)
    {
        _pParametersMatrix = new gtAutoPtr<apParameter>[vecSize];

        if (NULL != _pParametersMatrix)
        {
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apMatrixParameter::deleteElementsMatrix
// Description: Deletes the C elements vector.
// Author:  AMD Developer Tools Team
// Date:        18/7/2004
// ---------------------------------------------------------------------------
void apMatrixParameter::deleteElementsMatrix()
{
    if (NULL != _pParametersMatrix)
    {
        // Delete the elements vector:
        delete[] _pParametersMatrix;
    }
}


// ----------------------------- apZeroTerminatedListParameter ------------------------------

// ---------------------------------------------------------------------------
// Name:        apZeroTerminatedListParameter::apZeroTerminatedListParameter
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
apZeroTerminatedListParameter::apZeroTerminatedListParameter()
    : m_size(0), m_pParametersVector(NULL), m_ptrValue(OS_NULL_PROCEDURE_ADDRESS_64)
{

}

// ---------------------------------------------------------------------------
// Name:        apZeroTerminatedListParameter::~apZeroTerminatedListParameter
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
apZeroTerminatedListParameter::~apZeroTerminatedListParameter()
{
    deleteElementsVector();
}

// ---------------------------------------------------------------------------
// Name:        apZeroTerminatedListParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
bool apZeroTerminatedListParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    // Write the size:
    ipcChannel << (gtUInt32)m_size;

    if (1 < m_size)
    {
        // Sanity:
        GT_IF_WITH_ASSERT(NULL != m_pParametersVector)
        {
            // Write the parameters:
            for (unsigned int i = 0; i < m_size; i++)
            {
                // Sanity:
                const apParameter* pCurrentParam = m_pParametersVector[i].pointedObject();
                GT_IF_WITH_ASSERT(NULL != pCurrentParam)
                {
                    ipcChannel << (const osTransferableObject&)(*pCurrentParam);
                }
                else
                {
                    // Write an error parameter to avoid problems on the reading side:
                    static const apNotAvailableParameter errParam;
                    ipcChannel << (const osTransferableObject&)errParam;
                }
            }
        }
        else
        {
            // Write error parameters to avoid problems on the reading side:
            for (unsigned int i = 0; i < m_size; i++)
            {
                static const apNotAvailableParameter errParam;
                ipcChannel << (const osTransferableObject&)errParam;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apZeroTerminatedListParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
bool apZeroTerminatedListParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    // Clear any previous value:
    deleteElementsVector();

    // Read the size:
    gtUInt32 sizeAsUInt32 = 0;
    ipcChannel >> sizeAsUInt32;
    retVal = createElementsVector((unsigned int)sizeAsUInt32);
    GT_ASSERT(m_size == (unsigned int)sizeAsUInt32);

    if (1 < sizeAsUInt32)
    {
        for (gtUInt32 i = 0; i < sizeAsUInt32; i++)
        {
            // Read the parameter:
            gtAutoPtr<osTransferableObject> aptrTransferableObj;
            ipcChannel >> aptrTransferableObj;

            // Did we get anything?
            GT_IF_WITH_ASSERT(NULL != aptrTransferableObj.pointedObject())
            {
                // Did we get a parameter?
                GT_IF_WITH_ASSERT(aptrTransferableObj->isParameterObject())
                {
                    // Downcast and save the value:
                    m_pParametersVector[i] = (apParameter*)(aptrTransferableObj.releasePointedObjectOwnership());
                }
            }

            // Validate:
            retVal = retVal && (NULL != m_pParametersVector[i].pointedObject());
        }
    }

    return true;
}

// ---------------------------------------------------------------------------
// Name:        apZeroTerminatedListParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
void apZeroTerminatedListParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    // Read the pointer:
    void* pCVector = va_arg(pArgumentList, void*);

    // Get the values from it:
    readValueFromPointer(&pCVector);
}

// ---------------------------------------------------------------------------
// Name:        apZeroTerminatedListParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
void apZeroTerminatedListParameter::readValueFromPointer(void* pValue)
{
    // Set the array and members according to size:
    const void* pArray = *(const void**)pValue;

    unsigned int arraySize = sizeOfArray(pArray);
    bool rcVc = createElementsVector(arraySize);
    GT_IF_WITH_ASSERT(rcVc)
    {
        // Ignore empty and NULL pointers:
        if (1 < m_size)
        {
            // Make sure we have an even number:
            GT_ASSERT(0 == (m_size % 2));
            gtSizeType basePointer = (gtSizeType)pArray;
            gtSizeType variableOffset = (gtSizeType)sizeOfArrayElement();
            unsigned int validParamsNumber = m_size / 2;

            for (unsigned int i = 0; i < validParamsNumber; i++)
            {
                unsigned int idx1 = i * 2;
                unsigned int idx2 = idx1 + 1;
                const void* ptr1 = (const void*)(basePointer + (variableOffset * idx1));
                const void* ptr2 = (const void*)(basePointer + (variableOffset * idx2));
                bool rcElem = fillParamNameAndValue(ptr1, ptr2, m_pParametersVector[idx1], m_pParametersVector[idx2]);
                GT_ASSERT(rcElem);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        apZeroTerminatedListParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
gtSizeType apZeroTerminatedListParameter::sizeofData()
{
    // Should not be called !
    GT_ASSERT(0);

    return sizeof(void*);
}

// ---------------------------------------------------------------------------
// Name:        apZeroTerminatedListParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
void apZeroTerminatedListParameter::valueAsString(gtString& valueString) const
{
    if (1 < m_size)
    {
        valueString = '{';
        gtString currentValueString;

        for (unsigned int i = 0; i < m_size; i++)
        {
            const apParameter* pCurrentParam = m_pParametersVector[i].pointedObject();
            GT_IF_WITH_ASSERT(NULL != pCurrentParam)
            {
                pCurrentParam->valueAsString(currentValueString);
                valueString.append(currentValueString);
            }
            valueString.append(',').append(' ');
        }

        valueString.append(L"0}");
    }
    else if (1 == m_size)
    {
        // Empty array:
        valueString = L"{0}";
    }
    else
    {
        // Show the pointer value:
        gtUInt32 ptrValueAsUInt32 = (gtUInt32)m_ptrValue;
        valueString.appendFormattedString(GT_32_BIT_POINTER_FORMAT_UPPERCASE, ptrValueAsUInt32);
    }
}

// ---------------------------------------------------------------------------
// Name:        apZeroTerminatedListParameter::valueAsHexString
// Description: Returns the parameter value as a hexadecimal value string.
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
void apZeroTerminatedListParameter::valueAsHexString(gtString& valueString) const
{
    if (1 < m_size)
    {
        valueString = '{';
        gtString currentValueString;

        for (unsigned int i = 0; i < m_size; i++)
        {
            const apParameter* pCurrentParam = m_pParametersVector[i].pointedObject();
            GT_IF_WITH_ASSERT(NULL != pCurrentParam)
            {
                pCurrentParam->valueAsHexString(currentValueString);
                valueString.append(currentValueString);
            }
            valueString.append(',').append(' ');
        }

        valueString.append(L"0x0}");
    }
    else if (1 == m_size)
    {
        // Empty array:
        valueString = L"{0x0}";
    }
    else
    {
        // Show the pointer value:
        gtUInt32 ptrValueAsUInt32 = (gtUInt32)m_ptrValue;
        valueString.appendFormattedString(GT_32_BIT_POINTER_FORMAT_UPPERCASE, ptrValueAsUInt32);
    }
}

// ---------------------------------------------------------------------------
// Name:        apZeroTerminatedListParameter::compareToOther
// Description: Compares this with other
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
bool apZeroTerminatedListParameter::compareToOther(const apParameter& other)const
{
    // Validate the type:
    bool retVal = (other.type() == type());

    if (retVal)
    {
        // Downcast the value:
        const apZeroTerminatedListParameter& otherAsList = (const apZeroTerminatedListParameter&)other;

        // If the size is equal:
        retVal = (otherAsList.m_size == m_size);

        if (retVal && (1 < m_size))
        {
            // Only compare members if the size is >= 2:
            for (unsigned int i = 0; retVal && (i < m_size); i++)
            {
                const apParameter* pThisMember = m_pParametersVector[i].pointedObject();
                const apParameter* pOtherMember = otherAsList.m_pParametersVector[i].pointedObject();
                GT_IF_WITH_ASSERT((NULL != pThisMember) && (NULL != pOtherMember))
                {
                    retVal = pThisMember->compareToOther(*pOtherMember);
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apZeroTerminatedListParameter::sizeOfArray
// Description: Calculates the array size from a pointer
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
unsigned int apZeroTerminatedListParameter::sizeOfArray(const void* pArray) const
{
    unsigned int retVal = 0;

    if (NULL != pArray)
    {
        gtSizeType sizeOfElement = (gtSizeType)sizeOfArrayElement();
        gtSizeType sizeToCopy = (sizeOfElement > sizeof(gtUInt64)) ? sizeof(gtUInt64) : sizeOfElement;
        GT_ASSERT(sizeOfElement == sizeToCopy); // Assert, but continue to avoid unexpected failures
        gtSizeType basePointer = (gtSizeType)pArray;
        gtUInt64 currentParam = 0;
        ::memcpy(&currentParam, (const void*)basePointer, sizeToCopy);

        while (0 != currentParam)
        {
            // Add one parameter name and one value:
            retVal += 2;

            // Copy the next parameter name:
            ::memcpy(&currentParam, (const void*)(basePointer + (sizeOfElement * retVal)), sizeToCopy);
        }

        // Denote a single element of 0 as "1" sized, to differentiate from a NULL value.
        if (0 == retVal)
        {
            retVal = 1;
        }

    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apZeroTerminatedListParameter::createElementsVector
// Description: Creates the elements vector
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
bool apZeroTerminatedListParameter::createElementsVector(unsigned int size)
{
    bool retVal = true;

    // Delete the previous vector, if relevant:
    deleteElementsVector();

    // Create the vector. Note that no initialization is needed since gtAutoPtr defaults to NULL.
    m_size = size;

    if (1 < m_size)
    {
        m_pParametersVector = new gtAutoPtr<apParameter>[m_size];
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apZeroTerminatedListParameter::deleteElementsVector
// Description: Deletes the elements vector
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
void apZeroTerminatedListParameter::deleteElementsVector()
{
    delete[] m_pParametersVector;
    m_pParametersVector = NULL;
    m_size = 0;
}


// ----------------------------- apBitfieldParameter ------------------------------

// ---------------------------------------------------------------------------
// Name:        apBitfieldParameter::apBitfieldParameter
// Description:
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
apBitfieldParameter::apBitfieldParameter()
{

}

// ---------------------------------------------------------------------------
// Name:        apBitfieldParameter::~apBitfieldParameter
// Description:
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
apBitfieldParameter::~apBitfieldParameter()
{

}

// ---------------------------------------------------------------------------
// Name:        apBitfieldParameter::valueAsString
// Description: Calculates the value of the parameter as a string.
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
void apBitfieldParameter::valueAsString(gtString& valueString) const
{
    // If this value is named:
    bool isSpecial = getSpecialValue(valueString);

    if (!isSpecial)
    {
        // Get the bitfield:
        gtUInt64 bitField = getBitfieldAsUInt64();

        // If the value is 0 and we don't have a special name for that case, output the string "0":
        if (0 == bitField)
        {
            valueString = '0';
        }
        else
        {
            valueString.makeEmpty();
            gtString currentBitName;

            gtUInt64 unhandledBits = 0;

            for (int i = 0; i < 64; i++)
            {
                gtUInt64 currentBit = 1ULL << i;

                // Stop if we reach the last tripped bit:
                if (currentBit > bitField)
                {
                    break;
                }
                else if (0 != (currentBit & bitField))
                {
                    // If the current bit is tripped:
                    bool isBitNamed = getBitName(i, currentBitName);

                    if (isBitNamed)
                    {
                        valueString.append(valueString.isEmpty() ? L"" : L" | ").append(currentBitName);
                    }
                    else
                    {
                        unhandledBits |= currentBit;
                    }
                }
            }

            // Add the unhandled / unknown bits, if any:
            if (0 != unhandledBits)
            {
                valueString.append(valueString.isEmpty() ? L"" : L" | ").appendFormattedString(L"%#llx", unhandledBits);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        apBitfieldParameter::getSpecialValue
// Description: This function allows customizing via override with a specific
//              special value (usually an "all" value such as 0xFFFF or a named
//              zero / "none" value).
//              Return true to avoid evaluation of the value as a string
//              Return false to proceed as normal.
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
bool apBitfieldParameter::getSpecialValue(gtString& o_string) const
{
    GT_UNREFERENCED_PARAMETER(o_string);
    return false;
}



// -----------------------------   apNotAvailableParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apNotAvailableParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apNotAvailableParameter::type() const
{
    return OS_TOBJ_ID_NOT_AVAILABLE_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apNotAvailableParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apNotAvailableParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    gtInt32 i = 1;
    ipcChannel << i;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apNotAvailableParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apNotAvailableParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtInt32 i = 0;
    ipcChannel >> i;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apNotAvailableParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        5/5/2004
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apNotAvailableParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    (void)(pArgumentList); // unused
    // Nothing to be done.
}

// ---------------------------------------------------------------------------
// Name:        apNotAvailableParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
void apNotAvailableParameter::readValueFromPointer(void* pValue)
{
    (void)(pValue); // unused
    // Nothing to be done.
}


// ---------------------------------------------------------------------------
// Name:        apNotAvailableParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
gtSizeType apNotAvailableParameter::sizeofData()
{
    return 0;
}



// ---------------------------------------------------------------------------
// Name:        apNotAvailableParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
void apNotAvailableParameter::valueAsString(gtString& valueString) const
{
    valueString = AP_STR_NotAvailable;
}


// ---------------------------------------------------------------------------
// Name:        apNotAvailableParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apNotAvailableParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        retVal = true;
    }

    return retVal;
}

// -----------------------------   apNotSupportedParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apNotSupportedParameter::apNotSupportedParameter
// Description: Constructor.
// Arguments: unsupportReason - The reason why this parameter is not supported.
// Author:  AMD Developer Tools Team
// Date:        22/2/2006
// ---------------------------------------------------------------------------
apNotSupportedParameter::apNotSupportedParameter(apUnsupportReason unsupportReason)
    : _unsupportReason(unsupportReason)
{
}


// ---------------------------------------------------------------------------
// Name:        apNotSupportedParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apNotSupportedParameter::type() const
{
    return OS_TOBJ_ID_NOT_SUPPORTED_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apNotSupportedParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apNotSupportedParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtInt32)_unsupportReason;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apNotSupportedParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apNotSupportedParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtInt32 unsupportReasonAsInt32 = AP_NOT_SUPPORTED_BY_HARDWARE;
    ipcChannel >> unsupportReasonAsInt32;
    _unsupportReason = (apUnsupportReason)unsupportReasonAsInt32;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apNotSupportedParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        5/5/2004
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apNotSupportedParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    (void)(pArgumentList); // unused
    // Nothing to be done.
}


// ---------------------------------------------------------------------------
// Name:        apNotSupportedParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
void apNotSupportedParameter::readValueFromPointer(void* pValue)
{
    (void)(pValue); // unused
    // Nothing to be done.
}


// ---------------------------------------------------------------------------
// Name:        apNotSupportedParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
gtSizeType apNotSupportedParameter::sizeofData()
{
    return 0;
}


// ---------------------------------------------------------------------------
// Name:        apNotSupportedParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
void apNotSupportedParameter::valueAsString(gtString& valueString) const
{
    valueString = AP_STR_Unsupported;

    switch (_unsupportReason)
    {
        case AP_NOT_SUPPORTED_BY_HARDWARE:
            valueString += L" by hardware";
            break;
    }
}


// ---------------------------------------------------------------------------
// Name:        apNotSupportedParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apNotSupportedParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        retVal = true;
    }

    return retVal;
}

// ----------------------------   apRemovedParameter -----------------------------

// ---------------------------------------------------------------------------
// Name:        apRemovedParameter::apRemovedParameter
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        4/1/2010
// ---------------------------------------------------------------------------
apRemovedParameter::apRemovedParameter(apAPIVersion removedVersion)
    : _removedVersion(removedVersion)
{
}

// ---------------------------------------------------------------------------
// Name:        apRemovedParameter::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        4/1/2010
// ---------------------------------------------------------------------------
osTransferableObjectType apRemovedParameter::type() const
{
    return OS_TOBJ_ID_REMOVED_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apRemovedParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        4/1/2010
// ---------------------------------------------------------------------------
bool apRemovedParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt32)_removedVersion;
    return true;
}

// ---------------------------------------------------------------------------
// Name:        apRemovedParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        4/1/2010
// ---------------------------------------------------------------------------
bool apRemovedParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt32 removedVersionAsUInt32 = 0;
    ipcChannel >> removedVersionAsUInt32;
    _removedVersion = (apAPIVersion)removedVersionAsUInt32;
    return true;
}

// ---------------------------------------------------------------------------
// Name:        apRemovedParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        4/1/2010
// ---------------------------------------------------------------------------
void apRemovedParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    (void)(pArgumentList); // unused
    // Nothing to be done.
}

// ---------------------------------------------------------------------------
// Name:        apRemovedParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        4/1/2010
// ---------------------------------------------------------------------------
void apRemovedParameter::readValueFromPointer(void* pValue)
{
    (void)(pValue); // unused
    // Nothing to be done.
}

// ---------------------------------------------------------------------------
// Name:        apRemovedParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        4/1/2010
// ---------------------------------------------------------------------------
gtSizeType apRemovedParameter::sizeofData()
{
    return 0;
}

// ---------------------------------------------------------------------------
// Name:        apRemovedParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        4/1/2010
// ---------------------------------------------------------------------------
void apRemovedParameter::valueAsString(gtString& valueString) const
{
    bool rcStr = apAPIVersionToString(_removedVersion, valueString);

    if (rcStr)
    {
        valueString.prepend(AP_STR_RemoveOpenGLVersion);
    }
    else // ! rcStr
    {
        GT_ASSERT(rcStr);
        valueString.makeEmpty();
    }
}

// ---------------------------------------------------------------------------
// Name:        apRemovedParameter::compareToOther
// Description: Compares this with other
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        4/1/2010
// ---------------------------------------------------------------------------
bool apRemovedParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        const apRemovedParameter& otherAsRemoved = (const apRemovedParameter&)other;

        if (this->_removedVersion == otherAsRemoved._removedVersion)
        {
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apRemovedParameter::removedVersion
// Description: Returns the version at which the parameter was removed.
// Author:  AMD Developer Tools Team
// Date:        4/1/2010
// ---------------------------------------------------------------------------
apAPIVersion apRemovedParameter::removedVersion()
{
    return _removedVersion;
}

// ----------------------------- apPseudoParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apPseudoParameter::isPseudoParameter
// Description: Returns true - yes I am a pseudo parameter.
// Author:  AMD Developer Tools Team
// Date:        6/1/2005
// ---------------------------------------------------------------------------
bool apPseudoParameter::isPseudoParameter() const
{
    return true;
}



// -----------------------------   apFloatParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apFloatParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apFloatParameter::type() const
{
    return OS_TOBJ_ID_FLOAT_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apFloatParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apFloatParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apFloatParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apFloatParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apFloatParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        5/5/2004
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apFloatParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    double argumentValue = va_arg(pArgumentList , double);
    _value = (float)argumentValue;
}

// ---------------------------------------------------------------------------
// Name:        apFloatParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
void apFloatParameter::readValueFromPointer(void* pValue)
{
    _value = *((float*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apFloatParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
gtSizeType apFloatParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(float);
    return sizeOfValue;
}



// ---------------------------------------------------------------------------
// Name:        apFloatParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
void apFloatParameter::valueAsString(gtString& valueString) const
{
    valueString.makeEmpty();
    const wchar_t* pFloatParamFormatString = apGetFloatParamsFormatString();
    valueString.appendFormattedString(pFloatParamFormatString, _value);
}

// ---------------------------------------------------------------------------
// Name:        apFloatParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apFloatParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apFloatParameter* pParam  = (apFloatParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}


// -----------------------------   apIntParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apIntParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apIntParameter::type() const
{
    return OS_TOBJ_ID_INT_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apIntParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apIntParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtInt32)_value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apIntParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apIntParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtInt32 valueAsInt32 = 0;
    ipcChannel >> valueAsInt32;
    _value = (int)valueAsInt32;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apIntParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        5/5/2004
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apIntParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    long argumentValue = va_arg(pArgumentList , long);
    _value = (int)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apIntParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
void apIntParameter::readValueFromPointer(void* pValue)
{
    _value = *((int*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apIntParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
gtSizeType apIntParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(int);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apIntParameter::valueAsHexString
// Description: Returns the parameter value as a string in hexadecimal display
// Arguments:   gtString& valueString
// Author:  AMD Developer Tools Team
// Date:        14/4/2011
// ---------------------------------------------------------------------------
void apIntParameter::valueAsHexString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(GT_UNSIGNED_INT_HEXADECIMAL_8_CHAR_FORMAT, (gtInt32)_value);
}

// ---------------------------------------------------------------------------
// Name:        apIntParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
void apIntParameter::valueAsString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(L"%d", _value);
}


// ---------------------------------------------------------------------------
// Name:        apIntParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apIntParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apIntParameter* pParam  = (apIntParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}


// ----------------------------- apUnsignedIntParameter ------------------------------

// ---------------------------------------------------------------------------
// Name:        apUnsignedIntParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        22/3/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apUnsignedIntParameter::type() const
{
    return OS_TOBJ_ID_UNSIGNED_INT_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apIntParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        22/3/2009
// ---------------------------------------------------------------------------
bool apUnsignedIntParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt32)_value;
    return true;
}

// ---------------------------------------------------------------------------
// Name:        apUnsignedIntParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        22/3/2009
// ---------------------------------------------------------------------------
bool apUnsignedIntParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt32 valueAsUInt32 = 0;
    ipcChannel >> valueAsUInt32;
    _value = (unsigned int)valueAsUInt32;

    return true;
}

// ---------------------------------------------------------------------------
// Name:        apUnsignedIntParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Implementation notes: See "The use of stdarg" at the top of this file.
// Author:  AMD Developer Tools Team
// Date:        22/3/2009
// ---------------------------------------------------------------------------
void apUnsignedIntParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    long argumentValue = va_arg(pArgumentList , long);
    _value = (unsigned int)argumentValue;
}

// ---------------------------------------------------------------------------
// Name:        apUnsignedIntParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        22/3/2009
// ---------------------------------------------------------------------------
void apUnsignedIntParameter::readValueFromPointer(void* pValue)
{
    _value = *((unsigned int*)(pValue));
}

// ---------------------------------------------------------------------------
// Name:        apUnsignedIntParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        22/3/2009
// ---------------------------------------------------------------------------
gtSizeType apUnsignedIntParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(unsigned int);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apUnsignedIntParameter::valueAsHexString
// Description: Returns the parameter value as a string in hexadecimal display
// Arguments:   gtString& valueString
// Author:  AMD Developer Tools Team
// Date:        14/4/2011
// ---------------------------------------------------------------------------
void apUnsignedIntParameter::valueAsHexString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(GT_UNSIGNED_INT_HEXADECIMAL_8_CHAR_FORMAT, (gtUInt32)_value);
}

// ---------------------------------------------------------------------------
// Name:        apUnsignedIntParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        22/3/2009
// ---------------------------------------------------------------------------
void apUnsignedIntParameter::valueAsString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(L"%u", _value);
}

// ---------------------------------------------------------------------------
// Name:        apUnsignedIntParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        22/3/2009
// ---------------------------------------------------------------------------
bool apUnsignedIntParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apUnsignedIntParameter* pParam  = (apUnsignedIntParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// ----------------------------- apSizeTParameter ------------------------------

// ---------------------------------------------------------------------------
// Name:        apSizeTParameter::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        8/11/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apSizeTParameter::type() const
{
    return OS_TOBJ_ID_SIZE_T_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apIntParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        8/11/2009
// ---------------------------------------------------------------------------
bool apSizeTParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt64)_value;
    return true;
}

// ---------------------------------------------------------------------------
// Name:        apSizeTParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        8/11/2009
// ---------------------------------------------------------------------------
bool apSizeTParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt64 valueAsUInt64 = 0;
    ipcChannel >> valueAsUInt64;
    _value = (size_t)valueAsUInt64;

    return true;
}

// ---------------------------------------------------------------------------
// Name:        apSizeTParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Implementation notes: See "The use of stdarg" at the top of this file.
// Author:  AMD Developer Tools Team
// Date:        8/11/2009
// ---------------------------------------------------------------------------
void apSizeTParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    long argumentValue = va_arg(pArgumentList , long);
    _value = (unsigned int)argumentValue;
}

// ---------------------------------------------------------------------------
// Name:        apSizeTParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        8/11/2009
// ---------------------------------------------------------------------------
void apSizeTParameter::readValueFromPointer(void* pValue)
{
    _value = *((size_t*)(pValue));
}

// ---------------------------------------------------------------------------
// Name:        apSizeTParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        8/11/2009
// ---------------------------------------------------------------------------
gtSizeType apSizeTParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(size_t);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apSizeTParameter::valueAsHexString
// Description: Returns the parameter value as a string in hexadecimal display
// Arguments:   gtString& valueString
// Author:  AMD Developer Tools Team
// Date:        14/4/2011
// ---------------------------------------------------------------------------
void apSizeTParameter::valueAsHexString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(GT_UNSIGNED_INT_HEXADECIMAL_8_CHAR_FORMAT, (gtUInt32)_value);
}

// ---------------------------------------------------------------------------
// Name:        apSizeTParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        8/11/2009
// ---------------------------------------------------------------------------
void apSizeTParameter::valueAsString(gtString& valueString) const
{
    valueString.makeEmpty();
    gtUInt64 valueAsUInt64 = (gtUInt64)_value;
    valueString.appendFormattedString(L"%llu", valueAsUInt64);
}

// ---------------------------------------------------------------------------
// Name:        apSizeTParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        8/11/2009
// ---------------------------------------------------------------------------
bool apSizeTParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apSizeTParameter* pParam  = (apSizeTParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// ----------------------------- apBytesSizeParameter ------------------------

// ---------------------------------------------------------------------------
// Name:        apBytesSizeParameter::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        11/3/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apBytesSizeParameter::type() const
{
    return OS_TOBJ_ID_BYTES_SIZE_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apBytesSizeParameter::valueAsHexString
// Description: Returns the parameter value as a string in hexadecimal display
// Arguments:   gtString& valueString
// Author:  AMD Developer Tools Team
// Date:        14/4/2011
// ---------------------------------------------------------------------------
void apBytesSizeParameter::valueAsHexString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(GT_UNSIGNED_INT_HEXADECIMAL_8_CHAR_FORMAT, (gtUInt32)_value);
}

// ---------------------------------------------------------------------------
// Name:        apBytesSizeParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        11/3/2010
// ---------------------------------------------------------------------------
void apBytesSizeParameter::valueAsString(gtString& valueString) const
{
    gtUInt64 valueAsUInt64 = (gtUInt64)_value;
    valueString.makeEmpty();

    // Build the string from memory size:
    valueString.fromMemorySize(valueAsUInt64);
}

// -----------------------------   apStringParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apStringParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apStringParameter::type() const
{
    return OS_TOBJ_ID_STRING_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apStringParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apStringParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apStringParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apStringParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apStringParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        5/5/2004
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apStringParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    char* argumentValue = va_arg(pArgumentList , char*);
    _value = argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apStringParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
void apStringParameter::readValueFromPointer(void* pValue)
{
    _value = (char*)pValue;
}


// ---------------------------------------------------------------------------
// Name:        apStringParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
gtSizeType apStringParameter::sizeofData()
{
    // This function should not be called !!!
    GT_ASSERT(0);

    static gtSizeType sizeOfChar = sizeof(char);
    return sizeOfChar;
}


// ---------------------------------------------------------------------------
// Name:        apStringParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
void apStringParameter::valueAsString(gtString& valueString) const
{
    valueString.fromASCIIString(_value.asCharArray());
}

// ---------------------------------------------------------------------------
// Name:        apStringParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apStringParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apStringParameter* pParam  = (apStringParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}
