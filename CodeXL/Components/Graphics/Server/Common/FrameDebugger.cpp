//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  The API-agnostic FrameDebugger base class which should be inherited from for API-specific implementations
//==============================================================================

#include "FrameDebugger.h"
#include "CommandStrings.h"
#include "parser.h"
#include "xml.h"
#include "SharedGlobal.h"

//--------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------
FrameDebugger::FrameDebugger() :
    m_dwBreakPoint(0),
    m_ulDrawCallCounter(0),
    m_LastDrawCall(nullptr),
    m_bAutoRenderTarget(true),
    m_bForceClear(true)
{
    m_ClearColor[0] = 0.0f; // red
    m_ClearColor[1] = 0.4f; // green
    m_ClearColor[2] = 0.8f; // blue
    m_ClearColor[3] = 1.0f; // alpha

    m_iWireframeColor = SG_GET_INT(OptionWireFrameColor);

    m_bWireframeOverlay = SG_GET_BOOL(OptionWireFrameOverlay);

    // 1 = pink/purple, 2 = green, 3 = blue
    if (2 == m_iWireframeColor)
    {
        // green
        m_fWireframeOverlayColor[0] = 0.0f;
        m_fWireframeOverlayColor[1] = 1.0f;
        m_fWireframeOverlayColor[2] = 0.0f;
    }
    else if (3 == m_iWireframeColor)
    {
        // blue
        m_fWireframeOverlayColor[0] = 0.0f;
        m_fWireframeOverlayColor[1] = 0.0f;
        m_fWireframeOverlayColor[2] = 1.0f;
    }
    else
    {
        // 1 or other
        // pink / purple
        m_fWireframeOverlayColor[0] = 1.0f;
        m_fWireframeOverlayColor[1] = 0.0f;
        m_fWireframeOverlayColor[2] = 1.0f;
    }

    m_fWireframeOverlayColor[3] = 1.0f;

    AddCommand(CONTENT_XML, "breakpoint",          "BreakPoint",            CMD_BREAKPOINT,           NO_DISPLAY, INCLUDE, m_dwBreakPoint);
    AddCommand(CONTENT_XML, "autorendertarget",    "Auto Render Target",    CMD_AUTORENDERTARGET,     NO_DISPLAY, INCLUDE, m_bAutoRenderTarget);
    AddCommand(CONTENT_XML, "forceclear",          "Force Clear",           CMD_FORCECLEAR,           NO_DISPLAY, INCLUDE, m_bForceClear);
    AddCommand(CONTENT_XML, "wireframeoverlay",    "WireFrame Overlay",     CMD_WIREFRAMEOVERLAY,     NO_DISPLAY, INCLUDE, m_bWireframeOverlay);
    AddCommand(CONTENT_XML, "stats",               "Stats",                 CMD_STATISTICS,           NO_DISPLAY, INCLUDE, m_Stats);
    AddCommand(CONTENT_XML, "currentdrawcall",     "Current Draw Call",     CMD_CURRENTDRAWCALL,      NO_DISPLAY, INCLUDE, m_CurrentDrawCall);
    AddCommand(CONTENT_XML, "DrawCallList",        "DrawCallList",          "DrawCallList.xml",       NO_DISPLAY, INCLUDE, m_drawCallList);
    AddCommand(CONTENT_XML, "ConfigHUD",           "Config HUD",            CMD_CONFIGHUD,            NO_DISPLAY, INCLUDE, m_bConfigHUD);

    SetLayerName("FrameDebugger");
}


//--------------------------------------------------------------------------
/// Initializes FrameDebugger per-frame values and does ForceClear if enabled
//--------------------------------------------------------------------------
void FrameDebugger::BeginFrame()
{
    // only force clear if both the frameDebugger is enabled and ForceClear is enabled
    if (m_bForceClear)
    {
        DoForceClear(m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], m_ClearColor[3]);
    }

    if (m_drawCallList.IsActive())
    {
        m_drawCallList.makeEmpty();
    }

    ResetDrawCallCount();
}

//--------------------------------------------------------------------------
/// Responds to active requests
//--------------------------------------------------------------------------
void FrameDebugger::EndFrame()
{
    if (m_drawCallList.IsActive())
    {
        // In GL m_LastDrawCall is a pointer to an auto variable. The Auto variable has gone out of scope when m_LastDrawCall is used and causes a crash in GL.
        // Commenting this out until a fix is available.
        // m_drawCallList.Send( m_LastDrawCall->GetHashCategories() + m_drawCallList);
        m_drawCallList.Send(m_drawCallList.asCharArray());
    }

    if (m_Stats.IsActive())
    {
        gtASCIIString out;
        out = XML("TotalDrawCallCount", m_ulDrawCallCounter);
        out += XML("BreakPoint", m_dwBreakPoint);

        m_Stats.Send(out.asCharArray());
    }
}

