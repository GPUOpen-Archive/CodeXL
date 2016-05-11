//------------------------------ gpProjectSettingsExtension.h ------------------------------

#ifndef __GPPROJECTSETTINGSEXTENSION_H
#define __GPPROJECTSETTINGSEXTENSION_H

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTApplicationFramework/Include/afProjectSettingsExtension.h>

// Local:
#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>

// ----------------------------------------------------------------------------------
// Class Name:           gpProjectSettingsExtension : public afProjectSettingsExtension
// General Description:  This class is used for handling the thread profiling project settings
// ----------------------------------------------------------------------------------
class AMDT_GPU_PROF_API gpProjectSettingsExtension : public afProjectSettingsExtension
{
    Q_OBJECT

public:
    gpProjectSettingsExtension();
    virtual ~gpProjectSettingsExtension();

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

protected slots:
    /// Handle the changes of the controls. This is needed to know when to updated the settings
    void OnAutomaticClicked(bool);
    void OnConnectionSelected(int);
    void OnTextEdited(const QString&);
    void OnNumFramesEdited(const QString& text);

private:

    /// Project settings widgets:
    QCheckBox* m_pAutomaticCheckBox;
    QComboBox* m_pOptionsComboBox;
    QLineEdit* m_pOptionsEdit;
    QLineEdit* m_pPortLineEdit;

    /// should update the settings only if something changed
    bool m_bUpdateSettings;

    QString m_processName;
    QString m_processNumber;

    QIntValidator m_validator;

    /// number of frames to capture in each capture action
    QLineEdit* m_pNumberFramesEdit;

    QString m_numFramesToCapture;
};

#endif //__GPPROJECTSETTINGSEXTENSION_H

