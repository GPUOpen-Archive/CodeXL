//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsDisplayListMonitor.cpp
///
//==================================================================================

//------------------------------ gsDisplayListMonitor.cpp ------------------------------


// Standard C:
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Infra:
#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTAPIClasses/Include/apDefaultTextureNames.h>
#include <AMDTAPIClasses/Include/apDefinitions.h>
#include <AMDTAPIClasses/Include/ap2DRectangle.h>

// Spies Utilities:
#include <AMDTServerUtilities/Include/suGlobalVariables.h>

// Local:
#include <src/gsDisplayListMonitor.h>
#include <src/gsExtensionsManager.h>
#include <src/gsMonitoredFunctionPointers.h>
#include <src/gsOpenGLMonitor.h>
#include <src/gsRenderContextMonitor.h>
#include <src/gsRenderPrimitivesStatisticsLogger.h>
#include <src/gsStringConstants.h>
#include <src/gsWrappersCommon.h>

// ---------------------------------------------------------------------------
// Name:        gsDisplayListMonitor::gsDisplayListMonitor
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        25/5/2008
// ---------------------------------------------------------------------------
gsDisplayListMonitor::gsDisplayListMonitor(gsRenderContextMonitor* pRenderContextMonitor):
    _pCurrentlyActiveDisplayList(NULL), _pCurrentlyActiveDisplayListGeometryMapping(NULL), _pRenderContextMonitor(pRenderContextMonitor)
{
}

// ---------------------------------------------------------------------------
// Name:        gsDisplayListMonitor::~gsDisplayListMonitor
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        25/5/2008
// ---------------------------------------------------------------------------
gsDisplayListMonitor::~gsDisplayListMonitor()
{
    // Release the display list objects:
    _displayLists.deleteElementsAndClear();

    // Release the primitives amount memory:
    int numberOfPrimitiveMappings = (int)_displayListPrimitivesAmount.size();

    for (int i = 0; i < numberOfPrimitiveMappings; i++)
    {
        delete[] _displayListPrimitivesAmount[i];
    }

    _displayListPrimitivesAmount.clear();

    int numberOfTotalGeometryMappings = (int)_displayListTotalGeometryMapping.size();

    for (int i = 0; i < numberOfTotalGeometryMappings; i++)
    {
        delete[] _displayListTotalGeometryMapping[i];
    }

    _displayListTotalGeometryMapping.clear();

    // If we have a current list object, destroy it:
    delete _pCurrentlyActiveDisplayList;
    _pCurrentlyActiveDisplayList = NULL;

    delete[] _pCurrentlyActiveDisplayListGeometryMapping;
    _pCurrentlyActiveDisplayListGeometryMapping = NULL;
}

