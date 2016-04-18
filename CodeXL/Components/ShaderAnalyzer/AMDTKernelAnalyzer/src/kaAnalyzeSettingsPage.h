//------------------------------ kaAnalyzeSettingsPage.h ------------------------------

#ifndef __KATARGETDEVICESETTINGPAGE_H
#define __KATARGETDEVICESETTINGPAGE_H

#include <QtWidgets>

class TiXmlNode;
class kaTreeModel;

// Infra:
#include <AMDTApplicationFramework/Include/afGlobalSettingsPage.h>
#include <AMDTApplicationComponents/Include/acTreeCtrl.h>

// Local:
#include <AMDTKernelAnalyzer/Include/kaAMDTKernelAnalyzerDLLBuild.h>

class acListCtrl;
class KA_API kaAnalyzeSettingsPage : public afGlobalSettingsPage
{
    Q_OBJECT

public:
    kaAnalyzeSettingsPage();
    virtual ~kaAnalyzeSettingsPage();

    // Must be overridden by child classes:
    virtual void initialize();

    // Names for the extension:
    virtual gtString pageTitle(); // Must be unique
    virtual gtString xmlSectionTitle(); // Can be shared between different pages

    // Load / Save the settings into a string:
    virtual bool getXMLSettingsString(gtString& projectAsXMLString);
    virtual bool setSettingsFromXMLString(const gtString& projectAsXMLString);

    virtual void loadCurrentSettings();

    // Restore the content to default settings:
    virtual void restoreDefaultSettings();

    // Save the data as it was changed in the widget to the specific settings manager (when "Ok" is pressed):
    virtual bool saveCurrentSettings();

    /// Check if the page has valid data or the user need to take farther action
    virtual bool IsPageDataValid();

private:

    /// Check if the driver version is not too old
    void checkDriverVersion();

    // Replace current model with a new one of a specific QStringList:
    void replaceModel(QStringList& modelStringList);

    /// Information caption
    QLabel* m_pInformationCaption;

    // Main Layout:
    QVBoxLayout* m_pMainLayout;

    // Tree object
    QTreeView* m_pTargetDevicesTreeView;

    // Tree model of devices:
    kaTreeModel* m_pTargetDevicesTreeModel;
};

#endif // __KATARGETDEVICESETTINGPAGE_H

