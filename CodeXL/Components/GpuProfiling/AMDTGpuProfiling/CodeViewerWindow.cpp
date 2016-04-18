//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/CodeViewerWindow.cpp $
/// \version $Revision: #18 $
/// \brief  This file contains code viewer controls
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/CodeViewerWindow.cpp#18 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================
#include <qtIgnoreCompilerWarnings.h>

// Qt:
#include <QtCore>
#include <QtWidgets>

#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexercpp.h>

#include <AMDTGpuProfiling/Util.h>
#include "Session.h"
#include "CodeViewerWindow.h"

// AMDTApplicationComponents:
#include <AMDTApplicationComponents/Include/acSourceCodeDefinitions.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osDirectory.h>


QFont CodeViewerLexer::defaultFont(int style) const
{
    QFont font = QsciLexerCPP::defaultFont(style);
    font.setFamily(AC_SOURCE_CODE_EDITOR_DEFAULT_FONT_FAMILY);
    font.setFixedPitch(true);
    font.setPointSize(AC_SOURCE_CODE_EDITOR_DEFAULT_FONT_SIZE);
    return font;
}

CodeViewerWindow::CodeViewerWindow(QWidget* parent) : QWidget(parent), m_codeViewerTextEdit(parent, true)
{
    m_pCodeViewerCB = new QComboBox(this);


    m_codeViewerTextEdit.setReadOnly(true);
    m_codeViewerTextEdit.setEnabled(false);
    m_codeViewerTextEdit.setBraceMatching(QsciScintilla::StrictBraceMatch);
    m_codeViewerTextEdit.setTabWidth(4);
    m_codeViewerTextEdit.setSelectionBackgroundColor(palette().highlight().color());

    m_codeViewerTextEdit.clear();
    m_codeViewerTextEdit.setLexer(&m_codeViewerLexer);

    m_pCodeViewerVLayout = new QVBoxLayout(this);


    m_pCodeViewerVLayout->addWidget(m_pCodeViewerCB);
    m_pCodeViewerVLayout->addWidget(&m_codeViewerTextEdit);

    setLayout(m_pCodeViewerVLayout);

    connect(m_pCodeViewerCB, SIGNAL(currentIndexChanged(int)), this, SLOT(ComboBoxIndexChangedHandler(int)));
}

void CodeViewerWindow::Clear()
{
    m_pCodeViewerCB->clear();
    m_codeViewerTextEdit.clear();
    m_fileContents.clear();
}

void CodeViewerWindow::ComboBoxIndexChangedHandler(int index)
{
    if (index >= 0 && index < (int)m_fileContents.size())
    {
        gtASCIIString fileContents = m_fileContents[index];

        if (!fileContents.isEmpty())
        {
            if (!m_codeViewerTextEdit.isEnabled())
            {
                m_codeViewerTextEdit.setEnabled(true);
            }

            m_codeViewerTextEdit.clear();
            m_codeViewerTextEdit.setText(acGTASCIIStringToQString(fileContents));
        }
    }
}

bool CodeViewerWindow::LoadFile(const osFilePath& filePath)
{
    bool retVal = false;
    osFile kernelFile;

    if (kernelFile.open(filePath, osChannel::OS_ASCII_TEXT_CHANNEL))
    {
        gtString fileExt;

        if (filePath.getFileExtension(fileExt))
        {
            fileExt.toUpperCase();

            // show DXASM for .asm files
            if (fileExt == L"ASM")
            {
                fileExt = L"DXASM";
            }

            gtASCIIString fileContents;

            if (kernelFile.readIntoString(fileContents))
            {
                m_fileContents.push_back(fileContents);
                m_pCodeViewerCB->addItem(acGTStringToQString(fileExt));
                retVal = true;
            }
        }
    }

    return retVal;
}

bool CodeViewerWindow::LoadKernelCodeFiles(GPUSessionTreeItemData* pSessionData, const QString& strKernelName)
{
    m_pCodeViewerCB->blockSignals(true); //don't update the source text while populating view
    Clear();

    bool retVal = false;

    gtList<osFilePath> kernelFilesList;

    if (Util::GetKernelFiles(pSessionData, strKernelName, kernelFilesList))
    {
        gtList<osFilePath>::const_iterator iter = kernelFilesList.begin();
        gtList<osFilePath>::const_iterator iterEnd = kernelFilesList.end();

        for (; iter != iterEnd; iter++)
        {
            retVal |= LoadFile(*iter);
        }

    }

    m_pCodeViewerCB->blockSignals(false);

    if (retVal)
    {
        int newCurrentIndex = m_pCodeViewerCB->count() - 1;

        if (m_pCodeViewerCB->currentIndex() != newCurrentIndex)
        {
            // if changing the index, a signal will be emitted and ComboBoxIndexChangedHandler will get called
            m_pCodeViewerCB->setCurrentIndex(newCurrentIndex);
        }
        else
        {
            // otherwise, we need to manually call ComboBoxIndexChangedHandler with the index to load the file -- this happens if there is only one kernel file available
            ComboBoxIndexChangedHandler(newCurrentIndex);
        }
    }

    return retVal;
}

void CodeViewerWindow::OnEditCopy()
{
    m_codeViewerTextEdit.copy();
}

void CodeViewerWindow::OnEditSelectAll()
{
    m_codeViewerTextEdit.selectAll();
}

void CodeViewerWindow::onUpdateEdit_Copy(bool& isEnabled)
{
    isEnabled = !m_codeViewerTextEdit.selectedText().isEmpty();
}

void CodeViewerWindow::onUpdateEdit_SelectAll(bool& isEnabled)
{
    isEnabled = true; // should not depend on current selection
}

void CodeViewerWindow::onFindClick()
{
    m_codeViewerTextEdit.onFindClick();
}

void CodeViewerWindow::onEditFindNext()
{
    m_codeViewerTextEdit.onFindNext();
}

