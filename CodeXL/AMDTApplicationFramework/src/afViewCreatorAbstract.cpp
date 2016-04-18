//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afViewCreatorAbstract.cpp
///
//==================================================================================

//Qt:
#include <QtWidgets>

// Local:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afViewCreatorAbstract.h>
#include <AMDTApplicationFramework/Include/afActionCreatorAbstract.h>



// ---------------------------------------------------------------------------
// Name:        afViewCreator::afViewCreator
// Description: Constructor
// Author:      Gilad Yarnitzky
// Date:        14/7/2011
// ---------------------------------------------------------------------------
afViewCreatorAbstract::afViewCreatorAbstract() : _pCreationEvent(nullptr), _pViewActionCreator(nullptr)
{
}

// ---------------------------------------------------------------------------
// Name:        afViewCreator::~afViewCreator
// Description: Destructor
// Author:      Gilad Yarnitzky
// Date:        14/7/2011
// ---------------------------------------------------------------------------
afViewCreatorAbstract::~afViewCreatorAbstract()
{
    if (_pCreationEvent != nullptr)
    {
        delete _pCreationEvent;
    }
}


// ---------------------------------------------------------------------------
// Name:        afViewCreatorAbstract::initialize
// Description: Initializes the creator (perform initialization actions that
//              required virtual functionality
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        16/8/2011
// ---------------------------------------------------------------------------
void afViewCreatorAbstract::initialize()
{
    // Initialize the view icons:
    initViewsIcons();
}


// ---------------------------------------------------------------------------
// Name:        afActionCreatorAbstract::iconAsPixmap
// Description: Get the icon file name
// Arguments:   int actionIndex
// Author:      Sigal Algranaty
// Date:        14/8/2011
// ---------------------------------------------------------------------------
QPixmap* afViewCreatorAbstract::iconAsPixmap(int viewIndex)
{
    QPixmap* pRetVal = nullptr;

    if ((viewIndex < (int)_iconsDataVector.size()) && (viewIndex >= 0))
    {
        // If the icon is initialized:
        if (_iconsDataVector[viewIndex]._isPixmapInitialized)
        {
            pRetVal = _iconsDataVector[viewIndex]._pCommandPixmap;
        }
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        afActionCreatorAbstract::afCommandIconData::afCommandIconData
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        14/8/2011
// ---------------------------------------------------------------------------
afViewCreatorAbstract::afViewIconData::afViewIconData()
    : _pCommandPixmap(nullptr), _isPixmapInitialized(false)
{
}

// ---------------------------------------------------------------------------
// Name:        afActionCreatorAbstract::initViewsIcons
// Description: Default implementation
// Author:      Sigal Algranaty
// Date:        14/8/2011
// ---------------------------------------------------------------------------
void afViewCreatorAbstract::initViewsIcons()
{
    _iconsDataVector.resize(amountOfViewTypes());
}

// ---------------------------------------------------------------------------
// Name:        afActionCreatorAbstract::initSingleViewIcon
// Description: Initializes a single view icon
// Arguments:   actionIndex - the view index
//              xpm[] - the action xpm string
// Author:      Sigal Algranaty
// Date:        16/8/2011
// ---------------------------------------------------------------------------
void afViewCreatorAbstract::initSingleViewIcon(int viewIndex, acIconId iconId)
{
    if (_iconsDataVector.size() == 0)
    {
        initViewsIcons();
    }

    // Sanity check:
    GT_IF_WITH_ASSERT((viewIndex < (int)_iconsDataVector.size()) && (viewIndex >= 0))
    {
        GT_IF_WITH_ASSERT(AC_NUMBER_OF_ICON_IDS > iconId)
        {
            QPixmap* pPixmap = new QPixmap;
            acSetIconInPixmap(*pPixmap, iconId);
            _iconsDataVector[viewIndex]._pCommandPixmap = pPixmap;

            _iconsDataVector[viewIndex]._isPixmapInitialized = true;
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        afViewCreatorAbstract::CreatedViewsTypeID
// Description: An MDI views creator, which is supposed to handle a group of views,
//              should implement this function, and return a string, which is similar to what
//              set in
// Return Val:  apEvent::EventType
// Author:      Sigal Algranaty
// Date:        22/8/2011
// ---------------------------------------------------------------------------
const gtString afViewCreatorAbstract::CreatedMDIType() const
{
    return AF_STR_Empty;
}


// ---------------------------------------------------------------------------
// Name:        afViewCreatorAbstract::onMDISubWindowClose
// Description: Default implementation.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        23/8/2011
// ---------------------------------------------------------------------------
bool afViewCreatorAbstract::onMDISubWindowClose(afQMdiSubWindow* pMDISubWindow)
{
    GT_UNREFERENCED_PARAMETER(pMDISubWindow);

    return true;
}

// ---------------------------------------------------------------------------
// Name:        afViewCreatorAbstract::supportedEditCommandIds
// Description: Accessor for the supported edit commands, stored in the actions creator.
// Return Val:  Vector of supported edit command IDs.
// Author:      Doron Ofek
// Date:        Mar-3, 2015
// ---------------------------------------------------------------------------
const gtVector<int>& afViewCreatorAbstract::supportedCommandIds() const
{
    return actionCreator()->supportedCommandIds();
}

// ---------------------------------------------------------------------------
// Name:        afViewCreatorAbstract::actionIndexToCommandId
// Description: Convert the action index to the corresponding command id.
//              The actual conversion is performed by the actions creator
// Return Val:  command ID.
// Author:      Doron Ofek
// Date:        Mar-3, 2015
// ---------------------------------------------------------------------------
int afViewCreatorAbstract::actionIndexToCommandId(const int actionIndex) const
{
    return actionCreator()->actionIndexToCommandId(actionIndex);
}