/************************************************************************************//**
** Copyright (c) 2015-2016 Advanced Micro Devices, Inc. All rights reserved.
**
** \author AMD Developer Tools
** \file
** \brief  The AMD GPU Kernel Debugging API to implement device kernel debugging on
**          AMD Graphics Core Next (GCN) GPUs.
**
** \mainpage Introduction
** \section Overview
** This document describes a set of interfaces which can be used
** by debugger or application developers to incorporate GPU kernel debugging
** functionality into their debugger or application running on AMD Graphics Core Next
** GPUs (or APUs).
**
** The AMD GPU Kernel Debugging API has been designed to hide the multiple
** driver API specific implementations and the internal architecture of a
** particular GPU device.  It has evolved starting from a minimal set of GPU debugging
** APIs that can be currently supported by AMD GPUs and software stacks.  As more
** GPU debug features are implemented and validated, the API will evolve further.
** It is still a work-in-progress.
**
** For HSA, this API together with the AMD HSA binary interface, AMD HSA debug info and
** AMD HSA API and dispatch interception mechanism form the AMD HSA GPU Debugging
** Architecture.
** Refer to the "AMD HSA GPU Debugging Architecture" document for more information.
**
** \section Assumptions
** The AMD GPU Kernel Debugging API is an "in-process" debug API.  That is, the API
** must be called from the same process address space as the program being debugged
** and will have direct access to all process resources.  No OS provided inter-process
** debug mechanisms are required, but it should be reasonably straightforward for
** tool developers to create a client/server remote debugging model through the
** introduction of a simple communication protocol.
**
** To inject these kernel debugging API calls into the debugged application process
** address space, the API and kernel dispatch interception mechanism provided through
** amd_hsa_tools_interfaces.h can be used.
**
** \section Requirements
** For HSA:
** 1. AMD Kaveri and Carrizo APUs
** 2. HSA Runtime and HSAIL 1.0 Final
**
****************************************************************************************/
#ifndef AMDGPUDEBUG_H_
#define AMDGPUDEBUG_H_

#include <stddef.h>   /* for size_t type */
#include <stdint.h>   /* for uintXX_t type */

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) || defined(__CYGWIN__)
    #if defined(HWDBG_EXPORTS)
        /** Export symbols when building the library on windows. */
        #define HWDBG_API_ENTRY __declspec(dllexport)
    #else
        /** No symbol export when used by the library user on windows. */
        #define HWDBG_API_ENTRY __declspec(dllimport)
    #endif
    /** The API calling convention on windows. */
    #define HWDBG_API_CALL __cdecl
#else
    #if __GNUC__ >= 4
        /** Use symbol visibility control supported by GCC 4.x and newer. */
        #define HWDBG_API_ENTRY __attribute__ ((visibility("default")))
    #else
        /** No symbol visibility control for GCC older than 4.0. */
        #define HWDBG_API_ENTRY
    #endif
    /** The API calling convention on linux. **/
    #define HWDBG_API_CALL
#endif

/** The AMD GPU Debug API major version. */
#define AMDGPUDEBUG_VERSION_MAJOR 1
/** The AMD GPU Debug API minor version. */
#define AMDGPUDEBUG_VERSION_MINOR 3
/** The AMD GPU Debug API build number. */
#define AMDGPUDEBUG_VERSION_BUILD 3698

/** The maximum number of lanes in a wavefront for the GPU device. */
#define HWDBG_WAVEFRONT_SIZE 64


/********************************* ENUMERATIONS *********************************/

/** The enumeration values of the possible return status from the provided API. */
/** \warning Not all the enum values are supported currently */
typedef enum
{
    /** the API was executed successfully */
    HWDBG_STATUS_SUCCESS                   = 0x0,

    /** a debugger internal error occurred */
    HWDBG_STATUS_ERROR                     = 0x01,

    /** the GPU device does not support debugging */
    HWDBG_STATUS_DEVICE_ERROR              = 0x02,

    /** the driver is not compatible with the API */
    HWDBG_STATUS_DRIVER_ERROR              = 0x03,

    /** a duplicate breakpoint is detected */
    HWDBG_STATUS_DUPLICATE_BREAKPOINT      = 0x04,

    /** invalid address alignment was provided */
    HWDBG_STATUS_INVALID_ADDRESS_ALIGNMENT = 0x05,

    /** an invalid debug context handle was provided */
    HWDBG_STATUS_INVALID_HANDLE            = 0x06,

    /** invalid input arguments were provided */
    HWDBG_STATUS_INVALID_PARAMETER         = 0x07,

    /** expected a non NULL input argument */
    HWDBG_STATUS_NULL_POINTER              = 0x08,

    /** out of range address was provided */
    HWDBG_STATUS_OUT_OF_RANGE_ADDRESS      = 0x09,

    /** failed to allocate memory */
    HWDBG_STATUS_OUT_OF_MEMORY             = 0x0A,

    /** ran out of hardware resources (for data breakpoints)  */
    HWDBG_STATUS_OUT_OF_RESOURCES          = 0x0B,

    /** started debugging on more than one application process */
    HWDBG_STATUS_REGISTRATION_ERROR        = 0x0C,

    /** an undefined operation was detected (i.e. an incorrect call order) */
    HWDBG_STATUS_UNDEFINED                 = 0x0D,

    /** the API has not been implemented */
    HWDBG_STATUS_UNSUPPORTED               = 0x0E,

    /** HwDbgInit has not been called */
    HWDBG_STATUS_NOT_INITIALIZED           = 0x0F,

    /** The debug context was created with unsupported behavior flags for the API*/
    HWDBG_STATUS_INVALID_BEHAVIOR_STATE    = 0x10

} HwDbgStatus;

