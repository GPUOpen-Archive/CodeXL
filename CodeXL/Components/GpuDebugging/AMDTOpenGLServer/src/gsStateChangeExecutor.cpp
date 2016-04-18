//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsStateChangeExecutor.cpp
///
//==================================================================================

//------------------------------ gsStateChangeExecutor.cpp ------------------------------

// Standard C:
#include <limits.h>
#include <AMDTOSWrappers/Include/osStdLibIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTAPIClasses/Include/apOpenGLParameters.h>

// Local:
#include <src/gsStateChangeExecutor.h>
// ---------------------------------------------------------------------------
// Name:        gsStateChangeExecutor::gsStateChangeExecutor
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        20/8/2008
// ---------------------------------------------------------------------------
gsStateChangeExecutor::gsStateChangeExecutor(gsStateVariablesSnapshot* pStateVariableSnapShot)
{
    _pStateVariableSnapShot = pStateVariableSnapShot;
}

// ---------------------------------------------------------------------------
// Name:        gsStateChangeExecutor::gsStateChangeExecutor
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        20/8/2008
// ---------------------------------------------------------------------------
gsStateChangeExecutor::~gsStateChangeExecutor()
{
}

// ---------------------------------------------------------------------------
// Name:        gsStateChangeExecutor::applyStateChange
// Description: Is called before a function call is executed. This function simulated OpenGL state
//              variable values change, apply this change on the snapshot, and returns the redundancy status of
//              the function.
//              The function also translates certain type of parameters to OpenGL state variable type.
// Arguments: int calledFunctionId - the function id
//            int argumentsAmount - the argument amount
//            va_list& pArgumentList - the list of argument
//            apFunctionRedundancyStatus& redundancyStatus - the function call redundancy status.
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        20/8/2008
// ---------------------------------------------------------------------------
bool gsStateChangeExecutor::applyStateChange(apMonitoredFunctionId calledFunctionId, int argumentsAmount, va_list& pArgumentList, apFunctionRedundancyStatus& redundancyStatus)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pStateVariableSnapShot != NULL)
    {
        switch (calledFunctionId)
        {
            // Color Functions:
            case ap_glColor3b:
            case ap_glColor3bv:
            case ap_glColor3ub:
            case ap_glColor3ubv:
            case ap_glColor3d:
            case ap_glColor3dv:
            case ap_glColor3f:
            case ap_glColor3fv:
            case ap_glColor3i:
            case ap_glColor3iv:
            case ap_glColor3s:
            case ap_glColor3sv:
            case ap_glColor3ui:
            case ap_glColor3uiv:
            case ap_glColor3us:
            case ap_glColor3usv:
            case ap_glColor4b:
            case ap_glColor4bv:
            case ap_glColor4d:
            case ap_glColor4dv:
            case ap_glColor4f:
            case ap_glColor4fv:
            case ap_glColor4i:
            case ap_glColor4iv:
            case ap_glColor4s:
            case ap_glColor4sv:
            case ap_glColor4ub:
            case ap_glColor4ubv:
            case ap_glColor4ui:
            case ap_glColor4uiv:
            case ap_glColor4us:
            case ap_glColor4usv:
                retVal = applyColorStateChange(calledFunctionId, argumentsAmount, pArgumentList, redundancyStatus);
                break;

                // OpenGL ES is currently supported on windows only:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

            case ap_glColor4x:
                retVal = applyColorStateChange(calledFunctionId, argumentsAmount, pArgumentList, redundancyStatus);
                break;
#endif

            // Index Functions:
            case ap_glIndexd:
            case ap_glIndexdv:
            case ap_glIndexf:
            case ap_glIndexfv:
            case ap_glIndexi:
            case ap_glIndexiv:
            case ap_glIndexs:
            case ap_glIndexsv:
            case ap_glIndexub:
            case ap_glIndexubv:
                retVal = applyIndexStateChange(calledFunctionId, argumentsAmount, pArgumentList, redundancyStatus);
                break;

            // Secondary Color Functions:
            case ap_glSecondaryColor3b:
            case ap_glSecondaryColor3bv:
            case ap_glSecondaryColor3d:
            case ap_glSecondaryColor3dv:
            case ap_glSecondaryColor3f:
            case ap_glSecondaryColor3fv:
            case ap_glSecondaryColor3i:
            case ap_glSecondaryColor3iv:
            case ap_glSecondaryColor3s:
            case ap_glSecondaryColor3sv:
            case ap_glSecondaryColor3ub:
            case ap_glSecondaryColor3ubv:
            case ap_glSecondaryColor3ui:
            case ap_glSecondaryColor3uiv:
            case ap_glSecondaryColor3us:
            case ap_glSecondaryColor3usv:
                retVal = applySecondaryColorStateChange(calledFunctionId, argumentsAmount, pArgumentList, redundancyStatus);
                break;

            // Index Functions:
            case ap_glMaterialf:
            case ap_glMaterialfv:
            case ap_glMateriali:
            case ap_glMaterialiv:
                retVal = applyMaterialStateChange(calledFunctionId, argumentsAmount, pArgumentList, redundancyStatus);
                break;

            default:
                retVal = false;
                break;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsStateChangeExecutor::getColorFloatRGBAValuesFromByte
// Description: Extract float r,g,b,a values from unsigned byte arguments
// Arguments:
//            int argumentsAmount - amount of arguments
//            va_list& pArgumentList - the argument list
//            GLfloat& rAsFloat - output - R value as float
//            GLfloat& gAsFloat - output - G value as float
//            GLfloat& bAsFloat - output - B value as float
//            GLfloat& aAsFloat - output - A value as float
//            bool isPointer - are the arguments given as pointer
//            bool is4Arguments - are there 4 arguments
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/8/2008
// ---------------------------------------------------------------------------
bool gsStateChangeExecutor::getColorFloatRGBAValuesFromByte(int argumentsAmount, va_list& pArgumentList, GLfloat& rAsFloat, GLfloat& gAsFloat, GLfloat& bAsFloat, GLfloat& aAsFloat, bool isPointer, bool is4Arguments)
{
    bool retVal = false;

    // Iterate on the argument list:
    va_list pCurrentArgument;
    va_copy(pCurrentArgument, pArgumentList);

    GLbyte rAsByte = 0;
    GLbyte gAsByte = 0;
    GLbyte bAsByte = 0;
    GLbyte aAsByte = 1;

    if (!isPointer)
    {
        // Get the byte value for r,g,b,a according to the arguments type:
        GT_IF_WITH_ASSERT(argumentsAmount >= 3)
        {
            // Read the function parameters:
            osTransferableObjectType argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
            GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_BYTE_PARAMETER)
            {
                // Get the first argument:
                long argumentValue = va_arg(pCurrentArgument , long);
                rAsByte = (GLbyte)argumentValue;

                // Get the second argument:
                argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
                GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_BYTE_PARAMETER)
                {
                    argumentValue = va_arg(pCurrentArgument , long);
                    gAsByte = (GLbyte)argumentValue;
                    argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
                    // Get the third argument:
                    GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_BYTE_PARAMETER)
                    {
                        argumentValue = va_arg(pCurrentArgument , long);
                        bAsByte = (GLbyte)argumentValue;

                        // Get the a value if given in arguments:
                        if (is4Arguments == true)
                        {
                            argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
                            GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_BYTE_PARAMETER)
                            {
                                argumentValue = va_arg(pCurrentArgument , long);
                                aAsByte = (GLbyte)argumentValue;
                                retVal = true;
                            }
                        }
                        else
                        {
                            retVal = true;
                        }
                    }

                }

            }
        }
    }
    else
    {
        // Arguments are given as pointers:
        osTransferableObjectType argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
        GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_VECTOR_PARAMETER)
        {
            // Read the pointer argument type:
            argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
            GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_FLOAT_PARAMETER)
            {
                // skip the number of items in the vector:
                (void) va_arg(pCurrentArgument , int);

                // Get a pointer to the "C" vector that contains the values:
                void* pCVector = va_arg(pCurrentArgument , void*);

                // Get the float values:
                GLbyte* pByteVals = (GLbyte*)pCVector;
                rAsByte = pByteVals[0];
                gAsByte = pByteVals[1];
                bAsByte = pByteVals[2];

                if (is4Arguments)
                {
                    aAsByte = pByteVals[3];
                }
            }
        }
    }

    // If byte arguments value was extracted correctly, translate to float:
    if (retVal == true)
    {
        // Translate the values to float:
        rAsFloat = (GLfloat)rAsByte / CHAR_MAX;
        gAsFloat = (GLfloat)gAsByte / CHAR_MAX;
        bAsFloat = (GLfloat)bAsByte / CHAR_MAX;

        if (aAsByte != 1)
        {
            aAsFloat = (GLfloat)aAsByte / CHAR_MAX;
        }
    }

    // Free the arguments pointer:
    va_end(pCurrentArgument);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsStateChangeExecutor::getFloatRGBAValuesFromUByte
