//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdProjectSettingsExtension.cpp
///
//==================================================================================

//------------------------------ gdProjectSettingsExtension.h ------------------------------

// QT:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSAPIWrappers/Include/oaDriver.h>
#include <AMDTAPIClasses/Include/apFrameTerminators.h>
#include <AMDTAPIClasses/Include/Events/apBreakpointsUpdatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afCSSSettings.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>

// AMDTGpuDebuggingComponents:
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdProjectSettingsExtension.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/commands/gdLoadProjectCommand.h>
#include <AMDTGpuDebuggingComponents/Include/commands/gdSaveProjectCommand.h>

// ---------------------------------------------------------------------------
// Name:        gdProjectSettingsExtension::gdProjectSettingsExtension
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        4/4/2012
// ---------------------------------------------------------------------------
gdProjectSettingsExtension::gdProjectSettingsExtension():
    afProjectSettingsExtension(),
    m_pGlClearCheckbox(NULL), m_pGlFlushCheckbox(NULL), m_pGlFinishCheckbox(NULL), m_pSwapBuffersCheckbox(NULL),
    m_pMakeCurrentCheckbox(NULL), m_pSwapLayerBuffersCheckbox(NULL), m_pGlFrameTerminatorGREMEDYCheckbox(NULL),
    m_pCl_gremedy_computation_frameCheckbox(NULL), m_pClFlushCheckbox(NULL), m_pClFinishCheckbox(NULL),
    m_pClWaitForEventsCheckbox(NULL), m_pShouldDebugHSAKernelsCheckbox(nullptr)
{
}

// ---------------------------------------------------------------------------
// Name:        gdProjectSettingsExtension::~gdProjectSettingsExtension
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        4/4/2012
// ---------------------------------------------------------------------------
gdProjectSettingsExtension::~gdProjectSettingsExtension()
{
}

// ---------------------------------------------------------------------------
// Name:        gdProjectSettingsExtension::initialize
// Description: Create the widget that is reading the debug setting for the debugger
// Author:      Sigal Algranaty
// Date:        4/4/2012
// ---------------------------------------------------------------------------
void gdProjectSettingsExtension::Initialize()
{
    // Create the OpenGL frame terminators group box:
    QLabel* pCaption1 = new QLabel(GD_STR_projectSettingsGLFrameTerminators);
    pCaption1->setStyleSheet(AF_STR_captionLabelStyleSheetMain);

    QVBoxLayout* pGLFrameTerminatorsLayout = new QVBoxLayout;
    createGLControls(pGLFrameTerminatorsLayout);

    QLabel* pCaption2 = new QLabel(GD_STR_projectSettingsCLFrameTerminators);
    pCaption2->setStyleSheet(AF_STR_captionLabelStyleSheet);

    QVBoxLayout* pCLFrameTerminatorsLayout = new QVBoxLayout;
    createCLControls(pCLFrameTerminatorsLayout);

#ifdef GD_ALLOW_HSA_DEBUGGING
    QLabel* pCaption3 = new QLabel(GD_STR_projectSettingsHSASettings);
    pCaption3->setStyleSheet(AF_STR_captionLabelStyleSheet);
    m_pShouldDebugHSAKernelsCheckbox = new QCheckBox(GD_STR_DebugSettingsDebugHSAKernels);
    bool hsaAllowed = (0 != (afGlobalVariablesManager::instance().InstalledAMDComponentsBitmask() & AF_AMD_HSA_COMPONENT));
    m_pShouldDebugHSAKernelsCheckbox->setEnabled(hsaAllowed);
#elif defined (GD_DISALLOW_HSA_DEBUGGING)
#else
#error GD_ALLOW_HSA_DEBUGGING and GD_DISALLOW_HSA_DEBUGGING both not defined. Please include gdApplicationCommands.h!
#endif // GD_ALLOW_HSA_DEBUGGING

    // Create the breakpoints button:
    QPushButton* pBreakpointsButton = new QPushButton(GD_STR_projectSettingsAddBreakpoints);
    bool rc = connect(pBreakpointsButton, SIGNAL(clicked()), this, SLOT(onAddRemoveBreakpoints()));
    GT_ASSERT(rc);

    // The main layout - contain box group boxes:
    QVBoxLayout* pMainLayout = new QVBoxLayout;
    pMainLayout->addWidget(pCaption1);
    pMainLayout->addLayout(pGLFrameTerminatorsLayout);
    pMainLayout->addWidget(pCaption2);
    pMainLayout->addLayout(pCLFrameTerminatorsLayout);
#ifdef GD_ALLOW_HSA_DEBUGGING
    pMainLayout->addWidget(pCaption3);
    pMainLayout->addSpacing(5);
    pMainLayout->addWidget(m_pShouldDebugHSAKernelsCheckbox, 0, Qt::AlignLeft);
#endif // GD_ALLOW_HSA_DEBUGGING
    pMainLayout->addStretch();
    pMainLayout->addWidget(pBreakpointsButton, 0, Qt::AlignRight);

    setLayout(pMainLayout);
}

