//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuGlobalSettings.cpp
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/src/CpuGlobalSettings.cpp#28 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569613 $
//=============================================================
//qt
#include <QtCore>
#include <QtWidgets>
#include <qfiledialog.h>

//infrastructure
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/apBasicParameters.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afBrowseAction.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTOSWrappers/Include/osFilePath.h>

//local
#include <inc/CpuGlobalSettings.h>
#include <inc/CpuProfilingOptions.h>
#include <inc/StringConstants.h>
#include <inc/DirSearchDialog.h>

// Default block size of disassembly instructions to fetch.
const unsigned int DEFAULT_DISASSEMBLY_INSTRUCTIONS_BLOCK_SIZE = 1024;
const unsigned int MIN_DISASSEMBLY_INSTRUCTIONS_BLOCK_SIZE = 512;
const unsigned int MAX_DISASSEMBLY_INSTRUCTIONS_BLOCK_SIZE = 65536;

CpuGlobalSettings::CpuGlobalSettings() : afGlobalSettingsPage(), m_pBrowseSymbolDirAction(nullptr)
{
}

CpuGlobalSettings::~CpuGlobalSettings()
{
}

// Inherited from afGlobalSettingsPage
void CpuGlobalSettings::initialize()
{
    setupUi(this);

#if AMDT_BUILD_TARGET != AMDT_WINDOWS_OS
    m_pUseSymbolServer->hide();
#endif

    // Create the browse action for the symbols directories:
    m_pBrowseSymbolDirAction = new afBrowseAction(AF_Str_NewProjectBrowseForSourceCodeRootFolder);
    m_pBrowseSymbolDirAction->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    m_pBrowseSymbolDir->setDefaultAction(m_pBrowseSymbolDirAction);
    m_pBrowseSymbolDir->setContentsMargins(0, 0, 0, 0);

    // Connect the buttons to actions:
    bool rc = connect(m_pBrowseDebugs, SIGNAL(clicked()), SLOT(onBrowseDebugDirs()));
    GT_ASSERT(rc);
    rc = connect(m_pBrowseSymbolDirAction, SIGNAL(triggered()), SLOT(onSearchSymDown()));
    GT_ASSERT(rc);
    rc = connect(m_pNewSymServer, SIGNAL(clicked()), SLOT(onNewSymDir()));
    GT_ASSERT(rc);
    rc = connect(m_pRemoveSymServer, SIGNAL(clicked()), SLOT(onRemoveSymDir()));
    GT_ASSERT(rc);
    rc = connect(m_pMoveUp, SIGNAL(clicked()), SLOT(onSymDirUp()));
    GT_ASSERT(rc);
    rc = connect(m_pMoveDown, SIGNAL(clicked()), SLOT(onSymDirDown()));
    GT_ASSERT(rc);

    rc = connect(m_pSymServerList, SIGNAL(currentItemChanged(QListWidgetItem*,
                                                             QListWidgetItem*)), SLOT(onSymServeItemChange(QListWidgetItem*)));
    GT_ASSERT(rc);

    // Set the text box to accept only numbers in the range [MIN_DISASSEMBLY_INSTRUCTIONS_BLOCK_SIZE, MAX_DISASSEMBLY_INSTRUCTIONS_BLOCK_SIZE].
    m_pDisassemblyInstructionCountLineEdit->setValidator(new QIntValidator(MIN_DISASSEMBLY_INSTRUCTIONS_BLOCK_SIZE,
                                                         MAX_DISASSEMBLY_INSTRUCTIONS_BLOCK_SIZE, this));

}

// Inherited from afGlobalSettingsPage
gtString CpuGlobalSettings::pageTitle()
{
    gtString retVal = CPU_STR_PROJECT_SETTINGS_TAB_NAME;
    return retVal;
}

// Inherited from afGlobalSettingsPage
gtString CpuGlobalSettings::xmlSectionTitle()
{
    gtString retVal = CPU_STR_PROJECT_EXTENSION;
    return retVal;
}

// Inherited from afGlobalSettingsPage
// Load / Save the settings into a string:
bool CpuGlobalSettings::getXMLSettingsString(gtString& projectAsXMLString)
{
    return CpuProfilingOptions::instance().getProjectSettingsXML(projectAsXMLString);
}
// Inherited from afGlobalSettingsPage
bool CpuGlobalSettings::setSettingsFromXMLString(const gtString& projectAsXMLString)
{
    bool retVal = false;
    retVal = CpuProfilingOptions::instance().setProjectSettingsXML(projectAsXMLString);
    //Update gui from settings
    updateGui();

    return retVal;
}