// Description: Extract float r,g,b,a values from ubyte arguments
// Arguments:
//            int argumentsAmount - amount of arguments
//            va_list& pArgumentList - the argument list
//            GLfloat& rAsFloat - output - R value as float
//            GLfloat& gAsFloat - output - G value as float
//            GLfloat& bAsFloat - output - B value as float
//            GLfloat& aAsFloat - output - A value as float
//            bool isPointer - are the arguments given as pointer
//            bool is4Arguments - are there 4 arguments
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/8/2008
// ---------------------------------------------------------------------------
bool gsStateChangeExecutor::getColorFloatRGBAValuesFromUByte(int argumentsAmount, va_list& pArgumentList, GLfloat& rAsFloat, GLfloat& gAsFloat, GLfloat& bAsFloat, GLfloat& aAsFloat, bool isPointer, bool is4Arguments)
{
    bool retVal = false;

    // Iterate on the argument list:
    va_list pCurrentArgument;
    va_copy(pCurrentArgument, pArgumentList);

    GLbyte rAsByte = 0;
    GLbyte gAsByte = 0;
    GLbyte bAsByte = 0;
    GLbyte aAsByte = 1;

    if (!isPointer)
    {
        // Get the byte value for r,g,b,a according to the arguments type:
        GT_IF_WITH_ASSERT(argumentsAmount >= 3)
        {
            // Read the function parameters:
            osTransferableObjectType argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
            GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_BYTE_PARAMETER)
            {
                // Get the first argument:
                long argumentValue = va_arg(pCurrentArgument , long);
                rAsByte = (GLbyte)argumentValue;

                // Get the second argument:
                argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
                GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_BYTE_PARAMETER)
                {
                    argumentValue = va_arg(pCurrentArgument , long);
                    gAsByte = (GLbyte)argumentValue;
                    argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
                    // Get the third argument:
                    GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_BYTE_PARAMETER)
                    {
                        argumentValue = va_arg(pCurrentArgument , long);
                        bAsByte = (GLbyte)argumentValue;

                        // Get the a value if given in arguments:
                        if (is4Arguments == true)
                        {
                            argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
                            GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_BYTE_PARAMETER)
                            {
                                argumentValue = va_arg(pCurrentArgument , long);
                                aAsByte = (GLbyte)argumentValue;
                                retVal = true;
                            }
                        }
                        else
                        {
                            retVal = true;
                        }
                    }

                }

            }
        }
    }
    else
    {
        // Arguments are given as pointers:
        osTransferableObjectType argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
        GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_VECTOR_PARAMETER)
        {
            // Read the pointer argument type:
            argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
            GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_FLOAT_PARAMETER)
            {
                // Skip the number of items in the vector:
                (void) va_arg(pCurrentArgument , int);

                // Get a pointer to the "C" vector that contains the values:
                void* pCVector = va_arg(pCurrentArgument , void*);

                // Get the float values:
                GLbyte* pByteVals = (GLbyte*)pCVector;
                rAsByte = pByteVals[0];
                gAsByte = pByteVals[1];
                bAsByte = pByteVals[2];

                if (is4Arguments)
                {
                    aAsByte = pByteVals[3];
                }
            }
        }
    }

    // If byte arguments value was extracted correctly, translate to float:
    if (retVal == true)
    {
        // Translate the values to float:
        rAsFloat = (GLfloat)rAsByte / CHAR_MAX;
        gAsFloat = (GLfloat)gAsByte / CHAR_MAX;
        bAsFloat = (GLfloat)bAsByte / CHAR_MAX;

        if (aAsByte != 1)
        {
            aAsFloat = (GLfloat)aAsByte / CHAR_MAX;
        }
    }

    // Free the arguments pointer:
    va_end(pCurrentArgument);

    return retVal;

}
// --------------------------------------------------------------------------
// Name:        gsStateChangeExecutor::getColorFloatRGBAValuesFromInt
// Description: Extract float r,g,b,a values from Int arguments
// Arguments:
//            int argumentsAmount - amount of arguments
//            va_list& pArgumentList - the argument list
//            GLfloat& rAsFloat - output - R value as float
//            GLfloat& gAsFloat - output - G value as float
//            GLfloat& bAsFloat - output - B value as float
//            GLfloat& aAsFloat - output - A value as float
//            bool isPointer - are the arguments given as pointer
//            bool is4Arguments - are there 4 arguments
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/8/2008
// ---------------------------------------------------------------------------
bool gsStateChangeExecutor::getColorFloatRGBAValuesFromInt(int argumentsAmount, va_list& pArgumentList, GLfloat& rAsFloat, GLfloat& gAsFloat, GLfloat& bAsFloat, GLfloat& aAsFloat, bool isPointer, bool is4Arguments)
{
    (void)(argumentsAmount); // unused
    // Iterate on the argument list:
    va_list pCurrentArgument;
    va_copy(pCurrentArgument, pArgumentList);

    bool retVal = false;

    if (!isPointer)
    {
        // Get the first argument type:
        osTransferableObjectType argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
        GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_INT_PARAMETER)
        {
            int argumentValue = va_arg(pCurrentArgument , int);
            rAsFloat = (GLfloat)argumentValue;
            argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
            GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_INT_PARAMETER)
            {
                argumentValue = va_arg(pCurrentArgument , int);
                gAsFloat = (GLfloat)argumentValue;
                argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
                GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_INT_PARAMETER)
                {
                    argumentValue = va_arg(pCurrentArgument , int);
                    bAsFloat = (GLfloat)argumentValue;
                    retVal = true;

                    if (is4Arguments)
                    {
                        argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));

                        if (argumentType == OS_TOBJ_ID_GL_INT_PARAMETER)
                        {
                            argumentValue = va_arg(pCurrentArgument , int);
                            aAsFloat = (GLfloat)argumentValue;
                        }
                        else
                        {
                            GT_ASSERT_EX(false, L"Unexpectged argument");
                            retVal = false;
                        }
                    }
                }
            }
        }
    }
    else
    {
        // Read the argument type:
        osTransferableObjectType argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
        GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_VECTOR_PARAMETER)
        {
            // Read the pointer argument type:
            argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
            GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_INT_PARAMETER)
            {
                // skip the number of items in the vector:
                (void) va_arg(pCurrentArgument , int);

                // Get a pointer to the "C" vector that contains the values:
                void* pCVector = va_arg(pCurrentArgument , void*);

                // Get the float values:
                GLint* pIntVals = (GLint*)pCVector;
                rAsFloat = (GLfloat) pIntVals[0];
                gAsFloat = (GLfloat) pIntVals[1];
                bAsFloat = (GLfloat) pIntVals[2];

                if (is4Arguments)
                {
                    aAsFloat = (GLfloat)pIntVals[3];
                }

                retVal = true;
            }
        }
    }

    // Free the arguments pointer:
    va_end(pCurrentArgument);
    return retVal;
}

// --------------------------------------------------------------------------
// Name:        gsStateChangeExecutor::getColorFloatRGBAValuesFromUInt
// Description: Extract float r,g,b,a values from unsgiend int arguments
// Arguments:
//            int argumentsAmount - amount of arguments
//            va_list& pArgumentList - the argument list
//            GLfloat& rAsFloat - output - R value as float
//            GLfloat& gAsFloat - output - G value as float
//            GLfloat& bAsFloat - output - B value as float
//            GLfloat& aAsFloat - output - A value as float
//            bool isPointer - are the arguments given as pointer
//            bool is4Arguments - are there 4 arguments
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/8/2008
// ---------------------------------------------------------------------------
bool gsStateChangeExecutor::getColorFloatRGBAValuesFromUInt(int argumentsAmount, va_list& pArgumentList, GLfloat& rAsFloat, GLfloat& gAsFloat, GLfloat& bAsFloat, GLfloat& aAsFloat, bool isPointer, bool is4Arguments)
{
    (void)(argumentsAmount); // unused
    // Iterate on the argument list:
    va_list pCurrentArgument;
    va_copy(pCurrentArgument, pArgumentList);

    bool retVal = false;

    if (isPointer)
    {
        // Get the first argument type:
        osTransferableObjectType argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
        GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_UINT_PARAMETER)
        {
            unsigned long argumentValue = va_arg(pCurrentArgument , unsigned long);
            rAsFloat = (GLfloat)argumentValue;
            argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
            GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_UINT_PARAMETER)
            {
                argumentValue = va_arg(pCurrentArgument , unsigned long);
                gAsFloat = (GLfloat)argumentValue;
                argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
                GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_UINT_PARAMETER)
                {
                    argumentValue = va_arg(pCurrentArgument , unsigned long);
                    bAsFloat = (GLfloat)argumentValue;
                    retVal = true;

                    if (is4Arguments)
                    {
                        argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));

                        if (argumentType == OS_TOBJ_ID_GL_UINT_PARAMETER)
                        {
                            argumentValue = va_arg(pCurrentArgument , unsigned long);
                            aAsFloat = (GLfloat)argumentValue;
                        }
                        else
                        {
                            GT_ASSERT_EX(false, L"Unexpectged argument");
                            retVal = false;
                        }
                    }
                }
            }
        }
    }
    // Arguments are pointer:
    else
    {
        // Read the argument type:
        osTransferableObjectType argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
        GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_VECTOR_PARAMETER)
        {
            // Read the pointer argument type:
            argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
            GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_UINT_PARAMETER)
            {
                // skip the number of items in the vector:
                (void) va_arg(pCurrentArgument , int);

                // Get a pointer to the "C" vector that contains the values:
                void* pCVector = va_arg(pCurrentArgument , void*);

                // Get the float values:
                GLuint* pIntVals = (GLuint*)pCVector;
                rAsFloat = (GLfloat) pIntVals[0];
                gAsFloat = (GLfloat) pIntVals[1];
                bAsFloat = (GLfloat) pIntVals[2];

                if (is4Arguments)
                {
                    aAsFloat = (GLfloat)pIntVals[3];
                }

                retVal = true;
            }
        }
    }

    // Free the arguments pointer:
    va_end(pCurrentArgument);
    return retVal;
}


// --------------------------------------------------------------------------
// Name:        gsStateChangeExecutor::getColorFloatRGBAValuesFromShort
// Description: Extract float r,g,b,a values from short arguments
// Arguments:
//            int argumentsAmount - amount of arguments
//            va_list& pArgumentList - the argument list
//            GLfloat& rAsFloat - output - R value as float
//            GLfloat& gAsFloat - output - G value as float
//            GLfloat& bAsFloat - output - B value as float
//            GLfloat& aAsFloat - output - A value as float
//            bool isPointer - are the arguments given as pointer
//            bool is4Arguments - are there 4 arguments
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/8/2008
// ---------------------------------------------------------------------------
bool gsStateChangeExecutor::getColorFloatRGBAValuesFromShort(int argumentsAmount, va_list& pArgumentList, GLfloat& rAsFloat, GLfloat& gAsFloat, GLfloat& bAsFloat, GLfloat& aAsFloat, bool isPointer, bool is4Arguments)
{
    (void)(argumentsAmount); // unused
    // Iterate on the argument list:
    va_list pCurrentArgument;
    va_copy(pCurrentArgument, pArgumentList);

    bool retVal = false;

    if (isPointer)
    {
        // Get the first argument type:
        osTransferableObjectType argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
        GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_SHORT_PARAMETER)
        {
            long argumentValue = va_arg(pCurrentArgument , long);
            rAsFloat = (GLfloat)argumentValue;
            argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
            GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_SHORT_PARAMETER)
            {
                argumentValue = va_arg(pCurrentArgument , long);
                gAsFloat = (GLfloat)argumentValue;
                argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
                GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_SHORT_PARAMETER)
                {
                    argumentValue = va_arg(pCurrentArgument , long);
                    bAsFloat = (GLfloat)argumentValue;
                    retVal = true;

                    if (is4Arguments)
                    {
                        argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));

                        if (argumentType == OS_TOBJ_ID_GL_SHORT_PARAMETER)
                        {
                            argumentValue = va_arg(pCurrentArgument , long);
                            aAsFloat = (GLfloat)argumentValue;
                        }
                        else
                        {
                            GT_ASSERT_EX(false, L"Unexpectged argument");
                            retVal = false;
                        }
                    }
                }
            }
        }
    }
    // Arguments are pointer:
    else
    {
        // Read the argument type:
        osTransferableObjectType argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
        GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_VECTOR_PARAMETER)
        {
            // Read the pointer argument type:
            argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
            GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_FLOAT_PARAMETER)
            {
                // Skip the number of items in the vector:
                (void) va_arg(pCurrentArgument , int);

                // Get a pointer to the "C" vector that contains the values:
                void* pCVector = va_arg(pCurrentArgument , void*);

                // Get the float values:
                GLshort* pShortVals = (GLshort*)pCVector;
                rAsFloat = (GLfloat) pShortVals[0];
                gAsFloat = (GLfloat) pShortVals[1];
                bAsFloat = (GLfloat) pShortVals[2];

                if (is4Arguments)
                {
                    aAsFloat = (GLfloat)pShortVals[3];
                }

                retVal = true;
            }
        }
    }

    // Free the arguments pointer:
    va_end(pCurrentArgument);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsStateChangeExecutor::getColorFloatRGBAValuesFromFixed
