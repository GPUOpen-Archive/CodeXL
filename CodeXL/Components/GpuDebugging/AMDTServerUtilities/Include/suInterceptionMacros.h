//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suInterceptionMacros.h
///
//==================================================================================

//------------------------------ suInterceptionMacros.h ------------------------------

#ifndef __SUINTERCEPTIONMACROS_H
#define __SUINTERCEPTIONMACROS_H

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Local:
#include <AMDTServerUtilities/Include/suSWMRInstance.h>

// Mac OSX interception utilities:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    #include <AMDTServerUtilities/Include/suMacOSXInterception.h>
    #include <AMDTServerUtilities/Include/suGlobalVariables.h>
#endif


// --------------------------------------------------------
//             Private interception related macros
// --------------------------------------------------------

// Define the instance of the technology main wrapper:
//#if defined (_GR_OPENGL32) || defined (_GR_OPENGL_MODULE) || defined (_GR_OPENGLES_COMMON) || defined (_GR_EGL) || defined (_GR_OPENGLES_IPHONE)
#ifdef _AMDT_OPENGLSERVER_EXPORTS
    #define SU_TECHNOLOGY_MONITOR gs_stat_openGLMonitorInstance
    #define SU_REAL_FUNCTION_POINTERS_STRUCT gs_stat_realFunctionPointers
    #define SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET 0

    // OpenGL error testing - this will check for OpenGL errors after each time a function
    // is called by the spy that wasn't called by the user.
    // Only allow this to happen in the OpenGL spy:
    // DO NOT RELEASE A VERSION WITH THIS UN-COMMENTED
    //#define GS_CHECK_OPENGL_ERRORS_AFTER_EACH_REAL_FUNCTION_CALL
    #ifdef GS_CHECK_OPENGL_ERRORS_AFTER_EACH_REAL_FUNCTION_CALL
        // Specific includes:
        #include <AMDTBaseTools/Include/gtAssert.h>
        #include <AMDTBaseTools/Include/gtString.h>
        #include <AMDTAPIClasses/Include/apGLenumParameter.h>
        #include <AMDTAPIClasses/Include/apMonitoredFunctionsManager.h>
        #include <src/gsGlobalVariables.h>
        #include <src/gsMonitoredFunctionPointers.h>

        // Only allow this to enabled in Debug builds:
        #if AMDT_BUILD_CONFIGURATION == AMDT_RELEASE_BUILD
            #error Disable GS_CHECK_OPENGL_ERRORS_AFTER_EACH_REAL_FUNCTION_CALL before building release
        #endif // AMDT_BUILD_CONFIGURATION == AMDT_RELEASE_BUILD
    #endif // GS_CHECK_OPENGL_ERRORS_AFTER_EACH_REAL_FUNCTION_CALL
    //#elif defined _GR_OPENCLSPY_EXPORTS
#elif defined _AMDT_OPENCLSERVER_EXPORTS
    #define SU_TECHNOLOGY_MONITOR cs_stat_openCLMonitorInstance
    #define SU_REAL_FUNCTION_POINTERS_STRUCT cs_stat_realFunctionPointers
    #define SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET apFirstOpenCLFunctionIndex
#else
    #error Error unknown module uses suInterceptionMacros.h !!
#endif



// --------------------------------------------------------
//             Public interception related macros
// --------------------------------------------------------


#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
//////////////////////////////////////////////////////////////////////////
// These are the Mac Macros, see implementation notes in gsMacOSXInterception.cpp
//////////////////////////////////////////////////////////////////////////
#ifdef _GR_IPHONE_DEVICE_BUILD
#if SU_LENGTH_OF_JMP_COMMAND_IN_BYTES != 8
    #error function wrapper macros do not match interception code!
#endif

// Uri, 18/10/09: In iPhone OS 2.0 and higher, a memory page cannot be writable and
// executable at the same time. As a result, to change the code in runtime, we need
// to change the access rights before and after each change, as calling a
// non-executable memory location causes an access violation.
#include <mach/vm_map.h>