/** The list of debugger commands for the HwDbgContinueEvent API to advance to the
    next state in the GPU debug engine. **/
typedef enum
{
    HWDBG_COMMAND_CONTINUE = 0x0, /**< resume the device execution */
} HwDbgCommand;

/** The enumeration values of possible driver software stacks supported by the library */
typedef enum
{
    HWDBG_API_HSA = 0x0,   /**< the library is built for HSA software stack */
} HwDbgAPIType;

/** The enumeration values of possible breakpoint types supported by the library. */
/** \warning This is not yet supported */
typedef enum
{
    HWDBG_BREAKPOINT_TYPE_NONE = 0x0,  /**< no breakpoint type */
    HWDBG_BREAKPOINT_TYPE_CODE = 0x1,  /**< instruction-based breakpoint type */
    HWDBG_BREAKPOINT_TYPE_DATA = 0x2,  /**< memory-based or data breakpoint type */
} HwDbgBreakpointType;

/** The enumeration values of possible event types returned by the HwDbgWaitForEvent
    API. */
typedef enum
{
    HWDBG_EVENT_INVALID         = 0x0, /**< an invalid event */
    HWDBG_EVENT_TIMEOUT         = 0x1, /**< has reached the user timeout value */
    HWDBG_EVENT_POST_BREAKPOINT = 0x2, /**< has reached a breakpoint */
    HWDBG_EVENT_END_DEBUGGING   = 0x3, /**< has completed kernel execution */
} HwDbgEventType;

/** The list of possible access modes of data breakpoints supported. */
/** \warning This is not yet supported */
typedef enum
{
    /** read operations only */
    HWDBG_DATABREAKPOINT_MODE_READ    = 0x1,

    /** write or atomic operations only */
    HWDBG_DATABREAKPOINT_MODE_NONREAD = 0x2,

    /** atomic operations only */
    HWDBG_DATABREAKPOINT_MODE_ATOMIC  = 0x4,

    /** read, write or atomic operations */
    HWDBG_DATABREAKPOINT_MODE_ALL     = 0x7,
} HwDbgDataBreakpointMode;


/************************************ TYPEDEFS **********************************/

/** The code location type (in bytes). */
typedef uint64_t HwDbgCodeAddress;

/** The hardware wavefront location type. */
typedef uint32_t HwDbgWavefrontAddress;

/** A unique handle for the kernel debug context (returned by HwDbgBeginDebugContext). */
typedef void* HwDbgContextHandle;

/** A unique handle for a code breakpoint (returned by HwDbgCreateCodeBreakpoint). */
typedef void* HwDbgCodeBreakpointHandle;

/** A unique handle for a data breakpoint (returned by HwDbgCreateDataBreakpoint). */
/** \warning This is not yet supported */
typedef void* HwDbgDataBreakpointHandle;


/*********************************** STRUCTURES *********************************/

/** A three dimensional type, used by work-group and work-item ids. */
typedef struct
{
    uint32_t x;  /**< x dimension */
    uint32_t y;  /**< y dimension */
    uint32_t z;  /**< z dimension */
} HwDbgDim3;

/** A structure to hold all the info required to create a single data breakpoint. */
/** \warning This is not yet supported */
typedef struct
{
    /** the relevant mode for the data breakpoint */
    HwDbgDataBreakpointMode dataBreakpointMode;

    /** the size of data in bytes being watched */
    uint64_t                dataSize;

    /** the memory address to be watched */
    void*                   pAddress;
} HwDbgDataBreakpointInfo;

/** A structure to hold the active wave info returned by HwDbgGetActiveWavefronts API */
typedef struct
{
    /** the work-group id */
    HwDbgDim3                 workGroupId;

    /** the work-item id (local id within a work-group) */
    HwDbgDim3                 workItemId[HWDBG_WAVEFRONT_SIZE];

    /** the execution mask of the work-items */
    uint64_t                  executionMask;

    /** the hardware wavefront slot address (not unique for a dispatch) */
    HwDbgWavefrontAddress     wavefrontAddress;

    /** the byte offset in the ISA binary for the wavefront */
    HwDbgCodeAddress          codeAddress;

    /** the data breakpoint handle */
    /** \warning This is not yet supported */
    HwDbgDataBreakpointHandle dataBreakpointHandle;

    /** the type of breakpoint that was signaled */
    /** \warning This is not yet supported */
    HwDbgBreakpointType       breakpointType;

    /** additional data that can be returned */
    void*                     pOtherData;
} HwDbgWavefrontInfo;

/** The enumerated bitfield values of supported behavior, the flags can be used internally to optimize behavior */
typedef enum
{
    /** Default flag, used to debug GPU dispatches */
    HWDBG_BEHAVIOR_NONE                         = 0x00,

    /** Disable GPU dispatch debugging.
     ** However this behavior mode allows extraction of kernel binaries and breakpoint management.
     ** Allowed API calls are HwDbg[Begin or End]DebugContext, HwDbgGetKernelBinary,
     ** HwDbg[*CodeBreakpoint*] and HwDbg[*DataBreakpoint*] */
    HWDBG_BEHAVIOR_DISABLE_DISPATCH_DEBUGGING   = 0x01

}HwDbgBehaviorType;

/** A structure to hold the device state as an input to the HwDbgBeginDebugContext */
typedef struct
{
    /** set to hsa_agent_t.handle from the pre-dispatch callback function */
    void* pDevice;

    /** set to hsa_kernel_dispatch_packet_t* from the pre-dispatch callback function */
    void* pPacket;

    /** set to packet_id from the pre-dispatch callback function */
    /** \warning This is not yet supported */
    uint64_t packetId;

    /** flags that the control the behavior of the debug context */
    uint32_t behaviorFlags;
} HwDbgState;