// Description: Extract float r,g,b,a values from fixed arguments
// Arguments:
//            int argumentsAmount
//            va_list& pArgumentList
//            GLfloat& rAsFloat
//            GLfloat& gAsFloat
//            GLfloat& bAsFloat
//            GLfloat& aAsFloat
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        20/8/2008
// ---------------------------------------------------------------------------
bool gsStateChangeExecutor::getColorFloatRGBAValuesFromFixed(int argumentsAmount, va_list& pArgumentList, GLfloat& rAsFloat, GLfloat& gAsFloat, GLfloat& bAsFloat, GLfloat& aAsFloat)
{
    (void)(argumentsAmount); // unused
    bool retVal = false;

    // OpenGL ES is currently supported on windows only:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    {
        // Iterate on the argument list:
        va_list pCurrentArgument;
        va_copy(pCurrentArgument, pArgumentList);

        // Get the first argument type:
        osTransferableObjectType argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
        GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_FIXED_PARAMETER)
        {
            int argumentValue = va_arg(pCurrentArgument , long);
            rAsFloat = GLfloat(apGLfixedParameter::fixedToFloat(argumentValue));
            argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
            GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_FIXED_PARAMETER)
            {
                argumentValue = va_arg(pCurrentArgument , long);
                gAsFloat = GLfloat(apGLfixedParameter::fixedToFloat(argumentValue));
                argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
                GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_FIXED_PARAMETER)
                {
                    argumentValue = va_arg(pCurrentArgument , long);
                    bAsFloat = GLfloat(apGLfixedParameter::fixedToFloat(argumentValue));
                    argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));

                    if (argumentType == OS_TOBJ_ID_GL_FIXED_PARAMETER)
                    {
                        argumentValue = va_arg(pCurrentArgument , long);
                        aAsFloat = GLfloat(apGLfixedParameter::fixedToFloat(argumentValue));
                        retVal = true;
                    }

                }
            }
        }

        // Free the arguments pointer:
        va_end(pCurrentArgument);
    }
#else
    // Resolve the compiler warning for the Linux variant
    (void)(pArgumentList);
    (void)(rAsFloat);
    (void)(gAsFloat);
    (void)(bAsFloat);
    (void)(aAsFloat);
#endif

    return retVal;
}


// --------------------------------------------------------------------------
// Name:        gsStateChangeExecutor::getColorFloatRGBAValuesFromUShort
// Description: Extract float r,g,b,a values from short arguments
// Arguments: apMonitoredFunctionId calledFunctionId
//            int argumentsAmount - amount of arguments
//            va_list& pArgumentList - the argument list
//            GLfloat& rAsFloat - output - R value as float
//            GLfloat& gAsFloat - output - G value as float
//            GLfloat& bAsFloat - output - B value as float
//            GLfloat& aAsFloat - output - A value as float
//            bool isPointer - are the arguments given as pointer
//            bool is4Arguments - are there 4 arguments
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/8/2008
// ---------------------------------------------------------------------------
bool gsStateChangeExecutor::getColorFloatRGBAValuesFromUShort(int argumentsAmount, va_list& pArgumentList, GLfloat& rAsFloat, GLfloat& gAsFloat, GLfloat& bAsFloat, GLfloat& aAsFloat, bool isPointer, bool is4Arguments)
{
    (void)(argumentsAmount); // unused
    // Iterate on the argument list:
    va_list pCurrentArgument;
    va_copy(pCurrentArgument, pArgumentList);

    bool retVal = false;

    if (isPointer)
    {
        // Get the first argument type:
        osTransferableObjectType argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
        GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_USHORT_PARAMETER)
        {
            unsigned long argumentValue = va_arg(pCurrentArgument , unsigned long);
            rAsFloat = (GLfloat)argumentValue;
            argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
            GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_USHORT_PARAMETER)
            {
                argumentValue = va_arg(pCurrentArgument , long);
                gAsFloat = (GLfloat)argumentValue;
                argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
                GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_USHORT_PARAMETER)
                {
                    argumentValue = va_arg(pCurrentArgument , unsigned long);
                    bAsFloat = (GLfloat)argumentValue;
                    retVal = true;

                    if (is4Arguments)
                    {
                        argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));

                        if (argumentType == OS_TOBJ_ID_GL_USHORT_PARAMETER)
                        {
                            argumentValue = va_arg(pCurrentArgument , unsigned long);
                            aAsFloat = (GLfloat)argumentValue;
                        }
                        else
                        {
                            GT_ASSERT_EX(false, L"Unexpectged argument");
                            retVal = false;
                        }
                    }
                }
            }
        }
    }
    // Arguments are pointer:
    else
    {
        // Read the argument type:
        osTransferableObjectType argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
        GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_VECTOR_PARAMETER)
        {
            // Read the pointer argument type:
            argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
            GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_FLOAT_PARAMETER)
            {
                // Skip the number of items in the vector:
                (void) va_arg(pCurrentArgument , int);

                // Get a pointer to the "C" vector that contains the values:
                void* pCVector = va_arg(pCurrentArgument , void*);

                // Get the float values:
                GLushort* pUShortVals = (GLushort*)pCVector;
                rAsFloat = (GLfloat) pUShortVals[0];
                gAsFloat = (GLfloat) pUShortVals[1];
                bAsFloat = (GLfloat) pUShortVals[2];

                if (is4Arguments)
                {
                    aAsFloat = (GLfloat)pUShortVals[3];
                }

                retVal = true;
            }
        }
    }

    // Free the arguments pointer:
    va_end(pCurrentArgument);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsStateChangeExecutor::getColorFloatRGBAValuesFromDouble
// Description: The function receives as parameter glColor arguments expected to
//              be double arguments, and translate it to float number represent RGBA values
// Arguments:
//            int argumentsAmount
//            va_list& pArgumentList
//            GLfloat& rAsFloat
//            GLfloat& gAsFloat
//            GLfloat& bAsFloat
//            GLfloat& aAsFloat
//            bool isPointer - are the arguments received as pointer
//            bool is4Arguments - are there 4 arguments
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        21/8/2008
// ---------------------------------------------------------------------------
bool gsStateChangeExecutor::getColorFloatRGBAValuesFromDouble(int argumentsAmount, va_list& pArgumentList, GLfloat& rAsFloat, GLfloat& gAsFloat, GLfloat& bAsFloat, GLfloat& aAsFloat, bool isPointer, bool is4Arguments)
{
    (void)(argumentsAmount); // unused
    // Iterate on the argument list:
    va_list pCurrentArgument;
    va_copy(pCurrentArgument, pArgumentList);

    bool retVal = false;

    if (isPointer)
    {
        // Get the first argument type:
        osTransferableObjectType argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
        GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_DOUBLE_PARAMETER)
        {
            double argumentValue = va_arg(pCurrentArgument , double);
            rAsFloat = (GLfloat)argumentValue;
            argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
            GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_DOUBLE_PARAMETER)
            {
                argumentValue = va_arg(pCurrentArgument , double);
                gAsFloat = (GLfloat)argumentValue;
                argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
                GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_DOUBLE_PARAMETER)
                {
                    argumentValue = va_arg(pCurrentArgument , double);
                    bAsFloat = (GLfloat)argumentValue;
                    retVal = true;

                    if (is4Arguments)
                    {
                        argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));

                        if (argumentType == OS_TOBJ_ID_GL_DOUBLE_PARAMETER)
                        {
                            argumentValue = va_arg(pCurrentArgument , double);
                            aAsFloat = (GLfloat)argumentValue;
                        }
                        else
                        {
                            GT_ASSERT_EX(false, L"Unexpectged argument");
                            retVal = false;
                        }
                    }
                }
            }
        }
    }
    else
    {
        // Read the argument type:
        osTransferableObjectType argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
        GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_VECTOR_PARAMETER)
        {
            // Read the pointer argument type:
            argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
            GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_DOUBLE_PARAMETER)
            {
                // Skip the number of items in the vector:
                (void) va_arg(pCurrentArgument , int);

                // Get a pointer to the "C" vector that contains the values:
                void* pCVector = va_arg(pCurrentArgument , void*);

                // Get the float values:
                GLdouble* pDoubleVals = (GLdouble*)pCVector;
                rAsFloat = (GLfloat) pDoubleVals[0];
                gAsFloat = (GLfloat) pDoubleVals[1];
                bAsFloat = (GLfloat) pDoubleVals[2];

                if (is4Arguments)
                {
                    aAsFloat = (GLfloat)pDoubleVals[3];
                }

                retVal = true;
            }
        }
    }

    // Free the arguments pointer:
    va_end(pCurrentArgument);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsStateChangeExecutor::getFloatRGBAValuesFromDouble