// ---------------------------------------------------------------------------
// Name:        SU_START_FUNCTION_WRAPPER
// Description: Starts a function wrappers.
// Arguments:   funcId - The wrapped function id.
// Author:      Uri Shomroni
// Date:        14/10/2009
// ---------------------------------------------------------------------------
#ifdef SU_USE_SINGLE_WRITE_MULTIPLE_READ_SYNC
#define SU_START_FUNCTION_WRAPPER(funcId) \
        suSWMRInstance::SharedLock();
    su_stat_functionInterceptionInfo[funcId]._isCurrentlyInsideWrapper = true;
#else // !defined(SU_USE_SINGLE_WRITE_MULTIPLE_READ_SYNC)
#define SU_START_FUNCTION_WRAPPER(funcId) \
    su_stat_functionInterceptionInfo[funcId]._isCurrentlyInsideWrapper = true;
#endif

// ---------------------------------------------------------------------------
// Name:        SU_START_DRAW_FUNCTION_WRAPPER
// Description: Starts a "draw" function wrappers.
//              A draw function is a function that has a visual effect on the
//              rendered buffer.
// Arguments:   funcId - The wrapped function id.
// Author:      Uri Shomroni
// Date:        14/10/2009
// ---------------------------------------------------------------------------
#define SU_START_DRAW_FUNCTION_WRAPPER(funcId) if (gs_stat_isInNullOpenGLImplementationMode) { return; };\
    su_stat_functionInterceptionInfo[funcId]._isCurrentlyInsideWrapper = true;

// ---------------------------------------------------------------------------
// Name:        SU_END_FUNCTION_WRAPPER
// Description: Ends a function wrapper.
// Arguments:   funcId - The wrapped function id.
// Author:      Uri Shomroni
// Date:        14/10/2009
// ---------------------------------------------------------------------------
#define SU_END_FUNCTION_WRAPPER(funcId) if (!su_stat_interoperabilityHelper.isInNestedFunction()) {SU_TECHNOLOGY_MONITOR.afterMonitoredFunctionExecutionActions(funcId);};\
    su_stat_functionInterceptionInfo[funcId]._isCurrentlyInsideWrapper = false;

// ---------------------------------------------------------------------------
// Name:        SU_BEFORE_EXECUTING_REAL_FUNCTION
// Description: Should be used before calling a real function inside the spy
// Arguments: funcId = ap_glXXXX (before glXXXX is used).
// Author:      Uri Shomroni
// Date:        14/10/2009
// ---------------------------------------------------------------------------
#define SU_BEFORE_EXECUTING_REAL_FUNCTION(funcId)

// ---------------------------------------------------------------------------
// Name:        SU_AFTER_EXECUTING_REAL_FUNCTION
// Description: Should be used after calling a real function inside the spy
// Arguments: funcId = ap_glXXXX (after glXXXX is used).
// Author:      Uri Shomroni
// Date:        14/10/2009
// ---------------------------------------------------------------------------
#define SU_AFTER_EXECUTING_REAL_FUNCTION_ACTIONS(funcId)

#else // ifndef _GR_IPHONE_DEVICE_BUILD
#if SU_LENGTH_OF_JMP_COMMAND_IN_BYTES != 5
    #error function wrapper macros do not match interception code!
#endif
// ---------------------------------------------------------------------------
// Name:        SU_START_FUNCTION_WRAPPER
// Description: Starts a function wrappers.
// Arguments:   funcId - The wrapped function id.
// Author:      Uri Shomroni
// Date:        22/1/2009
// ---------------------------------------------------------------------------
#define SU_START_FUNCTION_WRAPPER(funcId) \
    su_stat_functionInterceptionInfo[funcId]._isCurrentlyInsideWrapper = true;\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[0] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[0];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[1] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[1];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[2] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[2];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[3] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[3];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[4] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[4];

