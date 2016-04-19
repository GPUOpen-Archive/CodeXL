//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspSaveListDialog.cpp
///
//==================================================================================

//------------------------------ vspSaveListDialog.cpp ------------------------------
#include "stdafx.h"

// C++:
#include <sstream>
#include <string>

// Qt:
#include <QtGui>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acTreeCtrl.h>

// Local:
#include <src/vspSaveListDialog.h>
#include <src/vspDTEConnector.h>
#include <CodeXLVSPackage/Include/vspStringConstants.h>
#include <Include/vscCoreInternalUtils.h>
#include <Include/Public/vscVspDTEInvoker.h>


#define VSP_SAVE_DIALOG_MIN_WIDTH                   400
#define VSP_SAVE_DIALOG_MIN_HEIGHT                  400
#define VSP_SAVE_DIALOG_LIST_MIN_WIDTH              300
#define VSP_SAVE_DIALOG_LIST_MIN_HEIGHT             200


// ---------------------------------------------------------------------------
// Name:        vspSaveListDialog
// Description: constructor
// Author:      Gilad Yarnitzky
// Date:        14/4/2011
// ---------------------------------------------------------------------------
vspSaveListDialog::vspSaveListDialog(QWidget* pParent)
    : QDialog(pParent), m_pFilesTree(NULL), m_pUpperText(NULL), m_pYesButton(NULL), m_userDecision(false), m_pTreeRoot(NULL)
{
    // Build the dialog layout:
    buildLayout();

    // Set dialog title and size:
    setWindowTitle(VSP_STR_BuildListDialogCaption);
    resize(QSize(VSP_SAVE_DIALOG_MIN_WIDTH, VSP_SAVE_DIALOG_MIN_HEIGHT));

    // Set window flags (minimize / maximize / close buttons):
    Qt::WindowFlags flags = windowFlags();
    flags &= ~Qt::WindowContextHelpButtonHint;
    setWindowFlags(flags);

    // Add the Icon to the dialog
    QIcon dummyIcon;
    setWindowIcon(dummyIcon);

    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setSizePolicy(sizePolicy);

    // Add the items to the tree:
    vscVspDTEInvoker_BuildSaveTree(this);
}

// ---------------------------------------------------------------------------
// Name:        ~vspSaveListDialog
// Description: destructor
// Author:      Gilad Yarnitzky
// Date:        14/4/2011
// ---------------------------------------------------------------------------
vspSaveListDialog::~vspSaveListDialog()
{
}

// ---------------------------------------------------------------------------
// Name:        vspSaveListDialog::buildLayout
// Description: build the dialog layout
// Author:      Gilad Yarnitzky
// Date:        14/4/2011
// ---------------------------------------------------------------------------
void vspSaveListDialog::buildLayout()
{
    // Creating the sizers:
    QVBoxLayout* pMainLayout = new QVBoxLayout;;


    // Projects list to build:
    m_pFilesTree = new acTreeCtrl(NULL);

    m_pFilesTree->setHeaderHidden(true);

    m_pFilesTree->resize(QSize(VSP_SAVE_DIALOG_LIST_MIN_WIDTH, VSP_SAVE_DIALOG_LIST_MIN_HEIGHT));

    // Create text lines:
    m_pUpperText = new QLabel(VSP_STR_SaveListDialogUpperText);


    QDialogButtonBox* pBox = new QDialogButtonBox(QDialogButtonBox::Yes | QDialogButtonBox::No | QDialogButtonBox::Cancel);


    m_pYesButton = pBox->button(QDialogButtonBox::Yes);
    QPushButton* pNoButton = pBox->button(QDialogButtonBox::No);

    pMainLayout->addSpacing(10);
    pMainLayout->addWidget(m_pUpperText, 0, Qt::AlignLeft | Qt::AlignTop);
    pMainLayout->addSpacing(5);
    pMainLayout->addWidget(m_pFilesTree, 0, Qt::AlignCenter | Qt::AlignTop);
    pMainLayout->addSpacing(5);
    pMainLayout->addWidget(pBox);


    // Connect the Ok button:
    bool rc = connect(m_pYesButton, SIGNAL(clicked()), this, SLOT(onAccept()));
    GT_ASSERT(rc);

    rc = connect(pNoButton, SIGNAL(clicked()), this, SLOT(onAccept()));
    GT_ASSERT(rc);

    rc = connect(pBox, SIGNAL(rejected()), this, SLOT(reject()));
    GT_ASSERT(rc);

    pMainLayout->setSizeConstraint(QLayout::SetFixedSize);

    setLayout(pMainLayout);
}