// Description: The function recieves as parameter glColor arguments expceted to
//              be float parguments, and translate it to float number represent RGBA values
// Arguments:
//            int argumentsAmount
//            va_list& pArgumentList
//            GLfloat& rAsFloat
//            GLfloat& gAsFloat
//            GLfloat& bAsFloat
//            GLfloat& aAsFloat
//            bool isPointer - are the arguments received as pointer
//            bool is4Arguments - are there 4 arguments
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        21/8/2008
// ---------------------------------------------------------------------------
bool gsStateChangeExecutor::getColorFloatRGBAValuesFromFloat(int argumentsAmount, va_list& pArgumentList, GLfloat& rAsFloat, GLfloat& gAsFloat, GLfloat& bAsFloat, GLfloat& aAsFloat, bool isPointer, bool is4Arguments)
{
    (void)(argumentsAmount); // unused
    bool retVal = false;
    // Iterate on the argument list:
    va_list pCurrentArgument;
    va_copy(pCurrentArgument, pArgumentList);

    if (!isPointer)
    {
        // GEt the first argument type:
        osTransferableObjectType argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
        GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_FLOAT_PARAMETER)
        {
            double argumentValue = va_arg(pCurrentArgument , double);
            rAsFloat = (GLfloat)argumentValue;
            argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
            GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_FLOAT_PARAMETER)
            {
                argumentValue = va_arg(pCurrentArgument , double);
                gAsFloat = (GLfloat)argumentValue;
                argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
                GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_FLOAT_PARAMETER)
                {
                    argumentValue = va_arg(pCurrentArgument , double);
                    bAsFloat = (GLfloat)argumentValue;
                    retVal = true;

                    if (is4Arguments)
                    {
                        argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
                        GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_FLOAT_PARAMETER)
                        {
                            argumentValue = va_arg(pCurrentArgument , double);
                            aAsFloat = (GLfloat)argumentValue;
                        }
                        else
                        {
                            retVal = false;
                        }
                    }
                }
            }
        }
    }
    else
    {
        // ap_glColor4fv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_FLOAT_PARAMETER, 4, v);
        // Read the argument type:
        osTransferableObjectType argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
        GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_VECTOR_PARAMETER)
        {
            // Read the pointer argument type:
            argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
            GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_FLOAT_PARAMETER)
            {
                // Skip the number of items in the vector:
                (void) va_arg(pCurrentArgument , int);

                // Get a pointer to the "C" vector that contains the values:
                void* pCVector = va_arg(pCurrentArgument , void*);

                // Get the float values:
                GLfloat* pFloatVals = (GLfloat*)pCVector;
                rAsFloat = pFloatVals[0];
                gAsFloat = pFloatVals[1];
                bAsFloat = pFloatVals[2];

                if (is4Arguments)
                {
                    aAsFloat = pFloatVals[3];
                }

                retVal = true;
            }
        }
    }

    // Free the arguments pointer:
    va_end(pCurrentArgument);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsStateChangeExecutor::applyColorStateChange
