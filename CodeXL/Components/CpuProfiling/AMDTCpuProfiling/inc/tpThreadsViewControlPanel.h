//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpThreadsViewControlPanel.h
///
//==================================================================================

//------------------------------ tpThreadsView.h ------------------------------

#ifndef __TPTHREADSVIEWCONTROLPANEL_H
#define __TPTHREADSVIEWCONTROLPANEL_H

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// Infra:
#include <AMDTApplicationComponents/Include/acSplitter.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/views/afBaseView.h>

// Local:
#include <inc/tpDisplaySettingsDialog.h>
#include <inc/tpDisplayInfo.h>
#include <inc/tpSessionData.h>

class tpThreadsViewControlPanel : public QWidget
{
    Q_OBJECT

public:
    tpThreadsViewControlPanel(QWidget* pParent, tpSessionData* pSessionData);
    virtual ~tpThreadsViewControlPanel();

    /// get displayed settings element
    /// \returns a reference to the displayed settings element
    tpDisplaySettingsDialog* DisplayedSettings() { return m_pDisplaySettingDialog; }

signals:
    /// signal for control panel data changed
    /// \param data is a structure of the panel data
    void OnControlPanelDateChanged(const tpControlPanalData& data);

protected:

    void BuildPanelLayout();

    /// update settings link label
    /// \returns threads total number
    int UpdateSettingsLinkLabel();

protected slots:
    /// function called on display settings link clicked
    void OnDisplaySettingsClicked(const QString&);

    /// control panel widget changed slot
    /// \param val - is not used
    void ControlPanelWidgetChanged(int val);

    /// control panel widget changed slot
    /// \param val - is not used
    void ControlPanelWidgetChanged(QString val);

private:
    /// display settings link label
    QLabel* m_pTpSettingsLink;

    /// display setting dialog window instance
    tpDisplaySettingsDialog* m_pDisplaySettingDialog;

    /// is to display top
    QCheckBox* m_pDisplayTopCheckBox;

    /// display top value
    QSpinBox* m_pDisplayTopSpinBox;

    /// session data
    tpSessionData* m_pSessionData;
};

#endif // __TPTHREADSVIEWCONTROLPANEL_H
