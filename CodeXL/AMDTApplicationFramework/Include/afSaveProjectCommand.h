//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afSaveProjectCommand.h
///
//==================================================================================

#ifndef __AFSAVEPROJECTCOMMAND_H
#define __AFSAVEPROJECTCOMMAND_H

//TinyXml
#include <tinyxml.h>


// Infra:
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTOSWrappers/Include/osFilePath.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afCommand.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>
#include <AMDTApplicationFramework/Include/afSaveProjectCommand.h>

// ----------------------------------------------------------------------------------
// Class Name:          AF_API afSaveProjectCommand : public afCommand
// General Description: A command which is handling the save of an application framework project
// Author:              Sigal Algranaty
// Creation Date:       4/4/2012
// ----------------------------------------------------------------------------------
class AF_API afSaveProjectCommand : public afCommand
{
public:

    afSaveProjectCommand(const osFilePath& fileName);
    virtual ~afSaveProjectCommand();

    // Overrides afCommand:
    virtual bool canExecuteSpecificCommand();
    virtual bool executeSpecificCommand();

    /// This function should be used when the save command should save a project other then the current project:
    /// pProjectSettings pointer to the project which its settings should be set
    /// extensionSettingsAsString vector containing strings to the extensions settings
    void SetProjectSettings(const apProjectSettings* pProjectSettings, const gtVector<gtString>& extensionSettingsAsString);

protected:

    bool createAppDataDir();
    bool SaveGeneralProjectSettings();
    bool SaveExtensionsProjectSettings(TiXmlHandle& docHandle);
    bool setDebuggedApplicationEnvironmentVariables(TiXmlHandle& docHandle);
    bool initXMLFileStructure(TiXmlDocument& xmlDoc);
    bool setDebuggedApplicationData(TiXmlHandle& docHandle, const gtString& dataFieldName, const gtString& dataFieldValue);

protected:

    osFilePath _filePath;

    /// A pointer to the settings that should be saved:
    const apProjectSettings* m_pProjectSettings;

    /// Map of strings of the extensions settings:
    gtVector<gtString> m_extensionSettingsStrings;
};


#endif //__AFSAVEPROJECTCOMMAND_H

