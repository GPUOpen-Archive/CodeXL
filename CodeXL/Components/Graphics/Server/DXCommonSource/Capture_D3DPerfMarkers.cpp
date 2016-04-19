//=============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  PerfMarkers capture classes implementation
//=============================================================================

#include "windows.h"
#include <string.h>
#include "d3d9.h"
#include "Interceptor.h"
#include "../Common/Logger.h"
#include "../Common/misc.h"
#include "../Common/mymutex.h"
#include "HookHelpers.h"
#include "../Common/CaptureStream.h"
#include "../Common/CaptureLayer.h"
#include "Capture_D3DPerfMarkers_Classes.h"

typedef void (WINAPI* D3DPERF_BeginEvent_type)(D3DCOLOR col, LPCWSTR wszName);
typedef void (WINAPI* D3DPERF_EndEvent_type)();
typedef void (WINAPI* D3DPERF_SetMarker_type)(D3DCOLOR col, LPCWSTR wszName);
typedef void (WINAPI* D3DPERF_SetRegion_type)(D3DCOLOR col, LPCWSTR wszName);

static D3DPERF_BeginEvent_type Real_D3DPERF_BeginEvent = nullptr;
static D3DPERF_EndEvent_type Real_D3DPERF_EndEvent = nullptr;
static D3DPERF_SetMarker_type Real_D3DPERF_SetMarker = nullptr;
static D3DPERF_SetRegion_type Real_D3DPERF_SetRegion = nullptr;

static DWORD dwAttached = 0;
static mutex s_mtx;

static CaptureLayer* s_pCaptureLayer = nullptr;

//-----------------------------------------------------------------------------
/// makes the captured call, the call doesnt necessarily need to be the Real call as in this function
/// \return return true, although nothing is done with the returned parameter
//-----------------------------------------------------------------------------
bool Capture_D3DPERF_BeginEvent::Play()
{
    if (Real_D3DPERF_BeginEvent != 0)
    {
        Real_D3DPERF_BeginEvent(m_color, m_wszName);
    }

    return true;
}

//-----------------------------------------------------------------------------
/// makes the captured call, the call doesnt necessarily need to be the Real call as in this function
/// \return return true, although nothing is done with the returned parameter
//-----------------------------------------------------------------------------
bool Capture_D3DPERF_EndEvent::Play()
{
    if (Real_D3DPERF_EndEvent != 0)
    {
        Real_D3DPERF_EndEvent();
    }

    return true;
}

//-----------------------------------------------------------------------------
/// makes the captured call, the call doesnt necessarily need to be the Real call as in this function
/// \return return true, although nothing is done with the returned parameter
//-----------------------------------------------------------------------------
bool Capture_D3DPERF_SetRegion::Play()
{
    if (Real_D3DPERF_SetRegion != 0)
    {
        Real_D3DPERF_SetRegion(m_color, m_wszName);
    }

    return true;
}

//-----------------------------------------------------------------------------
/// makes the captured call, the call doesnt necessarily need to be the Real call as in this function
/// \return return true, although nothing is done with the returned parameter
//-----------------------------------------------------------------------------
bool Capture_D3DPERF_SetMarker::Play()
{
    if (Real_D3DPERF_SetMarker != 0)
    {
        Real_D3DPERF_SetMarker(m_color, m_wszName);
    }

    return true;
}

//-----------------------------------------------------------------------------
/// Captures the BeginEvent calls made by the 3D application
/// \param col color of the event
/// \param col name of the event
//-----------------------------------------------------------------------------
static void WINAPI Mine_D3DPERF_BeginEvent(D3DCOLOR col, LPCWSTR wszName)
{
    ScopeLock t(&s_mtx);

    Capture_D3DPERF_BeginEvent* pCap = new Capture_D3DPERF_BeginEvent();

    // captures the parameters
    pCap->OnCreate(col, wszName);

    if (s_pCaptureLayer != nullptr)
    {
        // add captured call to the capture list, that is hold by the layer
        s_pCaptureLayer->GetCapturedAPICalls()->Add(pCap);
    }

    // makes the real call
    Real_D3DPERF_BeginEvent(col, wszName);
}

//-----------------------------------------------------------------------------
/// Captures D3DPERFEndEvent() calls
//-----------------------------------------------------------------------------
static void WINAPI Mine_D3DPERF_EndEvent()
{
    ScopeLock t(&s_mtx);

    Capture_D3DPERF_EndEvent* pCap = new Capture_D3DPERF_EndEvent();
    pCap->OnCreate();

    if (s_pCaptureLayer != nullptr)
    {
        s_pCaptureLayer->GetCapturedAPICalls()->Add(pCap);
    }

    Real_D3DPERF_EndEvent();
}


//-----------------------------------------------------------------------------
/// Captures D3DPERF_SetMarker() calls
//-----------------------------------------------------------------------------
static void WINAPI Mine_D3DPERF_SetMarker(D3DCOLOR col, LPCWSTR wszName)
{
    ScopeLock t(&s_mtx);

    Capture_D3DPERF_SetMarker* pCap = new Capture_D3DPERF_SetMarker();
    pCap->OnCreate(col, wszName);

    if (s_pCaptureLayer != nullptr)
    {
        s_pCaptureLayer->GetCapturedAPICalls()->Add(pCap);
    }

    Real_D3DPERF_SetMarker(col, wszName);
}

