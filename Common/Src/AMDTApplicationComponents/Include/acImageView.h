//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acImageView.h
///
//==================================================================================

//------------------------------ acImageView.h ------------------------------

#ifndef __ACIMAGEVIEW
#define __ACIMAGEVIEW

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local:
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>

class AC_API acImageView : public QGraphicsView
{
    Q_OBJECT

public:
    /// Constructor:
    acImageView(QWidget* pParent, const osFilePath& imageFilePath);

    // Destructor:
    ~acImageView();

protected:

    /// The displayed image file path
    osFilePath m_imageFilePath;

    /// The graphic scene
    QGraphicsScene* m_pGraphicScene;

    /// The graphic widget (used for layouting the graphic object)
    QGraphicsWidget* m_pGraphicWidget;

    /// The graphic layout
    QGraphicsLinearLayout* m_pGraphicLayout;
};

#endif  // __ACIMAGEVIEW
