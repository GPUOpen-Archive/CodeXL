//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdGlobalDebugSettingsPage.cpp
///
//==================================================================================

//------------------------------ gdGlobalDebugSettingsPage.cpp ------------------------------

#include <memory>
// Qt:
#include <QtWidgets>

// TinyXml:
#include <tinyxml.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/apBasicParameters.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationComponents/Include/acDefinitions.h>
#include <AMDTApplicationComponents/Include/acDisplay.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/src/afUtils.h>


// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdGlobalDebugSettingsPage.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>


// Constants for the page:

// Minimums and Maximums for the spin controls:
#define GD_OPTIONS_OPENGL_CALLS_MIN_AMOUNT 1
#define GD_OPTIONS_OPENGL_CALLS_MAX_AMOUNT 100000000
#define GD_OPTIONS_OPENGL_CALLS_INIT_AMOUNT AP_DEFAULT_OPENGL_CONTEXT_CALLS_LOG_MAX_SIZE
#define GD_OPTIONS_OPENGL_CALLS_AMOUNT_STEP 10000

#define GD_OPTIONS_OPENCL_CALLS_MIN_AMOUNT 1
#define GD_OPTIONS_OPENCL_CALLS_MAX_AMOUNT 100000
#define GD_OPTIONS_OPENCL_CALLS_INIT_AMOUNT AP_DEFAULT_OPENCL_CONTEXT_CALLS_LOG_MAX_SIZE
#define GD_OPTIONS_OPENCL_CALLS_AMOUNT_STEP 1000

#define GD_OPTIONS_OPENCL_QUEUE_COMMANDS_MIN_AMOUNT 1
#define GD_OPTIONS_OPENCL_QUEUE_COMMANDS_MAX_AMOUNT 100000
#define GD_OPTIONS_OPENCL_QUEUE_COMMANDS_INIT_AMOUNT AP_DEFAULT_OPENCL_QUEUE_COMMANDS_LOG_MAX_SIZE
#define GD_OPTIONS_OPENCL_QUEUE_COMMANDS_AMOUNT_STEP 50

#define GD_OPTIONS_DEBUG_OBJECTS_TREE_ITEMS_PER_TYPE_MIN 1
#define GD_OPTIONS_DEBUG_OBJECTS_TREE_ITEMS_PER_TYPE_MAX 0x7FFFFFFF
#define GD_OPTIONS_DEBUG_OBJECTS_TREE_ITEMS_PER_TYPE_INIT GD_DEFAULT_DEBUG_OBJECTS_TREE_MAX_ITEMS_PER_TYPE
#define GD_OPTIONS_DEBUG_OBJECTS_TREE_ITEMS_PER_TYPE_STEP 10

#define GD_OPTIONS_DEBUG_SPIN_BOXES_WIDTH_HINT (AC_DEFAULT_TEXT_AVERAGE_CHAR_WIDTH * 14)


// This must match the apFileType enum:
const char* gwTextureFormatStrings[4] =
{
    GD_STR_globalSettingsTextureFileFormatBmp,
    GD_STR_globalSettingsTextureFileFormatTiff,
    GD_STR_globalSettingsTextureFileFormatJpg,
    GD_STR_globalSettingsTextureFileFormatPng,
};



// ---------------------------------------------------------------------------
// Name:        gdGlobalDebugSettingsPage::gdGlobalDebugSettingsPage
// Description: Constructor
// Author:      Uri Shomroni
// Date:        22/5/2012
// ---------------------------------------------------------------------------
gdGlobalDebugSettingsPage::gdGlobalDebugSettingsPage()
    : m_pCollectAllocatedObjectsCreationCallStacks(NULL), m_pSaveTexturesToLogFileCheckBox(NULL), m_pLogFileTextureFileFormatComboBox(NULL),
      m_pOpenGLLoggedCallsMaxSpinBox(NULL), m_pOpenCLLoggedCallsMaxSpinBox(NULL), m_pMaxTreeItemsPerTypeSpinBox(NULL), m_pFlushLogAfterEachFunctionCheckBox(NULL)
{

}

// ---------------------------------------------------------------------------
// Name:        gdGlobalDebugSettingsPage::~gdGlobalDebugSettingsPage
// Description: Destructor
// Author:      Uri Shomroni
// Date:        22/5/2012
// ---------------------------------------------------------------------------
gdGlobalDebugSettingsPage::~gdGlobalDebugSettingsPage()
{

}

