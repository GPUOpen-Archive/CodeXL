//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   ObjectDatabaseProcessor.cpp
/// \brief  The ObjectDatabaseProcessor is responsible for queries sent to the
///         object database. Through this processor, we can collect data related
///         to API objects that have been wrapped and stored within the database.
//==============================================================================

#include "ObjectDatabaseProcessor.h"
#include "WrappedObjectDatabase.h"
#include "IInstanceBase.h"
#include <AMDTOSWrappers/Include/osFile.h>
#include "../Common/SessionManager.h"
#include "ModernAPILayerManager.h"
#include "TraceMetadata.h"
#include "FrameInfo.h"

/// Definition
#define OBJECT_UNDEFINED -1

//-----------------------------------------------------------------------------
/// Constructor used to initialize necessary CommandResponses.
//-----------------------------------------------------------------------------
ObjectDatabaseProcessor::ObjectDatabaseProcessor()
    : mSelectedObject(NULL), mbObjectDatabaseForCapture(false)
{
    // Add each command as a child of "this" CommandProcessor, and set each to not autoreply.
    // We need to do this so that we can detect which Command is active, read the client arguments,
    // and generate a suitable response string to reply with.
    AddCommand(CONTENT_XML, "ObjType", "ObjType", "ObjType.xml", NO_DISPLAY, NO_INCLUDE, mObjectTypeResponse);
    mObjectTypeResponse.SetEditableContentAutoReply(false);

    AddCommand(CONTENT_XML, "ObjTag", "ObjTag", "ObjTag.xml", NO_DISPLAY, NO_INCLUDE, mObjectTagResponse);
    mObjectTagResponse.SetEditableContentAutoReply(false);

    AddCommand(CONTENT_XML, "ObjCreateInfo", "ObjCreateInfo", "ObjCreateInfo.xml", DISPLAY, INCLUDE, mObjectCreateInfoResponse);
    mObjectCreateInfoResponse.SetEditableContentAutoReply(false);

    AddCommand(CONTENT_XML, "ObjectTree", "ObjectTree", "ObjectTree.xml", NO_DISPLAY, NO_INCLUDE, mObjectTreeResponse);
    mObjectTreeResponse.SetEditableContentAutoReply(false);

    AddCommand(CONTENT_XML, "ObjectDatabase", "ObjectDatabase", "ObjectDatabase.xml", NO_DISPLAY, NO_INCLUDE, mObjectDatabaseResponse);
    mObjectDatabaseResponse.SetEditableContentAutoReply(false);

    AddCommand(CONTENT_TEXT, "SelectedObject", "Change DB's the selected object", "SelectedObject", NO_DISPLAY, NO_INCLUDE, mSelectedObjectResponse);
    mSelectedObjectResponse.SetEditableContentAutoReply(false);

    AddCommand(CONTENT_XML, "AllCreateInfo", "AllCreateInfo", "AllCreateInfo.xml", DISPLAY, INCLUDE, mAllCreateInfoResponse);
    mAllCreateInfoResponse.SetEditableContentAutoReply(false);

    SetLayerName("ObjectDatabaseProcessor");
}

//-----------------------------------------------------------------------------
/// Destructor.
//-----------------------------------------------------------------------------
ObjectDatabaseProcessor::~ObjectDatabaseProcessor()
{
}

//--------------------------------------------------------------------------
/// Invoked when the ObjectDatabaseProcessor is created.
/// \param type The type of object that this processor is being created specifically for.
/// \param pPtr A pointer to the object instance that was created for this layer.
/// \returns True if layer creation succeeded.
//--------------------------------------------------------------------------
bool ObjectDatabaseProcessor::OnCreate(CREATION_TYPE type, void* pPtr)
{
    PS_UNREFERENCED_PARAMETER(type);
    PS_UNREFERENCED_PARAMETER(pPtr);

    return true;
}

