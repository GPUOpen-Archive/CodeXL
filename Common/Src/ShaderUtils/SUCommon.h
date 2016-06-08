//=====================================================================
// Copyright 2010-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SUCommon.h
/// \brief  This file defines enums and structs used in ShaderDebugger
///         and APP Profiler.
///
//=====================================================================
//=====================================================================
// $Id: //devtools/main/Common/Src/ShaderUtils/SUCommon.h#9 $
// Last checkin:   $DateTime: 2016/04/18 06:01:26 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569612 $
//=====================================================================

#ifndef _SU_COMMON_H_
#define _SU_COMMON_H_

#include <cassert>
#include <cstdlib>

// TODO: Figure out a way to support the following
#define SU_Assert assert
#define SU_Verify
#define SU_Break assert

#define hidden_quote( s ) #s
#define hidden_numquote( n ) hidden_quote( n )

#if defined (_WIN32)

    #define SU_TODO(x)  __pragma( message( __FILE__ "(" hidden_numquote( __LINE__ ) "): TODO: " x ) )

#elif defined (__linux__)

    #if __GNUC_PREREQ(4,1)
        // GCC 4.1.2 expands this incorrectly - nullify it
        #define SU_TODO(x)
    #else
        // Macros do not seem to directly expand on Linux in #pragma statements
        #define DO_PRAGMA(x)    _Pragma(#x)
        #define SU_TODO(x)  DO_PRAGMA( message( __FILE__ "(" hidden_numquote( __LINE__ ) "): TODO: " x ) )
    #endif

#elif defined (__CYGWIN__)

    #define SU_TODO(x)

#endif

namespace ShaderUtils
{

template<typename T>
inline void SU_SAFE_FREE(T& p)
{
    free(p);
    p = NULL;
}


template<typename T>
inline void SU_SAFE_DELETE(T& p)
{
    delete p;
    p = NULL;
}


/// The type of shader.
typedef enum
{
    ST_Unknown,       ///< Unknown shader type.
    ST_Pixel,         ///< Pixel shader.
    ST_Vertex,        ///< Vertex shader.
    ST_Geometry,      ///< Geometry shader.
    ST_Geometry_SO,   ///< Geometry shader with stream output.
    ST_Hull,          ///< Hull shader.
    ST_Domain,        ///< Domain shader.
    ST_Compute,       ///< Compute shader.
} ShaderType;

/// Type describing RGBA_32F buffer format.
typedef struct RGBA_32F
{
    RGBA_32F(float val = 0) { r = g = b = a = val; }
    float r, g, b, a;
} RGBA_32F;

/// Equality operator for comparing instances of RGBA_32F.
inline bool operator==(const RGBA_32F& lhs, const RGBA_32F& rhs)
{
    return (lhs.r == rhs.r) &&
           (lhs.g == rhs.g) &&
           (lhs.b == rhs.b) &&
           (lhs.a == rhs.a);
}

/// Inequality operator for comparing instances of RGBA_32F.
inline bool operator!=(const RGBA_32F& lhs, const RGBA_32F& rhs)
{
    return !(lhs == rhs);
}

/// Type describing RGBA_16 buffer format.
typedef struct
{
    unsigned short r, g, b, a;
} RGBA_16;

/// Type describing RGB_32 buffer format.
typedef struct
{
    unsigned int r, g, b;
} RGB_32;

/// Type describing RGB_16 buffer format.
typedef struct
{
    unsigned short r, g, b;
} RGB_16;

/// Type describing XYZW_32F buffer format.
typedef struct
{
    float x, y, z, w;
} XYZW_32F;


/// The data type.
typedef enum
{
    DT_Unknown,    ///< Unknown data type.
    DT_Float32,    ///< 32-bit floating point.
    DT_Int32,      ///< 32-bit integer (signed-ness unknown).
    DT_SInt32,     ///< 32-bit signed integer.
    DT_UInt32,     ///< 32-bit unsigned integer.
    DT_Boolean,    ///< Boolean.
} DataType;

/// The type of thread ID.
typedef enum
{
    TID_Unknown,   ///< Unknown thread ID type.
    TID_1D,        ///< 1D thread ID.
    TID_2D,        ///< 2D thread ID.
    TID_3D,        ///< 3D thread ID.
} ThreadIDType;

/// The ID of a thread.
typedef struct ThreadID
{
    ThreadIDType type;   ///< The thread ID type.
    int x;               ///< The x component of the thread ID.
    int y;               ///< The y component of the thread ID.
    int z;               ///< The z component of the thread ID.
    int nSubResource;    ///< The sub-resource component of the thread ID.

    /// Default constructor.
    ThreadID() { Clear(); };

    /// Constructor.
    /// \param[in] _type          The thread ID type.
    /// \param[in] _x             The x component of the thread ID.
    /// \param[in] _y             The y component of the thread ID.
    /// \param[in] _z             The z component of the thread ID.
    /// \param[in] _nSubResource  The sub-resource component of the thread ID.
    ThreadID(ThreadIDType _type, int _x, int _y, int _z, int _nSubResource)
    {
        type = _type; x = _x; y = _y; z = _z; nSubResource = _nSubResource;
    };

    /// Clear the thread ID to default values.
    void Clear() { type = TID_2D; x = y = z = 0; nSubResource = 0; };
} ThreadID;

extern const ThreadID g_tID_Error;

/// A null thread. Used for comparisons.
extern const ThreadID g_tID_Error;

/// Is the thread ID valid?
/// \param[in] threadID The thread ID to check for validity.
/// \return    True if threadID is valid, otherwise false.
bool IsValid(const ThreadID& threadID);
}

#endif //_SU_COMMON_H_