/********************************** DIAGNOSTIC LOGGING *********************************/

/** The enumerated bitfield values of supported logging message types */
typedef enum
{
    HWDBG_LOG_TYPE_NONE    = 0x00,  /**< do not register for any message */
    HWDBG_LOG_TYPE_ASSERT  = 0x01,  /**< register for assert messages */
    HWDBG_LOG_TYPE_ERROR   = 0x02,  /**< register for error messages */
    HWDBG_LOG_TYPE_TRACE   = 0x04,  /**< register for trace messages */
    HWDBG_LOG_TYPE_MESSAGE = 0x08,  /**< register for generic messages */
    HWDBG_LOG_TYPE_ALL     = 0x0f   /**< register for all messages */
} HwDbgLogType;

/************************************************************************************//**
** The user provided logging callback function to be registered.
**
** This function will be called when the message with the type registered by the user
** is generated by the library.
**
** \param[in] pUserData  The pointer specified by the user during registration
** \param[in] type       The type of log message being passed back
** \param[in] pMessage   The log message being passed back
****************************************************************************************/
typedef void (*HwDbgLoggingCallback)(      void*        pUserData,
                                     const HwDbgLogType type,
                                     const char* const  pMessage);

/************************************************************************************//**
** Register a logging callback function.
**
** Extra diagnostics output about the operation of the AMD GPU Debug API may
** be enabled by registering a client callback function through this API.
**
** This function can be called prior to a HwDbgInit call.
**
** \param[in] types      specifies the logging message types to be registered
**                        (a combination of HwDbgLogType enum value)
** \param[in] pCallback  specifies the logging callback function
**                        Set to a callback function function to enable logging
**                        Set to NULL to disable logging
** \param[in] pUserData  specifies a pointer to data that can be accessed by the
**                        user specified logging callback function
**
** \return HwDbgStatus
** \retval HWDBG_STATUS_SUCCESS       If the callback can be set successfully
** \retval HWDBG_STATUS_ERROR         If an error is encountered
** \retval HWDBG_STATUS_UNSUPPORTED   If the API has not been implemented
****************************************************************************************/
extern HWDBG_API_ENTRY HwDbgStatus HWDBG_API_CALL
HwDbgSetLoggingCallback(uint32_t             types,
                        HwDbgLoggingCallback pCallback,
                        void*                pUserData);


/***************************** LIBRARY VERSION AND TYPE ********************************/

/************************************************************************************//**
** Retrieve the library version (major, minor and build) number.
**
** This function can be called prior to a HwDbgInit call.
**
** \param[out] pVersionMajorOut  returns the API version major number
** \param[out] pVersionMinorOut  returns API version minor number
** \param[out] pVersionBuildOut  returns API build number
**
** \return HwDbgStatus
** \retval HWDBG_STATUS_SUCCESS       On success
** \retval HWDBG_STATUS_NULL_POINTER  If an input argument is NULL
** \retval HWDBG_STATUS_UNSUPPORTED   If the API is not yet implemented
****************************************************************************************/
extern HWDBG_API_ENTRY HwDbgStatus HWDBG_API_CALL
HwDbgGetAPIVersion(uint32_t* pVersionMajorOut,
                   uint32_t* pVersionMinorOut,
                   uint32_t* pVersionBuildOut);

/************************************************************************************//**
** Retrieve the driver API type of the loaded library.
**
** This function can be called prior to a HwDbgInit call.
**
** \param[out] pAPITypeOut  returns the API type of the library
**
** \return HwDbgStatus
** \retval HWDBG_STATUS_SUCCESS       On success
** \retval HWDBG_STATUS_NULL_POINTER  If the input argument is NULL
** \retval HWDBG_STATUS_UNSUPPORTED   If the API is not yet implemented
****************************************************************************************/
extern HWDBG_API_ENTRY HwDbgStatus HWDBG_API_CALL
HwDbgGetAPIType(HwDbgAPIType* pAPITypeOut);


/*********************** GPU DEBUG INITIALIZATION AND SHUTDOWN *************************/

/************************************************************************************//**
** Initialize the GPU debug engine.
**
** This function should be called right after the debugged process starts.
** For hsa, this is in the HSA Runtime's OnLoad callback.
**
** \param[in] pApiTable   Used by HSA: Pass in the pointer to the hsa api table
**                          provided by the HSA Runtime's OnLoad callback.
**                          Can be NULL (won't support full DBE functionality).
**
** \return HwDbgStatus
** \retval HWDBG_STATUS_SUCCESS        On success
** \retval HWDBG_STATUS_ERROR          If called multiple times without a
**                                      corresponding HwDbgShutDown
** \retval HWDBG_STATUS_OUT_OF_MEMORY  If fail to allocate necessary memory
**
** \see HwDbgShutDown
****************************************************************************************/
extern HWDBG_API_ENTRY HwDbgStatus HWDBG_API_CALL
HwDbgInit(void* pApiTable);

/************************************************************************************//**
** Shut down the GPU debug engine.
**
** This function should be called before the debugged process ends.
** For hsa, this should be called right before calling the hsa_shut_down API.
**
** \return HwDbgStatus
** \retval HWDBG_STATUS_SUCCESS          On success
** \retval HWDBG_STATUS_NOT_INITIALIZED  If called without a corresponding HwDbgInit
**
** \see HwDbgInit
****************************************************************************************/
extern HWDBG_API_ENTRY HwDbgStatus HWDBG_API_CALL
HwDbgShutDown();

