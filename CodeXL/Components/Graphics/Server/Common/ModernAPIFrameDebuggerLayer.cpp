//==============================================================================
/// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file ModernAPIFrameDebuggerLayer.cpp
/// \brief A Frame Debugger baseclass that can handle general frame requests.
//==============================================================================

#include "ModernAPIFrameDebuggerLayer.h"
#include "ModernAPILayerManager.h"
#include "FrameInfo.h"

//--------------------------------------------------------------------------
/// The size of the buffer used to compute FPS.
//--------------------------------------------------------------------------
static const uint32 kFrameHistoryLength = 10;

//--------------------------------------------------------------------------
/// Default constructor for ModernAPIFrameDebuggerLayer.
//--------------------------------------------------------------------------
ModernAPIFrameDebuggerLayer::ModernAPIFrameDebuggerLayer()
{
    // Command that collects a CPU and GPU trace from the same frame.
    AddCommand(CONTENT_XML, "GetCurrentFrameInfo", "GetCurrentFrameInfo", "GetCurrentFrameInfo.xml", DISPLAY, INCLUDE, mCmdGetCurrentFrameInfo);
}

//--------------------------------------------------------------------------
/// Default destructor for ModernAPIFrameDebuggerLayer.
//--------------------------------------------------------------------------
ModernAPIFrameDebuggerLayer::~ModernAPIFrameDebuggerLayer()
{
}

//--------------------------------------------------------------------------
/// The layer must create its resources here, it may hook some functions if really needed
/// \param type the creation object type.
/// \param pPtr Pointer to the object that was just created.
/// \return True if success, False if fail.
//--------------------------------------------------------------------------
bool ModernAPIFrameDebuggerLayer::OnCreate(CREATION_TYPE type, void* pPtr)
{
    PS_UNREFERENCED_PARAMETER(type);
    PS_UNREFERENCED_PARAMETER(pPtr);

    return true;
}

//--------------------------------------------------------------------------
/// End the frame.
//--------------------------------------------------------------------------
void ModernAPIFrameDebuggerLayer::EndFrame()
{
    if (mCmdGetCurrentFrameInfo.IsActive())
    {
        FrameInfo frameInfo;
        GetFrameInfo(&frameInfo);

        gtASCIIString frameInfoXML;
        frameInfo.WriteToXML(frameInfoXML);

        mCmdGetCurrentFrameInfo.Send(frameInfoXML.asCharArray());
    }
}

//--------------------------------------------------------------------------
/// Retrieve basic timing and API usage information by filling in a FrameInfo
/// instance.
/// \param outFrameInfo The FrameInfo structure that will be populated with data.
//--------------------------------------------------------------------------
void ModernAPIFrameDebuggerLayer::GetFrameInfo(FrameInfo* outFrameInfo)
{
    outFrameInfo->mFrameNumber = GetParentLayerManager()->GetCurrentFrameIndex();
    outFrameInfo->mTotalElapsedTime = GetParentLayerManager()->GetElapsedTimeMilliseconds();
    outFrameInfo->mFrameDuration = GetParentLayerManager()->GetLastFrameDurationMilliseconds();
    outFrameInfo->mRunningFPS = GetParentLayerManager()->GetAverageFPS();

    //Log(logERROR, "5) ModernAPIFrameDebuggerLayer::GetFrameInfo FPS: %lf, Frame Count: %d\n", outFrameInfo->mRunningFPS, outFrameInfo->mFrameNumber);
}