//-----------------------------------------------------------------------------
/// Captures D3DPERF_SetRegion calls
/// \param col color of the event
/// \param col name of the event
//-----------------------------------------------------------------------------
static void WINAPI Mine_D3DPERF_SetRegion(D3DCOLOR col, LPCWSTR wszName)
{
    ScopeLock t(&s_mtx);

    Capture_D3DPERF_SetRegion* pCap = new Capture_D3DPERF_SetRegion();
    pCap->OnCreate(col, wszName);

    if (s_pCaptureLayer != nullptr)
    {
        s_pCaptureLayer->GetCapturedAPICalls()->Add(pCap);
    }

    Real_D3DPERF_SetRegion(col, wszName);
}

//-----------------------------------------------------------------------------
/// After this call all D3DPerf calls will be captured and appended to a list
/// \param pCaptureLayer pointer to the layer where captured calls will be stored
//-----------------------------------------------------------------------------
LONG CAPTURE_D3DPerfMarkers_Hook(CaptureLayer* pCaptureLayer)
{
    LogTrace(traceENTER, "");

    ScopeLock t(&s_mtx);

    if (dwAttached > 0)
    {
        Log(logERROR, "Trying to attach twice!\n");
        return -1;
    }

    s_pCaptureLayer = pCaptureLayer;

    HMODULE hModule = GetModuleHandle("d3d9.dll");

    if (hModule == 0)
    {
        hModule = LoadLibrary("d3d9.dll");
    }

    Real_D3DPERF_BeginEvent  = (D3DPERF_BeginEvent_type)GetProcAddress(hModule, "D3DPERF_BeginEvent");
    Real_D3DPERF_EndEvent    = (D3DPERF_EndEvent_type)GetProcAddress(hModule, "D3DPERF_EndEvent");
    Real_D3DPERF_SetMarker   = (D3DPERF_SetMarker_type)GetProcAddress(hModule, "D3DPERF_SetMarker");
    Real_D3DPERF_SetRegion   = (D3DPERF_SetRegion_type)GetProcAddress(hModule, "D3DPERF_SetRegion");

    LONG error;
    AMDT::BeginHook();

    if (Real_D3DPERF_BeginEvent != nullptr)
    {
        error = AMDT::HookAPICall(&(PVOID&)Real_D3DPERF_BeginEvent, Mine_D3DPERF_BeginEvent);
        PsAssert(error == NO_ERROR);
    }

    if (Real_D3DPERF_EndEvent != nullptr)
    {
        error = AMDT::HookAPICall(&(PVOID&)Real_D3DPERF_EndEvent, Mine_D3DPERF_EndEvent);
        PsAssert(error == NO_ERROR);
    }

    if (Real_D3DPERF_SetMarker != nullptr)
    {
        error = AMDT::HookAPICall(&(PVOID&)Real_D3DPERF_SetMarker, Mine_D3DPERF_SetMarker);
        PsAssert(error == NO_ERROR);
    }

    if (Real_D3DPERF_SetRegion != nullptr)
    {
        error = AMDT::HookAPICall(&(PVOID&)Real_D3DPERF_SetRegion, Mine_D3DPERF_SetRegion);
        PsAssert(error == NO_ERROR);
    }

    error = AMDT::EndHook();

    if (error != NO_ERROR)
    {
        Log(logERROR, "failed\n");
    }
    else
    {
        dwAttached++;
    }

    LogTrace(traceEXIT, "");

    return error;
}

//-----------------------------------------------------------------------------
/// Stops capturing D3DPerf markers
//-----------------------------------------------------------------------------
LONG CAPTURE_D3DPerfMarkers_Unhook()
{
    LogTrace(traceENTER, "");

    ScopeLock t(&s_mtx);

    if (dwAttached <= 0)
    {
        Log(logERROR, "Trying to Detach ID3D10Device twice!\n");
        return -1;
    }

    s_pCaptureLayer = nullptr;

    LONG error;
    AMDT::BeginHook();

    if (Real_D3DPERF_BeginEvent != nullptr)
    {
        error = AMDT::UnhookAPICall(&(PVOID&)Real_D3DPERF_BeginEvent, Mine_D3DPERF_BeginEvent);
        PsAssert(error == NO_ERROR);
    }

    if (Real_D3DPERF_EndEvent != nullptr)
    {
        error = AMDT::UnhookAPICall(&(PVOID&)Real_D3DPERF_EndEvent, Mine_D3DPERF_EndEvent);
        PsAssert(error == NO_ERROR);
    }

    if (Real_D3DPERF_SetMarker != nullptr)
    {
        error = AMDT::UnhookAPICall(&(PVOID&)Real_D3DPERF_SetMarker, Mine_D3DPERF_SetMarker);
        PsAssert(error == NO_ERROR);
    }

    if (Real_D3DPERF_SetRegion != nullptr)
    {
        error = AMDT::UnhookAPICall(&(PVOID&)Real_D3DPERF_SetRegion, Mine_D3DPERF_SetRegion);
        PsAssert(error == NO_ERROR);
    }

    error = AMDT::EndHook();

    if (error != NO_ERROR)
    {
        Log(logERROR, "failed\n");
    }
    else
    {
        dwAttached--;
    }

    LogTrace(traceEXIT, "");

    return error;
}

