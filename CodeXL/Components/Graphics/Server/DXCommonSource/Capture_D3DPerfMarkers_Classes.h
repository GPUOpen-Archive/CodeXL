//=============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  D3D PerfMarkers capture classes
//=============================================================================

#ifndef CAPTUREPERFMARKERS_CLASSES_H
#define CAPTUREPERFMARKERS_CLASSES_H
#include "windows.h"
#include <string.h>
#include "d3d9.h"
#include "../Common/misc.h"
#include "D3DCapture.h"

//-----------------------------------------------------------------------------
/// Class that captures D3DPERF_BeginEvent calls
//-----------------------------------------------------------------------------
class Capture_D3DPERF_BeginEvent : public D3DCapture
{
    /// parameters of the BeginEvent call
    D3DCOLOR m_color;
    LPWSTR m_wszName; ///< DOCUMENTATION REQUIRED

public:

    //-----------------------------------------------------------------------------
    /// initializes pointers
    //-----------------------------------------------------------------------------
    Capture_D3DPERF_BeginEvent()
    {
        m_wszName = nullptr;
    }

    //-----------------------------------------------------------------------------
    /// records parameters used by the call
    //-----------------------------------------------------------------------------
    void OnCreate(D3DCOLOR color, LPCWSTR wszName)
    {
        //record first parameter, the event's color
        m_color = color;

        //record second parameter (the events name), note that we allocate a buffer to copy this string
        DWORD len = (DWORD)(wcslen(wszName) + 1);
        m_wszName = new WCHAR[ len ];
        wcscpy_s(m_wszName, len, wszName);
    }

    //-----------------------------------------------------------------------------
    /// records parameters used by the call
    //-----------------------------------------------------------------------------
    void OnCreate(D3DCOLOR color, const char* str)
    {
        //record first parameter, the event's color
        m_color = color;

        //record second parameter (the events name), note that we allocate a buffer to copy this string
        DWORD len = (DWORD)(strlen(str) + 1);
        m_wszName = new WCHAR[ len ];
        swprintf(m_wszName, len, L"%S", str);
    }

    //-----------------------------------------------------------------------------
    /// releases the events name
    //-----------------------------------------------------------------------------
    ~Capture_D3DPERF_BeginEvent()
    {
        delete m_wszName;
    }

    //-----------------------------------------------------------------------------
    /// this function is defined in the cpp, it just replays the captured call
    //-----------------------------------------------------------------------------
    bool Play();

    //-----------------------------------------------------------------------------
    /// \return the class' type
    //-----------------------------------------------------------------------------
    virtual CaptureClassType GetClassType()
    {
        return CCT_PerfMarker;
    }

    //-----------------------------------------------------------------------------
    /// \return a string with the function name and parameters
    //-----------------------------------------------------------------------------
    std::string Print()
    {
        return FormatString("D3DPERF_BeginEvent( 0x%x, %S )", m_color, m_wszName);
    }
};

//-----------------------------------------------------------------------------
/// Class that captures D3DPERF_EndEvent calls
//-----------------------------------------------------------------------------
class Capture_D3DPERF_EndEvent : public D3DCapture
{
public:
    //-----------------------------------------------------------------------------
    /// no parameters to capture
    //-----------------------------------------------------------------------------
    void OnCreate()
    {
    }

    //-----------------------------------------------------------------------------
    /// \return the class' type
    //-----------------------------------------------------------------------------
    bool Play();

    //-----------------------------------------------------------------------------
    /// \return the class' type
    //-----------------------------------------------------------------------------
    virtual CaptureClassType GetClassType()
    {
        return CCT_PerfMarker;
    }

    //-----------------------------------------------------------------------------
    /// \return a string with the function name and parameters
    //-----------------------------------------------------------------------------
    std::string Print()
    {
        return FormatString("D3DPERF_EndEvent()");
    }
};

//-----------------------------------------------------------------------------
/// Class that captures D3DPERF_SetRegion calls
//-----------------------------------------------------------------------------
class Capture_D3DPERF_SetRegion : public Capture
{
    // parameters of the BeginEvent call
    D3DCOLOR m_color;
    LPWSTR m_wszName; ///< DOCUMENTATION REQUIRED

public:
    //-----------------------------------------------------------------------------
    /// initializes pointers
    //-----------------------------------------------------------------------------
    Capture_D3DPERF_SetRegion()
    {
        m_wszName = nullptr;
    }

    //-----------------------------------------------------------------------------
    /// records parameters used by the call
    //-----------------------------------------------------------------------------
    void OnCreate(D3DCOLOR color, LPCWSTR wszName)
    {
        m_color = color;
        DWORD len = (DWORD)(wcslen(wszName) + 1);
        m_wszName = new WCHAR[ len ];
        wcscpy_s(m_wszName, len, wszName);
    }

    //-----------------------------------------------------------------------------
    /// releases region's name
    //-----------------------------------------------------------------------------
    ~Capture_D3DPERF_SetRegion()
    {
        delete m_wszName;
    }

    //-----------------------------------------------------------------------------
    /// this function is defined in the cpp, it just replays the captured call
    //-----------------------------------------------------------------------------
    bool Play();

    //-----------------------------------------------------------------------------
    /// \return the class' type
    //-----------------------------------------------------------------------------
    virtual CaptureClassType GetClassType()
    {
        return CCT_PerfMarker;
    }

    //-----------------------------------------------------------------------------
    /// \return a string with the function name and parameters
    //-----------------------------------------------------------------------------
    std::string Print()
    {
        return FormatString("D3DPERF_SetRegion( 0x%x, %S )", m_color, m_wszName);
    }
};

//-----------------------------------------------------------------------------
/// Class that captures D3DPERF_SetMarker calls
//-----------------------------------------------------------------------------
class Capture_D3DPERF_SetMarker : public Capture
{
    //-----------------------------------------------------------------------------
    /// parameters of the BeginEvent call
    //-----------------------------------------------------------------------------
    D3DCOLOR m_color;
    LPWSTR m_wszName; ///< DOCUMENTATION REQUIRED

public:
    //-----------------------------------------------------------------------------
    /// records parameters used by the call
    //-----------------------------------------------------------------------------
    void OnCreate(D3DCOLOR color, LPCWSTR wszName)
    {
        m_color = color;
        DWORD len = (DWORD)(wcslen(wszName) + 1);
        m_wszName = new WCHAR[ len ];
        wcscpy_s(m_wszName, len, wszName);
    }

    //-----------------------------------------------------------------------------
    /// releases marker name
    //-----------------------------------------------------------------------------
    ~Capture_D3DPERF_SetMarker()
    {
        delete m_wszName;
    }

    //-----------------------------------------------------------------------------
    /// this function is defined in the cpp, it just replays the captured call
    //-----------------------------------------------------------------------------
    bool Play();

    //-----------------------------------------------------------------------------
    /// \return the class' type
    //-----------------------------------------------------------------------------
    virtual CaptureClassType GetClassType()
    {
        return CCT_PerfMarker;
    }

    //-----------------------------------------------------------------------------
    /// \return a string with the function name and parameters
    //-----------------------------------------------------------------------------
    std::string Print()
    {
        return FormatString("D3DPERF_SetMarker( 0x%x, %S )", m_color, m_wszName);
    }
};

#endif
