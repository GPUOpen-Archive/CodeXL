//==============================================================================
/// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file ModernAPIFrameDebuggerLayer.h
/// \brief A Frame Debugger baseclass that can handle general frame requests.
//==============================================================================

#ifndef MODERNAPIFRAMEDEBUGGERLAYER_H
#define MODERNAPIFRAMEDEBUGGERLAYER_H

#include "IModernAPILayer.h"
#include "CommandProcessor.h"
#include "CommonTypes.h"

class FrameInfo;

//--------------------------------------------------------------------------
/// A Frame Debugger baseclass that can handle general frame requests.
//--------------------------------------------------------------------------
class ModernAPIFrameDebuggerLayer : public IModernAPILayer, public CommandProcessor
{
public:
    //--------------------------------------------------------------------------
    /// Default constructor for ModernAPIFrameDebuggerLayer.
    //--------------------------------------------------------------------------
    ModernAPIFrameDebuggerLayer();

    //--------------------------------------------------------------------------
    /// Default destructor for ModernAPIFrameDebuggerLayer.
    //--------------------------------------------------------------------------
    virtual ~ModernAPIFrameDebuggerLayer();

    //--------------------------------------------------------------------------
    /// The layer must create its resources here, it may hook some functions if really needed
    /// \param type the creation object type.
    /// \param pPtr Pointer to the object that was just created.
    /// \return True if success, False if fail.
    //--------------------------------------------------------------------------
    virtual bool OnCreate(CREATION_TYPE type, void* pPtr);

    //--------------------------------------------------------------------------
    /// End the frame.
    //--------------------------------------------------------------------------
    virtual void EndFrame();

    //--------------------------------------------------------------------------
    /// Retrieve basic timing and API usage information by filling in a FrameInfo
    /// instance.
    /// \param outFrameInfo The FrameInfo structure that will be populated with data.
    //--------------------------------------------------------------------------
    virtual void GetFrameInfo(FrameInfo* outFrameInfo);

    //--------------------------------------------------------------------------
    /// Capture the current frame buffer image, and return an byte array of PNG-encoded image data. NOTE: Passing in both a width and height of 0 will causes the frame buffer's size to be used when generating the output image.
    /// Note that the output "ioFrameBufferPngData" array must be deleted when finished, or else it will leak.
    /// \param inWidth The requested width of the captured frame buffer image.
    /// \param inHeight The requested height of the captured frame buffer image.
    /// \param ioFrameBufferPngData A pointer to the byte array of PNG-encoded image data.
    /// \param outNumBytes The total number of bytes in the array of encoded image data.
    /// \param adjustAspectRatio Turns on or off the resizing of the input height and width for cases where the aspect ratios do not match.
    /// \returns True if the frame buffer image was captured successfully. False if it failed.
    //--------------------------------------------------------------------------
    virtual bool CaptureFrameBuffer(unsigned int inWidth, unsigned int inHeight, unsigned char** ioFrameBufferPngData, unsigned int* outNumBytes, bool adjustAspectRatio) = 0;

protected:


private:

    //--------------------------------------------------------------------------
    /// A command responsible for retrieving basic frame information as a chunk
    /// of formatted XML.
    //--------------------------------------------------------------------------
    CommandResponse mCmdGetCurrentFrameInfo;
};

#endif // MODERNAPIFRAMEDEBUGGERLAYER_H