/************************************************************************************//**
** Mark the start debugging of a kernel dispatch.
**
** This function should be called right before the execution of the kernel
** to be debugged (such as within the pre-dispatch callback function).
** Only one kernel dispatch should be between HwDbgBeginDebugContext and
** HwDbgEndDebugContext.
** Only one process can be debugged at a time in the system.
**
** \param[in]  state             specifies the input debug state
** \param[out] pDebugContextOut  returns the handle that identifies the particular
**                                kernel debug context
**
** \return HwDbgStatus
** \retval HWDBG_STATUS_SUCCESS             On success
** \retval HWDBG_STATUS_ERROR               If an internal error occurs
**                                           (check the log output for details)
** \retval HWDBG_STATUS_NOT_INITIALIZED     If called prior to a HwDbgInit call
** \retval HWDBG_STATUS_NULL_POINTER        If the input argument is NULL
** \retval HWDBG_STATUS_OUT_OF_MEMORY       If fail to allocate necessary memory
** \retval HWDBG_STATUS_REGISTRATION_ERROR  If more than 1 debug process is detected
** \retval HWDBG_STATUS_UNSUPPORTED         If the API has not been implemented
**
** \see HwDbgEndDebugContext
****************************************************************************************/
extern HWDBG_API_ENTRY HwDbgStatus HWDBG_API_CALL
HwDbgBeginDebugContext(const HwDbgState          state,
                             HwDbgContextHandle* pDebugContextOut);

/************************************************************************************//**
** Mark the end debugging of a kernel dispatch.
**
** This function must be called after the kernel has complete execution.
** Only one kernel dispatch should be between HwDbgBeginDebugContext and
** HwDbgEndDebugContext.
** Only one process can be debugged at a time in the system.
**
** \param[in] hDebugContext  specifies the context handle received
**                            from HwDbgBeginDebugContext API.
**                            If it is NULL, then all sessions in flight
**                            will be terminated and deleted
**
** \return HwDbgStatus
** \retval HWDBG_STATUS_SUCCESS          On success
** \retval HWDBG_STATUS_INVALID_HANDLE   If hDebugContext is an invalid handle
** \retval HWDBG_STATUS_NOT_INITIALIZED  If called prior to a HwDbgInit call
** \retval HWDBG_STATUS_UNDEFINED        If kernel execution has not yet completed
** \retval HWDBG_STATUS_UNSUPPORTED      If the API has not been implemented
**
** \see HwDbgBeginDebugContext
****************************************************************************************/
extern HWDBG_API_ENTRY HwDbgStatus HWDBG_API_CALL
HwDbgEndDebugContext(HwDbgContextHandle hDebugContext);


/******************************* GPU EVENT LOOP PROCESSING *****************************/

/************************************************************************************//**
** Wait on a debug event from the GPU device.
**
** This is a synchronous function that will not return until it receives an
** event or reaches the specified timeout value.
**
** \param[in] hDebugContext   specifies the context handle received
**                             from HwDbgBeginDebugContext API
** \param[in]  timeout        specifies how long to wait in milliseconds
**                             before timing out
** \param[out] pEventTypeOut  The resulting event type
**
** \return HwDbgStatus
** \retval HWDBG_STATUS_SUCCESS          On success
** \retval HWDBG_STATUS_ERROR            If an internal error occurs
**                                        (check the log output for details)
** \retval HWDBG_STATUS_INVALID_BEHAVIOR If the context behavior flags are invalid
** \retval HWDBG_STATUS_INVALID_HANDLE   If the input hDebugContext is invalid
** \retval HWDBG_STATUS_NOT_INITIALIZED  If called prior to a HwDbgInit call
** \retval HWDBG_STATUS_NULL_POINTER     If an input argument is NULL
** \retval HWDBG_STATUS_UNDEFINED        If the kernel has completed execution
** \retval HWDBG_STATUS_UNSUPPORTED      If the API has not been implemented
**
** \see HwDbgContinueEvent
****************************************************************************************/
extern HWDBG_API_ENTRY HwDbgStatus HWDBG_API_CALL
HwDbgWaitForEvent(      HwDbgContextHandle hDebugContext,
                  const uint32_t           timeout,
                        HwDbgEventType*    pEventTypeOut);

/************************************************************************************//**
** Continue to the next operation (resume device execution, run to the next
** breakpoint).
**
** This is performed after receiving an event from HwDbgWaitForEvent.
** This is an asynchronous call, subsequent calls are undefined until
** the next HwDbgWaitEvent call.
**
** \param[in] hDebugContext  specifies the context handle received
**                            from HwDbgBeginDebugContext API
** \param[in] command        specifies the debugger command to execute next
**
** \return HwDbgStatus
** \retval HWDBG_STATUS_SUCCESS            On success
** \retval HWDBG_STATUS_ERROR              If an internal error occurs
**                                          (check the log output for details)
** \retval HWDBG_STATUS_INVALID_BEHAVIOR   If the context behavior flags are invalid
** \retval HWDBG_STATUS_INVALID_HANDLE     If the input hDebugContext is invalid
** \retval HWDBG_STATUS_INVALID_PARAMETER  If the command argument is invalid
** \retval HWDBG_STATUS_NOT_INITIALIZED    If called prior to a HwDbgInit call
** \retval HWDBG_STATUS_UNDEFINED          If the kernel has completed execution
** \retval HWDBG_STATUS_UNSUPPORTED        If the API has not been implemented
**
** \see HwDbgWaitForEvent
****************************************************************************************/
extern HWDBG_API_ENTRY HwDbgStatus HWDBG_API_CALL
HwDbgContinueEvent(      HwDbgContextHandle hDebugContext,
                   const HwDbgCommand       command);


/************************ GPU INSTRUCTION-BASED BREAKPOINT CONTROL *********************/

