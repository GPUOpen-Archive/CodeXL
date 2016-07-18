//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   ObjectDatabaseProcessor.h
/// \brief  The ObjectDatabaseProcessor is responsible for queries sent to the
///         object database. Through this processor, we can collect data related
///         to API objects that have been wrapped and stored within the database.
//==============================================================================

#ifndef OBJECTDATABASEPROCESSOR_H
#define OBJECTDATABASEPROCESSOR_H

class IInstanceBase;
class WrappedObjectDatabase;

#include "IModernAPILayer.h"
#include "CommandProcessor.h"

#include <tinyxml.h>
#include <AMDTBaseTools/Include/gtString.h>

/// Object map
typedef std::map<gtASCIIString, int> ObjectTypeNameToValueMap;

//--------------------------------------------------------------------------
/// The ObjectDatabaseProcessor is a processor that is not only responsible
/// for maintaining handles to the stored content, and can be used to query
/// the stored object instances to retrieve info.
//--------------------------------------------------------------------------
class ObjectDatabaseProcessor : public IModernAPILayer, public CommandProcessor
{
public:
    //-----------------------------------------------------------------------------
    /// Constructor used to initialize necessary CommandResponses.
    //-----------------------------------------------------------------------------
    ObjectDatabaseProcessor();

    //-----------------------------------------------------------------------------
    /// Destructor.
    //-----------------------------------------------------------------------------
    virtual ~ObjectDatabaseProcessor();

    virtual bool OnCreate(CREATION_TYPE type, void* pPtr);
    virtual bool OnDestroy(CREATION_TYPE type, void* pPtr);
    virtual void BeginFrame();
    virtual void EndFrame();

    /// CommandProcessor
    virtual std::string GetDerivedSettings() { return ""; }

    //--------------------------------------------------------------------------
    /// Enable Object Database collection to disk
    //--------------------------------------------------------------------------
    void EnableObjectDatabaseCollection();

    //--------------------------------------------------------------------------
    /// Disable Object Database collection to disk
    //--------------------------------------------------------------------------
    void DisableObjectDatabaseCollection();

    //--------------------------------------------------------------------------
    /// Use the DatabaseProcessor to retrieve a pointer to the object database instance.
    /// Must be implemented by subclass ObjectDatabaseProcessor types.
    //--------------------------------------------------------------------------
    virtual WrappedObjectDatabase* GetObjectDatabase() = 0;

protected:
    //--------------------------------------------------------------------------
    /// Retrieve the object type enumeration value from a type string.
    /// \param inObjectTypeString A string containing the type of object to get the value for.
    /// \returns The enumeration value for the incoming object type string.
    //--------------------------------------------------------------------------
    virtual int GetObjectTypeFromString(const gtASCIIString& inObjectTypeString) const = 0;

    //--------------------------------------------------------------------------
    /// A helper function that does the dirty work of building object XML.
    //--------------------------------------------------------------------------
    void BuildObjectTreeResponse(gtASCIIString& outObjectTreeXml);

    //--------------------------------------------------------------------------
    /// Parse a comma-separated string of addresses into a list of pointers.
    /// \param inAddressesString A string containing comma-separated hex addresses.
    /// \param outAddressStrings A vector of address pointers.
    //--------------------------------------------------------------------------
    bool ParseAddressesString(const gtASCIIString& inAddressesString, std::vector<void*>& outAddressStrings);

    //--------------------------------------------------------------------------
    /// Return the value of the first object in the API's typelist.
    /// \returns the value of the first object in the API's type list.
    //--------------------------------------------------------------------------
    virtual int GetFirstObjectType() const = 0;

    //--------------------------------------------------------------------------
    /// Return the value of the last object in the API's typelist.
    /// \returns the value of the last object in the API's type list.
    //--------------------------------------------------------------------------
    virtual int GetLastObjectType() const = 0;

    //--------------------------------------------------------------------------
    /// Return the value of the Device type in the API's type list.
    /// \returns the value of the Device type in the API's type list.
    //--------------------------------------------------------------------------
    virtual int GetDeviceType() const = 0;

protected:
    //--------------------------------------------------------------------------
    /// A member that points to the currently-selected object within the object database.
    /// The selection is changed by responding to messages from the client.
    //--------------------------------------------------------------------------
    IInstanceBase* mSelectedObject;

    //--------------------------------------------------------------------------
    /// A member that points to the currently-selected object within the XML object database loaded from cache.
    /// The selection is changed by responding to messages from the client.
    /// Used only when Capture Player is active
    //--------------------------------------------------------------------------
    TiXmlElement*  mSelectedObjectXml = nullptr;

private:
    //--------------------------------------------------------------------------
    /// Respond to object selection change commands.
    //--------------------------------------------------------------------------
    void UpdateSelectedObject();

