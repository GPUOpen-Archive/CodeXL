//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afMessageBox.h
///
//==================================================================================

#ifndef __AFMESSAGEBOX_H
#define __AFMESSAGEBOX_H

// Qt
#include <QtWidgets>

// infra:
#include <AMDTApplicationComponents/Include/acMessageBox.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>

class AF_API afMessageBox
{
public:
    static afMessageBox& instance();
    virtual ~afMessageBox();

    /// A wrapper function to all specific message type functions
    /// @param[in] QMessageBox::Icon type  Message bos type: critical, information, question or warning
    /// @param[in] title Message box title
    /// @param[in] buttons  Message box buttons
    /// @param[in] defaultButton button with initial focus
    /// @return    button pressed
    QMessageBox::StandardButton ShowMessageBox(QMessageBox::Icon type, const QString& title, const QString& text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton);
    QMessageBox::StandardButton critical(const QString& title, const QString& text, QMessageBox::StandardButtons buttons = QMessageBox::Ok, QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);
    QMessageBox::StandardButton information(const QString& title, const QString& text, QMessageBox::StandardButtons buttons = QMessageBox::Ok, QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);
    QMessageBox::StandardButton question(const QString& title, const QString& text, QMessageBox::StandardButtons buttons = QMessageBox::Ok, QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);
    QMessageBox::StandardButton warning(const QString& title, const QString& text, QMessageBox::StandardButtons buttons = QMessageBox::Ok, QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

protected:
    //initial steps of the dialog:
    bool initDialog(const QString& title);

    // terminate dialog:
    void terminateDialog(const QString& title);

    // Do not allow the use of my default constructor:
    afMessageBox();
    static afMessageBox* m_spMessageBoxSingleInstance;
};

#endif //__AFMESSAGEBOX_H