// ---------------------------------------------------------------------------
// Name:        gdGlobalDebugSettingsPage::initialize
// Description: Initializes the page and all its sub items
// Return Val:  void
// Author:      Uri Shomroni
// Date:        24/4/2012
// ---------------------------------------------------------------------------
void gdGlobalDebugSettingsPage::initialize()
{
    // The main page layout:
    QVBoxLayout* pPageLayout = new QVBoxLayout;

    // Group 1: Call stack
    QGroupBox* pCallStackGroupBox = new QGroupBox(GD_STR_globalSettingsCallStackGroupTitle);
    QVBoxLayout* pCallStackGroupLayout = new QVBoxLayout;
    m_pCollectAllocatedObjectsCreationCallStacks = new QCheckBox(GD_STR_globalSettingsCollectObjectCreationStacks);
    pCallStackGroupLayout->addWidget(m_pCollectAllocatedObjectsCreationCallStacks);

    pCallStackGroupBox->setLayout(pCallStackGroupLayout);
    pPageLayout->addWidget(pCallStackGroupBox);

    // Group 2: HTML Log file:
    QGroupBox* pLogFileGroupBox = new QGroupBox(GD_STR_globalSettingsHTMLLogGroupTitle);
    QVBoxLayout* pLogFileGroupLayout = new QVBoxLayout;
    m_pSaveTexturesToLogFileCheckBox = new QCheckBox(GD_STR_globalSettingsSaveTexturesToLogFile);
    QHBoxLayout* pLogFileTextureFormatLayout = new QHBoxLayout;
    QLabel* pLogFileTextureFormatLabel = new QLabel(GD_STR_globalSettingsTextureFileFormat);
    m_pLogFileTextureFileFormatComboBox = new QComboBox;
    QStringList textureFileFormats;

    for (int i = 0; i < 4; i++)
    {
        textureFileFormats.append(gwTextureFormatStrings[i]);
    }

    m_pLogFileTextureFileFormatComboBox->addItems(textureFileFormats);
    QLabel* pLogFileTextureFileFormatNote1 = new QLabel(GD_STR_globalSettingsTextureFileFormatTiffNote);
    QLabel* pLogFileTextureFileFormatNote2 = new QLabel(GD_STR_globalSettingsTextureFileFormatAlphaNote);
    pLogFileTextureFormatLayout->addWidget(pLogFileTextureFormatLabel, 1);
    pLogFileTextureFormatLayout->addWidget(m_pLogFileTextureFileFormatComboBox);
    pLogFileGroupLayout->addWidget(m_pSaveTexturesToLogFileCheckBox);
    pLogFileGroupLayout->addLayout(pLogFileTextureFormatLayout);
    pLogFileGroupLayout->addWidget(pLogFileTextureFileFormatNote1);
    pLogFileGroupLayout->addWidget(pLogFileTextureFileFormatNote2);

    pLogFileGroupBox->setLayout(pLogFileGroupLayout);
    pPageLayout->addWidget(pLogFileGroupBox);

    // Group 3: Calls logging:
    unsigned int spinBoxWidth = acScalePixelSizeToDisplayDPI(GD_OPTIONS_DEBUG_SPIN_BOXES_WIDTH_HINT);
    QGroupBox* pCallsLoggingGroupBox = new QGroupBox(GD_STR_globalSettingsCallsLoggingGroupTitle);
    QVBoxLayout* pCallsLoggingGroupLayout = new QVBoxLayout;
    QLabel* pCallsLoggingTitle = new QLabel(GD_STR_globalSettingsAPILoggingTitle);

    QHBoxLayout* pOpenGLCallsLoggingLayout = new QHBoxLayout;
    QLabel* pOpenGLCallsLoggingLabel = new QLabel(GD_STR_globalSettingsOpenGLLogging);
    m_pOpenGLLoggedCallsMaxSpinBox = new QSpinBox;
    m_pOpenGLLoggedCallsMaxSpinBox->setRange(GD_OPTIONS_OPENGL_CALLS_MIN_AMOUNT, GD_OPTIONS_OPENGL_CALLS_MAX_AMOUNT);
    connect(m_pOpenGLLoggedCallsMaxSpinBox, SIGNAL(valueChanged(int)), this, SLOT(handleValueChanged(int)));
    m_pOpenGLLoggedCallsMaxSpinBox->setSingleStep(GD_OPTIONS_OPENGL_CALLS_AMOUNT_STEP);
    m_pOpenGLLoggedCallsMaxSpinBox->setMinimumWidth(spinBoxWidth);
    m_pOpenGLLoggedCallsMaxSpinBox->setMaximumWidth(spinBoxWidth);
    m_pOpenGLLoggedCallsMaxSpinBox->setAlignment(Qt::AlignRight);

    QHBoxLayout* pOpenCLCallsLoggingLayout = new QHBoxLayout;
    QLabel* pOpenCLCallsLoggingLabel = new QLabel(GD_STR_globalSettingsOpenCLLogging);
    m_pOpenCLLoggedCallsMaxSpinBox = new QSpinBox;
    m_pOpenCLLoggedCallsMaxSpinBox->setRange(GD_OPTIONS_OPENCL_CALLS_MIN_AMOUNT, GD_OPTIONS_OPENCL_CALLS_MAX_AMOUNT);
    connect(m_pOpenCLLoggedCallsMaxSpinBox, SIGNAL(valueChanged(int)), this, SLOT(handleValueChanged(int)));
    m_pOpenCLLoggedCallsMaxSpinBox->setSingleStep(GD_OPTIONS_OPENCL_CALLS_AMOUNT_STEP);
    m_pOpenCLLoggedCallsMaxSpinBox->setMinimumWidth(spinBoxWidth);
    m_pOpenCLLoggedCallsMaxSpinBox->setMaximumWidth(spinBoxWidth);
    m_pOpenCLLoggedCallsMaxSpinBox->setAlignment(Qt::AlignRight);

    QLabel* pCallsLoggingExceedWarning = new QLabel(GD_STR_globalSettingsOpenCLLoggingWillBeClearedWarning);
    QLabel* pCallsLoggingBeginEndBlockWarning = new QLabel(GD_STR_globalSettingsOpenGLBeginEndBlockWarning);

    pOpenGLCallsLoggingLayout->addWidget(pOpenGLCallsLoggingLabel, 1);
    pOpenGLCallsLoggingLayout->addWidget(m_pOpenGLLoggedCallsMaxSpinBox);
    pOpenCLCallsLoggingLayout->addWidget(pOpenCLCallsLoggingLabel, 1);
    pOpenCLCallsLoggingLayout->addWidget(m_pOpenCLLoggedCallsMaxSpinBox);
    pCallsLoggingGroupLayout->addWidget(pCallsLoggingTitle);
    pCallsLoggingGroupLayout->addLayout(pOpenGLCallsLoggingLayout);
    pCallsLoggingGroupLayout->addLayout(pOpenCLCallsLoggingLayout);
    pCallsLoggingGroupLayout->addWidget(pCallsLoggingExceedWarning);
    pCallsLoggingGroupLayout->addWidget(pCallsLoggingBeginEndBlockWarning);

    pCallsLoggingGroupBox->setLayout(pCallsLoggingGroupLayout);
    pPageLayout->addWidget(pCallsLoggingGroupBox);

    // Group 4: Advanced:
    QGroupBox* pAdvancedGroupBox = new QGroupBox(AF_STR_globalSettingsAdvancedGroupTitle);
    QVBoxLayout* pAdvancedGroupLayout = new QVBoxLayout;
    QHBoxLayout* pMaxTreeItemsLayout = new QHBoxLayout;
    QLabel* pMaxTreeItemsLabel = new QLabel(GD_STR_globalSettingsMaxTreeItems);
    m_pMaxTreeItemsPerTypeSpinBox = new QSpinBox;
    m_pMaxTreeItemsPerTypeSpinBox->setRange(GD_OPTIONS_DEBUG_OBJECTS_TREE_ITEMS_PER_TYPE_MIN, GD_OPTIONS_DEBUG_OBJECTS_TREE_ITEMS_PER_TYPE_MAX);
    connect(m_pMaxTreeItemsPerTypeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(handleValueChanged(int)));
    m_pMaxTreeItemsPerTypeSpinBox->setSingleStep(GD_OPTIONS_DEBUG_OBJECTS_TREE_ITEMS_PER_TYPE_STEP);
    m_pMaxTreeItemsPerTypeSpinBox->setMinimumWidth(spinBoxWidth);
    m_pMaxTreeItemsPerTypeSpinBox->setMaximumWidth(spinBoxWidth);
    m_pMaxTreeItemsPerTypeSpinBox->setAlignment(Qt::AlignRight);

    m_pFlushLogAfterEachFunctionCheckBox = new QCheckBox(GD_STR_globalSettingsFlushLogFile);
    pMaxTreeItemsLayout->addWidget(pMaxTreeItemsLabel, 1);
    pMaxTreeItemsLayout->addWidget(m_pMaxTreeItemsPerTypeSpinBox);
    pAdvancedGroupLayout->addLayout(pMaxTreeItemsLayout);
    pAdvancedGroupLayout->addWidget(m_pFlushLogAfterEachFunctionCheckBox);
    pAdvancedGroupBox->setLayout(pAdvancedGroupLayout);
    pPageLayout->addWidget(pAdvancedGroupBox);

    // Set the layout:
    pPageLayout->addStretch(1);
    setLayout(pPageLayout);
}