//--------------------------------------------------------------------------
/// Is the current draw call index enabled.
/// Note: the increment on m_ulDrawCallCounter is deliberate so that the
/// index being checked is the same as the index being checked from
/// OnDrawCall
/// \return true if enabled, false otherwise
//--------------------------------------------------------------------------
bool FrameDebugger::DrawCallEnabled()
{
    return IsDrawCallEnabled(m_ulDrawCallCounter + 1);
}

//--------------------------------------------------------------------------
/// Should be called by an API-specific FrameDebugger at each drawcall
/// \param rDrawCall the API-specific DrawCall that is being executed
//--------------------------------------------------------------------------
void FrameDebugger::OnDrawCall(IDrawCall& rDrawCall)
{
    m_ulDrawCallCounter++;

    if (m_drawCallList.IsActive())
    {
        m_LastDrawCall = &rDrawCall;
        gtASCIIString xmlString = rDrawCall.GetXML();
        xmlString += XML("hash", rDrawCall.GetHash().asCharArray());
        m_drawCallList += GetDrawCallXML(m_ulDrawCallCounter, xmlString.asCharArray()) ;
    }

    // Could collect drawcall XML here if desired
    if (IsDrawCallEnabled(m_ulDrawCallCounter))
    {
        // Make sure to not render after the frame debugger's break point
        if (IsTargetDrawCall(m_ulDrawCallCounter) == false)
        {
            // execute the drawcall for the app
            rDrawCall.Execute();
        }
        else
        {
            // have the pipeline process non-hud commands before drawing anything on HUD
            OnDrawCallAtBreakPointPreHUD(rDrawCall);

            // allow the API-specific FrameDebugger to decide whether or not to do the drawcall at the breakpoint
            DoDrawCallAtBreakPoint(rDrawCall);

            // Send the current draw call back to the client.
            if (m_CurrentDrawCall.IsActive())
            {
                m_CurrentDrawCall.Send(rDrawCall.GetXML().asCharArray());
            }

            bool bRes = BeginHUD();
            PsAssert(bRes != false);

            if (bRes == false)
            {
                Log(logERROR, "BeginHUD() failed");
                return;
            }

            // show the wireframe overlay
            if (m_bWireframeOverlay)
            {
                DoWireframeOverlay(rDrawCall, m_fWireframeOverlayColor[0], m_fWireframeOverlayColor[1], m_fWireframeOverlayColor[2], m_fWireframeOverlayColor[3]);
            }

            if (m_bAutoRenderTarget)
            {
                DoAutoRenderTarget();
            }

            OnDrawCallAtBreakPoint(rDrawCall);

            EndHUD();

            // skip the rest of the draws if HUD is being configured
            if (false == m_bConfigHUD)
            {
                // When the frame debugger is enabled we process the messages from inside the FD and no longer re-render the Frame Capture. This speeds up processing of messages
                // in apps that are running very slowly
                for (int ProcessedCommands = 0;;)
                {
                    if (m_drawCallList.IsActive() == true || m_Stats.IsActive() == true)
                    {
                        break;
                    }

                    // have the pipeline process non-hud commands before drawing anything on HUD
                    OnDrawCallAtBreakPointPreHUD(rDrawCall);
                    OnDrawCallAtBreakPoint(rDrawCall);

                    gtASCIIString SuCmd = PeekPendingRequests();

                    // Check for non FD command, if this changes then we have to re-render the HUD etc. So we break out
                    if (SuCmd.length() != 0)
                    {
                        char* pCmd = (char*)SuCmd.asCharArray();
                        ProcessedCommands++;

                        if (strstr(pCmd, "FD/Pipeline") == NULL)
                        {
                            LogConsole(logMESSAGE, "Processed in FD %3u\n", ProcessedCommands);
                            break;
                        }

                        GetSinglePendingRequest();
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------
/// Determines if the specified drawcall should be executed
/// \param uDrawCall index of the drawcall that should be compared against
///    the breakpoint to see if it should be executed
/// \return true if the draw call should be executed; false otherwise
//--------------------------------------------------------------------------
bool FrameDebugger::IsDrawCallEnabled(unsigned long uDrawCall)
{
    // if the frame debugger is enabled, then compare against breakpoint
    return uDrawCall <= m_dwBreakPoint;
}

//--------------------------------------------------------------------------
/// Determines if the specified drawcall is the breakpoint (target) drawcall
/// \param uDrawCall index of the drawcall that should be compared against
///    the breakpoint to see if it is the target
/// \return true if the draw call should be executed as the target; false otherwise
//--------------------------------------------------------------------------
bool FrameDebugger::IsTargetDrawCall(unsigned long uDrawCall)
{
    // if the frame debugger is enabled, then compare against breakpoint
    return uDrawCall == m_dwBreakPoint;
}

//--------------------------------------------------------------------------
/// Gets if auto render target is On or Off.
/// \return True if on, False if off.
//--------------------------------------------------------------------------
bool FrameDebugger::GetAutoRenderTarget()
{
    return m_bAutoRenderTarget;
}
