//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A common baseclass for objects being tracked in the object database.
//==============================================================================

#ifndef IINSTANCEBASE_H
#define IINSTANCEBASE_H

// For XML support
#include "xml.h"

enum eObjectType : int;

    //--------------------------------------------------------------------------
    /// The baseclass for all API object instances.
    //--------------------------------------------------------------------------
    class IInstanceBase
{
public:
    //--------------------------------------------------------------------------
    /// Default constructor for IInstanceBase.
    //--------------------------------------------------------------------------
    IInstanceBase() : mbIsDestroyed(false) {}

    //--------------------------------------------------------------------------
    /// Default destructor for IInstanceBase.
    //--------------------------------------------------------------------------
    virtual ~IInstanceBase() {}

    //--------------------------------------------------------------------------
    /// Each wrapped object type needs to implement this function to determine the object type.
    /// \returns The type of object that this wrapper instance is wrapping.
    //--------------------------------------------------------------------------
    virtual eObjectType GetObjectType() const = 0;

    //--------------------------------------------------------------------------
    /// Stringify the type of object being wrapped for display in the client.
    /// \returns A string containing the interface type.
    //--------------------------------------------------------------------------
    virtual const char* GetTypeAsString() const = 0;

    //--------------------------------------------------------------------------
    /// Retrieve a pointer to the parent device that created this instance.
    /// \returns A pointer to the parent device that created the object being wrapped.
    //--------------------------------------------------------------------------
    virtual void* GetParentDeviceHandle() const = 0;

    //--------------------------------------------------------------------------
    /// Retrieve a pointer to the real instance. This is the same handle that the application uses.
    /// \returns A void* to the real API object instance.
    //--------------------------------------------------------------------------
    virtual void* GetApplicationHandle() const = 0;

    //--------------------------------------------------------------------------
    /// Print a hex-formatted application handle into the input string.
    /// \param outHandleString The string that the application handle will be printed into.
    //--------------------------------------------------------------------------
    virtual void PrintFormattedApplicationHandle(gtASCIIString& outHandleString) const = 0;

    //--------------------------------------------------------------------------
    /// Write the create data for the wrapped object as XML.
    /// \param outCreateInfoXML The CreateInfo formatted in XML.
    //--------------------------------------------------------------------------
    virtual void AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const = 0;

    //--------------------------------------------------------------------------
    /// Format the object tag data into the output parameter.
    /// \param outTagDataString A string containing the tag data for this instance.
    /// \returns True if the object contains tag data, false if it doesn't.
    //--------------------------------------------------------------------------
    virtual bool AppendTagDataXML(gtASCIIString& outTagDataString) const = 0;

    //--------------------------------------------------------------------------
    /// Write the type of object that this wrapper wraps.
    /// \param outTypeXml The object type as an XML element.
    //--------------------------------------------------------------------------
    virtual void AppendTypeXML(gtASCIIString& outTypeXml) const
    {
        outTypeXml += XML("Type", GetTypeAsString());
    }

    //--------------------------------------------------------------------------
    /// Check if this object has been destroyed.
    /// \returns True if this object instance has been destroyed.
    //--------------------------------------------------------------------------
    inline bool IsDestroyed() const { return mbIsDestroyed; }

    //--------------------------------------------------------------------------
    /// Set an object's destroyed flag to true.
    //--------------------------------------------------------------------------
    inline void FlagAsDestroyed() { mbIsDestroyed = true; }

private:
    //--------------------------------------------------------------------------
    /// A flag used to track whether the wrapped runtime instance has been destroyed.
    //--------------------------------------------------------------------------
    bool mbIsDestroyed;
};

#endif // IINSTANCEBASE_H
