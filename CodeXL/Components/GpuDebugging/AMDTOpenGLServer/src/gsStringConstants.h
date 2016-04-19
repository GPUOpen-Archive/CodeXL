//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsStringConstants.h
///
//==================================================================================

//------------------------------ gsStringConstants.h ------------------------------

#ifndef __GSSTRINGCONSTANTS
#define __GSSTRINGCONSTANTS

// Local:
#include <AMDTOpenGLServer/Include/gsPublicStringConstants.h>

// ------------------------------------------------------------------------
//  This file contains strings that are displayed by the OpenGL spy
// ------------------------------------------------------------------------

// General strings:
#define GS_STR_ES_PC_EmulatorProductIdString L"OpenGL ES Emulator"
#define GS_STR_OpenGLServerInitializedSuccessfully L"CodeXL OpenGL Server was initialized"
#define GS_STR_OpenGLServerIsTerminating L"CodeXL OpenGL Server is terminating"
#define GS_STR_OpenGLServerInitializationFailureMessage L"Error: CodeXL's OpenGL Server failed to initialize\nThe debugged application (%ls) will now exit"
// #define GS_STR_communicationFailedMessage L"Error: Communication between the debugged application and CodeXL has failed!\n\n- This communication is carried out using Windows sockets facilities.\n  Please check your firewall and security settings."
#define GS_STR_OpenGLESFrameworkPathNotValid L"Could not find the OpenGL ES framework at path specified by" GS_STR_envVar_openglesFrameworkPath L"environment variable. Path is "
#define GS_STR_OpenGLESFrameworkPathNotSet GS_STR_envVar_openglesFrameworkPath L" environment variable value not set"
#define GS_STR_iPhoneDeviceOpenGLESFrameworkPath L"/System/Library/Frameworks/OpenGLES.framework/"
#define GS_STR_OpenGLErrorBeforeErrorsTest L"Notice: OpenGL error was recorded before \"Break on OpenGL errors\" was turned on."
#define GS_STR_deletingNonExistingTexture L"Warning: The debugged program delete a texture that does not exist. Texture name:"
#define GS_STR_deletingNonExistingRenderBuffer L"Warning: The debugged program deleted a render buffer that does not exist. Render Buffer name:"
#define GS_STR_deletingNonExistingVBO L"Warning: The debugged program deleted a vertex buffer object that does not exist. Vertex Buffer Object name:"
#define GS_STR_bindNonExistingVBO L"Warning: The debugged program bound a vertex buffer object that does not exist. Vertex Buffer Object name:"
#define GS_STR_tryingToEnableCGLCEMPEngine L"Warning: A call to enable the multi-threaded OpenGL engine [ CGLEnable(..., kCGLCEMPEngine) ] has been issued. This engine interferes with some of CodeXL's core features, and must remain disabled for CodeXL's features to run correctly."