//--------------------------------------------------------------------------
/// Invoked when the ObjectDatabaseProcessor is destroyed.
/// \param type The type of object that's probably going to die.
/// \param pPtr A pointer to the object instance that's going to die.
/// \returns True if layer destruction succeeded.
//--------------------------------------------------------------------------
bool ObjectDatabaseProcessor::OnDestroy(CREATION_TYPE type, void* pPtr)
{
    PS_UNREFERENCED_PARAMETER(type);
    PS_UNREFERENCED_PARAMETER(pPtr);

    return true;
}

//--------------------------------------------------------------------------
/// Invoked when rendering the frame begins. Used to clean up state before a new frame's trace begins.
//--------------------------------------------------------------------------
void ObjectDatabaseProcessor::BeginFrame()
{
    // Before starting frame tracing, purge all the dead objects from the object database.
    // We only want to track existing objects and instances that are created during the new frame's tracing.
    WrappedObjectDatabase* objectDatabase = GetObjectDatabase();
    objectDatabase->TrackObjectLifetime(true);
}

//--------------------------------------------------------------------------
/// Invoked when the frame finishes rendering. At this point, we can check
/// if there are any requests to retrieve data from the object database.
//--------------------------------------------------------------------------
void ObjectDatabaseProcessor::EndFrame()
{
    ModernAPILayerManager* parentLayerManager = GetParentLayerManager();

    if (parentLayerManager == nullptr)
    {
        Log(logERROR, "ObjectDatabaseProcessor::EndFrame - parentLayerManager is NULL\n");
        return;
    }

    // The frame is over. Don't track any new object instance creation again until we've started a new frame.
    WrappedObjectDatabase* objectDatabase = GetObjectDatabase();
    objectDatabase->TrackObjectLifetime(false);

    // Check if the selection has been changed. This should be checked first in case future responses need it.
    UpdateSelectedObject();

    if (mObjectTypeResponse.IsActive())
    {
        gtASCIIString typeString;
        typeString.makeEmpty();

        if (false == parentLayerManager->InCapturePlayer())
        {
            if (mSelectedObject != NULL)
            {
                mSelectedObject->AppendTypeXML(typeString);
            }
        }
        else
        {
            if (nullptr != mSelectedObjectXml)
            {
                SelObjTypeFromDBase(typeString);
            }
        }

        mObjectTypeResponse.Send(typeString.asCharArray());
    }

    if (mObjectTreeResponse.IsActive() || mbObjectDatabaseForCapture)
    {
        // Create a response string by querying a bunch of objects and serializing an object hierarchy.
        gtASCIIString objectTreeResponseString;
        BuildObjectTreeResponse(objectTreeResponseString);

        if (objectTreeResponseString.length() > 0)
        {
            const char* responseString = objectTreeResponseString.asCharArray();

            if (!mbObjectDatabaseForCapture)
            {
                mObjectTreeResponse.Send(responseString);
            }
        }
    }

    if (mObjectTagResponse.IsActive())
    {
        gtASCIIString tagDataString;
        tagDataString.makeEmpty();

        // Write the tag data for the selected wrapper object (if there is any) to a string.
        if (false == parentLayerManager->InCapturePlayer())
        {
            if (mSelectedObject != NULL)
            {
                mSelectedObject->AppendTagDataXML(tagDataString);
            }
        }
        else
        {
            if (nullptr != mSelectedObjectXml)
            {
                SelObjTagFromDBase(tagDataString);
            }
        }

        mObjectTagResponse.Send(tagDataString.asCharArray());
    }

    // Check to see if the "Get CreateInfo" request is active. If so, look for the selected object in the wrapper database.
    // When found, write it's CreateInfo struct into the response.
    if (mObjectCreateInfoResponse.IsActive())
    {
        gtASCIIString createInfoXml;
        createInfoXml.makeEmpty();

        if (false == parentLayerManager->InCapturePlayer())
        {
            if (mSelectedObject != NULL)
            {
                mSelectedObject->AppendCreateInfoXML(createInfoXml);
            }
        }
        else
        {
            SelObjCreateInfoFromDBase(createInfoXml);
        }

        mObjectCreateInfoResponse.Send(createInfoXml.asCharArray());
    }

    // Retrieve a large dump of CreateInfo XML for all objects of a given type.
    if (mAllCreateInfoResponse.IsActive() || mbObjectDatabaseForCapture)
    {
        WrappedObjectDatabase* objDatabase = GetObjectDatabase();
        gtASCIIString allCreateInfoResponse;

        // First find out which object type we're interested in dumping.
        gtASCIIString argString(mAllCreateInfoResponse.GetValue());

        if (mbObjectDatabaseForCapture)
        {
            argString = "ALL";
        }

        // Is this a request to dump the info for a single object with a given handle?
        if (argString.startsWith("0x"))
        {
            // We received the handle as a hex string. Convert it to an address and look up the object in the database.
            std::vector<void*> addressHandle;
            bool bHandleConverted = ParseAddressesString(argString, addressHandle);

            if (bHandleConverted && addressHandle.size() == 1)
            {
                gtASCIIString instanceCreateInfo;

                IInstanceBase* instancePointer = objDatabase->GetWrappedInstance(addressHandle[0]);
                instancePointer->AppendCreateInfoXML(instanceCreateInfo);

                // Now include the handle as an attribute of the CreateInfo element within the response.
                gtASCIIString applicationHandleString;
                instancePointer->PrintFormattedApplicationHandle(applicationHandleString);
                gtASCIIString tagWithHandle;
                tagWithHandle.appendFormattedString("<CreateInfo handle=\"%s\"", applicationHandleString.asCharArray());
                instanceCreateInfo.replace("<CreateInfo", tagWithHandle, false);

                allCreateInfoResponse.append(instanceCreateInfo);
            }
            else
            {
                allCreateInfoResponse.appendFormattedString("Error: Failed to parse object handle '%s'.\n", argString.asCharArray());
            }
        }
        else if (argString.startsWith("ALL"))
        {
            // "ALL" save data response disabled
            allCreateInfoResponse.append("Error: AllCreateInfo.xml=ALL no longer supported.");
        }
        else
        {
            // This is a request to dump info for all objects with a given type.
            int objType = GetObjectTypeFromString(argString);
            int firstType = GetFirstObjectType();
            int lastType = GetLastObjectType();

            if (objType != -1 && (objType >= firstType && objType <= lastType))
            {
                // Dump each type of object in order.
                eObjectType objectTypeEnum = static_cast<eObjectType>(objType);

                WrappedInstanceVector objectVector;
                objDatabase->GetObjectsByType(objectTypeEnum, objectVector);

                if (!objectVector.empty())
                {
                    for (size_t objectIndex = 0; objectIndex < objectVector.size(); ++objectIndex)
                    {
                        gtASCIIString thisCreateInfo;
                        IInstanceBase* instancePointer = objectVector[objectIndex];
                        instancePointer->AppendCreateInfoXML(thisCreateInfo);

                        gtASCIIString applicationHandleString;
                        instancePointer->PrintFormattedApplicationHandle(applicationHandleString);

                        gtASCIIString tagWithHandle;
                        tagWithHandle.appendFormattedString("<CreateInfo handle=\"%s\"", applicationHandleString.asCharArray());
                        thisCreateInfo.replace("<CreateInfo", tagWithHandle, false);

                        allCreateInfoResponse.append(thisCreateInfo);
                    }
                }
            }
            else
            {
                allCreateInfoResponse.appendFormattedString("Error: Please choose an object type between '%d' and '%d'\n", firstType, lastType);
            }
        }

        if (!mbObjectDatabaseForCapture)
        {
            mAllCreateInfoResponse.Send(allCreateInfoResponse.asCharArray());
        }
    }

    // Send the response string back to the server through a specific request command.
    if ((mObjectTreeResponse.IsActive() || mObjectDatabaseResponse.IsActive()) && parentLayerManager->InCapturePlayer())
    {
        HandleObjInfoResponse();
    }
}

