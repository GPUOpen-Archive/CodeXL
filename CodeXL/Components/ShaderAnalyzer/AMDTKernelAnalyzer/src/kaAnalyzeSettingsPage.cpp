//------------------------------ kaAnalyzeSettingsPage.cpp ------------------------------

// Qt:
#include <QtWidgets>

// TinyXml:
#include <tinyxml.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acListCtrl.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/src/afUtils.h>
#include <AMDTBackEnd/Include/beBackend.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTOSAPIWrappers/Include/oaDriver.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Local:
#include <AMDTKernelAnalyzer/src/kaAnalyzeSettingsPage.h>
#include <AMDTKernelAnalyzer/src/kaApplicationCommands.h>
#include <AMDTKernelAnalyzer/src/kaBackendManager.h>
#include <AMDTKernelAnalyzer/src/kaDataAnalyzerFunctions.h>
#include <AMDTKernelAnalyzer/src/kaGlobalVariableManager.h>
#include <AMDTKernelAnalyzer/src/kaProjectDataManager.h>
#include <AMDTKernelAnalyzer/src/kaTreeModel.h>
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>

#define KA_TARGET_DEVICES_TREE_VIEW_MIN_HEIGHT 300

// ---------------------------------------------------------------------------
// Name:        kaAnalyzeSettingsPage::kaAnalyzeSettingsPage
// Description: Constructor
// Author:      Gilad Yarnitzky
// Date:        30/7/2013
// ---------------------------------------------------------------------------
kaAnalyzeSettingsPage::kaAnalyzeSettingsPage() :
    m_pInformationCaption(NULL),
    m_pMainLayout(NULL), m_pTargetDevicesTreeView(NULL), m_pTargetDevicesTreeModel(NULL)
{
}

// ---------------------------------------------------------------------------
// Name:        kaAnalyzeSettingsPage::~kaAnalyzeSettingsPage
// Description: Destructor
// Author:      Gilad Yarnitzky
// Date:        30/7/2013
// ---------------------------------------------------------------------------
kaAnalyzeSettingsPage::~kaAnalyzeSettingsPage()
{

}

// ---------------------------------------------------------------------------
// Name:        kaAnalyzeSettingsPage::initialize
// Description: Initializes the page and all its sub items
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        24/4/2012
// ---------------------------------------------------------------------------
void kaAnalyzeSettingsPage::initialize()
{
    // Create the layout and the controls in it:
    m_pMainLayout = new QVBoxLayout(this);
    m_pInformationCaption = new QLabel(KA_STR_targetDeviceSettingsPageGeneralInfo);

    checkDriverVersion();

    m_pTargetDevicesTreeView = new QTreeView(NULL);
    m_pTargetDevicesTreeView->setMinimumHeight(KA_TARGET_DEVICES_TREE_VIEW_MIN_HEIGHT);

    // Add widgets to layout:
    m_pMainLayout->addWidget(m_pInformationCaption);
    m_pMainLayout->addWidget(m_pTargetDevicesTreeView, 1);

    // fill the tree with the default tree list:
    QStringList& defaultList = kaGlobalVariableManager::instance().defaultTreeList();
    replaceModel(defaultList);

    setLayout(m_pMainLayout);
}

// ---------------------------------------------------------------------------
// Name:        kaAnalyzeSettingsPage::pageTitle
// Description: Returns the title for the page's tab in the global settings dialog
// Author:      Gilad Yarnitzky
// Date:        30/7/2013
// ---------------------------------------------------------------------------
gtString kaAnalyzeSettingsPage::pageTitle()
{
    return KA_STR_analyzeSettingsPageTitle;
}

// ---------------------------------------------------------------------------
// Name:        kaAnalyzeSettingsPage::xmlSectionTitle
// Description: Returns the section title for this page in the global settings file
// Author:      Gilad Yarnitzky
// Date:        30/7/2013
// ---------------------------------------------------------------------------
gtString kaAnalyzeSettingsPage::xmlSectionTitle()
{
    return KA_STR_targetDeviceSettingsXMLSectionPageTitle;
}