// ---------------------------------------------------------------------------
// Name:        gdProjectSettingsExtension::ExtensionXMLString
// Description: Return the extension string
// Return Val:  gtString&
// Author:      Sigal Algranaty
// Date:        4/4/2012
// ---------------------------------------------------------------------------
gtString gdProjectSettingsExtension::ExtensionXMLString()
{
    gtString retVal = GD_STR_projectSettingsExtensionName;
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdProjectSettingsExtension::extensionDisplayName
// Description: Return the display name for the extension
// Return Val:  gtString
// Author:      Sigal Algranaty
// Date:        16/5/2012
// ---------------------------------------------------------------------------
gtString gdProjectSettingsExtension::ExtensionTreePathAsString()
{
    gtString retVal = GD_STR_projectSettingsExtensionDisplayName;
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdProjectSettingsExtension::getXMLSettingsString
// Description: Get the current debugger project settings as XML string
// Arguments:   gtString& projectAsXMLString
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        5/4/2012
// ---------------------------------------------------------------------------
bool gdProjectSettingsExtension::GetXMLSettingsString(gtString& projectAsXMLString)
{
    bool retVal = false;

    // Create a save project command:
    gdSaveProjectCommand saveCommand;

    retVal = saveCommand.execute();

    if (retVal)
    {
        // Get the XML as string:
        retVal = saveCommand.getXMLOutputString(projectAsXMLString);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdProjectSettingsExtension::setSettingsFromXMLString
// Description: Get the project settings from the XML string
// Arguments:   const gtString& projectAsXMLString
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        8/4/2012
// ---------------------------------------------------------------------------
bool gdProjectSettingsExtension::SetSettingsFromXMLString(const gtString& projectAsXMLString)
{
    bool retVal = false;
    // Load the project settings from the XML string:
    gdLoadProjectCommand loadCommand(projectAsXMLString);
    retVal = loadCommand.execute();

    // Get the loaded settings:
    const apDebugProjectSettings& loadedDebugSettings = loadCommand.loadedProjectSettings();

    // Copy to a new object:
    apProjectSettings projectSettings = afProjectManager::instance().currentProjectSettings();

    // Create a settings with both configurations:
    apDebugProjectSettings debugSettings(projectSettings, loadedDebugSettings);

    // Set the current project settings:
    gdGDebuggerGlobalVariablesManager::instance().setCurrentDebugProjectSettings(debugSettings);

    // Trigger breakpoints update event:
    // The -1 states the all the breakpoints are updated, and lists should be updated from scratch:
    apBreakpointsUpdatedEvent eve(-1);
    apEventsHandler::instance().registerPendingDebugEvent(eve);

    // Load settings to the controls:
    retVal = RestoreCurrentSettings() && retVal;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdProjectSettingsExtension::SaveCurrentSettings
// Description: Get the current project settings from the controls, and store into
//              the current project properties
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        4/4/2012
// ---------------------------------------------------------------------------
bool gdProjectSettingsExtension::SaveCurrentSettings()
{
    bool retVal = false;

    // Save the frame terminators:
    retVal = saveProjectSettingsFromControls();

    // Get the loaded settings:
    const apDebugProjectSettings& loadedDebugSettings = gdGDebuggerGlobalVariablesManager::instance().currentDebugProjectSettings();

    // Copy to a new object:
    const apProjectSettings& projectSettings = afProjectManager::instance().currentProjectSettings();

    // Create a settings with both configurations:
    apDebugProjectSettings debugSettings(projectSettings, loadedDebugSettings);

    // Set the current project settings:
    gdGDebuggerGlobalVariablesManager::instance().setCurrentDebugProjectSettings(debugSettings);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdProjectSettingsExtension::createGLControls
// Description: Create the GL frame terminators controls
// Arguments:   QVBoxLayout* pGLFrameTerminatorsLayout
// Author:      Sigal Algranaty
// Date:        4/4/2012
// ---------------------------------------------------------------------------
void gdProjectSettingsExtension::createGLControls(QVBoxLayout* pGLFrameTerminatorsLayout)
{
    GT_IF_WITH_ASSERT(pGLFrameTerminatorsLayout != NULL)
    {
        m_pGlClearCheckbox = new QCheckBox(GD_STR_DebugSettingsGlClear);
        m_pGlFlushCheckbox = new QCheckBox(GD_STR_DebugSettingsGlFlush);
        m_pGlFinishCheckbox = new QCheckBox(GD_STR_DebugSettingsGlFinish);
        m_pGlFrameTerminatorGREMEDYCheckbox = new QCheckBox(GD_STR_DebugSettingsGlFrameTerminatorGREMEDY);
        m_pMakeCurrentCheckbox = new QCheckBox(GD_STR_DebugSettingsMakeCurrent);
        m_pSwapLayerBuffersCheckbox = new QCheckBox(GD_STR_DebugSettingsSwapLayerBuffers);
        m_pSwapBuffersCheckbox = new QCheckBox(GD_STR_DebugSettingsSwapBuffers);

        // Add the check boxes to the vertical layout:
        pGLFrameTerminatorsLayout->addWidget(m_pGlClearCheckbox);
        pGLFrameTerminatorsLayout->addWidget(m_pGlFlushCheckbox);
        pGLFrameTerminatorsLayout->addWidget(m_pGlFinishCheckbox);
        pGLFrameTerminatorsLayout->addWidget(m_pSwapBuffersCheckbox);
        pGLFrameTerminatorsLayout->addWidget(m_pMakeCurrentCheckbox);
        pGLFrameTerminatorsLayout->addWidget(m_pSwapLayerBuffersCheckbox);
        pGLFrameTerminatorsLayout->addWidget(m_pGlFrameTerminatorGREMEDYCheckbox);

        pGLFrameTerminatorsLayout->setContentsMargins(2, 2, 2, 2);
    }
}


// ---------------------------------------------------------------------------
// Name:        gdProjectSettingsExtension::createCLControls
// Description: Create the GL frame terminators controls
// Arguments:   QGroupBox* pGLFrameTerminatorsGroupBox
// Author:      Sigal Algranaty
// Date:        4/4/2012
// ---------------------------------------------------------------------------
void gdProjectSettingsExtension::createCLControls(QVBoxLayout* pCLFrameTerminatorsLayout)
{
    // Create the OpenCL frame terminators check boxes:
    m_pCl_gremedy_computation_frameCheckbox = new QCheckBox(GD_STR_DebugSettingsCl_gremedy_computation_frameCheckbox);
    m_pClFlushCheckbox = new QCheckBox(GD_STR_DebugSettingsClFlush);
    m_pClFinishCheckbox = new QCheckBox(GD_STR_DebugSettingsClFinish);
    m_pClWaitForEventsCheckbox = new QCheckBox(GD_STR_DebugSettingsClWaitForEvents);

    // Add the check boxes to the layout:
    pCLFrameTerminatorsLayout->addWidget(m_pCl_gremedy_computation_frameCheckbox);
    pCLFrameTerminatorsLayout->addWidget(m_pClFlushCheckbox);
    pCLFrameTerminatorsLayout->addWidget(m_pClFinishCheckbox);
    pCLFrameTerminatorsLayout->addWidget(m_pClWaitForEventsCheckbox);

    pCLFrameTerminatorsLayout->setContentsMargins(2, 2, 2, 2);
}


// ---------------------------------------------------------------------------
// Name:        gdProjectSettingsExtension::saveProjectSettingsFromControls
// Description: Save the frame terminators, as set by the user on the widget
// Arguments:   unsigned int& frameTerminators
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        4/4/2012
// ---------------------------------------------------------------------------
bool gdProjectSettingsExtension::saveProjectSettingsFromControls()
{
    bool retVal = false;

    // Get the frame terminators from the widgets:
    unsigned int frameTerminators = getCurrentFrameTerminators();

    retVal = ((frameTerminators & AP_ALL_GL_FRAME_TERMINATORS) != 0);

    if (!retVal)
    {
        // Add default frame terminator for GL:
        frameTerminators |= AP_SWAP_BUFFERS_TERMINATOR;
        retVal = true;
    }

    // Get the gdGDebuggerGlobalVariablesManager instance
    // in order to set the updated parameters from the Dialog
    gdGDebuggerGlobalVariablesManager& theStateManager = gdGDebuggerGlobalVariablesManager::instance();

    // Get the process creation data:
    apDebugProjectSettings projectSettings = theStateManager.currentDebugProjectSettings();

    projectSettings.setFrameTerminators(frameTerminators);

#ifdef GD_ALLOW_HSA_DEBUGGING
    GT_IF_WITH_ASSERT(nullptr != m_pShouldDebugHSAKernelsCheckbox)
    {
        bool enableHSA = (m_pShouldDebugHSAKernelsCheckbox->isChecked() && m_pShouldDebugHSAKernelsCheckbox->isEnabled());
        projectSettings.setShouldDebugHSAKernels(enableHSA);
    }
#endif // GD_ALLOW_HSA_DEBUGGING

    // Set the process creation data:
    theStateManager.setCurrentDebugProjectSettings(projectSettings);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdProjectSettingsExtension::getCurrentFrameTerminators
// Description: Save the frame terminators, as set by the user on the widget
// Arguments:   unsigned int& frameTerminators
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        4/4/2012
// ---------------------------------------------------------------------------
unsigned int gdProjectSettingsExtension::getCurrentFrameTerminators()
{
    unsigned int retVal = 0;

    // Sanity check:
    GT_IF_WITH_ASSERT((m_pGlClearCheckbox != NULL) && (m_pGlFlushCheckbox != NULL) &&
                      (m_pGlFinishCheckbox != NULL) && (m_pSwapBuffersCheckbox != NULL) &&
                      (m_pMakeCurrentCheckbox != NULL) && (m_pSwapLayerBuffersCheckbox != NULL) &&
                      (m_pGlFrameTerminatorGREMEDYCheckbox != NULL) && (m_pCl_gremedy_computation_frameCheckbox != NULL) &&
                      (m_pClFinishCheckbox != NULL) && (m_pClFlushCheckbox != NULL) && (m_pClWaitForEventsCheckbox != NULL))
    {
        if (m_pGlFlushCheckbox->isChecked())
        {
            retVal |= AP_GL_FLUSH_TERMINATOR;
        }

        if (m_pSwapBuffersCheckbox->isChecked())
        {
            retVal |= AP_SWAP_BUFFERS_TERMINATOR;
        }

        if (m_pGlFinishCheckbox->isChecked())
        {
            retVal |= AP_GL_FINISH_TERMINATOR;
        }

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

        if (m_pSwapLayerBuffersCheckbox->isChecked())
        {
            retVal |= AP_SWAP_LAYER_BUFFERS_TERMINATOR;
        }

#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

        if (m_pGlClearCheckbox->isChecked())
        {
            retVal |= AP_GL_CLEAR_TERMINATOR;
        }

        if (m_pMakeCurrentCheckbox->isChecked())
        {
            retVal |= AP_MAKE_CURRENT_TERMINATOR;
        }

        if (m_pGlFrameTerminatorGREMEDYCheckbox->isChecked())
        {
            retVal |= AP_GL_FRAME_TERMINATOR_GREMEDY;
        }

        if (m_pCl_gremedy_computation_frameCheckbox->isChecked())
        {
            retVal |= AP_CL_GREMEDY_COMPUTATION_FRAME_TERMINATORS;
        }

        if (m_pClFlushCheckbox->isChecked())
        {
            retVal |= AP_CL_FLUSH_TERMINATOR;
        }

        if (m_pClFinishCheckbox->isChecked())
        {
            retVal |= AP_CL_FINISH_TERMINATOR;
        }

        if (m_pClWaitForEventsCheckbox->isChecked())
        {
            retVal |= AP_CL_WAIT_FOR_EVENTS_TERMINATOR;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdProjectSettingsExtension::RestoreDefaultProjectSettings
// Description: Restore default project settings
// Author:      Sigal Algranaty
// Date:        11/4/2012
// ---------------------------------------------------------------------------
void gdProjectSettingsExtension::RestoreDefaultProjectSettings()
{
    // Set the default frame terminators:
    unsigned int defaultFrameTerminator = AP_DEFAULT_FRAME_TERMINATORS;

    // Set the frame terminator on the controls:
    setFrameTerminator(defaultFrameTerminator);

#ifdef GD_ALLOW_HSA_DEBUGGING
    GT_IF_WITH_ASSERT(nullptr != m_pShouldDebugHSAKernelsCheckbox)
    {
        m_pShouldDebugHSAKernelsCheckbox->setCheckState(Qt::Unchecked);
    }
#endif // GD_ALLOW_HSA_DEBUGGING
}

// ---------------------------------------------------------------------------
// Name:        gdProjectSettingsExtension::setFrameTerminator
// Description: Set the frame terminator on the controls
// Arguments:   unsigned int frameTerminator
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        11/4/2012
// ---------------------------------------------------------------------------
void gdProjectSettingsExtension::setFrameTerminator(unsigned int frameTerminators)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pGlClearCheckbox != NULL) && (m_pGlFlushCheckbox != NULL) &&
                      (m_pGlFinishCheckbox != NULL) && (m_pSwapBuffersCheckbox != NULL) &&
                      (m_pMakeCurrentCheckbox != NULL) && (m_pSwapLayerBuffersCheckbox != NULL) &&
                      (m_pGlFrameTerminatorGREMEDYCheckbox != NULL) && (m_pCl_gremedy_computation_frameCheckbox != NULL) &&
                      (m_pClFinishCheckbox != NULL) && (m_pClFlushCheckbox != NULL) && (m_pClWaitForEventsCheckbox != NULL))
    {
        m_pSwapBuffersCheckbox->setChecked(frameTerminators & AP_SWAP_BUFFERS_TERMINATOR);
        m_pGlFlushCheckbox->setChecked(frameTerminators & AP_GL_FLUSH_TERMINATOR);
        m_pGlFinishCheckbox->setChecked(frameTerminators & AP_GL_FINISH_TERMINATOR);
        m_pSwapLayerBuffersCheckbox->setChecked(frameTerminators & AP_SWAP_LAYER_BUFFERS_TERMINATOR);
        m_pMakeCurrentCheckbox->setChecked(frameTerminators & AP_MAKE_CURRENT_TERMINATOR);
        m_pGlClearCheckbox->setChecked(frameTerminators & AP_GL_CLEAR_TERMINATOR);
        m_pGlFrameTerminatorGREMEDYCheckbox->setChecked(frameTerminators & AP_GL_FRAME_TERMINATOR_GREMEDY);
        m_pCl_gremedy_computation_frameCheckbox->setChecked(frameTerminators & AP_CL_GREMEDY_COMPUTATION_FRAME_TERMINATORS);
        m_pClFlushCheckbox->setChecked(frameTerminators & AP_CL_FLUSH_TERMINATOR);
        m_pClFinishCheckbox->setChecked(frameTerminators & AP_CL_FINISH_TERMINATOR);
        m_pClWaitForEventsCheckbox->setChecked(frameTerminators & AP_CL_WAIT_FOR_EVENTS_TERMINATOR);
    }
}


// ---------------------------------------------------------------------------
// Name:        gdProjectSettingsExtension::AreSettingsValid
// Description: Check if the current settings are valid
// Arguments:   gtString& invalidMessageStr
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        11/4/2012
// ---------------------------------------------------------------------------
bool gdProjectSettingsExtension::AreSettingsValid(gtString& invalidMessageStr)
{
    bool retVal = true;

    // Get the current set frame terminators:
    unsigned int frameTerminators = getCurrentFrameTerminators();
    retVal = ((frameTerminators & AP_ALL_GL_FRAME_TERMINATORS) != 0);

    if (!retVal)
    {
        invalidMessageStr = GD_STR_DebugSettingsInvalidFrameTerminator;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdProjectSettingsExtension::onAddRemoveBreakpoints
// Description: Opens the breakpoints dialog
// Author:      Sigal Algranaty
// Date:        12/4/2012
// ---------------------------------------------------------------------------
void gdProjectSettingsExtension::onAddRemoveBreakpoints()
{
    // Get the application commands handler:
    gdApplicationCommands* pApplicationCommands = gdApplicationCommands::gdInstance();
    GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
    {
        pApplicationCommands->openBreakpointsDialog();
    }
}


// ---------------------------------------------------------------------------
// Name:        gdProjectSettingsExtension::RestoreCurrentSettings
// Description: Load the current settings to the displayed widgets
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        15/4/2012
// ---------------------------------------------------------------------------
bool gdProjectSettingsExtension::RestoreCurrentSettings()
{
    bool retVal = false;

    // Get the current debug settings:
    const apDebugProjectSettings& debugProjectSettings = gdGDebuggerGlobalVariablesManager::instance().currentDebugProjectSettings();

    // Set the frame terminators:
    setFrameTerminator(debugProjectSettings.frameTerminatorsMask());

#ifdef GD_ALLOW_HSA_DEBUGGING
    GT_IF_WITH_ASSERT(nullptr != m_pShouldDebugHSAKernelsCheckbox)
    {
        bool hsaAllowed = oaIsHSADriver();
        bool hsaEnabled = hsaAllowed && debugProjectSettings.shouldDebugHSAKernels();
        m_pShouldDebugHSAKernelsCheckbox->setCheckState(hsaEnabled ? Qt::Checked : Qt::Unchecked);
    }
#endif // GD_ALLOW_HSA_DEBUGGING

    retVal = true;

    return retVal;
}