//--------------------------------------------------------------------------
/// Handle object tree or object database requestes.
//--------------------------------------------------------------------------
void ObjectDatabaseProcessor::HandleObjInfoResponse()
{
    ModernAPILayerManager* parentLayerManager = GetParentLayerManager();

    if (parentLayerManager == nullptr)
    {
        Log(logERROR, "ObjectDatabaseProcessor::HandleObjInfoResponse - parentLayerManager is NULL\n");
        return;
    }

    if (parentLayerManager->InCapturePlayer())
    {
        const std::string& metadataFile = parentLayerManager->GetPathToTargetMetadataFile();

        if (metadataFile.length() > 0)
        {
            // Read the metadata file and store the contents in a structure.
            TraceMetadata frameMetadata;
            frameMetadata.mFrameInfo = new FrameInfo;

            bool bReadMetadataFileSuccessfully = ReadMetadataFile(metadataFile, &frameMetadata);

            if (bReadMetadataFileSuccessfully)
            {
                gtASCIIString objContents;
                gtASCIIString objfile;
                bool bReadFileSuccessfully = false;

                if (mObjectTreeResponse.IsActive())
                {
                    bReadFileSuccessfully = LoadFile(frameMetadata.mPathToObjectTreeFile, objContents);

                    if (bReadFileSuccessfully)
                    {
                        mObjectTreeResponse.Send(objContents.asCharArray());
                    }
                    else
                    {
                        mObjectTreeResponse.Send("Failed");
                    }
                }
                else if (mObjectDatabaseResponse.IsActive())
                {
                    bReadFileSuccessfully = LoadFile(frameMetadata.mPathToObjectDatabaseFile, objContents);

                    if (bReadFileSuccessfully)
                    {
                        mObjectDatabaseResponse.Send(objContents.asCharArray());
                    }
                    else
                    {
                        mObjectDatabaseResponse.Send("Failed");
                    }
                }
                else
                {
                    // no read required, always success
                    bReadFileSuccessfully = true;
                }

                if (!bReadFileSuccessfully)
                {
                    Log(logERROR, "Failed to read frame info file at '%s'.", frameMetadata.mPathToObjectTreeFile.c_str());
                }
            }
            else
            {
                Log(logERROR, "Failed to read metadata file at '%s'.", metadataFile.c_str());
            }

            // Destroy the FrameInfo instance that was created above.
            SAFE_DELETE(frameMetadata.mFrameInfo);
        }
        else
        {
            Log(logERROR, "Failed to locate valid path to trace metadata file.");
        }
    }
}