/************************************************************************************//**
** Create a breakpoint at a specified program counter.
**
** \param[in]  hDebugContext   specifies the context handle received
**                              from HwDbgBeginDebugContext API
** \param[in]  codeAddress     specifies the byte offset into the ISA binary
**                              indicating where to set the breakpoint.
**                              This has to be 4-byte aligned for AMD GPUs.
** \param[out] pBreakpointOut  returns the handle of the newly created instruction-based
**                              breakpoint. It is valid for use anywhere after creation.
**                              However, it is undefined to change the breakpoint
**                              state outside the HwDbgWaitForEvent/
**                              HwDbgContinueEvent pair associated with the
**                              kernel dispatch that the breakpoint was created for
**
** \return HwDbgStatus
** \retval HWDBG_STATUS_SUCCESS          On success
** \retval HWDBG_STATUS_ERROR            If the codeAddress is invalid (not 4-byte
**                                        aligned or out of range) or has been
**                                        inserted before
** \retval HWDBG_STATUS_INVALID_BEHAVIOR If the context behavior flags are invalid
** \retval HWDBG_STATUS_INVALID_HANDLE   If the input hDebugContext is invalid
** \retval HWDBG_STATUS_NOT_INITIALIZED  If called prior to a HwDbgInit call
** \retval HWDBG_STATUS_NULL_POINTER     If the input argument is NULL
** \retval HWDBG_STATUS_UNSUPPORTED      If the API has not been implemented
**
** \see HwDbgDeleteCodeBreakpoint, HwDbgDeleteAllCodeBreakpoints,
**      HwDbgGetCodeBreakpointAddress
****************************************************************************************/
extern HWDBG_API_ENTRY HwDbgStatus HWDBG_API_CALL
HwDbgCreateCodeBreakpoint(      HwDbgContextHandle         hDebugContext,
                          const HwDbgCodeAddress           codeAddress,
                                HwDbgCodeBreakpointHandle* pBreakpointOut);

/************************************************************************************//**
** Delete a instruction-based breakpoint.
**
** \param[in] hDebugContext   specifies the context handle received
**                             from HwDbgBeginDebugContext API
** \param[in] hBreakpoint     specifies the breakpoint handle. The handle is
**                             invalid after this call and may be
**                             returned in future calls to HwDbgCreateCodeBreakpoint
**
** \return HwDbgStatus
** \retval HWDBG_STATUS_SUCCESS          On success
** \retval HWDBG_STATUS_ERROR            If breakpoint handle is invalid or
**                                        contains an invalid code address
** \retval HWDBG_STATUS_INVALID_BEHAVIOR If the context behavior flags are invalid
** \retval HWDBG_STATUS_INVALID_HANDLE   If the input hDebugContext is invalid
** \retval HWDBG_STATUS_NOT_INITIALIZED  If called prior to a HwDbgInit call
** \retval HWDBG_STATUS_UNSUPPORTED      If the API has not been implemented
**
** \see HwDbgCreateCodeBreakpoint, HwDbgDeleteAllCodeBreakpoints,
**      HwDbgGetCodeBreakpointAddress
****************************************************************************************/
extern HWDBG_API_ENTRY HwDbgStatus HWDBG_API_CALL
HwDbgDeleteCodeBreakpoint(HwDbgContextHandle        hDebugContext,
                          HwDbgCodeBreakpointHandle hBreakpoint);

/************************************************************************************//**
** Delete all instruction-based breakpoints.
**
** \param[in] hDebugContext  specifies the context handle received
**                            from HwDbgBeginDebugContext API
**
** \return HwDbgStatus
** \retval HWDBG_STATUS_SUCCESS          On success
** \retval HWDBG_STATUS_ERROR            If an internal error occurs
**                                        (check the log output for details)
** \retval HWDBG_STATUS_INVALID_BEHAVIOR If the context behavior flags are invalid
** \retval HWDBG_STATUS_INVALID_HANDLE   If the input hDebugContext is invalid
** \retval HWDBG_STATUS_NOT_INITIALIZED  If called prior to a HwDbgInit call
** \retval HWDBG_STATUS_UNSUPPORTED      If the API has not been implemented
**
** \see HwDbgCreateCodeBreakpoint, HwDbgDeleteCodeBreakpoint,
**      HwDbgGetCodeBreakpointAddress
****************************************************************************************/
extern HWDBG_API_ENTRY HwDbgStatus HWDBG_API_CALL
HwDbgDeleteAllCodeBreakpoints(HwDbgContextHandle hDebugContext);

/************************************************************************************//**
** Retrieve the code location from an instruction-based breakpoint handle.
**
** \param[in]  hDebugContext    specifies the context handle received
**                               from HwDbgBeginDebugContext API
** \param[in]  hBreakpoint      specifies the breakpoint handle
** \param[out] pCodeAddressOut  returns the code address (program counter)
**
** \return HwDbgStatus
** \return HWDBG_STATUS_SUCCESS          On success
** \retval HWDBG_STATUS_ERROR            If an internal error occurs
**                                        (check the log output for details)
** \retval HWDBG_STATUS_INVALID_BEHAVIOR If the context behavior flags are invalid
** \retval HWDBG_STATUS_INVALID_HANDLE   If the input hDebugContext is invalid
** \retval HWDBG_STATUS_NOT_INITIALIZED  If called prior to a HwDbgInit call
** \retval HWDBG_STATUS_NULL_POINTER     If the input argument is NULL
** \retval HWDBG_STATUS_UNSUPPORTED      If the API has not been implemented
**
** \see HwDbgCreateCodeBreakpoint, HwDbgDeleteCodeBreakpoint,
**      HwDbgDeleteAllCodeBreakpoints
****************************************************************************************/
extern HWDBG_API_ENTRY HwDbgStatus HWDBG_API_CALL
HwDbgGetCodeBreakpointAddress(const HwDbgContextHandle        hDebugContext,
                              const HwDbgCodeBreakpointHandle hBreakpoint,
                                    HwDbgCodeAddress*         pCodeAddressOut);