// Description: Handle color function state change apply.
//              The function parse the input arguments according to the function type,
//              compare to the current color state variable value, and set the new color
//              state variable value.
// Arguments: apMonitoredFunctionId calledFunctionId
//            int argumentsAmount
//            va_list& pArgumentList
//            apFunctionRedundancyStatus& redundancyStatus
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        20/8/2008
// ---------------------------------------------------------------------------
bool gsStateChangeExecutor::applyColorStateChange(apMonitoredFunctionId calledFunctionId, int argumentsAmount, va_list& pArgumentList, apFunctionRedundancyStatus& redundancyStatus)
{
    // The variables contain the color arguments as float:
    GLfloat rAsFloat = 0;
    GLfloat gAsFloat = 0;
    GLfloat bAsFloat = 0;
    GLfloat aAsFloat = 1;

    bool retVal = false;

    // Call the appropriate function to parse the arguments, according to the called function id:
    switch (calledFunctionId)
    {
        case ap_glColor3b:
            retVal = getColorFloatRGBAValuesFromByte(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, false, false);
            break;

        case ap_glColor4b:
            retVal = getColorFloatRGBAValuesFromByte(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, false, true);
            break;

        case ap_glColor3bv:
            retVal = getColorFloatRGBAValuesFromByte(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, true, false);
            break;

        case ap_glColor4bv:
            retVal = getColorFloatRGBAValuesFromByte(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, true, true);
            break;

        case ap_glColor3ub:
            retVal = getColorFloatRGBAValuesFromUByte(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, false, false);
            break;

        case ap_glColor4ub:
            retVal = getColorFloatRGBAValuesFromUByte(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, false, true);
            break;

        case ap_glColor3ubv:
            retVal = getColorFloatRGBAValuesFromUByte(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, true, false);
            break;

        case ap_glColor4ubv:
            retVal = getColorFloatRGBAValuesFromUByte(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, true, true);
            break;

        case ap_glColor3f:
            retVal = getColorFloatRGBAValuesFromFloat(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, false, false);
            break;

        case ap_glColor4f:
            retVal = getColorFloatRGBAValuesFromFloat(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, false, true);
            break;

        case ap_glColor3fv:
            retVal = getColorFloatRGBAValuesFromFloat(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, true, false);
            break;

        case ap_glColor4fv:
            retVal = getColorFloatRGBAValuesFromFloat(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, true, true);
            break;

        case ap_glColor3d:
            retVal = getColorFloatRGBAValuesFromDouble(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, false, false);
            break;

        case ap_glColor4d:
            retVal = getColorFloatRGBAValuesFromDouble(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, false, true);
            break;

        case ap_glColor3dv:
            retVal = getColorFloatRGBAValuesFromDouble(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, true, false);
            break;

        case ap_glColor4dv:
            retVal = getColorFloatRGBAValuesFromDouble(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, true, true);
            break;

        case ap_glColor3i:
            retVal = getColorFloatRGBAValuesFromInt(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, false, false);
            break;

        case ap_glColor4i:
            retVal = getColorFloatRGBAValuesFromInt(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, false, true);
            break;

        case ap_glColor3iv:
            retVal = getColorFloatRGBAValuesFromInt(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, true, false);
            break;

        case ap_glColor4iv:
            retVal = getColorFloatRGBAValuesFromUInt(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, true, true);
            break;

        case ap_glColor3ui:
            retVal = getColorFloatRGBAValuesFromUInt(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, false, false);
            break;

        case ap_glColor4ui:
            retVal = getColorFloatRGBAValuesFromUInt(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, false, true);
            break;

        case ap_glColor3uiv:
            retVal = getColorFloatRGBAValuesFromUInt(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, true, false);
            break;

        case ap_glColor4uiv:
            retVal = getColorFloatRGBAValuesFromUInt(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, true, true);
            break;

        case ap_glColor3us:
            retVal = getColorFloatRGBAValuesFromUShort(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, false, false);
            break;

        case ap_glColor4us:
            retVal = getColorFloatRGBAValuesFromUShort(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, false, true);
            break;

        case ap_glColor3usv:
            retVal = getColorFloatRGBAValuesFromUShort(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, true, false);
            break;

        case ap_glColor4usv:
            retVal = getColorFloatRGBAValuesFromUShort(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, true, true);
            break;

        case ap_glColor3s:
            retVal = getColorFloatRGBAValuesFromShort(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, false, false);
            break;

        case ap_glColor4s:
            retVal = getColorFloatRGBAValuesFromShort(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, false, true);
            break;

        case ap_glColor3sv:
            retVal = getColorFloatRGBAValuesFromShort(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, true, false);
            break;

        case ap_glColor4sv:
            retVal = getColorFloatRGBAValuesFromShort(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, true, true);
            break;

            // OpenGL ES is currently supported on windows only:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

        case ap_glColor4x:
            retVal = getColorFloatRGBAValuesFromFixed(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat);
            break;
#endif

        default:
            GT_ASSERT(false);
            retVal = false;
            break;
    }


    GT_IF_WITH_ASSERT(retVal)
    {
        const apParameter* pColorParam = NULL;
        bool rc = _pStateVariableSnapShot->getStateVariableValue(apGL_CURRENT_COLOR, pColorParam);
        GT_IF_WITH_ASSERT(rc)
        {
            GT_IF_WITH_ASSERT(pColorParam->type() == OS_TOBJ_ID_VECTOR_PARAMETER)
            {
                apVectorParameter* pVectorParam = (apVectorParameter*)(pColorParam);
                GT_IF_WITH_ASSERT(pVectorParam != NULL)
                {
                    // Get the 4 float number used to represent the color vector:
                    GT_IF_WITH_ASSERT(pVectorParam->vectorSize() == 4)
                    {
                        // Make sure that the 4 items exist:
                        bool itemsExist = (((*pVectorParam)[0] != NULL) && ((*pVectorParam)[1] != NULL) && ((*pVectorParam)[2] != NULL) && ((*pVectorParam)[3] != NULL));
                        GT_IF_WITH_ASSERT(itemsExist)
                        {
                            // Make sure that the 4 items are float numbers:
                            bool itemsAreFloat = (((*pVectorParam)[0]->type() == OS_TOBJ_ID_GL_FLOAT_PARAMETER) && ((*pVectorParam)[1]->type() == OS_TOBJ_ID_GL_FLOAT_PARAMETER) && ((*pVectorParam)[2]->type() == OS_TOBJ_ID_GL_FLOAT_PARAMETER) && ((*pVectorParam)[3]->type() == OS_TOBJ_ID_GL_FLOAT_PARAMETER));

                            GT_IF_WITH_ASSERT(itemsAreFloat)
                            {
                                // Downcast the parameters to float parameters:
                                apGLfloatParameter* pRAsFloat = (apGLfloatParameter*)(*pVectorParam)[0];
                                apGLfloatParameter* pGAsFloat = (apGLfloatParameter*)(*pVectorParam)[1];
                                apGLfloatParameter* pBAsFloat = (apGLfloatParameter*)(*pVectorParam)[2];
                                apGLfloatParameter* pAAsFloat = (apGLfloatParameter*)(*pVectorParam)[3];
                                GT_IF_WITH_ASSERT((pRAsFloat != NULL) && (pGAsFloat != NULL) && (pBAsFloat != NULL) && (pAAsFloat != NULL))
                                {
                                    // Compare the values:
                                    static float comparisonEpsilon = 0.0000001f;
                                    static float negativeComparisonEpsilon = -0.0000001f;
                                    float valuesDiff1 = pRAsFloat->value() - rAsFloat;
                                    float valuesDiff2 = pGAsFloat->value() - gAsFloat;
                                    float valuesDiff3 = pBAsFloat->value() - bAsFloat;
                                    float valuesDiff4 = pAAsFloat->value() - aAsFloat;

                                    // Check if the values are equal:
                                    bool equalValues = true;

                                    if ((valuesDiff1 > comparisonEpsilon) || (valuesDiff1 < negativeComparisonEpsilon))
                                    {
                                        equalValues = false;
                                    }

                                    if ((valuesDiff2 > comparisonEpsilon) || (valuesDiff2 < negativeComparisonEpsilon))
                                    {
                                        equalValues = false;
                                    }

                                    if ((valuesDiff3 > comparisonEpsilon) || (valuesDiff3 < negativeComparisonEpsilon))
                                    {
                                        equalValues = false;
                                    }

                                    if ((valuesDiff4 > comparisonEpsilon) || (valuesDiff4 < negativeComparisonEpsilon))
                                    {
                                        equalValues = false;
                                    }

                                    // Set the values to the new ones:
                                    pRAsFloat->readValueFromPointer(&rAsFloat);
                                    pGAsFloat->readValueFromPointer(&gAsFloat);
                                    pBAsFloat->readValueFromPointer(&bAsFloat);
                                    pAAsFloat->readValueFromPointer(&aAsFloat);

                                    // Set the redundancy status:
                                    if (equalValues)
                                    {
                                        redundancyStatus = AP_REDUNDANCY_REDUNDANT;
                                    }
                                    else
                                    {
                                        redundancyStatus = AP_REDUNDANCY_NOT_REDUNDANT;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsStateChangeExecutor::getFloatValueFromDoubleArg
// Description: The function receives as parameter argument expected to
//              be double argument, and translate it to float number represent index value
// Arguments:
//            int argumentsAmount
//            va_list& pArgumentList
//            GLfloat& argAsFloat
//            bool isPointerArgument - states whether the input argument is received as pointer
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        21/8/2008
// ---------------------------------------------------------------------------
bool gsStateChangeExecutor::getFloatValueFromDoubleArg(int argumentsAmount, va_list& pArgumentList, GLfloat& argumentAsFloat, bool isPointer)
{
    (void)(argumentsAmount); // unused
    // Iterate on the argument list:
    va_list pCurrentArgument;
    va_copy(pCurrentArgument, pArgumentList);

    bool retVal = false;

    if (!isPointer)
    {
        // Get the first argument type:
        osTransferableObjectType argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
        GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_DOUBLE_PARAMETER)
        {
            double argumentValue = va_arg(pCurrentArgument , double);
            argumentAsFloat = (GLfloat)argumentValue;
            retVal = true;
        }
    }
    else
    {
        // Read the argument type:
        osTransferableObjectType argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
        GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_VECTOR_PARAMETER)
        {
            argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
            GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_DOUBLE_PARAMETER)
            {
                // Get the number of items in the vector:
                int numOfItems = va_arg(pCurrentArgument , int);

                GT_IF_WITH_ASSERT(numOfItems == 1)
                {
                    // Get a pointer to the "C" vector that contains the values:
                    void* pCVector = va_arg(pCurrentArgument , void*);

                    // Get the float values:
                    GLdouble* pDoubleVals = (GLdouble*)pCVector;
                    argumentAsFloat = (GLfloat) pDoubleVals[0];
                    retVal = true;
                }

            }
        }
    }

    // Free the arguments pointer:
    va_end(pCurrentArgument);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsStateChangeExecutor::getFloatValueFromIntArg
// Description: The function receives as parameter OpenGL function arguments expected to
//              be int argument, and translate it to float number represent index value
// Arguments: apMonitoredFunctionId calledFunctionId
//            int argumentsAmount
//            va_list& pArgumentList
//            GLfloat& argAsFloat
//            bool isPointerArgument - states whether the input argument is received as pointer
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        21/8/2008
// ---------------------------------------------------------------------------
bool gsStateChangeExecutor::getFloatValueFromIntArg(int argumentsAmount, va_list& pArgumentList, GLfloat& argAsFloat, bool isPointerArgument)
{
    (void)(argumentsAmount); // unused
    // Iterate on the argument list:
    va_list pCurrentArgument;
    va_copy(pCurrentArgument, pArgumentList);

    bool retVal = false;

    if (!isPointerArgument)
    {
        // Get the first argument type:
        osTransferableObjectType argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
        GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_INT_PARAMETER)
        {
            long argumentValue = va_arg(pCurrentArgument , long);
            argAsFloat = (GLfloat)argumentValue;
        }
    }

    else
    {

        // Read the argument type:
        osTransferableObjectType argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
        GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_VECTOR_PARAMETER)
        {
            argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
            GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_INT_PARAMETER)
            {
                // Get the number of items in the vector:
                int numOfItems = va_arg(pCurrentArgument , int);

                GT_IF_WITH_ASSERT(numOfItems == 1)
                {
                    // Get a pointer to the "C" vector that contains the values:
                    void* pCVector = va_arg(pCurrentArgument , void*);

                    // Get the float values:
                    GLint* pDoubleVals = (GLint*)pCVector;
                    argAsFloat = (GLfloat) pDoubleVals[0];
                    retVal = true;
                }

            }
        }
    }

    // Free the arguments pointer:
    va_end(pCurrentArgument);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsStateChangeExecutor::getFloatValueFromShortArg
// Description: The function receives as parameter glIndex arguments expected to
//              be short argument, and translate it to float number represent index value
// Arguments: apMonitoredFunctionId calledFunctionId
//            int argumentsAmount
//            va_list& pArgumentList
//            GLfloat& indexAsFloat
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        21/8/2008
// ---------------------------------------------------------------------------
bool gsStateChangeExecutor::getFloatValueFromShortArg(int argumentsAmount, va_list& pArgumentList, GLfloat& argAsFloat, bool isPointer)
{
    (void)(argumentsAmount); // unused
    // Iterate on the argument list:
    va_list pCurrentArgument;
    va_copy(pCurrentArgument, pArgumentList);

    bool retVal = false;

    if (!isPointer)
    {
        // Get the first argument type:
        osTransferableObjectType argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
        GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_SHORT_PARAMETER)
        {
            long argumentValue = va_arg(pCurrentArgument , long);
            argAsFloat = (GLfloat)argumentValue;
        }
    }
    else
    {
        // Read the argument type:
        osTransferableObjectType argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
        GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_VECTOR_PARAMETER)
        {
            argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
            GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_SHORT_PARAMETER)
            {
                // Get the number of items in the vector:
                int numOfItems = va_arg(pCurrentArgument , int);

                GT_IF_WITH_ASSERT(numOfItems == 1)
                {
                    // Get a pointer to the "C" vector that contains the values:
                    void* pCVector = va_arg(pCurrentArgument , void*);

                    // Get the float values:
                    GLshort* pDoubleVals = (GLshort*)pCVector;
                    argAsFloat = (GLfloat) pDoubleVals[0];
                    retVal = true;
                }

            }
        }
    }

    // Free the arguments pointer:
    va_end(pCurrentArgument);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsStateChangeExecutor::getFloatValueFromUByteArgs
// Description: Extract float value from unsigned byte argument
// Arguments:
//            int argumentsAmount
//            va_list& pArgumentList
//            GLfloat& argumentAsFloat
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        20/8/2008
// ---------------------------------------------------------------------------
bool gsStateChangeExecutor::getFloatValueFromUByteArgs(int argumentsAmount, va_list& pArgumentList, GLfloat& argumentAsFloat, bool isPointer)
{
    bool retVal = false;

    // Iterate on the argument list:
    va_list pCurrentArgument;
    va_copy(pCurrentArgument, pArgumentList);


    GT_IF_WITH_ASSERT(argumentsAmount == 1)
    {
        if (!isPointer)
        {
            // Read the function parameters:
            osTransferableObjectType argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
            GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_UBYTE_PARAMETER)
            {
                // Skip the first argument:
                (void) va_arg(pCurrentArgument , long);

                // Translate the values to float:
                float charMaxAsFloat = (float)CHAR_MAX;
                argumentAsFloat = (float)argumentAsFloat / charMaxAsFloat;
                retVal = true;
            }
        }
        else
        {
            osTransferableObjectType argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
            GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_VECTOR_PARAMETER)
            {
                // Read the pointer argument type:
                argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
                GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_UBYTE_PARAMETER)
                {
                    // Get the number of items in the vector:
                    int numOfItems = va_arg(pCurrentArgument , int);

                    // Make sure that there is 1 argument:
                    GT_IF_WITH_ASSERT(numOfItems == 1)
                    {
                        // skip a pointer to the "C" vector that contains the values:
                        (void) va_arg(pCurrentArgument , void*);

                        // Translate the values to float:
                        float charMaxAsFloat = (float)CHAR_MAX;
                        argumentAsFloat = (float)argumentAsFloat / charMaxAsFloat;
                        retVal = true;
                    }
                }
            }
        }

    }

    return retVal;

}

// ---------------------------------------------------------------------------
// Name:        gsStateChangeExecutor::getFloatValueFromFloat
// Description: Extract a float argument
// Arguments: apMonitoredFunctionId calledFunctionId
//            int argumentsAmount
//            va_list& pArgumentList
//            GLfloat& argumentAsFloat
//            bool isPointerArgument - states whether the input argument is received as pointer
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        20/8/2008
// ---------------------------------------------------------------------------
bool gsStateChangeExecutor::getFloatValueFromFloatArg(int argumentsAmount, va_list& pArgumentList, GLfloat& argumentAsFloat, bool isPointer)
{
    bool retVal = false;

    // Iterate on the argument list:
    va_list pCurrentArgument;
    va_copy(pCurrentArgument, pArgumentList);

    // Parse the arguments according to the function id:
    if (!isPointer)
    {
        GT_IF_WITH_ASSERT(argumentsAmount == 1)
        {
            // Read the function parameters:
            osTransferableObjectType argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
            GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_FLOAT_PARAMETER)
            {
                // Get the first argument:
                argumentAsFloat = float(va_arg(pCurrentArgument , double));
                retVal = true;
            }
        }
    }
    else
    {

        osTransferableObjectType argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
        GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_VECTOR_PARAMETER)
        {
            // Read the pointer argument type:
            argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
            GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_FLOAT_PARAMETER)
            {
                // skip the number of items in the vector:
                (void) va_arg(pCurrentArgument , int);

                // Get a pointer to the "C" vector that contains the values:
                void* pCVector = va_arg(pCurrentArgument , void*);

                // Get the float values:
                GLfloat* pByteVals = (GLfloat*)pCVector;
                argumentAsFloat = pByteVals[0];
                retVal = true;
            }
        }
    }

    // Free the arguments pointer:
    va_end(pCurrentArgument);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsStateChangeExecutor::applyIndexStateChange
