//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apiClassesInitFunc.cpp
///
//==================================================================================

//------------------------------ apiClassesInitFunc.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osWrappersInitFunc.h>
#include <AMDTOSWrappers/Include/osTransferableObjectCreator.h>
#include <AMDTOSWrappers/Include/osTransferableObjectCreatorsManager.h>

// Local:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    #ifndef _GR_IPHONE_BUILD
        #include <AMDTAPIClasses/Include/apCGLParameters.h>
    #endif
#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
    #include <AMDTAPIClasses/Include/apGLXParameters.h>
#endif

// OpenCL is not supported on the iPhone platform:
#ifndef _GR_IPHONE_BUILD
    #include <AMDTAPIClasses/Include/apCLEnqueuedCommands.h>
    #include <AMDTAPIClasses/Include/apCLContext.h>
#endif

#include <AMDTAPIClasses/Include/Events/apAddWatchEvent.h>
#include <AMDTAPIClasses/Include/Events/apAfterKernelDebuggingEvent.h>
#include <AMDTAPIClasses/Include/Events/apApiConnectionEstablishedEvent.h>
#include <AMDTAPIClasses/Include/Events/apApiConnectionEndedEvent.h>
#include <AMDTAPIClasses/Include/Events/apBeforeDebuggedProcessRunResumedEvent.h>
#include <AMDTAPIClasses/Include/Events/apBeforeKernelDebuggingEvent.h>
#include <AMDTAPIClasses/Include/Events/apBreakpointHitEvent.h>
#include <AMDTAPIClasses/Include/Events/apCallStackFrameSelectedEvent.h>
#include <AMDTAPIClasses/Include/Events/apComputeContextCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apComputeContextDeletedEvent.h>
#include <AMDTAPIClasses/Include/Events/apContextDataSnapshotWasUpdatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessDetectedErrorEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessIsDuringTerminationEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessOutputStringEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessRunResumedEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessRunStartedEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessRunStartedExternallyEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessRunSuspendedEvent.h>
#include <AMDTAPIClasses/Include/Events/apDeferredCommandEvent.h>
#include <AMDTAPIClasses/Include/Events/apExceptionEvent.h>
#include <AMDTAPIClasses/Include/Events/apFlushTextureImageEvent.h>
#include <AMDTAPIClasses/Include/Events/apGDBErrorEvent.h>
#include <AMDTAPIClasses/Include/Events/apGDBListenerThreadWasSuspendedEvent.h>
#include <AMDTAPIClasses/Include/Events/apGDBOutputStringEvent.h>
#include <AMDTAPIClasses/Include/Events/apInfrastructureEndsBeingBusyEvent.h>
#include <AMDTAPIClasses/Include/Events/apInfrastructureFailureEvent.h>
#include <AMDTAPIClasses/Include/Events/apInfrastructureStartsBeingBusyEvent.h>
#include <AMDTAPIClasses/Include/Events/apKernelDebuggingFailedEvent.h>
#include <AMDTAPIClasses/Include/Events/apKernelDebuggingInterruptedEvent.h>
#include <AMDTAPIClasses/Include/Events/apKernelSourceBreakpointsUpdatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apMemoryAllocationFailureEvent.h>
#include <AMDTAPIClasses/Include/Events/apMemoryLeakEvent.h>
#include <AMDTAPIClasses/Include/Events/apModuleLoadedEvent.h>
#include <AMDTAPIClasses/Include/Events/apModuleUnloadedEvent.h>
#include <AMDTAPIClasses/Include/Events/apOutputDebugStringEvent.h>
#include <AMDTAPIClasses/Include/Events/apUserWarningEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessTerminatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessCreationFailureEvent.h>
#include <AMDTAPIClasses/Include/Events/apRenderContextCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apRenderContextDeletedEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLQueueCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLQueueDeletedEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLProgramBuildEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLProgramBuildFailedWithDebugFlagsEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLProgramCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLProgramDeletedEvent.h>
#include <AMDTAPIClasses/Include/Events/apGLDebugOutputMessageEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLErrorEvent.h>
#include <AMDTAPIClasses/Include/Events/apSearchingForMemoryLeaksEvent.h>
#include <AMDTAPIClasses/Include/Events/apSpyProgressEvent.h>
#include <AMDTAPIClasses/Include/Events/apTechnologyMonitorFailureEvent.h>
#include <AMDTAPIClasses/Include/Events/apThreadCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apThreadTerminatedEvent.h>
#include <AMDTAPIClasses/Include/ap2DPoint.h>
#include <AMDTAPIClasses/Include/ap2DRectangle.h>
#include <AMDTAPIClasses/Include/apApiFunctionsInitializationData.h>
#include <AMDTAPIClasses/Include/apCounterInfo.h>
#include <AMDTAPIClasses/Include/apDetectedErrorParameters.h>
#include <AMDTAPIClasses/Include/apExpression.h>
#include <AMDTAPIClasses/Include/apFunctionCall.h>
#include <AMDTAPIClasses/Include/apGLComputeShader.h>
#include <AMDTAPIClasses/Include/apGLFBO.h>
#include <AMDTAPIClasses/Include/apGLFragmentShader.h>
#include <AMDTAPIClasses/Include/apGLGeometryShader.h>
#include <AMDTAPIClasses/Include/apGLItemsCollection.h>
#include <AMDTAPIClasses/Include/apGLPipeline.h>
#include <AMDTAPIClasses/Include/apGLPixelInternalFormatParameter.h>
#include <AMDTAPIClasses/Include/apGLProgram.h>
#include <AMDTAPIClasses/Include/apGLSampler.h>
#include <AMDTAPIClasses/Include/apGLTexture.h>
#include <AMDTAPIClasses/Include/apGLTessellationControlShader.h>
#include <AMDTAPIClasses/Include/apGLTessellationEvaluationShader.h>
#include <AMDTAPIClasses/Include/apGLTransformFeedbackObject.h>
#include <AMDTAPIClasses/Include/apGLUnsupportedShader.h>
#include <AMDTAPIClasses/Include/apGLVertexShader.h>
#include <AMDTAPIClasses/Include/apKernelFunctionNameBreakpoint.h>
#include <AMDTAPIClasses/Include/apKernelSourceCodeBreakpoint.h>
#include <AMDTAPIClasses/Include/apHostSourceBreakPoint.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionBreakPoint.h>
#include <AMDTAPIClasses/Include/apGenericBreakpoint.h>
#include <AMDTAPIClasses/Include/apParameters.h> // Includes OpenGL / OpenCL parameters
#include <AMDTAPIClasses/Include/apPointerParameterCreator.h>
#include <AMDTAPIClasses/Include/apSourceCodeBreakpoint.h>
#include <AMDTAPIClasses/Include/apiClassesInitFunc.h>