void CpuGlobalSettings::updateGui(bool bReset)
{
    PROFILE_OPTIONS* pOptions = CpuProfilingOptions::instance().options();

    if (bReset)
    {
        m_pAddDebugDirs->setChecked(false);
        m_pDebugDirText->setText("");
        m_pUseSymbolServer->setChecked(false);
        m_pSymbolDirText->setText("");

        // Disassembly instructions block size: take the default value.
        gtString disassemblyBlockSizeStr;
        disassemblyBlockSizeStr.appendFormattedString(L"%d", DEFAULT_DISASSEMBLY_INSTRUCTIONS_BLOCK_SIZE);
        m_pDisassemblyInstructionCountLineEdit->setText(disassemblyBlockSizeStr.asASCIICharArray());
    }
    else if (pOptions != nullptr)
    {
        m_pAddDebugDirs->setChecked(pOptions->addDebug);
        m_pDebugDirText->setText(pOptions->debugSearchPaths);
        m_pUseSymbolServer->setChecked(pOptions->enableSymServer);
        m_pSymbolDirText->setText(pOptions->symbolDownloadDir);

        // Just for extra safety.
        if (pOptions->disassemblyInstrcutionsChunkSize < MIN_DISASSEMBLY_INSTRUCTIONS_BLOCK_SIZE ||
            pOptions->disassemblyInstrcutionsChunkSize > MAX_DISASSEMBLY_INSTRUCTIONS_BLOCK_SIZE)
        {
            pOptions->disassemblyInstrcutionsChunkSize = DEFAULT_DISASSEMBLY_INSTRUCTIONS_BLOCK_SIZE;
        }

        // Disassembly instructions block size: take the value from the data object.
        gtString disassemblyBlockSizeStr;
        disassemblyBlockSizeStr.appendFormattedString(L"%d", pOptions->disassemblyInstrcutionsChunkSize);
        m_pDisassemblyInstructionCountLineEdit->setText(disassemblyBlockSizeStr.asASCIICharArray());
    }

    m_pSymServerList->clear();

    if (pOptions != nullptr && !pOptions->symSrvList.isEmpty())
    {
        QStringList servers = pOptions->symSrvList.split(';', QString::SkipEmptyParts);

        //Using a 32-bit value to store the server enabled mask
        int i = 0;

        do
        {
            QListWidgetItem* pItem = new QListWidgetItem(servers.at(i));

            if (i > 0)
            {
                pItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsUserCheckable | pItem->flags());
            }
            else
            {
                //The first item (0) is the default symbol servers from Microsoft and is not editable
                pItem->setFlags(Qt::ItemIsUserCheckable | pItem->flags());
            }

            if (!bReset && ((pOptions->useSymSrvMask & (1 << i)) > 0))
            {
                pItem->setCheckState(Qt::Checked);
            }
            else
            {
                pItem->setCheckState(Qt::Unchecked);
            }

            ++i;
            m_pSymServerList->addItem(pItem);
        }
        while (!bReset && (i < min(servers.size(), 32)));
    }
}

// Inherited from afGlobalSettingsPage
// Display the current settings in the page:
void CpuGlobalSettings::loadCurrentSettings()
{
    updateGui();
}

// Inherited from afGlobalSettingsPage
// Restore the content to default settings:
void CpuGlobalSettings::restoreDefaultSettings()
{
    updateGui(true);
}

// Inherited from afGlobalSettingsPage
// Save the data as it was changed in the widget to the specific options manager (when "Ok" is pressed):
bool CpuGlobalSettings::saveCurrentSettings()
{
    PROFILE_OPTIONS* pOptions = CpuProfilingOptions::instance().options();

    pOptions->addDebug = m_pAddDebugDirs->isChecked();
    pOptions->debugSearchPaths = m_pDebugDirText->text();
    pOptions->enableSymServer = m_pUseSymbolServer->isChecked();
    pOptions->symbolDownloadDir = m_pSymbolDirText->text();

    pOptions->useSymSrvMask = 0;
    QStringList servers;

    for (int i = 0; ((i < m_pSymServerList->count()) && (i < 32)); i++)
    {
        servers.append(m_pSymServerList->item(i)->text());

        if (Qt::Checked == m_pSymServerList->item(i)->checkState())
        {
            pOptions->useSymSrvMask |= (1 << i);
        }
    }

    pOptions->symSrvList = servers.join(";");

    // Trim and remove leading zeros from the instruction block size text box.
    QString disassemblyInstructionCountAsQstr = m_pDisassemblyInstructionCountLineEdit->text().trimmed().remove(QRegExp("^[0]*"));

    // Set the adjusted string to the text box.
    m_pDisassemblyInstructionCountLineEdit->setText(disassemblyInstructionCountAsQstr);

    // Extract the disassembly instructions chunk size.
    gtString disassemblyInstructionCountAsGtStr = acQStringToGTString(disassemblyInstructionCountAsQstr);
    unsigned int diassemblyInstructionCount = DEFAULT_DISASSEMBLY_INSTRUCTIONS_BLOCK_SIZE;
    bool isOk = disassemblyInstructionCountAsGtStr.toUnsignedIntNumber(diassemblyInstructionCount) &&
                (diassemblyInstructionCount >= MIN_DISASSEMBLY_INSTRUCTIONS_BLOCK_SIZE) && (diassemblyInstructionCount <= MAX_DISASSEMBLY_INSTRUCTIONS_BLOCK_SIZE);

    if (!isOk)
    {
        // Notify the user.
        acMessageBox::instance().warning(AF_STR_GLOBAL_SETTINGS_CPU_INVALID_DISASSEMBLY_BLOCK_SIZE_TITLE, AF_STR_GLOBAL_SETTINGS_CPU_INVALID_DISASSEMBLY_BLOCK_SIZE_BODY, QMessageBox::Ok);

        // Return to default value.
        gtString defaultValueAsStr;
        defaultValueAsStr.appendFormattedString(L"%d", DEFAULT_DISASSEMBLY_INSTRUCTIONS_BLOCK_SIZE);
        m_pDisassemblyInstructionCountLineEdit->setText(defaultValueAsStr.asASCIICharArray());
    }

    // Update the data object with the selected chunk size.
    pOptions->disassemblyInstrcutionsChunkSize = diassemblyInstructionCount;

    CpuProfilingOptions::instance().emitSettingsUpdated();
    return true;
}