// Description: Handle index function state change apply.
//              The function parse the input arguments according to the function type,
//              compare to the current index state variable value, and set the new color
//              state variable value.
// Arguments: apMonitoredFunctionId calledFunctionId
//            int argumentsAmount
//            va_list& pArgumentList
//            apFunctionRedundancyStatus& redundancyStatus
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        20/8/2008
// ---------------------------------------------------------------------------
bool gsStateChangeExecutor::applyIndexStateChange(apMonitoredFunctionId calledFunctionId, int argumentsAmount, va_list& pArgumentList, apFunctionRedundancyStatus& redundancyStatus)
{
    bool retVal = false;

    // The index argument as float:
    GLfloat indexAsFloat = 0;

    // Parse the arguments according to the function id:

    // Unsigned byte functions:
    if (calledFunctionId == ap_glIndexub)
    {
        retVal = getFloatValueFromUByteArgs(argumentsAmount, pArgumentList, indexAsFloat, false);
    }
    else if (calledFunctionId == ap_glIndexubv)
    {
        retVal = getFloatValueFromUByteArgs(argumentsAmount, pArgumentList, indexAsFloat, true);
    }

    // Float functions:
    else if (calledFunctionId == ap_glIndexf)
    {
        retVal = getFloatValueFromFloatArg(argumentsAmount, pArgumentList, indexAsFloat, false);
    }

    // Float from float pointer:
    else if (calledFunctionId == ap_glIndexfv)
    {
        retVal = getFloatValueFromFloatArg(argumentsAmount, pArgumentList, indexAsFloat, true);
    }

    // Double functions:
    else if (calledFunctionId == ap_glIndexd)
    {
        retVal = getFloatValueFromDoubleArg(argumentsAmount, pArgumentList, indexAsFloat, false);
    }
    else if (calledFunctionId == ap_glIndexdv)
    {
        retVal = getFloatValueFromDoubleArg(argumentsAmount, pArgumentList, indexAsFloat, true);
    }

    // Int functions:
    else if (calledFunctionId == ap_glIndexi)
    {
        retVal = getFloatValueFromIntArg(argumentsAmount, pArgumentList, indexAsFloat, false);
    }
    else if (calledFunctionId == ap_glIndexiv)
    {
        retVal = getFloatValueFromIntArg(argumentsAmount, pArgumentList, indexAsFloat, true);
    }

    else if (calledFunctionId == ap_glIndexs)
    {
        retVal = getFloatValueFromShortArg(argumentsAmount, pArgumentList, indexAsFloat, false);
    }
    else if (calledFunctionId == ap_glIndexsv)
    {
        retVal = getFloatValueFromShortArg(argumentsAmount, pArgumentList, indexAsFloat, true);
    }

    GT_IF_WITH_ASSERT(retVal)
    {
        const apParameter* pIndexParam = NULL;
        bool rc = _pStateVariableSnapShot->getStateVariableValue(apGL_CURRENT_INDEX, pIndexParam);
        GT_IF_WITH_ASSERT(rc)
        {
            GT_IF_WITH_ASSERT(pIndexParam->type() == OS_TOBJ_ID_GL_FLOAT_PARAMETER)
            {
                apGLfloatParameter* pIndexAsFloat = (apGLfloatParameter*)(pIndexParam);
                GT_IF_WITH_ASSERT(pIndexAsFloat != NULL)
                {
                    // Compare the values:
                    static float comparisonEpsilon = 0.0000001f;
                    static float negativeComparisonEpsilon = -0.0000001f;
                    float valuesDiff1 = pIndexAsFloat->value() - indexAsFloat;

                    // Check if the values are equal:
                    bool equalValues = true;

                    if ((valuesDiff1 > comparisonEpsilon) || (valuesDiff1 < negativeComparisonEpsilon))
                    {
                        equalValues = false;
                    }

                    // Set the values to the new ones:
                    pIndexAsFloat->readValueFromPointer(&indexAsFloat);

                    // Set the redundancy status:
                    if (equalValues)
                    {
                        redundancyStatus = AP_REDUNDANCY_REDUNDANT;
                    }
                    else
                    {
                        redundancyStatus = AP_REDUNDANCY_NOT_REDUNDANT;
                    }
                }
            }
        }
    }
    return retVal;

}