// ---------------------------------------------------------------------------
// Name:        apiClassesInitFunc
// Description: Initialization function for the ApiClasses library.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/6/2004
// Implementation Notes:
//   a. Initializes float and double display precision.
//   b. Registers all the ApiClasses transferable objects in the transferable
//      objects creator manager.
// ---------------------------------------------------------------------------
bool apiClassesInitFunc()
{
    bool retVal = true;

    // Initialize float and double display precisions:
    apSetFloatParamsDisplayPrecision(8);

    // Verify that this function code is executed only once:
    static bool wasThisFunctionCalled = false;

    if (!wasThisFunctionCalled)
    {
        wasThisFunctionCalled = true;

        // Initialize the GROSWrappers library:
        retVal = osWrappersInitFunc();

        // Get the osTransferableObjectCreatorsManager single instance:
        osTransferableObjectCreatorsManager& theTransfetableObsCreatorsManager = osTransferableObjectCreatorsManager::instance();

        // --------------- Parameter class -------------------

        osTransferableObjectCreator<apVectorParameter> apVectorParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apVectorParameterCreator);

        osTransferableObjectCreator<apMatrixParameter> apMatrixParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apMatrixParameterCreator);

        osTransferableObjectCreator<apNotAvailableParameter> apNotAvailableParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apNotAvailableParameterCreator);

        osTransferableObjectCreator<apNotSupportedParameter> apNotSupportedParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apNotSupportedParameterCreator);

        osTransferableObjectCreator<apRemovedParameter> apRemovedParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apRemovedParameterCreator);

        osTransferableObjectCreator<apAssociatedTextureNamesPseudoParameter> apAssciaterTextureNamesPseudoParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apAssciaterTextureNamesPseudoParameterCreator);

        osTransferableObjectCreator<apAssociatedProgramNamePseudoParameter> apAssociatedProgramNamePseudoParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apAssociatedProgramNamePseudoParameterCreator);

        osTransferableObjectCreator<apAssociatedShaderNamePseudoParameter> apAssociatedShaderNamePseudoParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apAssociatedShaderNamePseudoParameterCreator);

        osTransferableObjectCreator<apFloatParameter> apFloatParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apFloatParameterCreator);

        osTransferableObjectCreator<apIntParameter> apIntParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apIntParameterCreator);

        osTransferableObjectCreator<apUnsignedIntParameter> apUnsigendIntParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apUnsigendIntParameterCreator);

        osTransferableObjectCreator<apSizeTParameter> apSizeTParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apSizeTParameterCreator);

        osTransferableObjectCreator<apBytesSizeParameter> apBytesSizeParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apBytesSizeParameterCreator);

        osTransferableObjectCreator<apStringParameter> apStringParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apStringParameterCreator);

        osTransferableObjectCreator<apGLenumParameter> apGLenumParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLenumParameterCreator);

        osTransferableObjectCreator<apGLPixelInternalFormatParameter> apGLPixelInternalFormatParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLPixelInternalFormatParameterCreator);

        osTransferableObjectCreator<apGLPrimitiveTypeParameter> apGLPrimitiveTypeCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLPrimitiveTypeCreator);

        osTransferableObjectCreator<apGLbooleanParameter> apGLbooleanParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLbooleanParameterCreator);

        osTransferableObjectCreator<apGLbitfieldParameter> apGLbitfieldParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLbitfieldParameterCreator);

        osTransferableObjectCreator<apGLclearBitfieldParameter> apGLclearBitfieldParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLclearBitfieldParameterCreator);

        osTransferableObjectCreator<apGLbyteParameter> apGLbyteParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLbyteParameterCreator);

        osTransferableObjectCreator<apGLshortParameter> apGLshortParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLshortParameterCreator);

        osTransferableObjectCreator<apGLintParameter> apGLintParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLintParameterCreator);

        osTransferableObjectCreator<apGLint64Parameter> apInt64ParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apInt64ParameterCreator);

        osTransferableObjectCreator<apGLuint64Parameter> apUInt64ParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apUInt64ParameterCreator);

        osTransferableObjectCreator<apGLuint64AddressParameter> apUInt64AddressParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apUInt64AddressParameterCreator);

        osTransferableObjectCreator<apGLubyteParameter> apGLubyteParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLubyteParameterCreator);

        osTransferableObjectCreator<apGLushortParameter> apGLushortParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLushortParameterCreator);

        osTransferableObjectCreator<apGLuintParameter> apGLuintParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLuintParameterCreator);

        osTransferableObjectCreator<apGLsizeiParameter> apGLsizeiParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLsizeiParameterCreator);

        osTransferableObjectCreator<apGLfloatParameter> apGLfloatParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLfloatParameterCreator);

        osTransferableObjectCreator<apGLclampfParameter> apGLclampfParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLclampfParameterCreator);

        osTransferableObjectCreator<apGLdoubleParameter> apGLdoubleParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLdoubleParameterCreator);

        osTransferableObjectCreator<apGLclampdParameter> apGLclampdParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLclampdParameterCreator);

        osTransferableObjectCreator<apGLintptrParameter> apGLintptrParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLintptrParameterCreator);

        osTransferableObjectCreator<apGLsizeiptrParameter> apGLsizeiptrParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLsizeiptrParameterCreator);

        osTransferableObjectCreator<apGLStringParameter> apGLStringParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLStringParameterCreator);

        osTransferableObjectCreator<apGLMultiStringParameter> apGLMultiStringParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLMultiStringParameterCreator);

        osTransferableObjectCreator<apGLsyncParameter> apGLSyncParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLSyncParameterCreator);

        // OpenGL ES is currently supported on Windows and Mac only:
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

        osTransferableObjectCreator<apGLfixedParameter> apGLfixedParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLfixedParameterCreator);

        osTransferableObjectCreator<apGLclampxParameter> apGLclampxParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLclampxParameterCreator);

