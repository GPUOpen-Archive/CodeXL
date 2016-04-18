//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsDisplayListMonitor.h
///
//==================================================================================

//------------------------------ gsDisplayListMonitor.h ------------------------------

#ifndef __GSDISPLAYLISTMONITOR
#define __GSDISPLAYLISTMONITOR

// Forward declaration:
class gsRenderContextMonitor;

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTAPIClasses/Include/apGLDisplayList.h>

// Local:
#include <src/gsRenderPrimitiveType.h>

// ----------------------------------------------------------------------------------
// Class Name:           gsDisplayListMonitor
//
// General Description:
//   Monitors display list Objects allocated in a given context.
//
// Author:               Sigal Algranaty
// Creation Date:        23/9/2008
// ----------------------------------------------------------------------------------
class gsDisplayListMonitor
{
public:
    gsDisplayListMonitor(gsRenderContextMonitor* pRenderContextMonitor);
    ~gsDisplayListMonitor();

public:
    // Display List actions:
    bool removeDisplayList(GLuint displayListName);
    bool removeDisplayLists(GLuint displayListName, GLsizei displayListsRange);
    bool addNewDisplayList(GLuint displayListName);
    bool addNewDisplayLists(GLuint firstDisplayListName, GLsizei displayListsRange);
    bool onNewList(GLuint displayListName, GLenum displayListMode);
    bool displayListNameExists(GLuint displayListName) const {return getDisplayListDetails(displayListName) != NULL;};
    bool endDisplayList();
    bool callDisplayList(GLuint displayListName);
    bool callDisplayLists(GLsizei amountOfDispLists, GLenum type, const GLvoid* pListNames);

    // Active list (between glNewList() and glEndList() ):
    apGLDisplayList* getActiveDisplayListDetails() {return _pCurrentlyActiveDisplayList;};

    // Display List data:
    const apGLDisplayList* getDisplayListDetails(GLuint displayListName) const;
    apGLDisplayList* getDisplayListDetails(GLuint displayListName);
    bool getDisplayListName(int displayListIndex, GLuint& displayListName) const;
    int amountOfDisplayLists() const {return (int)_displayLists.size();};

    // Update the display list geometry mapping
    void updateActiveDisplayListGeometryMapping(gsRenderPrimitiveType primitiveTypeToUpdate, int amountOfVertices, int amountOfPrimitivesToAdd);
    bool isGeometryDrawnOutsideCurrentList() const;
    const int* getDisplayListTotalGeometryMapping(GLint displayListName) const;

    // Memory:
    bool calculateDisplayListsMemorySize(gtUInt64& displayListsMemorySize) const ;

private:
    // Do not allow use of the = operator for this class. Use reference or pointer transferral instead
    gsDisplayListMonitor& operator=(const gsDisplayListMonitor& otherMonitor);
    gsDisplayListMonitor(const gsDisplayListMonitor& otherMonitor);
    int* getDisplayListTotalGeometryMapping(GLint displayListName);
    int* getDisplayListGeometryMapping(GLint displayListName);

    int getDisplayListIndex(GLuint displayListName) const;
    void updateTotalGeometryCountersOnEndList(GLuint newListName);

    void recalculateGeometrySizesForCallingLists(gtVector<GLuint>& pathFromNewList, GLuint targetList);

private:
    // Hold the currently active display list (-1 if none):
    apGLDisplayList* _pCurrentlyActiveDisplayList;
    int* _pCurrentlyActiveDisplayListGeometryMapping;

    // A map of Display list names to vector indexes to make access faster:
    gtMap<GLuint, int> _displayListNameToVectorIndex;

    // Hold the display lists objects that reside in this render context:
    gtPtrVector<apGLDisplayList*> _displayLists;

    // Holds the amount of primitives (per type) drawn by the display list itself
    gtVector<int*> _displayListPrimitivesAmount;

    // Holds the amount of primitives (per type) drawn by the display list and all its nested lists:
    gtVector<int*> _displayListTotalGeometryMapping;

    // Holds the render context monitor:
    gsRenderContextMonitor* _pRenderContextMonitor;
};


#endif  // __GSDISPLAYLISTMONITOR
