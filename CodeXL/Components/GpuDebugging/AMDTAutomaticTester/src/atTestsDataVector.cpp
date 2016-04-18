//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file atTestsDataVector.cpp
///
//==================================================================================

/// Local:
#include <inc/atTestsDataVector.h>
#include <inc/atStringConstants.h>
#include <inc/atLoadTestsCommand.h>

atTestsDataVector::atTestsDataVector()
    : m_bIsXmlFileLoadedOK(true)
{
    addTestsFromXMLFile(TESTS_DATA_XML_FILE_NAME);
}

atTestsDataVector::atTestsDataVector(const gtString& xmlPath)
    : m_bIsXmlFileLoadedOK(true)
{
    addTestsFromXMLFile(xmlPath);
}

void atTestsDataVector::addTestsFromXMLFile(const gtString& xmlPath)
{
    gtString xmlFilePath = xmlPath;

    // If the path is not a full path;
    if (-1 == xmlPath.find(osFilePath::osPathSeparator))
    {
        // Create a path to the XML file in the same folder as the test executable
        osFilePath filePath(osFilePath::OS_CURRENT_DIRECTORY);
        xmlFilePath = filePath.reinterpretAsDirectory().asString();
        xmlFilePath.removeTrailing(osFilePath::osPathSeparator).append(osFilePath::osPathSeparator);
        xmlFilePath.append(xmlPath);
    }

    // Load the XML file containing the tests data
    atLoadTestsCommand loadTestDataFromXML(xmlFilePath, *this);
    m_bIsXmlFileLoadedOK = loadTestDataFromXML.execute() && m_bIsXmlFileLoadedOK;
}

