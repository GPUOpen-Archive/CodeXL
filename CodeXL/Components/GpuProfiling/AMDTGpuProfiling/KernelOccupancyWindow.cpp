//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/KernelOccupancyWindow.cpp $
/// \version $Revision: #10 $
/// \brief :  This file contains KernelOccupancyWindow
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/KernelOccupancyWindow.cpp#10 $
// Last checkin:   $DateTime: 2016/03/28 08:51:36 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 565856 $
//=====================================================================

// Infra:
#include <AMDTApplicationComponents/Include/acDisplay.h>

// Local:
#include <AMDTGpuProfiling/KernelOccupancyWindow.h>
#include <AMDTGpuProfiling/ProfileManager.h>

KernelOccupancyWindow::KernelOccupancyWindow(QWidget* parent) : QWidget(parent), m_pWebBrowser(NULL)
{
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    m_pWebBrowser = new QWebEngineView(this);
    m_pWebBrowser->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_pWebBrowser->setContextMenuPolicy(Qt::NoContextMenu);
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(m_pWebBrowser);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
}

void KernelOccupancyWindow::Clear()
{
    m_pWebBrowser->setUrl(QUrl("about:blank"));
}

bool KernelOccupancyWindow::LoadOccupancyHTMLFile(const QString& strOutputPage)
{
    double zoomFactor = (double)acGetDisplayScalePercent() / 100.0;
    m_pWebBrowser->setZoomFactor(zoomFactor);

    m_pWebBrowser->setUrl(QUrl::fromLocalFile(strOutputPage));
    m_pWebBrowser->setZoomFactor(zoomFactor);
    return true;
}

void KernelOccupancyWindow::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    if (event->oldSize().width() != event->size().width())
    {
        m_pWebBrowser->reload();
    }
}
