//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afLoadProjectCommand.h
///
//==================================================================================

#ifndef __AFLOADPROJECTCOMMAND
#define __AFLOADPROJECTCOMMAND

// TinyXml:
#include <tinyxml.h>

// Infra:
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTAPIClasses/Include/apProjectSettings.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afCommand.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:          AF_API afLoadProjectCommand : public afCommand
// General Description: Loads a CodeXL project from a cxl file
// Author:              Sigal Algranaty
// Creation Date:       9/4/2012
// ----------------------------------------------------------------------------------
class AF_API afLoadProjectCommand : public afCommand
{
public:
    afLoadProjectCommand(const gtString& projectFilePath);
    virtual ~afLoadProjectCommand();

    // Overrides afCommand:
    virtual bool canExecuteSpecificCommand();
    virtual bool executeSpecificCommand();

protected:

    bool SaveCurrentProject();
    bool ReadProjectGenericSettings();
    bool ReadProjectExtenstionsData(TiXmlHandle& docHandle);
    bool ReadProjectEnvironmentVariable(TiXmlHandle& docHandle);
    bool setExtensionXMLData();
    TiXmlNode* getProjectRootChildNode(TiXmlHandle& docHandle, const QString& nodeName, const QString& childNodeName = "", bool getFirstChild = true);
    static void readXmlFieldValue(TiXmlText& xmlText, gtString& fieldValue);
protected:

    // The loaded XML file path:
    gtString m_filePath;

    // The loaded project settings:
    apProjectSettings m_projectSettings;

    // Map that holds the extensions XML strings:
    gtMap<gtString, gtString> m_extensionXMLStringsMap;

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    // Special check for existence of gtkglext library
    bool IsTeapotNeededLibraryInstalled();
#endif

};

#endif  // __AFLOADPROJECTCOMMAND