#define GS_STR_noTextureImagePreviewAvailable L"<b>[Context %d - Texture %u: No preview available]</b>"
#define GS_STR_beginTexturePreview L"<b>[Context %d - Texture %u: </b>"
#define GS_STR_endTexturePreview L"<b>]</b>"
#define GS_STR_texturePreview L"<a href=\"%ls\" TARGET=\"_blank\"><img src=\"%ls\" name=\"Context %d - Texture %u\" alt=\"Context %d - Texture %u\" width=\"%d\" height=\"%d\" border=\"%d\"></a>"
#define GS_STR_textureNoPreviewAvailable L"<b>No preview available</b>"
#define GS_STR_foreignExtensionCallError1 L"The debugged process asked for an extension function pointer ";
#define GS_STR_foreignExtensionCallError2 L" from one render context, but called this function pointer in another render context"
#define GS_STR_unsupportedExtensionUse L"Warning: Using an OpenGL extension function that is not supported by CodeXL's OpenGL server";
#define GS_STR_retrievingExtensionFunctionPointer L"Retrieving OpenGL extension function pointer: "
#define GS_STR_extensionCallIgnored L"The extension function call is ignored"
#define GS_STR_usingNonObjectName L"Using a non-existing OpenGL managed object";
#define GS_STR_usingNonSupportedTextureUnit L"Using a non-supported texture unit";
#define GS_STR_renderContextDeletionFailed L"Render context deletion failed";
#define GS_STR_shaderAlreadyAttached L"Shader %u is already attached to program %u"
#define GS_STR_shaderIsNotAttached L"Shader %u is not attached to program %u"
#define GS_STR_shaderCompilationFailed L"Shader %u compilation failed"
#define GS_STR_programLinkFailed L"Program %u link failed"
#define GS_STR_assiciatedProgramTextStart L"<b>[Context %d - program %u"
#define GS_STR_assiciatedProgramTextShader L"<a href=\"%ls\" TARGET=\"_blank\">shader %u</a>"
#define GS_STR_assiciatedProgramTextShaderNoSourceCode L"shader %u"
#define GS_STR_assiciatedProgramTextEnd L"]</b>"
#define GS_STR_noShaderSourceCodeAvailable L"<b>[Context %d - Shader %u: source code is not available]</b>"
#define GS_STR_shaderNameAndSourceCodeBegin L"<b>[Context %d - <a href=\"%ls\" TARGET=\"_blank\">shader %u</a>]</b>"
#define GS_STR_contextWasDeletedHTMLLog L"<br><div style=\"background-color: FF0000;\"><br><b>The render context was deleted</b><br><br></div><br>"
#define GS_STR_usingSoftwareRenderer L"Context %d is rendered using a software renderer (Vendor: %ls, Renderer: %ls)"
#define GS_STR_NonRGBAImangeUsed L"Using none RGBA image format"
#define GS_STR_FailedToSaveTextureImage L"Failed to save texture image to a file: "
#define GS_STR_SavingFile L"Saving file: "
#define GS_STR_FailedToTerminateOGLWrappers L"Failed to terminate OpenGL wrappers"
#define GS_STR_ContextWasDeleted L"Context %d was deleted from the OS"
#define GS_STR_UnsupportedStateVarGetFunction L"Error: Unsupported get function, used in state variable: "
#define GS_STR_UnkownContextHandleUsed L"Error: An unknown render context handle was used"
#define GS_STR_UnknownDisplayAttribute L"Error: An unknown display attribute was used";
#define GS_STR_UnknownBindTarget L"Error: Unknown bind target: "
#define GS_STR_oglError L"OpenGL error"
#define GS_STR_notRunningUnderDebuggedApp L"Notice: This application is not CodeXL's debugged application (debugged application = %ls, this application = %ls)"
#define GS_STR_renderContextWasCreated L"Render Context %d was created"
#define GS_STR_renderContextAboutToBeDeleted L"Attempting to delete Render Context %d"
#define GS_STR_renderContextWasDeleted L"Render Context %d was deleted"
#define GS_STR_renderContextIsCurrentForFirstTime L"Render Context %d was made current for the first time"
#define GS_STR_OGLVendor L"Vendor"
#define GS_STR_OGLRenderer L"Renderer"
#define GS_STR_OGLVersion L"OpenGL version: %d.%d"
#define GS_STR_OGLShadingLangVersion L"Shading language version"
#define GS_STR_disabled_vertical_sync_for_context L"Vertical sync for context %d was disabled by CodeXL"

// Extensions support:
#define GS_STR_GREMEDY_SUPPORTED_EXTENSION1 "GL_GREMEDY_string_marker"
#define GS_STR_GREMEDY_SUPPORTED_EXTENSION2 "GL_GREMEDY_frame_terminator"
#define GS_STR_GREMEDY_SUPPORTED_EXTENSIONS GS_STR_GREMEDY_SUPPORTED_EXTENSION1 " " GS_STR_GREMEDY_SUPPORTED_EXTENSION2

// Debug output:
#define GS_STR_previousOpenGLError L"Error state before attempting debug message operation: "
#define GS_STR_settingDebugMessageCallback L"Setting debug message callback. Params: "
#define GS_STR_errorWhileSettingDebugMessageCallback L"Error while setting debug message callback. Params -> error: "
#define GS_STR_settingDebugMessageFilter L"Setting debug message filter. Params: "
#define GS_STR_errorWhileSettingDebugMessageFilter L"Error while setting debug message filter. Params -> error: "
#define GS_STR_GLDebugOutputMaxPrintoutsReached L"Maximal amount of debug output reports reached (%d reports). Additional debug output reports will be ignored."

// ATI Performance counters integration:
#define GS_STR_initializingATICounters L"Initializing ATI performance counters support..."
#define GS_STR_ATICountersAreNotSupported L"The installed hardware does not support the ATI performance counters"
#define GS_STR_ATICountersInitialized L"ATI performance counters support was initialized successfully"
#define GS_STR_ATICountersInitializationFailed L"Failed to initialize the ATI performance counters support"
#define GS_STR_ATICountersSamplingUnknownState L"ATI performance counters sampling failure: unknown state"
#define GS_STR_ATICountersSamplingUnknownDataType L"ATI performance counters sampling failure: unknown counter data type"
#define GS_STR_ATICountersSamplingCounterValueFailure L"ATI performance counters sampling failure: could not get counter value"
#define GS_STR_ATICountersSamplingNotStartFailure L"ATI performance counters sampling failure: Trying to end pass and sampling did not start"

// iPhone performance counters:
#define GS_STR_UNKNOWN_COUNTER_PART1 L"The iPhone driver supports a counter ("
#define GS_STR_UNKNOWN_COUNTER_PART2 L") that is not yet supported by CodeXL"
#define GS_STR_FAILED_TO_UPDATE_PERF_COUNTER L"Failed to update performance counter %ls"