// ---------------------------------------------------------------------------
// Name:        gsDisplayListMonitor::addNewDisplayList
// Description: Adds a new display list to our vector
// Arguments: GLuint displayListName
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        24/2/2009
// ---------------------------------------------------------------------------
bool gsDisplayListMonitor::addNewDisplayList(GLuint displayListName)
{
    bool retVal = true;

    // Construct new object:
    apGLDisplayList* pDisplayList = new apGLDisplayList(displayListName, 0);

    // Register this object in the allocated objects monitor:
    su_stat_theAllocatedObjectsMonitor.registerAllocatedObject(*pDisplayList);

    // Add the display list object to the vector of display lists:
    int newListIndex = (int)_displayLists.size();
    _displayLists.push_back(pDisplayList);

    // Allocate a render primitive mapping:
    int* pDisplayListGeometryMapping = new int[GS_AMOUNT_OF_PRIMITIVE_TYPES];
    int* pDisplayListTotalGeometryMapping = new int[GS_AMOUNT_OF_PRIMITIVE_TYPES];

    // Reset the memory:
    ::memset(pDisplayListGeometryMapping, 0, sizeof(int) * GS_AMOUNT_OF_PRIMITIVE_TYPES);
    ::memset(pDisplayListTotalGeometryMapping, 0, sizeof(int) * GS_AMOUNT_OF_PRIMITIVE_TYPES);

    // Add the geometry count to the mapping:
    int newGeometryMappingIndex = (int)_displayListPrimitivesAmount.size();
    GT_ASSERT(newGeometryMappingIndex == newListIndex);
    _displayListPrimitivesAmount.push_back(pDisplayListGeometryMapping);

    int newTotalGeometryMappingIndex = (int)_displayListTotalGeometryMapping.size();
    GT_ASSERT(newTotalGeometryMappingIndex == newListIndex);
    _displayListTotalGeometryMapping.push_back(pDisplayListTotalGeometryMapping);

    // Register the vectors index in the map:
    _displayListNameToVectorIndex[displayListName] = newListIndex;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsDisplayListMonitor::addNewDisplayList
// Description: Adds a range of new display lists to our vector
// Arguments: GLuint displayListName
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        24/2/2009
// ---------------------------------------------------------------------------
bool gsDisplayListMonitor::addNewDisplayLists(GLuint firstDisplayListName, GLsizei displayListsRange)
{
    bool retVal = true;

    gtVector<apAllocatedObject*> listsForAllocationMonitor;

    for (GLuint displayListName = firstDisplayListName; displayListName < firstDisplayListName + displayListsRange; displayListName++)
    {
        // Construct new object:
        apGLDisplayList* pDisplayList = new apGLDisplayList(displayListName, 0);
        listsForAllocationMonitor.push_back(pDisplayList);

        // Add the display list object to the vector of display lists:
        int newListIndex = (int)_displayLists.size();
        _displayLists.push_back(pDisplayList);

        // Allocate a render primitive mapping:
        int* pDisplayListGeometryMapping = new int[GS_AMOUNT_OF_PRIMITIVE_TYPES];
        int* pDisplayListTotalGeometryMapping = new int[GS_AMOUNT_OF_PRIMITIVE_TYPES];

        // Reset the memory:
        ::memset(pDisplayListGeometryMapping, 0, sizeof(int) * GS_AMOUNT_OF_PRIMITIVE_TYPES);
        ::memset(pDisplayListTotalGeometryMapping, 0, sizeof(int) * GS_AMOUNT_OF_PRIMITIVE_TYPES);

        // Add the geometry count to the mapping:
        int newGeometryMappingIndex = (int)_displayListPrimitivesAmount.size();
        GT_ASSERT(newGeometryMappingIndex == newListIndex);
        _displayListPrimitivesAmount.push_back(pDisplayListGeometryMapping);

        int newTotalGeometryMappingIndex = (int)_displayListTotalGeometryMapping.size();
        GT_ASSERT(newTotalGeometryMappingIndex == newListIndex);
        _displayListTotalGeometryMapping.push_back(pDisplayListTotalGeometryMapping);

        // Register the vectors index in the map:
        _displayListNameToVectorIndex[displayListName] = newListIndex;
    }

    // Register this object in the allocated objects monitor:
    su_stat_theAllocatedObjectsMonitor.registerAllocatedObjects(listsForAllocationMonitor);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsDisplayListMonitor::onNewList
// Description: Called when the glNewList function is called by the user, creating
//              a new list or replacing an old one.
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        23/9/2008
// ---------------------------------------------------------------------------
bool gsDisplayListMonitor::onNewList(GLuint displayListName, GLenum displayListMode)
{
    bool retVal = false;

    // The old list exists, create a new object for the active list:
    _pCurrentlyActiveDisplayList = new apGLDisplayList(displayListName, displayListMode);

    _pCurrentlyActiveDisplayListGeometryMapping = new int[GS_AMOUNT_OF_PRIMITIVE_TYPES];

    ::memset(_pCurrentlyActiveDisplayListGeometryMapping, 0, sizeof(int) * GS_AMOUNT_OF_PRIMITIVE_TYPES);

    // If this is a new list:
    if (!displayListNameExists(displayListName))
    {
        // Register this object in the allocated objects monitor (we will add it to our vectors
        // when glEndList is called):
        su_stat_theAllocatedObjectsMonitor.registerAllocatedObject(*_pCurrentlyActiveDisplayList);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsDisplayListMonitor::removeDisplayList
// Description: Destroys the apGLDisplayList object named by displayListName
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        23/9/2008
// ---------------------------------------------------------------------------
bool gsDisplayListMonitor::removeDisplayList(GLuint displayListName)
{
    bool retVal = true;

    int foundIndex = getDisplayListIndex(displayListName);

    if (foundIndex > -1)
    {
        // Delete the object:
        apGLDisplayList* pDeletedList = _displayLists[foundIndex];
        delete pDeletedList;
        int* pDeletedListGeometryMapping = _displayListPrimitivesAmount[foundIndex];
        delete[] pDeletedListGeometryMapping;
        int* pDeletedListTotalGeometryMapping = _displayListTotalGeometryMapping[foundIndex];
        delete[] pDeletedListTotalGeometryMapping;

        // Mark the object as deleted in the map:
        _displayListNameToVectorIndex[displayListName] = -1;

        // shift back the indices
        int n = (int)_displayLists.size();
        int nn = (int)_displayListPrimitivesAmount.size();
        int nnn = (int)_displayListTotalGeometryMapping.size();
        GT_ASSERT(n == nn);
        GT_ASSERT(n == nnn);

        for (int j = foundIndex; j < (n - 1); j++)
        {
            // Update the map:
            apGLDisplayList* pCurrentList = _displayLists[(j + 1)];
            GT_IF_WITH_ASSERT(pCurrentList != NULL)
            {
                GLuint listName = pCurrentList->getDisplayListName();
                _displayListNameToVectorIndex[listName] = j;
            }

            // Move the data back an index:
            _displayLists[j] = pCurrentList;
            _displayListPrimitivesAmount[j] = _displayListPrimitivesAmount[(j + 1)];
            _displayListTotalGeometryMapping[j] = _displayListTotalGeometryMapping[(j + 1)];
        }

        _displayLists.pop_back();
        _displayListPrimitivesAmount.pop_back();
        _displayListTotalGeometryMapping.pop_back();
    }
    else // foundIndex <= -1
    {
        gtString errorMessage;
        errorMessage.appendFormattedString(L"Attempted to remove display list %u, but it was not found", displayListName);
        GT_ASSERT_EX(foundIndex > -1, errorMessage.asCharArray());
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsDisplayListMonitor::removeDisplayLists
// Description: spy-side implementation of glDeleteLists
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        4/12/2008
// ---------------------------------------------------------------------------
bool gsDisplayListMonitor::removeDisplayLists(GLuint displayListName, GLsizei displayListsRange)
{
    bool retVal = true;

    // Remove each of the display lists:
    for (GLuint i = displayListName; i < (displayListName + displayListsRange); i++)
    {
        bool rc = removeDisplayList(i);

        if (!rc)
        {
            gtString failedMessage;
            failedMessage.appendFormattedString(L"Failed to remove display list %u (deleting %d lists, starting with %u)", i, displayListsRange, displayListName);
            GT_ASSERT_EX(rc, failedMessage.asCharArray());
            retVal = false;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsDisplayListMonitor::getDisplayListDetails
// Description: Returns a display list object details by name
// Author:      Sigal Algranaty
// Date:        24/9/2008
// ---------------------------------------------------------------------------
const apGLDisplayList* gsDisplayListMonitor::getDisplayListDetails(GLuint displayListName) const
{
    const apGLDisplayList* pRetDisplayList = NULL;

    // Get the list's index in the vector:
    int foundIndex = getDisplayListIndex(displayListName);

    // -1 Means this list was deleted:
    if (foundIndex >= 0)
    {
        // Get the object for this index:
        const apGLDisplayList* pFoundDisplayList = _displayLists[foundIndex];
        GT_IF_WITH_ASSERT(pFoundDisplayList != NULL)
        {
            // Verify it has the correct name:
            GT_IF_WITH_ASSERT(pFoundDisplayList->getDisplayListName() == displayListName)
            {
                pRetDisplayList = pFoundDisplayList;
            }
        }
    }

    return pRetDisplayList;
}

// ---------------------------------------------------------------------------
// Name:        gsDisplayListMonitor::getDisplayListDetails
// Description: Returns a mutable display list object details by name.
//              Note this function is private - to change our monitored items, get
//              the accessors.
// Author:      Uri Shomroni
// Date:        10/01/2010
// ---------------------------------------------------------------------------
apGLDisplayList* gsDisplayListMonitor::getDisplayListDetails(GLuint displayListName)
{
    apGLDisplayList* pRetDisplayList = NULL;

    // Get the list's index in the vector:
    int foundIndex = getDisplayListIndex(displayListName);

    // -1 Means this list was deleted:
    if (foundIndex >= 0)
    {
        // Get the object for this index:
        apGLDisplayList* pFoundDisplayList = _displayLists[foundIndex];
        GT_IF_WITH_ASSERT(pFoundDisplayList != NULL)
        {
            // Verify it has the correct name:
            GT_IF_WITH_ASSERT(pFoundDisplayList->getDisplayListName() == displayListName)
            {
                pRetDisplayList = pFoundDisplayList;
            }
        }
    }

    return pRetDisplayList;
}
// ---------------------------------------------------------------------------
// Name:        gsDisplayListMonitor::getDisplayListName
// Description: Gets the OpenGL name of a display list, by its internal index
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        17/11/2008
// ---------------------------------------------------------------------------
bool gsDisplayListMonitor::getDisplayListName(int displayListIndex, GLuint& displayListName) const
{
    bool retVal = false;

    GT_IF_WITH_ASSERT((displayListIndex >= 0) && (displayListIndex < (int)_displayLists.size()))
    {
        const apGLDisplayList* pDisplayListDetails = _displayLists[displayListIndex];
        GT_IF_WITH_ASSERT(pDisplayListDetails != NULL)
        {
            displayListName = pDisplayListDetails->getDisplayListName();
            retVal = true;
        }
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsDisplayListMonitor::endDisplayList
// Description: End display list creation
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        27/5/2009
// ---------------------------------------------------------------------------
bool gsDisplayListMonitor::endDisplayList()
{
    bool retVal = true;

    GT_IF_WITH_ASSERT(_pCurrentlyActiveDisplayList != NULL)
    {
        // Get the Name of the currently active List:
        GLuint listName = _pCurrentlyActiveDisplayList->getDisplayListName();
        apGLDisplayList* pNewListDetails = NULL;

        // Check if the list already exists:
        int foundIndex = getDisplayListIndex(listName);

        if (foundIndex > -1)
        {
            // The list already exists, update its data:
            apGLDisplayList* pReplacedDisplayList = _displayLists[foundIndex];
            GT_IF_WITH_ASSERT(pReplacedDisplayList != NULL)
            {
                // Notify the lists we are dependent on that we are no longer dependent on them:
                const gtVector<GLuint>& calledListsVector = pReplacedDisplayList->getCalledDisplayLists();
                int numberOfCalledLists = (int)calledListsVector.size();

                for (int i = 0; i < numberOfCalledLists; i++)
                {
                    // Get the index of the called list (we allow deleted lists here since there can be a recompilation of more than one list at once):
                    int calledListVectorsIndex = getDisplayListIndex(calledListsVector[i]);

                    if (calledListVectorsIndex > -1)
                    {
                        apGLDisplayList* pCalledDisplayList = _displayLists[calledListVectorsIndex];
                        GT_IF_WITH_ASSERT(pCalledDisplayList != NULL)
                        {
                            // Tell this list we no longer call it:
                            pCalledDisplayList->removeCallingDisplayList(listName);
                        }
                    }
                }

                // Copy only the relevant details:
                pReplacedDisplayList->copyListDataOnCompilation(*_pCurrentlyActiveDisplayList);

                // Delete the data struct:
                delete _pCurrentlyActiveDisplayList;
                _pCurrentlyActiveDisplayList = NULL;

                // Note that we are updating the existing list:
                pNewListDetails = pReplacedDisplayList;
            }
            else
            {
                // We couldn't find the list, so store the new list in the same place:
                _displayLists[foundIndex] = _pCurrentlyActiveDisplayList;

                // We will use the data we already constructed:
                pNewListDetails = _pCurrentlyActiveDisplayList;
                _pCurrentlyActiveDisplayList = NULL;
            }

            // Copy the primitives data:
            for (int i = 0; i < GS_AMOUNT_OF_PRIMITIVE_TYPES; i++)
            {
                _displayListPrimitivesAmount[foundIndex][i] = _pCurrentlyActiveDisplayListGeometryMapping[i];
            }

            // Delete the geometry mapping:
            delete[] _pCurrentlyActiveDisplayListGeometryMapping;
            _pCurrentlyActiveDisplayListGeometryMapping = NULL;
        }
        else // foundIndex <= -1
        {
            // This list is new (name was generated with glNewList), so add it to the vector:
            int newListIndex = (int)_displayLists.size();
            _displayLists.push_back(_pCurrentlyActiveDisplayList);

            int newGeometryMappingIndex = (int)_displayListPrimitivesAmount.size();
            _displayListPrimitivesAmount.push_back(_pCurrentlyActiveDisplayListGeometryMapping);
            GT_ASSERT(newListIndex == newGeometryMappingIndex);

            // Create the total geometry mapping for this list:
            int* pDisplayListTotalGeometryMapping = new int[GS_AMOUNT_OF_PRIMITIVE_TYPES];
            ::memset(pDisplayListTotalGeometryMapping, 0, sizeof(int) * GS_AMOUNT_OF_PRIMITIVE_TYPES);

            int newTotalGeometryMappingIndex = (int)_displayListTotalGeometryMapping.size();
            GT_ASSERT(newTotalGeometryMappingIndex == newListIndex);
            _displayListTotalGeometryMapping.push_back(pDisplayListTotalGeometryMapping);

            // We will use the data for the new list:
            pNewListDetails = _pCurrentlyActiveDisplayList;

            // Clear the members:
            _pCurrentlyActiveDisplayList = NULL;
            _pCurrentlyActiveDisplayListGeometryMapping = NULL;
        }

        // Make sure we cleared both members:
        GT_ASSERT(_pCurrentlyActiveDisplayList == NULL);
        GT_ASSERT(_pCurrentlyActiveDisplayListGeometryMapping == NULL);

        // Update the new list's dependencies:
        GT_IF_WITH_ASSERT(pNewListDetails != NULL)
        {
            // Iterate the new list's called lists, and let them know that this list calls them:
            const gtVector<GLuint>& calledListsVector = pNewListDetails->getCalledDisplayLists();
            int numberOfCalledLists = (int)calledListsVector.size();

            for (int i = 0; i < numberOfCalledLists; i++)
            {
                // Get the index of the called list (we allow deleted lists here since there can be a recompilation of more than one list at once):
                int calledListVectorsIndex = getDisplayListIndex(calledListsVector[i]);

                if (calledListVectorsIndex > -1)
                {
                    apGLDisplayList* pCalledDisplayList = _displayLists[calledListVectorsIndex];
                    GT_IF_WITH_ASSERT(pCalledDisplayList != NULL)
                    {
                        // Add the active display list to the list of calling display lists:
                        pCalledDisplayList->addCallingDisplayList(listName);
                    }
                }
            }
        }

        // Update the total geometry vectors:
        updateTotalGeometryCountersOnEndList(listName);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsDisplayListMonitor::callDisplayList
// Description: Execute a display list
// Arguments: GLuint displayListName
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        27/5/2009
// ---------------------------------------------------------------------------
bool gsDisplayListMonitor::callDisplayList(GLuint displayListName)
{
    bool retVal = true;

    // Add the called display list to the currently active display list (if there is):
    if (_pCurrentlyActiveDisplayList != NULL)
    {
        // Add the display list to the list of called display list of the active list:
        _pCurrentlyActiveDisplayList->addCalledDisplayList(displayListName);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsDisplayListMonitor::callDisplayLists
// Description: Call a range of display lists
// Arguments: GLsizei amountOfDispLists
//            GLenum type
//            const GLvoid pListNames
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        27/5/2009
// ---------------------------------------------------------------------------
bool gsDisplayListMonitor::callDisplayLists(GLsizei amountOfDispLists, GLenum type, const GLvoid* pListNames)
{
    bool retVal = true;

    switch (type)
    {
        case GL_BYTE:
        {
            const GLbyte* pByteListNames = (GLbyte*)pListNames;

            for (int i = 0; i < amountOfDispLists; i++)
            {
                const GLbyte& displayListName = pByteListNames[i];

                // Add the display list to the list of called display list of the active list:
                retVal = callDisplayList((GLuint)displayListName) && retVal;
            }

            break;
        }

        case GL_UNSIGNED_BYTE:
        {
            GLubyte* pUbyteListNames = (GLubyte*)pListNames;

            for (int i = 0; i < amountOfDispLists; i++)
            {
                const GLubyte& displayListName = pUbyteListNames[i];

                // Add the display list to the list of called display list of the active list:
                retVal = callDisplayList((GLuint)displayListName) && retVal;
            }
        }
        break;

        case GL_SHORT:
        {
            GLshort* pShortListNames = (GLshort*)pListNames;

            for (int i = 0; i < amountOfDispLists; i++)
            {
                const GLshort& displayListName = pShortListNames[i];

                // Add the display list to the list of called display list of the active list:
                retVal = callDisplayList((GLuint)displayListName) && retVal;
            }
        }
        break;

        case GL_UNSIGNED_SHORT:
        {
            GLushort* pUshortListNames = (GLushort*)pListNames;

            for (int i = 0; i < amountOfDispLists; i++)
            {
                const GLushort& displayListName = pUshortListNames[i];

                // Add the display list to the list of called display list of the active list:
                retVal = callDisplayList((GLuint)displayListName) && retVal;
            }
        }
        break;

        case GL_INT:
        {
            GLint* pIntListNames = (GLint*)pListNames;

            for (int i = 0; i < amountOfDispLists; i++)
            {
                const GLint& displayListName = pIntListNames[i];

                // Add the display list to the list of called display list of the active list:
                retVal = callDisplayList((GLuint)displayListName) && retVal;
            }
        }
        break;

        case GL_UNSIGNED_INT:
        {
            GLuint* pUintListNames = (GLuint*)pListNames;

            for (int i = 0; i < amountOfDispLists; i++)
            {
                const GLuint& displayListName = pUintListNames[i];

                // Add the display list to the list of called display list of the active list:
                retVal = callDisplayList((GLuint)displayListName) && retVal;
            }
        }
        break;

        default:
        {
            GT_ASSERT_EX(false, L"Unknown list index type");
            retVal = false;
        }
        break;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsDisplayListMonitor::updateActiveDisplayListGeometryMapping
// Description: Updates the display list to render primitive count mapping
// Arguments: gsRenderPrimitiveType primitiveTypeToUpdate
//            int amountOfPrimitivesToAdd
// Return Val: void
// Author:      Sigal Algranaty
// Date:        6/7/2009
// ---------------------------------------------------------------------------
void gsDisplayListMonitor::updateActiveDisplayListGeometryMapping(gsRenderPrimitiveType primitiveTypeToUpdate, int amountOfVertices, int amountOfPrimitivesToAdd)
{
    if (_pCurrentlyActiveDisplayList != NULL)
    {
        GT_IF_WITH_ASSERT(_pCurrentlyActiveDisplayListGeometryMapping != NULL)
        {
            _pCurrentlyActiveDisplayListGeometryMapping[GS_VERTICES] += amountOfVertices;
            _pCurrentlyActiveDisplayListGeometryMapping[primitiveTypeToUpdate] += amountOfPrimitivesToAdd;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsDisplayListMonitor::isGeometryDrawnOutsideCurrentList
// Description: Returns true iff there is an active display list, and its mode
//              is GL_COMPILE.
//              If there is no active list, the geometry is drawn to the framebuffer.
//              If the active list is in GL_COMPILE_AND_EXECUTE mode, the geometry
//              is written to the framebuffer as well as the list.
// Author:      Uri Shomroni
// Date:        10/1/2010
// ---------------------------------------------------------------------------
bool gsDisplayListMonitor::isGeometryDrawnOutsideCurrentList() const
{
    bool retVal = true;

    // Check if the list exists:
    if (_pCurrentlyActiveDisplayList != NULL)
    {
        // Check the mode:
        if (_pCurrentlyActiveDisplayList->getDisplayListMode() == GL_COMPILE)
        {
            retVal = false;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsDisplayListMonitor::getDisplayListTotalGeometryMapping
// Description: Check if a display list exist within the geometry mapping
// Author:      Sigal Algranaty
// Date:        6/7/2009
// ---------------------------------------------------------------------------
const int* gsDisplayListMonitor::getDisplayListTotalGeometryMapping(GLint displayListName) const
{
    const int* retVal = NULL;

    int foundIndex = getDisplayListIndex(displayListName);

    if (foundIndex >= 0)
    {
        retVal = _displayListTotalGeometryMapping[foundIndex];
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsDisplayListMonitor::getDisplayListTotalGeometryMapping
// Description: Check if a display list exist within the geometry mapping
// Author:      Uri Shomroni
// Date:        10/1/2010
// ---------------------------------------------------------------------------
int* gsDisplayListMonitor::getDisplayListTotalGeometryMapping(GLint displayListName)
{
    int* retVal = NULL;

    int foundIndex = getDisplayListIndex(displayListName);

    if (foundIndex >= 0)
    {
        retVal = _displayListTotalGeometryMapping[foundIndex];
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsDisplayListMonitor::getDisplayListGeometryMapping
// Description: Check if a display list exist within the geometry mapping
// Author:      Sigal Algranaty
// Date:        10/1/2010
// ---------------------------------------------------------------------------
int* gsDisplayListMonitor::getDisplayListGeometryMapping(GLint displayListName)
{
    int* retVal = NULL;

    int foundIndex = getDisplayListIndex(displayListName);

    if (foundIndex >= 0)
    {
        retVal = _displayListPrimitivesAmount[foundIndex];
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsDisplayListMonitor::getDisplayListIndex
// Description: Gets a display list's index in the vectors from its OpenGL name.
//              or -1 if it isn't (invalid value, deleted, ...)
// Author:      Uri Shomroni
// Date:        10/1/2010
// ---------------------------------------------------------------------------
int gsDisplayListMonitor::getDisplayListIndex(GLuint displayListName) const
{
    int retVal = -1;

    gtMap<GLuint, int>::const_iterator findIter = _displayListNameToVectorIndex.find(displayListName);
    gtMap<GLuint, int>::const_iterator endIter = _displayListNameToVectorIndex.end();

    if (findIter != endIter)
    {
        // Get the list's index in the vector:
        int foundIndex = (*findIter).second;

        int numberOfDisplayLists = (int)_displayLists.size();
        int numberOfDisplayListGeometryMappings = (int)_displayListPrimitivesAmount.size();
        int numberOfDisplayListTotalGeometryMappings = (int)_displayListTotalGeometryMapping.size();

        // Make sure this value is either -1 or a valid index in both vectors:
        GT_IF_WITH_ASSERT((foundIndex >= -1) || ((foundIndex < numberOfDisplayLists) &&
                                                 (foundIndex < numberOfDisplayListGeometryMappings) && (foundIndex < numberOfDisplayListTotalGeometryMappings)))
        {
            retVal = foundIndex;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsDisplayListMonitor::updateTotalGeometryCountersOnEndList
//              (was gsRenderPrimitivesStatisticsLogger::onEndDisplayList_
// Description: Is called when a display list creation is done
// Author:      Sigal Algranaty
// Date:        27/5/2009
// ---------------------------------------------------------------------------
void gsDisplayListMonitor::updateTotalGeometryCountersOnEndList(GLuint newListName)
{
    // Calculate the new lists' geometry sizes:
    int amountOfPrimitivesDrawnByList[GS_AMOUNT_OF_PRIMITIVE_TYPES];
    ::memset(amountOfPrimitivesDrawnByList, 0, sizeof(int) * GS_AMOUNT_OF_PRIMITIVE_TYPES);

    // Recalculate our calling lists' new geometry mappings:
    gtVector<GLuint> pathFromNewList;
    recalculateGeometrySizesForCallingLists(pathFromNewList, newListName);
}

// ---------------------------------------------------------------------------
// Name:        gsDisplayListMonitor::recalculateGeometrySizesForCallingLists
// Description: When targetList's geometry sizes change (due to calling glNewList and glEndList),
//              we need to update any list in which it is nested. This function
// Implementation Notes:
// Note that this function might calculate the same list's value twice in the
// iterations, but the last value calculated will be correct. Consider:
//      1 -.
//     / \  \.
//    2   3  5
//     \ /
//      4
// where 4 was just recompiled.
// We will go up 4 -> 2 -> 1 then 4 -> 3 -> 1.
// The first time we calculate 1's total geometry, it will use the old value of 3, but the second
// time, 2's value will have already been updated.
// Performing it like this might cause this redundancy, but it allows us not to
// recalculate unneeded lists (e.g. 5), which would harm performance worse.
// Author:      Uri Shomroni
// Date:        10/1/2010
// ---------------------------------------------------------------------------
void gsDisplayListMonitor::recalculateGeometrySizesForCallingLists(gtVector<GLuint>& pathFromNewList, GLuint targetList)
{
    // See if we have a loop by checking if we passed this place on the path:
    bool isAcyclic = true;
    int pathLength = (int)pathFromNewList.size();

    for (int i = 0; i < pathLength; i++)
    {
        if (pathFromNewList[i] == targetList)
        {
            isAcyclic = false;
            break;
        }
    }

    // If we got here on a cycle-free path from the newly compiled list.
    if (isAcyclic)
    {
        // Push us into the path:
        pathFromNewList.push_back(targetList);

        // Get the object representing this list:
        const apGLDisplayList* pTargetList = getDisplayListDetails(targetList);

        // Update the values for this list:
        int* pTargetListTotalGeometry = getDisplayListTotalGeometryMapping(targetList);
        GT_IF_WITH_ASSERT(pTargetListTotalGeometry != NULL)
        {
            // Add the values from the list itself:
            int* pTargetListGeometry = getDisplayListGeometryMapping(targetList);
            GT_IF_WITH_ASSERT(pTargetListGeometry != NULL)
            {
                for (int primType = 0; primType < GS_AMOUNT_OF_PRIMITIVE_TYPES; primType++)
                {
                    // Set the values instead of adding since we want to replace the former values:
                    pTargetListTotalGeometry[primType] = pTargetListGeometry[primType];
                }
            }
            else
            {
                // Something's wrong, just reset the values to 0:
                ::memset(pTargetListTotalGeometry, 0, GS_AMOUNT_OF_PRIMITIVE_TYPES * sizeof(int));
            }

            // Get the values for all my children:
            if (pTargetList != NULL)
            {
                const gtVector<GLuint>& targetListCalledLists = pTargetList->getCalledDisplayLists();
                int numberOfCalledLists = (int)targetListCalledLists.size();

                for (int i = 0; i < numberOfCalledLists; i++)
                {
                    // Get the current child list's total geomtery information. If a list is called
                    // twice, we will add this twice:
                    GLuint currentListName = targetListCalledLists[i];
                    int* pCurrentListTotalGeometry = getDisplayListTotalGeometryMapping(currentListName);

                    if (pCurrentListTotalGeometry != NULL)
                    {
                        for (int primType = 0; primType < GS_AMOUNT_OF_PRIMITIVE_TYPES; primType++)
                        {
                            pTargetListTotalGeometry[primType] += pCurrentListTotalGeometry[primType];
                        }
                    }
                }
            }
        }

        // Call this function recursively for all my calling lists. Note that the calling lists
        // vector should have no duplicate values:
        GT_IF_WITH_ASSERT(pTargetList != NULL)
        {
            const gtVector<GLuint>& targetListCallingLists = pTargetList->getCallingDisplayLists();
            int numberOfCallingLists = (int)targetListCallingLists.size();

            for (int i = 0; i < numberOfCallingLists; i++)
            {
                // Call this function recursively:
                const GLuint& callingListName = targetListCallingLists[i];
                recalculateGeometrySizesForCallingLists(pathFromNewList, callingListName);
            }
        }

        // Remove us from the path:
        pathFromNewList.pop_back();
    }
}

// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::calculateDisplayListsMemorySize
// Description: Calculate the current existing display lists memory size
// Arguments:   gtUInt64& displayListsMemorySize
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        19/7/2010
// ---------------------------------------------------------------------------
bool gsDisplayListMonitor::calculateDisplayListsMemorySize(gtUInt64& displayListsMemorySize) const
{
    bool retVal = true;
    displayListsMemorySize = 0;

    // Iterate the display lists:
    for (int i = 0; i < (int)_displayLists.size(); i++)
    {
        // Get the current display list:
        apGLDisplayList* pDisplayList = _displayLists[i];

        if (pDisplayList != NULL)
        {
            gtSize_t dispListMemorySize = 0;

            // Update the display list geometry size:
            dispListMemorySize = pDisplayList->geometrySize();

            // Set the object size in KB (ceil the number for 0.1KB to turn to be 1KB):
            float val = (float)dispListMemorySize / (float)(1024 * 8);
            dispListMemorySize = (gtUInt32)ceil(val);

            // Add the current memory size to display lists memory size:
            displayListsMemorySize += dispListMemorySize;
        }
    }

    return retVal;
}
