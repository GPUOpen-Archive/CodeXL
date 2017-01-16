//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpThreadsViewLegend.cpp
///
//==================================================================================

//------------------------------ tpThreadsViewLegend.cpp ------------------------------

// QT:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>


// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Backend:
#include <AMDTThreadProfileAPI/inc/AMDTThreadProfileDataTypes.h>
#include <AMDTThreadProfileAPI/inc/AMDTThreadProfileApi.h>

// Local:
#include <inc/StringConstants.h>
#include <inc/tpDisplayInfo.h>
#include <inc/tpSessionData.h>
#include <inc/tpThreadsViewLegend.h>


tpThreadsViewLegend::tpThreadsViewLegend(QWidget* pParent, tpSessionData* pSessionData) : acTreeCtrl(pParent), m_pSessionData(pSessionData)
{
    // Set the header caption:
    setHeaderLabel(CP_STR_ThreadsLegendCaption);

    // Add each of the thread states to the legend:
    for (int i = (int)AMDT_THREAD_STATE_INITIALIZED; i < (int)AMDT_THREAD_STATE_GATEWAIT; i++)
    {
        QColor color;
        QString str;
        AMDTThreadState state = (AMDTThreadState)i;
        tpDisplayInfo::Instance().GetThreadStateDisplayData(state, color, str);

        str.prepend(CP_STR_ThreadsLegendPrefix);

        QTreeWidgetItem* pNewItem = new QTreeWidgetItem(QStringList(str));
        pNewItem->setTextColor(0, color);
        addTopLevelItem(pNewItem);

    }

    // Add the cores to the tree:
    QTreeWidgetItem* pCoresItem = new QTreeWidgetItem(QStringList(CP_STR_ThreadsLegendRunningOn));
    addTopLevelItem(pCoresItem);

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSessionData != nullptr)
    {
        // Get the number of cores:
        int coresCount = m_pSessionData->TotalCoresCount();

        for (int i = 0; i < coresCount; i++)
        {
            QTreeWidgetItem* pNewItem = new QTreeWidgetItem(pCoresItem, QStringList(QString(CP_STR_ThreadsTimelineCoreSubBranch).arg(i)));
            QColor color = tpDisplayInfo::Instance().GetColorForCore(i, coresCount);
            QIcon icon;
            tpDisplayInfo::Instance().GetIconForCore(icon, i, coresCount);
            pNewItem->setTextColor(0, color);
            pNewItem->setIcon(0, icon);
            pCoresItem->addChild(pNewItem);
        }
    }

    // Expand the cores item:
    pCoresItem->setExpanded(true);

}

tpThreadsViewLegend::~tpThreadsViewLegend()
{

}