//--------------------------------------------------------------------------
/// Write a string to XML file
/// \param xmlString pointer to a string for outputing to XML file, must be valid string with minimal 1 length
/// \param fullFilePath Full path with filename.
/// \returns True if writing the file was successful.
//--------------------------------------------------------------------------
bool ObjectDatabaseProcessor::WriteXMLFile(gtASCIIString* xmlString, const std::string& fullFilePath)
{
    bool bWriteSuccessful = false;
    gtString fullFilepathAsGTString;

    if ((nullptr == xmlString) || (1 > xmlString->length()))
    {
        return bWriteSuccessful;
    }

    fullFilepathAsGTString.fromASCIIString(fullFilePath.c_str());

    osFile xmlFile(fullFilepathAsGTString);
    bool bFileOpened = xmlFile.open(osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);

    if (bFileOpened)
    {
        // Write the string xml into the output file.
        gtASCIIString fullXMLString;
        fullXMLString = XML("XML", xmlString->asCharArray());

        // Convert the gtASCIIString to a gtString.
        gtString fullXMLAsGTString;
        fullXMLAsGTString.fromASCIIString(fullXMLString.asCharArray());

        // Write the XML into the file, and close.
        xmlFile.writeString(fullXMLAsGTString);
        xmlFile.close();

        Log(logMESSAGE, "Wrote XML ObjectDatabase file to '%s'.\n", fullFilePath.c_str());

        bWriteSuccessful = true;
    }
    else
    {
        Log(logERROR, "Failed to open file for writing: '%s'\n", fullFilePath.c_str());
    }

    return bWriteSuccessful;
}