// ---------------------------------------------------------------------------
// Name:        gsStateChangeExecutor::applySecondaryColorStateChange
// Description: Handle secondary color function state change apply.
//              The function parse the input arguments according to the function type,
//              compare to the current color state variable value, and set the new secondary color
//              state variable value.
// Arguments: apMonitoredFunctionId calledFunctionId
//            int argumentsAmount
//            va_list& pArgumentList
//            apFunctionRedundancyStatus& redundancyStatus
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        20/8/2008
// ---------------------------------------------------------------------------
bool gsStateChangeExecutor::applySecondaryColorStateChange(apMonitoredFunctionId calledFunctionId, int argumentsAmount, va_list& pArgumentList, apFunctionRedundancyStatus& redundancyStatus)
{
    // The variables contain the color arguments as float:
    GLfloat rAsFloat = 0;
    GLfloat gAsFloat = 0;
    GLfloat bAsFloat = 0;
    GLfloat aAsFloat = 1;

    bool retVal = false;

    // Call the appropriate function to parse the arguments, according to the called function id:
    switch (calledFunctionId)
    {
        case ap_glSecondaryColor3b:
            retVal = getColorFloatRGBAValuesFromByte(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, false, false);
            break;

        case ap_glSecondaryColor3bv:
            retVal = getColorFloatRGBAValuesFromByte(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, true, false);
            break;

        case ap_glSecondaryColor3ub:
            retVal = getColorFloatRGBAValuesFromUByte(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, false, false);
            break;

        case ap_glSecondaryColor3ubv:
            retVal = getColorFloatRGBAValuesFromUByte(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, true, false);
            break;

        case ap_glColor4ubv:
            retVal = getColorFloatRGBAValuesFromUByte(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, true, true);
            break;

        case ap_glSecondaryColor3f:
            retVal = getColorFloatRGBAValuesFromFloat(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, false, false);
            break;

        case ap_glSecondaryColor3fv:
            retVal = getColorFloatRGBAValuesFromFloat(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, true, false);
            break;

        case ap_glSecondaryColor3d:
            retVal = getColorFloatRGBAValuesFromDouble(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, false, false);
            break;

        case ap_glSecondaryColor3dv:
            retVal = getColorFloatRGBAValuesFromDouble(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, true, false);
            break;

        case ap_glSecondaryColor3i:
            retVal = getColorFloatRGBAValuesFromInt(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, false, false);
            break;

        case ap_glSecondaryColor3iv:
            retVal = getColorFloatRGBAValuesFromInt(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, true, false);
            break;

        case ap_glSecondaryColor3ui:
            retVal = getColorFloatRGBAValuesFromUInt(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, false, false);
            break;

        case ap_glSecondaryColor3uiv:
            retVal = getColorFloatRGBAValuesFromUInt(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, true, false);
            break;

        case ap_glSecondaryColor3us:
            retVal = getColorFloatRGBAValuesFromUShort(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, false, false);
            break;

        case ap_glSecondaryColor3usv:
            retVal = getColorFloatRGBAValuesFromUShort(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, true, false);
            break;

        case ap_glSecondaryColor3s:
            retVal = getColorFloatRGBAValuesFromShort(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, false, false);
            break;

        case ap_glSecondaryColor3sv:
            retVal = getColorFloatRGBAValuesFromShort(argumentsAmount, pArgumentList, rAsFloat, gAsFloat, bAsFloat, aAsFloat, true, false);
            break;

        default:
            retVal = false;
            GT_ASSERT_EX(false, L"Unsupported type of function in apply state change process");
            break;

    }


    GT_IF_WITH_ASSERT(retVal)
    {
        const apParameter* pColorParam = NULL;
        bool rc = _pStateVariableSnapShot->getStateVariableValue(apGL_CURRENT_SECONDARY_COLOR, pColorParam);
        GT_IF_WITH_ASSERT(rc)
        {
            GT_IF_WITH_ASSERT(pColorParam->type() == OS_TOBJ_ID_VECTOR_PARAMETER)
            {
                apVectorParameter* pVectorParam = (apVectorParameter*)(pColorParam);
                GT_IF_WITH_ASSERT(pVectorParam != NULL)
                {
                    // Get the 4 float number used to represent the color vector:
                    GT_IF_WITH_ASSERT(pVectorParam->vectorSize() == 4)
                    {
                        // Make sure that the 4 items exist:
                        bool itemsExist = (((*pVectorParam)[0] != NULL) && ((*pVectorParam)[1] != NULL) && ((*pVectorParam)[2] != NULL) && ((*pVectorParam)[3] != NULL));
                        GT_IF_WITH_ASSERT(itemsExist)
                        {
                            // Make sure that the 4 items are float numbers:
                            bool itemsAreFloat = (((*pVectorParam)[0]->type() == OS_TOBJ_ID_GL_FLOAT_PARAMETER) && ((*pVectorParam)[1]->type() == OS_TOBJ_ID_GL_FLOAT_PARAMETER) && ((*pVectorParam)[2]->type() == OS_TOBJ_ID_GL_FLOAT_PARAMETER) && ((*pVectorParam)[3]->type() == OS_TOBJ_ID_GL_FLOAT_PARAMETER));

                            GT_IF_WITH_ASSERT(itemsAreFloat)
                            {
                                // Downcast the parameters to float parameters:
                                apGLfloatParameter* pRAsFloat = (apGLfloatParameter*)(*pVectorParam)[0];
                                apGLfloatParameter* pGAsFloat = (apGLfloatParameter*)(*pVectorParam)[1];
                                apGLfloatParameter* pBAsFloat = (apGLfloatParameter*)(*pVectorParam)[2];
                                apGLfloatParameter* pAAsFloat = (apGLfloatParameter*)(*pVectorParam)[3];
                                GT_IF_WITH_ASSERT((pRAsFloat != NULL) && (pGAsFloat != NULL) && (pBAsFloat != NULL) && (pAAsFloat != NULL))
                                {
                                    // Compare the values:
                                    static float comparisonEpsilon = 0.0000001f;
                                    static float negativeComparisonEpsilon = -0.0000001f;
                                    float valuesDiff1 = pRAsFloat->value() - rAsFloat;
                                    float valuesDiff2 = pGAsFloat->value() - gAsFloat;
                                    float valuesDiff3 = pBAsFloat->value() - bAsFloat;
                                    float valuesDiff4 = pAAsFloat->value() - aAsFloat;

                                    // Check if the values are equal:
                                    bool equalValues = true;

                                    if ((valuesDiff1 > comparisonEpsilon) || (valuesDiff1 < negativeComparisonEpsilon))
                                    {
                                        equalValues = false;
                                    }

                                    if ((valuesDiff2 > comparisonEpsilon) || (valuesDiff2 < negativeComparisonEpsilon))
                                    {
                                        equalValues = false;
                                    }

                                    if ((valuesDiff3 > comparisonEpsilon) || (valuesDiff3 < negativeComparisonEpsilon))
                                    {
                                        equalValues = false;
                                    }

                                    if ((valuesDiff4 > comparisonEpsilon) || (valuesDiff4 < negativeComparisonEpsilon))
                                    {
                                        equalValues = false;
                                    }

                                    // Set the values to the new ones:
                                    pRAsFloat->readValueFromPointer(&rAsFloat);
                                    pGAsFloat->readValueFromPointer(&gAsFloat);
                                    pBAsFloat->readValueFromPointer(&bAsFloat);
                                    pAAsFloat->readValueFromPointer(&aAsFloat);

                                    // Set the redundancy status:
                                    if (equalValues)
                                    {
                                        redundancyStatus = AP_REDUNDANCY_REDUNDANT;
                                    }
                                    else
                                    {
                                        redundancyStatus = AP_REDUNDANCY_NOT_REDUNDANT;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return retVal;

}


// ---------------------------------------------------------------------------
// Name:        gsStateChangeExecutor::applyShininessMaterialStateChange
// Description: Handle material function with one argument - shininess
// Arguments: apMonitoredFunctionId calledFunctionId
//            int argumentsAmount
//            va_list& pArgumentList
//            apFunctionRedundancyStatus& redundancyStatus
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/8/2008
// ---------------------------------------------------------------------------
bool gsStateChangeExecutor::applyShininessMaterialStateChange(apMonitoredFunctionId calledFunctionId, int argumentsAmount, va_list& pArgumentList, apFunctionRedundancyStatus& redundancyStatus)
{
    bool retVal = false;

    // Iterate on the argument list:
    va_list pCurrentArgument;
    va_copy(pCurrentArgument, pArgumentList);

    float shininessAsFloat = 0;
    GLenum face = GL_BACK;
    // Make sure there is 3 arguments:
    GT_IF_WITH_ASSERT(argumentsAmount == 3)
    {
        // Read the function parameters:
        osTransferableObjectType argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
        GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_ENUM_PARAMETER)
        {
            // Get the first argument - face:
            unsigned long argumentValue = va_arg(pCurrentArgument , unsigned long);
            face = (GLenum)argumentValue;
            GT_IF_WITH_ASSERT((face == GL_BACK) || (face == GL_FRONT))
            {
                // Get the second argument:
                argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
                GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_ENUM_PARAMETER)
                {
                    argumentValue = va_arg(pCurrentArgument , unsigned long);
                    GLenum pname = (GLenum)argumentValue;
                    GT_IF_WITH_ASSERT(pname == GL_SHININESS)
                    {
                        if (calledFunctionId == ap_glMaterialf)
                        {
                            // Get the shininess parameter as float value:
                            // Get the second argument:
                            argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
                            GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_FLOAT_PARAMETER)
                            {
                                double argumentValue2 = va_arg(pCurrentArgument , double);
                                shininessAsFloat = (float)argumentValue2;
                            }
                        }
                        else if (calledFunctionId == ap_glMateriali)
                        {
                            // Get the shininess parameter int float value:
                            // Get the second argument:
                            argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
                            GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_INT_PARAMETER)
                            {
                                long argumentValue3 = va_arg(pCurrentArgument , long);
                                shininessAsFloat = (float)argumentValue3;
                            }
                        }

                    }
                }
            }
        }
    }
    GT_IF_WITH_ASSERT(retVal)
    {
        // Get the shininess state variable and compare with function parameter:
        const apParameter* pShininsessParam = NULL;
        apOpenGLStateVariableId stateVariableId = apGL_SHININESS_back;

        if (face == GL_FRONT)
        {
            stateVariableId = apGL_SHININESS_front;
        }

        bool rc = _pStateVariableSnapShot->getStateVariableValue(stateVariableId, pShininsessParam);
        GT_IF_WITH_ASSERT(rc)
        {
            GT_IF_WITH_ASSERT(pShininsessParam->type() == OS_TOBJ_ID_GL_FLOAT_PARAMETER)
            {
                apGLfloatParameter* pShininessAsFloat = (apGLfloatParameter*)(pShininsessParam);
                GT_IF_WITH_ASSERT(pShininessAsFloat != NULL)
                {
                    // Compare the values:
                    static float comparisonEpsilon = 0.0000001f;
                    static float negativeComparisonEpsilon = -0.0000001f;
                    float valuesDiff1 = pShininessAsFloat->value() - shininessAsFloat;

                    // Check if the values are equal:
                    bool equalValues = true;

                    if ((valuesDiff1 > comparisonEpsilon) || (valuesDiff1 < negativeComparisonEpsilon))
                    {
                        equalValues = false;
                    }

                    // Set the values to the new ones:
                    pShininessAsFloat->readValueFromPointer(&shininessAsFloat);

                    // Set the redundancy status:
                    if (equalValues)
                    {
                        redundancyStatus = AP_REDUNDANCY_REDUNDANT;
                    }
                    else
                    {
                        redundancyStatus = AP_REDUNDANCY_NOT_REDUNDANT;
                    }
                }
            }
        }
    }

    // Release the arguments pointer;
    va_end(pCurrentArgument);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsStateChangeExecutor::getMaterialFaceAndPname
// Description: Extract a pname, and face parameter from material function arguments.
// Arguments: int argumentsAmount - the argument amount
//            va_list& pCurrentArgument - the list is copies before this function and released after it.
//            apFunctionRedundancyStatus& redundancyStatus
//            GLenum face - the face argument of the function
//            GLenum pname - the pname argument of the function
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/8/2008
// ---------------------------------------------------------------------------
bool gsStateChangeExecutor::getMaterialFaceAndPname(int argumentsAmount , va_list& pCurrentArgument, GLenum& face, GLenum& pname)
{
    bool retVal = false;

    // Initialize face and pname:
    face = GL_BACK;
    pname = GL_DIFFUSE;
    // Make sure there is 3 arguments:
    GT_IF_WITH_ASSERT(argumentsAmount == 3)
    {
        // Read the function parameters:
        osTransferableObjectType argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
        GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_ENUM_PARAMETER)
        {
            // Get the first argument - face:
            unsigned long argumentValue = va_arg(pCurrentArgument , unsigned long);
            face = (GLenum)argumentValue;
            GT_IF_WITH_ASSERT((face == GL_BACK) || (face == GL_FRONT) || (face == GL_FRONT_AND_BACK))
            {
                // Get the second argument:
                argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
                GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_GL_ENUM_PARAMETER)
                {
                    argumentValue = va_arg(pCurrentArgument , unsigned long);
                    pname = (GLenum)argumentValue;
                    retVal = true;
                }
            }
        }
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsStateChangeExecutor::getMaterial4Values
// Description:
// Arguments: va_list& pCurrentArgument - points the values argument pointer of glMatrial function
//            float& value1
//            float& value2
//            float& value3
//            float& value4
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/8/2008
// ---------------------------------------------------------------------------
bool gsStateChangeExecutor::getMaterial4Values(va_list& pCurrentArgument, float& value1, float& value2, float& value3, float& value4)
{
    bool retVal = false;

    osTransferableObjectType argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
    GT_IF_WITH_ASSERT(argumentType == OS_TOBJ_ID_VECTOR_PARAMETER)
    {
        // Read the pointer argument type:
        argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));

        if (argumentType == OS_TOBJ_ID_GL_FLOAT_PARAMETER)
        {
            // Get the number of items in the vector:
            int numOfItems = va_arg(pCurrentArgument , int);

            GT_IF_WITH_ASSERT(numOfItems >= 3)
            {
                // Get a pointer to the "C" vector that contains the values:
                void* pCVector = va_arg(pCurrentArgument , void*);

                // Get the float values:
                GLfloat* pFloatVals = (GLfloat*)pCVector;
                value1 = pFloatVals[0];
                value2 = pFloatVals[1];
                value3 = pFloatVals[2];

                if (numOfItems == 4)
                {
                    value4 = pFloatVals[3];
                }
            }
        }
        else if (argumentType == OS_TOBJ_ID_GL_INT_PARAMETER)
        {
            // Get the number of items in the vector:
            int numOfItems = va_arg(pCurrentArgument , int);

            GT_IF_WITH_ASSERT(numOfItems == 4)
            {
                // Get a pointer to the "C" vector that contains the values:
                void* pCVector = va_arg(pCurrentArgument , void*);

                // Get the float values:
                GLint* pIntVals = (GLint*)pCVector;
                value1 = (float)pIntVals[0];
                value2 = (float)pIntVals[1];
                value3 = (float)pIntVals[2];
                value4 = (float)pIntVals[3];
            }
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsStateChangeExecutor::applyMaterialStateChange
// Description: Handle material function state change apply.
//              The function parse the input arguments according to the function type,
//              compare to the current material state variable value, and set the new material
//              state variable value.
// Arguments: apMonitoredFunctionId calledFunctionId
//            int argumentsAmount
//            va_list& pArgumentList
//            apFunctionRedundancyStatus& redundancyStatus
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        20/8/2008
// ---------------------------------------------------------------------------
bool gsStateChangeExecutor::applyMaterialStateChange(apMonitoredFunctionId calledFunctionId, int argumentsAmount, va_list& pArgumentList, apFunctionRedundancyStatus& redundancyStatus)
{
    bool retVal = false;

    if ((calledFunctionId == ap_glMaterialf) || (calledFunctionId == ap_glMateriali))
    {

        retVal = applyShininessMaterialStateChange(calledFunctionId, argumentsAmount, pArgumentList, redundancyStatus);

        if (!retVal)
        {
            // If shininess arguments didn't fit the function format, it means that the user called the function with illegal arguments,
            // therefore the function call is redundant:
            redundancyStatus = AP_REDUNDANCY_REDUNDANT;
            retVal = true;
        }
    }
    else
    {
        GLenum face = GL_BACK;
        GLenum pname = GL_DIFFUSE;

        // Create list for the arguments iteration;
        va_list pCurrentArgument;
        va_copy(pCurrentArgument, pArgumentList);

        retVal = getMaterialFaceAndPname(argumentsAmount, pCurrentArgument, face, pname);
        GT_IF_WITH_ASSERT(retVal)
        {
            float materialValueAsFloat1 = 0;
            float materialValueAsFloat2 = 0;
            float materialValueAsFloat3 = 0;
            float materialValueAsFloat4 = 0;

            if (calledFunctionId == ap_glMaterialiv)
            {
                retVal = getMaterial4Values(pCurrentArgument, materialValueAsFloat1, materialValueAsFloat2, materialValueAsFloat3, materialValueAsFloat4);
            }
            else if (calledFunctionId == ap_glMaterialfv)
            {
                retVal = getMaterial4Values(pCurrentArgument, materialValueAsFloat1, materialValueAsFloat2, materialValueAsFloat3, materialValueAsFloat4);
            }

            // Get list of state variables to compare and apply:
            gtVector<apOpenGLStateVariableId> stateVariableIds;
            retVal = getListOfMaterialStateVariableToCompareTo(pname, face, stateVariableIds);

            bool equalValues = true;

            // Iterate through the ids vector, and compare for each of them:
            for (size_t i = 0; i < stateVariableIds.size(); i++)
            {
                // Get the current state variable id:
                apOpenGLStateVariableId stateVariableId = stateVariableIds[i];

                // Get the parameter:
                const apParameter* pParam = NULL;
                bool rc = _pStateVariableSnapShot->getStateVariableValue(stateVariableId, pParam);
                GT_IF_WITH_ASSERT(rc)
                {
                    GT_IF_WITH_ASSERT(pParam->type() == OS_TOBJ_ID_VECTOR_PARAMETER)
                    {
                        apVectorParameter* pVectorParam = (apVectorParameter*)(pParam);
                        GT_IF_WITH_ASSERT(pVectorParam != NULL)
                        {
                            // Get the 4 float number used to represent the color vector:
                            GT_IF_WITH_ASSERT(pVectorParam->vectorSize() == 4)
                            {
                                // Make sure that the 4 items exist:
                                bool itemsExist = (((*pVectorParam)[0] != NULL) && ((*pVectorParam)[1] != NULL) && ((*pVectorParam)[2] != NULL) && ((*pVectorParam)[3] != NULL));
                                GT_IF_WITH_ASSERT(itemsExist)
                                {
                                    // Make sure that the 4 items are float numbers:
                                    bool itemsAreFloat = (((*pVectorParam)[0]->type() == OS_TOBJ_ID_GL_FLOAT_PARAMETER) && ((*pVectorParam)[1]->type() == OS_TOBJ_ID_GL_FLOAT_PARAMETER) && ((*pVectorParam)[2]->type() == OS_TOBJ_ID_GL_FLOAT_PARAMETER) && ((*pVectorParam)[3]->type() == OS_TOBJ_ID_GL_FLOAT_PARAMETER));

                                    GT_IF_WITH_ASSERT(itemsAreFloat)
                                    {
                                        // Downcast the parameters to float parameters:
                                        apGLfloatParameter* pValue1AsFloat = (apGLfloatParameter*)(*pVectorParam)[0];
                                        apGLfloatParameter* pValue2AsFloat = (apGLfloatParameter*)(*pVectorParam)[1];
                                        apGLfloatParameter* pValue3AsFloat = (apGLfloatParameter*)(*pVectorParam)[2];
                                        apGLfloatParameter* pValue4AsFloat = (apGLfloatParameter*)(*pVectorParam)[3];
                                        GT_IF_WITH_ASSERT((pValue1AsFloat != NULL) && (pValue2AsFloat != NULL) && (pValue3AsFloat != NULL) && (pValue4AsFloat != NULL))
                                        {
                                            // Compare the values:
                                            static float comparisonEpsilon = 0.0000001f;
                                            static float negativeComparisonEpsilon = -0.0000001f;
                                            float valuesDiff1 = pValue1AsFloat->value() - materialValueAsFloat1;
                                            float valuesDiff2 = pValue2AsFloat->value() - materialValueAsFloat2;
                                            float valuesDiff3 = pValue3AsFloat->value() - materialValueAsFloat3;
                                            float valuesDiff4 = pValue4AsFloat->value() - materialValueAsFloat4;

                                            // Compare the values with epsilon:
                                            if ((valuesDiff1 > comparisonEpsilon) || (valuesDiff1 < negativeComparisonEpsilon))
                                            {
                                                equalValues = false;
                                            }

                                            if ((valuesDiff2 > comparisonEpsilon) || (valuesDiff2 < negativeComparisonEpsilon))
                                            {
                                                equalValues = false;
                                            }

                                            if ((valuesDiff3 > comparisonEpsilon) || (valuesDiff3 < negativeComparisonEpsilon))
                                            {
                                                equalValues = false;
                                            }

                                            if ((valuesDiff4 > comparisonEpsilon) || (valuesDiff4 < negativeComparisonEpsilon))
                                            {
                                                equalValues = false;
                                            }

                                            // Set the values to the new ones:
                                            pValue1AsFloat->readValueFromPointer(&materialValueAsFloat1);
                                            pValue2AsFloat->readValueFromPointer(&materialValueAsFloat2);
                                            pValue3AsFloat->readValueFromPointer(&materialValueAsFloat3);
                                            pValue4AsFloat->readValueFromPointer(&materialValueAsFloat4);
                                        }
                                    }
                                }
                            }
                        }
                    }

                }
            }

            // Set the redundancy status:
            if (equalValues)
            {
                redundancyStatus = AP_REDUNDANCY_REDUNDANT;
            }
            else
            {
                redundancyStatus = AP_REDUNDANCY_NOT_REDUNDANT;
            }
        }

        // Release the arguments pointer;
        va_end(pCurrentArgument);

    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsStateChangeExecutor::getListOfMaterialStateVariableToCompareTo
// Description: Checks the pname and face parameters of glMaterial function, and
//              fills a vector of state variable ids needed to be compared
// Arguments: GLenum pname
//            GLenum face
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/8/2008
// ---------------------------------------------------------------------------
bool gsStateChangeExecutor::getListOfMaterialStateVariableToCompareTo(GLenum pname, GLenum face, gtVector<apOpenGLStateVariableId>& stateVariableIdsVector)
{
    bool retVal = false;

    // Fill the state variable ids according to pname and face arguments:
    if (pname == GL_DIFFUSE)
    {
        if (face == GL_BACK)
        {
            stateVariableIdsVector.push_back(apGL_DIFFUSE_back);
            retVal = true;
        }
        else if (face == GL_FRONT)
        {
            stateVariableIdsVector.push_back(apGL_DIFFUSE_front);
            retVal = true;
        }
        else if (face == GL_FRONT_AND_BACK)
        {
            stateVariableIdsVector.push_back(apGL_DIFFUSE_front);
            stateVariableIdsVector.push_back(apGL_DIFFUSE_back);
            retVal = true;
        }
    }
    else if (pname == GL_AMBIENT)
    {
        if (face == GL_BACK)
        {
            stateVariableIdsVector.push_back(apGL_AMBIENT_back);
            retVal = true;
        }
        else if (face == GL_FRONT)
        {
            stateVariableIdsVector.push_back(apGL_AMBIENT_front);
            retVal = true;
        }
        else if (face == GL_FRONT_AND_BACK)
        {
            stateVariableIdsVector.push_back(apGL_AMBIENT_back);
            stateVariableIdsVector.push_back(apGL_AMBIENT_front);
            retVal = true;
        }
    }
    else if (pname == GL_SPECULAR)
    {
        if (face == GL_BACK)
        {
            stateVariableIdsVector.push_back(apGL_SPECULAR_back);
            retVal = true;
        }
        else if (face == GL_FRONT)
        {
            stateVariableIdsVector.push_back(apGL_SPECULAR_front);
            retVal = true;
        }
        else if (face == GL_FRONT_AND_BACK)
        {
            stateVariableIdsVector.push_back(apGL_SPECULAR_back);
            stateVariableIdsVector.push_back(apGL_SPECULAR_front);
            retVal = true;
        }
    }
    else if (pname == GL_EMISSION)
    {
        if (face == GL_BACK)
        {
            stateVariableIdsVector.push_back(apGL_EMISSION_back);
            retVal = true;
        }
        else if (face == GL_FRONT)
        {
            stateVariableIdsVector.push_back(apGL_EMISSION_front);
            retVal = true;
        }
        else if (face == GL_FRONT_AND_BACK)
        {
            stateVariableIdsVector.push_back(apGL_EMISSION_back);
            stateVariableIdsVector.push_back(apGL_EMISSION_front);
            retVal = true;
        }
    }
    else if (pname == GL_AMBIENT_AND_DIFFUSE)
    {
        if (face == GL_BACK)
        {
            stateVariableIdsVector.push_back(apGL_AMBIENT_back);
            stateVariableIdsVector.push_back(apGL_DIFFUSE_back);
            retVal = true;
        }
        else if (face == GL_FRONT)
        {
            stateVariableIdsVector.push_back(apGL_AMBIENT_front);
            stateVariableIdsVector.push_back(apGL_DIFFUSE_front);
            retVal = true;
        }
        else if (face == GL_FRONT_AND_BACK)
        {
            stateVariableIdsVector.push_back(apGL_AMBIENT_back);
            stateVariableIdsVector.push_back(apGL_DIFFUSE_back);
            stateVariableIdsVector.push_back(apGL_AMBIENT_front);
            stateVariableIdsVector.push_back(apGL_DIFFUSE_front);
            retVal = true;
        }
    }
    else if (pname == GL_COLOR_INDEXES)
    {
        if (face == GL_BACK)
        {
            stateVariableIdsVector.push_back(apGL_COLOR_INDEXES_back);
            retVal = true;
        }
        else if (face == GL_FRONT)
        {
            stateVariableIdsVector.push_back(apGL_COLOR_INDEXES_front);
            retVal = true;
        }
        else if (face == GL_FRONT_AND_BACK)
        {
            stateVariableIdsVector.push_back(apGL_COLOR_INDEXES_back);
            stateVariableIdsVector.push_back(apGL_COLOR_INDEXES_front);
            retVal = true;
        }
    }

    return retVal;
}