    //--------------------------------------------------------------------------
    /// Write a string to XML file
    /// \param xmlString pointer to a string for outputing to XML file, must be valid string with minimal 1 length
    /// \param fullFilePath Full path with filename.
    /// \returns True if writing the file was successful.
    //--------------------------------------------------------------------------
    bool WriteXMLFile(gtASCIIString* xmlString, const std::string& fullFilePath);

    //--------------------------------------------------------------------------
    /// Retrieve full frame data storage folder
    /// \param FrameStorageFolder A string containing frame data storage folder
    //--------------------------------------------------------------------------
    bool GetFrameStorageFullPath(gtString& FrameStorageFolder);

    //--------------------------------------------------------------------------
    /// A helper function that loads full object database from disk cache into class storage
    //--------------------------------------------------------------------------
    bool LoadObjectDatabase();

    //--------------------------------------------------------------------------
    /// Retrieve Object type string from currently selected Obj XML node, Capture player only
    /// \param strObjType   formatted string for object type
    /// \returns    True if object type retrieved successfully.
    //--------------------------------------------------------------------------
    bool SelObjTypeFromDBase(gtASCIIString& strObjType);

    //--------------------------------------------------------------------------
    /// Retrieve Object tag string from currently selected Obj XML node, Capture player only
    /// \param strTagName   formatted string for object tag
    /// \returns    True if object tag retrieved successfully.
    //--------------------------------------------------------------------------
    bool SelObjTagFromDBase(gtASCIIString& strObjTag);

    //--------------------------------------------------------------------------
    /// Retrieve Object CreateInfo string from currently selected Obj XML node, Capture player only
    /// \param strObjCreateInfo   formatted string for object CreateInfo
    /// \returns    True if object CreateInfo retrieved successfully.
    //--------------------------------------------------------------------------
    bool SelObjCreateInfoFromDBase(gtASCIIString& strObjCreateInfo);

    //--------------------------------------------------------------------------
    /// Handle object tree or object database requestes.
    //--------------------------------------------------------------------------
    void HandleObjInfoResponse();

    //--------------------------------------------------------------------------
    /// Load a text/xml file from disk when given a valid path.
    /// \param inFilepath The full filepath to a text file.
    /// \param outFileContents The full contents of the loaded text file.
    /// \returns True if the text file was loaded correctly.
    //--------------------------------------------------------------------------
    bool LoadFile(const std::string& inFilepath, gtASCIIString& outFileContents);

    //--------------------------------------------------------------------------
    /// A command that responds with an object's type.
    //--------------------------------------------------------------------------
    CommandResponse mObjectTypeResponse;

    //--------------------------------------------------------------------------
    /// A command that responds with an object's tag data.
    //--------------------------------------------------------------------------
    CommandResponse mObjectTagResponse;

    //--------------------------------------------------------------------------
    /// A command that responds with an object's CreateInfo structure.
    //--------------------------------------------------------------------------
    CommandResponse mObjectCreateInfoResponse;

    //--------------------------------------------------------------------------
    /// A command that responds with a large tree of XML representing active devices and objects.
    //--------------------------------------------------------------------------
    CommandResponse mObjectTreeResponse;

    //--------------------------------------------------------------------------
    /// A command that responds with a full of XML representing all captured objects.
    //--------------------------------------------------------------------------
    CommandResponse mObjectDatabaseResponse;

    //--------------------------------------------------------------------------
    /// This command responds with a large dump of CreateInfo XML for objects with
    /// a given type. The argument is an instance of eObjectType.
    //--------------------------------------------------------------------------
    TextCommandResponse mAllCreateInfoResponse;

    //--------------------------------------------------------------------------
    /// A CommandResponse used by the client to change the currently selected object. When there's a request for
    /// tag data or create info responses, this is updated first, and then the response is generated using
    /// the selected wrapper object.
    //--------------------------------------------------------------------------
    TextCommandResponse mSelectedObjectResponse;

    //-----------------------------------------------------------------------------
    /// TinyXML DOM of the full object database. First child is the XML node, which is parent of all the CreateInfo's.
    //-----------------------------------------------------------------------------
    TiXmlDocument mXmlDomObjectDatabase;

    //--------------------------------------------------------------------------
    /// A flag to indicate that an Object Database is collected for capture to disk mode.
    //--------------------------------------------------------------------------
    bool mbObjectDatabaseForCapture;
};

#endif // OBJECTDATABASEPROCESSOR_H
