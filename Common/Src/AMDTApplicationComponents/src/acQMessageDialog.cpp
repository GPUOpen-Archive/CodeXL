//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acQMessageDialog.cpp
///
//==================================================================================

//------------------------------ acQMessageDialog.cpp ------------------------------

// Warnings:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Local:
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acQMessageDialog.h>


// Graphics:
#define ACQ_MESSAGE_DIALOG_DEFAULT_BG_COLOR QColor(236, 233, 216, 255)

// ---------------------------------------------------------------------------
// Name:        acQMessageDialog::acQMessageDialog
// Description: A customized message dialog with an OK button and a QEditBox
// Arguments:   const gtASCIIString& title - message title
//              const gtASCIIString& header - message header
//              const gtASCIIString& message - message body
//              QWidget* pParent
//              QSize size - the size of the text box - determines the size of the window
// Author:      Yoni Rabin
// Date:        22/5/2012
// ---------------------------------------------------------------------------
acQMessageDialog::acQMessageDialog(const QString& title, const QString& header, const QString& message, QWidget* pParent, QSize size)
    : QDialog(pParent, Qt::Popup | Qt::Dialog | Qt::WindowCloseButtonHint)
{
    // Set the window title:
    setWindowTitle(title);

    QVBoxLayout* pLayout = new QVBoxLayout;


    // Create the header text:
    QLabel* pHeaderLabel = new QLabel(header);


    pLayout->addWidget(pHeaderLabel);

    if (!message.isEmpty())
    {
        // Create the message body text:
        m_pTextBox = new QTextEdit();

        // Allow the text box to accept the text in the proper format:
        m_pTextBox->setAcceptRichText(true);
        m_pTextBox->setText(message);

        // Make text box use BG color:
        m_pTextBox->setAutoFillBackground(true);
        m_pTextBox->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        m_pTextBox->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        m_pTextBox->setReadOnly(true);
        m_pTextBox->setMinimumWidth(size.width() - 20);
        m_pTextBox->setMinimumHeight(size.height() - 100);
        // Set the text box color:
        m_palette.setColor(QPalette::Base, ACQ_MESSAGE_DIALOG_DEFAULT_BG_COLOR);
        m_palette.setColor(QPalette::Text, Qt::black);
        m_pTextBox->setPalette(m_palette);

        // Add the text box to the layout:
        pLayout->addWidget(m_pTextBox, 1);
    }

    // Create the dialog button box:
    QDialogButtonBox* pDialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Vertical);


    // Add the button box to the layout:
    pLayout->addWidget(pDialogButtonBox, 0, Qt::AlignCenter);
    connect(pDialogButtonBox, SIGNAL(accepted()), this, SLOT(accept()));

    // Set the main layout:
    this->resize(size);
    pLayout->setSizeConstraint(QLayout::SetFixedSize);
    setLayout(pLayout);
}