/******************************* KERNEL BINARY INFORMATION *****************************/

/************************************************************************************//**
** Retrieve the kernel binary (in ELF) of the kernel dispatch.
**
** For HSA, the binary is the loaded and relocated code object.
** The binary contains the debugging information (in DWARF) from high level source
** to ISA (can be multiple level of DWARFs such as one DWARF to represent the mapping
** from a high level kernel source to BRIG and another DWARF to represent the mapping
** from BRIG to ISA).
** \note Refer to the following two documentation for more information:
** 1. HSA Application Binary Interface - AMD GPU Architecture document for the
**    complete ABI.
** 2. HSA Debug Information document for the HSA DWARF extension
**
** \param[in]  hDebugContext   specifies the context handle received
**                              from HwDbgBeginDebugContext API
** \param[out] ppBinaryOut     returns a pointer to a buffer containing the
**                              binary kernel code object
**                              The lifetime of the buffer is within the
**                              debug context (i.e. after HwDbgBeginDebugContext call
**                              until the HwDbgEndDebugContext call)
** \param[out] pBinarySizeOut  returns the binary size in bytes
**
** \return HwDbgStatus
** \retval HWDBG_STATUS_SUCCESS          On success
** \retval HWDBG_STATUS_DRIVER_ERROR     If the retrieved kernel binary is NULL
**                                        or the binary size is 0
** \retval HWDBG_STATUS_ERROR            If an internal error occurs
**                                        (check the log output for details)
** \retval HWDBG_STATUS_INVALID_BEHAVIOR If the context behavior flags are invalid
** \retval HWDBG_STATUS_INVALID_HANDLE   If the input hDebugContext is invalid
** \retval HWDBG_STATUS_NOT_INITIALIZED  If called prior to a HwDbgInit call
** \retval HWDBG_STATUS_NULL_POINTER     If the input argument is NULL
** \retval HWDBG_STATUS_UNSUPPORTED      If the API has not been implemented
****************************************************************************************/
extern HWDBG_API_ENTRY HwDbgStatus HWDBG_API_CALL
HwDbgGetKernelBinary(const HwDbgContextHandle hDebugContext,
                     const void**             ppBinaryOut,
                           size_t*            pBinarySizeOut);

/************************************************************************************//**
** Retrieve the dispatched kernel name.
**
** \param[in]  hDebugContext     specifies the context handle received
**                                from HwDbgBeginDebugContext API
** \param[out] ppKernelNameOut   returns a pointer to a null-terminated character array
**                                 The lifetime of the character array is within the
**                                 debug context (i.e. after HwDbgBeginDebugContext call
**                                 until the HwDbgEndDebugContext call)
**
** \return HwDbgStatus
** \retval HWDBG_STATUS_SUCCESS          On success
** \retval HWDBG_STATUS_DRIVER_ERROR     If the retrieved kernel name is NULL
** \retval HWDBG_STATUS_ERROR            If an internal error occurs
**                                        (check the log output for details)
** \retval HWDBG_STATUS_INVALID_BEHAVIOR If the context behavior flags are invalid
** \retval HWDBG_STATUS_INVALID_HANDLE   If the input hDebugContext is invalid
** \retval HWDBG_STATUS_NULL_POINTER     If the input argument is NULL
** \retval HWDBG_STATUS_NOT_INITIALIZED  If called prior to a HwDbgInit call
** \retval HWDBG_STATUS_UNSUPPORTED      If the API has not been implemented
****************************************************************************************/
extern HWDBG_API_ENTRY HwDbgStatus HWDBG_API_CALL
HwDbgGetDispatchedKernelName(const HwDbgContextHandle hDebugContext,
                             const char**             ppKernelNameOut);


/****************************** GPU DEVICE STATE INSPECTION ****************************/

/************************************************************************************//**
** Retrieve the list of active wavefronts for the kernel dispatch in the GPU device.
**
** Must only be called after receiving a HWDBG_EVENT_POST_BREAKPOINT event from
** HwDbgWaitForEvent API.
**
** \param[in]  hDebugContext       specifies the context handle received
**                                  from HwDbgBeginDebugContext API
** \param[out] ppWavefrontInfoOut  returns a pointer to HwDbgWavefrontInfo structures.
**                                  It contains the work-group ids, work-
**                                  item ids, code adress, etc for each wavefront
** \param[out] pNumWavefrontsOut   returns the number of active wavefronts
**
** \return HwDbgStatus
** \retval HWDBG_STATUS_SUCCESS          On success
** \retval HWDBG_STATUS_ERROR            If an internal error occurs
**                                        (check the log output for details)
** \retval HWDBG_STATUS_INVALID_BEHAVIOR If the context behavior flags are invalid
** \retval HWDBG_STATUS_INVALID_HANDLE   If the input hDebugContext is invalid
** \retval HWDBG_STATUS_NOT_INITIALIZED  If called prior to a HwDbgInit call
** \retval HWDBG_STATUS_NULL_POINTER     If the ppWaveInfoOut is NULL
** \retval HWDBG_STATUS_UNDEFINED        If it is called after not receiving
**                                        a HWDBG_EVENT_POST_BREAKPOINT event
** \retval HWDBG_STATUS_UNSUPPORTED      If the API has not been implemented
****************************************************************************************/
extern HWDBG_API_ENTRY HwDbgStatus HWDBG_API_CALL
HwDbgGetActiveWavefronts(const HwDbgContextHandle   hDebugContext,
                         const HwDbgWavefrontInfo** ppWavefrontInfoOut,
                               uint32_t*            pNumWavefrontsOut);

