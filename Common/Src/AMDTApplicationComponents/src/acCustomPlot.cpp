//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acCustomPlot.cpp
///
//==================================================================================


// Infra
#include  <AMDTBaseTools/Include/gtAssert.h>
#include  <AMDTOSWrappers/Include/osDebuggingFunctions.h>

// Local:
#include <AMDTApplicationComponents/inc/acStringConstants.h>
#include <AMDTApplicationComponents/Include/acCustomPlot.h>
#include <AMDTApplicationComponents/Include/acVectorLineGraph.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>

// Static members initialization:
acCustomPlotDropManager* acCustomPlotDropManager::m_psMySingleInstance = NULL;
QPixmap* acCustomPlot::m_spHighlightBackgroundPixmap = nullptr;
QPixmap* acCustomPlot::m_spBackgroundPixmap = nullptr;

static const QColor AC_HIGHLIGHT_RIBBON_BG_COLOR(0, 0, 200, 10);

acCustomPlotDropManager& acCustomPlotDropManager::Instance()
{
    if (m_psMySingleInstance == NULL)
    {
        m_psMySingleInstance = new acCustomPlotDropManager;
        GT_ASSERT(m_psMySingleInstance);
    }

    return *m_psMySingleInstance;
}

void acCustomPlotDropManager::EmitPlotDropped(acCustomPlot* pPlotDropped, acCustomPlot* pDraggedPlot)
{
    emit PlotDropped(pPlotDropped, pDraggedPlot);
}

acCustomPlot::acCustomPlot(QWidget* pParent) : QCustomPlot(pParent),
    m_isDragAndDropEnabled(false), m_isDragging(false), m_isHighlighted(false)

{
    // Enable drop:
    setAcceptDrops(true);

    if (m_spBackgroundPixmap == nullptr)
    {
        m_spBackgroundPixmap = new QPixmap(8, 8);
        m_spHighlightBackgroundPixmap = new QPixmap(8, 8);

        m_spHighlightBackgroundPixmap->fill(AC_HIGHLIGHT_RIBBON_BG_COLOR);

        QPainter painter;
        painter.begin(m_spHighlightBackgroundPixmap);
        painter.drawPixmap(m_spHighlightBackgroundPixmap->rect(), *m_spHighlightBackgroundPixmap);
        painter.end();

        QPainter* pPaint2 = new QPainter(m_spBackgroundPixmap);
        pPaint2->fillRect(0, 0, 12, 12, Qt::white);
    }

}

acCustomPlot::~acCustomPlot()
{

}

void acCustomPlot::dragEnterEvent(QDragEnterEvent* pEvent)
{
    // Call the base class implementation:
    QCustomPlot::dragEnterEvent(pEvent);

    // Check if this is another custom plot:
    if (pEvent->mimeData()->hasFormat(AC_STR_CustomPlotDragString) && m_isDragAndDropEnabled)
    {
        // Accept drag & drop of other graphs:
        pEvent->acceptProposedAction();
    }
    else
    {
        pEvent->ignore();
    }

}

void acCustomPlot::dragMoveEvent(QDragMoveEvent* pEvent)
{
    // Call the base class implementation:
    QCustomPlot::dragMoveEvent(pEvent);
}

void acCustomPlot::dropEvent(QDropEvent* pEvent)
{
    // Call the base class implementation:
    QCustomPlot::dropEvent(pEvent);

    if (pEvent->mimeData()->hasFormat(AC_STR_CustomPlotDragString) && m_isDragAndDropEnabled)
    {
        const QMimeData* pMimeData = pEvent->mimeData();
        GT_IF_WITH_ASSERT(pMimeData != NULL)
        {
            QByteArray itemData = pMimeData->data(AC_STR_CustomPlotDragString);
            QDataStream dataStream(&itemData, QIODevice::ReadOnly);

            quint64 dataAsUint = 0;
            void* pPointer = NULL;
            dataStream >> dataAsUint;
            pPointer = (void*)dataAsUint;

            if (pPointer != NULL)
            {
                // Find the dragged plot:
                acCustomPlot* pDraggedPlot = (acCustomPlot*)pPointer;
                emit acCustomPlotDropManager::Instance().EmitPlotDropped(this, pDraggedPlot);
            }
        }
    }
}

void acCustomPlot::mouseReleaseEvent(QMouseEvent* pEvent)
{
    // Call the base class implementation:
    QCustomPlot::mouseReleaseEvent(pEvent);

    // Drag ended:
    m_isDragging = false;

    // Reset to default cursor when dragging is done:
    setCursor(Qt::ArrowCursor);
}
void acCustomPlot::mousePressEvent(QMouseEvent* pEvent)
{
    // Call the base class implementation:
    QCustomPlot::mousePressEvent(pEvent);

    // Drag started:
    m_isDragging = true;

    // Set a hand cursor while dragging:
    setCursor(Qt::PointingHandCursor);

    if ((pEvent != NULL) && m_isDragAndDropEnabled)
    {
        if (pEvent->button() == Qt::LeftButton)
        {
            QPoint hotSpot = pEvent->pos() - this->pos();

            QByteArray itemData;
            QDataStream dataStream(&itemData, QIODevice::WriteOnly);
            dataStream << (quint64)((void*)this);

            QMimeData* mimeData = new QMimeData;
            mimeData->setData(AC_STR_CustomPlotDragString, itemData);
            mimeData->setText("test");
            QDrag* drag = new QDrag(this);
            drag->setMimeData(mimeData);
            drag->setHotSpot(hotSpot);

            drag->exec(Qt::MoveAction | Qt::CopyAction, Qt::CopyAction);
        }
    }
}

void acCustomPlot::enterEvent(QEvent* pEvent)
{
    // Call the base class implementation:
    QCustomPlot::enterEvent(pEvent);

    // Trigger a signal for users of this class, that mouse entered the area of the plot:
    emit PlotEntered(this);
}

void acCustomPlot::leaveEvent(QEvent* pEvent)
{
    // Call the base class implementation:
    QCustomPlot::leaveEvent(pEvent);

    // Trigger a signal for users of this class, that mouse left the area of the plot:
    emit PlotLeave(this);
}

void acCustomPlot::EnableDragAndDrop(bool isEnabled)
{
    m_isDragAndDropEnabled = isEnabled;

    setAcceptDrops(m_isDragAndDropEnabled);
}

void acCustomPlot::SetHighlighted(bool isHighlighted)
{
    if (m_isHighlighted != isHighlighted)
    {
        m_isHighlighted = isHighlighted;

        QPixmap* pPixmap = m_isHighlighted ? m_spHighlightBackgroundPixmap : m_spBackgroundPixmap;
        GT_IF_WITH_ASSERT(pPixmap != nullptr)
        {
            setBackground(*pPixmap);
        }

        replot();
    }
}

