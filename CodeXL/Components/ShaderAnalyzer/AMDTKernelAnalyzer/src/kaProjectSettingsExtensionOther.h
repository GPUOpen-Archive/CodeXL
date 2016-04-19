//------------------------------ kaProjectSettingsExtensionOther.h ------------------------------

#ifndef __KAPROJECTSETTINGSEXTENSIONOTHER_H
#define __KAPROJECTSETTINGSEXTENSIONOTHER_H

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTApplicationFramework/Include/afProjectSettingsExtension.h>

// Local:
#include <AMDTKernelAnalyzer/Include/kaAMDTKernelAnalyzerDLLBuild.h>
#include <AMDTKernelAnalyzer/src/kaProjectSettingsExtensionBase.h>

// ----------------------------------------------------------------------------------
// Class Name:           kaDebugActionsCreator : public afActionCreatorAbstract
// General Description:  This class is used for handling the creation and execution of
//                       all actions related to the source code editing
// Author:               Gilad Yarnitzky
// Date:                 5/8/2013
// ----------------------------------------------------------------------------------
class KA_API kaProjectSettingsExtensionOther : public kaProjectSettingsExtensionBase
{
    Q_OBJECT

public:
    kaProjectSettingsExtensionOther();
    virtual ~kaProjectSettingsExtensionOther();

    /// Initialize the widget:
    virtual void Initialize();

    /// Return the extension name:
    virtual gtString ExtensionXMLString();

    /// Return the extension page title:
    virtual gtString ExtensionTreePathAsString();

    /// Load / Save the project settings into a string:
    virtual bool GetXMLSettingsString(gtString& projectAsXMLString);
    virtual bool SetSettingsFromXMLString(const gtString& projectAsXMLString);
    virtual void RestoreDefaultProjectSettings();
    virtual bool AreSettingsValid(gtString& invalidMessageStr);

    virtual bool RestoreCurrentSettings();

    /// Get the data from the widget:
    virtual bool SaveCurrentSettings();
protected:
    virtual void UpdateProjectDataManagerWithTBOptions(const QString& value, QStringList& optionsList) override;
private:
    /// Generate the table of controls
    void GenerateOptionsTable();
};

#endif //__KAPROJECTSETTINGSEXTENSION_H

