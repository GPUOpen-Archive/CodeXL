//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdGLDebugOutputSettingsDialog.h
///
//==================================================================================

//------------------------------ gdGLDebugOutputSettingsDialog.h ------------------------------

#ifndef __GDGLDEBUGOUTPUTSETTINGSDIALOG
#define __GDGLDEBUGOUTPUTSETTINGSDIALOG

#include <QtWidgets>

// Infra:
#include <AMDTAPIClasses/Include/apGLDebugOutput.h>
#include <AMDTApplicationComponents/Include/acDialog.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:           gdGLDebugOutputSettingsDialog : public acDialog
// General Description:  Is handling the setting of OpenGL debug output mechanism
// Author:               Sigal Algranaty
// Creation Date:        7/6/2010
// ----------------------------------------------------------------------------------
class GD_API gdGLDebugOutputSettingsDialog : public acDialog
{
    Q_OBJECT

public:
    gdGLDebugOutputSettingsDialog(QWidget* pParent);

public:

private slots:
    void onGLDebugOutputLoggingEnableClick(int state);
    void onSourceHeaderClick(int state);
    void onTypeHeaderClick(int state);
    void onKindCheckboxClick(int state);
    void onAccept();

private:

    void setFrameLayout();
    bool setDialogInitialValues();

    void setGLDebugOutputDialogItemsStatus();

    void updateSourceHeaderCheckbox(int s);
    void updateTypeHeaderCheckbox(int t);

    QCheckBox* m_pBreakOnGLDebugOutputReportsCheckbox;
    QCheckBox* m_pGLDebugOutputLoggingEnableCheckbox;

    // Severity combo box:
    QLabel* m_pSeverityText;
    QCheckBox* m_pSeverityCheckBoxes[AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES];

    // Categories check boxes:
    QGroupBox* m_pGLCategoriesText;
    QLabel* m_pSourceHeaderLabels[AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES];
    QCheckBox* m_pSourceHeaderCheckBoxes[AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES];
    QLabel* m_pTypeHeaderLabels[AP_NUMBER_OF_USER_VISIBLE_DEBUG_OUTPUT_TYPES];
    QCheckBox* m_pTypeHeaderCheckBoxes[AP_NUMBER_OF_USER_VISIBLE_DEBUG_OUTPUT_TYPES];
    QCheckBox* m_pKindCheckBoxes[AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES * AP_NUMBER_OF_USER_VISIBLE_DEBUG_OUTPUT_TYPES];
    bool m_isUpdatingCheckboxes;
};


#endif  // __GDNVGLDEBUGOUTPUTSETTINGSDIALOG
