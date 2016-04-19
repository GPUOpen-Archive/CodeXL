//------------------------------ kaProjectSettingsExtension.h ------------------------------

#ifndef __KAPROJECTSETTINGSEXTENSION_H
#define __KAPROJECTSETTINGSEXTENSION_H

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtMap.h>

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
class KA_API kaProjectSettingsExtension : public kaProjectSettingsExtensionBase
{
    Q_OBJECT

public:
    kaProjectSettingsExtension();
    virtual ~kaProjectSettingsExtension();

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

private:
    /// Generate the table of controls
    void GenerateOptionsTable();

    /// Original build options so it can be restored if the user cancel
    QString m_originalBuildOptions;

    // Inherited via kaProjectSettingsExtensionBase
    virtual void UpdateProjectDataManagerWithTBOptions(const QString& value, QStringList& optionsList) override;
    virtual void UpdateProjectDataManagerWithTBOptions(const QString& value) override;
private:
    QLineEdit* m_pMacrosLineEdit;
};

#endif //__KAPROJECTSETTINGSEXTENSION_H