#endif

        // OpenCL is not supported on the iPhone platform:
#ifndef _GR_IPHONE_BUILD
        // OpenCL parameters:
        osTransferableObjectCreator<apCLucharParameter> apCLucharParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apCLucharParameterCreator);

        osTransferableObjectCreator<apCLcharParameter> apCLcharParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apCLcharParameterCreator);

        osTransferableObjectCreator<apCLuintParameter> apCLuintParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apCLuintParameterCreator);

        osTransferableObjectCreator<apCLintParameter> apCLintParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apCLintParameterCreator);

        osTransferableObjectCreator<apCLulongParameter> apCLulongParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apCLulongParameterCreator);

        osTransferableObjectCreator<apCLlongParameter> apCLlongParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apCLlongParameterCreator);

        osTransferableObjectCreator<apCLBoolParameter> apCLBoolParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apCLBoolParameterCreator);

        osTransferableObjectCreator<apCLMemFlags> apCLMemFlagsCreator;
        theTransfetableObsCreatorsManager.registerCreator(apCLMemFlagsCreator);

        osTransferableObjectCreator<apCLSVMMemFlagsParameter> apCLSVMMemFlagsParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apCLSVMMemFlagsParameterCreator);

        osTransferableObjectCreator<apCLMemoryMigrationFlagsParameter> apCLMemoryMigrationFlagsParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apCLMemoryMigrationFlagsParameterCreator);

        osTransferableObjectCreator<apCLDeviceAffinityDomainParameter> apCLDeviceAffinityDomainParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apCLDeviceAffinityDomainParameterCreator);

        osTransferableObjectCreator<apCLDeviceSVMCapabilitiesParameter> apCLDeviceSVMCapabilitiesParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apCLDeviceSVMCapabilitiesParameterCreator);

        osTransferableObjectCreator<apCLKernelArgTypeQualifierParameter> apCLKernelArgTypeQualifierParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apCLKernelArgTypeQualifierParameterCreator);

        osTransferableObjectCreator<apCLImageDescriptionParameter> apCLImageDescriptionParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apCLImageDescriptionParameterCreator);

        osTransferableObjectCreator<apCLContextPropertyListParameter> apCLContextPropertyListParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apCLContextPropertyListParameterCreator);

        osTransferableObjectCreator<apCLCommandQueuePropertyListParameter> apCLCommandQueuePropertyListParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apCLCommandQueuePropertyListParameterCreator);

        osTransferableObjectCreator<apCLPipePropertyListParameter> apCLPipePropertyListParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apCLPipePropertyListParameterCreator);

        osTransferableObjectCreator<apCLSamplerPropertyListParameter> apCLSamplerPropertyListParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apCLSamplerPropertyListParameterCreator);

        osTransferableObjectCreator<apCLGLObjectTypeParameter> apCLGLObjectTypeParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apCLGLObjectTypeParameterCreator);

        osTransferableObjectCreator<apCLGLTextureInfoParameter> apCLGLTextureInfoParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apCLGLTextureInfoParameterCreator);

        osTransferableObjectCreator<apCLMultiStringParameter> apCLMultiStringParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apCLMultiStringParameterCreator);

        // Register pointers to OpenCL types:
        apPointerParameterCreator pointerCreatorToPointerToCLUint(OS_TOBJ_ID_CL_P_UINT_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToPointerToCLUint);

        apPointerParameterCreator pointerCreatorToPointerToCLint(OS_TOBJ_ID_CL_P_INT_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToPointerToCLint);

        osTransferableObjectCreator<apCLHandleParameter> apCLHandleParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apCLHandleParameterCreator);

        osTransferableObjectCreator<apCLDeviceTypeParameter> apCLDeviceTypeParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apCLDeviceTypeParameterCreator);

        osTransferableObjectCreator<apCLDeviceExecutionCapabilitiesParameter> apCLDeviceExecutionCapabilitiesParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apCLDeviceExecutionCapabilitiesParameterCreator);

        osTransferableObjectCreator<apCLDeviceFloatingPointConfigParameter> apCLDeviceFloatingPointConfigParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apCLDeviceFloatingPointConfigParameterCreator);

        osTransferableObjectCreator<apCLCommandQueuePropertiesParameter> apCLCommandQueuePropertiesParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apCLCommandQueuePropertiesParameterCreator);

        osTransferableObjectCreator<apCLMapFlagsParameter> apCLMapFlagsParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apCLMapFlagsParameterCreator);

        osTransferableObjectCreator<apCLEnumParameter> apCLEnumParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apCLEnumParameterCreator);

#endif // ndef _GR_IPHONE_BUILD

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        // Windows only types:
        osTransferableObjectCreator<apWin32BOOLParameter> apWin32BOOLParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apWin32BOOLParameterCreator);

        osTransferableObjectCreator<apWin32UINTParameter> apWin32UINTParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apWin32UINTParameterCreator);

        osTransferableObjectCreator<apWin32INTParameter> apWin32INTParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apWin32INTParameterCreator);

        osTransferableObjectCreator<apWin32FLOATParameter> apWin32FLOATParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apWin32FLOATParameterCreator);

        osTransferableObjectCreator<apWin32DWORDParameter> apWin32DWORDParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apWin32DWORDParameterCreator);

        osTransferableObjectCreator<apWin32HANDLEParameter> apWin32HANDLEParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apWin32HANDLEParameterCreator);