void CpuGlobalSettings::onNewSymDir()
{
    QListWidgetItem* pItem = new QListWidgetItem("http://");
    pItem->setCheckState(Qt::Checked);
    pItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsUserCheckable | pItem->flags());
    m_pSymServerList->addItem(pItem);
    m_pSymServerList->setCurrentItem(pItem);
    m_pSymServerList->editItem(pItem);
}

void CpuGlobalSettings::onRemoveSymDir()
{
    if (nullptr == m_pSymServerList->currentItem())
    {
        return;
    }

    int row = m_pSymServerList->currentRow();
    delete m_pSymServerList->currentItem();
    m_pSymServerList->setCurrentRow(row);
}


void CpuGlobalSettings::onSymDirUp()
{
    if (nullptr == m_pSymServerList->currentItem())
    {
        return;
    }

    int row = m_pSymServerList->currentRow();

    if (1 >= row)
    {
        return;
    }

    QListWidgetItem* pItem = m_pSymServerList->takeItem(row);
    m_pSymServerList->insertItem((row - 1), pItem);
    m_pSymServerList->setCurrentItem(pItem);
}

void CpuGlobalSettings::onSymDirDown()
{
    if (nullptr == m_pSymServerList->currentItem())
    {
        return;
    }

    int row = m_pSymServerList->currentRow();

    if ((0 == row) || ((m_pSymServerList->count() - 1) == row))
    {
        return;
    }

    QListWidgetItem* pItem = m_pSymServerList->takeItem(row);
    m_pSymServerList->insertItem((row + 1), pItem);
    m_pSymServerList->setCurrentItem(pItem);
}


void CpuGlobalSettings::onSearchSymDown()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(afApplicationCommands::instance() != nullptr)
    {
        // Get the user browsed folder:
        QString defaultFolder = m_pSymbolDirText->text();
        QString directoryPath = afApplicationCommands::instance()->ShowFolderSelectionDialog(C_STR_cpuProfileSymbolsServerDirectories, defaultFolder, m_pBrowseSymbolDirAction);

        if (!directoryPath.isNull())
        {
            m_pSymbolDirText->setText(directoryPath);
        }
    }
}

void CpuGlobalSettings::onBrowseDebugDirs()
{
    DirSearchDialog dlg(this);
    dlg.setDirList(m_pDebugDirText->text());

    if (QDialog::Accepted == dlg.exec())
    {
        m_pDebugDirText->setText(dlg.getDirList());
    }
}

void CpuGlobalSettings::onSymServeItemChange(QListWidgetItem* pCurrent)
{
    if (nullptr == pCurrent)
    {
        m_pRemoveSymServer->setEnabled(false);
        m_pMoveUp->setEnabled(false);
        m_pMoveDown->setEnabled(false);
    }
    else
    {
        int curRow = m_pSymServerList->currentRow();

        if (0 != curRow)
        {
            m_pRemoveSymServer->setEnabled(true);
            m_pMoveDown->setEnabled((m_pSymServerList->count() - 1) != curRow);
            m_pMoveUp->setEnabled(1 != curRow);
        }
        else
        {
            m_pRemoveSymServer->setEnabled(false);
            m_pMoveUp->setEnabled(false);
            m_pMoveDown->setEnabled(false);
        }
    }
}