// ---------------------------------------------------------------------------
// Name:        gdGlobalDebugSettingsPage::pageTitle
// Description: Returns the title for the page's tab in the global settings dialog
// Author:      Uri Shomroni
// Date:        24/4/2012
// ---------------------------------------------------------------------------
gtString gdGlobalDebugSettingsPage::pageTitle()
{
    return GD_STR_globalSettingsDebugPageTitle;
}

// ---------------------------------------------------------------------------
// Name:        gdGlobalDebugSettingsPage::xmlSectionTitle
// Description: Returns the section title for this page in the global settings file
// Author:      Uri Shomroni
// Date:        24/4/2012
// ---------------------------------------------------------------------------
gtString gdGlobalDebugSettingsPage::xmlSectionTitle()
{
    return GD_STR_globalSettingsDebugXMLSectionPageTitle;
}

// ---------------------------------------------------------------------------
// Name:        gdGlobalDebugSettingsPage::getXMLSettingsString
// Description: Gets the XML representing the Debug settings
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        22/5/2012
// ---------------------------------------------------------------------------
bool gdGlobalDebugSettingsPage::getXMLSettingsString(gtString& projectAsXMLString)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT((NULL != m_pCollectAllocatedObjectsCreationCallStacks) &&
                      (NULL != m_pSaveTexturesToLogFileCheckBox) &&
                      (NULL != m_pLogFileTextureFileFormatComboBox) &&
                      (NULL != m_pOpenGLLoggedCallsMaxSpinBox) &&
                      (NULL != m_pOpenCLLoggedCallsMaxSpinBox) &&
                      (NULL != m_pMaxTreeItemsPerTypeSpinBox) &&
                      (NULL != m_pFlushLogAfterEachFunctionCheckBox))
    {
        retVal = true;

        bool collectObjectCreationStacks = m_pCollectAllocatedObjectsCreationCallStacks->isChecked();

        bool saveTexturesToLogFile = m_pSaveTexturesToLogFileCheckBox->isChecked();
        int textureFormatIndex = m_pLogFileTextureFileFormatComboBox->currentIndex();

        int maxLoggedGLCalls = m_pOpenGLLoggedCallsMaxSpinBox->value();
        int maxLoggedCLCalls = m_pOpenCLLoggedCallsMaxSpinBox->value();
        int maxTreeItems = m_pMaxTreeItemsPerTypeSpinBox->value();

        bool flushLogFile = m_pFlushLogAfterEachFunctionCheckBox->isChecked();

        projectAsXMLString.appendFormattedString(L"<%ls>", xmlSectionTitle().asCharArray());
        afUtils::addFieldToXML(projectAsXMLString, GD_STR_ToolsOptionsCollectAllocatedObjectsCreationCallsStacksNode, collectObjectCreationStacks);
        afUtils::addFieldToXML(projectAsXMLString, GD_STR_ToolsOptionsEnableTexturesLoggingNode, saveTexturesToLogFile);
        afUtils::addFieldToXML(projectAsXMLString, GD_STR_ToolsOptionsTexturesFileFormatNode, textureFormatIndex);
        afUtils::addFieldToXML(projectAsXMLString, GD_STR_ToolsOptionsMaxOpenGLCallsPerContextNode, maxLoggedGLCalls);
        afUtils::addFieldToXML(projectAsXMLString, GD_STR_ToolsOptionsMaxOpenCLCallsPerContextNode, maxLoggedCLCalls);
        afUtils::addFieldToXML(projectAsXMLString, GD_STR_ToolsOptionsMaxDebugTreeItemsPerTypeNode, maxTreeItems);
        afUtils::addFieldToXML(projectAsXMLString, GD_STR_ToolsOptionsFlushLogFileAfterEveryFunctionNode, flushLogFile);
        projectAsXMLString.appendFormattedString(L"</%ls>", xmlSectionTitle().asCharArray());
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdGlobalDebugSettingsPage::setSettingsFromXMLString
// Description: Reads the settings from an XML string
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        23/5/2012
// ---------------------------------------------------------------------------
bool gdGlobalDebugSettingsPage::setSettingsFromXMLString(const gtString& projectAsXMLString)
{
    bool retVal = false;

    gtString additionalSourceDirPaths;
    gtString sourceCodeRootPath;
    bool collectObjectCreationStacks = true;
    bool saveTexturesToLogFile = true;
    int textureFormatIndex = (int)AP_PNG_FILE;
    int maxLoggedGLCalls = GD_OPTIONS_OPENGL_CALLS_INIT_AMOUNT;
    int maxLoggedCLCalls = GD_OPTIONS_OPENCL_CALLS_INIT_AMOUNT;
    int maxTreeItems = GD_OPTIONS_DEBUG_OBJECTS_TREE_ITEMS_PER_TYPE_INIT;
    bool flushLogFile = false;

    std::unique_ptr<TiXmlNode> pDebugNode(new TiXmlElement(xmlSectionTitle().asASCIICharArray()));

    // TinyXML does not support wide strings but it does supports UTF8 so we convert the strings to UTF8
    std::string utf8XMLString;
    projectAsXMLString.asUtf8(utf8XMLString);
    pDebugNode->Parse(utf8XMLString.c_str(), 0, TIXML_ENCODING_UTF8);
    gtString debugNodeTitle;
    debugNodeTitle.fromASCIIString(pDebugNode->Value());

    if (xmlSectionTitle() == debugNodeTitle)
    {
        retVal = true;
        afUtils::getFieldFromXML(*pDebugNode, GD_STR_ToolsOptionsCollectAllocatedObjectsCreationCallsStacksNode, collectObjectCreationStacks);
        afUtils::getFieldFromXML(*pDebugNode, GD_STR_ToolsOptionsEnableTexturesLoggingNode, saveTexturesToLogFile);
        afUtils::getFieldFromXML(*pDebugNode, GD_STR_ToolsOptionsTexturesFileFormatNode, textureFormatIndex);
        afUtils::getFieldFromXML(*pDebugNode, GD_STR_ToolsOptionsMaxOpenGLCallsPerContextNode, maxLoggedGLCalls);
        afUtils::getFieldFromXML(*pDebugNode, GD_STR_ToolsOptionsMaxOpenCLCallsPerContextNode, maxLoggedCLCalls);
        afUtils::getFieldFromXML(*pDebugNode, GD_STR_ToolsOptionsMaxDebugTreeItemsPerTypeNode, maxTreeItems);
        afUtils::getFieldFromXML(*pDebugNode, GD_STR_ToolsOptionsFlushLogFileAfterEveryFunctionNode, flushLogFile);
    }

    m_pCollectAllocatedObjectsCreationCallStacks->setChecked(collectObjectCreationStacks);

    m_pSaveTexturesToLogFileCheckBox->setChecked(saveTexturesToLogFile);
    m_pLogFileTextureFileFormatComboBox->setCurrentIndex(textureFormatIndex);

    m_pOpenGLLoggedCallsMaxSpinBox->setValue(maxLoggedGLCalls);
    m_pOpenCLLoggedCallsMaxSpinBox->setValue(maxLoggedCLCalls);
    m_pMaxTreeItemsPerTypeSpinBox->setValue(maxTreeItems);

    m_pFlushLogAfterEachFunctionCheckBox->setChecked(flushLogFile);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdGlobalDebugSettingsPage::loadCurrentSettings
// Description: Loads the current values into the settings page
// Author:      Uri Shomroni
// Date:        24/4/2012
// ---------------------------------------------------------------------------
void gdGlobalDebugSettingsPage::loadCurrentSettings()
{
    GT_IF_WITH_ASSERT((NULL != m_pCollectAllocatedObjectsCreationCallStacks) &&
                      (NULL != m_pSaveTexturesToLogFileCheckBox) &&
                      (NULL != m_pLogFileTextureFileFormatComboBox) &&
                      (NULL != m_pOpenGLLoggedCallsMaxSpinBox) &&
                      (NULL != m_pOpenCLLoggedCallsMaxSpinBox) &&
                      (NULL != m_pMaxTreeItemsPerTypeSpinBox) &&
                      (NULL != m_pFlushLogAfterEachFunctionCheckBox))
    {
        gdGDebuggerGlobalVariablesManager& theGDGlobalVariablesManager = gdGDebuggerGlobalVariablesManager::instance();
        bool isCollectingStacks = true;
        bool rcStk = gaGetCollectAllocatedObjectCreationCallsStacks(isCollectingStacks);
        GT_ASSERT(rcStk);
        m_pCollectAllocatedObjectsCreationCallStacks->setChecked(isCollectingStacks);

        bool areImagesLogged = true;
        bool rcImg = gaIsImagesDataLogged(areImagesLogged);
        GT_ASSERT(rcImg);
        m_pSaveTexturesToLogFileCheckBox->setChecked(areImagesLogged);
        m_pLogFileTextureFileFormatComboBox->setCurrentIndex((int)theGDGlobalVariablesManager.imagesFileFormat());

        unsigned int loggedGLCalls = GD_OPTIONS_OPENGL_CALLS_INIT_AMOUNT;
        unsigned int loggedCLCalls = GD_OPTIONS_OPENCL_CALLS_INIT_AMOUNT;
        unsigned int loggedQueueCommands = GD_OPTIONS_OPENCL_QUEUE_COMMANDS_INIT_AMOUNT;
        theGDGlobalVariablesManager.getLoggingLimits(loggedGLCalls, loggedCLCalls, loggedQueueCommands);
        m_pOpenGLLoggedCallsMaxSpinBox->setValue((int)loggedGLCalls);
        m_pOpenCLLoggedCallsMaxSpinBox->setValue((int)loggedCLCalls);

        unsigned int shownTreeItems = GD_OPTIONS_DEBUG_OBJECTS_TREE_ITEMS_PER_TYPE_INIT;
        shownTreeItems = theGDGlobalVariablesManager.maxTreeItemsPerType();
        m_pMaxTreeItemsPerTypeSpinBox->setValue((int)shownTreeItems);

        bool isLogFlushed = false;
        bool rcLog = gaIsLogFileFlushedAfterEachFunctionCall(isLogFlushed);
        GT_ASSERT(rcLog);
        m_pFlushLogAfterEachFunctionCheckBox->setChecked(isLogFlushed);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdGlobalDebugSettingsPage::restoreDefaultSettings
// Description: Restores the default values into all the views.
// Author:      Uri Shomroni
// Date:        24/4/2012
// ---------------------------------------------------------------------------
void gdGlobalDebugSettingsPage::restoreDefaultSettings()
{
    GT_IF_WITH_ASSERT((NULL != m_pCollectAllocatedObjectsCreationCallStacks) &&
                      (NULL != m_pSaveTexturesToLogFileCheckBox) &&
                      (NULL != m_pLogFileTextureFileFormatComboBox) &&
                      (NULL != m_pOpenGLLoggedCallsMaxSpinBox) &&
                      (NULL != m_pOpenCLLoggedCallsMaxSpinBox) &&
                      (NULL != m_pMaxTreeItemsPerTypeSpinBox) &&
                      (NULL != m_pFlushLogAfterEachFunctionCheckBox))
    {
        m_pCollectAllocatedObjectsCreationCallStacks->setChecked(true);

        m_pSaveTexturesToLogFileCheckBox->setChecked(true);
        m_pLogFileTextureFileFormatComboBox->setCurrentIndex((int)AP_PNG_FILE);

        m_pOpenGLLoggedCallsMaxSpinBox->setValue(GD_OPTIONS_OPENGL_CALLS_INIT_AMOUNT);
        m_pOpenCLLoggedCallsMaxSpinBox->setValue(GD_OPTIONS_OPENCL_CALLS_INIT_AMOUNT);
        m_pMaxTreeItemsPerTypeSpinBox->setValue(GD_OPTIONS_DEBUG_OBJECTS_TREE_ITEMS_PER_TYPE_INIT);

        m_pFlushLogAfterEachFunctionCheckBox->setChecked(false);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdGlobalDebugSettingsPage::saveCurrentSettings
// Description: Applies the current settings to the data structures
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        24/4/2012
// ---------------------------------------------------------------------------
bool gdGlobalDebugSettingsPage::saveCurrentSettings()
{
    bool retVal = false;
    GT_IF_WITH_ASSERT((NULL != m_pCollectAllocatedObjectsCreationCallStacks) &&
                      (NULL != m_pSaveTexturesToLogFileCheckBox) &&
                      (NULL != m_pLogFileTextureFileFormatComboBox) &&
                      (NULL != m_pOpenGLLoggedCallsMaxSpinBox) &&
                      (NULL != m_pOpenCLLoggedCallsMaxSpinBox) &&
                      (NULL != m_pMaxTreeItemsPerTypeSpinBox) &&
                      (NULL != m_pFlushLogAfterEachFunctionCheckBox))
    {
        gdGDebuggerGlobalVariablesManager& theGDGlobalVariablesManager = gdGDebuggerGlobalVariablesManager::instance();

        bool rcStk = gaCollectAllocatedObjectsCreationCallsStacks(m_pCollectAllocatedObjectsCreationCallStacks->isChecked());
        GT_ASSERT(rcStk);

        gaEnableImagesDataLogging(m_pSaveTexturesToLogFileCheckBox->isChecked());
        theGDGlobalVariablesManager.setImagesFileFormat((apFileType)m_pLogFileTextureFileFormatComboBox->currentIndex());

        theGDGlobalVariablesManager.setLoggingLimits(m_pOpenGLLoggedCallsMaxSpinBox->value(), m_pOpenCLLoggedCallsMaxSpinBox->value(), GD_OPTIONS_OPENCL_QUEUE_COMMANDS_INIT_AMOUNT);

        theGDGlobalVariablesManager.setMaxTreeItemsPerType(m_pMaxTreeItemsPerTypeSpinBox->value());

        gaFlushLogFileAfterEachFunctionCall(m_pFlushLogAfterEachFunctionCheckBox->isChecked());

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdGlobalDebugSettingsPage::handleValueChanged
// Description: Handles a change to one of the spin boxes' values.
// Author:      Koushik Bhattacharyya
// Date:        9/8/2012
// ---------------------------------------------------------------------------
void gdGlobalDebugSettingsPage::handleValueChanged(int value)
{
    int minValue = 0;
    int maxValue = INT_MAX;

    if (sender() == m_pOpenCLLoggedCallsMaxSpinBox)
    {
        minValue = GD_OPTIONS_OPENCL_CALLS_MIN_AMOUNT;
        maxValue = GD_OPTIONS_OPENCL_CALLS_MAX_AMOUNT;
    }
    else if (sender() == m_pOpenGLLoggedCallsMaxSpinBox)
    {
        minValue = GD_OPTIONS_OPENGL_CALLS_MIN_AMOUNT;
        maxValue = GD_OPTIONS_OPENGL_CALLS_MAX_AMOUNT;
    }
    else if (sender() == m_pMaxTreeItemsPerTypeSpinBox)
    {
        minValue = GD_OPTIONS_DEBUG_OBJECTS_TREE_ITEMS_PER_TYPE_MIN;
        maxValue = GD_OPTIONS_DEBUG_OBJECTS_TREE_ITEMS_PER_TYPE_MAX;
    }
    else
    {
        GT_ASSERT_EX(false, L"minValue / maxValue are undefined.")
    }

    QSpinBox* pSpinBox = qobject_cast<QSpinBox*>(sender());

    if (NULL != pSpinBox)
    {
        QString strMessage;
        int newValue = value;

        if (minValue > value)
        {
            strMessage = QString(AF_STR_globalSettingsMinBoundErrorMsg).arg(minValue);
            newValue = minValue;
        }
        else if (maxValue < value)
        {
            strMessage = QString(AF_STR_globalSettingsMaxBoundErrorMsg).arg(minValue);
            newValue = maxValue;
        }

        if (newValue != value)
        {
            acMessageBox::instance().warning(afGlobalVariablesManager::ProductNameA(), strMessage, QMessageBox::Ok);
            pSpinBox->setValue(newValue);
        }
    }
}