#elif (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT)

        // Linux generic variant only classes:
        osTransferableObjectCreator<apX11BoolParamter> apX11BoolParamterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apX11BoolParamterCreator);

        osTransferableObjectCreator<apXOrgCARD32Paramter> apXOrgCARD32ParamterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apXOrgCARD32ParamterCreator);

        osTransferableObjectCreator<apXIDParamter> apXIDParamterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apXIDParamterCreator);

        osTransferableObjectCreator<apLongBitfieldParameter> apLongBitfieldParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apLongBitfieldParameterCreator);

        osTransferableObjectCreator<apGLXenumParameter> apGLXenumParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLXenumParameterCreator);

#elif (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)

        // Mac only classes:
        osTransferableObjectCreator<apEAGLRenderingAPIParameter> apEAGLRenderingAPIParameterCreator;
        theTransfetableObsCreatorsManager.registerCreator(apEAGLRenderingAPIParameterCreator);

#endif // AMDT_BUILD_TARGET

        // --------------- Pointer parameter class -------------------

        apPointerParameterCreator pointerCreatorForPointerParameter(OS_TOBJ_ID_POINTER_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorForPointerParameter);

        apPointerParameterCreator pointerCreatorForPointerToPointerParameter(OS_TOBJ_ID_POINTER_TO_POINTER_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorForPointerToPointerParameter);

        apPointerParameterCreator pointerCreatorToCharParameter(OS_TOBJ_ID_P_CHAR_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToCharParameter);

        apPointerParameterCreator pointerCreatorTopCharParameter(OS_TOBJ_ID_PP_CHAR_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorTopCharParameter);

        apPointerParameterCreator pointerCreatorToIntParameter(OS_TOBJ_ID_P_INT_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToIntParameter);

        apPointerParameterCreator pointerCreatorToUIntParameter(OS_TOBJ_ID_P_UINT_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToUIntParameter);

        apPointerParameterCreator pointerCreatorToSizeTParameter(OS_TOBJ_ID_P_SIZE_T_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToSizeTParameter);

        apPointerParameterCreator pointerCreatorToSizeInBytesParameter(OS_TOBJ_ID_P_BYTES_SIZE_T_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToSizeInBytesParameter);

        apPointerParameterCreator pointerCreatorToVoidPointerParameter(OS_TOBJ_ID_PP_VOID_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToVoidPointerParameter);

        apPointerParameterCreator pointerCreatorToGLIntParameter(OS_TOBJ_ID_GL_P_INT_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToGLIntParameter);

        apPointerParameterCreator pointerCreatorToGLInt64Parameter(OS_TOBJ_ID_GL_P_INT_64_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToGLInt64Parameter);

        apPointerParameterCreator pointerCreatorToGLUInt64Parameter(OS_TOBJ_ID_GL_P_UINT_64_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToGLUInt64Parameter);

        apPointerParameterCreator pointerCreatorToBOOLParameter(OS_TOBJ_ID_GL_P_BOOL_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToBOOLParameter);

        apPointerParameterCreator pointerCreatorToVoidParameter(OS_TOBJ_ID_GL_P_VOID_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToVoidParameter);

        apPointerParameterCreator pointerCreatorToByteParameter(OS_TOBJ_ID_GL_P_BYTE_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToByteParameter);

        apPointerParameterCreator pointerCreatorToShortParameter(OS_TOBJ_ID_GL_P_SHORT_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToShortParameter);

        apPointerParameterCreator pointerCreatorToGLubyteParameter(OS_TOBJ_ID_GL_P_UBYTE_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToGLubyteParameter);

        apPointerParameterCreator pointerCreatorToGLushortParameter(OS_TOBJ_ID_GL_P_USHORT_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToGLushortParameter);

        apPointerParameterCreator pointerCreatorToGLuintParameter(OS_TOBJ_ID_GL_P_UINT_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToGLuintParameter);

        apPointerParameterCreator pointerCreatorToGLfloatParameter(OS_TOBJ_ID_GL_P_FLOAT_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToGLfloatParameter);

        apPointerParameterCreator pointerCreatorToGLclampfParameter(OS_TOBJ_ID_GL_P_CLAMPF_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToGLclampfParameter);

        apPointerParameterCreator pointerCreatorToGLdoubleParameter(OS_TOBJ_ID_GL_P_DOUBLE_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToGLdoubleParameter);

        apPointerParameterCreator pointerCreatorToGLVoidPointerParameter(OS_TOBJ_ID_GL_PP_VOID_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToGLVoidPointerParameter);

        apPointerParameterCreator pointerCreatorToGLsizei(OS_TOBJ_ID_GL_P_SIZEI_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToGLsizei);

        apPointerParameterCreator pointerCreatorToGLsizeiptr(OS_TOBJ_ID_GL_P_SIZEIPTR_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToGLsizeiptr);

        apPointerParameterCreator pointerCreatorToGLintptr(OS_TOBJ_ID_GL_P_INTPTR_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToGLintptr);

        apPointerParameterCreator pointerCreatorToGLEnum(OS_TOBJ_ID_GL_P_ENUM_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToGLEnum);

        apPointerParameterCreator pointerCreatorToPointerToGLchar(OS_TOBJ_ID_GL_P_CHAR);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToPointerToGLchar);

        apPointerParameterCreator pointerCreatorToPointerTopGLchar(OS_TOBJ_ID_GL_PP_CHAR);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToPointerTopGLchar);

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        // CGL types are irrelevant on the iPhone:
#ifndef _GR_IPHONE_BUILD
        osTransferableObjectCreator<apCGLPixelFormatAttributeParameter> pointerCreatorToCGLPixelFormatAttributeParameter;
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToCGLPixelFormatAttributeParameter);

        osTransferableObjectCreator<apCGLContextEnableParameter> pointerCreatorToCGLContextEnableParameter;
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToCGLContextEnableParameter);

        osTransferableObjectCreator<apCGLContextParameterParameter> pointerCreatorToCGLContextParameter;
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToCGLContextParameter);

        osTransferableObjectCreator<apCGLGlobalOptionParameter> pointerCreatorToCGLGlobalOptionParameter;
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToCGLGlobalOptionParameter);

        osTransferableObjectCreator<apCGLRendererPropertyParameter> pointerCreatorToCGLRendererPropertyParameter;
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToCGLRendererPropertyParameter);

        osTransferableObjectCreator<apCGLBufferModeMaskParameter> pointerCreatorToCGLBufferModeMaskParameter;
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToCGLBufferModeMaskParameter);

        osTransferableObjectCreator<apCGLColorBufferFormatMaskParameter> pointerCreatorToCGLBufferFormatMaskParameter;
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToCGLBufferFormatMaskParameter);

        osTransferableObjectCreator<apCGLRendererIDParameter> pointerCreatorToCGLRendererIDParameter;
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToCGLRendererIDParameter);

        osTransferableObjectCreator<apCGLSamplingModeMaskParameter> pointerCreatorToCGLSamplingModeMaskParameter;
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToCGLSamplingModeMaskParameter);

        osTransferableObjectCreator<apCGLStencilAndDepthModeMaskParameter> pointerCreatorToCGLStencilAndDepthModeMaskParameter;
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToCGLStencilAndDepthModeMaskParameter);
#endif // _GR_IPHONE_BUILD
#endif

        apPointerParameterCreator pointerCreatorToGLclampxParameter(OS_TOBJ_ID_GL_P_CLAMPX_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToGLclampxParameter);

        apPointerParameterCreator pointerCreatorToGLfixedParameter(OS_TOBJ_ID_GL_P_FIXED_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToGLfixedParameter);

        apPointerParameterCreator pointerCreatorToWin32FLOATParameter(OS_TOBJ_ID_P_WIN32_FLOAT_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToWin32FLOATParameter);

        apPointerParameterCreator pointerCreatorToWin32UINTParameter(OS_TOBJ_ID_P_WIN32_UINT_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToWin32UINTParameter);

        apPointerParameterCreator pointerCreatorToWin32INTParameter(OS_TOBJ_ID_P_WIN32_INT_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToWin32INTParameter);

        apPointerParameterCreator pointerCreatorToWin32HDCParameter(OS_TOBJ_ID_WIN32_HDC_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToWin32HDCParameter);

        apPointerParameterCreator pointerCreatorToWin32HGLRCParameter(OS_TOBJ_ID_WIN32_HGLRC_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToWin32HGLRCParameter);

        apPointerParameterCreator pointerCreatorToWin32HPBUFFERARBParameter(OS_TOBJ_ID_GL_HPBUFFERARB);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToWin32HPBUFFERARBParameter);

        apPointerParameterCreator pointerCreatorToWin32PixelFormatLPParameter(OS_TOBJ_ID_LP_PIXELFORMATDESCRIPTOR_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToWin32PixelFormatLPParameter);

        apPointerParameterCreator pointerCreatorToWin32PixelFormatPParameter(OS_TOBJ_ID_P_PIXELFORMATDESCRIPTOR_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToWin32PixelFormatPParameter);

        apPointerParameterCreator pointerCreatorToWin32LayerPlaneParameter(OS_TOBJ_ID_LP_LAYERPLANEDESCRIPTOR_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToWin32LayerPlaneParameter);

        apPointerParameterCreator pointerCreatorToWin32GlympParameter(OS_TOBJ_ID_LP_GLYPHMETRICSFLOAT_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToWin32GlympParameter);

        apPointerParameterCreator pointerCreatorToWin32CSTRParameter(OS_TOBJ_ID_LP_CSTR_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToWin32CSTRParameter);

        apPointerParameterCreator pointerCreatorToWin32WGLSwapParameter(OS_TOBJ_ID_P_WGLSWAP_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToWin32WGLSwapParameter);

        apPointerParameterCreator pointerCreatorToWin32ColorrefParameter(OS_TOBJ_ID_P_COLORREF_PARAMETER);
        theTransfetableObsCreatorsManager.registerCreator(pointerCreatorToWin32ColorrefParameter);

        // --------------- Other ApiClasses class -------------------

        osTransferableObjectCreator<apFunctionCall> apFunctionCallCreator;
        theTransfetableObsCreatorsManager.registerCreator(apFunctionCallCreator);

        osTransferableObjectCreator<apApiFunctionsInitializationData> apApiFunctionsInitializationDataCreator;
        theTransfetableObsCreatorsManager.registerCreator(apApiFunctionsInitializationDataCreator);

        osTransferableObjectCreator<apKernelFunctionNameBreakpoint> apKernelFunctionNameBreakpointCreator;
        theTransfetableObsCreatorsManager.registerCreator(apKernelFunctionNameBreakpointCreator);

        osTransferableObjectCreator<apKernelSourceCodeBreakpoint> apKernelSourceCodeBreakpointCreator;
        theTransfetableObsCreatorsManager.registerCreator(apKernelSourceCodeBreakpointCreator);

        osTransferableObjectCreator<apHostSourceCodeBreakpoint> apHostSourceCodeBreakpointCreator;
        theTransfetableObsCreatorsManager.registerCreator(apHostSourceCodeBreakpointCreator);

        osTransferableObjectCreator<apSourceCodeBreakpoint> apSourceCodeBreakpointCreator;
        theTransfetableObsCreatorsManager.registerCreator(apSourceCodeBreakpointCreator);

        osTransferableObjectCreator<apMonitoredFunctionBreakPoint> apMonitoredFunctionBreakPointCreator;
        theTransfetableObsCreatorsManager.registerCreator(apMonitoredFunctionBreakPointCreator);

        osTransferableObjectCreator<apGenericBreakpoint> apGenericBreakpointCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGenericBreakpointCreator);

        osTransferableObjectCreator<ap2DPoint> ap2DPointCreator;
        theTransfetableObsCreatorsManager.registerCreator(ap2DPointCreator);

        osTransferableObjectCreator<ap2DRectangle> ap2DRectangleCreator;
        theTransfetableObsCreatorsManager.registerCreator(ap2DRectangleCreator);

        osTransferableObjectCreator<apExpression> apExpressionCreator;
        theTransfetableObsCreatorsManager.registerCreator(apExpressionCreator);

        osTransferableObjectCreator<apGLComputeShader> apGLComputeShaderCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLComputeShaderCreator);

        osTransferableObjectCreator<apGLTexture> apGLTextureCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLTextureCreator);

        osTransferableObjectCreator<apGLSampler> apGLSamplerCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLSamplerCreator);

        osTransferableObjectCreator<apGLFBO> apGLFBOCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLFBOCreator);

        osTransferableObjectCreator<apGLProgram> apGLProgramCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLProgramCreator);

        osTransferableObjectCreator<apGLPipeline> apGLPipelineCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLPipelineCreator);

        osTransferableObjectCreator<apGLTessellationControlShader> apGLTessellationControlShaderCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLTessellationControlShaderCreator);

        osTransferableObjectCreator<apGLTessellationEvaluationShader> apGLTessellationEvaluationShaderCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLTessellationEvaluationShaderCreator);

        osTransferableObjectCreator<apGLVertexShader> apGLVertexShaderCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLVertexShaderCreator);

        osTransferableObjectCreator<apGLGeometryShader> apGLGeometryShaderCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLGeometryShaderCreator);

        osTransferableObjectCreator<apGLFragmentShader> apGLFragmentShaderCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLFragmentShaderCreator);

        osTransferableObjectCreator<apGLItemsCollection> apGLItemsCollectionCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLItemsCollectionCreator);

        osTransferableObjectCreator<apGLUnsupportedShader> apGLUnsupportedShaderCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLUnsupportedShaderCreator);

        osTransferableObjectCreator<apGLTransformFeedbackObject> apGLTransformFeedbackObjectCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGLTransformFeedbackObjectCreator);

        osTransferableObjectCreator<apCounterInfo> apCounterInfoCreator;
        theTransfetableObsCreatorsManager.registerCreator(apCounterInfoCreator);

        osTransferableObjectCreator<apDetectedErrorParameters> apDetectedErrorParametersCreator;
        theTransfetableObsCreatorsManager.registerCreator(apDetectedErrorParametersCreator);

        osTransferableObjectCreator<apOpenCLErrorParameters> apOpenCLErrorParametersCreator;
        theTransfetableObsCreatorsManager.registerCreator(apOpenCLErrorParametersCreator);

        osTransferableObjectCreator<apDebugProjectSettings> apDebugProjectSettingsCreator;
        theTransfetableObsCreatorsManager.registerCreator(apDebugProjectSettingsCreator);


        // --------------- APIClasses events -------------------

        osTransferableObjectCreator<apAfterKernelDebuggingEvent> apAfterKernelDebuggingEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apAfterKernelDebuggingEventCreator);

        osTransferableObjectCreator<apApiConnectionEstablishedEvent> apApiConnectionEstablishedEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apApiConnectionEstablishedEventCreator);

        osTransferableObjectCreator<apApiConnectionEndedEvent> apApiConnectionEndedEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apApiConnectionEndedEventCreator);

        osTransferableObjectCreator<apBeforeDebuggedProcessRunResumedEvent> apBeforeDebuggedProcessRunResumedEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apBeforeDebuggedProcessRunResumedEventCreator);

        osTransferableObjectCreator<apBeforeKernelDebuggingEvent> apBeforeKernelDebuggingEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apBeforeKernelDebuggingEventCreator);

        osTransferableObjectCreator<apCallStackFrameSelectedEvent> apCallStackFrameSelectedEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apCallStackFrameSelectedEventCreator);

        osTransferableObjectCreator<apDeferredCommandEvent> apDeferredCommandEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apDeferredCommandEventCreator);

        osTransferableObjectCreator<apDebuggedProcessIsDuringTerminationEvent> apDebuggedProcessIsDuringTerminationEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apDebuggedProcessIsDuringTerminationEventCreator);

        osTransferableObjectCreator<apContextDataSnapshotWasUpdatedEvent> apContextDataSnapshotWasUpdatedEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apContextDataSnapshotWasUpdatedEventCreator);

        osTransferableObjectCreator<apBreakpointHitEvent> apBreakpointHitEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apBreakpointHitEventCreator);

        osTransferableObjectCreator<apDebuggedProcessDetectedErrorEvent> apDebuggedProcessDetectedErrorEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apDebuggedProcessDetectedErrorEventCreator);

        osTransferableObjectCreator<apDebuggedProcessOutputStringEvent> apDebuggedProcessOutputStringEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apDebuggedProcessOutputStringEventCreator);

        osTransferableObjectCreator<apDebuggedProcessRunResumedEvent> apDebuggedProcessRunResumedEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apDebuggedProcessRunResumedEventCreator);

        osTransferableObjectCreator<apDebuggedProcessRunStartedEvent> apDebuggedProcessRunStartedEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apDebuggedProcessRunStartedEventCreator);

        osTransferableObjectCreator<apDebuggedProcessRunStartedExternallyEvent> apDebuggedProcessRunStartedExternallyEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apDebuggedProcessRunStartedExternallyEventCreator);

        osTransferableObjectCreator<apDebuggedProcessRunSuspendedEvent> apDebuggedProcessRunSuspendedEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apDebuggedProcessRunSuspendedEventCreator);

        osTransferableObjectCreator<apExceptionEvent> apExceptionEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apExceptionEventCreator);

        osTransferableObjectCreator<apGDBErrorEvent> apGDBErrorEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGDBErrorEventCreator);

        osTransferableObjectCreator<apGDBListenerThreadWasSuspendedEvent> apGDBListenerThreadWasSuspendedEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGDBListenerThreadWasSuspendedEventCreator);

        osTransferableObjectCreator<apGDBOutputStringEvent> apGDBOutputStringEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apGDBOutputStringEventCreator);

        osTransferableObjectCreator<apInfrastructureEndsBeingBusyEvent> apInfrastructureEndsBeingBusyEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apInfrastructureEndsBeingBusyEventCreator);

        osTransferableObjectCreator<apInfrastructureFailureEvent> apInfrastructureFailureEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apInfrastructureFailureEventCreator);

        osTransferableObjectCreator<apInfrastructureStartsBeingBusyEvent> apInfrastructureStartsBeingBusyEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apInfrastructureStartsBeingBusyEventCreator);

        osTransferableObjectCreator<apKernelDebuggingFailedEvent> apKernelDebuggingFailedEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apKernelDebuggingFailedEventCreator);

        osTransferableObjectCreator<apKernelDebuggingInterruptedEvent> apKernelDebuggingInterruptedEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apKernelDebuggingInterruptedEventCreator);

        osTransferableObjectCreator<apKernelSourceBreakpointsUpdatedEvent> apKernelSourceBreakpointsUpdatedEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apKernelSourceBreakpointsUpdatedEventCreator);

        osTransferableObjectCreator<apMemoryLeakEvent> apMemoryLeakEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apMemoryLeakEventCreator);

        osTransferableObjectCreator<apModuleLoadedEvent> apModuleLoadedEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apModuleLoadedEventCreator);

        osTransferableObjectCreator<apModuleUnloadedEvent> apModuleUnloadedEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apModuleUnloadedEventCreator);

        osTransferableObjectCreator<apOutputDebugStringEvent> apOutputDebugStringEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apOutputDebugStringEventCreator);

        osTransferableObjectCreator<apUserWarningEvent> apUserWarningEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apUserWarningEventCreator);

        osTransferableObjectCreator<apDebuggedProcessCreatedEvent> apDebuggedProcessCreatedEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apDebuggedProcessCreatedEventCreator);

        osTransferableObjectCreator<apDebuggedProcessTerminatedEvent> apDebuggedProcessTerminatedEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apDebuggedProcessTerminatedEventCreator);

        osTransferableObjectCreator<apDebuggedProcessCreationFailureEvent> apDebuggedProcessCreationFailureEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apDebuggedProcessCreationFailureEventCreator);

        osTransferableObjectCreator<apSearchingForMemoryLeaksEvent> apSearchingForMemoryLeaksEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apSearchingForMemoryLeaksEventCreator);

        osTransferableObjectCreator<apSpyProgressEvent> apSpyProgressEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apSpyProgressEventCreator);

        osTransferableObjectCreator<apTechnologyMonitorFailureEvent> apTechnologyMonitorFailureEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apTechnologyMonitorFailureEventCreator);

        osTransferableObjectCreator<apThreadCreatedEvent> apThreadCreatedEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apThreadCreatedEventCreator);

        osTransferableObjectCreator<apThreadTerminatedEvent> apThreadTerminatedEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apThreadTerminatedEventCreator);

        osTransferableObjectCreator<apRenderContextCreatedEvent> apRenderContextCreatedEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apRenderContextCreatedEventCreator);

        osTransferableObjectCreator<apRenderContextDeletedEvent> apRenderContextDeletedEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apRenderContextDeletedEventCreator);

        osTransferableObjectCreator<apComputeContextCreatedEvent> apComputeContextCreatedEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apComputeContextCreatedEventCreator);

        osTransferableObjectCreator<apComputeContextDeletedEvent> apComputeContextDeletedEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apComputeContextDeletedEventCreator);

        osTransferableObjectCreator<apOpenCLErrorEvent> apOpenCLErrorEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apOpenCLErrorEventCreator);

        osTransferableObjectCreator<apOpenCLQueueCreatedEvent> apOpenCLQueueCreatedEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apOpenCLQueueCreatedEventCreator);

        osTransferableObjectCreator<apOpenCLQueueDeletedEvent> apOpenCLQueueDeletedEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apOpenCLQueueDeletedEventCreator);

        osTransferableObjectCreator<apOpenCLProgramCreatedEvent> apOpenCLProgramCreatedEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apOpenCLProgramCreatedEventCreator);

        osTransferableObjectCreator<apOpenCLProgramBuildEvent> apOpenCLProgramBuildEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apOpenCLProgramBuildEventCreator);

        osTransferableObjectCreator<apOpenCLProgramBuildFailedWithDebugFlagsEvent> apOpenCLProgramBuildFailedWithDebugFlagsEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apOpenCLProgramBuildFailedWithDebugFlagsEventCreator);

        osTransferableObjectCreator<apOpenCLProgramDeletedEvent> apOpenCLProgramDeletedEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apOpenCLProgramDeletedEventCreator);

        osTransferableObjectCreator<apGLDebugOutputMessageEvent> apapGLDebugOutputMessageEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apapGLDebugOutputMessageEventCreator);

        osTransferableObjectCreator<apFlushTextureImageEvent> apFlushTextureImageEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apFlushTextureImageEventCreator);

        osTransferableObjectCreator<apAddWatchEvent> apAddWatchEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apAddWatchEventCreator);

        osTransferableObjectCreator<apMemoryAllocationFailureEvent> apMemoryAllocationFailureEventCreator;
        theTransfetableObsCreatorsManager.registerCreator(apMemoryAllocationFailureEventCreator);

        // Uri, 2/12/09: OpenCL (ES) is not yet supported on the iPhone:
