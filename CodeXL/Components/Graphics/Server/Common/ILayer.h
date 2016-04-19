//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Provides the interface for a Layer.
///         A layer can be enabled and disabled, is informed when a primary resource
///         is created or destroyed, and is informed of the beginning and end of a frame
//==============================================================================

#ifndef ILAYER_H
#define ILAYER_H
#include "Logger.h"
#include "IMonitor.h"
#include "misc.h"
#include "CommandProcessor.h"

/// Provides the interface for a Layer.
class ILayer
{
public:

    /// Constructor
    ILayer()
        : m_bEnabled(false)
    {
        m_layerName = "Unset";
    }

    /// Called to indicate that a resource is being created
    ///
    /// The layer must create its resources and hook functions here
    ///
    /// \param type the type of resource that is being created
    /// \param pPtr pointer to the resource that is being created
    /// \return should return false on error; true otherwise
    virtual bool OnCreate(CREATION_TYPE type, void* pPtr) = 0;

    /// Called to indicate that a resource is being destroyed
    ///
    /// detaches from anything that was attached in OnCreate
    /// \param type the type of resource that is being destroyed
    /// \param pPtr pointer to the resource that is being destroyed
    /// \return should return false on error; true otherwise
    virtual bool OnDestroy(CREATION_TYPE type, void* pPtr) = 0;

    /// Called at the beginning of a frame
    virtual void BeginFrame() {};

    /// Called at the end of a frame
    virtual void EndFrame() {};

    /// Enable/disable the layer.
    ///
    /// This is called when the layer is being enabled and it should create any
    /// necessary resource or hook any necessary entrypoints
    /// \param bNewStatus indicates whether this layer should be enabled (true) or disabled (false)
    /// \param pRequest Command response to send messages back to the client
    /// \return EL_BUSY if layer can't be enabled/disabled because it is in the middle of an operation;
    ///   EL_CANNOTBEDISABLED if the layer cannot ever be disabled; EL_OK if enabled/disabled is successful
    bool EnableLayer(bool bNewStatus, CommandResponse* pRequest)
    {
        if (OnEnableLayer(bNewStatus, pRequest))
        {
            m_bEnabled = bNewStatus;
        }

        return true;
    }

    /// Accessor to find out if the layer is enabled / disabled
    /// \return true if the layer is enabled, false otherwise
    bool IsEnabled()
    {
        return m_bEnabled;
    }

    /// Set the name of the layer - used for debug purposes
    /// \param name Input layer name
    void SetLayerName(const char* name)
    {
        m_layerName = name;
    }

    /// Accessor to get the name of the layer
    /// \return The name of the layer
    gtASCIIString& GetLayerName()
    {
        return m_layerName;
    }

protected:

    /// A hook for derived classes to react when the layer is enabled or disabled
    /// \param bNewStatus indicates the desired status of the layer
    /// \param pRequest the request for which this layer is being enabled
    /// \return true
    virtual bool OnEnableLayer(bool bNewStatus, CommandResponse* pRequest)
    {
        PS_UNREFERENCED_PARAMETER(bNewStatus);

        m_bEnabled = bNewStatus;

        if (pRequest != NULL)
        {
            pRequest->Send("OK");
        }

        return true;
    }

private:

    /// Indicates whether this layer is enabled (true) or disabled (false)
    bool m_bEnabled;

    /// Name of the layer
    gtASCIIString m_layerName;
};

#endif