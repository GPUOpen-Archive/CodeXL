//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osSettingsFileHandler.cpp
///
//=====================================================================

//------------------------------ osFile.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osSettingsFileHandler.h>


// ---------------------------------------------------------------------------
// Name:        osSettingsFileHandler::osSettingsFileHandler
// Description: Constructor
// Arguments: settingsFilePath - The settings file path.
// Author:      AMD Developer Tools Team
// Date:        12/11/2007
// ---------------------------------------------------------------------------
osSettingsFileHandler::osSettingsFileHandler(const osFilePath& settingsFilePath)
    : _settingsFilePath(settingsFilePath)
{
}


// ---------------------------------------------------------------------------
// Name:        osSettingsFileHandler::~osSettingsFileHandler
// Description: Destructor.
// Author:      AMD Developer Tools Team
// Date:        12/11/2007
// ---------------------------------------------------------------------------
osSettingsFileHandler::~osSettingsFileHandler()
{
}


osSettingsFileHandler& osSettingsFileHandler::operator=(const osSettingsFileHandler& other)
{
    _settingsFilePath = other._settingsFilePath;
    _settingsFileAsXMLDocument = other._settingsFileAsXMLDocument;
    return *this;
}


// ---------------------------------------------------------------------------
// Name:        osSettingsFileHandler::initFromSettingsFileDefaultValues
// Description: Initialize the XML document to contain the settings file
//              default values.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        15/11/2007
// ---------------------------------------------------------------------------
bool osSettingsFileHandler::initFromSettingsFileDefaultValues()
{
    bool retVal = initXMLDocToSettingsFileDefaultValues(_settingsFileAsXMLDocument);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osSettingsFileHandler::loadSettingsFile
// Description:
//   Loads the settings file from disk into an XML document.
//
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        11/11/2007
// ---------------------------------------------------------------------------
bool osSettingsFileHandler::loadSettingsFile()
{
    bool retVal = false;

    // Verify that the settings file exists and is a regular file:
    bool rc1 = _settingsFilePath.isRegularFile();

    if (!rc1)
    {
        // Output an error message:
        gtString errMsg = OS_STR_cannotAccessSettingsFile;
        errMsg += _settingsFilePath.asString();
        GT_ASSERT_EX(false, errMsg.asCharArray());
    }
    else
    {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        // Unfortunately, TinyXML uses fopen which does not support unicode paths on Windows.
        // To workaround this, on Windows we open the file ourselves and pass the file pointer to TinyXML for loading.
        FILE* pXmlFile = _wfopen(_settingsFilePath.asString().asCharArray(), L"rb");

        if (pXmlFile)
        {
            // Load the settings file:
            retVal = _settingsFileAsXMLDocument.LoadFile(pXmlFile);
            fclose(pXmlFile);
        }

#else
        // On Linux the function fopen supports UTF8 paths so we need to convert the path to UTF8 before proceeding
        std::string utf8Path;
        _settingsFilePath.asString().asUtf8(utf8Path);
        // Load the settings file:
        retVal = _settingsFileAsXMLDocument.LoadFile(utf8Path.c_str());
#endif // #if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS 
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osSettingsFileHandler::saveSettingsFile
// Description: Saves the XML document to the settings file on disk.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        11/11/2007
// ---------------------------------------------------------------------------
bool osSettingsFileHandler::saveSettingsFile()
{
    bool retVal = false;

    // Create the settings file directory:
    bool rc1 = createSettingsFileDirectory();
    GT_IF_WITH_ASSERT(rc1)
    {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        // Unfortunately, TinyXML uses fopen which does not support unicode paths on Windows.
        // To workaround this, on Windows we open the file ourselves and pass the file pointer to TinyXML.
        FILE* pXmlFile = _wfopen(_settingsFilePath.asString().asCharArray(), L"w");

        if (pXmlFile)
        {
            // Save the settings file:
            retVal = _settingsFileAsXMLDocument.SaveFile(pXmlFile);
            fclose(pXmlFile);
        }

#else
        // On Linux the function fopen supports UTF8 paths so we need to convert the path to UTF8 before proceeding
        std::string utf8Path;
        _settingsFilePath.asString().asUtf8(utf8Path);
        // Load the settings file:
        retVal = _settingsFileAsXMLDocument.SaveFile(utf8Path.c_str());
#endif // #if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS 

    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osSettingsFileHandler::writeBoolToXmlNode
// Description: Writes a boolean value into an XML node.
// Arguments: pXmlNode - The XML node who's string value will be changes to contain
//                       the input boolean value.
//            booleanValue - The input boolean value.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        12/11/2007
// ---------------------------------------------------------------------------
bool osSettingsFileHandler::writeBoolToXmlNode(TiXmlNode* pXmlNode, bool booleanValue)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(pXmlNode != NULL)
    {
        // Get the node text:
        TiXmlText* pXmlNodeText = pXmlNode->ToText();
        GT_IF_WITH_ASSERT(pXmlNodeText != NULL)
        {
            // Set its value:
            if (booleanValue)
            {
                pXmlNodeText->SetValue(OS_STR_TrueXMLValue);
            }
            else
            {
                pXmlNodeText->SetValue(OS_STR_FalseXMLValue);
            }

            retVal = true;
        }
    }

    GT_RETURN_WITH_ASSERT(retVal);
}


// ---------------------------------------------------------------------------
// Name:        osSettingsFileHandler::readBoolFromXMLNode
// Description: Reads a boolean value from an XML node.
// Arguments: pXmlNode - The XML node who's string value will be read and
//                       converted to the output boolean value.
//            booleanValue - The output boolean value.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        12/11/2007
// ---------------------------------------------------------------------------
bool osSettingsFileHandler::readBoolFromXMLNode(const TiXmlNode* pXmlNode, bool& booleanValue) const
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(pXmlNode != NULL)
    {
        // Get the node text:
        const TiXmlText* pXmlNodeText = pXmlNode->ToText();
        GT_IF_WITH_ASSERT(pXmlNodeText != NULL)
        {
            // Convert it to a string:
            gtString stringValue;
            stringValue.fromASCIIString(pXmlNodeText->Value());

            // Set its value:
            if (stringValue == OS_STR_TrueXMLValueUnicode)
            {
                booleanValue = true;
            }
            else
            {
                booleanValue = false;
            }

            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osSettingsFileHandler::writeStringToXmlNode
// Description: Writes a string value into an XML node.
// Arguments: pXmlNode - The XML node who's string value will be changes.
//            stringValue - The new XML node string value.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        12/11/2007
// ---------------------------------------------------------------------------
bool osSettingsFileHandler::writeStringToXmlNode(TiXmlNode* pXmlNode, const gtString& stringValue)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(pXmlNode != NULL)
    {
        // Get the node text:
        TiXmlText* pXmlNodeText = pXmlNode->ToText();
        GT_IF_WITH_ASSERT(pXmlNodeText != NULL)
        {
            // If the input string is empty:
            if (stringValue.isEmpty())
            {
                pXmlNodeText->SetValue(OS_STR_EmptyXMLString);
            }
            else
            {
                std::string utf8String;
                stringValue.asUtf8(utf8String);
                pXmlNodeText->SetValue(utf8String.c_str());
            }

            retVal = true;
        }
    }

    GT_RETURN_WITH_ASSERT(retVal);
}


// ---------------------------------------------------------------------------
// Name:        osSettingsFileHandler::readStringFromXmlNode
// Description: Reads a string value from an XML node.
// Arguments: pXmlNode - The XML node who's string value will be read.
//            booleanValue - Will get the XML nodes string value.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        12/11/2007
// ---------------------------------------------------------------------------
bool osSettingsFileHandler::readStringFromXmlNode(const TiXmlNode* pXmlNode, gtString& stringValue) const
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(pXmlNode != NULL)
    {
        // Get the node text:
        const TiXmlText* pXmlNodeText = pXmlNode->ToText();
        GT_IF_WITH_ASSERT(pXmlNodeText != NULL)
        {
            // Convert it to a string:
            std::string utf8String = pXmlNodeText->Value();
            stringValue.fromUtf8String(utf8String);
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osSettingsFileHandler::createSettingsFileDirectory
// Description:
//  Verifies that the settings file directory exists. If it does not exists -
//  creates it.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        12/11/2007
// ---------------------------------------------------------------------------
bool osSettingsFileHandler::createSettingsFileDirectory()
{
    bool retVal = false;

    // Get the settings file directory path:
    osDirectory destinationDir;
    _settingsFilePath.getFileDirectory(destinationDir);

    // If the directory already exists:
    if (destinationDir.exists())
    {
        retVal = true;
    }
    else
    {
        // Create the destination directory:
        retVal = destinationDir.create();
    }

    return retVal;
}

bool osSettingsFileHandler::WriteAttributeToXMLNode(TiXmlNode* pXmlNode, const gtString& attributeName, const gtString& attributeValue)
{
    bool retVal = false;

    if ((pXmlNode != NULL) && (pXmlNode->ToElement() != NULL))
    {
        pXmlNode->ToElement()->SetAttribute(attributeName.asASCIICharArray(), attributeValue.asASCIICharArray());
        retVal = true;
    }

    return retVal;
}

bool osSettingsFileHandler::ReadAttributeFromXMLNode(const TiXmlNode* pXmlNode, const gtString& attributeName, gtString& attributeValue)
{
    bool retVal = false;

    if ((pXmlNode != NULL) && (pXmlNode->ToElement() != NULL))
    {
        const char* pAttr = pXmlNode->ToElement()->Attribute(attributeName.asASCIICharArray());

        if (pAttr != NULL)
        {
            attributeValue.fromASCIIString(pAttr);
            retVal = true;
        }
    }

    return retVal;
}