//--------------------------------------------------------------------------
/// Retrieve full path for temp frame data storage
/// \param FrameStorageFolder   Full path of frame data storage folder
/// \returns                    True if folder retrieved successfully.
//--------------------------------------------------------------------------
bool ObjectDatabaseProcessor::GetFrameStorageFullPath(gtString& FrameStorageFolder)
{
    // Session folders that already exist
    gtASCIIString sessionString, pathToDataDirectory;
    SessionManagerData smd;

    ModernAPILayerManager* parentLayerManager = GetParentLayerManager();

    if (parentLayerManager == nullptr)
    {
        Log(logERROR, "ObjectDatabaseProcessor::GetFrameStorageFullPath - parentLayerManager is NULL\n");
        return false;
    }

    smd.frameIndex = parentLayerManager->GetCapturedFrameStartIndex();

    bool bFolderRetrieved = SessionManager::Instance()->GetSessionManagerData(smd);

    if (bFolderRetrieved == false)
    {
        return bFolderRetrieved;
    }

    pathToDataDirectory = smd.pathToDataDirectory;
    FrameStorageFolder.fromASCIIString(pathToDataDirectory.asCharArray());
    bFolderRetrieved = true;

    return bFolderRetrieved;
}

//--------------------------------------------------------------------------
/// Retrieve Object type string from currently selected Obj XML node, Capture player only
/// \param strObjType   formatted string for object type
/// \returns    True if object type retrieved successfully.
//--------------------------------------------------------------------------
bool ObjectDatabaseProcessor::SelObjTypeFromDBase(gtASCIIString& strObjType)
{
    bool bRtn = false;
    strObjType.makeEmpty();

    if (nullptr != mSelectedObjectXml)
    {
        // return 1st element name bracketed in <Type>
        strObjType.appendFormattedString("<Type>%s</Type>", mSelectedObjectXml->FirstChildElement()->Value());
        bRtn = true;
    }

    return bRtn;
}

//--------------------------------------------------------------------------
/// Retrieve Object tag string from currently selected Obj XML node, Capture player only
/// \param strTagName   formatted string for object tag
/// \returns    True if object tag retrieved successfully.
//--------------------------------------------------------------------------
bool ObjectDatabaseProcessor::SelObjTagFromDBase(gtASCIIString& strTagName)
{
    bool bRtn = false;
    strTagName.makeEmpty();

    if (nullptr != mSelectedObjectXml)
    {
        // Just return "test" for now, not used
        strTagName.append("test");
        bRtn = true;
    }

    return bRtn;
}

//--------------------------------------------------------------------------
/// Retrieve Object CreateInfo string from currently selected Obj XML node, Capture player only
/// \param strObjCreateInfo   formatted string for object CreateInfo
/// \returns    True if object CreateInfo retrieved successfully.
//--------------------------------------------------------------------------
bool ObjectDatabaseProcessor::SelObjCreateInfoFromDBase(gtASCIIString& strObjCreateInfo)
{
    bool bRtn = false;
    strObjCreateInfo.makeEmpty();

    if (nullptr != mSelectedObjectXml)
    {
        // print the entire node into XML text
        TiXmlPrinter xmlPrinter;
        mSelectedObjectXml->Accept(&xmlPrinter);
        std::stringstream ss;
        ss << xmlPrinter.CStr();

        strObjCreateInfo.append(ss.str().c_str());
        bRtn = true;
    }

    return bRtn;
}

//--------------------------------------------------------------------------
/// This CommandProcessor has responses that operate on vectors of memory addresses.
/// This utility function is responsible for splitting the input string by commas, and into a vector of addresses.
//--------------------------------------------------------------------------
bool ObjectDatabaseProcessor::ParseAddressesString(const gtASCIIString& inAddressesString, std::vector<void*>& outAddressStrings)
{
    bool bHaveAddresses = false;

    gtASCIIString addressStringCopy(inAddressesString);

    if (addressStringCopy.length() > 0)
    {
        std::list<gtASCIIString> addressList;
        addressStringCopy.Split(",", false, addressList);
        bHaveAddresses = !(addressList.empty());

        if (bHaveAddresses)
        {
            std::list<gtASCIIString>::iterator addressIter;
            addressIter = addressList.begin();

            // Convert from string to memory address.
            gtASCIIString& address = (*addressIter);
            unsigned long long memoryAddress = 0;
            bool bConversionSuccessful = address.toUnsignedLongLongNumber(memoryAddress);

            if (bConversionSuccessful)
            {
                outAddressStrings.push_back(reinterpret_cast<void*>(memoryAddress));
            }

            ++addressIter;
        }
    }

    return bHaveAddresses;
}