// Log message printouts:
#define GS_STR_DebugLog_OpenGLServerInitializing L"The OpenGL Server is initializing"
#define GS_STR_DebugLog_OpenGLServerInitialionEnded L"The OpenGL Server initialization ended"
#define GS_STR_DebugLog_startedHandlingAPIInitCalls L"Started handling OpenGL Server API initialization calls"
#define GS_STR_DebugLog_endedHandlingAPIInitCalls L"Ended handling OpenGL Server API initialization calls"
#define GS_STR_DebugLog_OpenGLServerInitializationFailed L"CodeXL's OpenGL Server initialization has failed. The debugged application will now exit"
#define GS_STR_DebugLog_MainThreadWaitsForAPIThreadToHandleOGLAPIInitialization L"The main thread waits for OpenGL Server API initililization, handled by the API thread"
#define GS_STR_DebugLog_MainThreadFinishedWaitingForAPIThreadToHandleOGLAPIInitialization L"The main thread finished waiting for OpenGL Server API initililization, handled by the API thread"
#define GS_STR_DebugLog_UpdateCurrentThreadCtxSnapshotStart L"gaUpdateCurrentThreadRenderContextDataSnapshotStub (ran by current context thread) started"
#define GS_STR_DebugLog_UpdateCurrentThreadCtxSnapshotEnd L"gaUpdateCurrentThreadRenderContextDataSnapshotStub (ran by current context thread) ended"
#define GS_STR_DebugLog_UpdateContextDataSnapshotStart L"gaUpdateContextDataSnapshotStub (ran by API thread) started"
#define GS_STR_DebugLog_UpdateContextDataSnapshotEnd L"gaUpdateContextDataSnapshotStub (ran by API thread) ended"
#define GS_STR_DebugLog_UpdateTextureDataStarted L"Textures data update started"
#define GS_STR_DebugLog_UpdateTextureDataEnded L"Textures data update ended"
#define GS_STR_DebugLog_UpdateRenderBufferDataStarted L"Render buffers data update started"
#define GS_STR_DebugLog_UpdateRenderBufferDataEnded L"Render buffers data update ended"
#define GS_STR_DebugLog_UpdateVBODataStarted L"VBO data update started"
#define GS_STR_DebugLog_UpdateVBODataEnded L"VBO data update ended"
#define GS_STR_DebugLog_UpdateBuffersDataStarted L"Buffers data update started"
#define GS_STR_DebugLog_UpdateBuffersDataEnded L"Buffers data update ended"
#define GS_STR_DebugLog_UpdateStateVariablesDataStarted L"State variables data update started"
#define GS_STR_DebugLog_UpdateStateVariablesDataEnded L"State variables data update ended"
#define GS_STR_DebugLog_UpdateStateVariablesFilteredDataEnded L"State variables data update ended (filtered)"
#define GS_STR_DebugLog_UpdateProgramsAnsShadersDataStarted L"Programs and Shaders data update started"
#define GS_STR_DebugLog_UpdateProgramsAnsShadersDataEnded L"Programs and Shaders data update ended"
#define GS_STR_DebugLog_UpdateProgramsUniformsDataStarted L"Programs uniforms data update started"
#define GS_STR_DebugLog_UpdateProgramsUniformsDataEnded L"Programs uniforms data update ended"
#define GS_STR_DebugLog_StaticBufferRawDataStarted L"Static buffer raw data update started"
#define GS_STR_DebugLog_StaticBufferRawDataEnded L"Static buffer raw data update ended"
#define GS_STR_DebugLog_PBufferRawDataStarted L"PBuffer raw data update started"
#define GS_STR_DebugLog_PBufferRawDataEnded L"PBuffer raw data update ended"
#define GS_STR_DebugLog_TextureRawDataStarted L"Texture raw data update started"
#define GS_STR_DebugLog_TextureRawDataEnded L"Texture raw data update ended"
#define GS_STR_DebugLog_SetShaderSourceCodeStarted L"Set shader source code started"
#define GS_STR_DebugLog_SetShaderSourceCodeEnded L"Set shader source code ended"
#define GS_STR_DebugLog_UpdatingWhileInBeginEndBlock L"Updating context data snapshot while in glBegin - glEnd block. Context data snapshot is cleared"
#define GS_STR_DebugLog_loadingSystemOGLServer L"Loading system's OpenGL module: "
#define GS_STR_DebugLog_systemOGLServerLoadedOk L"System's OpenGL module was loaded successfully from: "
#define GS_STR_DebugLog_systemOGLServerLoadFailed L"Error: Failed to load the system's OpenGL module"
#define GS_STR_DebugLog_wrappingSystemOGLFunctions L"Wrapping system's OpenGL module functions"
#define GS_STR_DebugLog_wrappingSystemOGLFunctionsEnded L"Finished wrapping system's OpenGL module functions"
#define GS_STR_DebugLog_wrappingSystemCGLFunctions L"Wrapping system's CGL functions"
#define GS_STR_DebugLog_wrappingSystemCGLFunctionsEnded L"Finished wrapping system's CGL functions"
#define GS_STR_DebugLog_cannotGetOGLFuncPtr L"Error: Cannot retrieve OpenGL function pointer: "
#define GS_STR_DebugLog_gaBeforeDirectAPIFunctionExecutionStarted L"gaBeforeDirectAPIFunctionExecutionStub started"
#define GS_STR_DebugLog_gaBeforeDirectAPIFunctionExecutionEnded L"gaBeforeDirectAPIFunctionExecutionStub ended"
#define GS_STR_DebugLog_gaBeforeDirectAPIFunctionExecutionDuringBreakStubStarted L"gaBeforeDirectAPIFunctionExecutionDuringBreakStub started"
#define GS_STR_DebugLog_gaBeforeDirectAPIFunctionExecutionDuringBreakEnded L"gaBeforeDirectAPIFunctionExecutionDuringBreakStub ended"
#define GS_STR_DebugLog_overridingCallToCGLEnablekCGLCEMPEngine L"Overriding call to CGLEnable(..., kCGLCEMPEngine)."
#define GS_STR_DebugLog_directDrawStart L"Initializing DirectDraw"
#define GS_STR_DebugLog_directDrawEnd L"Done Initializing DirectDraw"
#define GS_STR_DebugLog_directDrawDidNotInitialize L"Skipped DirectDraw library initialization"
#define GS_STR_DebugLog_directDrawBeforeDirectDrawCreate L"Before DirectDrawCreate"
#define GS_STR_DebugLog_directDrawAfterDirectDrawCreate L"After DirectDrawCreate"
#define GS_STR_DebugLog_directDrawCheckDisplayMonitor L"Determining DirectDraw support by device"
#define GS_STR_DebugLog_directDrawIgnoreATI L"Ignoring DirectDraw initialization setting, ATI device detected."
#define GS_STR_DebugLog_aboutToReadCLObject L"About to read OpenGL-OpenCL interop object data."