// ---------------------------------------------------------------------------
// Name:        SU_START_DRAW_FUNCTION_WRAPPER
// Description: Starts a "draw" function wrappers.
//              A draw function is a function that has a visual effect on the
//              rendered buffer.
// Arguments:   funcId - The wrapped function id.
// Author:      Uri Shomroni
// Date:        2/1/2009
// ---------------------------------------------------------------------------
#define SU_START_DRAW_FUNCTION_WRAPPER(funcId) if (gs_stat_isInNullOpenGLImplementationMode) { return; };\
    su_stat_functionInterceptionInfo[funcId]._isCurrentlyInsideWrapper = true;\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[0] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[0];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[1] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[1];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[2] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[2];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[3] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[3];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[4] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[4];

// ---------------------------------------------------------------------------
// Name:        SU_END_FUNCTION_WRAPPER
// Description: Ends a function wrapper.
// Arguments:   funcId - The wrapped function id.
// Author:      Uri Shomroni
// Date:        2/1/2009
// ---------------------------------------------------------------------------
#define SU_END_FUNCTION_WRAPPER(funcId) if (!su_stat_interoperabilityHelper.isInNestedFunction()) {SU_TECHNOLOGY_MONITOR.afterMonitoredFunctionExecutionActions(funcId);};\
    su_stat_functionInterceptionInfo[funcId]._isCurrentlyInsideWrapper = false;\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[0] = su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[0];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[1] = su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[1];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[2] = su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[2];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[3] = su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[3];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[4] = su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[4];

// ---------------------------------------------------------------------------
// Name:        SU_BEFORE_EXECUTING_REAL_FUNCTION
// Description: Should be used before calling a real function inside the spy
// Arguments: funcId = ap_glXXXX (before glXXXX is used).
// Author:      Uri Shomroni
// Date:        25/1/2009
// ---------------------------------------------------------------------------
#define SU_BEFORE_EXECUTING_REAL_FUNCTION(funcId)\
    if (!su_stat_functionInterceptionInfo[funcId]._isCurrentlyInsideWrapper)\
    {\
        ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[0] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[0];\
        ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[1] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[1];\
        ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[2] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[2];\
        ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[3] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[3];\
        ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[4] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[4];\
    }

// ---------------------------------------------------------------------------
// Name:        SU_AFTER_EXECUTING_REAL_FUNCTION
// Description: Should be used after calling a real function inside the spy
// Arguments: funcId = ap_glXXXX (after glXXXX is used).
// Author:      Uri Shomroni
// Date:        25/1/2009
// ---------------------------------------------------------------------------
#define SU_AFTER_EXECUTING_REAL_FUNCTION_ACTIONS(funcId)\
    if (!su_stat_functionInterceptionInfo[funcId]._isCurrentlyInsideWrapper)\
    {\
        ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[0] = su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[0];\
        ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[1] = su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[1];\
        ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[2] = su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[2];\
        ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[3] = su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[3];\
        ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[4] = su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[4];\
    }
#endif // _GR_IPHONE_DEVICE_BUILD

#elif AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE

#if SU_LENGTH_OF_JMP_COMMAND_IN_BYTES != 13
    #error function wrapper macros do not match interception code!
#endif
// ---------------------------------------------------------------------------
// Name:        SU_START_FUNCTION_WRAPPER
// Description: Starts a function wrappers.
// Arguments:   funcId - The wrapped function id.
// Author:      Uri Shomroni
// Date:        6/9/2009
// ---------------------------------------------------------------------------
#define SU_START_FUNCTION_WRAPPER(funcId) \
    su_stat_functionInterceptionInfo[funcId]._isCurrentlyInsideWrapper = true;\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[0] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[0];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[1] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[1];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[2] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[2];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[3] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[3];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[4] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[4];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[5] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[5];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[6] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[6];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[7] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[7];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[8] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[8];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[9] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[9];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[10] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[10];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[11] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[11];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[12] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[12];