//--------------------------------------------------------------------------
/// Check if the selected object has been changed by the client. If so, update the selected object pointer.
//--------------------------------------------------------------------------
void ObjectDatabaseProcessor::UpdateSelectedObject()
{
    if (mSelectedObjectResponse.IsActive())
    {
        bool bObjectSelected = false;

        gtASCIIString appHandleASCIIString(mSelectedObjectResponse.GetValue());
        vector<void*> applicationHandles;
        ModernAPILayerManager* parentLayerManager = GetParentLayerManager();

        if (parentLayerManager == nullptr)
        {
            Log(logERROR, "ObjectDatabaseProcessor::UpdateSelectedObject - parentLayerManager is NULL\n");
            return;
        }

        if (!appHandleASCIIString.startsWith("NULL") && ParseAddressesString(appHandleASCIIString, applicationHandles))
        {
            if (!parentLayerManager->InCapturePlayer())
            {
                void* selectedObject = applicationHandles[0];
                IInstanceBase* selectedWrappedInstance = GetObjectDatabase()->GetWrappedInstance(selectedObject);

                if (selectedWrappedInstance != NULL)
                {
                    mSelectedObject = selectedWrappedInstance;
                    bObjectSelected = true;
                }
            }
            else
            {
                gtASCIIString  selectedObjHandle;
                selectedObjHandle.makeEmpty();
                selectedObjHandle.appendFormattedString("0x%p", applicationHandles[0]);

                if (mXmlDomObjectDatabase.NoChildren())
                {
                    LoadObjectDatabase();
                }

                // find the child with [selectedObjHandle]
                if (!mXmlDomObjectDatabase.NoChildren())
                {
                    TiXmlElement* pXmlElement;
                    gtASCIIString strHandle;

                    // Grab the 1st CreateInfo element, FullXML/XML/CreateInfo*
                    pXmlElement = mXmlDomObjectDatabase.FirstChildElement()->FirstChildElement();

                    while (nullptr != pXmlElement)
                    {
                        strHandle = pXmlElement->Attribute("handle");

                        if (0 == strHandle.compareNoCase(selectedObjHandle))
                        {
                            // object found
                            mSelectedObjectXml = pXmlElement;
                            bObjectSelected = true;
                            break;
                        }

                        pXmlElement = pXmlElement->NextSiblingElement();
                    }
                }
            }
        }
        else
        {
            // check database here
            if (false == parentLayerManager->InCapturePlayer())
            {
                // Just select the first device.
                WrappedInstanceVector devices;
                int deviceType = GetDeviceType();
                GetObjectDatabase()->GetObjectsByType((eObjectType)deviceType, devices);

                if (!devices.empty())
                {
                    mSelectedObject = devices[0];
                    bObjectSelected = true;
                }
            }
            else
            {
                if (mXmlDomObjectDatabase.NoChildren())
                {
                    LoadObjectDatabase();
                }

                // find the first createinfo child
                if (!mXmlDomObjectDatabase.NoChildren())
                {
                    // Grab the 1st CreateInfo element, FullXML/XML/CreateInfo*
                    mSelectedObjectXml = mXmlDomObjectDatabase.FirstChildElement()->FirstChildElement();
                    bObjectSelected = true;
                }
            }
        }

        // Respond to let the client know that selection worked fine.
        if (true == bObjectSelected)
        {
            mSelectedObjectResponse.Send("OK");
        }
        else
        {
            mSelectedObjectResponse.Send("Failed");
        }
    }
}

