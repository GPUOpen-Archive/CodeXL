//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLDisplayList.cpp
///
//==================================================================================

// -----------------------------   apGLDisplayList.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <AMDTAPIClasses/Include/apGLDisplayList.h>

// ---------------------------------------------------------------------------
// Name:        apGLDisplayList::apGLDisplayList
// Description: Constructor
// Arguments:   displayListName - The OpenGL display list object name.
// Author:  AMD Developer Tools Team
// Date:        23/9/2008
// ---------------------------------------------------------------------------
apGLDisplayList::apGLDisplayList(GLuint displayListName, GLenum displayListMode)
    : apAllocatedObject(), _displayListName(displayListName), _displayListMode(displayListMode), _geometrySize(0), _amountOfVertices(0)
{
}


// ---------------------------------------------------------------------------
// Name:        apGLDisplayList::apGLDisplayList
// Description: Copy constructor
// Arguments: other - The other display list class from which I am copied.
// Author:  AMD Developer Tools Team
// Date:        23/9/2008
// ---------------------------------------------------------------------------
apGLDisplayList::apGLDisplayList(const apGLDisplayList& other)
{
    apGLDisplayList::operator=(other);
}


// ---------------------------------------------------------------------------
// Name:        apGLDisplayList::~apGLDisplayList
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        23/9/2008
// ---------------------------------------------------------------------------
apGLDisplayList::~apGLDisplayList()
{
}

// ---------------------------------------------------------------------------
// Name:        apGLDisplayList::operator=
// Description: Assignment operator
// Arguments:   other - The other object from which I am copied.
// Author:  AMD Developer Tools Team
// Date:        23/9/2008
// ---------------------------------------------------------------------------
apGLDisplayList& apGLDisplayList::operator=(const apGLDisplayList& other)
{
    _displayListName = other._displayListName;
    _displayListMode = other._displayListMode;
    _geometrySize = other._geometrySize;
    _amountOfVertices = other._amountOfVertices;

    // Copy the vectors:
    _calledDisplayListsVector.clear();
    const gtVector<GLuint>& otherCalledLists = other._calledDisplayListsVector;
    int numberOfCalledLists = (int)otherCalledLists.size();

    for (int i = 0; i < numberOfCalledLists; i++)
    {
        const GLuint& otherCalledList = otherCalledLists[i];
        _calledDisplayListsVector.push_back(otherCalledList);
    }

    _callingDisplayLists.clear();
    const gtVector<GLuint>& otherCallingLists = other._callingDisplayLists;
    int numberOfCallingLists = (int)otherCallingLists.size();

    for (int i = 0; i < numberOfCallingLists; i++)
    {
        const GLuint& otherCallingList = otherCallingLists[i];
        _callingDisplayLists.push_back(otherCallingList);
    }

    setAllocatedObjectId(other.getAllocatedObjectId(), true);

    return *this;
}

// ---------------------------------------------------------------------------
// Name:        apGLDisplayList::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        23/9/2008
// ---------------------------------------------------------------------------
osTransferableObjectType apGLDisplayList::type() const
{
    return OS_TOBJ_ID_DISPLAY_LIST;
}

// ---------------------------------------------------------------------------
// Name:        apGLDisplayList::writeSelfIntoChannel
// Description: Writes this class into a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        23/9/2008
// ---------------------------------------------------------------------------
bool apGLDisplayList::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    // Write the display list attributes into the channel:
    ipcChannel << (gtUInt32)_displayListName;
    ipcChannel << (gtInt32)_displayListMode;
    ipcChannel << _geometrySize;
    ipcChannel << _amountOfVertices;

    apAllocatedObject::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLDisplayList::readSelfFromChannel
// Description: Read self from a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        23/9/2008
// ---------------------------------------------------------------------------
bool apGLDisplayList::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    // Read the display list attributes into the channel:
    gtUInt32 displayListNameAsUInt32 = 0;
    ipcChannel >> displayListNameAsUInt32;
    _displayListName = (GLuint)displayListNameAsUInt32;

    gtInt32 displayListModeAsInt32 = 0;
    ipcChannel >> displayListModeAsInt32;
    _displayListMode = (GLenum)displayListModeAsInt32;

    ipcChannel >> _geometrySize;
    ipcChannel >> _amountOfVertices;

    apAllocatedObject::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLDisplayList::copyListDataOnCompilation