/************************************************************************************//**
** Read data from a memory region.
**
** \warning Only private memory region (IMR_Scratch = 1) is currently supported.
**
** Must only be called after receiving a HWDBG_EVENT_POST_BREAKPOINT event from
** HwDbgWaitForEvent API.
**
** \param[in]  hDebugContext   specifies the context handle received
**                              from HwDbgBeginDebugContext API
** \param[in]  memoryRegion    specifies the target memory region to read from.
**                              This should be set to an enum value stored in
**                              DW_AT_HSA_isa_memory_region attribute of
**                              DW_TAG_variable tag in ISA DWARF.
** \param[in]  workGroupId     specifies the work-group id of interest (from
**                              HwDbgGetActiveWavefronts)
** \param[in]  workItemId      specifies the work-item id of interest (from
**                              HwDbgGetActiveWavefronts)
** \param[in]  offset          specifies a byte offset for the logical
**                              location that should be retrieved. On GPU,
**                              this must be a multiple of 4 bytes
**                              (align on a DWORD boundary)
** \param[in]  numBytesToRead  specifies the number of bytes to retrieve
**                              On GPU, this must be a multiple of 4 bytes
** \param[out] pMemOut         returns a pointer to a memory chunk of at least
**                              "numBytesToRead" bytes long
** \param[out] pNumBytesOut    returns the number of bytes written into pMemOut
**
** \return HwDbgStatus
** \retval HWDBG_STATUS_SUCCESS          On success
** \retval HWDBG_STATUS_INVALID_BEHAVIOR If the context behavior flags are invalid
** \retval HWDBG_STATUS_INVALID_HANDLE   If the input hDebugContext is invalid
** \retval HWDBG_STATUS_NOT_INITIALIZED  If called prior to a HwDbgInit call
** \retval HWDBG_STATUS_NULL_POINTER     If an input argument is NULL
** \retval HWDBG_STATUS_UNSUPPORTED      If the API has not been implemented
****************************************************************************************/
extern HWDBG_API_ENTRY HwDbgStatus HWDBG_API_CALL
HwDbgReadMemory(const HwDbgContextHandle hDebugContext,
                const uint32_t           memoryRegion,
                const HwDbgDim3          workGroupId,
                const HwDbgDim3          workItemId,
                const size_t             offset,
                const size_t             numBytesToRead,
                      void*              pMemOut,
                      size_t*            pNumBytesOut);


/***************************** GPU DEVICE EXECUTION CONTROL ****************************/

/************************************************************************************//**
** Break kernel execution of all active wavefronts for a kernel dispatch.
**
** Can be called at any time after a HwDbgBeginDebugContext call.
**
** \param[in]  hDebugContext  specifies the context handle received
**                             from HwDbgBeginDebugContext API
**
** \return HwDbgStatus
** \retval HWDBG_STATUS_SUCCESS          On success
** \retval HWDBG_STATUS_ERROR            If an internal error occurs
**                                        (check the log output for details)
** \retval HWDBG_STATUS_INVALID_BEHAVIOR If the context behavior flags are invalid
** \retval HWDBG_STATUS_INVALID_HANDLE   If the input hDebugContext is invalid
** \retval HWDBG_STATUS_NOT_INITIALIZED  If called prior to a HwDbgInit call
** \retval HWDBG_STATUS_UNSUPPORTED      If the API has not been implemented
****************************************************************************************/
extern HWDBG_API_ENTRY HwDbgStatus HWDBG_API_CALL
HwDbgBreakAll(const HwDbgContextHandle hDebugContext);

/************************************************************************************//**
** Terminate the kernel dispatch execution.
**
** Can be called at any time after a HwDbgBeginDebugContext call.
** Can be called multiple times to terminate a large kernel dispatch.
**
** \param[in]  hDebugContext  specifies the context handle received
**                             from HwDbgBeginDebugContext API
**
** \return HwDbgStatus
** \retval HWDBG_STATUS_SUCCESS          On success
** \retval HWDBG_STATUS_ERROR            If an internal error occurs
**                                        (check the log output for details)
** \retval HWDBG_STATUS_INVALID_BEHAVIOR If the context behavior flags are invalid
** \retval HWDBG_STATUS_INVALID_HANDLE   If the input hDebugContext is invalid
** \retval HWDBG_STATUS_NOT_INITIALIZED  If called prior to a HwDbgInit call
** \retval HWDBG_STATUS_UNSUPPORTED      If the API has not been implemented
****************************************************************************************/
extern HWDBG_API_ENTRY HwDbgStatus HWDBG_API_CALL
HwDbgKillAll(const HwDbgContextHandle hDebugContext);


/***************************** GPU DATA BREAKPOINT CONTROL *****************************/