// ---------------------------------------------------------------------------
// Name:        SU_START_DRAW_FUNCTION_WRAPPER
// Description: Starts a "draw" function wrappers.
//              A draw function is a function that has a visual effect on the
//              rendered buffer.
// Arguments:   funcId - The wrapped function id.
// Author:      Uri Shomroni
// Date:        6/9/2009
// ---------------------------------------------------------------------------
#define SU_START_DRAW_FUNCTION_WRAPPER(funcId) if (gs_stat_isInNullOpenGLImplementationMode) { return; };\
    su_stat_functionInterceptionInfo[funcId]._isCurrentlyInsideWrapper = true;\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[0] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[0];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[1] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[1];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[2] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[2];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[3] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[3];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[4] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[4];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[5] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[5];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[6] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[6];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[7] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[7];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[8] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[8];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[9] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[9];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[10] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[10];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[11] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[11];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[12] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[12];

// ---------------------------------------------------------------------------
// Name:        SU_END_FUNCTION_WRAPPER
// Description: Ends a function wrapper.
// Arguments:   funcId - The wrapped function id.
// Author:      Uri Shomroni
// Date:        6/9/2009
// ---------------------------------------------------------------------------
#define SU_END_FUNCTION_WRAPPER(funcId) if (!su_stat_interoperabilityHelper.isInNestedFunction()) {SU_TECHNOLOGY_MONITOR.afterMonitoredFunctionExecutionActions(funcId);};\
    su_stat_functionInterceptionInfo[funcId]._isCurrentlyInsideWrapper = false;\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[0] = su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[0];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[1] = su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[1];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[2] = su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[2];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[3] = su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[3];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[4] = su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[4];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[5] = su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[5];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[6] = su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[6];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[7] = su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[7];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[8] = su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[8];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[9] = su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[9];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[10] = su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[10];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[11] = su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[11];\
    ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[12] = su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[12];\

    // ---------------------------------------------------------------------------
    // Name:        SU_BEFORE_EXECUTING_REAL_FUNCTION
    // Description: Should be used before calling a real function inside the spy
    // Arguments: funcId = ap_glXXXX (before glXXXX is used).
    // Author:      Uri Shomroni
    // Date:        6/9/2009
    // ---------------------------------------------------------------------------
#define SU_BEFORE_EXECUTING_REAL_FUNCTION(funcId)\
    if (!su_stat_functionInterceptionInfo[funcId]._isCurrentlyInsideWrapper)\
    {\
        ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[0] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[0];\
        ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[1] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[1];\
        ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[2] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[2];\
        ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[3] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[3];\
        ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[4] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[4];\
        ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[5] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[5];\
        ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[6] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[6];\
        ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[7] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[7];\
        ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[8] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[8];\
        ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[9] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[9];\
        ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[10] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[10];\
        ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[11] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[11];\
        ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[12] = su_stat_functionInterceptionInfo[funcId]._originalInstructions[12];\
    }

    // ---------------------------------------------------------------------------
    // Name:        SU_AFTER_EXECUTING_REAL_FUNCTION
    // Description: Should be used after calling a real function inside the spy
    // Arguments: funcId = ap_glXXXX (after glXXXX is used).
    // Author:      Uri Shomroni
    // Date:        6/9/2009
    // ---------------------------------------------------------------------------
#define SU_AFTER_EXECUTING_REAL_FUNCTION_ACTIONS(funcId)\
    if (!su_stat_functionInterceptionInfo[funcId]._isCurrentlyInsideWrapper)\
    {\
        ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[0] = su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[0];\
        ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[1] = su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[1];\
        ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[2] = su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[2];\
        ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[3] = su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[3];\
        ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[4] = su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[4];\
        ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[5] = su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[5];\
        ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[6] = su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[6];\
        ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[7] = su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[7];\
        ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[8] = su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[8];\
        ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[9] = su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[9];\
        ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[10] = su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[10];\
        ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[11] = su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[11];\
        ((gtUByte*)(((osProcedureAddress*)(&SU_REAL_FUNCTION_POINTERS_STRUCT))[funcId - SU_REAL_FUNCTION_POINTERS_STRUCT_FIRST_FUNCTION_OFFSET]))[12] = su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[12];\
    }