//--------------------------------------------------------------------------
/// A helper function that does the dirty work of building object XML.
/// \param outObjectTreeXml An out-param that contains the object tree XML response.
/// \returns Nothing. The result is returned through an out-param string.
//--------------------------------------------------------------------------
void ObjectDatabaseProcessor::BuildObjectTreeResponse(gtASCIIString& outObjectTreeXml)
{
    ModernAPILayerManager* parentLayerManager = GetParentLayerManager();

    if (parentLayerManager == nullptr)
    {
        Log(logERROR, "ObjectDatabaseProcessor::BuildObjectTreeResponse - parentLayerManager is NULL\n");
        return;
    }

    if (!parentLayerManager->InCapturePlayer())
    {
        // Separate all of the objects into categories based on the object type.
        WrappedObjectDatabase* objectDatabase = GetObjectDatabase();

        // Find the device wrappers first- they are the roots of our treeview.
        int deviceType = GetDeviceType();
        WrappedInstanceVector deviceWrappers;
        objectDatabase->GetObjectsByType((eObjectType)deviceType, deviceWrappers);

        // Find the object instances created under each device to create a tree.
        for (size_t deviceIndex = 0; deviceIndex < deviceWrappers.size(); ++deviceIndex)
        {
            // Get the device's application handle from the wrapper. Look for objects with a handle to the parent device.
            IInstanceBase* deviceInstance = deviceWrappers[deviceIndex];

            // Don't bother building an object tree for a device that's been destroyed.
            if (!deviceInstance->IsDestroyed())
            {
                gtASCIIString applicationHandleString;
                deviceInstance->PrintFormattedApplicationHandle(applicationHandleString);

                // The object XML for each active device will be appended into a long string here.
                gtASCIIString deviceObjectXml = "";

                // Look for objects of all types besides "Device". We'll handle those last.
                int firstObjectType = GetFirstObjectType();
                int lastObjectType = GetLastObjectType();

                for (int objectType = firstObjectType; objectType < lastObjectType; objectType++)
                {
                    // Step through each type of object and store the results in vectors.
                    eObjectType currentType = (eObjectType)objectType;

                    if (currentType == GetDeviceType())
                    {
                        // Skip devices. They're the highest level node in the hierarchy. We're only looking for device children.
                        continue;
                    }

                    // We need to empty out the list of objects with the current type so that objects from the 0th device don't get added in.
                    WrappedInstanceVector objectsOfType;
                    objectDatabase->GetObjectsByType(currentType, objectsOfType);

                    // There's at least a single instance of this type within the object database. Serialize the object to the tree.
                    if (!objectsOfType.empty())
                    {
                        // Objects of the same type will be sorted into similar groups, and then divided based on parent device.
                        gtASCIIString instancesString = "";
                        size_t numInstances = objectsOfType.size();
                        const char* objectTypeAsString = objectsOfType[0]->GetTypeAsString();

                        for (size_t instanceIndex = 0; instanceIndex < numInstances; ++instanceIndex)
                        {
                            IInstanceBase* objectInstance = objectsOfType[instanceIndex];

                            // Check to see if the object's parent device matches the one we're building the tree for.
                            if (objectInstance->GetParentDeviceHandle() == deviceInstance->GetApplicationHandle())
                            {
                                gtASCIIString objectHandleString;
                                objectInstance->PrintFormattedApplicationHandle(objectHandleString);
                                instancesString.append(objectHandleString);

                                if (objectInstance->IsDestroyed())
                                {
                                    // Append a "|d" to indicate that this object instance was deleted during the frame.
                                    instancesString.append("|d");
                                }

                                if ((instanceIndex + 1) < numInstances)
                                {
                                    instancesString.append(",");
                                }
                            }
                        }

                        deviceObjectXml += XML(objectTypeAsString, instancesString.asCharArray());
                    }
                }

                // Surround all of the subobjects in an object element.
                gtASCIIString deviceAddressAttributeString;
                deviceAddressAttributeString.appendFormattedString("handle='%s'", applicationHandleString.asCharArray());
                outObjectTreeXml += XMLAttrib("Device", deviceAddressAttributeString.asCharArray(), deviceObjectXml.asCharArray());
            }
        }

        outObjectTreeXml = XML("Objects", outObjectTreeXml.asCharArray());
    }
    else
    {
        // load tree from cache folder
        gtString fullFilepathAsGTString = L"";
        std::string dataPath = parentLayerManager->GetPathToTargetMetadataFile();

        dataPath = dataPath.substr(0, dataPath.rfind("\\") + 1);

        fullFilepathAsGTString.fromASCIIString(dataPath.c_str());
        fullFilepathAsGTString.append(L"ObjectTree.xml");

        osFile xmlFile(fullFilepathAsGTString);

        if (xmlFile.exists())
        {
            bool bFileOpened = xmlFile.open(osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_READ);

            if (bFileOpened)
            {
                gtString fullXMLString;

                // Real all XML from file.
                xmlFile.readIntoString(outObjectTreeXml);
                xmlFile.close();

                Log(logMESSAGE, "Read XML tree from '%s'.\n", fullFilepathAsGTString.asASCIICharArray());
            }
            else
            {
                Log(logERROR, "Failed to open file for xml read: '%s'\n", fullFilepathAsGTString.asASCIICharArray());
            }
        }
        else
        {
            Log(logERROR, "Xml read, file missing: '%s'\n", fullFilepathAsGTString.asASCIICharArray());
        }
    }
}

