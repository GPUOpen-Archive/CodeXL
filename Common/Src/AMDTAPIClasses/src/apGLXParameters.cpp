//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLXParameters.cpp
///
//==================================================================================

//------------------------------ apGLXParameters.cpp ------------------------------

// Standard C:
#include <stdarg.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apGLXParameters.h>

// ---------------------------------------------------------------------------
// Name:        apGLXenumParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        6/7/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apGLXenumParameter::type() const
{
    return OS_TOBJ_ID_GLX_ENUM_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apGLXenumParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        6/7/2009
// ---------------------------------------------------------------------------
bool apGLXenumParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtInt32)_value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLXenumParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        6/7/2009
// ---------------------------------------------------------------------------
bool apGLXenumParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtInt32 valueAsInt32 = 0;
    ipcChannel >> valueAsInt32;
    _value = (int)valueAsInt32;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLXenumParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        6/7/2009
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apGLXenumParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    unsigned long argumentValue = va_arg(pArgumentList , unsigned long);
    _value = (int)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apGLXenumParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        6/7/2009
// ---------------------------------------------------------------------------
void apGLXenumParameter::readValueFromPointer(void* pValue)
{
    _value = *((int*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apGLXenumParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        6/7/2009
// ---------------------------------------------------------------------------
gtSizeType apGLXenumParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(int);
    return sizeOfValue;
}


// ---------------------------------------------------------------------------
// Name:        apGLXenumParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        6/7/2009
// ---------------------------------------------------------------------------
void apGLXenumParameter::valueAsString(gtString& valueString) const
{
    switch (_value)
    {
        // base glX:
        case GLX_USE_GL:
            valueString = L"GLX_USE_GL";
            break;

        case GLX_BUFFER_SIZE:
            valueString = L"GLX_BUFFER_SIZE";
            break;

        case GLX_LEVEL:
            valueString = L"GLX_LEVEL";
            break;

        case GLX_RGBA:
            valueString = L"GLX_RGBA";
            break;

        case GLX_DOUBLEBUFFER:
            valueString = L"GLX_DOUBLEBUFFER";
            break;

        case GLX_STEREO:
            valueString = L"GLX_STEREO";
            break;

        case GLX_AUX_BUFFERS:
            valueString = L"GLX_AUX_BUFFERS";
            break;

        case GLX_RED_SIZE:
            valueString = L"GLX_RED_SIZE";
            break;

        case GLX_GREEN_SIZE:
            valueString = L"GLX_GREEN_SIZE";
            break;

        case GLX_BLUE_SIZE:
            valueString = L"GLX_BLUE_SIZE";
            break;

        case GLX_ALPHA_SIZE:
            valueString = L"GLX_ALPHA_SIZE";
            break;

        case GLX_DEPTH_SIZE:
            valueString = L"GLX_DEPTH_SIZE";
            break;

        case GLX_STENCIL_SIZE:
            valueString = L"GLX_STENCIL_SIZE";
            break;

        case GLX_ACCUM_RED_SIZE:
            valueString = L"GLX_ACCUM_RED_SIZE";
            break;

        case GLX_ACCUM_GREEN_SIZE:
            valueString = L"GLX_ACCUM_GREEN_SIZE";
            break;

        case GLX_ACCUM_BLUE_SIZE:
            valueString = L"GLX_ACCUM_BLUE_SIZE";
            break;

        case GLX_ACCUM_ALPHA_SIZE:
            valueString = L"GLX_ACCUM_ALPHA_SIZE";
            break;

        // glX 1.3
        case GLX_CONFIG_CAVEAT:
            valueString = L"GLX_CONFIG_CAVEAT";
            break;

        case GLX_X_VISUAL_TYPE:
            valueString = L"GLX_X_VISUAL_TYPE";
            break;

        case GLX_TRANSPARENT_TYPE:
            valueString = L"GLX_TRANSPARENT_TYPE";
            break;

        case GLX_TRANSPARENT_INDEX_VALUE:
            valueString = L"GLX_TRANSPARENT_INDEX_VALUE";
            break;

        case GLX_TRANSPARENT_RED_VALUE:
            valueString = L"GLX_TRANSPARENT_RED_VALUE";
            break;

        case GLX_TRANSPARENT_GREEN_VALUE:
            valueString = L"GLX_TRANSPARENT_GREEN_VALUE";
            break;

        case GLX_TRANSPARENT_BLUE_VALUE:
            valueString = L"GLX_TRANSPARENT_BLUE_VALUE";
            break;

        case GLX_TRANSPARENT_ALPHA_VALUE:
            valueString = L"GLX_TRANSPARENT_ALPHA_VALUE";
            break;

        case GLX_DONT_CARE:
            valueString = L"GLX_DONT_CARE";
            break;

        case GLX_NONE:
            valueString = L"GLX_NONE";
            break;

        case GLX_SLOW_CONFIG:
            valueString = L"GLX_SLOW_CONFIG";
            break;

        case GLX_TRUE_COLOR:
            valueString = L"GLX_TRUE_COLOR";
            break;

        case GLX_DIRECT_COLOR:
            valueString = L"GLX_DIRECT_COLOR";
            break;

        case GLX_PSEUDO_COLOR:
            valueString = L"GLX_PSEUDO_COLOR";
            break;

        case GLX_STATIC_COLOR:
            valueString = L"GLX_STATIC_COLOR";
            break;

        case GLX_GRAY_SCALE:
            valueString = L"GLX_GRAY_SCALE";
            break;

        case GLX_STATIC_GRAY:
            valueString = L"GLX_STATIC_GRAY";
            break;

        case GLX_TRANSPARENT_RGB:
            valueString = L"GLX_TRANSPARENT_RGB";
            break;

        case GLX_TRANSPARENT_INDEX:
            valueString = L"GLX_TRANSPARENT_INDEX";
            break;

        case GLX_VISUAL_ID:
            valueString = L"GLX_VISUAL_ID";
            break;

        case GLX_SCREEN:
            valueString = L"GLX_SCREEN";
            break;

        case GLX_NON_CONFORMANT_CONFIG:
            valueString = L"GLX_NON_CONFORMANT_CONFIG";
            break;

        case GLX_DRAWABLE_TYPE:
            valueString = L"GLX_DRAWABLE_TYPE";
            break;

        case GLX_RENDER_TYPE:
            valueString = L"GLX_RENDER_TYPE";
            break;

        case GLX_X_RENDERABLE:
            valueString = L"GLX_X_RENDERABLE";
            break;

        case GLX_FBCONFIG_ID:
            valueString = L"GLX_FBCONFIG_ID";
            break;

        case GLX_RGBA_TYPE:
            valueString = L"GLX_RGBA_TYPE";
            break;

        case GLX_COLOR_INDEX_TYPE:
            valueString = L"GLX_COLOR_INDEX_TYPE";
            break;

        case GLX_MAX_PBUFFER_WIDTH:
            valueString = L"GLX_MAX_PBUFFER_WIDTH";
            break;

        case GLX_MAX_PBUFFER_HEIGHT:
            valueString = L"GLX_MAX_PBUFFER_HEIGHT";
            break;

        case GLX_MAX_PBUFFER_PIXELS:
            valueString = L"GLX_MAX_PBUFFER_PIXELS";
            break;

        case GLX_PRESERVED_CONTENTS:
            valueString = L"GLX_PRESERVED_CONTENTS";
            break;

        case GLX_LARGEST_PBUFFER:
            valueString = L"GLX_LARGEST_PBUFFER";
            break;

        case GLX_WIDTH:
            valueString = L"GLX_WIDTH";
            break;

        case GLX_HEIGHT:
            valueString = L"GLX_HEIGHT";
            break;

        case GLX_EVENT_MASK:
            valueString = L"GLX_EVENT_MASK";
            break;

        case GLX_DAMAGED:
            valueString = L"GLX_DAMAGED";
            break;

        case GLX_SAVED:
            valueString = L"GLX_SAVED";
            break;

        case GLX_WINDOW:
            valueString = L"GLX_WINDOW";
            break;

        case GLX_PBUFFER:
            valueString = L"GLX_PBUFFER";
            break;

        case GLX_PBUFFER_HEIGHT:
            valueString = L"GLX_PBUFFER_HEIGHT";
            break;

        case GLX_PBUFFER_WIDTH:
            valueString = L"GLX_PBUFFER_WIDTH";
            break;

        // glX 1.4
        case GLX_SAMPLE_BUFFERS:
            valueString = L"GLX_SAMPLE_BUFFERS";
            break;

        case GLX_SAMPLES:
            valueString = L"GLX_SAMPLES";
            break;

        // GLX_ARB_get_proc_address
        // no new enums

        // GLX_ARB_multisample
        // GLX_SAMPLE_BUFFERS_ARB is the same as GLX_SAMPLE_BUFFERS
        // GLX_SAMPLES_ARB is the same as GLX_SAMPLES

        // GLX_ARB_fbconfig_float
        case GLX_RGBA_FLOAT_TYPE_ARB:
            valueString = L"GLX_RGBA_FLOAT_TYPE_ARB";
            break;

        // GLX_ARB_create_context
        case GLX_CONTEXT_MAJOR_VERSION_ARB:
            valueString = L"GLX_CONTEXT_MAJOR_VERSION_ARB";
            break;

        case GLX_CONTEXT_MINOR_VERSION_ARB:
            valueString = L"GLX_CONTEXT_MINOR_VERSION_ARB";
            break;

        case GLX_CONTEXT_FLAGS_ARB:
            valueString = L"GLX_CONTEXT_FLAGS_ARB";
            break;

        // GLX_SGIS_multisample
        // GLX_SAMPLE_BUFFERS_SGIS is the same as GLX_SAMPLE_BUFFERS
        // GLX_SAMPLES_SGIS is the same as GLX_SAMPLES

        // GLX_EXT_visual_info
        // GLX_X_VISUAL_TYPE_EXT is the same as GLX_X_VISUAL_TYPE
        // GLX_TRANSPARENT_TYPE_EXT is the same as GLX_TRANSPARENT_TYPE
        // GLX_TRANSPARENT_INDEX_VALUE_EXT is the same as GLX_TRANSPARENT_INDEX_VALUE
        // GLX_TRANSPARENT_RED_VALUE_EXT is the same as GLX_TRANSPARENT_RED_VALUE
        // GLX_TRANSPARENT_GREEN_VALUE_EXT is the same as GLX_TRANSPARENT_GREEN_VALUE
        // GLX_TRANSPARENT_BLUE_VALUE_EXT is the same as GLX_TRANSPARENT_BLUE_VALUE
        // GLX_TRANSPARENT_ALPHA_VALUE_EXT is the same as GLX_TRANSPARENT_ALPHA_VALUE
        // GLX_NONE_EXT is the same as GLX_NONE
        // GLX_TRUE_COLOR_EXT is the same as GLX_TRUE_COLOR
        // GLX_DIRECT_COLOR_EXT is the same as GLX_DIRECT_COLOR
        // GLX_PSEUDO_COLOR_EXT is the same as GLX_PSEUDO_COLOR
        // GLX_STATIC_COLOR_EXT is the same as GLX_STATIC_COLOR
        // GLX_GRAY_SCALE_EXT is the same as GLX_GRAY_SCALE
        // GLX_STATIC_GRAY_EXT is the same as GLX_STATIC_GRAY
        // GLX_TRANSPARENT_RGB_EXT is the same as GLX_TRANSPARENT_RGB
        // GLX_TRANSPARENT_INDEX_EXT is the same as GLX_TRANSPARENT_INDEX

        // GLX_SGI_swap_control
        // no new enums

        // GLX_SGI_video_sync
        // no new enums

        // GLX_SGI_make_current_read
        // no new enums

        // GLX_SGIX_video_source
        // no new enums

        // GLX_EXT_visual_rating
        // GLX_VISUAL_CAVEAT_EXT is the same as GLX_CONFIG_CAVEAT
        // GLX_SLOW_VISUAL_EXT is the same as GLX_SLOW_CONFIG
        // GLX_NON_CONFORMANT_VISUAL_EXT is the same as GLX_NON_CONFORMANT_CONFIG
        // reuse GLX_NONE_EXT

        // GLX_EXT_import_context
        case GLX_SHARE_CONTEXT_EXT:
            valueString = L"GLX_SHARE_CONTEXT_EXT";
            break;

        // GLX_VISUAL_ID_EXT is the same as GLX_VISUAL_ID
        // GLX_SCREEN_EXT is the same as GLX_SCREEN

        // GLX_SGIX_fbconfig
        // GLX_DRAWABLE_TYPE_SGIX is the same as GLX_DRAWABLE_TYPE
        // GLX_RENDER_TYPE_SGIX is the same as GLX_RENDER_TYPE
        // GLX_X_RENDERABLE_SGIX is the same as GLX_X_RENDERABLE
        // GLX_FBCONFIG_ID_SGIX is the same as GLX_FBCONFIG_ID
        // GLX_RGBA_TYPE_SGIX is the same as GLX_RGBA_TYPE
        // GLX_COLOR_INDEX_TYPE_SGIX is the same as GLX_COLOR_INDEX_TYPE
        // reuse GLX_SCREEN_EXT

        // GLX_SGIX_pbuffer
        case GLX_BUFFER_CLOBBER_MASK_SGIX:
            valueString = L"GLX_BUFFER_CLOBBER_MASK_SGIX";
            break;

        // GLX_MAX_PBUFFER_WIDTH_SGIX is the same as GLX_MAX_PBUFFER_WIDTH
        // GLX_MAX_PBUFFER_HEIGHT_SGIX is the same as GLX_MAX_PBUFFER_HEIGHT
        // GLX_MAX_PBUFFER_PIXELS_SGIX is the same as GLX_MAX_PBUFFER_PIXELS

        case GLX_OPTIMAL_PBUFFER_WIDTH_SGIX:
            valueString = L"GLX_OPTIMAL_PBUFFER_WIDTH_SGIX";
            break;

        case GLX_OPTIMAL_PBUFFER_HEIGHT_SGIX:
            valueString = L"GLX_OPTIMAL_PBUFFER_HEIGHT_SGIX";
            break;

        // GLX_PRESERVED_CONTENTS_SGIX is the same as GLX_PRESERVED_CONTENTS
        // GLX_LARGEST_PBUFFER_SGIX is the same as GLX_LARGEST_PBUFFER
        // GLX_WIDTH_SGIX is the same as GLX_WIDTH
        // GLX_HEIGHT_SGIX is the same as GLX_HEIGHT
        // GLX_EVENT_MASK_SGIX is the same as GLX_EVENT_MASK
        // GLX_DAMAGED_SGIX is the same as GLX_DAMAGED
        // GLX_SAVED_SGIX is the same as GLX_SAVED
        // GLX_WINDOW_SGIX is the same as GLX_WINDOW
        // GLX_PBUFFER_SGIX is the same as GLX_PBUFFER

        // GLX_SGI_cushion
        // no new enums

        // GLX_SGIX_video_resize
        // no new enums

        // GLX_SGIX_dmbuffer
        case GLX_DIGITAL_MEDIA_PBUFFER_SGIX:
            valueString = L"GLX_DIGITAL_MEDIA_PBUFFER_SGIX";
            break;

        // GLX_SGIX_swap_group
        // no new enums

        // GLX_SGIX_swap_barrier
        // no new enums

        // GLX_SGIS_blended_overlay
        case GLX_BLENDED_RGBA_SGIS:
            valueString = L"GLX_BLENDED_RGBA_SGIS";
            break;

        // GLX_SGIS_shared_multisample
        case GLX_MULTISAMPLE_SUB_RECT_WIDTH_SGIS:
            valueString = L"GLX_MULTISAMPLE_SUB_RECT_WIDTH_SGIS";
            break;

        case GLX_MULTISAMPLE_SUB_RECT_HEIGHT_SGIS:
            valueString = L"GLX_MULTISAMPLE_SUB_RECT_HEIGHT_SGIS";
            break;

        // GLX_SUN_get_transparent_index
        // no new enums

        // GLX_3DFX_multisample
        case GLX_SAMPLE_BUFFERS_3DFX:
            valueString = L"GLX_SAMPLE_BUFFERS_3DFX";
            break;

        case GLX_SAMPLES_3DFX:
            valueString = L"GLX_SAMPLES_3DFX";
            break;

        // GLX_MESA_copy_sub_buffer
        // no new enums


        // GLX_MESA_pixmap_colormap
        // no new enums

        // GLX_MESA_release_buffers
        // no new enums

        // GLX_MESA_set_3dfx_mode
        // no new enums

        // GLX_SGIX_visual_select_group
        case GLX_VISUAL_SELECT_GROUP_SGIX:
            valueString = L"GLX_VISUAL_SELECT_GROUP_SGIX";
            break;

        // GLX_OML_swap_method
        case GLX_SWAP_METHOD_OML:
            valueString = L"GLX_SWAP_METHOD_OML";
            break;

        case GLX_SWAP_EXCHANGE_OML:
            valueString = L"GLX_SWAP_EXCHANGE_OML";
            break;

        case GLX_SWAP_COPY_OML:
            valueString = L"GLX_SWAP_COPY_OML";
            break;

        case GLX_SWAP_UNDEFINED_OML:
            valueString = L"GLX_SWAP_UNDEFINED_OML";
            break;

        // GLX_OML_sync_control
        // no new enums

        // GLX_NV_float_buffer
        case GLX_FLOAT_COMPONENTS_NV:
            valueString = L"GLX_FLOAT_COMPONENTS_NV";
            break;

        // GLX_SGIX_hyperpipe
        case GLX_HYPERPIPE_PIPE_NAME_LENGTH_SGIX:
            valueString = L"GLX_HYPERPIPE_PIPE_NAME_LENGTH_SGIX";
            break;

        case GLX_BAD_HYPERPIPE_CONFIG_SGIX:
            valueString = L"GLX_BAD_HYPERPIPE_CONFIG_SGIX";
            break;

        case GLX_BAD_HYPERPIPE_SGIX:
            valueString = L"GLX_BAD_HYPERPIPE_SGIX";
            break;

        case GLX_HYPERPIPE_ID_SGIX:
            valueString = L"GLX_HYPERPIPE_ID_SGIX";
            break;

        // GLX_MESA_agp_offset
        // no new enums

        // GLX_EXT_fbconfig_packed_float
        case GLX_RGBA_UNSIGNED_FLOAT_TYPE_EXT:
            valueString = L"GLX_RGBA_UNSIGNED_FLOAT_TYPE_EXT";
            break;

        // GLX_EXT_framebuffer_sRGB
        case GLX_FRAMEBUFFER_SRGB_CAPABLE_EXT:
            valueString = L"GLX_FRAMEBUFFER_SRGB_CAPABLE_EXT";
            break;

        // GLX_EXT_texture_from_pixmap
        case GLX_BIND_TO_TEXTURE_RGB_EXT:
            valueString = L"GLX_BIND_TO_TEXTURE_RGB_EXT";
            break;

        case GLX_BIND_TO_TEXTURE_RGBA_EXT:
            valueString = L"GLX_BIND_TO_TEXTURE_RGBA_EXT";
            break;

        case GLX_BIND_TO_MIPMAP_TEXTURE_EXT:
            valueString = L"GLX_BIND_TO_MIPMAP_TEXTURE_EXT";
            break;

        case GLX_BIND_TO_TEXTURE_TARGETS_EXT:
            valueString = L"GLX_BIND_TO_TEXTURE_TARGETS_EXT";
            break;

        case GLX_Y_INVERTED_EXT:
            valueString = L"GLX_Y_INVERTED_EXT";
            break;

        case GLX_TEXTURE_FORMAT_EXT:
            valueString = L"GLX_TEXTURE_FORMAT_EXT";
            break;

        case GLX_TEXTURE_TARGET_EXT:
            valueString = L"GLX_TEXTURE_TARGET_EXT";
            break;

        case GLX_MIPMAP_TEXTURE_EXT:
            valueString = L"GLX_MIPMAP_TEXTURE_EXT";
            break;

        case GLX_TEXTURE_FORMAT_NONE_EXT:
            valueString = L"GLX_TEXTURE_FORMAT_NONE_EXT";
            break;

        case GLX_TEXTURE_FORMAT_RGB_EXT:
            valueString = L"GLX_TEXTURE_FORMAT_RGB_EXT";
            break;

        case GLX_TEXTURE_FORMAT_RGBA_EXT:
            valueString = L"GLX_TEXTURE_FORMAT_RGBA_EXT";
            break;

        case GLX_TEXTURE_1D_EXT:
            valueString = L"GLX_TEXTURE_1D_EXT";
            break;

        case GLX_TEXTURE_2D_EXT:
            valueString = L"GLX_TEXTURE_2D_EXT";
            break;

        case GLX_TEXTURE_RECTANGLE_EXT:
            valueString = L"GLX_TEXTURE_RECTANGLE_EXT";
            break;

        case GLX_FRONT_LEFT_EXT:
            valueString = L"GLX_FRONT_LEFT_EXT";
            break;

        case GLX_FRONT_RIGHT_EXT:
            valueString = L"GLX_FRONT_RIGHT_EXT";
            break;

        case GLX_BACK_LEFT_EXT:
            valueString = L"GLX_BACK_LEFT_EXT";
            break;

        case GLX_BACK_RIGHT_EXT:
            valueString = L"GLX_BACK_RIGHT_EXT";
            break;

        // GLX_FRONT_EXT is the same as GLX_FRONT_LEFT_EXT
        // GLX_BACK_EXT is the same as GLX_BACK_LEFT_EXT

        case GLX_AUX0_EXT:
            valueString = L"GLX_AUX0_EXT";
            break;

        case GLX_AUX1_EXT:
            valueString = L"GLX_AUX1_EXT";
            break;

        case GLX_AUX2_EXT:
            valueString = L"GLX_AUX2_EXT";
            break;

        case GLX_AUX3_EXT:
            valueString = L"GLX_AUX3_EXT";
            break;

        case GLX_AUX4_EXT:
            valueString = L"GLX_AUX4_EXT";
            break;

        case GLX_AUX5_EXT:
            valueString = L"GLX_AUX5_EXT";
            break;

        case GLX_AUX6_EXT:
            valueString = L"GLX_AUX6_EXT";
            break;

        case GLX_AUX7_EXT:
            valueString = L"GLX_AUX7_EXT";
            break;

        case GLX_AUX8_EXT:
            valueString = L"GLX_AUX8_EXT";
            break;

        case GLX_AUX9_EXT:
            valueString = L"GLX_AUX9_EXT";
            break;

        // GLX_NV_present_video
        case GLX_NUM_VIDEO_SLOTS_NV:
            valueString = L"GLX_NUM_VIDEO_SLOTS_NV";
            break;

        // GLX_NV_video_out
        case GLX_VIDEO_OUT_COLOR_NV:
            valueString = L"GLX_VIDEO_OUT_COLOR_NV";
            break;

        case GLX_VIDEO_OUT_ALPHA_NV:
            valueString = L"GLX_VIDEO_OUT_ALPHA_NV";
            break;

        case GLX_VIDEO_OUT_DEPTH_NV:
            valueString = L"GLX_VIDEO_OUT_DEPTH_NV";
            break;

        case GLX_VIDEO_OUT_COLOR_AND_ALPHA_NV:
            valueString = L"GLX_VIDEO_OUT_COLOR_AND_ALPHA_NV";
            break;

        case GLX_VIDEO_OUT_COLOR_AND_DEPTH_NV:
            valueString = L"GLX_VIDEO_OUT_COLOR_AND_DEPTH_NV";
            break;

        case GLX_VIDEO_OUT_FRAME_NV:
            valueString = L"GLX_VIDEO_OUT_FRAME_NV";
            break;

        case GLX_VIDEO_OUT_FIELD_1_NV:
            valueString = L"GLX_VIDEO_OUT_FIELD_1_NV";
            break;

        case GLX_VIDEO_OUT_FIELD_2_NV:
            valueString = L"GLX_VIDEO_OUT_FIELD_2_NV";
            break;

        case GLX_VIDEO_OUT_STACKED_FIELDS_1_2_NV:
            valueString = L"GLX_VIDEO_OUT_STACKED_FIELDS_1_2_NV";
            break;

        case GLX_VIDEO_OUT_STACKED_FIELDS_2_1_NV:
            valueString = L"GLX_VIDEO_OUT_STACKED_FIELDS_2_1_NV";
            break;

        // GLX_NV_swap_group
        // no new enums

        default:
            // Unknown enum
            // (Usually because of the usage of an OpenGL extension that we don't support)
            gtString unknownEnumString;
            unknownEnumString.appendFormattedString(L"Unknown enum:  0x%X", _value);
            GT_ASSERT_EX(0, unknownEnumString.asCharArray());

            valueString = L"Unknown";
            break;
    }
}

// ---------------------------------------------------------------------------
// Name:        apGLXenumParameter::compareToOther
// Description: Compares this with other
// Return Val: true - the types and values are equal; false - otherwise
// Author:  AMD Developer Tools Team
// Date:        6/7/2009
// ---------------------------------------------------------------------------
bool apGLXenumParameter::compareToOther(const apParameter& other) const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apGLXenumParameter* pParam  = (apGLXenumParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLXenumParameter::setValueFromInt
// Description: Set my value from a GLint.
// Author:  AMD Developer Tools Team
// Date:        6/7/2009
// ---------------------------------------------------------------------------
void apGLXenumParameter::setValueFromInt(GLint val)
{
    _value = (int)val;
}


// ---------------------------------------------------------------------------
// Name:        apGLXenumParameter::setValueFromFloat
// Description: Set my value from a GLfloat.
// Author:  AMD Developer Tools Team
// Date:        6/7/2009
// ---------------------------------------------------------------------------
void apGLXenumParameter::setValueFromFloat(GLfloat val)
{
    _value = (int)val;
}

