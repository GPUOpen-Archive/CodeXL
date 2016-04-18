//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afQtViewCreatorAbstract.cpp
///
//==================================================================================

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Local:
#include <AMDTApplicationFramework/Include/afQtViewCreatorAbstract.h>

// ---------------------------------------------------------------------------
// Name:        afViewCreator::afViewCreator
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        2/8/2011
// ---------------------------------------------------------------------------
afQtViewCreatorAbstract::afQtViewCreatorAbstract(void)
{
}

// ---------------------------------------------------------------------------
// Name:        afQtViewCreatorAbstract::initialize
// Description: This function should be called right after the constructor.
//              It uses amountOfViewTypes which is virtual, and cannot be called from
//              the constructor
// Author:      Sigal Algranaty
// Date:        7/8/2011
// ---------------------------------------------------------------------------
void afQtViewCreatorAbstract::initialize()
{
    // Resize the vector to max possible views
    m_viewsCreated.resize(this->amountOfViewTypes());

    // Initialize the views:
    for (int view = 0; view < (int)m_viewsCreated.size() - 1 ; view++)
    {
        m_viewsCreated[view] = nullptr;
    }
}


// ---------------------------------------------------------------------------
// Name:        afViewCreator::~afViewCreator
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        2/8/2011
// ---------------------------------------------------------------------------
afQtViewCreatorAbstract::~afQtViewCreatorAbstract(void)
{
    m_viewsCreated.clear();
}


// ---------------------------------------------------------------------------
// Name:        afViewCreator::widget
// Description: Get the specific QWidget
// Arguments:   int viewIndex
// Return Val:  QWidget*
// Author:      Sigal Algranaty
// Date:        2/8/2011
// ---------------------------------------------------------------------------
QWidget* afQtViewCreatorAbstract::widget(int viewIndex)
{
    QWidget* pRetVal = nullptr;

    // Get the amount of views created:
    int amountOfViews = amountOfCreatedViews();

    // Validate asked window is in range:
    GT_IF_WITH_ASSERT(viewIndex >= 0 && viewIndex < amountOfViews)
    {
        pRetVal = m_viewsCreated[viewIndex];
    }

    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        afQtViewCreatorAbstract::wasWidgetCreated
// Description: Return true iff the requested widget was created by me
// Arguments:   QWidget* pWidget
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        11/12/2011
// ---------------------------------------------------------------------------
bool afQtViewCreatorAbstract::wasWidgetCreated(QWidget* pWidget)
{
    bool retVal = false;

    // Go through the created widget and look for the requested one:
    for (int i = 0; i < (int)m_viewsCreated.size(); i++)
    {
        if (pWidget == m_viewsCreated[i])
        {
            retVal = true;
            break;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afWxViewCreatorAbstract::restoreMinimalSize
// Description: Removes the  wrappers minimal size
// Author:      Gilad Yarnitzky
// Date:        27/2/2012
// ---------------------------------------------------------------------------
void afQtViewCreatorAbstract::restoreMinimalSize()
{

    int amountOfViews = amountOfCreatedViews();

    for (int i = 0; i < amountOfViews; i++)
    {
        QWidget* currentWidget = widget(i);

        if (nullptr != currentWidget)
        {
            currentWidget->setMinimumSize(0, 0);
        }
    }
}