//--------------------------------------------------------------------------
/// A helper function that loads full object database from disk cache into class storage
/// \returns true if database is loaded successfully
//--------------------------------------------------------------------------
bool ObjectDatabaseProcessor::LoadObjectDatabase()
{
    bool bRtn = false;
    ModernAPILayerManager* parentLayerManager = GetParentLayerManager();

    if (parentLayerManager == nullptr)
    {
        Log(logERROR, "ObjectDatabaseProcessor::LoadObjectDatabase - parentLayerManager is NULL\n");
        return bRtn;
    }

    if (mXmlDomObjectDatabase.NoChildren() || parentLayerManager->InCapturePlayer())
    {
        std::string dataPath = parentLayerManager->GetPathToTargetMetadataFile();

        dataPath = dataPath.substr(0, dataPath.rfind("\\") + 1);

        gtString fullFilepathAsGTString = L"";

        fullFilepathAsGTString.fromASCIIString(dataPath.c_str());
        fullFilepathAsGTString.append(L"FullObjectDatabase.xml");

        osFile xmlFile(fullFilepathAsGTString);

        if (xmlFile.exists())
        {
            bool bFileOpened = xmlFile.open(osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_READ);

            if (bFileOpened)
            {
                // Read the file xml into a string.
                gtASCIIString fullXMLString;

                // Real all XML from file.
                xmlFile.readIntoString(fullXMLString);
                xmlFile.close();

                mXmlDomObjectDatabase.Parse(fullXMLString.asCharArray());
                bRtn = true;
            }
        }
    }

    return bRtn;
}

//--------------------------------------------------------------------------
/// Load a text/xml file from disk when given a valid path.
/// \param inFilepath The full filepath to a text file.
/// \param outFileContents The full contents of the loaded text file.
/// \returns True if the text file was loaded correctly.
//--------------------------------------------------------------------------
bool ObjectDatabaseProcessor::LoadFile(const std::string& inFilepath, gtASCIIString& outFileContents)
{
    bool bReadSuccessful = false;
    gtString Filepath;
    Filepath.fromASCIIString(inFilepath.c_str());
    osFile textFile(Filepath);
    bool bTraceFileOpened = textFile.open(osChannel::OS_ASCII_TEXT_CHANNEL);

    if (bTraceFileOpened)
    {
        // Read the entire file and return the contents through the output string.
        if (textFile.readIntoString(outFileContents))
        {
            bReadSuccessful = true;
        }
        else
        {
            Log(logERROR, "Failed to read text file at path '%s'.", inFilepath.c_str());
        }
    }

    return bReadSuccessful;
}

//--------------------------------------------------------------------------
/// Enable collection of a linked trace before a new frame is started.
//--------------------------------------------------------------------------
void ObjectDatabaseProcessor::EnableObjectDatabaseCollection()
{
    mbObjectDatabaseForCapture = true;
}

//--------------------------------------------------------------------------
/// Disable collection of a linked trace after a new frame has finished.
//--------------------------------------------------------------------------
void ObjectDatabaseProcessor::DisableObjectDatabaseCollection()
{
    mbObjectDatabaseForCapture = false;
}
