//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  The API-agnostic FrameDebugger base class which should be inherited from for API-specific implementations
//==============================================================================

#ifndef FRAMEDEBUGGER_H
#define FRAMEDEBUGGER_H

#include "ILayer.h"
#include "CommandProcessor.h"
#include "IDrawCall.h"
#include "HTTPLogger.h"

//=============================================================================
/// A Common class which contains most of the logic and functionality for a
/// FrameDebugger Layer to be used in GPU PerfStudio 2. It does a ForceClear at
/// the beginning of each frame, can collect drawcall information, render
/// wireframe overlay at the breakpoint, display render targets on the HUD,
/// and collect information on all of the draw calls.
//=============================================================================
class FrameDebugger: public ILayer, public CommandProcessor
{
public:

    //--------------------------------------------------------------------------
    /// Constructor
    //--------------------------------------------------------------------------
    FrameDebugger();

    //--------------------------------------------------------------------------
    /// Destructor
    //--------------------------------------------------------------------------
    virtual ~FrameDebugger() {}

    //--------------------------------------------------------------------------
    /// Initializes FrameDebugger per-frame values and does ForceClear if enabled
    //--------------------------------------------------------------------------
    virtual void BeginFrame();

    //--------------------------------------------------------------------------
    /// Responds to active requests
    //--------------------------------------------------------------------------
    virtual void EndFrame();

    //--------------------------------------------------------------------------
    /// Should be called by an API-specific FrameDebugger at each drawcall
    /// \param rDrawCall the API-specific DrawCall that is being executed
    //--------------------------------------------------------------------------
    void OnDrawCall(IDrawCall& rDrawCall);

    //--------------------------------------------------------------------------
    /// Gets if auto render target is On or Off.
    /// \return True if on, False if off.
    //--------------------------------------------------------------------------
    bool GetAutoRenderTarget();

    //--------------------------------------------------------------------------
    /// Gets the current break point index.
    /// \return Index of current break point
    //--------------------------------------------------------------------------
    unsigned long GetBreakPoint()
    {
        return m_dwBreakPoint;
    }

    //--------------------------------------------------------------------------
    /// Is the current draw call index enabled
    /// \return true if enabled, false otherwise
    //--------------------------------------------------------------------------
    bool DrawCallEnabled();

private:

    //--------------------------------------------------------------------------
    /// Hook to force a clear on the active rendertargets
    /// \param fRed The color to set the red channel
    /// \param fGreen The color to set the green channel
    /// \param fBlue The color to set the blue channel
    /// \param fAlpha The color to set the alpha channel
    //--------------------------------------------------------------------------
    virtual void DoForceClear(float fRed, float fGreen, float fBlue, float fAlpha) = 0;

    //--------------------------------------------------------------------------
    /// Hook to draw a wireframe overlay of the specified draw call
    /// \param rDrawCall the drawcall to render as a wireframe overlay
    /// \param fRed The color to set the red channel of the overlay
    /// \param fGreen The color to set the green channel of the overlay
    /// \param fBlue The color to set the blue channel of the overlay
    /// \param fAlpha The color to set the alpha channel of the overlay
    //--------------------------------------------------------------------------
    virtual void DoWireframeOverlay(IDrawCall& rDrawCall, float fRed, float fGreen, float fBlue, float fAlpha) = 0;

    //--------------------------------------------------------------------------
    /// Hook to draw the active rendertarget(s) on the screen
    //--------------------------------------------------------------------------
    virtual void DoAutoRenderTarget() = 0;

    //--------------------------------------------------------------------------
    /// Hook to allow the API-specific FD to choose whether or not to execute the drawcall at the breakpoint
    //--------------------------------------------------------------------------
    virtual void DoDrawCallAtBreakPoint(IDrawCall& rDrawCall) = 0;

    //--------------------------------------------------------------------------
    /// Hook that is called when the app has has rendered the drawcall at the breakpoint
    /// \param rDrawCall the drawcall that is executed at the breakpoint
    //--------------------------------------------------------------------------
    virtual void OnDrawCallAtBreakPointPreHUD(IDrawCall& rDrawCall) = 0;

    //--------------------------------------------------------------------------
    /// Hook that is called when the app has has rendered the drawcall at the breakpoint
    /// \param rDrawCall the drawcall that is executed at the breakpoint
    //--------------------------------------------------------------------------
    virtual void OnDrawCallAtBreakPoint(IDrawCall& rDrawCall) = 0;

    //--------------------------------------------------------------------------
    /// Hook to initialize rendering to the HUD
    /// \return true if the HUD can be rendered to; false otherwise
    //--------------------------------------------------------------------------
    virtual bool BeginHUD() = 0;

    //--------------------------------------------------------------------------
    /// Hook to deinitialize rendering to the HUD
    //--------------------------------------------------------------------------
    virtual void EndHUD() = 0;

private:

    //--------------------------------------------------------------------------
    /// Determines if the specified drawcall should be executed
    /// \param uDrawCall index of the drawcall that should be compared against
    ///    the breakpoint to see if it should be executed
    /// \return true if the draw call should be executed; false otherwise
    //--------------------------------------------------------------------------
    bool IsDrawCallEnabled(unsigned long uDrawCall);

    //--------------------------------------------------------------------------
    /// Determines if the specified drawcall is the breakpoint (target) drawcall
    /// \param uDrawCall index of the drawcall that should be compared against
    ///    the breakpoint to see if it is the target
    /// \return true if the draw call should be executed as the target; false otherwise
    //--------------------------------------------------------------------------
    bool IsTargetDrawCall(unsigned long uDrawCall);

    //-----------------------------------------------------------------------------
    /// Resets the draw call count
    //-----------------------------------------------------------------------------
    void ResetDrawCallCount() { m_ulDrawCallCounter = 0; }

    //-----------------------------------------------------------------------------
    /// No additional settings
    //-----------------------------------------------------------------------------
    virtual std::string GetDerivedSettings() { return ""; }

private:

    /// The breakpoint (aka current targeted drawcall)
    ULongCommandResponse m_dwBreakPoint;

    /// List of draw calls
    HTTPLogger  m_drawCallList;

    /// Number of Draw Calls, this is incremented every frame and is used for breakpoints
    unsigned long m_ulDrawCallCounter;

    /// Indicates the color to use when clearing the back buffer
    float m_ClearColor[4];

    /// Indicates the color to use when showing the wireframe overlay on the HUD
    float m_fWireframeOverlayColor[4];

    /// Activated when the current stats data is requested
    CommandResponse m_Stats;

    /// Activated when the current drawcall data is requested
    CommandResponse m_CurrentDrawCall;

    /// Last executed drawcall
    IDrawCall* m_LastDrawCall;

    /// Wireframe Color
    int m_iWireframeColor;

protected:

    /// AutoRenderTarget is a mode where the current RenderTarget is automatically shown
    /// on the HUD if the rendertarget is not already the backbuffer
    BoolCommandResponse m_bAutoRenderTarget;

    /// Indicates whether or not to clear the back buffer at the start of each frame
    BoolCommandResponse m_bForceClear;

    /// Indicates whether or not a wireframe overlay should be shown on the HUD
    BoolCommandResponse m_bWireframeOverlay;

    /// Indicates whether or not the HUD config is getting updated, i.e. HUD controls window is up
    BoolCommandResponse m_bConfigHUD;
};

#endif //FRAMEDEBUGGER_H
