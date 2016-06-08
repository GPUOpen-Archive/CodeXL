//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLDisplayList.h
///
//==================================================================================

//------------------------------ apGLDisplayList.h ------------------------------

#ifndef __APGLDISPLAYLIST
#define __APGLDISPLAYLIST

// OpenGL
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTBaseTools/Include/gtVector.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTAPIClasses/Include/apAllocatedObject.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLDisplayList : public osTransferableObject
//
// General Description:
//   Represents an OpenGL display list.
// Author:  AMD Developer Tools Team
// Creation Date:        23/9/2008
// ----------------------------------------------------------------------------------
class AP_API apGLDisplayList : public apAllocatedObject
{
public:
    // Self functions:
    apGLDisplayList(GLuint displayListName = 0, GLenum displayListMode = GL_COMPILE);
    apGLDisplayList(const apGLDisplayList& other);
    virtual ~apGLDisplayList();
    apGLDisplayList& operator=(const apGLDisplayList& other);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    GLuint getDisplayListName() const {return _displayListName;};
    GLenum getDisplayListMode() const {return _displayListMode;};
    void setDisplayListMode(GLenum displayListMode) {_displayListMode = displayListMode;};
    gtUInt32 geometrySize() const {return _geometrySize;};
    gtUInt32 amountOfRenderedVertices()const {return _amountOfVertices;};
    void copyListDataOnCompilation(const apGLDisplayList& newListDetail);

    // Called display lists:
    void addCalledDisplayList(GLuint displayList) {_calledDisplayListsVector.push_back(displayList);};
    const gtVector<GLuint>& getCalledDisplayLists() const {return _calledDisplayListsVector;};

    // Calling display lists:
    void addCallingDisplayList(GLuint displayList);
    void removeCallingDisplayList(GLuint displayList);
    const gtVector<GLuint>& getCallingDisplayLists() const {return _callingDisplayLists;};

    void addGeometrySize(gtUInt32 addedGeometrySize, gtUInt32 amountOfVertices) {_geometrySize += addedGeometrySize; _amountOfVertices += amountOfVertices;};

private:
    // The OpenGL display list name:
    GLuint _displayListName;

    // GL_COMPILE or GL_COMPILE_AND_EXECUTE.
    GLenum _displayListMode;

    // Vector of called display lists:
    gtVector<GLuint> _calledDisplayListsVector;

    // Vector of lists calling this one:
    gtVector<GLuint> _callingDisplayLists;

    // Display list geometry size:
    gtUInt32 _geometrySize;

    // Display list amount of rendered vertices:
    gtUInt32 _amountOfVertices;
};


#endif  // __APGLDISPLAYLIST