/************************************************************************************//**
** Create a data breakpoint.
**
** \warning This is not yet supported
**
** \param[in]  hDebugContext       specifies the context handle received
**                                  from HwDbgBeginDebugContext API
** \param[in]  breakpointInfo      specifies the structure containing information
**                                  where to set the data breakpoint
** \param[out] pDataBreakpointOut  returns the handle of the newly created data
**                                  breakpoint. It is valid for use anywhere after
**                                  creation. However, it is undefined to change the
**                                  breakpoint state outside the HwDbgWaitForEvent/
**                                  HwDbgContinueEvent pair associated with the
**                                  shader dispatch that the breakpoint was created
**                                  for
**
** \return HwDbgStatus
** \retval HWDBG_STATUS_SUCCESS           On success
** \retval HWDBG_STATUS_ERROR             If an internal error occurs
**                                         (check the log output for details)
** \retval HWDBG_STATUS_INVALID_BEHAVIOR  If the context behavior flags are invalid
** \retval HWDBG_STATUS_INVALID_HANDLE    If the input hDebugContext is invalid
** \retval HWDBG_STATUS_NOT_INITIALIZED   If called prior to a HwDbgInit call
** \retval HWDBG_STATUS_NULL_POINTER      If the input argument or address is NULL
** \retval HWDBG_STATUS_OUT_OF_RESOURCES  If cannot be created due to hw limits
** \retval HWDBG_STATUS_UNSUPPORTED       If the API has not been implemented
**
** \see HwDbgDeleteDataBreakpoint, HwDbgDeleteAllDataBreakpoints,
**      HwDbgGetDataBreakpointInfo
****************************************************************************************/
extern HWDBG_API_ENTRY HwDbgStatus HWDBG_API_CALL
HwDbgCreateDataBreakpoint(      HwDbgContextHandle         hDebugContext,
                          const HwDbgDataBreakpointInfo    breakpointInfo,
                                HwDbgDataBreakpointHandle* pDataBreakpointOut);

/************************************************************************************//**
** Delete a data breakpoint.
**
** \warning This is not yet supported
**
** \param[in] hDebugContext    specifies the context handle received
**                              from HwDbgBeginDebugContext API
** \param[in] hDataBreakpoint  specifies the data breakpoint handle. The handle is
**                              invalid after this call and may be returned in
**                              future calls to HwDbgCreateCodeBreakpoint
**
** \return HwDbgStatus
** \retval HWDBG_STATUS_SUCCESS          On success
** \retval HWDBG_STATUS_ERROR            If an internal error occurs
**                                        (check the log output for details)
** \retval HWDBG_STATUS_INVALID_BEHAVIOR If the context behavior flags are invalid
** \retval HWDBG_STATUS_INVALID_HANDLE   If the input hDebugContext is invalid
** \retval HWDBG_STATUS_NOT_INITIALIZED  If called prior to a HwDbgInit call
** \retval HWDBG_STATUS_UNSUPPORTED      If the API has not been implemented
**
** \see HwDbgCreateDataBreakpoint, HwDbgDeleteAllDataBreakpoints,
**      HwDbgGetDataBreakpointInfo
****************************************************************************************/
extern HWDBG_API_ENTRY HwDbgStatus HWDBG_API_CALL
HwDbgDeleteDataBreakpoint(HwDbgContextHandle        hDebugContext,
                          HwDbgDataBreakpointHandle hDataBreakpoint);

/************************************************************************************//**
** Delete all data breakpoints.
**
** \warning This is not yet supported
**
** After this call, all data breakpoint handles created prior for the debug
** context will be invalid.
**
** \param[in] hDebugContext  specifies the context handle received
**                            from HwDbgBeginDebugContext API
**
** \return HwDbgStatus
** \retval HWDBG_STATUS_SUCCESS          On success
** \retval HWDBG_STATUS_ERROR            If an internal error occurs
**                                        (check the log output for details)
** \retval HWDBG_STATUS_INVALID_BEHAVIOR If the context behavior flags are invalid
** \retval HWDBG_STATUS_INVALID_HANDLE   If the input hDebugContext is invalid
** \retval HWDBG_STATUS_NOT_INITIALIZED  If called prior to a HwDbgInit call
** \retval HWDBG_STATUS_UNSUPPORTED      If the API has not been implemented
**
** \see HwDbgCreateDataBreakpoint, HwDbgDeleteDataBreakpoint,
**      HwDbgGetDataBreakpointInfo
****************************************************************************************/
extern HWDBG_API_ENTRY HwDbgStatus HWDBG_API_CALL
HwDbgDeleteAllDataBreakpoints(HwDbgContextHandle hDebugContext);

/************************************************************************************//**
** Retrieve the data breakpoint information from a data breakpoint handle.
**
** \warning This is not yet supported
**
** \param[in]  hDebugContext             specifies the context handle received
**                                        from HwDbgBeginDebugContext API
** \param[in]  hDataBreakpoint           specifies the data breakpoint handle
** \param[out] pDataBreakpointInfoOut    returns a structure containing
**                                        information of the data breakpoint
** \retval HWDBG_STATUS_NOT_INITIALIZED  If called prior to a HwDbgInit call
**
** \return HwDbgStatus
** \return HWDBG_STATUS_SUCCESS          On success
** \retval HWDBG_STATUS_ERROR            If an internal error occurs
**                                        (check the log output for details)
** \retval HWDBG_STATUS_INVALID_BEHAVIOR If the context behavior flags are invalid
** \retval HWDBG_STATUS_INVALID_HANDLE   If the input hDebugContext is invalid
** \retval HWDBG_STATUS_NULL_POINTER     If the input argument is NULL
** \retval HWDBG_STATUS_UNSUPPORTED      If the API has not been implemented
**
** \see HwDbgCreateDataBreakpoint, HwDbgDeleteDataBreakpoint,
**      HwDbgDeleteAllDataBreakpoints
****************************************************************************************/
extern HWDBG_API_ENTRY HwDbgStatus HWDBG_API_CALL
HwDbgGetDataBreakpointInfo(const HwDbgContextHandle        hDebugContext,
                           const HwDbgDataBreakpointHandle hDataBreakpoint,
                                 HwDbgDataBreakpointInfo*  pDataBreakpointInfoOut);


#ifdef __cplusplus
}
#endif

#endif