#else
#error Unknown Address Space type!
#endif


#else
// ---------------------------------------------------------------------------
// Name:        SU_START_FUNCTION_WRAPPER
// Description: Starts a function wrappers.
// Arguments:   funcId - The wrapped function id.
// Author:      Yaki Tebeka
// Date:        30/11/2006
// ---------------------------------------------------------------------------
#ifdef SU_USE_SINGLE_WRITE_MULTIPLE_READ_SYNC
#define SU_START_FUNCTION_WRAPPER(funcId) \
    suSWMRInstance::SharedLock();
#else // !defined(SU_USE_SINGLE_WRITE_MULTIPLE_READ_SYNC)
#define SU_START_FUNCTION_WRAPPER(funcId) 
#endif

// ---------------------------------------------------------------------------
// Name:        SU_START_FUNCTION_WRAPPER
// Description: Starts a WGL function wrappers.
// Arguments:   funcId - The WGL wrapped function id.
// Author:      Sigal Algranaty
// Date:        5/7/2010
// ---------------------------------------------------------------------------
#define SU_START_WGL_FUNCTION_WRAPPER(funcId) su_stat_interoperabilityHelper.onNestedFunctionEntered();

// ---------------------------------------------------------------------------
// Name:        SU_START_DRAW_FUNCTION_WRAPPER
// Description: Starts a "draw" function wrappers.
//              A draw function is a function that has a visual effect on the
//              rendered buffer.
// Arguments:   funcId - The wrapped function id.
// Author:      Yaki Tebeka
// Date:        30/11/2006
// ---------------------------------------------------------------------------
#ifdef SU_USE_SINGLE_WRITE_MULTIPLE_READ_SYNC
#define SU_START_DRAW_FUNCTION_WRAPPER(funcId) if (gs_stat_isInNullOpenGLImplementationMode) { return; }; \
    suSWMRInstance::SharedLock();
#else // !defined(SU_USE_SINGLE_WRITE_MULTIPLE_READ_SYNC)
#define SU_START_DRAW_FUNCTION_WRAPPER(funcId) if (gs_stat_isInNullOpenGLImplementationMode) { return; };
#endif


// ---------------------------------------------------------------------------
// Name:        SU_END_FUNCTION_WRAPPER
// Description: Ends a function wrapper.
// Arguments:   funcId - The wrapped function id.
// Author:      Sigal Algranaty
// Date:        5/7/2010
// ---------------------------------------------------------------------------
#define SU_END_WGL_FUNCTION_WRAPPER(funcId) su_stat_interoperabilityHelper.onNestedFunctionExited(); if (!su_stat_interoperabilityHelper.isInNestedFunction()) {SU_TECHNOLOGY_MONITOR.afterMonitoredFunctionExecutionActions(funcId);};

// ---------------------------------------------------------------------------
// Name:        SU_END_WGL_FUNCTION_WRAPPER
// Description: Ends a WGL function wrapper.
// Arguments:   funcId - The wrapped function id.
// Author:      Yaki Tebeka
// Date:        30/11/2006
// ---------------------------------------------------------------------------
#ifdef SU_USE_SINGLE_WRITE_MULTIPLE_READ_SYNC
#define SU_END_FUNCTION_WRAPPER(funcId) if (!su_stat_interoperabilityHelper.isInNestedFunction()) {SU_TECHNOLOGY_MONITOR.afterMonitoredFunctionExecutionActions(funcId);}; \
    suSWMRInstance::SharedUnLock();