#ifndef _GR_IPHONE_BUILD
        {
            // ---- APIClasses enqueued commands classes (apCLEnqueuedCommand) ----

            osTransferableObjectCreator<apCLAcquireGLObjectsCommand> apCLAcquireGLObjectsCommandCreator;
            theTransfetableObsCreatorsManager.registerCreator(apCLAcquireGLObjectsCommandCreator);

            osTransferableObjectCreator<apCLBarrierCommand> apCLBarrierCommandCreator;
            theTransfetableObsCreatorsManager.registerCreator(apCLBarrierCommandCreator);

            osTransferableObjectCreator<apCLCopyBufferCommand> apCLCopyBufferCommandCreator;
            theTransfetableObsCreatorsManager.registerCreator(apCLCopyBufferCommandCreator);

            osTransferableObjectCreator<apCLCopyBufferRectCommand> apCLCopyBufferRectCommandCreator;
            theTransfetableObsCreatorsManager.registerCreator(apCLCopyBufferRectCommandCreator);

            osTransferableObjectCreator<apCLCopyBufferToImageCommand> apCLCopyBufferToImageCommandCreator;
            theTransfetableObsCreatorsManager.registerCreator(apCLCopyBufferToImageCommandCreator);

            osTransferableObjectCreator<apCLCopyImageCommand> apCLCopyImageCommandCreator;
            theTransfetableObsCreatorsManager.registerCreator(apCLCopyImageCommandCreator);

            osTransferableObjectCreator<apCLCopyImageToBufferCommand> apCLCopyImageToBufferCommandCreator;
            theTransfetableObsCreatorsManager.registerCreator(apCLCopyImageToBufferCommandCreator);

            osTransferableObjectCreator<apCLMapBufferCommand> apCLMapBufferCommandCreator;
            theTransfetableObsCreatorsManager.registerCreator(apCLMapBufferCommandCreator);

            osTransferableObjectCreator<apCLMapImageCommand> apCLMapImageCommandCreator;
            theTransfetableObsCreatorsManager.registerCreator(apCLMapImageCommandCreator);

            osTransferableObjectCreator<apCLMarkerCommand> apCLMarkerCommandCreator;
            theTransfetableObsCreatorsManager.registerCreator(apCLMarkerCommandCreator);

            osTransferableObjectCreator<apCLNativeKernelCommand> apCLNativeKernelCommandCreator;
            theTransfetableObsCreatorsManager.registerCreator(apCLNativeKernelCommandCreator);

            osTransferableObjectCreator<apCLNDRangeKernelCommand> apCLNDRangeKernelCommandCreator;
            theTransfetableObsCreatorsManager.registerCreator(apCLNDRangeKernelCommandCreator);

            osTransferableObjectCreator<apCLReadBufferCommand> apCLReadBufferCommandCreator;
            theTransfetableObsCreatorsManager.registerCreator(apCLReadBufferCommandCreator);

            osTransferableObjectCreator<apCLReadBufferRectCommand> apCLReadBufferRectCommandCreator;
            theTransfetableObsCreatorsManager.registerCreator(apCLReadBufferRectCommandCreator);

            osTransferableObjectCreator<apCLReadImageCommand> apCLReadImageCommandCreator;
            theTransfetableObsCreatorsManager.registerCreator(apCLReadImageCommandCreator);

            osTransferableObjectCreator<apCLReleaseGLObjectsCommand> apCLReleaseGLObjectsCommandCreator;
            theTransfetableObsCreatorsManager.registerCreator(apCLReleaseGLObjectsCommandCreator);

            osTransferableObjectCreator<apCLTaskCommand> apCLTaskCommandCreator;
            theTransfetableObsCreatorsManager.registerCreator(apCLTaskCommandCreator);

            osTransferableObjectCreator<apCLUnmapMemObjectCommand> apCLUnmapMemObjectCommandCreator;
            theTransfetableObsCreatorsManager.registerCreator(apCLUnmapMemObjectCommandCreator);

            osTransferableObjectCreator<apCLWaitForEventsCommand> apCLWaitForEventsCommandCreator;
            theTransfetableObsCreatorsManager.registerCreator(apCLWaitForEventsCommandCreator);

            osTransferableObjectCreator<apCLWriteBufferCommand> apCLWriteBufferCommandCreator;
            theTransfetableObsCreatorsManager.registerCreator(apCLWriteBufferCommandCreator);

            osTransferableObjectCreator<apCLWriteBufferRectCommand> apCLWriteBufferRectCommandCreator;
            theTransfetableObsCreatorsManager.registerCreator(apCLWriteBufferRectCommandCreator);

            osTransferableObjectCreator<apCLWriteImageCommand> apCLWriteImageCommandCreator;
            theTransfetableObsCreatorsManager.registerCreator(apCLWriteImageCommandCreator);

            osTransferableObjectCreator<apCLQueueIdle> apCLQueueIdleCreator;
            theTransfetableObsCreatorsManager.registerCreator(apCLQueueIdleCreator);

            osTransferableObjectCreator<apCLFillBufferCommand> apCLFillBufferCommandCreator;
            theTransfetableObsCreatorsManager.registerCreator(apCLFillBufferCommandCreator);

            osTransferableObjectCreator<apCLFillImageCommand> apCLFillImageCommandCreator;
            theTransfetableObsCreatorsManager.registerCreator(apCLFillImageCommandCreator);

            osTransferableObjectCreator<apCLMigrateMemObjectsCommand> apCLMigrateMemObjectsCommandCreator;
            theTransfetableObsCreatorsManager.registerCreator(apCLMigrateMemObjectsCommandCreator);

            osTransferableObjectCreator<apCLMarkerWithWaitListCommand> apCLMarkerWithWaitListCommandCreator;
            theTransfetableObsCreatorsManager.registerCreator(apCLMarkerWithWaitListCommandCreator);

            osTransferableObjectCreator<apCLBarrierWithWaitListCommand> apCLBarrierWithWaitListCommandCreator;
            theTransfetableObsCreatorsManager.registerCreator(apCLBarrierWithWaitListCommandCreator);

            osTransferableObjectCreator<apCLSVMFreeCommand> apCLSVMFreeCommandCreator;
            theTransfetableObsCreatorsManager.registerCreator(apCLSVMFreeCommandCreator);

            osTransferableObjectCreator<apCLSVMMemcpyCommand> apCLSVMMemcpyCommandCreator;
            theTransfetableObsCreatorsManager.registerCreator(apCLSVMMemcpyCommandCreator);

            osTransferableObjectCreator<apCLSVMMemFillCommand> apCLSVMMemFillCommandCreator;
            theTransfetableObsCreatorsManager.registerCreator(apCLSVMMemFillCommandCreator);

            osTransferableObjectCreator<apCLSVMMapCommand> apCLSVMMapCommandCreator;
            theTransfetableObsCreatorsManager.registerCreator(apCLSVMMapCommandCreator);

            osTransferableObjectCreator<apCLSVMUnmapCommand> apCLSVMUnmapCommandCreator;
            theTransfetableObsCreatorsManager.registerCreator(apCLSVMUnmapCommandCreator);

            // ---- APIClasses other OpenCL related classes ----
            osTransferableObjectCreator<apCLContext> apCLContextCreator;
            theTransfetableObsCreatorsManager.registerCreator(apCLContextCreator);
        }
#endif // _GR_IPHONE_BUILD
    }

    return retVal;
}