// ---------------------------------------------------------------------------
// Name:        kaAnalyzeSettingsPage::getXMLSettingsString
// Description: Gets the XML representing the Debug settings
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        30/7/2013
// ---------------------------------------------------------------------------
bool kaAnalyzeSettingsPage::getXMLSettingsString(gtString& projectAsXMLString)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT((m_pTargetDevicesTreeView != NULL) && (m_pTargetDevicesTreeModel != NULL))
    {
        QStringList currentListToSave;
        m_pTargetDevicesTreeModel->getStrData(&currentListToSave);

        // join the string list to one long string to be saved:
        int numStrings = currentListToSave.count();
        QString saveString;

        for (int nString = 0 ; nString < numStrings ; nString ++)
        {
            saveString += (currentListToSave[nString] + ',');
        }

        gtString saveStringAsStr = acQStringToGTString(saveString);

        // replace \t to an escape code %9 since xml does not store tabs correctly
        saveStringAsStr.replace(L"\t", L"%9");

        // replace spaces to code %20 so in case we have multiple spaces they will be kept
        saveStringAsStr.replace(L" ", L"%20");

        projectAsXMLString.appendFormattedString(L"<%ls>", xmlSectionTitle().asCharArray());
        gtString targetDeviceNode(KA_STR_targetDeviceSettingTargetDeviceNode);
        afUtils::addFieldToXML(projectAsXMLString, targetDeviceNode, saveStringAsStr);
        projectAsXMLString.appendFormattedString(L"</%ls>", xmlSectionTitle().asCharArray());

        retVal = true;
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        kaAnalyzeSettingsPage::setSettingsFromXMLString
// Description: Reads the settings from an XML string
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        30/7/2013
// ---------------------------------------------------------------------------
bool kaAnalyzeSettingsPage::setSettingsFromXMLString(const gtString& projectAsXMLString)
{
    bool retVal = false;

    gtString targetDeviceAsStr;
    gtString defaultExecutionDataAsStr;

    TiXmlNode* pKANode = new TiXmlElement(xmlSectionTitle().asASCIICharArray());

    pKANode->Parse(projectAsXMLString.asASCIICharArray(), 0, TIXML_DEFAULT_ENCODING);
    gtString kaNodeTitle;
    kaNodeTitle.fromASCIIString(pKANode->Value());

    if (xmlSectionTitle() == kaNodeTitle)
    {
        retVal = true;
        gtString targetDeviceNode(KA_STR_targetDeviceSettingTargetDeviceNode);
        afUtils::getFieldFromXML(*pKANode, targetDeviceNode, targetDeviceAsStr);
        afUtils::getFieldFromXML(*pKANode, KA_STR_analsisSettingDefaultExecutionValuesNode, defaultExecutionDataAsStr);
        // restore \t from escape code %9 since xml does not store tabs correctly
        targetDeviceAsStr.replace(L"%9", L"\t");

        // restore space from escape code %20 to restore the case of multiple spaces
        targetDeviceAsStr.replace(L"%20", L" ");

        GT_IF_WITH_ASSERT((m_pTargetDevicesTreeView != NULL) && (m_pTargetDevicesTreeModel != NULL))
        {
            QString targetDeviceString(targetDeviceAsStr.asASCIICharArray());
            QStringList targetDeviceStringList = targetDeviceString.split(",");
            // remove the last item after the "," which is a dummy
            targetDeviceStringList.removeLast();

            // To overcome changes in the driver, we want to see if there is miss-match in the ASICs names. If this is the case we simply
            // use the default list which is the update asic list that the new drive supports.
            // This will result in reset in the ASICs the user chose.
            QStringList& defaultList = kaGlobalVariableManager::instance().defaultTreeList();
            bool doReset = false;

            // first check if all devices in the driver list exist in the current list (new devices) - just check the sizes
            if ((!doReset) && (defaultList.size() != targetDeviceStringList.size()))
            {
                doReset = true;
            }

            // second check if all devices in the current list exist in the driver list
            QChar qcHyphen('-');

            for (int i = 0; (!doReset) && (i < targetDeviceStringList.size()); i++)
            {
                // the list in the default contains the ASIC name followed by "-1" or "-0"
                QString targetDevice = targetDeviceStringList.at(i);
                int indexOfHyphenChar = targetDevice.lastIndexOf(qcHyphen);

                if (indexOfHyphenChar != -1)
                {
                    bool foundItem = false;
                    targetDevice.truncate(indexOfHyphenChar);
                    targetDevice = targetDevice.trimmed();

                    for (const auto& it : defaultList)
                    {
                        if (it.contains(targetDevice))
                        {
                            foundItem = true;
                        }

                    }

                    if (!foundItem)
                    {
                        doReset = true;
                        break;
                    }
                }
            }

            // now update the correct list
            if (doReset)
            {
                kaGlobalVariableManager::instance().setCurrentTreeList(defaultList);
                replaceModel(defaultList);
            }
            else
            {
                kaGlobalVariableManager::instance().setCurrentTreeList(targetDeviceStringList);
                replaceModel(targetDeviceStringList);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        kaAnalyzeSettingsPage::loadCurrentSettings
// Description: Loads the current values into the settings page
// Author:      Gilad Yarnitzky
// Date:        30/7/2013
// ---------------------------------------------------------------------------
void kaAnalyzeSettingsPage::loadCurrentSettings()
{
    GT_IF_WITH_ASSERT(m_pTargetDevicesTreeView != NULL)
    {
        // fill the tree again with the saved tree list in global manager:
        QStringList& currentList = kaGlobalVariableManager::instance().currentTreeList();
        replaceModel(currentList);
    }
}

// ---------------------------------------------------------------------------
// Name:        kaAnalyzeSettingsPage::restoreDefaultSettings
// Description: Restores the default values into all the views.
// Author:      Gilad Yarnitzky
// Date:        30/7/2013
// ---------------------------------------------------------------------------
void kaAnalyzeSettingsPage::restoreDefaultSettings()
{
    GT_IF_WITH_ASSERT((m_pTargetDevicesTreeView != NULL) && (m_pTargetDevicesTreeModel != NULL))
    {
        // fill the tree again with the default tree list:
        QStringList& defaultList = kaGlobalVariableManager::instance().defaultTreeList();
        replaceModel(defaultList);
    }
}

// ---------------------------------------------------------------------------
// Name:        kaAnalyzeSettingsPage::saveCurrentSettings
// Description: Applies the current settings to the data structures
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        30/7/2013
// ---------------------------------------------------------------------------
bool kaAnalyzeSettingsPage::saveCurrentSettings()
{
    bool retVal = false;

    GT_IF_WITH_ASSERT((m_pTargetDevicesTreeView != NULL) && (m_pTargetDevicesTreeModel != NULL))
    {
        QStringList currentListToSave;
        m_pTargetDevicesTreeModel->getStrData(&currentListToSave);
        kaGlobalVariableManager::instance().setCurrentTreeList(currentListToSave);

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
bool kaAnalyzeSettingsPage::IsPageDataValid()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pTargetDevicesTreeModel != nullptr)
    {
        bool shouldValidate = (m_pTargetDevicesTreeModel->rowCount() > 0);

        if (shouldValidate)
        {
            // Check that at least one device is selected
            set<string> selectedDeviceName;
            kaApplicationCommands::instance().getSelectedDevices(selectedDeviceName);

            if (selectedDeviceName.empty())
            {
                acMessageBox::instance().critical(AF_STR_WarningA, KA_STR_mustSelectDeviceError);
            }
        }
    }

    return true;
}


// ---------------------------------------------------------------------------
// Name:        kaAnalyzeSettingsPage::replaceModel
// Description: replace the model using the specified string list:
// Arguments:   QStringList& modelStringList
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        4/8/2013
// ---------------------------------------------------------------------------
void kaAnalyzeSettingsPage::replaceModel(QStringList& modelStringList)
{
    // Only make the new model if the string list is not empty (due to bug in QT that does not handle empty models correctly):
    if (!modelStringList.isEmpty())
    {
        QAbstractItemModel* oldModel = m_pTargetDevicesTreeView->model();
        m_pTargetDevicesTreeModel = new kaTreeModel(modelStringList, m_pTargetDevicesTreeView);
        GT_IF_WITH_ASSERT(NULL != m_pTargetDevicesTreeView)
        {
            m_pTargetDevicesTreeView->setModel(m_pTargetDevicesTreeModel);
        }

        if (oldModel != NULL)
        {
            delete oldModel;
        }
    }
}

// ---------------------------------------------------------------------------
void kaAnalyzeSettingsPage::checkDriverVersion()
{
    int driverYear = 0, driverMonth = 0;
    int errCode = 0;
    gtString driverVersion = oaGetDriverVersion(errCode);

    if (!driverVersion.isEmpty())
    {
        int itemsScaned = sscanf(driverVersion.asASCIICharArray(), "%d.%d", &driverYear, &driverMonth);
        GT_IF_WITH_ASSERT(2 == itemsScaned)
        {
            // Set the time for the driver:
            osTime driverTime;
            driverTime.setTime(osTime::LOCAL, 2000 + driverYear, driverMonth, 0, 0, 0, 0);
            // Get the computer date:
            osTime computerTime;
            computerTime.setFromCurrentTime();

            // Check if the difference. If it is bigger the 6 months notify the user his driver is out of date.
            if (computerTime.secondsFrom1970() - driverTime.secondsFrom1970() > 15724800) // 3600*24*182 (seconds in half a year)
            {
                if (NULL != m_pInformationCaption)
                {
                    QString newLabel = KA_STR_targetDeviceSettingsPageGeneralInfo;
                    newLabel += "<br><br>";
                    newLabel += KA_STR_driverOutOfData;
                    m_pInformationCaption->setText(newLabel);
                }
            }
        }
    }
}
