//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuRetryDialog.cpp
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/src/CpuRetryDialog.cpp#7 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569613 $
//=============================================================

#include <AMDTBaseTools/Include/gtAssert.h>

#include <inc/CpuRetryDialog.h>

const int SECONDSTOWAIT = 15;
const char FORMATSTRING[] = "Some other process has locked the hardware profiling counters\n"
                            "Retrying in %d seconds...";

CpuRetryDialog::CpuRetryDialog(QWidget* parent, Qt::WindowFlags fl) : QDialog(parent, fl), m_seconds(SECONDSTOWAIT)
{
    m_timerId = startTimer(1000); //timer goes every 1 second

    QVBoxLayout* pLayout = new QVBoxLayout;


    m_pText = new QLabel;


    if (nullptr != m_pText)
    {
        QString text;
        text.sprintf(FORMATSTRING, m_seconds);
        m_pText->setText(text);
        pLayout->addWidget(m_pText);
    }

    m_pCancel = new QPushButton("Cancel", this);


    if (nullptr != m_pCancel)
    {
        m_pCancel->setDefault(true);
        connect(m_pCancel, SIGNAL(clicked()), this, SLOT(onCancel()));
        pLayout->addWidget(m_pCancel);
    }

    setLayout(pLayout);
}

CpuRetryDialog::~CpuRetryDialog()
{
    if (0 != m_timerId)
    {
        killTimer(m_timerId);
        m_timerId = 0;
    }
}

void CpuRetryDialog::onCancel()
{
    killTimer(m_timerId);
    m_timerId = 0;
    reject();
}

//Implemented from QObject
void CpuRetryDialog::timerEvent(QTimerEvent* event)
{
    (void)(event); // unused
    --m_seconds;

    if (nullptr != m_pText)
    {
        QString text;
        text.sprintf(FORMATSTRING, m_seconds);
        m_pText->setText(text);
    }

    if (0 == m_seconds)
    {
        killTimer(m_timerId);
        m_timerId = 0;
        accept();
    }
}
