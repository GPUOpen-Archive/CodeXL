//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acImageView.cpp
///
//==================================================================================

//------------------------------ acImageView.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acImageView.h>

#define AC_IMAGE_WIDTH 120
#define AC_IMAGE_MARGIN 15

#define AC_SELECTED_ITEM_BG_COLOR QColor(225, 225, 225, 125)
#define AC_SELECTED_ITEM_FRAME_COLOR QColor(125, 125, 125, 255)

acImageView::acImageView(QWidget* pParent, const osFilePath& imageFilePath) : QGraphicsView(pParent), m_imageFilePath(imageFilePath)
{
    m_pGraphicScene = new QGraphicsScene;

    m_pGraphicWidget = new QGraphicsWidget(nullptr);

    /*
        // Initialize the graphic layout
        m_pGraphicLayout = new QGraphicsLinearLayout;
        m_pGraphicLayout->setColumnAlignment(0, Qt::AlignLeft);
        m_pGraphicLayout->setColumnAlignment(1, Qt::AlignLeft);
        m_pGraphicLayout->setColumnMinimumWidth(0, AC_IMAGE_WIDTH + AC_IMAGE_MARGIN);
        m_pGraphicLayout->setColumnMinimumWidth(1, AC_IMAGE_WIDTH + AC_IMAGE_MARGIN);
        m_pGraphicLayout->setContentsMargins(0, 0, 0, 0);
        m_pGraphicLayout->setSpacing(0);
        m_pGraphicLayout->setVerticalSpacing(0);
        m_pGraphicLayout->setHorizontalSpacing(0);

        m_pGraphicWidget->setLayout(m_pGraphicLayout);
    */
    m_pGraphicScene->addItem(m_pGraphicWidget);

    setScene(m_pGraphicScene);

    setMinimumWidth(AC_IMAGE_WIDTH * 2 + 4 * AC_IMAGE_MARGIN);

    setAlignment(Qt::AlignTop | Qt::AlignLeft);

    m_pGraphicScene->setSceneRect(rect());
    setScene(m_pGraphicScene);
    QPixmap pix(acGTStringToQString(m_imageFilePath.asString()));
    m_pGraphicScene->addPixmap(pix);
}

acImageView::~acImageView()
{

}

