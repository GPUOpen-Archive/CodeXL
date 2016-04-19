//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscBuildListDialog.cpp
///
//==================================================================================

//------------------------------ vscBuildListDialog.cpp ------------------------------
#include "stdafx.h"

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acListCtrl.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAidFunctions.h>

// Local:
#include <src/vscBuildListDialog.h>
#include <CodeXLVSPackage/Include/vspStringConstants.h>


#define VSP_BUILD_DIALOG_MIN_WIDTH                      400
#define VSP_BUILD_DIALOG_MIN_HEIGHT                     400
#define VSP_BUILD_DIALOG_LIST_MIN_WIDTH                 300
#define VSP_BUILD_DIALOG_LIST_MIN_HEIGHT                200
#define VSP_BUILD_DIALOG_LIST_ID                        100


// ---------------------------------------------------------------------------
// Name:        vscBuildListDialog
// Description: constructor
// Author:      Gilad Yarnitzky
// Date:        24/3/2011
// ---------------------------------------------------------------------------
vscBuildListDialog::vscBuildListDialog(QWidget* pParent, gtVector<gtString>& projectNames)
    : QDialog(pParent),  m_pProjectsList(NULL), m_pUpperText(NULL), m_pLowerText(NULL), m_pYesButton(NULL), m_shouldBuild(false)
{
    // Build the dialog layout:
    buildLayout();

    // Set the dialog caption:
    setWindowTitle(VSP_STR_BuildListDialogCaption);

    // Set window flags (minimize / maximize / close buttons):
    Qt::WindowFlags flags = windowFlags();
    flags &= ~Qt::WindowContextHelpButtonHint;
    setWindowFlags(flags);

    // Add the Icon to the dialog
    QIcon dummyIcon;
    setWindowIcon(dummyIcon);

    // Set the dialog size:
    resize(QSize(VSP_BUILD_DIALOG_MIN_WIDTH, VSP_BUILD_DIALOG_MIN_HEIGHT));

    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setSizePolicy(sizePolicy);


    // Copy projects names into the list:
    for (int nProject = 0 ; nProject < (int)projectNames.size(); nProject++)
    {
        m_pProjectsList->addRow(projectNames[nProject].asASCIICharArray());
    }
}

// ---------------------------------------------------------------------------
// Name:        ~vscBuildListDialog
// Description: destructor
// Author:      Gilad Yarnitzky
// Date:        24/3/2011
// ---------------------------------------------------------------------------
vscBuildListDialog::~vscBuildListDialog()
{
}

// ---------------------------------------------------------------------------
// Name:        vscBuildListDialog::buildLayout
// Description: build the dialog layout
// Author:      Gilad Yarnitzky
// Date:        24/3/2011
// ---------------------------------------------------------------------------
void vscBuildListDialog::buildLayout()
{
    // Creating the layout:
    QVBoxLayout* pMainLayout = new QVBoxLayout;

    // Projects list to build:
    m_pProjectsList = new acListCtrl(this, AC_DEFAULT_LINE_HEIGHT, false, false);

    m_pProjectsList->resize(QSize(VSP_BUILD_DIALOG_LIST_MIN_WIDTH, VSP_BUILD_DIALOG_LIST_MIN_HEIGHT));
    m_pProjectsList->setColumnCount(1);
    m_pProjectsList->setShowGrid(false);

    QColor bgColor = acGetSystemDefaultBackgroundColor();

    // Set the background color:
    QPalette p = m_pProjectsList->palette();
    p.setColor(m_pProjectsList->backgroundRole(), bgColor);
    p.setColor(QPalette::Base, bgColor);
    m_pProjectsList->setAutoFillBackground(true);
    m_pProjectsList->setPalette(p);



    m_pProjectsList->verticalHeader()->hide();
    m_pProjectsList->horizontalHeader()->hide();

    // Create text lines:
    m_pUpperText = new QLabel(VSP_STR_BuildListDialogUpperText);

    m_pLowerText = new QLabel(VSP_STR_BuildListDialogLowerText);

    QDialogButtonBox* pBox = new QDialogButtonBox(QDialogButtonBox::Yes | QDialogButtonBox::No | QDialogButtonBox::Cancel);

    m_pYesButton = pBox->button(QDialogButtonBox::Yes);
    QPushButton* pNoButton = pBox->button(QDialogButtonBox::No);

    pMainLayout->addSpacing(10);
    pMainLayout->addWidget(m_pUpperText, 0, Qt::AlignLeft | Qt::AlignTop);
    pMainLayout->addSpacing(5);
    pMainLayout->addWidget(m_pProjectsList, 0, Qt::AlignCenter | Qt::AlignTop);
    pMainLayout->addSpacing(5);
    pMainLayout->addWidget(m_pLowerText, 0, Qt::AlignLeft | Qt::AlignTop);
    pMainLayout->addWidget(pBox);

    pBox->setCenterButtons(true);

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
// Name:        vscBuildListDialog::onAccept
// Description: Stores the user selection (yes / no)
// Author:      Sigal Algranaty
// Date:        16/8/2012
// ---------------------------------------------------------------------------
void vscBuildListDialog::onAccept()
{
    m_shouldBuild = (m_pYesButton == sender());

    accept();
}