#else // !defined(SU_USE_SINGLE_WRITE_MULTIPLE_READ_SYNC)
#define SU_END_FUNCTION_WRAPPER(funcId) if (!su_stat_interoperabilityHelper.isInNestedFunction()) {SU_TECHNOLOGY_MONITOR.afterMonitoredFunctionExecutionActions(funcId);};
#endif
	

// ---------------------------------------------------------------------------
// Name:        SU_BEFORE_EXECUTING_REAL_FUNCTION
// Description: Should be used before calling a real function inside the spy
// Arguments: funcId = ap_glXXXX (before glXXXX is used).
// Author:      Uri Shomroni
// Date:        25/1/2009
// ---------------------------------------------------------------------------
#define SU_BEFORE_EXECUTING_REAL_FUNCTION(funcId)

// ---------------------------------------------------------------------------
// Name:        SU_AFTER_EXECUTING_REAL_FUNCTION
// Description: Should be used after calling a real function inside the spy
// Arguments: funcId = ap_glXXXX (after glXXXX is used).
// Author:      Uri Shomroni
// Date:        25/1/2009
// ---------------------------------------------------------------------------
#define SU_AFTER_EXECUTING_REAL_FUNCTION_ACTIONS(funcId)
#endif

// OpenGL Error checking - only applicable for QA:
#ifdef GS_CHECK_OPENGL_ERRORS_AFTER_EACH_REAL_FUNCTION_CALL
#define SU_AFTER_EXECUTING_REAL_FUNCTION(funcId)\
    SU_AFTER_EXECUTING_REAL_FUNCTION_ACTIONS(funcId)\
    {\
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError)\
        GLenum previousError = gs_stat_realFunctionPointers.glGetError();\
        SU_AFTER_EXECUTING_REAL_FUNCTION_ACTIONS(ap_glGetError)\
        \
        if (previousError != GL_NO_ERROR)\
        {\
            gtString errMsg;\
            apGLenumValueToString(previousError, errMsg);\
            const char* funcName = apMonitoredFunctionsManager::instance().monitoredFunctionName(funcId);\
            errMsg.prependFormattedString(L"OpenGL Error in function %ls: ", funcName);\
            GT_ASSERT_EX((previousError == GL_NO_ERROR), errMsg.asCharArray());\
        }\
    }
#else // ndef GS_CHECK_OPENGL_ERRORS_AFTER_EACH_REAL_FUNCTION_CALL
#define SU_AFTER_EXECUTING_REAL_FUNCTION(funcId) SU_AFTER_EXECUTING_REAL_FUNCTION_ACTIONS(funcId)
#endif