// ---------------------------------------------------------------------------
// Name:        vspSaveListDialog::hasItems
// Description: returns if the tree has items to save
// Author:      Gilad Yarnitzky
// Date:        14/4/2011
// ---------------------------------------------------------------------------
bool vspSaveListDialog::hasItems()
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(m_pFilesTree != NULL)
    {

        // More items then just the root:
        retVal = (m_pFilesTree->topLevelItemCount() != 0);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspSaveListDialog::onAccept
// Description: Stores the user selection (yes / no)
// Author:      Sigal Algranaty
// Date:        16/8/2012
// ---------------------------------------------------------------------------
void vspSaveListDialog::onAccept()
{
    m_userDecision = (m_pYesButton == sender());

    accept();
}

void vspSaveListDialog::vscAddSolutionAsItemToTree(const wchar_t* pSolutionName)
{
    GT_IF_WITH_ASSERT(m_pFilesTree != NULL)
    {
        std::string solName = vscWstrToStr(pSolutionName != NULL ? pSolutionName : L"");
        QStringList root;
        root << solName.c_str();
        m_pTreeRoot = m_pFilesTree->addItem(root, NULL);
    }
}

void vspSaveListDialog::vscAddIOpenDocumentAsItemToTree(const wchar_t* pDocumentName, const wchar_t* pDocumentProjectName, const wchar_t* pSolutionName)
{
    GT_IF_WITH_ASSERT(m_pTreeRoot != NULL)
    {
        // Check if the project name is already in the tree:
        std::wstring projectNameStr(pDocumentProjectName != NULL ? pDocumentProjectName : L"");
        std::string narrowProjectNameStr = vscWstrToStr(projectNameStr);
        QList<QTreeWidgetItem*> projectTreeItemList = m_pFilesTree->findItems(narrowProjectNameStr.c_str(), Qt::MatchExactly);
        QTreeWidgetItem* pProjectTreeItem = NULL;

        if (projectTreeItemList.isEmpty())
        {
            if (m_pTreeRoot == NULL)
            {
                QStringList solNameList;
                std::string solName = vscWstrToStr(pSolutionName != NULL ? pSolutionName : L"");
                solNameList << solName.c_str();
                m_pTreeRoot = m_pFilesTree->addItem(solNameList, NULL, NULL);
            }

            QStringList documentProjectNameList;
            std::wstring documentNameStr(pDocumentName != NULL ? pDocumentName : L"");
            std::string narrowstrDocumentName = vscWstrToStr(documentNameStr);
            documentProjectNameList << narrowstrDocumentName.c_str();
            pProjectTreeItem = m_pFilesTree->addItem(documentProjectNameList, NULL, m_pTreeRoot);
        }

        // Add the document item:
        std::wstring documentNameStr(pDocumentName != NULL ? pDocumentName : L"");
        std::string nDocumentName = vscWstrToStr(documentNameStr);
        QStringList documentNameList;
        documentNameList << nDocumentName.c_str();
        m_pFilesTree->addItem(documentNameList, NULL, pProjectTreeItem);
    }
}

void vspSaveListDialog::vscAddOpenProjectAsItemToTree(const wchar_t* pProjectName, const wchar_t* pSolutionName)
{
    // Check if the project name is already in the tree:
    std::wstring projectNameStr(pProjectName != NULL ? pProjectName : L"");
    std::string narrowProjectNameStr = vscWstrToStr(projectNameStr);
    QList<QTreeWidgetItem*> projectTreeItemList = m_pFilesTree->findItems(narrowProjectNameStr.c_str(), Qt::MatchExactly);
    QTreeWidgetItem* pProjectTreeItem = NULL;

    if (projectTreeItemList.isEmpty())
    {
        if (m_pTreeRoot == NULL)
        {
            QStringList solNameList;
            std::string solName = vscWstrToStr(pSolutionName != NULL ? pSolutionName : L"");
            solNameList << solName.c_str();
            m_pTreeRoot = m_pFilesTree->addItem(solNameList, NULL, NULL);
        }

        QStringList projectNameList;
        std::wstring projectNameWstr(pProjectName != NULL ? pProjectName : L"");
        std::string narrowProjectNameString = vscWstrToStr(projectNameWstr);
        projectNameList << narrowProjectNameString.c_str();
        pProjectTreeItem = m_pFilesTree->addItem(projectNameList, NULL, m_pTreeRoot);
    }
}

void vspSaveListDialog::vscExpandWholeTree()
{
    GT_IF_WITH_ASSERT(m_pFilesTree != NULL)
    {
        m_pFilesTree->expandAll();
    }
}
