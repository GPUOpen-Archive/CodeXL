//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   DX12FrameDebuggerLayer.h
/// \brief  The DX12FrameDebuggerLayer is responsible for operations related to
///         inspecting individual frames of an instrumented D3D12 application.
//=============================================================================

#ifndef DX12FRAMEDEBUGGERLAYER_H
#define DX12FRAMEDEBUGGERLAYER_H

#include "../../Common/ModernAPIFrameDebuggerLayer.h"
#include "../../Common/TSingleton.h"
#include <unordered_map>

struct IDXGISwapChain;
struct ID3D12CommandQueue;
class DX12ImageRenderer;

/// A type of map used to associate a swap chain with an ID3D12CommandQueue.
typedef std::unordered_map<IDXGISwapChain*, ID3D12CommandQueue*> SwapchainToQueueMap;

//-----------------------------------------------------------------------------
/// The root layer for all work related to DX12 Frame Debugging.
//-----------------------------------------------------------------------------
class DX12FrameDebuggerLayer : public ModernAPIFrameDebuggerLayer, public TSingleton< DX12FrameDebuggerLayer >
{
    //-----------------------------------------------------------------------------
    /// TSingleton is a friend of the DX12FrameDebuggerLayer.
    //-----------------------------------------------------------------------------
    friend TSingleton < DX12FrameDebuggerLayer >;
public:
    //-----------------------------------------------------------------------------
    /// Default destructor for DX12FrameDebuggerLayer.
    //-----------------------------------------------------------------------------
    virtual ~DX12FrameDebuggerLayer();

    //-----------------------------------------------------------------------------
    /// Retrieve a pointer to the parent LayerManager used by this tool.
    /// \returns A pointer to the parent LayerManager used by this tool.
    //-----------------------------------------------------------------------------
    virtual ModernAPILayerManager* GetParentLayerManager();

    //-----------------------------------------------------------------------------
    /// Called to indicate that a resource is being created
    /// The layer must create its resources and hook functions here
    /// \param type the type of resource that is being created
    /// \param pPtr pointer to the resource that is being created
    /// \return should return false on error; true otherwise
    //-----------------------------------------------------------------------------
    virtual bool OnCreate(CREATION_TYPE type, void* pPtr) { (void)type; (void)pPtr; return true; }

    //-----------------------------------------------------------------------------
    /// Called to indicate that a resource is being destroyed
    /// detaches from anything that was attached in OnCreate
    /// \param type the type of resource that is being destroyed
    /// \param pPtr pointer to the resource that is being destroyed
    /// \return should return false on error; true otherwise
    //-----------------------------------------------------------------------------
    virtual bool OnDestroy(CREATION_TYPE type, void* pPtr) { (void)type; (void)pPtr; return true; }

    //-----------------------------------------------------------------------------
    /// Called at the end of a frame
    //-----------------------------------------------------------------------------
    virtual void EndFrame();

    //-----------------------------------------------------------------------------
    /// Gets called after a frame has been presented.
    /// \param inSwapchain The swap chain instance used to present to the screen.
    //-----------------------------------------------------------------------------
    virtual void OnPresent(void* inSwapchain);

    //-----------------------------------------------------------------------------
    /// Handler invoked when a new swap chain is created.
    /// \param inSwapchain The swap chain instance that was just created.
    /// \param inDevice The CommandQueue used to create the swap chain.
    //-----------------------------------------------------------------------------
    virtual void OnSwapchainCreated(void* inSwapchain, void* inDevice);

    //-----------------------------------------------------------------------------
    /// Retrieve a Derived Settings string. Don't do anything in this case.
    //-----------------------------------------------------------------------------
    virtual string GetDerivedSettings() { return ""; }

    //-----------------------------------------------------------------------------
    /// Capture the current frame buffer image, and return an byte array of PNG-encoded image data. NOTE: Passing in both a width and height of 0 will causes the frame buffer's size to be used when generating the output image.
    /// Note that the output "ioFrameBufferPngData" array must be deleted when finished, or else it will leak.
    /// \param inWidth The requested width of the captured frame buffer image.
    /// \param inHeight The requested height of the captured frame buffer image.
    /// \param ioFrameBufferPngData A pointer to the byte array of PNG-encoded image data.
    /// \param outNumBytes The total number of bytes in the array of encoded image data.
    /// \param adjustAspectRatio Turns on or off the resizing of the input height and width for cases where the aspect ratios do not match.
    /// \returns True if the frame buffer image was captured successfully. False if it failed.
    //-----------------------------------------------------------------------------
    virtual bool CaptureFrameBuffer(unsigned int inWidth, unsigned int inHeight, unsigned char** ioFrameBufferPngData, unsigned int* outNumBytes, bool adjustAspectRatio);

private:
    //-----------------------------------------------------------------------------
    /// Default constructor for DX12FrameDebuggerLayer.
    //-----------------------------------------------------------------------------
    DX12FrameDebuggerLayer();

    //-----------------------------------------------------------------------------
    /// The frame buffer renderer object.
    //-----------------------------------------------------------------------------
    DX12ImageRenderer* mFrameBufferRenderer;

    //-----------------------------------------------------------------------------
    /// Handle an incoming image request by sending the image data as a response.
    /// \param inImageCommand The command used to request image data.
    /// \returns True when the frame buffer was successfully captured and sent back as a response.
    //-----------------------------------------------------------------------------
    bool HandleFrameBufferRequest(PictureCommandResponse& inImageCommand);

    //-----------------------------------------------------------------------------
    /// A command that allows the server to respond with PNG data of the frame buffer image.
    //-----------------------------------------------------------------------------
    PictureCommandResponse mGetFrameBufferImage;

    //-----------------------------------------------------------------------------
    /// The last swap chain used to present an image to the screen.
    //-----------------------------------------------------------------------------
    IDXGISwapChain* mLastPresentedSwapchain;

    //-----------------------------------------------------------------------------
    /// A map that associates a swap chain with the CommandQueue used to create it.
    //-----------------------------------------------------------------------------
    SwapchainToQueueMap mSwapchainToCommandQueue;

};

#endif // DX12FRAMEDEBUGGERLAYER_H