// Deprecation:
#define GS_STR_DeprecationInternalFormatArgName L"internalformat"
#define GS_STR_DeprecationFormatArgName L"format"
#define GS_STR_DeprecationCapArgName L"cap"
#define GS_STR_DeprecationCoordArgName L"coord"
#define GS_STR_DeprecationTargetArgName L"target"
#define GS_STR_DeprecationPNameArgName L"pname"
#define GS_STR_DeprecationNameArgName L"name"
#define GS_STR_DeprecationModeArgName L"mode"
#define GS_STR_DeprecationParamArgName L"param"
#define GS_STR_DeprecationTypeArgName L"type"
#define GS_STR_DeprecationWidthName L"width"
#define GS_STR_DeprecationBorderName L"border"
#define GS_STR_DeprecationMaskName L"mask"

// File names:
#define GS_STR_callsLogFilePath L"OpenGLContext%d-CallsLog"
#define GS_STR_shaderFilePath L"OpenGLContext%d-Shader%.3u-%ls"
#define GS_STR_textureFilePath L"OpenGLContext%d-Texture%u-Level%d-%ls"
#define GS_STR_vboFilePath L"OpenGLContext%d-VBO%u"
#define GS_STR_renderBufferFilePath L"OpenGLContext%d-RenderBuffer%u"
#define GS_STR_pbufferFilePath L"OpenGLContext%d-PBuffer%d-%ls"
#define GS_STR_staticBufferFilePath L"OpenGLContext%d-Static-%ls"

// Render Context Calls History logger:
#define GS_STR_RenderContextCallsHistoryLoggerMessagesLabelFormat L"OpenGL Render Context %d: "

// State variable reader:
#define GS_STR_StateVariableReaderMessageBefore L"Attempting to get State variable "
#define GS_STR_StateVariableReaderMessageBeforeDetails L", glGet* function id %d, GLenum value %#x"
#define GS_STR_StateVariableReaderMessageBeforeAdditional L", additional GLenum %#x"
#define GS_STR_StateVariableReaderMessageBeforeDims L", variable dimensions %dx%d"
#define GS_STR_StateVariableReaderMessageAfterTrue L"Attempt to get state variable completed with"
#define GS_STR_StateVariableReaderMessageAfterFalse L"Error in attempt to get state variable."
#define GS_STR_StateVariableReaderMessageAfterGLError L" OpenGL error: "
#define GS_STR_StateVariableReaderMessageAfterSuccess L" success."

#endif  // __GSSTRINGCONSTANTS
