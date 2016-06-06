//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osSettingsFileHandler.h
///
//=====================================================================

//------------------------------ osSettingsFileHandler.h ------------------------------

#ifndef __OSSETTINGSFILEHANDLER_H
#define __OSSETTINGSFILEHANDLER_H

// Tiny XML:
#include <tinyxml.h>

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local:
#include <AMDTOSWrappers/Include/osOSWrappersDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           OS_API osSettingsFileHandler
// General Description:
//   Base class for settings files handlers.
//   Supplies load, store, etc services that have common behavior across different
//   settings files handlers.
//
// Author:      AMD Developer Tools Team
// Creation Date:        12/4/2005
// ----------------------------------------------------------------------------------
class OS_API osSettingsFileHandler
{
public:
    osSettingsFileHandler(const osFilePath& settingsFilePath);
    virtual ~osSettingsFileHandler();

    bool initFromSettingsFileDefaultValues();
    bool loadSettingsFile();
    bool saveSettingsFile();


    osSettingsFileHandler& operator=(const osSettingsFileHandler& other);

protected:
    virtual bool initXMLDocToSettingsFileDefaultValues(TiXmlDocument& XMLDocument) = 0;

    // Utility functions to be used by sub-classes:
    bool writeBoolToXmlNode(TiXmlNode* pXmlNode, bool booleanValue);
    bool readBoolFromXMLNode(const TiXmlNode* pXmlNode, bool& booleanValue) const;
    bool writeStringToXmlNode(TiXmlNode* pXmlNode, const gtString& stringValue);
    bool readStringFromXmlNode(const TiXmlNode* pXmlNode, gtString& stringValue) const;

    /// Write an attribute to the XML node. This will build the following XML structure:
    /// <node attributeName=attributeValue></node>
    /// \param pXmlNode the XML node pointer
    /// \param attributeName the attribute name
    /// \param attributeValue the attribute value
    /// \return true for success
    bool WriteAttributeToXMLNode(TiXmlNode* pXmlNode, const gtString& attributeName, const gtString& attributeValue);

    /// Read an attribute from an XML node. Expecting the following XML structure:
    /// <node attributeName=attributeValue></node>
    /// \param pXmlNode the XML node pointer
    /// \param attributeName the attribute name
    /// \param attributeValue[out] the attribute value
    /// \return true for success
    bool ReadAttributeFromXMLNode(const TiXmlNode* pXmlNode, const gtString& attributeName, gtString& attributeValue);

private:
    bool createSettingsFileDirectory();

protected:
    // The settings file path:
    osFilePath _settingsFilePath;

    // An XML document that holds the settings file content:
    TiXmlDocument _settingsFileAsXMLDocument;
};


#endif //__OSSETTINGSFILEHANDLER_H

