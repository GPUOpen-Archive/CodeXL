//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuRetryDialog.h
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/inc/CpuRetryDialog.h#4 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569613 $
//=============================================================
#ifndef _CPURETRYDIALOG_H
#define _CPURETRYDIALOG_H

// Qt:
#include <QtCore>
#include <QtWidgets>

class CpuRetryDialog : public QDialog
{
    Q_OBJECT
public:
    CpuRetryDialog(QWidget* parent = 0,
                   Qt::WindowFlags fl = Qt::Dialog | Qt::WindowSystemMenuHint | Qt::WindowTitleHint);
    virtual ~CpuRetryDialog();

public slots:
    void onCancel();

protected:
    //Implemented from QObject
    virtual void timerEvent(QTimerEvent* event);
private:
    unsigned int m_seconds;
    int m_timerId;
    QPushButton* m_pCancel;
    QLabel* m_pText;
};

#endif //_CPURETRYDIALOG_H
