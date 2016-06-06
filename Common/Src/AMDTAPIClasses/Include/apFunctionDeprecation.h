//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apFunctionDeprecation.h
///
//==================================================================================

//------------------------------ apFunctionDeprecation.h ------------------------------

#ifndef __APFUNCTIONDEPRECATION
#define __APFUNCTIONDEPRECATION


// Infra:
#include <AMDTOSWrappers/Include/osTransferableObject.h>
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTAPIClasses/Include/apParameters.h>
#include <AMDTAPIClasses/Include/apAPIVersion.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionId.h>


// enum apFunctionDeprecationStatus - describe a function call deprecation status
// NOTICE: When a new deprecation type is added, please make sure that you add this
// deprecation type to getDeprecationAndRemoveVersionsByStatus (that return the
// OpenGL deprecation versions by type.
typedef enum
{
    AP_DEPRECATION_NONE = 0,                            // Function call is not deprecated
    AP_DEPRECATION_FULL = 1,                            // Function call is completely deprecated
    AP_DEPRECATION_PIXEL_FORMAT = 2,                    // One of the function call arguments has a deprecated pixel format
    AP_DEPRECATION_APPLICATION_GENERATED_NAMES = 3,     // Function is using an object without calling the glGenXXX related function before
    AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING = 4,// Edge flags and fixed pipeline vertex processing
    AP_DEPRECATION_COLOR_INDEX_MODE = 5,                // Color index mode
    AP_DEPRECATION_IMMEDIATE_MODE = 6,                  // Immediate drawing mode
    AP_DEPRECATION_DISPLAY_LISTS = 7,                   // Display lists
    AP_DEPRECATION_ATTRIBUTE_STACKS_FUNCTION = 8,       // Attribute Stacks functions
    AP_DEPRECATION_ATTRIBUTE_STACKS_STATE = 9,          // Attribute Stacks state variables
    AP_DEPRECATION_RECTANGLE = 10,                      // Rectangle functions
    AP_DEPRECATION_RASTER_POS_FUNCTION = 11,            // Current raster position function
    AP_DEPRECATION_RASTER_POS_STATE = 12,               // Current raster position state variable
    AP_DEPRECATION_SEPERATE_POLYGON_DRAW_MODE = 13,     // Separate polygon draw mode
    AP_DEPRECATION_POLYGON_AND_LINE_STIPPLE_FUNCTION = 14,// Polygon and line stipple function
    AP_DEPRECATION_POLYGON_AND_LINE_STIPPLE_STATE = 15,// Polygon and line stipple state variable
    AP_DEPRECATION_PIXEL_DRAWING = 16,                  // Pixel drawing
    AP_DEPRECATION_BITMAP = 17,                         // Bitmaps
    AP_DEPRECATION_TEXTURE_CLAMP_WRAP_MODE = 18,        // Texture wrap mode clamp
    AP_DEPRECATION_TEXTURE_AUTO_MIPMAP = 19,            // Texture mipmap auto generation
    AP_DEPRECATION_ALPHA_TEST_FUNCTION = 20,            // Alpha test function
    AP_DEPRECATION_ALPHA_TEST_STATE = 21,               // Alpha test state variable
    AP_DEPRECATION_EVALUATORS_FUNCTION = 22,            // Evaluators function
    AP_DEPRECATION_EVALUATORS_STATE = 23,               // Evaluators state variable
    AP_DEPRECATION_FEEDBACK = 24,                       // Selection and feedback modes
    AP_DEPRECATION_HINTS = 25,                          // Hints
    AP_DEPRECATION_NON_SPRITE_POINTS = 26,              // Non-sprite points
    AP_DEPRECATION_LINE_WIDTH = 27,                     // Line width
    AP_DEPRECATION_TEXTURE_BORDER = 28,                 // Texture border
    AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_FUNCTION = 29,// Fixed function fragment processing
    AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE = 30,// Fixed function fragment processing state variables
    AP_DEPRECATION_ACCUMMULATION_BUFFERS_FUNCTION = 31, // Accumulation buffers
    AP_DEPRECATION_ACCUMMULATION_BUFFERS_STATE = 32,    // Accumulation buffers
    AP_DEPRECATION_FRAMEBUFFER_SIZE_QUERIES = 33,       // Framebuffer size queries
    AP_DEPRECATION_PIXEL_COPY = 34,                     // Pixel copying
    AP_DEPRECATION_POLYGON_QUADS_PRIMITIVES = 35,       // Polygon and Quadrilateral primitives
    AP_DEPRECATION_AUXILIRY_BUFFERS = 36,               // Auxiliary buffers
    AP_DEPRECATION_UNIFIED_EXTENSION_STRING = 37,       // Unified extension string
    AP_DEPRECATION_CLIENT_VERTEX_AND_INDEX_ARRAYS = 38, // Unbound vertex buffer array object
    AP_DEPRECATION_PIXEL_TRANSFER = 39,                 // Pixel transfer
    AP_DEPRECATION_GLSL_VERSION = 40,                   // GLSL version
    AP_DEPRECATION_MAX_VARYING = 41,                    // Max varying state variables
    AP_DEPRECATION_COMPRESSED_TEXTURE_FORMATS = 42,     // Implementation-defined compressed texture formats
    AP_DEPRECATION_LSB_FIRST_PIXEL_PACKING = 43,        // GL_[UN]PACK_LSB_FIRST glPixelStore* parameters
    AP_DEPRECATION_STATUS_AMOUNT = 44,                  // Amount of deprecation status
} apFunctionDeprecationStatus;

AP_API bool apFunctionDeprecationStatusToString(apFunctionDeprecationStatus status, gtString& statusAsStr);


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apFunctionDeprecation : public osTransferableObject
//
// General Description:
//   Represents a function call deprecation description
//
// Author:  AMD Developer Tools Team
// Creation Date:        3/3/2009
// ----------------------------------------------------------------------------------
class AP_API apFunctionDeprecation : public osTransferableObject
{
public:
    apFunctionDeprecation();
    apFunctionDeprecation(const apFunctionDeprecation& other);
    ~apFunctionDeprecation();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    void setStatus(apFunctionDeprecationStatus status) {_status = status;};
    apFunctionDeprecationStatus status() {return _status;};

    void setReasonString(gtString reason) {_reasonStr = reason;};
    gtString reasonString() {return _reasonStr;};

    void setArgumentIndex(int argumentIndex) {_argumentIndex = argumentIndex;};
    int argumentIndex() {return _argumentIndex;};

    bool toString(gtString& str);

    static bool getDeprecationAndRemoveVersionsByStatus(apFunctionDeprecationStatus functionDeprecationStatus, apAPIVersion& deprecatedAtVersion, apAPIVersion& removedAtVersion);
    static bool getDeprecationStatusByFunctionId(int functionId, apFunctionDeprecationStatus& deprecationStatus);

private:
    // The function deprecation status:
    apFunctionDeprecationStatus _status;

    // String containing:
    // 1. in case of AP_DEPRECATION_ARG_VALUE - the argument name (in the function documentation)
    // 2. in case of AP_DEPRECATION_APPLICATION_GENERATED_NAMES - the object name (Texture/Render Buffer etc')
    gtString _reasonStr;

    // Is used only for AP_DEPRECATION_ARG_VALUE - contain the index of the argument with the
    // deprecated value:
    int _argumentIndex;
};


#endif  // __APFUNCTIONDEPRECATION