// ---------------------------------------------------------------------------
// Name:        SU_CALL_EXTENSION_FUNC
// Description: Calls an extension function.
// Arguments:   functionPointerName - The name of the function pointer.
//                                    Example: glTexImage3D.
//              argumentsList - The list of called function arguments.
//                              This list needs to be surrounded by parenthesis.
//                              Example: (target, level, internal format, width, height, depth, border, format, type, pixels)
// Author:      Yaki Tebeka
// Date:        21/3/2005
// Implementation notes:
//  We use the gsExtensionsManager instance to get a pointer to our extension function.
//  If this pointer is NULL, it means that the debugged application got the pointer from
//  another OpenGL render context. In this case we generate a debugged process error event
//  and ignore the function call.
// ---------------------------------------------------------------------------
#define SU_CALL_EXTENSION_FUNC(functionPointerName, argumentsList) \
    { \
        /* Get the current render context extension functions pointers structure: */ \
        gsMonitoredFunctionPointers* pRealExtFuncsPtrs = gs_stat_extensionsManager.currentRenderContextExtensionsRealImplPointers(); \
        \
        /* If we have the requested function pointer - call it: */ \
        if (pRealExtFuncsPtrs != NULL) \
        {  \
            if ((pRealExtFuncsPtrs->functionPointerName) != NULL) \
            { \
                (pRealExtFuncsPtrs->functionPointerName) argumentsList; \
            } \
            else \
            { \
                /* We don't have the requested function pointer - generate a debugged process error event: */ \
                bool extensionFunctionPtrWasCopiesToThisContext = false; \
                gs_stat_openGLMonitorInstance.handleForeignContextExtensionFuncCall(_T(#functionPointerName), extensionFunctionPtrWasCopiesToThisContext); \
                \
                /* If the extension function pointer was found in other contexts, and copies into this context: */ \
                if (extensionFunctionPtrWasCopiesToThisContext) \
                { \
                    /* Call the copied pointer: */ \
                    (pRealExtFuncsPtrs->functionPointerName) argumentsList; \
                } \
            }  \
        } \
    }


// ---------------------------------------------------------------------------
// Name:        SU_CALL_EXTENSION_FUNC_WITH_RET_VAL
// Description: Same to SU_CALL_EXTENSION_FUNC, but also inputs a retVal argument.
// Author:      Yaki Tebeka
// Date:        21/3/2005
// ---------------------------------------------------------------------------
#define SU_CALL_EXTENSION_FUNC_WITH_RET_VAL(functionPointerName, argumentsList, retVal) \
    { \
        /* Get the current render context extension functions pointers structure: */ \
        gsMonitoredFunctionPointers* pRealExtFuncsPtrs = gs_stat_extensionsManager.currentRenderContextExtensionsRealImplPointers(); \
        \
        /* If we have the requested function pointer - call it: */ \
        if ((pRealExtFuncsPtrs != NULL) && ((pRealExtFuncsPtrs->functionPointerName) != NULL)) \
        { \
            retVal = (pRealExtFuncsPtrs->functionPointerName) argumentsList; \
        } \
        else \
        { \
            /* We don't have the requested function pointer - generate a debugged process error event: */ \
            bool extensionFunctionPtrWasCopiesToThisContext = false; \
            gs_stat_openGLMonitorInstance.handleForeignContextExtensionFuncCall(_T(#functionPointerName), extensionFunctionPtrWasCopiesToThisContext); \
            \
            /* If the extension function pointer was found in other contexts, and copies into this context: */ \
            if (extensionFunctionPtrWasCopiesToThisContext) \
            { \
                /* Call the copied pointer: */ \
                retVal = (pRealExtFuncsPtrs->functionPointerName) argumentsList; \
            } \
        } \
    }

// ---------------------------------------------------------------------------
// Name:        SU_CALL_CL_EXTENSION_FUNC_WITH_RET_VAL
// Description: Calls an OpenCL extension function.
// Arguments:   functionPointerName - The name of the function pointer.
//                                    Example: clGetGLContextInfoKHR.
//              argumentsList - The list of called function arguments.
//                              This list needs to be surrounded by parenthesis.
//                              Example: (properties, param_name, param_value_size, param_value, param_value_size_ret)
// Author:      Sigal Algranaty
// Date:        5/6/2011
// ---------------------------------------------------------------------------
#define SU_CALL_CL_EXTENSION_FUNC_WITH_RET_VAL(functionPointerName, argumentsList, retVal) \
    { \
        \
        /* If we have the requested function pointer - call it: */ \
        if (cs_stat_realFunctionPointers.functionPointerName != NULL) \
        { \
            retVal = (cs_stat_realFunctionPointers.functionPointerName) argumentsList; \
        } \
        else \
        { \
            /* We don't have the requested function pointer - this should not happen, throw an assertion: */ \
            gtString msg; \
            msg.appendFormattedString(L"Trying to call an extension function which is not supported. Function Name: %ls", _T(#functionPointerName)); \
            GT_ASSERT_EX(false, msg.asCharArray()) \
            \
        } \
    }



#endif //__SUINTERCEPTIONMACROS_H

