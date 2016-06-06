//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCGLParameters.cpp
///
//==================================================================================

//------------------------------ apCGLParameters.cpp ------------------------------

// OpenGL:
#include <OpenGL/CGLRenderers.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osTransferableObjectType.h>

// Local:
#include <AMDTAPIClasses/Include/apCGLParameters.h>


// ----------------------------- apCGLPixelFormatAttributeParameter --------------------------------

// ---------------------------------------------------------------------------
// Name:        apCGLPixelFormatAttributeParameter::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
osTransferableObjectType apCGLPixelFormatAttributeParameter::type() const
{
    return OS_TOBJ_ID_CGL_PIXEL_FORMAT_ATTRIBUTE_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apCGLPixelFormatAttributeParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
bool apCGLPixelFormatAttributeParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCGLPixelFormatAttributeParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
bool apCGLPixelFormatAttributeParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCGLPixelFormatAttributeParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
void apCGLPixelFormatAttributeParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    unsigned long argumentValue = va_arg(pArgumentList , unsigned long);
    _value = (CGLPixelFormatAttribute)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apCGLPixelFormatAttributeParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
void apCGLPixelFormatAttributeParameter::readValueFromPointer(void* pValue)
{
    _value = *((CGLPixelFormatAttribute*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apCGLPixelFormatAttributeParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
gtSizeType apCGLPixelFormatAttributeParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(CGLPixelFormatAttribute);
    return sizeOfValue;
}


// ---------------------------------------------------------------------------
// Name:        apCGLPixelFormatAttributeParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
void apCGLPixelFormatAttributeParameter::valueAsString(gtString& valueString) const
{
    switch (_value)
    {
        case kCGLPFAAllRenderers:
            valueString = L"kCGLPFAAllRenderers";
            break;

        case kCGLPFADoubleBuffer:
            valueString = L"kCGLPFADoubleBuffer";
            break;

        case kCGLPFAStereo:
            valueString = L"kCGLPFAStereo";
            break;

        case kCGLPFAAuxBuffers:
            valueString = L"kCGLPFAAuxBuffers";
            break;

        case kCGLPFAColorSize:
            valueString = L"kCGLPFAColorSize";
            break;

        case kCGLPFAAlphaSize:
            valueString = L"kCGLPFAAlphaSize";
            break;

        case kCGLPFADepthSize:
            valueString = L"kCGLPFADepthSize";
            break;

        case kCGLPFAStencilSize:
            valueString = L"kCGLPFAStencilSize";
            break;

        case kCGLPFAAccumSize:
            valueString = L"kCGLPFAAccumSize";
            break;

        case kCGLPFAMinimumPolicy:
            valueString = L"kCGLPFAMinimumPolicy";
            break;

        case kCGLPFAMaximumPolicy:
            valueString = L"kCGLPFAMaximumPolicy";
            break;

        case kCGLPFAOffScreen:
            valueString = L"kCGLPFAOffScreen";
            break;

        case kCGLPFAFullScreen:
            valueString = L"kCGLPFAFullScreen";
            break;

        case kCGLPFASampleBuffers:
            valueString = L"kCGLPFASampleBuffers";
            break;

        case kCGLPFASamples:
            valueString = L"kCGLPFASamples";
            break;

        case kCGLPFAAuxDepthStencil:
            valueString = L"kCGLPFAAuxDepthStencil";
            break;

        case kCGLPFAColorFloat:
            valueString = L"kCGLPFAColorFloat";
            break;

        case kCGLPFAMultisample:
            valueString = L"kCGLPFAMultisample";
            break;

        case kCGLPFASupersample:
            valueString = L"kCGLPFASupersample";
            break;

        case kCGLPFASampleAlpha:
            valueString = L"kCGLPFASampleAlpha";
            break;

        case kCGLPFARendererID:
            valueString = L"kCGLPFARendererID";
            break;

        case kCGLPFASingleRenderer:
            valueString = L"kCGLPFASingleRenderer";
            break;

        case kCGLPFANoRecovery:
            valueString = L"kCGLPFANoRecovery";
            break;

        case kCGLPFAAccelerated:
            valueString = L"kCGLPFAAccelerated";
            break;

        case kCGLPFAClosestPolicy:
            valueString = L"kCGLPFAClosestPolicy";
            break;

        case kCGLPFARobust:
            valueString = L"kCGLPFARobust";
            break;

        case kCGLPFABackingStore:
            valueString = L"kCGLPFABackingStore";
            break;

        case kCGLPFAMPSafe:
            valueString = L"kCGLPFAMPSafe";
            break;

        case kCGLPFAWindow:
            valueString = L"kCGLPFAWindow";
            break;

        case kCGLPFAMultiScreen:
            valueString = L"kCGLPFAMultiScreen";
            break;

        case kCGLPFACompliant:
            valueString = L"kCGLPFACompliant";
            break;

        case kCGLPFADisplayMask:
            valueString = L"kCGLPFADisplayMask";
            break;

        case kCGLPFAPBuffer:
            valueString = L"kCGLPFAPBuffer";
            break;

        case kCGLPFARemotePBuffer:
            valueString = L"kCGLPFARemotePBuffer";
            break;

        case kCGLPFAAllowOfflineRenderers:
            valueString = L"kCGLPFAAllowOfflineRenderers";
            break;

        case kCGLPFAAcceleratedCompute:
            valueString = L"kCGLPFAAcceleratedCompute";
            break;

        case kCGLPFAVirtualScreenCount:
            valueString = L"kCGLPFAVirtualScreenCount";
            break;

        default:
            // Unknown CGLPixelFormatAttribute
            gtString unknownEnumString;
            unknownEnumString.appendFormattedString(L"Unknown CGLPixelFormatAttribute:  0x%X", _value);
            GT_ASSERT_EX(0, unknownEnumString.asCharArray());

            valueString = L"Unknown";
            break;
    }
}

// ---------------------------------------------------------------------------
// Name:        apCGLPixelFormatAttributeParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
bool apCGLPixelFormatAttributeParameter::compareToOther(const apParameter& other) const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apCGLPixelFormatAttributeParameter:
        apCGLPixelFormatAttributeParameter* pParam  = (apCGLPixelFormatAttributeParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}


// ----------------------------- apCGLContextEnableParameter --------------------------------

// ---------------------------------------------------------------------------
// Name:        apCGLContextEnableParameter::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
osTransferableObjectType apCGLContextEnableParameter::type() const
{
    return OS_TOBJ_ID_CGL_CONTEXT_ENABLE_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apCGLContextEnableParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
bool apCGLContextEnableParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCGLContextEnableParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
bool apCGLContextEnableParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCGLContextEnableParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
void apCGLContextEnableParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    unsigned long argumentValue = va_arg(pArgumentList , unsigned long);
    _value = (CGLContextEnable)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apCGLContextEnableParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
void apCGLContextEnableParameter::readValueFromPointer(void* pValue)
{
    _value = *((CGLContextEnable*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apCGLContextEnableParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
gtSizeType apCGLContextEnableParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(CGLContextEnable);
    return sizeOfValue;
}


// ---------------------------------------------------------------------------
// Name:        apCGLContextEnableParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
void apCGLContextEnableParameter::valueAsString(gtString& valueString) const
{
    switch (_value)
    {
        case kCGLCESwapRectangle:
            valueString = L"kCGLCESwapRectangle";
            break;

        case kCGLCESwapLimit:
            valueString = L"kCGLCESwapLimit";
            break;

        case kCGLCERasterization:
            valueString = L"kCGLCERasterization";
            break;

        case kCGLCEStateValidation:
            valueString = L"kCGLCEStateValidation";
            break;

        case kCGLCESurfaceBackingSize:
            valueString = L"kCGLCESurfaceBackingSize";
            break;

        case kCGLCEDisplayListOptimization:
            valueString = L"kCGLCEDisplayListOptimization";
            break;

        case kCGLCEMPEngine:
            valueString = L"kCGLCEMPEngine";
            break;

        default:
            // Unknown CGLPixelFormatAttribute
            gtString unknownEnumString;
            unknownEnumString.appendFormattedString(L"Unknown CGLContextEnable:  0x%X", _value);
            GT_ASSERT_EX(0, unknownEnumString.asCharArray());

            valueString = L"Unknown";
            break;
    }
}

// ---------------------------------------------------------------------------
// Name:        apCGLContextEnableParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
bool apCGLContextEnableParameter::compareToOther(const apParameter& other) const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apCGLContextEnableParameter:
        apCGLContextEnableParameter* pParam  = (apCGLContextEnableParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}



// ----------------------------- apCGLContextParameterParameter --------------------------------

// ---------------------------------------------------------------------------
// Name:        apCGLContextParameterParameter::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
osTransferableObjectType apCGLContextParameterParameter::type() const
{
    return OS_TOBJ_ID_CGL_CONTEXT_PARAMETER_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apCGLContextParameterParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
bool apCGLContextParameterParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCGLContextParameterParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
bool apCGLContextParameterParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCGLContextParameterParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
void apCGLContextParameterParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    unsigned long argumentValue = va_arg(pArgumentList , unsigned long);
    _value = (CGLContextParameter)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apCGLContextParameterParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
void apCGLContextParameterParameter::readValueFromPointer(void* pValue)
{
    _value = *((CGLContextParameter*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apCGLContextParameterParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
gtSizeType apCGLContextParameterParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(CGLContextParameter);
    return sizeOfValue;
}


// ---------------------------------------------------------------------------
// Name:        apCGLContextParameterParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
void apCGLContextParameterParameter::valueAsString(gtString& valueString) const
{
    switch (_value)
    {
        case kCGLCPSwapRectangle:
            valueString = L"kCGLCPSwapRectangle";
            break;

        case kCGLCPSwapInterval:
            valueString = L"kCGLCPSwapInterval";
            break;

        case kCGLCPDispatchTableSize:
            valueString = L"kCGLCPDispatchTableSize";
            break;

        case kCGLCPClientStorage:
            valueString = L"kCGLCPClientStorage";
            break;

        case kCGLCPSurfaceTexture:
            valueString = L"kCGLCPSurfaceTexture";
            break;

        case kCGLCPSurfaceOrder:
            valueString = L"kCGLCPSurfaceOrder";
            break;

        case kCGLCPSurfaceOpacity:
            valueString = L"kCGLCPSurfaceOpacity";
            break;

        case kCGLCPSurfaceBackingSize:
            valueString = L"kCGLCPSurfaceBackingSize";
            break;

        case kCGLCPSurfaceSurfaceVolatile:
            valueString = L"kCGLCPSurfaceSurfaceVolatile";
            break;

        case kCGLCPReclaimResources:
            valueString = L"kCGLCPReclaimResources";
            break;

        case kCGLCPCurrentRendererID:
            valueString = L"kCGLCPCurrentRendererID";
            break;

        case kCGLCPGPUVertexProcessing:
            valueString = L"kCGLCPGPUVertexProcessing";
            break;

        case kCGLCPGPUFragmentProcessing:
            valueString = L"kCGLCPGPUFragmentProcessing";
            break;

        case kCGLCPHasDrawable:
            valueString = L"kCGLCPHasDrawable";
            break;

        case kCGLCPMPSwapsInFlight:
            valueString = L"kCGLCPMPSwapsInFlight";
            break;

        default:
            // Unknown CGLPixelFormatAttribute
            gtString unknownEnumString;
            unknownEnumString.appendFormattedString(L"Unknown CGLContextParameter:  0x%X", _value);
            GT_ASSERT_EX(0, unknownEnumString.asCharArray());

            valueString = L"Unknown";
            break;
    }
}

// ---------------------------------------------------------------------------
// Name:        apCGLContextParameterParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
bool apCGLContextParameterParameter::compareToOther(const apParameter& other) const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apCGLContextParameterParameter:
        apCGLContextParameterParameter* pParam  = (apCGLContextParameterParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// ----------------------------- apCGLGlobalOptionParameter --------------------------------

// ---------------------------------------------------------------------------
// Name:        apCGLGlobalOptionParameter::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
osTransferableObjectType apCGLGlobalOptionParameter::type() const
{
    return OS_TOBJ_ID_CGL_GLOBAL_OPTION_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apCGLGlobalOptionParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
bool apCGLGlobalOptionParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCGLGlobalOptionParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
bool apCGLGlobalOptionParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCGLGlobalOptionParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
void apCGLGlobalOptionParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    unsigned long argumentValue = va_arg(pArgumentList , unsigned long);
    _value = (CGLGlobalOption)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apCGLGlobalOptionParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
void apCGLGlobalOptionParameter::readValueFromPointer(void* pValue)
{
    _value = *((CGLGlobalOption*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apCGLGlobalOptionParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
gtSizeType apCGLGlobalOptionParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(CGLGlobalOption);
    return sizeOfValue;
}


// ---------------------------------------------------------------------------
// Name:        apCGLGlobalOptionParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
void apCGLGlobalOptionParameter::valueAsString(gtString& valueString) const
{
    switch (_value)
    {
        case kCGLGOFormatCacheSize:
            valueString = L"kCGLGOFormatCacheSize";
            break;

        case kCGLGOClearFormatCache:
            valueString = L"kCGLGOClearFormatCache";
            break;

        case kCGLGORetainRenderers:
            valueString = L"kCGLGORetainRenderers";
            break;

        case kCGLGOResetLibrary:
            valueString = L"kCGLGOResetLibrary";
            break;

        case kCGLGOUseErrorHandler:
            valueString = L"kCGLGOUseErrorHandler";
            break;

        case kCGLGOUseBuildCache:
            valueString = L"kCGLGOUseBuildCache";
            break;

        default:
            // Unknown CGLPixelFormatAttribute
            gtString unknownEnumString;
            unknownEnumString.appendFormattedString(L"Unknown CGLGlobalOption:  0x%X", _value);
            GT_ASSERT_EX(0, unknownEnumString.asCharArray());

            valueString = L"Unknown";
            break;
    }
}

// ---------------------------------------------------------------------------
// Name:        apCGLGlobalOptionParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
bool apCGLGlobalOptionParameter::compareToOther(const apParameter& other) const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apCGLGlobalOptionParameter:
        apCGLGlobalOptionParameter* pParam  = (apCGLGlobalOptionParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// ----------------------------- apCGLRendererPropertyParameter --------------------------------

// ---------------------------------------------------------------------------
// Name:        apCGLRendererPropertyParameter::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
osTransferableObjectType apCGLRendererPropertyParameter::type() const
{
    return OS_TOBJ_ID_CGL_RENDERER_PROPERTY_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apCGLRendererPropertyParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
bool apCGLRendererPropertyParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCGLRendererPropertyParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
bool apCGLRendererPropertyParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCGLRendererPropertyParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
void apCGLRendererPropertyParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    unsigned long argumentValue = va_arg(pArgumentList , unsigned long);
    _value = (CGLRendererProperty)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apCGLRendererPropertyParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
void apCGLRendererPropertyParameter::readValueFromPointer(void* pValue)
{
    _value = *((CGLRendererProperty*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apCGLRendererPropertyParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
gtSizeType apCGLRendererPropertyParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(CGLRendererProperty);
    return sizeOfValue;
}


// ---------------------------------------------------------------------------
// Name:        apCGLRendererPropertyParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
void apCGLRendererPropertyParameter::valueAsString(gtString& valueString) const
{
    switch (_value)
    {
        case kCGLRPOffScreen:
            valueString = L"kCGLRPOffScreen";
            break;

        case kCGLRPFullScreen:
            valueString = L"kCGLRPFullScreen";
            break;

        case kCGLRPRendererID:
            valueString = L"kCGLRPRendererID";
            break;

        case kCGLRPAccelerated:
            valueString = L"kCGLRPAccelerated";
            break;

        case kCGLRPRobust:
            valueString = L"kCGLRPRobust";
            break;

        case kCGLRPBackingStore:
            valueString = L"kCGLRPBackingStore";
            break;

        case kCGLRPMPSafe:
            valueString = L"kCGLRPMPSafe";
            break;

        case kCGLRPWindow:
            valueString = L"kCGLRPWindow";
            break;

        case kCGLRPMultiScreen:
            valueString = L"kCGLRPMultiScreen";
            break;

        case kCGLRPCompliant:
            valueString = L"kCGLRPCompliant";
            break;

        case kCGLRPDisplayMask:
            valueString = L"kCGLRPDisplayMask";
            break;

        case kCGLRPBufferModes:
            valueString = L"kCGLRPBufferModes";
            break;

        case kCGLRPColorModes:
            valueString = L"kCGLRPColorModes";
            break;

        case kCGLRPAccumModes:
            valueString = L"kCGLRPAccumModes";
            break;

        case kCGLRPDepthModes:
            valueString = L"kCGLRPDepthModes";
            break;

        case kCGLRPStencilModes:
            valueString = L"kCGLRPStencilModes";
            break;

        case kCGLRPMaxAuxBuffers:
            valueString = L"kCGLRPMaxAuxBuffers";
            break;

        case kCGLRPMaxSampleBuffers:
            valueString = L"kCGLRPMaxSampleBuffers";
            break;

        case kCGLRPMaxSamples:
            valueString = L"kCGLRPMaxSamples";
            break;

        case kCGLRPSampleModes:
            valueString = L"kCGLRPSampleModes";
            break;

        case kCGLRPSampleAlpha:
            valueString = L"kCGLRPSampleAlpha";
            break;

        case kCGLRPVideoMemory:
            valueString = L"kCGLRPVideoMemory";
            break;

        case kCGLRPTextureMemory:
            valueString = L"kCGLRPTextureMemory";
            break;

        case kCGLRPGPUVertProcCapable:
            valueString = L"kCGLRPGPUVertProcCapable";
            break;

        case kCGLRPGPUFragProcCapable:
            valueString = L"kCGLRPGPUFragProcCapable";
            break;

        case kCGLRPRendererCount:
            valueString = L"kCGLRPRendererCount";
            break;

        case kCGLRPOnline:
            valueString = L"kCGLRPOnline";
            break;

        case kCGLRPAcceleratedCompute:
            valueString = L"kCGLRPAcceleratedCompute";
            break;

        default:
            // Unknown CGLPixelFormatAttribute
            gtString unknownEnumString;
            unknownEnumString.appendFormattedString(L"Unknown CGLRendererProperty:  0x%X", _value);
            GT_ASSERT_EX(0, unknownEnumString.asCharArray());

            valueString = L"Unknown";
            break;
    }
}

// ---------------------------------------------------------------------------
// Name:        apCGLRendererPropertyParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
bool apCGLRendererPropertyParameter::compareToOther(const apParameter& other) const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apCGLRendererPropertyParameter:
        apCGLRendererPropertyParameter* pParam  = (apCGLRendererPropertyParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// ----------------------------- apCGLBufferModeMaskParameter --------------------------------

// ---------------------------------------------------------------------------
// Name:        apCGLBufferModeMaskParameter::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
osTransferableObjectType apCGLBufferModeMaskParameter::type() const
{
    return OS_TOBJ_ID_CGL_BUFFER_MODE_MASK_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apCGLBufferModeMaskParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
bool apCGLBufferModeMaskParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCGLBufferModeMaskParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
bool apCGLBufferModeMaskParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCGLBufferModeMaskParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
void apCGLBufferModeMaskParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    unsigned int argumentValue = va_arg(pArgumentList , unsigned int);
    _value = argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apCGLBufferModeMaskParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
void apCGLBufferModeMaskParameter::readValueFromPointer(void* pValue)
{
    _value = *((unsigned int*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apCGLBufferModeMaskParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
gtSizeType apCGLBufferModeMaskParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(unsigned int);
    return sizeOfValue;
}


// ---------------------------------------------------------------------------
// Name:        apCGLBufferModeMaskParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
void apCGLBufferModeMaskParameter::valueAsString(gtString& valueString) const
{
    valueString = L"";
    bool firstParam = true;

    if (_value & kCGLMonoscopicBit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGLMonoscopicBit");
        firstParam = false;
    }

    if (_value & kCGLStereoscopicBit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGLStereoscopicBit");
        firstParam = false;
    }

    if (_value & kCGLSingleBufferBit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGLSingleBufferBit");
        firstParam = false;
    }

    if (_value & kCGLDoubleBufferBit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGLDoubleBufferBit");
        firstParam = false;
    }
}

// ---------------------------------------------------------------------------
// Name:        apCGLBufferModeMaskParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        14/12/2008
// ---------------------------------------------------------------------------
bool apCGLBufferModeMaskParameter::compareToOther(const apParameter& other) const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apCGLBufferModeMaskParameter:
        apCGLBufferModeMaskParameter* pParam  = (apCGLBufferModeMaskParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}



// ----------------------------- apCGLColorBufferFormatMaskParameter --------------------------------

// ---------------------------------------------------------------------------
// Name:        apCGLColorBufferFormatMaskParameter::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        15/12/2008
// ---------------------------------------------------------------------------
osTransferableObjectType apCGLColorBufferFormatMaskParameter::type() const
{
    return OS_TOBJ_ID_CGL_COLOR_BUFFER_FORMAT_MASK_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apCGLColorBufferFormatMaskParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        15/12/2008
// ---------------------------------------------------------------------------
bool apCGLColorBufferFormatMaskParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCGLColorBufferFormatMaskParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        15/12/2008
// ---------------------------------------------------------------------------
bool apCGLColorBufferFormatMaskParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCGLColorBufferFormatMaskParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        15/12/2008
// ---------------------------------------------------------------------------
void apCGLColorBufferFormatMaskParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    unsigned int argumentValue = va_arg(pArgumentList , unsigned int);
    _value = argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apCGLColorBufferFormatMaskParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        15/12/2008
// ---------------------------------------------------------------------------
void apCGLColorBufferFormatMaskParameter::readValueFromPointer(void* pValue)
{
    _value = *((unsigned int*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apCGLColorBufferFormatMaskParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        15/12/2008
// ---------------------------------------------------------------------------
gtSizeType apCGLColorBufferFormatMaskParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(unsigned int);
    return sizeOfValue;
}


// ---------------------------------------------------------------------------
// Name:        apCGLColorBufferFormatMaskParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        15/12/2008
// ---------------------------------------------------------------------------
void apCGLColorBufferFormatMaskParameter::valueAsString(gtString& valueString) const
{
    valueString = L"";
    bool firstParam = true;

    if (_value & kCGLRGB444Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGLRGB444Bit");
        firstParam = false;
    }

    if (_value & kCGLARGB4444Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGLARGB4444Bit");
        firstParam = false;
    }

    if (_value & kCGLRGB444A8Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGLRGB444A8Bit");
        firstParam = false;
    }

    if (_value & kCGLRGB555Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGLRGB555Bit");
        firstParam = false;
    }

    if (_value & kCGLARGB1555Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGLARGB1555Bit");
        firstParam = false;
    }

    if (_value & kCGLRGB555A8Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGLRGB555A8Bit");
        firstParam = false;
    }

    if (_value & kCGLRGB565Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGLRGB565Bit");
        firstParam = false;
    }

    if (_value & kCGLRGB565A8Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGLRGB565A8Bit");
        firstParam = false;
    }

    if (_value & kCGLRGB888Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGLRGB888Bit");
        firstParam = false;
    }

    if (_value & kCGLARGB8888Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGLARGB8888Bit");
        firstParam = false;
    }

    if (_value & kCGLRGB888A8Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGLRGB888A8Bit");
        firstParam = false;
    }

    if (_value & kCGLRGB101010Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGLRGB101010Bit");
        firstParam = false;
    }

    if (_value & kCGLARGB2101010Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGLARGB2101010Bit");
        firstParam = false;
    }

    if (_value & kCGLRGB101010_A8Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGLRGB101010_A8Bit");
        firstParam = false;
    }

    if (_value & kCGLRGB121212Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGLRGB121212Bit");
        firstParam = false;
    }

    if (_value & kCGLARGB12121212Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGLARGB12121212Bit");
        firstParam = false;
    }

    if (_value & kCGLRGB161616Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGLRGB161616Bit");
        firstParam = false;
    }

    if (_value & kCGLRGBA16161616Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGLRGBA16161616Bit");
        firstParam = false;
    }

    if (_value & kCGLRGBFloat64Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGLRGBFloat64Bit");
        firstParam = false;
    }

    if (_value & kCGLRGBAFloat64Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGLRGBAFloat64Bit");
        firstParam = false;
    }

    if (_value & kCGLRGBAFloat128Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGLRGBAFloat128Bit");
        firstParam = false;
    }

    if (_value & kCGLRGBFloat256Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGLRGBFloat256Bit");
        firstParam = false;
    }

    if (_value & kCGLRGBAFloat256Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGLRGBAFloat256Bit");
        firstParam = false;
    }
}


// ---------------------------------------------------------------------------
// Name:        apCGLColorBufferFormatMaskParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        15/12/2008
// ---------------------------------------------------------------------------
bool apCGLColorBufferFormatMaskParameter::compareToOther(const apParameter& other) const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apCGLColorBufferFormatMaskParameter:
        apCGLColorBufferFormatMaskParameter* pParam  = (apCGLColorBufferFormatMaskParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// ----------------------------- apCGLRendererIDParameter --------------------------------

// ---------------------------------------------------------------------------
// Name:        apCGLRendererIDParameter::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        15/12/2008
// ---------------------------------------------------------------------------
osTransferableObjectType apCGLRendererIDParameter::type() const
{
    return OS_TOBJ_ID_CGL_RENDERER_ID_MASK_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apCGLRendererIDParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        15/12/2008
// ---------------------------------------------------------------------------
bool apCGLRendererIDParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCGLRendererIDParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        15/12/2008
// ---------------------------------------------------------------------------
bool apCGLRendererIDParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCGLRendererIDParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        15/12/2008
// ---------------------------------------------------------------------------
void apCGLRendererIDParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    unsigned int argumentValue = va_arg(pArgumentList , unsigned int);
    _value = argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apCGLRendererIDParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        15/12/2008
// ---------------------------------------------------------------------------
void apCGLRendererIDParameter::readValueFromPointer(void* pValue)
{
    _value = *((unsigned int*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apCGLRendererIDParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        15/12/2008
// ---------------------------------------------------------------------------
gtSizeType apCGLRendererIDParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(unsigned int);
    return sizeOfValue;
}


// ---------------------------------------------------------------------------
// Name:        apCGLRendererIDParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        15/12/2008
// ---------------------------------------------------------------------------
void apCGLRendererIDParameter::valueAsString(gtString& valueString) const
{
    switch (_value)
    {
        case kCGLRendererGenericID:
            valueString = L"kCGLRendererGenericID";
            break;

        case kCGLRendererGenericFloatID:
            valueString = L"kCGLRendererGenericFloatID";
            break;

        case kCGLRendererAppleSWID:
            valueString = L"kCGLRendererAppleSWID";
            break;

        case kCGLRendererATIRage128ID:
            valueString = L"kCGLRendererATIRage128ID";
            break;

        case kCGLRendererATIRadeonID:
            valueString = L"kCGLRendererATIRadeonID";
            break;

        case kCGLRendererATIRageProID:
            valueString = L"kCGLRendererATIRageProID";
            break;

        case kCGLRendererATIRadeon8500ID:
            valueString = L"kCGLRendererATIRadeon8500ID";
            break;

        case kCGLRendererATIRadeon9700ID:
            valueString = L"kCGLRendererATIRadeon9700ID";
            break;

        case kCGLRendererATIRadeonX1000ID:
            valueString = L"kCGLRendererATIRadeonX1000ID";
            break;

        case kCGLRendererATIRadeonX2000ID:
            valueString = L"kCGLRendererATIRadeonX2000ID";
            break;

        case kCGLRendererGeForce2MXID:
            valueString = L"kCGLRendererGeForce2MXID";
            break;

        case kCGLRendererGeForce3ID:
            valueString = L"kCGLRendererGeForce3ID";
            break;

        case kCGLRendererGeForceFXID:
            valueString = L"kCGLRendererGeForceFXID";
            break;

        case kCGLRendererGeForce8xxxID:
            valueString = L"kCGLRendererGeForce8xxxID";
            break;

        case kCGLRendererVTBladeXP2ID:
            valueString = L"kCGLRendererVTBladeXP2ID";
            break;

        case kCGLRendererIntel900ID:
            valueString = L"kCGLRendererIntel900ID";
            break;

        case kCGLRendererIntelX3100ID:
            valueString = L"kCGLRendererIntelX3100ID";
            break;

        case kCGLRendererMesa3DFXID:
            valueString = L"kCGLRendererMesa3DFXID";
            break;

        default:
            // Unknown CGLRendererID
            gtString unknownEnumString;
            unknownEnumString.appendFormattedString(L"Unknown CGLRendererID: 0x%X", _value);
            GT_ASSERT_EX(false, unknownEnumString.asCharArray());

            valueString = L"Unknown";
            break;
    }
}

// ---------------------------------------------------------------------------
// Name:        apCGLRendererIDParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        15/12/2008
// ---------------------------------------------------------------------------
bool apCGLRendererIDParameter::compareToOther(const apParameter& other) const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apCGLRendererIDParameter:
        apCGLRendererIDParameter* pParam  = (apCGLRendererIDParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// ----------------------------- apCGLSamplingModeMaskParameter --------------------------------

// ---------------------------------------------------------------------------
// Name:        apCGLSamplingModeMaskParameter::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        15/12/2008
// ---------------------------------------------------------------------------
osTransferableObjectType apCGLSamplingModeMaskParameter::type() const
{
    return OS_TOBJ_ID_CGL_SAMPLING_MODE_MASK_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apCGLSamplingModeMaskParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        15/12/2008
// ---------------------------------------------------------------------------
bool apCGLSamplingModeMaskParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCGLSamplingModeMaskParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        15/12/2008
// ---------------------------------------------------------------------------
bool apCGLSamplingModeMaskParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCGLSamplingModeMaskParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        15/12/2008
// ---------------------------------------------------------------------------
void apCGLSamplingModeMaskParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    unsigned int argumentValue = va_arg(pArgumentList , unsigned int);
    _value = argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apCGLSamplingModeMaskParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        15/12/2008
// ---------------------------------------------------------------------------
void apCGLSamplingModeMaskParameter::readValueFromPointer(void* pValue)
{
    _value = *((unsigned int*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apCGLSamplingModeMaskParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        15/12/2008
// ---------------------------------------------------------------------------
gtSizeType apCGLSamplingModeMaskParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(unsigned int);
    return sizeOfValue;
}


// ---------------------------------------------------------------------------
// Name:        apCGLSamplingModeMaskParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        15/12/2008
// ---------------------------------------------------------------------------
void apCGLSamplingModeMaskParameter::valueAsString(gtString& valueString) const
{
    valueString = L"";
    bool firstParam = true;

    if (_value & kCGLSupersampleBit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGLSupersampleBit");
        firstParam = false;
    }

    if (_value & kCGLMultisampleBit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGLMultisampleBit");
        firstParam = false;
    }
}


// ---------------------------------------------------------------------------
// Name:        apCGLSamplingModeMaskParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        15/12/2008
// ---------------------------------------------------------------------------
bool apCGLSamplingModeMaskParameter::compareToOther(const apParameter& other) const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apCGLSamplingModeMaskParameter:
        apCGLSamplingModeMaskParameter* pParam  = (apCGLSamplingModeMaskParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// ----------------------------- apCGLStencilAndDepthModeMaskParameter --------------------------------

// ---------------------------------------------------------------------------
// Name:        apCGLStencilAndDepthModeMaskParameter::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        15/12/2008
// ---------------------------------------------------------------------------
osTransferableObjectType apCGLStencilAndDepthModeMaskParameter::type() const
{
    return OS_TOBJ_ID_CGL_STENCIL_AND_DEPTH_MASK_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apCGLStencilAndDepthModeMaskParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        15/12/2008
// ---------------------------------------------------------------------------
bool apCGLStencilAndDepthModeMaskParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCGLStencilAndDepthModeMaskParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        15/12/2008
// ---------------------------------------------------------------------------
bool apCGLStencilAndDepthModeMaskParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCGLStencilAndDepthModeMaskParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        15/12/2008
// ---------------------------------------------------------------------------
void apCGLStencilAndDepthModeMaskParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    unsigned int argumentValue = va_arg(pArgumentList , unsigned int);
    _value = argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apCGLStencilAndDepthModeMaskParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        15/12/2008
// ---------------------------------------------------------------------------
void apCGLStencilAndDepthModeMaskParameter::readValueFromPointer(void* pValue)
{
    _value = *((unsigned int*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apCGLStencilAndDepthModeMaskParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        15/12/2008
// ---------------------------------------------------------------------------
gtSizeType apCGLStencilAndDepthModeMaskParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(unsigned int);
    return sizeOfValue;
}


// ---------------------------------------------------------------------------
// Name:        apCGLStencilAndDepthModeMaskParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        15/12/2008
// ---------------------------------------------------------------------------
void apCGLStencilAndDepthModeMaskParameter::valueAsString(gtString& valueString) const
{
    valueString = L"";
    bool firstParam = true;

    if (_value & kCGL0Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGL0Bit");
        firstParam = false;
    }

    if (_value & kCGL1Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGL1Bit");
        firstParam = false;
    }

    if (_value & kCGL2Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGL2Bit");
        firstParam = false;
    }

    if (_value & kCGL3Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGL3Bit");
        firstParam = false;
    }

    if (_value & kCGL4Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGL4Bit");
        firstParam = false;
    }

    if (_value & kCGL5Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGL5Bit");
        firstParam = false;
    }

    if (_value & kCGL6Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGL6Bit");
        firstParam = false;
    }

    if (_value & kCGL8Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGL8Bit");
        firstParam = false;
    }

    if (_value & kCGL10Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGL10Bit");
        firstParam = false;
    }

    if (_value & kCGL12Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGL12Bit");
        firstParam = false;
    }

    if (_value & kCGL16Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGL16Bit");
        firstParam = false;
    }

    if (_value & kCGL24Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGL24Bit");
        firstParam = false;
    }

    if (_value & kCGL32Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGL32Bit");
        firstParam = false;
    }

    if (_value & kCGL48Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGL48Bit");
        firstParam = false;
    }

    if (_value & kCGL64Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGL64Bit");
        firstParam = false;
    }

    if (_value & kCGL96Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGL96Bit");
        firstParam = false;
    }

    if (_value & kCGL128Bit)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString.append("kCGL128Bit");
        firstParam = false;
    }
}

// ---------------------------------------------------------------------------
// Name:        apCGLStencilAndDepthModeMaskParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        15/12/2008
// ---------------------------------------------------------------------------
bool apCGLStencilAndDepthModeMaskParameter::compareToOther(const apParameter& other) const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apCGLStencilAndDepthModeMaskParameter:
        apCGLStencilAndDepthModeMaskParameter* pParam  = (apCGLStencilAndDepthModeMaskParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}
