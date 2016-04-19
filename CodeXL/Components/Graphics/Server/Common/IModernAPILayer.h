//==============================================================================
/// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file IModernAPILayer.h
/// \brief A Layer interface for use with tools supporting Modern APIs.
//==============================================================================

#ifndef IMODERNAPILAYER_H
#define IMODERNAPILAYER_H

#include "ILayer.h"

// Forward declarations.
class Capture;
class ModernAPILayerManager;

//--------------------------------------------------------------------------
/// A Layer interface for use with tools supporting Modern APIs.
//--------------------------------------------------------------------------
class IModernAPILayer : public ILayer
{
public:
    //--------------------------------------------------------------------------
    /// Default constructor for IModernAPILayer.
    //--------------------------------------------------------------------------
    IModernAPILayer() {}

    //--------------------------------------------------------------------------
    /// Default destructor for IModernAPILayer.
    //--------------------------------------------------------------------------
    virtual ~IModernAPILayer() {}

    //--------------------------------------------------------------------------
    /// Retrieve a pointer to the parent LayerManager used by this tool.
    /// \returns A pointer to the parent LayerManager used by this tool.
    //--------------------------------------------------------------------------
    virtual ModernAPILayerManager* GetParentLayerManager() = 0;

    //--------------------------------------------------------------------------
    /// @TODO: This should probably be pure virtual, because each Layer that inherits
    /// from this baseclass will likely want to do something Before/After a real API call.
    /// Callback that's invoked before a Capture object's RealCall is executed.
    /// \param inCaptureObject The Capture object that's about to be executed.
    //--------------------------------------------------------------------------
    virtual void BeforeCall(Capture* inCaptureObject) { (void)inCaptureObject; }

    //--------------------------------------------------------------------------
    /// @TODO: This should probably be pure virtual, because each Layer that inherits
    /// from this baseclass will likley want to do something Before/After a real API call.
    /// Callback that's invoked after a Capture object's RealCall has been executed.
    /// \param inCaptureObject The Capture object that has just been executed.
    //--------------------------------------------------------------------------
    virtual void AfterCall(Capture* inCaptureObject) { (void)inCaptureObject; }

    //--------------------------------------------------------------------------
    /// Gets called after a frame has been presented.
    /// \param inSwapchain The swapchain instance used to present to the screen.
    //--------------------------------------------------------------------------
    virtual void OnPresent(void* inSwapchain) { (void)inSwapchain; }

    //--------------------------------------------------------------------------
    /// Gets called when a swapchain is created.
    /// \param inSwapchain The swapchain instance that was just created.
    /// \param inDevice The device used to created the swapchain.
    //--------------------------------------------------------------------------
    virtual void OnSwapchainCreated(void* inSwapchain, void* inDevice) { (void)inSwapchain; (void)inDevice; }
};

#endif // IMODERNAPILAYER_H