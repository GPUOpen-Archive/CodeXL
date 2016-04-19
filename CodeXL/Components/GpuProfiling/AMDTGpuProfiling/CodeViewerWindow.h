//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/CodeViewerWindow.h $
/// \version $Revision: #8 $
/// \brief  This file contains code viewer Widget
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/CodeViewerWindow.h#8 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================
#ifndef _CODE_VIEWER_WINDOW_H_
#define _CODE_VIEWER_WINDOW_H_
#include <qtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

#include <QtCore>
#include <QtWidgets>
// QScintilla
#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexercpp.h>

#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtASCIIString.h>

#include <AMDTApplicationFramework/Include/views/afSourceCodeView.h>


//forward declaration
class osFilePath;
class GPUSessionTreeItemData;

// forward declaration
class Session;

/// QsciLexerCpp descendant used to force use of a fixed-width font
class CodeViewerLexer: public QsciLexerCPP
{
public:
    /// Gets the default font to use for this lexer
    /// \param style the style whose font is requested
    /// \return the font to use
    QFont defaultFont(int style) const;
};

/// CodeViewer widget class.
class CodeViewerWindow : public QWidget
{
    Q_OBJECT
public:
    /// Construct and initialize a new instance of the QCodeViewerControl class
    /// \param parent the parent widget
    CodeViewerWindow(QWidget* parent = 0);

    /// Destructor
    ~CodeViewerWindow() {}

    /// Load the specified session's kernel files.
    /// \param the session whose kernel files should be loaded
    /// \param strKernelName is the name of the kernel for which files need to load.
    /// \return Indicates execution status of the function.
    bool LoadKernelCodeFiles(GPUSessionTreeItemData* pSessionData, const QString& strKernelName);

    /// Copies any selected text to the clipboard:
    virtual void OnEditCopy();

    /// Selects all text
    virtual void OnEditSelectAll();

    /// Update Edit menu Copy and SelectAll items:
    void onUpdateEdit_Copy(bool& isEnabled);
    void onUpdateEdit_SelectAll(bool& isEnabled);

    /// Handle the find functionality
    void onFindClick();
    void onEditFindNext();

private:
    /// Loads the specified file, and stores its contents in the m_fileContents list
    /// \param filepath the file to load
    bool LoadFile(const osFilePath& filePath);

    /// Clears the view
    void Clear();

private slots:
    /// Called when item changes in m_pCodeViewerCB
    /// \param index the index of the item in the combo box
    void ComboBoxIndexChangedHandler(int index);

private:

    QComboBox*              m_pCodeViewerCB;       ///< The combo box to list all available valid kernel code file types.
    QVBoxLayout*            m_pCodeViewerVLayout;  ///< Code viewer vertical layout.
    afSourceCodeView        m_codeViewerTextEdit;  ///< The read-only text box to show kernel file contents.
    CodeViewerLexer         m_codeViewerLexer;     ///< Code viewer lexer

    gtVector<gtASCIIString> m_fileContents;        ///< list of kernel file contents
};


#endif
