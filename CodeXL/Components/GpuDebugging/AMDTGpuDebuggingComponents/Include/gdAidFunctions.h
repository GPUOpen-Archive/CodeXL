//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdAidFunctions.h
///
//==================================================================================

//------------------------------ gdAidFunctions.h ------------------------------

#ifndef __GDAIDFUNCTIONS
#define __GDAIDFUNCTIONS

// Qt:
#include <QtWidgets>

// Forward deceleration:
class gtString;
class osFilePath;
class apBreakPoint;
class apCLBuffer;
class apCLImage;
class apCLPipe;
class apContextID;
class apEvent;
class apFunctionCall;
struct apGLTextureMipLevelID;
class apGLVBO;
class apKernelSourceCodeBreakpoint;
class apParameter;

// Infra:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>
#include <AMDTAPIClasses/Include/apContextID.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdDebugApplicationTreeData.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>

GD_API bool gdGetCurrentBreakpointFunction(const apFunctionCall* pBreakedOnFunctionCall, gtString& functionName, gtString& functionArgs);
GD_API void gdGetContextNameString(apContextID renderContextID, gtString& renderContextNameString, bool addDeletionMark = false);
GD_API void gdExtractiPhoneSDKNameFromFullPath(const gtString& sdkFullPath, gtString& sdkName);
GD_API bool gdKernelDebuggingCurrentWorkItemAsString(gtString& wiString);
GD_API bool gdIsHSAKernelName(const gtString& kernelName);

// cl_gremedy_object_naming:
GD_API void gdGetVBODisplayName(const apGLVBO& vboDetails, gtString& vboDisplayName);
GD_API void gdGetBufferDisplayName(const apCLBuffer& bufferDetails, gtString& bufferDisplayName, bool longDefaultName = true);
GD_API void gdGetImageDisplayName(const apCLImage& imageDetails, gtString& imageDisplayName, bool longDefaultName = true);
GD_API void gdGetPipeDisplayName(const apCLPipe& pipeDetails, gtString& pipeDisplayName, bool longDefaultName = true);

// OpenGL error as string:
GD_API void gdOpenGLErrorToString(GLenum openGLError, gtString& errorString);
GD_API void gdOpenGLErrorToDescriptionString(GLenum openGLError, gtString& errorString);

// OpenCL error as string:
GD_API void gdOpenCLErrorToString(int openCLError, gtString& errorString);
GD_API void gdOpenCLErrorToDescriptionString(int openCLError, gtString& errorString);

// Parameters as string:
GD_API bool gdParameterAsString(const apParameter* pParam, gtString& functionArgAsString, bool addOriginalValue, bool addLink = false);
GD_API bool gdOpenCLHandleAsString(oaCLHandle clObjectHandle, gtString& functionArgAsString, bool addLink, bool stringContainsFormattedHandle, bool useCommandQueuesViewerNameFormat);
GD_API bool gdFunctionCallAsString(const apFunctionCall* pFunctionCall, gtString& functionCallAsString, apContextType functionContextType);
GD_API bool gdIsDebugging64BitApplication();
GD_API void gdUserApplicationAddressToDisplayString(osProcedureAddress64 pointer, gtString& outputString, bool inUppercase = false);

// Mode changes:
GD_API bool gdDoesModeChangeApplyToDebuggerViews(const apEvent& eve, bool& enableView);

// Source code utilities:
GD_API int gdSearchForKernelDeclarationInSourceFile(const osFilePath& sourceCodeFilePath, const gtString& kernelFuncName);
GD_API bool gdFindObjectFilePath(const afApplicationTreeItemData* pDisplayedItemData, osFilePath& filePath, int& lineNumber);
GD_API bool gdBreakpointAsString(const gtAutoPtr<apBreakPoint>& aptrBreakpoint, gtString& breakpointName);
GD_API oaCLHandle gdFindSourceCodeBreakpointMathingProgramHandle(const apKernelSourceCodeBreakpoint& kernelSourceCodeBP);

#endif  // __GDAIDFUNCTIONS