// Description: Copies the data from the new list, but leaves the allocated
//              object ID and the calling lists vector. This should be used
//              when an existing list (i.e. already allocated and possibly
//              called by other lists) is re-compiled.
// Author:  AMD Developer Tools Team
// Date:        10/1/2010
// ---------------------------------------------------------------------------
void apGLDisplayList::copyListDataOnCompilation(const apGLDisplayList& newListDetail)
{
    // When recompiling, we are not supposed to remove the old list (ie change the item's name
    GT_ASSERT(_displayListName == newListDetail._displayListName);

    _displayListName = newListDetail._displayListName;
    _displayListMode = newListDetail._displayListMode;
    _geometrySize = newListDetail._geometrySize;
    _amountOfVertices = newListDetail._amountOfVertices;

    // Copy the CALLED lists vector, but not the CALLING lists vector:
    _calledDisplayListsVector.clear();
    const gtVector<GLuint>& otherCalledLists = newListDetail._calledDisplayListsVector;
    int numberOfCalledLists = (int)otherCalledLists.size();

    for (int i = 0; i < numberOfCalledLists; i++)
    {
        const GLuint& otherCalledList = otherCalledLists[i];
        _calledDisplayListsVector.push_back(otherCalledList);
    }

    // Do not copy the allocated object ID as the object was re-compiled, not re-allocated.
}

// ---------------------------------------------------------------------------
// Name:        apGLDisplayList::addCallingDisplayList
// Description: Adds a display list to the "calling display lists" member
// Author:  AMD Developer Tools Team
// Date:        10/1/2010
// ---------------------------------------------------------------------------
void apGLDisplayList::addCallingDisplayList(GLuint displayList)
{
    // Try to find the display list in our struct:
    gtVector<GLuint>::iterator beginIter = _callingDisplayLists.begin();
    gtVector<GLuint>::iterator endIter = _callingDisplayLists.end();
    gtVector<GLuint>::iterator findIter = gtFind(beginIter, endIter, displayList);

    // If the display list is not yet logged
    if (findIter == endIter)
    {
        // TO_DO: Uri, 10/01/10 - This should probably be done in a more efficient manner, probably with
        // finding the spot (using the search) and inserting the member there so the vector will be sorted,
        // allowing for faster search.
        // However, since we have not yet decided if we should use gtVector as the implementation, this
        // implementation should do for now:
        _callingDisplayLists.push_back(displayList);

        // Update the iterators and use them to sort:
        beginIter = _callingDisplayLists.begin();
        endIter = _callingDisplayLists.end();
        gtSort(beginIter, endIter);
    }
}

// ---------------------------------------------------------------------------
// Name:        apGLDisplayList::removeCallingDisplayList
// Description: Removes a display list to the "calling display lists" member
// Author:  AMD Developer Tools Team
// Date:        10/1/2010
// ---------------------------------------------------------------------------
void apGLDisplayList::removeCallingDisplayList(GLuint displayList)
{
    // TO_DO: Uri, 10/01/10 - gtVector is not very efficient for item removal, this might not be the best implementation:
    bool foundDisplayList = false;
    int vecSize = (int)_callingDisplayLists.size();

    for (int i = 0; i < vecSize; i++)
    {
        const GLuint& currentItem = _callingDisplayLists[i];

        if (foundDisplayList)
        {
            // Note: since we already found the item we were searching for, i has to be > 0, so we can access i - 1:
            _callingDisplayLists[i - 1] = currentItem;
        }
        else
        {
            // Check if the current item is the list we are looking for:
            foundDisplayList = (displayList == currentItem);
        }
    }

    GT_IF_WITH_ASSERT(foundDisplayList)
    {
        // If the found item was last, remove it.
        // If the found item had another index, we moved back all the others, so the last item
        // should be duplicate, so remove it:
        _callingDisplayLists.pop_back();
    }
}

