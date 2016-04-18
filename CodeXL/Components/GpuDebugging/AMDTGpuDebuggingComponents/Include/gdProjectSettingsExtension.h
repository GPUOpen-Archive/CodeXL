//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdProjectSettingsExtension.h
///
//==================================================================================

//------------------------------ gdProjectSettingsExtension.h ------------------------------

#ifndef __GDPROJECTSETTINGSEXTENSION_H
#define __GDPROJECTSETTINGSEXTENSION_H

// Qt:
#include <QtWidgets>

QT_BEGIN_NAMESPACE
class QCheckBox;
class QLineEdit;
class QPushButton;
QT_END_NAMESPACE

// Infra:
#include <AMDTApplicationFramework/Include/afProjectSettingsExtension.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:           gwDebugActionsCreator : public afActionCreatorAbstract
// General Description:  This class is used for handling the creation and execution of
//                       all actions related to the source code editing
// Author:               Sigal Algranaty
// Date:                 16/9/2011
// ----------------------------------------------------------------------------------
class GD_API gdProjectSettingsExtension : public afProjectSettingsExtension
{
    Q_OBJECT

public:
    gdProjectSettingsExtension();
    virtual ~gdProjectSettingsExtension();

    // Initialize the widget:
    virtual void Initialize();

    // Return the extension name:
    virtual gtString ExtensionXMLString();

    // Return the extension page title:
    virtual gtString ExtensionTreePathAsString();

    // Load / Save the project settings into a string:
    virtual bool GetXMLSettingsString(gtString& projectAsXMLString);
    virtual bool SetSettingsFromXMLString(const gtString& projectAsXMLString);
    virtual void RestoreDefaultProjectSettings();
    virtual bool AreSettingsValid(gtString& invalidMessageStr);

    virtual bool RestoreCurrentSettings();

    // Get the data from the widget:
    virtual bool SaveCurrentSettings();

protected slots:
    void onAddRemoveBreakpoints();

protected:

    // Create controls:
    void createGLControls(QVBoxLayout* pGLFrameTerminatorsLayout);
    void createCLControls(QVBoxLayout* pCLFrameTerminatorsLayout);
    bool saveProjectSettingsFromControls();
    void setFrameTerminator(unsigned int frameTerminators);
    unsigned int getCurrentFrameTerminators();

protected:

    // Controls contained in the widget:
    // GL frame terminators controls:
    QCheckBox* m_pGlClearCheckbox;
    QCheckBox* m_pGlFlushCheckbox;
    QCheckBox* m_pGlFinishCheckbox;
    QCheckBox* m_pSwapBuffersCheckbox;
    QCheckBox* m_pMakeCurrentCheckbox;
    QCheckBox* m_pSwapLayerBuffersCheckbox;
    QCheckBox* m_pGlFrameTerminatorGREMEDYCheckbox;

    // CL frame terminators controls:
    QCheckBox* m_pCl_gremedy_computation_frameCheckbox;
    QCheckBox* m_pClFlushCheckbox;
    QCheckBox* m_pClFinishCheckbox;
    QCheckBox* m_pClWaitForEventsCheckbox;

    // HSA debugging controls:
    QCheckBox* m_pShouldDebugHSAKernelsCheckbox;
};


#endif //__GDPROJECTSETTINGSEXTENSION_H

